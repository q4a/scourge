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

MainMenu::MainMenu(Scourge *scourge){
  this->scourge = scourge;
  this->cloudCount = 30;
  this->mainWin = NULL;
  for(int i = 0; i < cloudCount; i++) {
	cloud[i].x = (int)((float)scourge->getSDLHandler()->getScreen()->w * rand()/RAND_MAX);
	cloud[i].y = (int)(50.0 * rand()/RAND_MAX);
	cloud[i].w = (int)(55.0 * rand()/RAND_MAX) + 200;
	cloud[i].h = (int)(28.0 * rand()/RAND_MAX) + 100;
	cloud[i].speed = (int)(2.0 * rand()/RAND_MAX) + 1;
  }
  // The new style gui
  mainWin = new Window( scourge->getSDLHandler(),
												50, 230, 270, 220, 
												strdup("Main Menu"), 
												scourge->getShapePalette()->getGuiTexture(),
												false );
  
  char version[100];
  sprintf(version, "Scourge version %7.2f", SCOURGE_VERSION);
  Label *label = new Label( 10, 20, strdup(version));
  label->setColor( 0, 0, 0, 1.0f );
  mainWin->addWidget((Widget*)label);
  
  newGameButton = new Button( 10, 40, 260, 60, strdup("New Game") );
  mainWin->addWidget((Widget*)newGameButton);
  continueButton = new Button( 10, 70, 260, 90, strdup("Continue Game") );
  mainWin->addWidget((Widget*)continueButton);
  optionsButton = new Button( 10, 100, 260, 120, strdup("Options") );
  mainWin->addWidget((Widget*)optionsButton);
  aboutButton = new Button( 10, 130, 260, 150, strdup("About") );
  mainWin->addWidget((Widget*)aboutButton);
  quitButton = new Button( 10, 160, 260, 180, strdup("Quit") );
  mainWin->addWidget((Widget*)quitButton);
}
MainMenu::~MainMenu(){
}

void MainMenu::drawView() {
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
      
  // draw the blended water
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
}

void MainMenu::drawAfter() {
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
    w = cloud[i].w;
    h = cloud[i].h;
	glPushMatrix();
	glTranslatef( cloud[i].x, 
				  (flipped ? 600 - (cloud[i].y + h / 2.0) : cloud[i].y), 
				  0 );
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

bool MainMenu::handleEvent(Widget *widget, SDL_Event *event) {

  if(scourge->getOptionsMenu()->isVisible()) {
	scourge->getOptionsMenu()->handleEvent(widget, event);
	return false;
  }

  if(widget == newGameButton) {
	value = NEW_GAME;
	return true;
  } else if(widget == continueButton) {
	value = CONTINUE_GAME;
	return true;
  } else if(widget == optionsButton) {
	value = OPTIONS;
	return true;
  } else if(widget == aboutButton) {
	value = ABOUT;
	return true;
  } else if(widget == quitButton) {
	value = QUIT;
	return true;
  }
  return false;
}

bool MainMenu::handleEvent(SDL_Event *event) {

  if(scourge->getOptionsMenu()->isVisible()) {
	scourge->getOptionsMenu()->handleEvent(event);
	return false;
  }

  switch(event->type) {
  case SDL_KEYDOWN:
    switch(event->key.keysym.sym) {
    case SDLK_1: value = NEW_GAME; return true;
    case SDLK_2: value = CONTINUE_GAME; return true;
    case SDLK_3: value = OPTIONS; return true;
    case SDLK_4: value = ABOUT; return true;
    case SDLK_5: case SDLK_ESCAPE: value = QUIT; return true;
    case SDLK_SPACE: mainWin->setVisible(false); mainWin->setVisible(true); break;
    default: break;
    }
  default: break;  
  }  
  return false;
}

int MainMenu::getValue() {
  return value;
}
