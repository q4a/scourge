
/***************************************************************************
                          sdlhandler.cpp  -  description
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

#include "sdlhandler.h"

SDLHandler::SDLHandler(){
  /* These are to calculate our fps */
  T0     = 0;
  Frames = 0;                       
  fps = 0;
  screen = NULL;
  mouseX = mouseY = mouseButton = mouseEvent = 0;
  mouseDragging = false;
  text = NULL;
  handlerCount = 0;
  invertMouse = false;
}

SDLHandler::~SDLHandler(){
}

void SDLHandler::pushHandlers(SDLEventHandler *eventHandler,
                              SDLScreenView *screenView) {
    if(handlerCount == 10) {
        fprintf(stderr, "Error: can't push any more handlers.");
        exit(1);
    }
    eventHandlers[handlerCount] = this->eventHandler;
    screenViews[handlerCount] = this->screenView;
    handlerCount++;
    setHandlers(eventHandler, screenView);
}

bool SDLHandler::popHandlers() {
    if(handlerCount == 0) return true;
    handlerCount--;
    setHandlers(eventHandlers[handlerCount], screenViews[handlerCount]);
    return false;
}

void SDLHandler::setHandlers(SDLEventHandler *eventHandler,
                             SDLScreenView *screenView) {
  this->eventHandler = eventHandler;
  this->screenView = screenView;
  if(screen) resizeWindow( screen->w, screen->h );
}

/* function to release/destroy our resources and restoring the old desktop */
void SDLHandler::quit( int returnCode ) {
    /* clean up the window */
    SDL_Quit( );

    /* and exit appropriately */
    exit( returnCode );
}

/* function to reset our viewport after a window resize */
int SDLHandler::resizeWindow( int width, int height ) {
   // Height / width ratio
//    GLfloat ratio;

    // Protect against a divide by zero
   if ( height == 0 ) height = 1;
   this->lastWidth = width;
   this->lastHeight = height;
   
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

    return( TRUE );
}

void SDLHandler::setOrthoView() {
    glOrtho(0.0f, lastWidth, lastHeight, 0.0f, -1000.0f, 1000.0f);
}

/* general OpenGL initialization function */
int SDLHandler::initGL( GLvoid ) {
    shapePal = new ShapePalette();

    /* Enable Texture Mapping */
    glEnable( GL_TEXTURE_2D );
      
    /* Enable smooth shading */
    glShadeModel( GL_SMOOTH );

	// which one to use?
    // default is good
	//glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_NEAREST);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

    /* Set the background black */
    glClearColor( 0.0f, 0.0f, 0.0f, 0.5f );

    /* Depth buffer setup */
    glClearDepth( 1.0f );
	glClearStencil(0);									// Clear The Stencil Buffer To 0

    /* Enables Depth Testing */
    glEnable( GL_DEPTH_TEST );

    /* The Type Of Depth Test To Do */
    glDepthFunc( GL_LEQUAL );

    /* Really Nice Perspective Calculations */
//    glHint( GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST );

    glColorMaterial ( GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE ) ;
    glEnable( GL_COLOR_MATERIAL );

    /* initialize opengl extensions */
	if(Constants::multitexture) {
	  fprintf(stderr, "BEFORE: glSDLActiveTextureARB=%u\n", glSDLActiveTextureARB);
	  glSDLActiveTextureARB = 
		(PFNGLACTIVETEXTUREARBPROC)SDL_GL_GetProcAddress ("glActiveTextureARB");
	  fprintf(stderr, "AFTER: glSDLActiveTextureARB=%u\n", glSDLActiveTextureARB);
	  glSDLMultiTexCoord2fARB = 
		(PFNGLMULTITEXCOORD2FARBPROC)SDL_GL_GetProcAddress ("glMultiTexCoord2fARB");
	  glSDLMultiTexCoord2iARB = 
		(PFNGLMULTITEXCOORD2IARBPROC)SDL_GL_GetProcAddress ("glMultiTexCoord2iARB");
	}
    return( TRUE );
}

bool testModesInFormat(SDL_PixelFormat *format, Uint32 flags) {
  SDL_Rect **modes;
  int i;

  printf("Available modes with given flags in %d bpp:\n", format->BitsPerPixel);

  /* Get available fullscreen/hardware modes */
  modes=SDL_ListModes(format, flags);
  
  /* Check is there are any modes available */
  if(modes == (SDL_Rect **)0){
	printf("\tNo modes available!\n");
	return false;
  }
  
  /* Check if our resolution is restricted */
  if(modes == (SDL_Rect **)-1){
	printf("\tAll resolutions available.\n");
	return true;
  }

  /* Print valid modes */
  for(i=0;modes[i];++i)
	printf("\t%d x %d\n", modes[i]->w, modes[i]->h);

  //free(modes); // crashes; ok to not free since we only do this a few times
  return true;
}

int testModes(Uint32 flags, bool findMaxBpp=false) {
  int bpp[] = { 32, 24, 16, 15, 8, 0 };
  SDL_PixelFormat format;
  for(int i = 0; bpp[i]; i++) {
	format.BitsPerPixel=bpp[i];
	if(testModesInFormat(&format, flags) && findMaxBpp) return bpp[i];
  }
  return -1;
}

// Get video variables from config file, these values can be overriden by 
// command line.
void SDLHandler::loadUserConfiguration(UserConfiguration *uc){
    fullscreen = uc->getFullscreen();
    doublebuf = uc->getDoublebuf();
    hwpal = uc->getHwpal();
    resizeable = uc->getResizeable();
    force_hwsurf = uc->getForce_hwsurf();
    force_swsurf = uc->getForce_swsurf();
    hwaccel = uc->getHwaccel();    
    bpp = uc->getBpp();
    w = uc->getW();
    h = uc->getH(); 
    Constants::shadowMode = uc->getShadows();
}

void SDLHandler::setVideoMode(int argc, char *argv[]) { 
  bool printusage = false;
  bool test = false;
  
  // interpret command line args
  for(int i = 1; i < argc; i++) {
	if(strstr(argv[i], "--bpp") == argv[i]) {	  
	  bpp = atoi(argv[i] + 5);
	  if(!(bpp ==8 || bpp == 15 || bpp == 16 || bpp == 24 || bpp == 32)) {
		printf("Error: bad bpp=%d\n", bpp);
		printusage = true;
	  }
	} else if(strstr(argv[i], "--width") == argv[i]) {	  
	  w = atoi(argv[i] + 7);
	  if(!w) {
		printf("Error: bad width=%s\n", argv[i] + 7);
		printusage = true;
	  }
	} else if(strstr(argv[i], "--height") == argv[i]) {	  
	  h = atoi(argv[i] + 8);
	  if(!h) {
		printf("Error: bad height=%s\n", argv[i] + 8);
		printusage = true;
	  }
	} else if(strstr(argv[i], "--shadow") == argv[i]) {	  
	  Constants::shadowMode = atoi(argv[i] + 8);	  
      if(!(Constants::shadowMode == 0 || 
           Constants::shadowMode == 1 || 
           Constants::shadowMode == 2)) {
          printf("Error: bad shadow mode: %d\n", Constants::shadowMode);
          printusage = true;
      }
	} else if(!strcmp(argv[i], "--version")) {
	  printf("Scourge, version %.2f\n", SCOURGE_VERSION);
	  quit(0);
	} else if(!strcmp(argv[i], "--test")) {
	  test = true;
	} else if(argv[i][0] == '-' && argv[i][1] != '-') {
	  for(int t = 1; t < (int)strlen(argv[i]); t++) {
		switch(argv[i][t]) {
		case 'h': case '?': printusage = true; break;
		case 'f': fullscreen = false; break;
		case 'd': doublebuf = false; break;
		case 'p': hwpal = false; break;
		case 'r': resizeable = false; break;
		case 'H': force_hwsurf = true; break;
		case 'S': force_swsurf = true; break;
		case 'a': hwaccel = false; break;
		case 's': Constants::stencilbuffer = false; break;
		case 'm': Constants::multitexture = false; break;
		}
	  }
	} else {
	  printusage = true;
	}
  }

  if(printusage) {
	printf("S.C.O.U.R.G.E.: Heroes of Lesser Renown\n");
	printf("A 3D, roguelike game of not quite epic proportions.\n\n");
	printf("Usage:\n");
	printf("scourge [-fdprHSa?hsm] [--test] [--bppXX] [--help] [--version] [--shadowX]\n");
	printf("version: %.2f\n", SCOURGE_VERSION);
	printf("\nOptions:\n");
	printf("\tf - disable fullscreen mode\n");
	printf("\td - disable double buffering\n");
	printf("\tp - disable hardware palette\n");
	printf("\tr - disable resizable window\n");
	printf("\tH - force hardware surface\n");
	printf("\tS - force software surface\n");
	printf("\ta - disable hardware acceleration\n");
	printf("\th,?,--help - show this info\n");
	printf("\ts - disable stencil buffer\n");
	printf("\tm - disable multitexturing\n");
	printf("\t--test - list card's supported video modes\n");
	printf("\t--version - print the build version\n");
	printf("\t--bppXX - use XX bits per pixel (8,15,16,24,32)\n");
	printf("\t--widthXX - use XX pixels for the screen width\n");
	printf("\t--heightXX - use XX pixels for the screen height\n");
    printf("\t--shadowX - shadow's cast by: 0-nothing, 1-objects and creatures, 2-everything\n");
	printf("\nBy default (with no options):\n\tbpp is the highest possible value\n\tfullscreen mode is on\n\tdouble buffering is on\n\thwpal is used if available\n\tresizeable is on (no effect in fullscreen mode)\n\thardware surface is used if available\n\thardware acceleration is used if available\n\tstencil buffer is used if available\n\tmultitexturing is used if available\n\tshadows are cast by everything.\n\n");
	exit(0);
  }

  /* this holds some info about our display */
  const SDL_VideoInfo *videoInfo;
  
  /* initialize SDL */
  if ( SDL_Init( SDL_INIT_VIDEO ) < 0 ) {
	fprintf( stderr, "Video initialization failed: %s\n", SDL_GetError( ) );
	quit( 1 );
  }
  
  /* Fetch the video info */
  videoInfo = SDL_GetVideoInfo( );
  
  if ( !videoInfo ) {
	fprintf( stderr, "Video query failed: %s\n", SDL_GetError( ) );
	quit( 1 );
  }
  
  /* the flags to pass to SDL_SetVideoMode */
  videoFlags  = SDL_OPENGL;          /* Enable OpenGL in SDL */
  if(doublebuf)
	videoFlags |= SDL_GL_DOUBLEBUFFER; /* Enable double buffering */
  if(hwpal)
	videoFlags |= SDL_HWPALETTE;       /* Store the palette in hardware */
  if(resizeable)
	videoFlags |= SDL_RESIZABLE;       /* Enable window resizing */
  
  /* This checks to see if surfaces can be stored in memory */
  if(force_hwsurf) videoFlags |= SDL_HWSURFACE;
  else if(force_swsurf) videoFlags |= SDL_SWSURFACE;
  else {
	if ( videoInfo->hw_available ) videoFlags |= SDL_HWSURFACE;
	else videoFlags |= SDL_SWSURFACE;
  }
  
  /* This checks if hardware blits can be done */
  if ( hwaccel && videoInfo->blit_hw ) videoFlags |= SDL_HWACCEL;
  
  if(fullscreen) videoFlags |= SDL_FULLSCREEN;

  if(test) {
	testModes(videoFlags);
	quit(0);
  }

  // try to find the highest bpp for this mode
  if(bpp == -1) bpp = testModes(videoFlags, true);
  if(bpp == -1) {
	fprintf(stderr, "Could not detect suitable opengl video mode.\n");
	fprintf(stderr, "You can manually select one with the -bpp option\n");
	quit(0);
  }
  
  /* Sets up OpenGL double buffering */
  if(doublebuf)
	SDL_GL_SetAttribute( SDL_GL_DOUBLEBUFFER, 1 );
  if(Constants::stencilbuffer) SDL_GL_SetAttribute( SDL_GL_STENCIL_SIZE, 8 );

  cout << "Setting video mode: " << w << "x" << h << "x" << bpp << endl;
  
  /* get a SDL surface */
  screen = SDL_SetVideoMode( w, h, bpp, videoFlags );
  /* Verify there is a surface */
  if ( !screen ) {
	fprintf( stderr,  "Video mode set failed: %s\n", SDL_GetError( ) );
	quit( 1 );
  }

  /* hide the mouse cursor; we have our own */
  SDL_ShowCursor(SDL_DISABLE);
  SDL_WM_SetCaption("Scourge", NULL);
  
  /* for Mac OS X in windowed mode, invert the mouse. SDL bug. */
#ifdef __APPLE__
  if(!fullscreen)
	invertMouse = true;
#endif	 
  
  /* initialize OpenGL */
  initGL( );
  
  /* resize the initial window */
  resizeWindow( screen->w, screen->h );
}

void SDLHandler::fireEvent(Widget *widget, SDL_Event *event) {
  storedWidget = widget;
  storedEvent = event;
}

bool SDLHandler::firedEventWaiting() {
  return storedEvent != NULL;
}

void SDLHandler::mainLoop() {
  /* whether or not the window is active */
  int isActive = TRUE;  
  SDL_Event event;
  while(true) {    
	int eventCount = 0;  
    while(SDL_PollEvent(&event) && (eventCount++) < 10) {
	  mouseEvent = mouseButton = 0;
	  Widget *widget = NULL;
      switch( event.type ) {
	  case SDL_MOUSEMOTION:
		if(invertMouse) event.motion.y = screen->h - event.motion.y;
		mouseX = event.motion.x;
		mouseY = event.motion.y;          
		mouseButton = event.button.button;
		mouseEvent = SDL_MOUSEMOTION;
		widget = Window::delegateEvent( &event, mouseX, mouseY );
		break;
      case SDL_MOUSEBUTTONUP:
		if(invertMouse) event.button.y = screen->h - event.button.y;
		mouseEvent = SDL_MOUSEBUTTONUP;
		mouseButton = event.button.button;
		mouseDragging = false;
		widget = Window::delegateEvent( &event, event.button.x, event.button.y );
		break;
      case SDL_MOUSEBUTTONDOWN:
		if(invertMouse) event.button.y = screen->h - event.button.y;			 
		mouseEvent = SDL_MOUSEBUTTONDOWN;
		mouseButton = event.button.button;
		mouseDragging = true;
		widget = Window::delegateEvent( &event, event.button.x, event.button.y );
		break;
      case SDL_ACTIVEEVENT:
		/* Something's happend with our focus
		 * If we lost focus or we are iconified, we
		 * shouldn't draw the screen
		 */
		if ( event.active.gain == 0 )
		  isActive = FALSE;
		else
		  isActive = TRUE;
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
	  case SDL_KEYDOWN:
		switch(event.key.keysym.sym) {
		case SDLK_F1:
		  SDL_WM_ToggleFullScreen(screen);
		  break;
		default:
		  break;
		}
		break;
	  case SDL_QUIT:
		/* handle quit requests */
		quit(0);
		break;
	  default:
		break;
      }
	  
	  bool res = false;
	  if(widget) {
		res = eventHandler->handleEvent(widget, &event);
	  } else {
		res = eventHandler->handleEvent(&event);
	  }
      if(res) {
		if(popHandlers()) return;
      }
    }
	
    if(isActive) {
	  screenView->drawView(screen);

	  // redraw the gui
	  Window::drawVisibleWindows();
	  	  
      if(shapePal->cursorImage) {
        // for cursor: do alpha bit testing
        glDisable(GL_TEXTURE_2D);
        glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
        glDisable(GL_DEPTH_TEST);
        glEnable(GL_ALPHA_TEST);
        glAlphaFunc(GL_NOTEQUAL, 0);        
        glLoadIdentity( );                         
        glPixelZoom( 1.0, -1.0 );
        glRasterPos2f( (float)mouseX, (float)mouseY );
		glDrawPixels(shapePal->cursor->w, shapePal->cursor->h,
					 GL_BGRA, GL_UNSIGNED_BYTE, shapePal->cursorImage);
		
        //glDrawPixels(shapePal->cursor->w, shapePal->cursor->h,
        //             GL_BGR, GL_UNSIGNED_BYTE, shapePal->cursor->pixels);
        glDisable(GL_ALPHA_TEST);
        glEnable(GL_DEPTH_TEST);
        glEnable(GL_TEXTURE_2D);
      }
    }
	
#ifdef SHOW_DEBUG_INFO
	glDisable(GL_TEXTURE_2D);
	glDisable(GL_DEPTH_TEST);
	glColor4f( 0.8f, 0.7f, 0.2f, 1.0f );
	texPrint(700, 10, "FPS: %g", getFPS());
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_TEXTURE_2D);
#endif
	
	/* Draw it to the screen */
    SDL_GL_SwapBuffers( );
    

    /* Gather our frames per second */
    Frames++;
    {
	  GLint t = SDL_GetTicks();
	  if (t - T0 >= 5000) {
        GLfloat seconds = (t - T0) / 1000.0;
  	    fps = Frames / seconds;
  	    //printf("%d frames in %g seconds = %g FPS\n", Frames, seconds, fps);
  	    T0 = t;
  	    Frames = 0;
	  }
    }
  }
}


void print_bitmap_string(void* font, char* s) {
   if (s && strlen(s)) {
      while (*s) {
         glutBitmapCharacter(font, *s);
         s++;
      }
   }
}

void SDLHandler::texPrint(GLfloat x, GLfloat y, 
						  const char *fmt, ...) {
  
  if(!text) text = new TexturedText();

  char str[256]; // Holds our string
  va_list ap;     // Pointer to our list of elements

  // If there's no text, do nothing
  if ( fmt == NULL ) return;

  // Parses The String For Variables
  va_start( ap, fmt );

  // Converts Symbols To Actual Numbers
  vsprintf( str, fmt, ap );
  va_end( ap );

  glPushMatrix();


  glRasterPos2f(x, y);
  print_bitmap_string(GLUT_BITMAP_HELVETICA_12, str);

/*
  glLoadIdentity();
  glTranslatef(x, y, 0);
  glScalef(1 / 3.0, 1 / 3.0, 1);

  glEnable( GL_TEXTURE_2D );
  glEnable( GL_LIGHTING );
  glEnable( GL_LIGHT2 );
  glEnable(GL_ALPHA_TEST);
  //  glAlphaFunc(GL_NOTEQUAL, 0);
  glAlphaFunc(GL_GEQUAL, 0.1);

  text->txfEstablishTexture(0, GL_TRUE);
  text->txfRenderString(str, strlen(str));

  glDisable(GL_ALPHA_TEST);
  glDisable( GL_LIGHT2 );
*/
  glPopMatrix();
}

bool SDLHandler::sectionIntersects(int a1, int a2, int b1, int b2) {
  return(((a1 <= b1 && a2 > b1) || (a1 >= b1 && a1 < b2))
        ? true : false);
}

bool SDLHandler::intersects(SDL_Rect *a, SDL_Rect *b) {
  return((sectionIntersects(a->x, a->x + a->w, b->x, b->x + b->w) &&
    		 sectionIntersects(a->y, a->y + a->h, b->y, b->y + b->h))
        ? true : false);
}

void SDLHandler::testDrawView() {
   /* Clear The Screen And The Depth Buffer */
    glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );

    /* Move Right 3 Units */
    glLoadIdentity( );
    glTranslatef( 1.5f, 0.0f, -6.0f );

    /* Rotate The Quad On The X axis ( NEW ) */
    glRotatef( rquad, 1.0f, 0.0f, 0.0f );

    /* Set The Color To Blue One Time Only */
    glColor3f( 0.5f, 0.5f, 1.0f);

    glBegin( GL_QUADS );                 /* Draw A Quad                      */
      glColor3f(   0.0f,  1.0f,  0.0f ); /* Set The Color To Green           */
      glVertex3f(  1.0f,  1.0f, -1.0f ); /* Top Right Of The Quad (Top)      */
      glVertex3f( -1.0f,  1.0f, -1.0f ); /* Top Left Of The Quad (Top)       */
      glVertex3f( -1.0f,  1.0f,  1.0f ); /* Bottom Left Of The Quad (Top)    */
      glVertex3f(  1.0f,  1.0f,  1.0f ); /* Bottom Right Of The Quad (Top)   */

      glColor3f(   1.0f,  0.5f,  0.0f ); /* Set The Color To Orange          */
      glVertex3f(  1.0f, -1.0f,  1.0f ); /* Top Right Of The Quad (Botm)     */
      glVertex3f( -1.0f, -1.0f,  1.0f ); /* Top Left Of The Quad (Botm)      */
      glVertex3f( -1.0f, -1.0f, -1.0f ); /* Bottom Left Of The Quad (Botm)   */
      glVertex3f(  1.0f, -1.0f, -1.0f ); /* Bottom Right Of The Quad (Botm)  */

      glColor3f(   1.0f,  0.0f,  0.0f ); /* Set The Color To Red             */
      glVertex3f(  1.0f,  1.0f,  1.0f ); /* Top Right Of The Quad (Front)    */
      glVertex3f( -1.0f,  1.0f,  1.0f ); /* Top Left Of The Quad (Front)     */
      glVertex3f( -1.0f, -1.0f,  1.0f ); /* Bottom Left Of The Quad (Front)  */
      glVertex3f(  1.0f, -1.0f,  1.0f ); /* Bottom Right Of The Quad (Front) */

      glColor3f(   1.0f,  1.0f,  0.0f ); /* Set The Color To Yellow          */
      glVertex3f(  1.0f, -1.0f, -1.0f ); /* Bottom Left Of The Quad (Back)   */
      glVertex3f( -1.0f, -1.0f, -1.0f ); /* Bottom Right Of The Quad (Back)  */
      glVertex3f( -1.0f,  1.0f, -1.0f ); /* Top Right Of The Quad (Back)     */
      glVertex3f(  1.0f,  1.0f, -1.0f ); /* Top Left Of The Quad (Back)      */

      glColor3f(   0.0f,  0.0f,  1.0f ); /* Set The Color To Blue            */
      glVertex3f( -1.0f,  1.0f,  1.0f ); /* Top Right Of The Quad (Left)     */
      glVertex3f( -1.0f,  1.0f, -1.0f ); /* Top Left Of The Quad (Left)      */
      glVertex3f( -1.0f, -1.0f, -1.0f ); /* Bottom Left Of The Quad (Left)   */
      glVertex3f( -1.0f, -1.0f,  1.0f ); /* Bottom Right Of The Quad (Left)  */

      glColor3f(   1.0f,  0.0f,  1.0f ); /* Set The Color To Violet          */
      glVertex3f(  1.0f,  1.0f, -1.0f ); /* Top Right Of The Quad (Right)    */
      glVertex3f(  1.0f,  1.0f,  1.0f ); /* Top Left Of The Quad (Right)     */
      glVertex3f(  1.0f, -1.0f,  1.0f ); /* Bottom Left Of The Quad (Right)  */
      glVertex3f(  1.0f, -1.0f, -1.0f ); /* Bottom Right Of The Quad (Right) */
    glEnd( );                            /* Done Drawing The Quad            */

    /* Move Left 1.5 Units And Into The Screen 6.0 */
    glLoadIdentity();
    glTranslatef( 0.5f, 0.0f, -8.0f );

    /* Rotate The Triangle On The Y axis ( NEW ) */
    glRotatef( rtri, 0.0f, 1.0f, 0.0f );

    glBegin( GL_TRIANGLES );             /* Drawing Using Triangles       */
      glColor3f(   1.0f,  0.0f,  0.0f ); /* Red                           */
      glVertex3f(  0.0f,  1.0f,  0.0f ); /* Top Of Triangle (Front)       */
      glColor3f(   0.0f,  1.0f,  0.0f ); /* Green                         */
      glVertex3f( -1.0f, -1.0f,  1.0f ); /* Left Of Triangle (Front)      */
      glColor3f(   0.0f,  0.0f,  1.0f ); /* Blue                          */
      glVertex3f(  1.0f, -1.0f,  1.0f ); /* Right Of Triangle (Front)     */

      glColor3f(   1.0f,  0.0f,  0.0f ); /* Red                           */
      glVertex3f(  0.0f,  1.0f,  0.0f ); /* Top Of Triangle (Right)       */
      glColor3f(   0.0f,  0.0f,  1.0f ); /* Blue                          */
      glVertex3f(  1.0f, -1.0f,  1.0f ); /* Left Of Triangle (Right)      */
      glColor3f(   0.0f,  1.0f,  0.0f ); /* Green                         */
      glVertex3f(  1.0f, -1.0f, -1.0f ); /* Right Of Triangle (Right)     */

      glColor3f(   1.0f,  0.0f,  0.0f ); /* Red                           */
      glVertex3f(  0.0f,  1.0f,  0.0f ); /* Top Of Triangle (Back)        */
      glColor3f(   0.0f,  1.0f,  0.0f ); /* Green                         */
      glVertex3f(  1.0f, -1.0f, -1.0f ); /* Left Of Triangle (Back)       */
      glColor3f(   0.0f,  0.0f,  1.0f ); /* Blue                          */
      glVertex3f( -1.0f, -1.0f, -1.0f ); /* Right Of Triangle (Back)      */

      glColor3f(   1.0f,  0.0f,  0.0f ); /* Red                           */
      glVertex3f(  0.0f,  1.0f,  0.0f ); /* Top Of Triangle (Left)        */
      glColor3f(   0.0f,  0.0f,  1.0f ); /* Blue                          */
      glVertex3f( -1.0f, -1.0f, -1.0f ); /* Left Of Triangle (Left)       */
      glColor3f(   0.0f,  1.0f,  0.0f ); /* Green                         */
      glVertex3f( -1.0f, -1.0f,  1.0f ); /* Right Of Triangle (Left)      */
    glEnd( );                            /* Finished Drawing The Triangle */   

    /* Increase The Rotation Variable For The Triangle ( NEW ) */
    rtri  += 0.2f;
    /* Decrease The Rotation Variable For The Quad     ( NEW ) */
    rquad -=0.15f;
}


