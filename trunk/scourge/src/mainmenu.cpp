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

//#define AT_WORK

#define LOGO_DELTA 0.05f
#define LOGO_ROT_POS 30.0f
#define LOGO_ROT_NEG 0
#define LOGO_ZOOM 0.8f

#define LOGO_SPRITE_DELTA 8.0f
#define PI 3.14159f

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

  logoRot = 0;
  logoRotDelta = LOGO_DELTA;
  logoTicks = 0;
  logoTicksDelta = 50;
  logoSpriteCount = 0;

  // The new style gui
#ifndef AT_WORK

#ifdef HAVE_SDL_NET
  mainWin = new Window( scourge->getSDLHandler(),
												50, 230, 270, 250, 
												strdup("Main Menu"), 
												scourge->getShapePalette()->getGuiTexture(),
												false );
#else
  mainWin = new Window( scourge->getSDLHandler(),
												50, 230, 270, 220, 
												strdup("Main Menu"), 
												scourge->getShapePalette()->getGuiTexture(),
												false );
#endif
  
  char version[100];
  sprintf(version, "Scourge version %7.2f", SCOURGE_VERSION);
  Label *label = new Label( 10, 20, strdup(version));
  label->setColor( 0, 0, 0, 1.0f );
  mainWin->addWidget((Widget*)label);
  
  int y = 40;
  newGameButton = new Button( 10, y, 260, y + 20, scourge->getShapePalette()->getHighlightTexture(), strdup("New Game") );
  mainWin->addWidget((Widget*)newGameButton);
  y += 30;
  continueButton = new Button( 10, y, 260, y + 20, scourge->getShapePalette()->getHighlightTexture(), strdup("Continue Game") );
  mainWin->addWidget((Widget*)continueButton);
  y += 30;
#ifdef HAVE_SDL_NET
  multiplayer = new Button( 10, y, 260, y + 20, scourge->getShapePalette()->getHighlightTexture(), strdup("Multiplayer") );
  mainWin->addWidget((Widget*)multiplayer);
  y += 30;
#endif
  optionsButton = new Button( 10, y, 260, y + 20, scourge->getShapePalette()->getHighlightTexture(), strdup("Options") );
  mainWin->addWidget((Widget*)optionsButton);
  y += 30;
  aboutButton = new Button( 10, y, 260, y + 20, scourge->getShapePalette()->getHighlightTexture(), strdup("About") );
  mainWin->addWidget((Widget*)aboutButton);
  y += 30;
  quitButton = new Button( 10, y, 260, y + 20, scourge->getShapePalette()->getHighlightTexture(), strdup("Quit") );
  mainWin->addWidget((Widget*)quitButton);
  y += 30;
#else
  mainWin = new Window( scourge->getSDLHandler(),
												50, 230, 5, 5, 
												strdup(""), 
												scourge->getShapePalette()->getGuiTexture(),
												false );
#endif
}
MainMenu::~MainMenu(){
}

void MainMenu::drawView() {
#ifndef AT_WORK
  // create a stencil for the water
  glDisable(GL_DEPTH_TEST);
  glColorMask(0,0,0,0);
  if(scourge->getUserConfiguration()->getStencilbuf()
     && scourge->getUserConfiguration()->getStencilBufInitialized()){
    glEnable(GL_STENCIL_TEST);
    glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);
    glStencilFunc(GL_ALWAYS, 1, 1);
  }
  drawWater();
  
  // Use the stencil to draw
  glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
  glEnable(GL_DEPTH_TEST);
  if(scourge->getUserConfiguration()->getStencilbuf()
     && scourge->getUserConfiguration()->getStencilBufInitialized()){
    glStencilOp(GL_KEEP, GL_KEEP, GL_KEEP);
    glStencilFunc(GL_EQUAL, 1, 0xffffffff);  // draw if stencil==1
  }
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

  
  glDisable(GL_TEXTURE_2D);
  glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
  glDisable(GL_DEPTH_TEST);
  glEnable(GL_ALPHA_TEST);
  glAlphaFunc(GL_NOTEQUAL, 0);        
  glPushMatrix();
  glLoadIdentity( );                         
  glPixelZoom( 1.0, -1.0 );
  glRasterPos2f( scourge->getSDLHandler()->getScreen()->w - scourge->getShapePalette()->scourge->w, 0 );
  glDrawPixels(scourge->getShapePalette()->scourge->w, 
			   scourge->getShapePalette()->scourge->h,
			   GL_BGRA, GL_UNSIGNED_BYTE, scourge->getShapePalette()->scourgeImage);
  glDisable(GL_ALPHA_TEST);
  glPopMatrix();

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
  //glEnable(GL_ALPHA_TEST);
  //glAlphaFunc(GL_NOTEQUAL, 0);        
  //  glPixelZoom( 1.0, -1.0 );
  //  glRasterPos2f( 250, 30 );
  //  glDrawPixels(scourge->getShapePalette()->logo->w, 
  //			   scourge->getShapePalette()->logo->h,
  //			   GL_BGRA, GL_UNSIGNED_BYTE, scourge->getShapePalette()->logoImage);
  //glDisable(GL_ALPHA_TEST);

  drawLogo();

  //glPopMatrix();
#else
  glLoadIdentity();
  glTranslatef(0.0f,0.0f,-1.0f);
  glDisable(GL_CULL_FACE);
  glColor4f(1, 1, 1, 1);
  glBegin(GL_QUADS);
  glVertex3f( 100, 100, 0 );
  glVertex3f( 200, 100, 0 );
  glVertex3f( 200, 200, 0 );
  glVertex3f( 100, 200, 0 );
  glEnd();
  scourge->getSDLHandler()->texPrint(300, 300, "Hello world");
#endif
  glEnable( GL_TEXTURE_2D );
  //  glEnable( GL_LIGHTING );
  glEnable(GL_DEPTH_TEST);
}

void MainMenu::drawAfter() {
}

void MainMenu::drawLogo() {

  //  if((int)(5.0f * rand()/RAND_MAX) == 0) 
	addLogoSprite();
  drawLogoSprites();

  drawParticles();

  //  glDisable( GL_DEPTH_TEST );
  glEnable( GL_TEXTURE_2D );
  glEnable(GL_BLEND);  
  //  glBlendFunc( GL_SRC_ALPHA, GL_DST_ALPHA );
  glBlendFunc( GL_SRC_ALPHA, GL_ONE );
  //  scourge->setBlendFunc();
  glPushMatrix();
  glLoadIdentity();
  glRotatef(logoRot, 0, 0, 1 );
  glTranslatef( 70, 10 - abs((int)(logoRot / 0.25f)), 500 );
  float zoom = (logoRot / (LOGO_ROT_POS / LOGO_ZOOM)) + 1.0f;
  glScalef( zoom, zoom, 1 );
  float w = scourge->getShapePalette()->logo->w;
  float h = scourge->getShapePalette()->logo->h;
  glColor4f( 1, 1, 1, 1 );
  glBindTexture( GL_TEXTURE_2D, scourge->getShapePalette()->logo_texture );
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
  //  glEnable( GL_DEPTH_TEST );

  GLint t = SDL_GetTicks();
  if(t - logoTicks > logoTicksDelta) {
	logoTicks = t;
	logoRot += logoRotDelta;
	if(logoRotDelta < 0) {
	  logoRotDelta += -LOGO_DELTA;
	  if(logoRot <= LOGO_ROT_NEG ) {
		logoRotDelta = LOGO_DELTA;
	  }
	} else {
	  logoRotDelta += LOGO_DELTA;
	  if(logoRot >= LOGO_ROT_POS) {
		logoRotDelta = -LOGO_DELTA;		
	  }
	}
  }
}

void MainMenu::addLogoSprite() {
  if(logoSpriteCount >= MAX_LOGOS - 1) return;
  logoSprite[logoSpriteCount].x = 70.0f;
  logoSprite[logoSpriteCount].y = 10 - abs((int)(logoRot / 0.25f));
  logoSprite[logoSpriteCount].angle = 1.0f + (88.0f * rand()/RAND_MAX);
  logoSprite[logoSpriteCount].quadrant = (int)(4.0f * rand()/RAND_MAX);
  logoSprite[logoSpriteCount].steps = 0;
  logoSprite[logoSpriteCount].rot = logoRot;
  logoSpriteCount++;
}

void MainMenu::drawLogoSprites() {
  for(int i = 0; i < logoSpriteCount; i++) {
	glEnable( GL_TEXTURE_2D );
	glEnable(GL_BLEND);  
	//	GL_ONE_MINUS_SRC_COLOR, GL_ZERO
	// GL_DST_COLOR, GL_ONE
	glBlendFunc( GL_SRC_ALPHA, GL_ONE );
	//scourge->setBlendFunc();
	glPushMatrix();
	glLoadIdentity();
	glRotatef(logoSprite[i].rot, 0, 0, 1 );
	glTranslatef( logoSprite[i].x, logoSprite[i].y, 500 );
	float zoom = 1.2f;
	glScalef( zoom, zoom, 1 );
	float w = scourge->getShapePalette()->logo->w;
	float h = scourge->getShapePalette()->logo->h;

	float alpha = (float)logoSprite[i].steps / 70.0f;
	//	if(alpha > 1.0f) alpha = 1.0f;
	if(alpha >= 1.0f) alpha = 1.0f - (logoSprite[i].steps / 10.0f);

	//	cerr << "i=" << i << " steps=" << logoSprite[i].steps << " alpha=" << alpha << endl;
	logoSprite[i].steps++;

	glColor4f( 0.5, 0.5, 1, alpha );
	glBindTexture( GL_TEXTURE_2D, scourge->getShapePalette()->logo_texture );
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

	float rad = PI / (180.0f / logoSprite[i].angle);
	switch(logoSprite[i].quadrant) {
	case 0:
	  logoSprite[i].x += cos(rad) * LOGO_SPRITE_DELTA;
	  logoSprite[i].y -= sin(rad) * LOGO_SPRITE_DELTA;
	  break;
	case 1:
	  logoSprite[i].x += cos(rad) * LOGO_SPRITE_DELTA;
	  logoSprite[i].y += sin(rad) * LOGO_SPRITE_DELTA;
	  break;
	case 2:
	  logoSprite[i].x -= cos(rad) * LOGO_SPRITE_DELTA;
	  logoSprite[i].y += sin(rad) * LOGO_SPRITE_DELTA;
	  break;
	case 3:
	  logoSprite[i].x -= cos(rad) * LOGO_SPRITE_DELTA;
	  logoSprite[i].y -= sin(rad) * LOGO_SPRITE_DELTA;
	  break;
	}

	// delete if off-screen
	if(logoSprite[i].steps > 20 || 
	   logoSprite[i].x <= -w * 2.0f || logoSprite[i].x >= scourge->getSDLHandler()->getScreen()->w ||
	   logoSprite[i].y <= -h * 2.0f || logoSprite[i].y >= scourge->getSDLHandler()->getScreen()->h) {
	  logoSprite[i].x = logoSprite[logoSpriteCount - 1].x;
	  logoSprite[i].y = logoSprite[logoSpriteCount - 1].y;
	  logoSprite[i].angle = logoSprite[logoSpriteCount - 1].angle;
	  logoSprite[i].quadrant = logoSprite[logoSpriteCount - 1].quadrant;
	  logoSprite[i].steps = logoSprite[logoSpriteCount - 1].steps;
	  logoSprite[i].rot = logoSprite[logoSpriteCount - 1].rot;
	  logoSpriteCount--;
	  i--;
	}
  }
}

void MainMenu::drawParticles() {
  /*
	// draw particles from logo to right like a torch
  for(int i = 0; i < PARTICLE_COUNT; i++) {
	if(particle[i] == null) {
	  particle[i] = new Particle();
	  particle[i]->x = 80;
	  particle[i]->y = 10 + (30.0f * rand()/RAND_MAX);

	}
  }
  */
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
				  (flipped ? 600 - (cloud[i].y + h / 2.0) : cloud[i].y + 130), 
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

  if(scourge->getMultiplayerDialog()->isVisible()) {
    scourge->getMultiplayerDialog()->handleEvent(widget, event);
    if(!scourge->getMultiplayerDialog()->isVisible()) {
      if(scourge->getMultiplayerDialog()->getValue() == MultiplayerDialog::START_SERVER) {
        value = MULTIPLAYER_START;
        return true;
      } else if(scourge->getMultiplayerDialog()->getValue() == MultiplayerDialog::JOIN_SERVER) {
        if(!(strlen(scourge->getMultiplayerDialog()->getServerName()) &&
             strlen(scourge->getMultiplayerDialog()->getServerPort()) &&
             strlen(scourge->getMultiplayerDialog()->getUserName()))) {
          scourge->showMessageDialog(Constants::getMessage(Constants::JOIN_SERVER_ERROR));
        } else {
          value = MULTIPLAYER_START;
          return true;
        }
      }
    }
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
  } else if(widget == multiplayer) {
    value = MULTIPLAYER;
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

  if(scourge->getMultiplayerDialog()->isVisible()) {
    scourge->getMultiplayerDialog()->handleEvent(event);
    return false;
  }

  switch(event->type) {
  case SDL_KEYDOWN:
    switch(event->key.keysym.sym) {
    case SDLK_1: value = NEW_GAME; return true;
    case SDLK_2: value = CONTINUE_GAME; return true;
    case SDLK_3: value = MULTIPLAYER; return true;
    case SDLK_4: value = OPTIONS; return true;
    case SDLK_5: value = ABOUT; return true;
    case SDLK_6: case SDLK_ESCAPE: value = QUIT; return true;
    default: break;
    }
	break;
  case SDL_KEYUP:
	if(event->key.keysym.sym == '8'){
	  Scourge::blendA++; if(Scourge::blendA >= 11) Scourge::blendA = 0;
	  fprintf(stderr, "blend: a=%d b=%d\n", Scourge::blendA, Scourge::blendB);
    }
    else if(event->key.keysym.sym == '9'){    
	  Scourge::blendB++; if(Scourge::blendB >= 11) Scourge::blendB = 0;
	  fprintf(stderr, "blend: a=%d b=%d\n", Scourge::blendA, Scourge::blendB);
	}
	break;
  default: break;  
  }  
  return false;
}

int MainMenu::getValue() {
  return value;
}
