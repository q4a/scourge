/***************************************************************************
                          mainmenu.cpp  -  description
                             -------------------
    begin                : Tue Aug 12 2003
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

#include "mainmenu.h"

// good for debugging blending
int MainMenu::blendA = 2;
int MainMenu::blendB = 6;     // 3
int MainMenu::blend[] = { 
    GL_ZERO, GL_ONE, GL_SRC_COLOR, GL_ONE_MINUS_SRC_COLOR, 
    GL_DST_COLOR, GL_ONE_MINUS_DST_COLOR, GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA,
    GL_DST_ALPHA, GL_ONE_MINUS_DST_ALPHA, GL_SRC_ALPHA_SATURATE
};

void MainMenu::setBlendFunc() {
    glBlendFunc(blend[blendA], blend[blendB]);
}

MainMenu::MainMenu(Scourge *scourge){
  this->scourge = scourge;
  this->showDebug = false;
  this->win = -1;  
  this->cloudCount = 30;
  for(int i = 0; i < cloudCount; i++) {
	cloud[i].x = (int)((float)scourge->getSDLHandler()->getScreen()->w * rand()/RAND_MAX);
	cloud[i].y = (int)(50.0 * rand()/RAND_MAX);
	cloud[i].w = (int)(55.0 * rand()/RAND_MAX) + 200;
	cloud[i].h = (int)(28.0 * rand()/RAND_MAX) + 100;
	cloud[i].speed = (int)(2.0 * rand()/RAND_MAX) + 1;
  }
}
MainMenu::~MainMenu(){
}

void MainMenu::init() {
    if(win == -1) {
	  win = scourge->getGui()->addWindow(50, 230,
										 270,
										 170,
										 &Gui::drawMainMenu);
	  scourge->getGui()->addActiveRegion(60, 245, 310, 265, Constants::MENU_0, this);
	  scourge->getGui()->addActiveRegion(60, 265, 310, 285, Constants::MENU_1, this);
	  scourge->getGui()->addActiveRegion(60, 285, 310, 305, Constants::MENU_2, this);
	  scourge->getGui()->addActiveRegion(60, 305, 310, 325, Constants::MENU_3, this);
	  scourge->getGui()->addActiveRegion(60, 325, 310, 345, Constants::MENU_4, this);

	  // The new style gui (testing for now)
	  mainWin = new Window( scourge->getSDLHandler(),
							350, 230, 270, 270, 
							"Main Menu (Testing new UI)", 
							scourge->getShapePalette()->getGuiTexture() );
	  
	  char version[100];
	  sprintf(version, "Scourge version %7.2f", SCOURGE_VERSION);
	  Label *label = new Label( 10, 20, version);
	  label->setColor( 0.6f, 0.75f, 0.95f, 1.0f );
	  mainWin->addWidget((Widget*)label);

	  newGameButton = new Button( 10, 40, 260, 70, "New Game" );
	  newGameButton->getLabel()->setColor( 0.25f, 0.25f, 0, 1.0f );
	  mainWin->addWidget((Widget*)newGameButton);
	  continueButton = new Button( 10, 80, 260, 110, "Continue Game" );
	  continueButton->getLabel()->setColor( 0.25f, 0.25f, 0, 1.0f );
	  mainWin->addWidget((Widget*)continueButton);
	  optionsButton = new Button( 10, 120, 260, 150, "Options" );
	  optionsButton->getLabel()->setColor( 0.25f, 0.25f, 0, 1.0f );
	  mainWin->addWidget((Widget*)optionsButton);
	  aboutButton = new Button( 10, 160, 260, 190, "About" );
	  aboutButton->getLabel()->setColor( 0.25f, 0.25f, 0, 1.0f );
	  mainWin->addWidget((Widget*)aboutButton);
	  quitButton = new Button( 10, 200, 260, 230, "Quit" );
	  quitButton->getLabel()->setColor( 0.25f, 0.25f, 0, 1.0f );
	  mainWin->addWidget((Widget*)quitButton);

    } else {
	  scourge->getGui()->popWindows();
    }
	mainWin->setVisible(true);
}

void MainMenu::destroy() {
  scourge->getGui()->pushWindows();
	mainWin->setVisible(false);
}

void MainMenu::drawView(SDL_Surface *screen) {
  glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT );
  glClearColor( 0.0f, 0.0f, 0.0f, 0.5f );
  glClearDepth( 1.0f );



  // create a stencil for the water
  glDisable(GL_DEPTH_TEST);
  glColorMask(0,0,0,0);
  glEnable(GL_STENCIL_TEST);
  glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);
  glStencilFunc(GL_ALWAYS, 1, 1);
  drawWater();
  
  // Use the stencil to draw
  glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
  glEnable(GL_DEPTH_TEST);
  glStencilOp(GL_KEEP, GL_KEEP, GL_KEEP);
  glStencilFunc(GL_EQUAL, 1, 0xffffffff);  // draw if stencil==1
  drawClouds(false, true);
  glDisable(GL_STENCIL_TEST);
      
  // draw the blended walls
  glEnable(GL_BLEND);  
  glDepthMask(GL_FALSE);
  //glDisable(GL_LIGHTING);
  glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_COLOR);
  drawWater();
  //glEnable(GL_LIGHTING);
  glDepthMask(GL_TRUE);    
  glDisable(GL_BLEND);



  glDisable(GL_DEPTH_TEST);

  drawClouds(true, false);

  // drawWater();


  // draw the house
  glDisable( GL_TEXTURE_2D );
  //  glDisable( GL_LIGHTING );
  glEnable(GL_ALPHA_TEST);
  glAlphaFunc(GL_NOTEQUAL, 0);        
  glPushMatrix();
  //glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
  glRasterPos2f( scourge->getSDLHandler()->getScreen()->w - scourge->getShapePalette()->scourge->w, 
				 0 );
  glDrawPixels(scourge->getShapePalette()->scourge->w, 
			   scourge->getShapePalette()->scourge->h,
			   GL_BGRA, GL_UNSIGNED_BYTE, scourge->getShapePalette()->scourgeImage);
  glDisable(GL_ALPHA_TEST);

  // draw candle flame
  glEnable( GL_TEXTURE_2D );
  glEnable(GL_BLEND);  
  glBlendFunc(GL_SRC_COLOR, GL_ONE);
  //setBlendFunc();
  glBindTexture( GL_TEXTURE_2D, scourge->getShapePalette()->candle );
  glColor4f( 0.7, 0.7, 0.3, 0.5 );
  glPushMatrix();
  glLoadIdentity();
  glTranslatef( scourge->getSDLHandler()->getScreen()->w - 215 + (int)(8.0 * rand()/RAND_MAX) - 4, 
				385 + (int)(8.0 * rand()/RAND_MAX) - 4, 
				0 ); 
  float w = 64;
  float h = 64;
  glBegin( GL_QUADS );
  glNormal3f(0.0f, 0.0f, 1.0f);
  glTexCoord2f( 1.0f, 1.0f );
  glVertex3f(w, h, 0);
  glTexCoord2f( 1.0f, 0.0f );
  glVertex3f(w, 0, 0);
  glTexCoord2f( 0.0f, 0.0f );
  glVertex3f(0, 0, 0);
  glTexCoord2f( 0.0f, 1.0f );
  glVertex3f(0, h, 0);
  glEnd();
  glPopMatrix();
  glDisable( GL_TEXTURE_2D );
  glDisable( GL_BLEND );

  // draw the logo
  glEnable(GL_ALPHA_TEST);
  glAlphaFunc(GL_NOTEQUAL, 0);        
  glPixelZoom( 1.0, -1.0 );
  glRasterPos2f( 250, 30 );
  glDrawPixels(scourge->getShapePalette()->logo->w, 
			   scourge->getShapePalette()->logo->h,
			   GL_BGRA, GL_UNSIGNED_BYTE, scourge->getShapePalette()->logoImage);
  glDisable(GL_ALPHA_TEST);

  glPopMatrix();
  glEnable( GL_TEXTURE_2D );
  //  glEnable( GL_LIGHTING );
  glEnable(GL_DEPTH_TEST);

  // draw the window
  scourge->getGui()->drawWindows();
}

void MainMenu::drawClouds(bool moveClouds, bool flipped) {
  // draw clouds
  float w, h;
  glEnable( GL_TEXTURE_2D );
  //  glDisable( GL_LIGHTING );
  glEnable( GL_BLEND );
  glBlendFunc( GL_ONE_MINUS_DST_COLOR, GL_ONE );
  //  setBlendFunc();
  glBindTexture( GL_TEXTURE_2D, scourge->getShapePalette()->cloud );
  if(flipped) 
	glColor4f( 0.1, 0.1, 0.3, 0.5 );
  else
	glColor4f( 1, 1, 1, 1 );
  for(int i = 0; i < cloudCount; i++) {
	glPushMatrix();
	glTranslatef( cloud[i].x, 
				  (flipped ? 600 - (cloud[i].y + h / 2.0) : cloud[i].y), 
				  0 );
	w = cloud[i].w;
	h = cloud[i].h;
    glBegin( GL_QUADS );
    glNormal3f(0.0f, 0.0f, 1.0f);
    glTexCoord2f( 1.0f, (flipped ? 0.0f : 1.0f) );
    glVertex3f(w, (flipped ? h / 2.0 : h), 0);
    glTexCoord2f( 1.0f, (flipped ? 1.0f : 0.0f) );
    glVertex3f(w, 0, 0);
    glTexCoord2f( 0.0f, (flipped ? 1.0f : 0.0f) );
    glVertex3f(0, 0, 0);
    glTexCoord2f( 0.0f, (flipped ? 0.0f : 1.0f) );
    glVertex3f(0, (flipped ? h / 2.0 : h), 0);
    glEnd();
	glPopMatrix();

	if(moveClouds) {
	  cloud[i].x += cloud[i].speed;
	  if(cloud[i].x >= scourge->getSDLHandler()->getScreen()->w) {
		cloud[i].x = -cloud[i].w;
	  }
	}
  }
  glDisable( GL_BLEND );
  //glEnable(GL_DEPTH_TEST);
  glDisable( GL_TEXTURE_2D );
  //  glDisable( GL_LIGHTING );
}

void MainMenu::drawWater() {
  float w, h;
  // draw the water
  glPushMatrix();
  w = scourge->getSDLHandler()->getScreen()->w;
  h = 130;
  glLoadIdentity();
  glTranslatef( 0, 600 - h, 0);
  glDisable( GL_TEXTURE_2D );
  //  glDisable( GL_LIGHTING );
  //glEnable( GL_BLEND );
  //glBlendFunc( GL_ONE_MINUS_DST_COLOR, GL_ONE );
  glBegin( GL_QUADS );
  glNormal3f(0.0f, 0.0f, 1.0f);
  glColor4f(0, 0, 0.1, 1);
  glVertex3f(w, h, 0);
  glColor4f(0, 0.1, 0.4, 1);
  glVertex3f(w, 0, 0);
  glColor4f(0, 0.1, 0.4, 1);
  glVertex3f(0, 0, 0);
  glColor4f(0, 0, 0.1, 1);
  glVertex3f(0, h, 0);  
  glEnd();
  //glDisable( GL_BLEND );
  glPopMatrix();
}

// window callback
void MainMenu::drawMenu(int x, int y) {
  glColor3f(1.0f, 0.8f, 0.3f);
  
  scourge->getSDLHandler()->texPrint(20, 150, 
									 "Scourge version %7.2f", 
									 SCOURGE_VERSION);

  char menu[][40] = {
    { "1. Start a new game" },
    { "2. Continue a game" },
    { "3. Options" },
    { "4. About" },
    { "5. Quit" }
  };
  for(int i = 0; i < 5; i++) {
	scourge->getSDLHandler()->texPrint(20.0f, 30 + (i * 20), "%s", menu[i]);
  }
  
  if(showDebug) {
      scourge->getGui()->debugActiveRegions();
  }
}

bool MainMenu::handleEvent(Widget *widget, SDL_Event *event) {
  if(widget == newGameButton) {
	fprintf(stderr, "new game!!\n");
	value = NEW_GAME;
	return true;
  } else if(widget == continueButton) {
	fprintf(stderr, "continue!!\n");
	value = CONTINUE_GAME;
	return true;
  } else if(widget == optionsButton) {
	fprintf(stderr, "options!!\n");
	value = OPTIONS;
	return true;
  } else if(widget == aboutButton) {
	fprintf(stderr, "about!!\n");
	value = ABOUT;
	return true;
  } else if(widget == quitButton) {
	fprintf(stderr, "quit!!\n");
	value = QUIT;
	return true;
  }
  return false;
}

bool MainMenu::handleEvent(SDL_Event *event) {
  switch(event->type) {
  case SDL_MOUSEBUTTONUP:
    if(event->button.button) {
        int region = scourge->getGui()->testActiveRegions(event->button.x, event->button.y);
        if(region >= Constants::MENU_0 && region <= Constants::MENU_4) {
            value = (region - Constants::MENU_0) + NEW_GAME;
            fprintf(stderr, "value=%d region=%d\n", value, region);
            return true;
        }
    }
    break;
  case SDL_KEYDOWN:
    switch(event->key.keysym.sym) {
    case SDLK_1: value = NEW_GAME; return true;
    case SDLK_2: value = CONTINUE_GAME; return true;
    case SDLK_3: value = OPTIONS; return true;
    case SDLK_4: value = ABOUT; return true;
	case SDLK_5: case SDLK_ESCAPE: value = QUIT; return true;
    case SDLK_6:
        blendA++; if(blendA >= 11) blendA = 0;
        fprintf(stderr, "blend: a=%d b=%d\n", blendA, blendB);
        break;
    case SDLK_7:
        blendB++; if(blendB >= 11) blendB = 0;
        fprintf(stderr, "blend: a=%d b=%d\n", blendA, blendB);
        break;

    case SDLK_SPACE: showDebug = true; return false;
    default: break;
    }
  case SDL_KEYUP:
    switch(event->key.keysym.sym) {
    case SDLK_SPACE: showDebug = false; return false;
    default: break;
    }      
  default: break;  
  }  
  return false;
}

int MainMenu::getValue() {
  return value;
}
