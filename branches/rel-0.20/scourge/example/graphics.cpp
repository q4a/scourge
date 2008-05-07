/***************************************************************************
                          graphics.cpp  -  description
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

/* 
  Load the Scourge constants, include sdl, opengl, etc.                      
*/
#include "graphics.h"

Graphics::Graphics() {
  stencilBufferUsed = false; 
  lastWidth=0, lastHeight=0;
  screen = NULL;
  T0=0;
  Frames=0;
  fps=0;
  videoFlags=0;
  mouseX = mouseY = 0;
  mouseMovingOverMap = false;
}

Graphics::~Graphics() {
}

/* function to release/destroy our resources and restoring the old desktop */
void Graphics::quit( int returnCode ) {

  cerr << "Quitting." << endl;

  /* clean up the window */
  SDL_Quit( );
  
  /* and exit appropriately */
  exit( returnCode );
}

void Graphics::setPerspective() {
  //GLfloat ratio = ( GLfloat )lastWidth / ( GLfloat )lastHeight;
  //gluPerspective( 45.0f, ratio, 0.1f, 100.0f );
  glOrtho(0.0f, lastWidth, lastHeight, 0.0f, -1000.0f, 1000.0f);
}

/* function to reset our viewport after a window resize */
void Graphics::resizeWindow( int width, int height ) {
  // Protect against a divide by zero
  if ( height == 0 ) height = 1;
  lastWidth = width;
  lastHeight = height;
  
  // Setup our viewport.
  glViewport( 0, 0, ( GLsizei )width, ( GLsizei )height );
  
  // change to the projection matrix and set our viewing volume.
  glMatrixMode( GL_PROJECTION );
  glLoadIdentity( );
  
  // Set our perspective
  setPerspective();
  
  // Make sure we're chaning the model view and not the projection 
  glMatrixMode( GL_MODELVIEW );
  
  // Reset The View 
  glLoadIdentity( );
  
  return;
}

/* general OpenGL initialization function */
void Graphics::initGL( GLvoid ) {
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
  if( stencilBufferUsed ) glClearStencil(0); // Clear The Stencil Buffer To 0

  /* Enables Depth Testing */
  glEnable( GL_DEPTH_TEST );

  /* The Type Of Depth Test To Do */
  glDepthFunc( GL_LEQUAL );

  /* Really Nice Perspective Calculations */
  // glHint( GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST );

  glColorMaterial ( GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE ) ;
  glEnable( GL_COLOR_MATERIAL );

  /* initialize opengl extensions */
  if( Constants::multitexture ) {
    //	  fprintf(stderr, "BEFORE: glSDLActiveTextureARB=%u\n", glSDLActiveTextureARB);
    glSDLActiveTextureARB = 
      (PFNGLACTIVETEXTUREARBPROC)SDL_GL_GetProcAddress ("glActiveTextureARB");
    //	  fprintf(stderr, "AFTER: glSDLActiveTextureARB=%u\n", glSDLActiveTextureARB);
    glSDLMultiTexCoord2fARB = 
      (PFNGLMULTITEXCOORD2FARBPROC)SDL_GL_GetProcAddress ("glMultiTexCoord2fARB");
    glSDLMultiTexCoord2iARB = 
      (PFNGLMULTITEXCOORD2IARBPROC)SDL_GL_GetProcAddress ("glMultiTexCoord2iARB");
  }

  return;
}

bool Graphics::testModesInFormat( SDL_PixelFormat *format, Uint32 flags ) {
  SDL_Rect **modes;
  int i;

  printf( "Available modes with given flags in %d bpp:\n", format->BitsPerPixel );

  /* Get available fullscreen/hardware modes */
  modes = SDL_ListModes( format, flags );
  
  /* Check is there are any modes available */
  if( modes == (SDL_Rect **)0 ){
    printf( "\tNo modes available!\n" );
    return false;
  }
  
  /* Check if our resolution is restricted */
  if( modes == (SDL_Rect **)-1 ){
    printf( "\tAll resolutions available.\n" );
    return true;
  }

  /* Print valid modes */
  for( i = 0; modes[i]; ++i )
    printf( "\t%d x %d\n", modes[i]->w, modes[i]->h );
  
  //free(modes); // crashes; ok to not free since we only do this a few times
  return true;
}

int Graphics::testModes( Uint32 flags, bool findMaxBpp ) {
  int bpp[] = { 32, 24, 16, 15, 8, 0 };
  SDL_PixelFormat format;
  for( int i = 0; bpp[i]; i++ ) {
    format.BitsPerPixel=bpp[i];
    if( testModesInFormat( &format, flags ) && findMaxBpp ) return bpp[ i ];
  }
  return -1;
}

void Graphics::setVideoMode( Preferences * uc ) {  
  /* this holds some info about our display */
  const SDL_VideoInfo *videoInfo;
  
  /* initialize SDL */
  if( SDL_Init( SDL_INIT_VIDEO ) < 0 ) {
    fprintf( stderr, "Video initialization failed: %s\n", SDL_GetError( ) );
    quit( 1 );
  }
  
  /* Fetch the video info */
  videoInfo = SDL_GetVideoInfo( );
  
  if( !videoInfo ) {
    fprintf( stderr, "Video query failed: %s\n", SDL_GetError( ) );
    quit( 1 );
  }
  
  /* the flags to pass to SDL_SetVideoMode */
  videoFlags  = SDL_OPENGL;          /* Enable OpenGL in SDL */
  if(uc->getDoublebuf())
	videoFlags |= SDL_GL_DOUBLEBUFFER; /* Enable double buffering */
  if(uc->getHwpal())
	videoFlags |= SDL_HWPALETTE;       /* Store the palette in hardware */
  if(uc->getResizeable())
	videoFlags |= SDL_RESIZABLE;       /* Enable window resizing */
  
  /* This checks to see if surfaces can be stored in memory */
  if( uc->getForce_hwsurf() ) videoFlags |= SDL_HWSURFACE;
  else if(uc->getForce_swsurf() ) videoFlags |= SDL_SWSURFACE;
  else {
    if ( videoInfo->hw_available ) videoFlags |= SDL_HWSURFACE;
    else videoFlags |= SDL_SWSURFACE;
  }
  
  /* This checks if hardware blits can be done */
  if( uc->getHwaccel() && videoInfo->blit_hw ) videoFlags |= SDL_HWACCEL;
  
  if( uc->getFullscreen() ) videoFlags |= SDL_FULLSCREEN;
  
  // try to find the highest bpp for this mode
  int bpp;
  if( uc->getBpp() == -1 ) {
    bpp = testModes( videoFlags, true );
    if( bpp == -1 ) {
    	fprintf( stderr, "Could not detect suitable opengl video mode.\n" );
      fprintf( stderr, "You can manually select one with the -bpp option\n" );
    	quit( 0 );
    } else{
      uc->setBpp( bpp );
    }
  }
  
  /* Sets up OpenGL double buffering */
  if( uc->getDoublebuf() ) 
    SDL_GL_SetAttribute( SDL_GL_DOUBLEBUFFER, 1 );
  if( uc->getStencilbuf() ) {
    uc->setStencilBufInitialized( true );
    stencilBufferUsed = true;
    SDL_GL_SetAttribute( SDL_GL_STENCIL_SIZE, 8 );
  }

  cout << "Setting video mode: " << 
    uc->getW() << "x" << 
    uc->getH() << "x" << 
    uc->getBpp() << endl;
  
  /* get a SDL surface */
  screen = SDL_SetVideoMode( uc->getW(), uc->getH(), uc->getBpp(), videoFlags );
  
  /* Verify there is a surface */
  if( !screen ) {
    fprintf( stderr,  "Video mode set failed: %s\n", SDL_GetError( ) );
    quit( 1 );
  }

  /* hide the mouse cursor if you have your own */
  //SDL_ShowCursor( SDL_DISABLE );
  SDL_WM_SetCaption( "Scourge Example App", NULL );
    
  /* initialize OpenGL */
  initGL( );
  
  /* resize the initial window */
  resizeWindow( screen->w, screen->h );
}

void Graphics::drawScreen( Game *game ) {
  glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
  if( stencilBufferUsed ) glClear( GL_STENCIL_BUFFER_BIT );
  glClearColor( 0.0f, 0.0f, 0.0f, 0.5f );
  glClearDepth( 1.0f );

  // draw the game
  game->drawView();

  /* Draw it to the screen */
  SDL_GL_SwapBuffers( );
    
  /* Gather our frames per second */
  Frames++;
  {
    GLint t = SDL_GetTicks();
    if( t - T0 >= 2500 ) {
      GLfloat seconds = ( t - T0 ) / 1000.0;
      fps = Frames / seconds;
      //printf("%d frames in %g seconds = %g FPS\n", Frames, seconds, fps);
      T0 = t;
      Frames = 0;
    }
  }
}

void Graphics::mainLoop( Game *game ) {

  cerr << "Starting main loop" << endl;

  /* whether or not the window is active */
  bool isActive = true;  
  SDL_Event event;
  while(true) {    
    int eventCount = 0;
    mouseMovingOverMap = false;
    while( SDL_PollEvent( &event ) && 
           ( eventCount++ ) < 10 ) {
      switch( event.type ) {
      case SDL_MOUSEMOTION:
        mouseX = event.motion.x;
        mouseY = event.motion.y;
        mouseMovingOverMap = true;
        break;
      case SDL_ACTIVEEVENT:
        /* Something's happend with our focus
         * If we lost focus or we are iconified, we
         * shouldn't draw the screen
         */
        isActive = ( event.active.gain == 0 ? false : true );
        break;
      case SDL_VIDEORESIZE:
        /* handle resize event */
        screen = SDL_SetVideoMode( event.resize.w,
                                   event.resize.h,
                                   16, videoFlags );
        if( !screen ) {
          fprintf( stderr, "Could not get a surface after resize: %s\n", SDL_GetError( ) );
          quit( 1 );
        }
        resizeWindow( event.resize.w, event.resize.h );
        break;
      case SDL_KEYUP:
        // only process CTRL + F1 once (on keyup)
        if( event.key.keysym.sym == SDLK_F1 &&
            event.key.keysym.mod & KMOD_CTRL ) {
          SDL_WM_ToggleFullScreen(screen);
          break;
        } else if( event.key.keysym.sym == SDLK_ESCAPE ) {
          cerr << "Hit escape." << endl;
          quit( 0 );
        }
        break;
      case SDL_QUIT:
        cerr << "SDL_QUIT event." << endl;
        quit( 0 ); // handle quit requests
        break;
      default:
        break;
      }

      // let the game handle the event:
      game->handleEvent( &event );
    }

    // draw the screen
    if(isActive) drawScreen( game );
  }

  cerr << "Ending main loop" << endl;
}


