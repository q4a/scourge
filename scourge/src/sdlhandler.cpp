/***************************************************************************
                     sdlhandler.cpp  -  Interface to SDL
                             -------------------
    begin                : Sat May 3 2003
    copyright            : (C) 2003 by Gabor Torok
    email                : cctorok@yahoo.com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "common/constants.h"
#include "sdlhandler.h"
#include "gameadapter.h"
#include "gui/window.h"
#include "gui/eventhandler.h"
#include "preferences.h"
#include "sdleventhandler.h"
#include "sdlscreenview.h"
#include "session.h"
#include "party.h"
#include "debug.h"
#include "freetype/fontmgr.h"
#include "sound.h"
#include "sqbinding/sqbinding.h"
#include "render/cutscene.h"
#include "render/texture.h"

// ###### MS Visual C++ specific ###### 
#if defined(_MSC_VER) && defined(_DEBUG)
# define new DEBUG_NEW
# undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif 

using namespace std;

// FIXME: use static member variable, std::string is clear by default
string willSavePath = "";

vector<SDLHandler::FontInfo*> SDLHandler::fontInfos;
vector<string> modeDescriptions;
int videoModeCount;

#define FORBIDDEN_CURSOR_TIME 2000

#define FADEOUT_SPEED 10

//#define DEBUG_MOUSE_FOCUS 1

bool SDLHandler::stencilBufferUsed = false;
bool SDLHandler::showDebugInfo = SHOW_FPS;

// milllis
#define DOUBLE_CLICK_INTERVAL 500
// pixel range
#define DOUBLE_CLICK_TOLERANCE 5

#define DEFAULT_MIN_DEPTH -3000
#define DEFAULT_MAX_DEPTH 1000

SDLHandler::SDLHandler( GameAdapter *gameAdapter ) {
	/* These are to calculate our fps */
	this->gameAdapter = gameAdapter;
	T0     = 0;
	Frames = 0;
	fps = 0;
	screen = NULL;
	lastMouseX = lastMouseY = mouseX = mouseY = mouseButton = mouseEvent = 0;
	mouseFocusX = mouseFocusY = 0;
	mouseDragging = false;
	mouseIsMovingOverMap = false;
	handlerCount = 0;
	invertMouse = false;
	cursorMode = Constants::CURSOR_NORMAL;
	font_initialized = false;
	debugStr = NULL;
	lastLeftClick = 0;
	lastMouseMoveTime = SDL_GetTicks();
	isDoubleClick = false;
	dontUpdateScreen = false;
	mouseLock = NULL;
	willUnlockMouse = false;
	willBlockEvent = false;
	forbiddenTimer = 0;
	fadeoutStartAlpha = fadeoutEndAlpha = 0;
	fadeoutSteps = 16;
	fadeoutCurrentStep = 0;
	fadeoutTimer = 0;
	cursorVisible = true;
	continueFunc = "";
	continueTimeout = continueStart = 0;
	orthoDepthMin = DEFAULT_MIN_DEPTH;
	orthoDepthMax = DEFAULT_MAX_DEPTH;
}

SDLHandler::~SDLHandler() {
	// undoing XXX by ShapePalette::initFonts() + SDLHandler::initFonts()
	for ( size_t i = 0; i < fontInfos.size(); ++i ) {
		delete fontInfos[ i ]->fontMgr;
		delete fontInfos[ i ];
	}
	fontInfos.clear();
}

void SDLHandler::pushHandlers( SDLEventHandler *eventHandler,
                               SDLScreenView *screenView ) {
	if ( handlerCount == 10 ) {
		fprintf( stderr, "Error: can't push any more handlers." );
		exit( 1 );
	}
	eventHandlers[handlerCount] = this->eventHandler;
	screenViews[handlerCount] = this->screenView;
	handlerCount++;
	setHandlers( eventHandler, screenView );
}

bool SDLHandler::popHandlers() {
	if ( handlerCount == 0 ) return true;
	handlerCount--;
	setHandlers( eventHandlers[handlerCount], screenViews[handlerCount] );
	return false;
}

void SDLHandler::setHandlers( SDLEventHandler *eventHandler,
                              SDLScreenView *screenView ) {
	this->eventHandler = eventHandler;
	this->screenView = screenView;
	if ( screen ) resizeWindow( screen->w, screen->h );
}

/* function to release/destroy our resources and restoring the old desktop */
void SDLHandler::quit( int returnCode ) {
	gameAdapter->getSession()->setExiting( true );
	
#ifdef HAVE_SDL_NET
	// shutdown SDL_net
	SDLNet_Quit();
#endif

	/* clean up the window */
	SDL_Quit( );

	/* and exit appropriately */
	exit( returnCode );
}

void SDLHandler::resetDepthLimits() {
	setDepthLimits( DEFAULT_MIN_DEPTH, DEFAULT_MAX_DEPTH );
}

void SDLHandler::setDepthLimits( float min, float max ) {
	this->orthoDepthMin = min;
	this->orthoDepthMax = max;
	resizeWindow( screen->w, screen->h );
}

/* function to reset our viewport after a window resize */
void SDLHandler::resizeWindow( int width, int height ) {
	// Height / width ratio
//    GLfloat ratio;

	// Protect against a divide by zero
	if ( height == 0 ) height = 1;
	lastWidth = width;
	lastHeight = height;

//    ratio = ( GLfloat )width / ( GLfloat )height;

	// Setup our viewport.
	glViewport( 0, 0, ( GLsizei )width, ( GLsizei )height );

	// change to the projection matrix and set our viewing volume.
	glMatrixMode( GL_PROJECTION );
	glLoadIdentity( );

	// Set our perspective
	//gluPerspective( 45.0f, ratio, 0.1f, 100.0f );
	setOrthoView();

	// Make sure we're chaning the model view and not the projection
	glMatrixMode( GL_MODELVIEW );

	// Reset The View
	glLoadIdentity( );

}

// Note: !!! also called from Map::getMapXYZAtScreenXY !!!
void SDLHandler::setOrthoView() {
	glOrtho( 0.0f, lastWidth, lastHeight, 0.0f, orthoDepthMin, orthoDepthMax );
}

/* general OpenGL initialization function */
void SDLHandler::initGL() {
	/* Enable Texture Mapping */
	glsEnable( GLS_TEXTURE_2D );

	/* Enable smooth shading */
	glShadeModel( GL_SMOOTH );

	// We use ortho projection, so perspective correction isn't needed
	glHint( GL_PERSPECTIVE_CORRECTION_HINT, GL_FASTEST );

	// Create really nice mipmaps.
	glHint( GL_GENERATE_MIPMAP_HINT, GL_NICEST );

	// Use per-vertex fog.
	glHint( GL_FOG_HINT, GL_FASTEST );

	// Faster antialiasing for primitives.
	glHint( GL_POINT_SMOOTH_HINT, GL_FASTEST );
	glHint( GL_LINE_SMOOTH_HINT, GL_FASTEST );

	// which one to use?
	// default is good
	//glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_NEAREST);
	glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
	glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );

	/* Set the background black */
	glClearColor( 0.0f, 0.0f, 0.0f, 0.5f );

	/* Depth buffer setup */
	glClearDepth( 1.0f );
	if ( stencilBufferUsed ) glClearStencil( 0 ); // Clear The Stencil Buffer To 0

	/* Enables Depth Testing */
	glsEnable( GLS_DEPTH_TEST );

	/* The Type Of Depth Test To Do */
	glDepthFunc( GL_LEQUAL );

	// Don't dither textures on truecolor displays.
	if ( gameAdapter->getPreferences()->getBpp() > 16 ) {
		glDisable( GL_DITHER );
	} else {
		glEnable( GL_DITHER );
	}

	glColorMaterial ( GL_FRONT, GL_AMBIENT ) ;
	glEnable( GL_COLOR_MATERIAL );

	/* initialize opengl extensions */
	if ( Constants::multitexture ) {
//   fprintf(stderr, "BEFORE: glSDLActiveTextureARB=%u\n", glSDLActiveTextureARB);
		glSDLActiveTextureARB =
		  ( PFNGLACTIVETEXTUREARBPROC )SDL_GL_GetProcAddress ( "glActiveTextureARB" );
//   fprintf(stderr, "AFTER: glSDLActiveTextureARB=%u\n", glSDLActiveTextureARB);
		glSDLMultiTexCoord2fARB =
		  ( PFNGLMULTITEXCOORD2FARBPROC )SDL_GL_GetProcAddress ( "glMultiTexCoord2fARB" );
		glSDLMultiTexCoord2iARB =
		  ( PFNGLMULTITEXCOORD2IARBPROC )SDL_GL_GetProcAddress ( "glMultiTexCoord2iARB" );
	}
}

bool testModesInFormat( SDL_PixelFormat *format, Uint32 flags ) {
	SDL_Rect **modes;
	int i;

	printf( "Available modes with given flags in %d bpp:\n", format->BitsPerPixel );

	/* Get available fullscreen/hardware modes */
	modes = SDL_ListModes( format, flags );

	/* Check is there are any modes available */
	if ( modes == ( SDL_Rect ** )0 ) {
		printf( "\tNo modes available!\n" );
		return false;
	}

	/* Check if our resolution is restricted */
	if ( modes == ( SDL_Rect ** ) - 1 ) {
		printf( "\tAll resolutions available.\n" );
		return true;
	}

	/* Print valid modes */
	for ( i = 0;modes[i];++i )
		printf( "\t%d x %d\n", modes[i]->w, modes[i]->h );

	//free(modes); // crashes; ok to not free since we only do this a few times
	return true;
}

int testModes( Uint32 flags, bool findMaxBpp = false ) {
	int bpp[] = { 32, 24, 16, 15, 8, 0 };
	SDL_PixelFormat format;
	for ( int i = 0; bpp[i]; i++ ) {
		format.BitsPerPixel = bpp[i];
		if ( testModesInFormat( &format, flags ) && findMaxBpp ) return bpp[i];
	}
	return -1;
}


void SDLHandler::getVideoModes() {
	SDL_Rect **modes;
	Uint32 flags;

	modeDescriptions.clear();

	if ( !screen ) {
		fprintf( stderr, "SDLHandler :: you must allocate screen before calling getVideoModes!!\n" );
		exit( -1 );
	}

	// Get current video flags (hwsurface/swsurface, fullscreen/not fullscreen..)
	flags = screen->flags;

	// Get available modes for the current flags
	modes = SDL_ListModes( NULL, flags );

	// Copy them to a char array
	if ( modes != ( SDL_Rect ** )0 ) {
		videoModeCount = 0;
		if ( modes == ( SDL_Rect ** ) - 1 ) {
			// All modes are available, so let's go..
			videoModeCount = 14;
			modeDescriptions.push_back( "800 x 600" );
			modeDescriptions.push_back( "1024 x 600" );
			modeDescriptions.push_back( "1024 x 768" );
			modeDescriptions.push_back( "1152 x 864" );
			modeDescriptions.push_back( "1280 x 768" );
			modeDescriptions.push_back( "1280 x 800" );
			modeDescriptions.push_back( "1280 x 960" );
			modeDescriptions.push_back( "1280 x 1024" );
			modeDescriptions.push_back( "1400 x 1050" );
			modeDescriptions.push_back( "1440 x 900" );
			modeDescriptions.push_back( "1600 x 1200" );
			modeDescriptions.push_back( "1680 x 1050" );
			modeDescriptions.push_back( "1920 x 1200" );
			modeDescriptions.push_back( "2048 x 1536" );
		} else {
			// Only a few modes available, which ones ?
			for ( videoModeCount = 0; modes[videoModeCount]; videoModeCount++ );
			if ( videoModeCount ) {
				for ( int i = 0; i < videoModeCount; i++ ) {
					if ( modes[i]->h > 599 && modes[i]->w < 2049 ) {
						char temp[ 50 ];
						snprintf( temp, 50, "%d x %d", modes[i]->w, modes[i]->h );
						modeDescriptions.push_back( temp );
					}
				}
				videoModeCount = modeDescriptions.size();
			} else {
				videoModeCount = 1;
				modeDescriptions.push_back( "No modes available!\n" );
			}
		}
	} else {
		videoModeCount = 1;
		modeDescriptions.push_back( "No modes available!\n" );
	}
}

int SDLHandler::getVideoModeCount() {
	getVideoModes();
	return videoModeCount;
}

std::string SDLHandler::getVideoMode( int mode ) {
//  char vm[255];
//  snprintf(vm, 255, "%s", modeDescriptions[mode].c_str());
	return modeDescriptions[mode];
//  return vm;
}

void SDLHandler::setVideoMode( Preferences * uc ) {
	/* this holds some info about our display */
	const SDL_VideoInfo *videoInfo;

	/* initialize SDL */
	if ( SDL_Init( SDL_INIT_VIDEO ) < 0 ) {
		fprintf( stderr, "Video initialization failed: %s\n", SDL_GetError( ) );
		quit( 1 );
	}
	char* str = new char[100]; strcpy(str,"SDL_Init leaked a block"); // a test leak

	if ( TTF_Init() < 0 ) {
		fprintf( stderr, "Couldn't initialize SDL_ttf: %s\n", SDL_GetError() );
		quit( 1 );
	}

#ifdef HAVE_SDL_NET
	// initialize SDL_net
	if ( SDLNet_Init() == -1 ) {
		cerr << "*** error: SDLNet_Init: " << SDL_GetError() << endl;
		exit( 2 );
	}
#endif

	/* Fetch the video info */
	videoInfo = SDL_GetVideoInfo( );

	if ( !videoInfo ) {
		fprintf( stderr, "Video query failed: %s\n", SDL_GetError( ) );
		quit( 1 );
	}


	videoFlags  = SDL_OPENGL;          /* Enable OpenGL in SDL */

	// these make no sense, accrd. to: http://osdl.sourceforge.net/main/documentation/rendering/SDL-openGL.html#flags
	//if(uc->getDoublebuf()) {
	//videoFlags |= SDL_GL_DOUBLEBUFFER; /* Enable double buffering */
	//}
	//if(uc->getHwpal())
	//videoFlags |= SDL_HWPALETTE;       /* Store the palette in hardware */
	/* This checks to see if surfaces can be stored in memory */
// if(uc->getForce_hwsurf()) videoFlags |= SDL_HWSURFACE;
// else if(uc->getForce_swsurf()) videoFlags |= SDL_SWSURFACE;
// else {
	//if ( videoInfo->hw_available ) videoFlags |= SDL_HWSURFACE;
	//else videoFlags |= SDL_SWSURFACE;
	//}
	///* This checks if hardware blits can be done */
	//if ( uc->getHwaccel() && videoInfo->blit_hw ) videoFlags |= SDL_HWACCEL;

	if ( uc->getFullscreen() ) {
		videoFlags |= SDL_FULLSCREEN;
	} else if ( uc->getResizeable() ) {
		videoFlags |= SDL_RESIZABLE;       // Enable window resizing
	}

	if ( uc->getTest() ) {
		testModes( videoFlags );
		quit( 0 );
	}

	// try to find the highest bpp for this mode
	int bpp;
	if ( uc->getBpp() == -1 ) {
		bpp = testModes( videoFlags, true );
		if ( bpp == -1 ) {
			fprintf( stderr, "Could not detect suitable opengl video mode.\n" );
			fprintf( stderr, "You can manually select one with the -bpp option\n" );
			quit( 0 );
		} else {
			uc->setBpp( bpp );
		}
	}

	/* Sets up OpenGL double buffering */
	if ( uc->getDoublebuf() ) {
		SDL_GL_SetAttribute( SDL_GL_DOUBLEBUFFER, 1 );
	} else {
		SDL_GL_SetAttribute( SDL_GL_DOUBLEBUFFER, 0 );
	}

	// vertical retrace (0-off, 1-on, >1 every n-th retrace)
	SDL_GL_SetAttribute( SDL_GL_SWAP_CONTROL, 0 );

	if ( uc->getStencilbuf() ) {
		uc->setStencilBufInitialized( true );
		stencilBufferUsed = true;
		SDL_GL_SetAttribute( SDL_GL_STENCIL_SIZE, 8 );
	}

	cout << "Setting video mode: " << uc->getW() << "x" << uc->getH() << "x" << uc->getBpp() << endl;

	/* get a SDL surface */
	screen = SDL_SetVideoMode( uc->getW(), uc->getH(), uc->getBpp(), videoFlags );
	/* Verify there is a surface */
	if ( !screen ) {
		fprintf( stderr,  "Video mode set failed: %s\n", SDL_GetError( ) );
		quit( 1 );
	}

	/* hide the mouse cursor; we have our own */
	SDL_ShowCursor( SDL_DISABLE );
	SDL_WM_SetCaption( "Scourge", NULL );

	/* initialize OpenGL */
	initGL( );

	/* resize the initial window */
	resizeWindow( screen->w, screen->h );
	SDL_WarpMouse ( uc->getW() / 2, uc->getH() / 2 );
}

/* unused: void SDLHandler::fireEvent( Widget *widget, SDL_Event *event ) {
	storedWidget = widget;
	storedEvent = event;
}*/

/* unused: bool SDLHandler::firedEventWaiting() {
	return storedEvent != NULL;
}*/

void SDLHandler::applyMouseOffset( int x, int y, int *newX, int *newY ) {
	if ( cursorMode == Constants::CURSOR_CROSSHAIR ) {
		mouseFocusX = mouseFocusY = 24;
	} else {
		mouseFocusX = mouseFocusY = 0;
	}
	*newX = x + mouseFocusX;
	*newY = y + mouseFocusY;
}

Uint32 lastAmbientTime = 0;
void SDLHandler::mainLoop() {
	bool isActive = true;
	running = true;
	while ( true ) {
		if ( processEvents( &isActive ) ) return;
		if ( !running && popHandlers() ) return;

		/*if ( isActive )*/ drawScreen();
		gameAdapter->getSession()->getSound()->checkMusic( gameAdapter->getCurrentCombatMusic() || gameAdapter->inTurnBasedCombat(), 
		                                                   gameAdapter->getCurrentCombatMusic() );
		Uint32 now = SDL_GetTicks();
		if ( !gameAdapter->getAmbientPaused() && now - lastAmbientTime > AMBIENT_PAUSE_MIN ) {
			lastAmbientTime = now;
			if ( Util::dice( AMBIENT_ROLL ) == 0 ) {
				gameAdapter->getSession()->getSound()->
				startAmbientSound( gameAdapter->getSession()->getAmbientSoundName(),
				                   gameAdapter->getCurrentDepth() );
			}
		}
		if ( continueTimeout > 0 && now - continueStart > continueTimeout ) {
			char tmp[200];
			strcpy( tmp, continueFunc.c_str() );
			// clear it first, in case continueAt is called again from squirrel
			continueStart = continueTimeout = 0;
			continueFunc = "";
			gameAdapter->getSession()->getSquirrel()->callNoArgMethod( tmp );
		}
	}
}

bool SDLHandler::processEvents( bool *isActive ) {
	SDL_Event event;
	int mx, my;

	Uint32 now = SDL_GetTicks();
	mouseIsMovingOverMap = false;
	while ( SDL_PollEvent( &event ) ) {
		isDoubleClick = false;
		mouseEvent = mouseButton = 0;
		Widget *widget = NULL;
		Window *win = NULL;

		switch ( event.type ) {
		case SDL_MOUSEMOTION:
			if ( invertMouse ) event.motion.y = screen->h - event.motion.y;
			applyMouseOffset( event.motion.x, event.motion.y, &mx, &my );
			mouseX = mx;
			mouseY = my;
			mouseButton = event.button.button;
			mouseEvent = SDL_MOUSEMOTION;
			// don't process events during a fade
			if ( fadeoutTimer <= 0 && cursorVisible ) widget = delegateEvent( &event, mouseX, mouseY, &win );
			if ( !widget && !win ) {
				mouseIsMovingOverMap = true;
				lastMouseMoveTime = now;
			}
			break;
		case SDL_MOUSEBUTTONUP:
			if ( invertMouse ) event.button.y = screen->h - event.button.y;
			applyMouseOffset( event.button.x, event.button.y, &mx, &my );
			mouseEvent = SDL_MOUSEBUTTONUP;
			mouseButton = event.button.button;
			mouseDragging = false;
			if ( event.button.button == SDL_BUTTON_LEFT || event.button.button == SDL_BUTTON_RIGHT ) {
				isDoubleClick = ( now - lastLeftClick < DOUBLE_CLICK_INTERVAL &&
				                  abs( lastMouseX - event.button.x ) < DOUBLE_CLICK_TOLERANCE &&
				                  abs( lastMouseY - event.button.y ) < DOUBLE_CLICK_TOLERANCE );
				lastLeftClick = now;
				// don't process events during a fade
				if ( fadeoutTimer <= 0 && cursorVisible ) widget = delegateEvent( &event, mx, my, &win );
			}
			lastMouseX = event.button.x;
			lastMouseY = event.button.y;
			break;
		case SDL_MOUSEBUTTONDOWN:
			if ( invertMouse ) event.button.y = screen->h - event.button.y;
			applyMouseOffset( event.button.x, event.button.y, &mx, &my );
			mouseEvent = SDL_MOUSEBUTTONDOWN;
			mouseButton = event.button.button;
			mouseDragging = ( event.button.button == SDL_BUTTON_LEFT );
			//if(event.button.button == SDL_BUTTON_LEFT || event.button.button == SDL_BUTTON_RIGHT) {
			// don't process events during a fade
			if ( fadeoutTimer <= 0 && cursorVisible ) widget = delegateEvent( &event, mx, my, &win );
			//}
			break;
		case SDL_ACTIVEEVENT:
			/* Something's happend with our focus
			 * If we lost focus or we are iconified, we
			 * shouldn't draw the screen
			 */
			if ( isActive ) {
				*isActive = ( event.active.gain == 0 ? false : true );
			}
			break;
		case SDL_VIDEORESIZE:
			/* handle resize event */
			screen = SDL_SetVideoMode( event.resize.w,
			                           event.resize.h,
			                           16, videoFlags );
			if ( !screen ) {
				fprintf( stderr, "Could not get a surface after resize: %s\n", SDL_GetError( ) );
				quit( 1 );
			}
			resizeWindow( event.resize.w, event.resize.h );
			break;
		case SDL_KEYUP:
			// only process CTRL + F1 once (on keyup)
			if ( event.key.keysym.sym == SDLK_F1 &&
			        event.key.keysym.mod & KMOD_CTRL ) {
				SDL_WM_ToggleFullScreen( screen );
				break;
			}
			// NO BREAK!!!
		case SDL_KEYDOWN:
			applyMouseOffset( mouseX, mouseY, &mx, &my );
			// don't process events during a fade
			if ( fadeoutTimer <= 0 && cursorVisible ) widget = delegateEvent( &event, mx, my, &win );
			break;
		case SDL_QUIT:
			quit( 0 ); // handle quit requests
			break;
		default:
			break;
		}

		// Show pointer over widgets unless casting a spell
		if ( getCursorMode() == Constants::CURSOR_FORBIDDEN &&
		        SDL_GetTicks() - forbiddenTimer > FORBIDDEN_CURSOR_TIME ) {
			setCursorMode( Constants::CURSOR_NORMAL );
		}

		if ( !mouseIsMovingOverMap && getCursorMode() != Constants::CURSOR_CROSSHAIR ) {
			setCursorMode( Constants::CURSOR_NORMAL );
		}

		// swallow this event
		if ( willBlockEvent ) {
			willBlockEvent = false;
			continue;
		}

		bool res = false;
		// don't process events during a fade
		if ( fadeoutTimer <= 0 ) {
			if ( widget ) {
				if ( !mouseLock || mouseLock == widget ) {
					EventHandler *eh = getEventHandler( widget );
					if( eh ) {
						// new-style event handling
						res = eh->handleEvent( widget, &event );
					} else {
						// old-style event handling
						res = eventHandler->handleEvent( widget, &event );
					}
					if ( event.type == SDL_MOUSEMOTION ) {
						if( win && win->getRawEventHandler() ) {
							res = win->getRawEventHandler()->handleEvent( &event );
						}
						
						// also run the generic event handler.
						// this is so that moving the cursor over a
						// window doesn't scroll the map forever
						res = eventHandler->handleEvent( &event );
					}
				}
			} else {
				if ( !mouseLock ) {
					if( win && win->getRawEventHandler() ) {
						res = win->getRawEventHandler()->handleEvent( &event );
					}
					
					// also run the generic event handler
					res = eventHandler->handleEvent( &event );
				}
			}
			if ( res ) {
				if ( popHandlers() ) {
					return true;
				}
			}
		}
	}

	if ( willUnlockMouse ) {
		mouseLock = NULL;
		willUnlockMouse = false;
	}

	return false;
}

void SDLHandler::drawCursor() {
	// for cursor: do alpha bit testing
	//  glsEnable( GLS_ALPHA_TEST );
	//  glAlphaFunc( GL_NOTEQUAL, 0 ); // this works better for people with the reverse alpha problem (see forums)
	glsDisable( GLS_CULL_FACE | GLS_DEPTH_TEST );
	glsEnable( GLS_TEXTURE_2D | GLS_BLEND );
	glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );
	glPushMatrix();
	glLoadIdentity();
	glTranslatef( mouseX - mouseFocusX, mouseY - mouseFocusY, 0 );
	gameAdapter->getCursorTexture( cursorMode ).glBind();
	glColor4f( 1, 1, 1, 1 );
//  glNormal3f( 0, 0, 1 );
	glBegin( GL_TRIANGLE_STRIP );
	glTexCoord2i( 0, 0 );
	glVertex2i( 0, 0 );
	glTexCoord2i( 1, 0 );
	glVertex2i( gameAdapter->getCursorWidth(), 0 );
	glTexCoord2i( 0, 1 );
	glVertex2i( 0, gameAdapter->getCursorHeight() );
	glTexCoord2i( 1, 1 );
	glVertex2i( gameAdapter->getCursorWidth(), gameAdapter->getCursorHeight() );
	glEnd();
	glPopMatrix();

	//  glsDisable( GLS_ALPHA_TEST );
	glsDisable( GLS_TEXTURE_2D | GLS_BLEND );

#ifdef DEBUG_MOUSE_FOCUS
	// cursor focus
	glPushMatrix();
	glLoadIdentity();
	glTranslatef( mouseX, mouseY, 0 );
	glColor4f( 1, 1, 1, 1 );
	glBegin( GL_TRIANGLE_STRIP );
	glVertex2i( 0, 0 );
	glVertex2i( 10, 0 );
	glVertex2i( 0, 10 );
	glVertex2i( 10, 10 );
	glEnd();
	glPopMatrix();
#endif

	glsEnable( GLS_CULL_FACE | GLS_DEPTH_TEST );
}

void SDLHandler::processEventsAndRepaint() {
	processEvents();
	drawScreen();
}

void SDLHandler::drawScreen() {

	if ( eventHandler == NULL ) { // it's never set to NULL
		SDL_GL_SwapBuffers();
		return;
	}

	drawScreenInternal();

	if ( !willSavePath.empty() ) { // it's always empty
		saveScreenInternal( willSavePath );
		willSavePath.clear();
	}

	/* Gather our frames per second */
	calculateFps();
}

void SDLHandler::saveScreen( string& path, bool thumbnail ) {
	drawScreenInternal();
	saveScreenInternal( path, thumbnail );
}

void SDLHandler::drawScreenInternal() {
	glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
	if ( stencilBufferUsed ) glClear( GL_STENCIL_BUFFER_BIT );
	glClearColor( 0.0f, 0.0f, 0.0f, 0.5f );
	glClearDepth( 1.0f );
	screenView->drawView();

	// redraw the gui
	drawVisibleWindows();

	screenView->drawAfter();

	if ( fadeoutTimer > 0 ) {
		drawFadeout();
	}

	if ( cursorVisible ) drawCursor();

	if ( showDebugInfo ) {
		drawDebugInfo();
	}

	// these tie cpu execution to the gpu's so there is no benefit from the overlap
	//glFlush();
	//glFinish();

	/* Draw it to the screen */
	SDL_GL_SwapBuffers( );
}

#define SCREEN_SHOT_WIDTH 160
#define SCREEN_SHOT_HEIGHT 120

void SDLHandler::saveScreenInternal( string& path, bool thumbnail ) {
	if ( !gameAdapter->getPreferences()->getEnableScreenshots() ) {
		cerr << "*** Screenshots disabled in options. Not saving: " << path << endl;
		return;
	}

	SDL_Surface *surface = SDL_CreateRGBSurface( SDL_SWSURFACE, screen->w, screen->h, 24,
#if SDL_BYTEORDER == SDL_LIL_ENDIAN
	                        0x00FF0000, 0x0000FF00, 0x000000FF, 0 );
#else
	                        0x000000FF, 0x0000FF00, 0x00FF0000, 0 );
#endif

	// read the center of the screen
	int sx = ( screen->w - surface->w ) / 2;
	int sy = ( screen->h - surface->h ) / 2;

	glReadPixels( sx, sy, surface->w, surface->h, GL_BGR, GL_UNSIGNED_BYTE, surface->pixels );

	int w, h;
	if ( thumbnail ) {
		w = SCREEN_SHOT_WIDTH; h = SCREEN_SHOT_HEIGHT;
	} else {
		w = surface->w; h = surface->h;
	}

	SDL_Surface *scaled = SDL_CreateRGBSurface( SDL_SWSURFACE, w, h, 24,
#if SDL_BYTEORDER == SDL_LIL_ENDIAN
	                        0x00FF0000, 0x0000FF00, 0x000000FF, 0 );
#else
	                        0x000000FF, 0x0000FF00, 0x00FF0000, 0 );
#endif

	// scale to desired size (also flip image at the same time)
	Uint8 *src = ( Uint8* )surface->pixels;
	Uint8 *dst = ( Uint8* )scaled->pixels;
	float dx = ( static_cast<float>( surface->w ) / static_cast<float>( scaled->w ) );
	float dy = ( static_cast<float>( surface->h ) / static_cast<float>( scaled->h ) );

	for ( int x = 0; x < w; x++ ) {
		for ( int y = 0; y < h; y++ ) {
			memcpy( dst + ( scaled->pitch * y + x * scaled->format->BytesPerPixel ), src + ( surface->pitch * ( surface->h - 1 - static_cast<int>( y * dy ) ) + static_cast<int>( x * dx ) * surface->format->BytesPerPixel ), scaled->format->BytesPerPixel );
		}
	}

	SDL_SaveBMP( scaled, path.c_str() );

	SDL_FreeSurface( surface );
	SDL_FreeSurface( scaled );
}

void SDLHandler::calculateFps() {
	Frames++;
	GLint t = SDL_GetTicks();
	if ( t - T0 >= 2500 ) {
		GLfloat seconds = ( t - T0 ) / 1000.0;
		fps = Frames / seconds;
		//printf("%d frames in %g seconds = %g FPS\n", Frames, seconds, fps);
		T0 = t;
		Frames = 0;
	}
}

void SDLHandler::drawDebugInfo() {
	glPushMatrix();
	glsDisable( GLS_CULL_FACE | GLS_DEPTH_TEST | GLS_DEPTH_MASK );
	glLoadIdentity();
	glColor3f( 0, 0, 0 );
	glBegin( GL_TRIANGLE_STRIP );
	glVertex2i( 400, 0 );
	glVertex2i( screen->w, 0 );
	glVertex2i( 400, 12 );
	glVertex2i( screen->w, 12 );
	glEnd();
	glsEnable( GLS_TEXTURE_2D );
	glColor4f( 0.8f, 0.7f, 0.2f, 1.0f );
	texPrint( 400, 10, "FPS: %g %s", getFPS(), ( debugStr ? debugStr : "" ) );
	glsEnable( GLS_DEPTH_TEST | GLS_DEPTH_MASK );
	glPopMatrix();
}

void SDLHandler::drawFadeout() {
	glPushMatrix();
	glsDisable( GLS_TEXTURE_2D | GLS_DEPTH_TEST );
	glsEnable( GLS_BLEND );
	glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );

	if ( fadeoutStartAlpha < fadeoutEndAlpha ) {
		glColor4f( 0, 0, 0, ( fadeoutStartAlpha + ( ( ( fadeoutEndAlpha - fadeoutStartAlpha ) *
		                                              fadeoutCurrentStep ) / static_cast<float>( fadeoutSteps ) ) ) );
	} else {
		glColor4f( 0, 0, 0, ( fadeoutStartAlpha - ( ( ( fadeoutStartAlpha - fadeoutEndAlpha ) *
		                                              fadeoutCurrentStep ) / static_cast<float>( fadeoutSteps ) ) ) );
	}
	glLoadIdentity();
	glBegin( GL_TRIANGLE_STRIP );
	glVertex2i( 0, 0 );
	glVertex2i( screen->w, 0 );
	glVertex2i( 0, screen->h );
	glVertex2i( screen->w, screen->h );
	glEnd();

	glsEnable( GLS_TEXTURE_2D | GLS_BLEND | GLS_DEPTH_TEST );
	glPopMatrix();


	Uint32 t = SDL_GetTicks();
	if ( t - fadeoutTimer > FADEOUT_SPEED ) {
		fadeoutCurrentStep++;
		if ( fadeoutCurrentStep > fadeoutSteps ) {
			fadeoutTimer = 0;
		} else {
			fadeoutTimer = t;
		}
	}
}

void SDLHandler::fade( float startAlpha, float endAlpha, int steps ) {
	fadeoutEndAlpha = endAlpha;
	fadeoutStartAlpha = startAlpha;
	fadeoutSteps = steps;
	fadeoutCurrentStep = 0;
	fadeoutTimer = SDL_GetTicks();
	while ( fadeoutTimer > 0 ) {
		processEventsAndRepaint();
	}
	setCursorMode( Constants::CURSOR_NORMAL );
}

TTF_Font *SDLHandler::getCurrentTTFFont() {
	initFonts();
	return fontInfos[ fontType ]->font;
}

FontMgr *SDLHandler::getCurrentFontManager() {
	initFonts();
	return fontInfos[ fontType ]->fontMgr;
}


int SDLHandler::textWidth( const char *fmt, ... ) {
	char str[256]; // Holds our string
	va_list ap;     // Pointer to our list of elements

	// If there's no text, do nothing
	if ( fmt == NULL ) return 0;

	// Parses The String For Variables
	va_start( ap, fmt );

	// Converts Symbols To Actual Numbers
	vsprintf( str, fmt, ap );
	va_end( ap );

	initFonts();

	//return getTextLengthSimple( *(getCurrentFont()), str );
	SDL_Rect r;
	getCurrentFontManager()->textSizeUTF8( str, &r );
	return r.w;
}

void SDLHandler::texPrint( GLfloat x, GLfloat y,
                           const char *fmt, ... ) {
	char str[256]; // Holds our string
	va_list ap;     // Pointer to our list of elements

	// If there's no text, do nothing
	if ( fmt == NULL ) return;

	// Parses The String For Variables
	va_start( ap, fmt );

	// Converts Symbols To Actual Numbers
	vsprintf( str, fmt, ap );
	va_end( ap );

	initFonts();

//  freetype_print_simple( *(getCurrentFont()), x, y, str );
	fontInfos[ fontType ]->fontMgr->drawTextUTF8( str,
	                                              toint( x ),
	                                              toint( y + fontInfos[ fontType ]->yoffset ) );
}

// XXX: second-half-initializing first half is done by ShapePalette 
void SDLHandler::initFonts() {
	if ( !font_initialized ) {
		//cerr << "Loading " << fontInfos.size() << " fonts: " << endl;
		for ( unsigned int i = 0; i < fontInfos.size(); i++ ) {
			FontInfo *info = fontInfos[i];
			//cerr << "\t" << info->path << endl;
			string s = rootDir + "/" + info->path;
			info->font = TTF_OpenFont( s.c_str(), info->size );
			TTF_SetFontStyle( info->font, info->style );
			if ( !info->font ) {
				fprintf( stderr, "Couldn't load %d pt font from %s: %s\n", info->size, s.c_str(), SDL_GetError() );
				quit( 2 );
			} else {
				//cerr << "\t\tSuccess." << endl;
				info->fontMgr = new FontMgr( info->font, info->shadowX, info->shadowY );
			}
		}
		//cerr << "Done loading fonts." << endl;
		font_initialized = true;
	}
}

bool SDLHandler::sectionIntersects( int a1, int a2, int b1, int b2 ) {
	return( ( ( a1 <= b1 && a2 > b1 ) || ( a1 >= b1 && a1 < b2 ) )
	        ? true : false );
}

bool SDLHandler::intersects( SDL_Rect *a, SDL_Rect *b ) {
	return( ( sectionIntersects( a->x, a->x + a->w, b->x, b->x + b->w ) &&
	          sectionIntersects( a->y, a->y + a->h, b->y, b->y + b->h ) )
	        ? true : false );
}

bool SDLHandler::intersects( int x, int y, int w, int h,
                             int x2, int y2, int w2, int h2 ) {
	SDL_Rect ra = {
		x, y, w, h
	};
	SDL_Rect rb = {
		x2, y2, w2, h2
	};
	return intersects( &ra, &rb );
}

void SDLHandler::drawTooltip( float xpos2, float ypos2, float zpos2,
                              float zrot, float yrot,
                              char *message,
                              float r, float g, float b,
                              float zoom ) {
	setFontType( Constants::SCOURGE_MONO_FONT );

	int w = 0;
	vector<int> widths;
	vector<string> lines = Util::Tokenize<vector<string> >( message, "|" );
	for ( vector<string>::iterator i = lines.begin(); i != lines.end(); i++ ) {
		int ww = textWidth( i->c_str() ) + 10;
		widths.push_back( ww );
		if ( w < ww )
			w = ww;
	}

	//int w = textWidth( message ) + 10;
	//int w = strlen( message ) * 8 + 4;
	int h = 12 * lines.size() + 5;
	int x = -2;
	int y = -14;

	// only for widget tooltips: see if it hangs off the screen
	bool right = false;
	if ( zrot == 0 && yrot == 0 ) {
		// do gluProject b/c parent window coordinates aren't part of xpos2.
		GLdouble screenx, screeny, screenz;
		double projection[16];
		double modelview[16];
		GLint viewport[4];
		glGetDoublev( GL_PROJECTION_MATRIX, projection );
		glGetDoublev( GL_MODELVIEW_MATRIX, modelview );
		glGetIntegerv( GL_VIEWPORT, viewport );
		int res = gluProject( xpos2 + w + x, 0, 0,
		                      modelview,
		                      projection,
		                      viewport,
		                      &screenx, &screeny, &screenz );
		if ( res && screenx > getScreenWidth() ) {
			xpos2 -= ( w + x );
			right = true;
		}
	}

	// for widget tooltips only (hence the check for zrot/yrot)
	if ( zrot == 0 && yrot == 0 && xpos2 + w + x > screen->w ) {

	}

	glPushMatrix();
	glTranslatef( xpos2, ypos2 - ( y + h - 20 ), zpos2 );
	glRotatef( zrot, 0.0f, 0.0f, 1.0f );
	glRotatef( yrot, 1.0f, 0.0f, 0.0f );

	glScalef( zoom, zoom, zoom );

	glsDisable( GLS_CULL_FACE | GLS_DEPTH_TEST );
	glsEnable( GLS_BLEND );
	glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );

	//glColor4f( 0, 0.15f, 0.05f, 0.5 );
	glColor4f( r, g, b, 0.8f );
	glBegin( GL_TRIANGLE_STRIP );
	glVertex2i( x, y );
	glVertex2i( x + w, y );
	glVertex2i( x, y + h );
	glVertex2i( x + w, y + h );
	glEnd();
	glBegin( GL_TRIANGLES );
	if ( right ) {
		glVertex2i( x + w, y + h - 5 );
		glVertex2i( x + w + 5, y + h + 5 );
		glVertex2i( x + w - 5, y + h );
	} else {
		glVertex2i( x, y + h - 5 );
		glVertex2i( x - 5, y + h + 5 );
		glVertex2i( x + 5, y + h );
	}
	glEnd();
	glsDisable( GLS_BLEND );

	//glColor4f( 0, 0.4f, 0.15f, 0.5 );
	for ( int i = 0; i < 2; i++ ) {
		if ( !i ) {
			glLineWidth( 3.0f );
			glColor4f( 0, 0, 0, 0 );
		} else {
			glLineWidth( 1.0f );
			glColor4f( r + 0.35f, g + 0.35f, b + 0.35f, 0.8f );
		}
		glBegin( GL_LINE_LOOP );
		if ( right ) {
			glVertex2i( x + w, y );
			glVertex2i( x, y  );
			glVertex2i( x, y + h  );
			glVertex2i( x + w - 5, y + h  );
			glVertex2i( x + w + 5, y + h + 5  );
			glVertex2i( x + w, y + h - 5  );
		} else {
			glVertex2i( x + w, y );
			glVertex2i( x, y  );
			glVertex2i( x, y + h - 5 );
			glVertex2i( x - 5, y + h + 5 );
			glVertex2i( x + 5, y + h );
			glVertex2i( x + w, y + h );
		}
		glEnd();
	}

	glColor4f( 1, 1, 1, 1 );
	for ( unsigned int i = 0; i < lines.size(); i++ ) {
		int ww = widths[ i ];
		int x = static_cast<int>( ( w - ww ) / 2.0f ) + 5;
		texPrint( x, i * 12, "%s", lines[i].c_str() );
	}
	//texPrint( 0, 0, "%s", message );
	setFontType( Constants::SCOURGE_DEFAULT_FONT );
	glPopMatrix();
}

Texture const& SDLHandler::getHighlightTexture() {
	return gameAdapter->getHighlightTexture();
}

Texture const& SDLHandler::getGuiTexture() {
	return gameAdapter->getGuiTexture();
}

Texture const& SDLHandler::getGuiTexture2() {
	return gameAdapter->getGuiTexture2();
}

Texture const& SDLHandler::loadSystemTexture( char *line ) {
	return gameAdapter->loadSystemTexture( line );
}

void SDLHandler::allWindowsClosed() {
	/*
	if( gameAdapter->getSession()->getParty() &&
	    gameAdapter->getSession()->getParty()->getPartySize() > 0 ) {
	  gameAdapter->getSession()->getParty()->toggleRound( false );
	}
	*/
}

void SDLHandler::setCursorMode( int n, bool useTimer ) {
	cursorMode = n;
	if ( cursorMode == Constants::CURSOR_FORBIDDEN && useTimer ) {
		forbiddenTimer = SDL_GetTicks();
	}
}

void SDLHandler::setUpdate( char *message, int n, int total ) {
	if ( screenView->setUpdate( message, n, total ) ) {
		processEventsAndRepaint();
	}
}

void SDLHandler::playSound( const std::string& file, int panning ) {
	gameAdapter->getSession()->getSound()->playSound( file, panning );
}

void SDLHandler::setContinueAt( char *func, int timeout ) {
	this->continueFunc = func;
	this->continueTimeout = timeout;
	this->continueStart = SDL_GetTicks();
}
