
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

bool SDLHandler::stencilBufferUsed = false;

SDLHandler::SDLHandler(ShapePalette *shapePal){
  /* These are to calculate our fps */
  this->shapePal = shapePal;
  T0     = 0;
  Frames = 0;                       
  fps = 0;
  screen = NULL;
  mouseX = mouseY = mouseButton = mouseEvent = 0;
  mouseDragging = false;
  text = NULL;
  handlerCount = 0;
  invertMouse = false; 
  cursorMode = CURSOR_NORMAL;
  font_initialized = false;
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

#ifdef HAVE_SDL_NET
  // shutdown SDL_net
  SDLNet_Quit();
#endif

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
    if(stencilBufferUsed) glClearStencil(0); // Clear The Stencil Buffer To 0

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


char **SDLHandler::getVideoModes(int &nbModes){
    SDL_Rect **modes;
    char ** modesDescription;
    char temp [50];
    Uint32 flags;  
    int i;    
        
    if(!screen){
        fprintf(stderr, "SDLHandler :: you must allocate screen before calling getVideoModes!!\n");
        exit(-1);
    }
    
    // Get current video flags (hwsurface/swsurface, fullscreen/not fullscreen..)
    flags = screen->flags;    
        
    // Get available modes for the current flags    
    modes = SDL_ListModes(NULL, flags);    

    // Copy them to a char array
    if(modes != (SDL_Rect **)0){                    
        nbModes = 0;            
        if(modes == (SDL_Rect **)-1) {
            // All modes are available, so let's go..
            nbModes = 16;
            modesDescription = (char **) malloc (nbModes * sizeof(char *));
            modesDescription[0] = strdup("  320 x 200");
            modesDescription[1] = strdup("  320 x 240");
            modesDescription[2] = strdup("  400 x 300");
            modesDescription[3] = strdup("  512 x 384");
            modesDescription[4] = strdup("  640 x 480");
            modesDescription[5] = strdup("  800 x 600");
            modesDescription[6] = strdup("  848 x 480");
            modesDescription[7] = strdup(" 1024 x 768");
            modesDescription[8] = strdup(" 1152 x 864");
            modesDescription[9] = strdup(" 1280 x 720");
            modesDescription[10] = strdup(" 1280 x 768");
            modesDescription[11] = strdup(" 1280 x 960");
            modesDescription[12] = strdup("1280 x 1024");
            modesDescription[13] = strdup(" 1360 x 768");
            modesDescription[14] = strdup("1600 x 1024");
            modesDescription[15] = strdup("1600 x 1200");        
        }
        else{
            // Only a few modes available, which ones ?            
            for(nbModes = 0; modes[nbModes]; nbModes++);
            if(nbModes) {
              modesDescription = (char **)malloc(nbModes * sizeof(char *));
              for(i=0; i < nbModes; i++){
                sprintf(temp, "%d x %d", modes[i]->w, modes[i]->h);                
                modesDescription[i] = strdup(temp);
              }
           } else {
             nbModes = 1;           
             modesDescription = (char **) malloc (sizeof(char *));
             modesDescription[0] = strdup("No modes available!\n");              
           }  
        } 
    } else {         
        nbModes = 1;           
        modesDescription = (char **) malloc (sizeof(char *));
        modesDescription[0] = strdup("No modes available!\n");         
    }
    return modesDescription;
}

void SDLHandler::setVideoMode( UserConfiguration * uc ) {  
  
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
  if(uc->getDoublebuf())
	videoFlags |= SDL_GL_DOUBLEBUFFER; /* Enable double buffering */
  if(uc->getHwpal())
	videoFlags |= SDL_HWPALETTE;       /* Store the palette in hardware */
  if(uc->getResizeable())
	videoFlags |= SDL_RESIZABLE;       /* Enable window resizing */
  
  /* This checks to see if surfaces can be stored in memory */
  if(uc->getForce_hwsurf()) videoFlags |= SDL_HWSURFACE;
  else if(uc->getForce_swsurf()) videoFlags |= SDL_SWSURFACE;
  else {
	if ( videoInfo->hw_available ) videoFlags |= SDL_HWSURFACE;
	else videoFlags |= SDL_SWSURFACE;
  }
  
  /* This checks if hardware blits can be done */
  if ( uc->getHwaccel() && videoInfo->blit_hw ) videoFlags |= SDL_HWACCEL;
  
  if(uc->getFullscreen()) videoFlags |= SDL_FULLSCREEN;

  if(uc->getTest()) {
	testModes(videoFlags);
	quit(0);
  }

  // try to find the highest bpp for this mode
  int bpp;
  if(uc->getBpp() == -1) {
    bpp = testModes(videoFlags, true);
    if(bpp == -1) {
    	fprintf(stderr, "Could not detect suitable opengl video mode.\n");
    	fprintf(stderr, "You can manually select one with the -bpp option\n");
    	quit(0);
    }
    else{
        uc->setBpp(bpp);
    }
  }
  
  /* Sets up OpenGL double buffering */
  if(uc->getDoublebuf()) 
    SDL_GL_SetAttribute( SDL_GL_DOUBLEBUFFER, 1 );
  if(uc->getStencilbuf()) {
		uc->setStencilBufInitialized(true);
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
  SDL_ShowCursor(SDL_DISABLE);
  SDL_WM_SetCaption("Scourge", NULL);
  
  /* for Mac OS X in windowed mode, invert the mouse. SDL bug. */
#ifdef __APPLE__
  if(!uc->getFullscreen())
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
  int mx, my;
  while(true) {    
    int eventCount = 0;  
    while(SDL_PollEvent(&event) && (eventCount++) < 10) {
      mouseEvent = mouseButton = 0;
      Widget *widget = NULL;
      switch( event.type ) {
      case SDL_MOUSEMOTION:
        if(invertMouse) event.motion.y = screen->h - event.motion.y;
        //		applyMouseOffset(event.button.x, event.button.y, &mx, &my);
        mouseX = event.motion.x;
        mouseY = event.motion.y;          
        mouseButton = event.button.button;
        mouseEvent = SDL_MOUSEMOTION;
        widget = Window::delegateEvent( &event, event.button.x, event.button.y );
        break;
      case SDL_MOUSEBUTTONUP:
        if(invertMouse) event.button.y = screen->h - event.button.y;
        applyMouseOffset(event.button.x, event.button.y, &mx, &my);
        mouseEvent = SDL_MOUSEBUTTONUP;
        mouseButton = event.button.button;
        mouseDragging = false;
        widget = Window::delegateEvent( &event, mx, my );
        break;
      case SDL_MOUSEBUTTONDOWN:
        if(invertMouse) event.button.y = screen->h - event.button.y;
        applyMouseOffset(event.button.x, event.button.y, &mx, &my);
        mouseEvent = SDL_MOUSEBUTTONDOWN;
        mouseButton = event.button.button;
        mouseDragging = true;
        widget = Window::delegateEvent( &event, mx, my );
        break;
      case SDL_ACTIVEEVENT:
        /* Something's happend with our focus
         * If we lost focus or we are iconified, we
         * shouldn't draw the screen
         */
        if( event.active.gain == 0 )
          isActive = FALSE;
        else
          isActive = TRUE;
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
      // only process F1 once (on keyup)
      if(event.key.keysym.sym == SDLK_F1) {
        SDL_WM_ToggleFullScreen(screen);
        break;
      }
      case SDL_KEYDOWN:
      applyMouseOffset(mouseX, mouseY, &mx, &my);
      widget = Window::delegateEvent( &event, mx, my );
      break;
      case SDL_QUIT:
      quit(0); // handle quit requests
      break;
      default:
      break;
      }

      bool res = false;
      if(widget) {
        res = eventHandler->handleEvent(widget, &event);
        // this is so that moving the cursor over a 
        // window doesn't scroll the map forever
        if( event.type == SDL_MOUSEMOTION ) {
          res = eventHandler->handleEvent(&event);
        }
      } else {
        res = eventHandler->handleEvent(&event);
      }
      if(res) {
        if(popHandlers()) {
          return;
        }
      }
    }

    if(isActive) {
      drawScreen();
    }
  }
}

void SDLHandler::drawScreen() {
  glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
  if(stencilBufferUsed) glClear( GL_STENCIL_BUFFER_BIT );
  glClearColor( 0.0f, 0.0f, 0.0f, 0.5f );
  glClearDepth( 1.0f );

  screenView->drawView();

  // redraw the gui
  Window::drawVisibleWindows();

  screenView->drawAfter();

  if(shapePal->cursorImage) {
    // for cursor: do alpha bit testing
    glPushAttrib(GL_LIST_BIT | GL_CURRENT_BIT  | GL_ENABLE_BIT | GL_TRANSFORM_BIT);	
    //glMatrixMode(GL_MODELVIEW);

    /*
    glDisable(GL_TEXTURE_2D);
    glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
    glDisable(GL_DEPTH_TEST);
    glEnable(GL_ALPHA_TEST);
    glAlphaFunc(GL_NOTEQUAL, 0);        
    glPushMatrix();
    glLoadIdentity( );                         
    glPixelZoom( 1.0, -1.0 );
    glRasterPos2f( (float)mouseX, (float)mouseY );
    if(cursorMode == CURSOR_NORMAL) {
      glDrawPixels(shapePal->cursor->w, shapePal->cursor->h,
                   GL_BGRA, GL_UNSIGNED_BYTE, shapePal->cursorImage);
    } else if(cursorMode == CURSOR_CROSSHAIR) {
      glDrawPixels(shapePal->crosshair->w, shapePal->crosshair->h,
                   GL_BGRA, GL_UNSIGNED_BYTE, shapePal->crosshairImage);
    }
    */

    ///*
    glEnable( GL_ALPHA_TEST );
    glAlphaFunc( GL_EQUAL, 0xff );
    glEnable(GL_TEXTURE_2D);
    glDisable(GL_DEPTH_TEST);
    glDisable(GL_CULL_FACE);
    glPushMatrix();
    glLoadIdentity();
    glTranslatef( mouseX, mouseY, 0 );
    glBindTexture( GL_TEXTURE_2D, 
                   cursorMode == CURSOR_NORMAL ? 
                   shapePal->cursor_texture :
                   shapePal->crosshair_texture );
    glColor4f(1, 1, 1, 1);
    glBegin( GL_QUADS );
    glNormal3f( 0, 0, 1 );
    glTexCoord2f( 1, 1 );
    glVertex2f( shapePal->cursor->w, shapePal->cursor->h );
    glTexCoord2f( 0, 1 );
    glVertex2f( 0, shapePal->cursor->h );
    glTexCoord2f( 0, 0 );
    glVertex2f( 0, 0 );
    glTexCoord2f( 1, 0 );
    glVertex2f( shapePal->cursor->w, 0 );
    glEnd();
    //*/

    glPopMatrix();
    glPopAttrib();		
  }


#ifdef SHOW_DEBUG_INFO
  glPushMatrix();
  glLoadIdentity();
  glColor4f( 0.8f, 0.7f, 0.2f, 1.0f );
  texPrint(700, 10, "FPS: %g", getFPS());
  glPopMatrix();
#endif

  /* Draw it to the screen */
  SDL_GL_SwapBuffers( );
  
  
  /* Gather our frames per second */
  Frames++;
  {
    GLint t = SDL_GetTicks();
    if(t - T0 >= 5000) {
      GLfloat seconds = (t - T0) / 1000.0;
      fps = Frames / seconds;
      //printf("%d frames in %g seconds = %g FPS\n", Frames, seconds, fps);
      T0 = t;
      Frames = 0;
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

  initFonts();
  
  freetype_print_simple(font, x, y, str);
}

void SDLHandler::texPrintMono(GLfloat x, GLfloat y, 
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

  initFonts();
  
  freetype_print_simple(monoFont, x, y, str);
}

void SDLHandler::initFonts() {
  if(!font_initialized) {
    char s[200];
    sprintf(s, "%s/Vera.ttf", rootDir);
    font.init(s, 8);
    sprintf(s, "%s/VeraMono.ttf", rootDir);
    monoFont.init(s, 8);
    font_initialized = true;
  }
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


