
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
    fprintf(stderr, "BEFORE: glSDLActiveTextureARB=%u\n", glSDLActiveTextureARB);
    glSDLActiveTextureARB = 
      (PFNGLACTIVETEXTUREARBPROC)SDL_GL_GetProcAddress ("glActiveTextureARB");
    fprintf(stderr, "AFTER: glSDLActiveTextureARB=%u\n", glSDLActiveTextureARB);
    glSDLMultiTexCoord2fARB = 
      (PFNGLMULTITEXCOORD2FARBPROC)SDL_GL_GetProcAddress ("glMultiTexCoord2fARB");
    glSDLMultiTexCoord2iARB = 
      (PFNGLMULTITEXCOORD2IARBPROC)SDL_GL_GetProcAddress ("glMultiTexCoord2iARB");

    return( TRUE );
}

void SDLHandler::setVideoMode(int w, int h, int bpp, bool fullscreen) {
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
    videoFlags |= SDL_GL_DOUBLEBUFFER; /* Enable double buffering */
    videoFlags |= SDL_HWPALETTE;       /* Store the palette in hardware */
    videoFlags |= SDL_RESIZABLE;       /* Enable window resizing */

    /* This checks to see if surfaces can be stored in memory */
    if ( videoInfo->hw_available ) videoFlags |= SDL_HWSURFACE;
    else videoFlags |= SDL_SWSURFACE;

    /* This checks if hardware blits can be done */
    if ( videoInfo->blit_hw ) videoFlags |= SDL_HWACCEL;

    if(fullscreen) videoFlags |= SDL_FULLSCREEN;

    /* Sets up OpenGL double buffering */
    SDL_GL_SetAttribute( SDL_GL_DOUBLEBUFFER, 1 );
    SDL_GL_SetAttribute( SDL_GL_STENCIL_SIZE, 8 );

    /* get a SDL surface */
    screen = SDL_SetVideoMode( w, h, bpp, videoFlags );
    
    int value;
    SDL_GL_GetAttribute(SDL_GL_STENCIL_SIZE , &value);
    fprintf(stderr,"SDL_GL_STENCIL_SIZE = %d\n",value);

    /* hide the mouse cursor; we have our own */
    SDL_ShowCursor(SDL_DISABLE);
    SDL_WM_SetCaption("Scourge", NULL);

    /* Verify there is a surface */
    if ( !screen ) {
	    fprintf( stderr,  "Video mode set failed: %s\n", SDL_GetError( ) );
	    quit( 1 );
    }
	 
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

void SDLHandler::mainLoop() {
  /* whether or not the window is active */
  int isActive = TRUE;  
  SDL_Event event;
  while(true) {    
	mouseEvent = mouseButton = 0;
	int eventCount = 0;  
    while(SDL_PollEvent(&event) && (eventCount++) < 10) {
      switch( event.type ) {
        case SDL_MOUSEMOTION:
				if(invertMouse) event.motion.y = screen->h - event.motion.y;
          mouseX = event.motion.x;
				mouseY = event.motion.y;          
          mouseEvent = SDL_MOUSEMOTION;
          break;
      case SDL_MOUSEBUTTONUP:
			 if(invertMouse) event.button.y = screen->h - event.button.y;
          mouseEvent = SDL_MOUSEBUTTONUP;
          mouseButton = event.button.button;
          mouseDragging = false;
          break;
      case SDL_MOUSEBUTTONDOWN:
			 if(invertMouse) event.button.y = screen->h - event.button.y;			 
          mouseEvent = SDL_MOUSEBUTTONDOWN;
          mouseButton = event.button.button;
          mouseDragging = true;
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
      if(eventHandler->handleEvent(&event)) {
          if(popHandlers()) return;
      }
    }

    if(isActive) {
	  screenView->drawView(screen);
	  /*
      // if cursor doesn't display, disable depth testing, lights, etc.
      glPushMatrix();
	  glLoadIdentity();

      float w = 48.0f;
      float h = 48.0f;
	  glDisable(GL_LIGHTING);
	  glDisable(GL_DEPTH_TEST);
	  glTranslatef((float)mouseX, (float)mouseY, 0.0f);

	  glEnable( GL_BLEND );
	  glBlendFunc( GL_SRC_COLOR, GL_SRC_ALPHA );

      glBindTexture( GL_TEXTURE_2D, shapePal->getCursorTex() );
      glColor4f(1.0f, 1.0f, 1.0f, 1.0f);      
      glBegin( GL_QUADS );
        // front
        //glNormal3f(0.0f, 1.0f, 0.0f);
        glTexCoord2f( 0, 0 );
        glVertex2f(0, 0);
        glTexCoord2f( 1, 0 );
        glVertex2f(w, 0);
        glTexCoord2f( 1, 1 );
        glVertex2f(w, h);
        glTexCoord2f( 0, 1 );
        glVertex2f(0, h);
      glEnd();
	  glDisable(GL_BLEND);
	  glEnable(GL_DEPTH_TEST);
	  glEnable( GL_LIGHTING );
      // reset the model_view matrix
      glPopMatrix();
	  */

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


