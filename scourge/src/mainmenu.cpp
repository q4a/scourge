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
#include "render/renderlib.h"
#include "sdlhandler.h"
#include "scourge.h"
#include "text.h"
#include "partyeditor.h"
#include "gui/window.h"
#include "gui/label.h"
#include "gui/button.h"
#include "gui/scrollinglabel.h"
#include "gui/progress.h"
#include "shapepalette.h"
#include "savegamedialog.h"
#include "freetype/fontmgr.h"
#include "sound.h"
#include "texteffect.h"

using namespace std;

#define LOGO_DELTA 0.05f
#define LOGO_ROT_POS 30.0f
#define LOGO_ROT_NEG 0
#define LOGO_ZOOM 0.8f

#define LOGO_SPRITE_DELTA 1.0f
//#define PI 3.14159f

#define WATER_HEIGHT 130

#define MENU_ITEM_WIDTH 256
#define MENU_ITEM_HEIGHT 32
#define MENU_ITEM_ZOOM 1.5f
#define MENU_ITEM_PARTICLE_ZOOM 1.1f
#define MAX_PARTICLE_LIFE 50

bool eventsEnabled = false;

const char *MainMenu::menuText[] = { 
  N_( "New Game" ), 
	N_( "Continue Game" ), 
	N_( "Multiplayer Game" ), 
	N_( "Options" ), 
	N_( "About S.c.o.u.r.g.e."), 
	N_( "Quit" ), 
	""
};

int activeMenuItem = -1;

const int MainMenu::values[] = {
  NEW_GAME, CONTINUE_GAME, MULTIPLAYER, OPTIONS, ABOUT, QUIT, 0
};

MainMenu::MainMenu(Scourge *scourge){
  this->scourge = scourge;
  this->cloudCount = 30;
	this->lastMenuTick = 0;
	this->savegameDialog = new SavegameDialog( scourge );
  for(int i = 0; i < cloudCount; i++) {
	cloud[i].x = Util::dice( scourge->getSDLHandler()->getScreen()->w );
	cloud[i].y = Util::dice( 50 );
	cloud[i].w = Util::pickOne( 200, 254 );
	cloud[i].h = Util::pickOne( 100, 127 );
	cloud[i].speed = Util::pickOne( 1, 2 );
  }

  logoRot = -scourge->getShapePalette()->logo->h;
  logoRotDelta = LOGO_DELTA;
  logoTicks = 0;
  logoTicksDelta = 50;
  logoSpriteCount = 0;
  candleFlameX = candleFlameY = 0;

  top = (scourge->getSDLHandler()->getScreen()->h - 600) / 2;
  openingTop = scourge->getSDLHandler()->getScreen()->h / 2;
	musicStarted = false;
  lastTick = 0;
  lastTickMenu = 0;
  initTextures = false;

  starCount = 200;
  for(int i = 0; i < starCount; i++) {
    star[i].x = Util::dice( scourge->getSDLHandler()->getScreen()->w );
    star[i].y = Util::pickOne( top, top + 599 - WATER_HEIGHT - 160 );
  }
  // The new style gui  
  int w = 250;
  int h = 120;
  newGameConfirm = new Window(scourge->getSDLHandler(),
                              (scourge->getSDLHandler()->getScreen()->w/2) - (w/2), 
                              (scourge->getSDLHandler()->getScreen()->h/2) - (h/2), 
                              w, h,
                              _( "New Game Confirmation" ),
                              scourge->getShapePalette()->getGuiTexture(), false);
  newGameConfirmOK = newGameConfirm->createButton( 40, 55, 110, 75, Constants::getMessage( Constants::OK_LABEL ) );
  newGameConfirmCancel = newGameConfirm->createButton( 140, 55, 210, 75, Constants::getMessage( Constants::CANCEL_LABEL ));
  newGameConfirm->createLabel( 20, 20, Constants::getMessage( Constants::DELETE_OLD_SAVED_GAME ));
  newGameConfirm->setVisible( false );
  newGameConfirm->setModal( true );

  partyEditor = NULL;
  
  // about dialog
  w = 500;
  h = 300;
  aboutDialog = new Window( scourge->getSDLHandler(),
                            (scourge->getSDLHandler()->getScreen()->w/2) - (w/2), 
                            (scourge->getSDLHandler()->getScreen()->h/2) - (h/2), 
                            w, h,
                            _( "About S.c.o.u.r.g.e." ),
                            scourge->getShapePalette()->getGuiTexture(), true );
  aboutText = new ScrollingLabel( 8, 0, 
                                  w - 18, 
                                  h - 65, 
                                  scourge->getShapePalette()->getAboutText() );
  aboutDialog->addWidget( aboutText );
  aboutOK = aboutDialog->createButton( (w/2) - 40, 
                                       ( h - 55 ),
                                       (w/2) + 40, 
                                       ( h - 35 ), 
                                       Constants::getMessage( Constants::OK_LABEL ) );
  aboutDialog->setVisible( false );


	progress = new Progress( scourge->getSDLHandler(),
													 scourge->getSession()->getShapePalette()->getProgressTexture(),
													 scourge->getSession()->getShapePalette()->getProgressHighlightTexture(),
													 100, false, false );
}

MainMenu::~MainMenu(){
	for( unsigned int i = 0; i < textEffects.size(); i++ ) {
		TextEffect *textEffect = textEffects[i];
		delete textEffect;
	}
	textEffects.clear();
}

void MainMenu::drawView() {
	int tickNow = SDL_GetTicks();
	if((tickNow - lastMenuTick) < 15)
		SDL_Delay( 15 - (tickNow - lastMenuTick) );
	lastMenuTick = SDL_GetTicks(); 

	drawStars();

	glDisable(GL_TEXTURE_2D);
	glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
  glEnable(GL_ALPHA_TEST);
  glAlphaFunc(GL_NOTEQUAL, 0);        
  glPushMatrix();
  glLoadIdentity( );                         
	glTranslatef( 0, 0, -100 );
  glPixelZoom( 1.0, -1.0 );
  glRasterPos2f( 0, top + (600 - WATER_HEIGHT - scourge->getShapePalette()->scourgeBackdrop->h) );
  glDrawPixels(scourge->getShapePalette()->scourgeBackdrop->w, 
							 scourge->getShapePalette()->scourgeBackdrop->h,
							 GL_BGRA, GL_UNSIGNED_BYTE, scourge->getShapePalette()->scourgeImageBackdrop);
	glDisable(GL_ALPHA_TEST);
  glPopMatrix();
  glEnable( GL_TEXTURE_2D );	

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
	
	
	glDisable(GL_TEXTURE_2D);
	glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
  glDisable(GL_DEPTH_TEST);
  glEnable(GL_ALPHA_TEST);
  glAlphaFunc(GL_NOTEQUAL, 0);        
  glPushMatrix();
  glLoadIdentity( );                         
  glPixelZoom( 1.0, -1.0 );
  glRasterPos2f( scourge->getSDLHandler()->getScreen()->w - scourge->getShapePalette()->scourge->w, top );
  glDrawPixels(scourge->getShapePalette()->scourge->w, 
							 scourge->getShapePalette()->scourge->h,
							 GL_BGRA, GL_UNSIGNED_BYTE, scourge->getShapePalette()->scourgeImage);
	glDisable(GL_ALPHA_TEST);
  glPopMatrix();
  glEnable( GL_TEXTURE_2D );
  glEnable(GL_DEPTH_TEST);

	drawMenu();
						 
  // draw the boards
  if(openingTop > 0) {
    glPushMatrix();
    glColor3f( 1, 1, 1 );
    glBindTexture( GL_TEXTURE_2D, scourge->getShapePalette()->getGuiWoodTexture() );
    //    float TILE_W = 510 / 2.0f;
    float TILE_H = 270 / 3.0f; 
    glEnable( GL_TEXTURE_2D );

    glLoadIdentity();
    glTranslatef( 0, 0, 0 );
    glBegin( GL_QUADS );
    glTexCoord2f( 0, openingTop / TILE_H );
    glVertex2i( 0, 0 );
    glTexCoord2f( 0, 0 );
    glVertex2i( 0, openingTop );
    glTexCoord2f( 1, 0 );
    glVertex2i( scourge->getSDLHandler()->getScreen()->w, openingTop );
    glTexCoord2f( 1, openingTop / TILE_H );
    glVertex2i( scourge->getSDLHandler()->getScreen()->w, 0 );
    glEnd();

    glLoadIdentity();
    glTranslatef( 0, scourge->getSDLHandler()->getScreen()->h - openingTop, 0 );
    glBegin( GL_QUADS );
    glTexCoord2f( 0, 0 );
    glVertex2i( 0, 0 );
    glTexCoord2f( 0, openingTop / TILE_H );
    glVertex2i( 0, openingTop );
    glTexCoord2f( 1, openingTop / TILE_H );
    glVertex2i( scourge->getSDLHandler()->getScreen()->w, openingTop );
    glTexCoord2f( 1, 0 );
    glVertex2i( scourge->getSDLHandler()->getScreen()->w, 0 );
    glEnd();
    glDisable( GL_TEXTURE_2D );

    for(int i = 0; i < 2; i++) {
      glLoadIdentity();
      glTranslatef( 0, (i == 0 ? openingTop : scourge->getSDLHandler()->getScreen()->h - openingTop), 0 );
      glColor4f( 1, 0.7f, 0, 1 );
      glBegin( GL_LINES );
      glVertex2i( 0, 0 );
      glVertex2i( scourge->getSDLHandler()->getScreen()->w, 0 );
      glEnd();
    }
	 
    glLoadIdentity();
    glTranslatef( 10, scourge->getSDLHandler()->getScreen()->h - openingTop + 12, 0 );
    char version[100];
    snprintf(version, 100, _( "Scourge version %s" ), SCOURGE_VERSION);
	scourge->getSDLHandler()->setFontType( Constants::SCOURGE_DEFAULT_FONT );
    scourge->getSDLHandler()->texPrint( 0, 0, version );
    glColor3f( 0.8f, 0.75f, 0.65f );
    int y = 14;
    scourge->getSDLHandler()->texPrint( 0, y, _( "Optionally compiled modules:" ) );
    y += 14;
#ifdef HAVE_SDL_NET
    scourge->getSDLHandler()->texPrint( 0, y, _( "[Network]" ) );
    y += 14;
#endif
#ifdef HAVE_SDL_MIXER
    scourge->getSDLHandler()->texPrint( 0, y, _( "[Sound]" ) );
		y += 14;    
#endif
    glPopMatrix();

    if( openingTop > top && scourge->getSession()->isDataInitialized() ) {
      Uint32 t = SDL_GetTicks();
      if( t - lastTick > 40 ) {
        int d = (scourge->getSDLHandler()->getScreen()->h - openingTop) / 20;
        openingTop -= (10 + (int)(d * 1.2));
        if(openingTop < top) openingTop = top;
        lastTick = t;
      }
    }
  }

	if( openingTop <= top ) {
    drawLogo();
		if( !musicStarted ) {
			scourge->getSession()->getSound()->playMusicMenu();
			musicStarted = true;
		}
  }

	// initialize universe (nice how this is hidden here...)
	scourge->getSession()->initData();
}

void MainMenu::drawAfter() {
	if( strlen( getUpdate() ) ) {
		glPushMatrix();
				
		/*
		glLoadIdentity();
		glEnable( GL_BLEND );
		glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );
		glDisable( GL_TEXTURE_2D );
		glColor4f( 0, 0, 0, 0.75f );
		glBegin( GL_QUADS );
		glVertex3f( 0, 0, 0 );
		glVertex3f( 0, scourge->getScreenHeight(), 0 );
		glVertex3f( scourge->getScreenWidth(), scourge->getScreenHeight(), 0 );
		glVertex3f( scourge->getScreenWidth(), 0, 0 );
		glEnd();		
		glEnable( GL_TEXTURE_2D );
		glDisable( GL_BLEND );
		*/

		glLoadIdentity();
    glTranslatef( 10, scourge->getSDLHandler()->getScreen()->h - openingTop + 12, 0 );
		int y = 70;
		int x = 100;
		float maxStatus = ( scourge->getScreenWidth() - 200 ) / 20.0f;
		glColor3f( 0.8f, 0.75f, 0.65f );
		if( getUpdateTotal() > -1 ) {
			scourge->getSDLHandler()->
				texPrint( x, y - 3, "%s: %d%%", 
									getUpdate(), 
									(int)( ( getUpdateValue() + 1 ) / ( getUpdateTotal() / 100.0f ) ) );
			glTranslatef( x + 150, y - 15, 0 );

			progress->updateStatusLight( NULL, 
																	 (int)( ( getUpdateValue() + 1 ) / ( getUpdateTotal() / maxStatus ) ), 
																	 (int)maxStatus );
		} else {
			scourge->getSDLHandler()->texPrint( x, y - 3, getUpdate() );
		}
		glPopMatrix();
	}
	eventsEnabled = scourge->getSession()->isDataInitialized();
	if( eventsEnabled ) scourge->getSDLHandler()->setCursorMode( Constants::CURSOR_NORMAL );
	else scourge->getSDLHandler()->setCursorMode( Constants::CURSOR_FORBIDDEN );
}

void MainMenu::show() { 
  logoRot = -scourge->getShapePalette()->logo->h;
}

void MainMenu::hide() { 
  openingTop = scourge->getSDLHandler()->getScreen()->h / 2;
	musicStarted = false;
}

void MainMenu::drawMenu() {

	if( textEffects.empty() && openingTop <= top ) {
		int x = 50;
		int y = top + 230;
		for( int i = 0; strlen( menuText[i] ); i++ ) {
			TextEffect *textEffect = new TextEffect( scourge, x - 40, y - 20, _(menuText[i]) );
			textEffects.push_back( textEffect );
			y += 50;
		}
	}

	glDisable(GL_STENCIL_TEST);
	glDisable(GL_SCISSOR_TEST);
	glDisable(GL_CULL_FACE);
	glDisable(GL_BLEND);
	glDisable(GL_ALPHA_TEST);
	glEnable(GL_TEXTURE_2D);
	glDisable(GL_DEPTH_TEST);
	glColor4f( 1, 1, 1, 1 );
	for( int i = 0; i < (int)textEffects.size(); i++ ) {
		TextEffect *textEffect = textEffects[i];
		textEffect->setActive( i == activeMenuItem );
		textEffect->draw();
	}
}

void MainMenu::drawLogo() {


  glEnable( GL_ALPHA_TEST );
  //glAlphaFunc( GL_EQUAL, 0xff );
	glAlphaFunc( GL_NOTEQUAL, 0 );
  glEnable( GL_TEXTURE_2D );
  glPushMatrix();
  glLoadIdentity();
  glTranslatef( 70, logoRot, 0 );
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

  for( int i = 0; i < 2; i++ ) {
    glPushMatrix();
    glLoadIdentity();
    glTranslatef( ( !i ? 100 : 
                    70 + scourge->getShapePalette()->logo->w - 30 - 
                    scourge->getShapePalette()->chain->w ), 
                  logoRot - scourge->getShapePalette()->chain->h, 0 );
    float w = scourge->getShapePalette()->chain->w;
    float h = scourge->getShapePalette()->chain->h;
    glColor4f( 1, 1, 1, 1 );
    glBindTexture( GL_TEXTURE_2D, scourge->getShapePalette()->chain_texture );
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
  }

  glDisable( GL_TEXTURE_2D );
  glDisable( GL_ALPHA_TEST );



  GLint t = SDL_GetTicks();
  if(t - logoTicks > logoTicksDelta) {
    if( logoRot < 120 - ( ( 1024 - scourge->getScreenHeight() ) / 4 ) ) {
      logoTicks = t;
      logoRot += 8;
		}
    candleFlameX = scourge->getSDLHandler()->getScreen()->w - 215 + Util::dice( 4 ) - 4;
    candleFlameY = top + 385 + Util::dice( 4 ) - 4;
  }

  // draw candle flame
  glEnable( GL_TEXTURE_2D );
  glEnable(GL_BLEND);  
  glBlendFunc(GL_SRC_COLOR, GL_ONE);
  //setBlendFunc();
  glBindTexture( GL_TEXTURE_2D, scourge->getShapePalette()->candle );
  glColor4f( 0.7f, 0.7f, 0.3f, 0.5f );
  glPushMatrix();
  glLoadIdentity();
  glTranslatef( candleFlameX, candleFlameY, 0 ); 
  w = 64;
  h = 64;
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
}

void MainMenu::drawStars() {
  glDisable( GL_TEXTURE_2D );
  for(int i = 0; i < starCount; i++) {
    glPushMatrix();
    glLoadIdentity();
    glTranslatef( star[i].x, star[i].y, -200 );
    glColor3f( Util::roll( 0.2f, 0.99f ), 
               Util::roll( 0.2f, 0.99f ), 
               Util::roll( 0.2f, 0.99f ) );
    //int n = (int)(2.0f * rand()/RAND_MAX) + 1;
    int n = 1;
    glBegin( GL_QUADS );
    glVertex2d( 0, 0 );
    glVertex2d( 0, n );
    glVertex2d( n, n );
    glVertex2d( n, 0 );
    glEnd();
    glPopMatrix();
  }
  glEnable( GL_TEXTURE_2D );
  
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
	glColor4f( 0.1f, 0.1f, 0.3f, 0.5f );
  else
	glColor4f( 1, 1, 1, 1 );
  for(int i = 0; i < cloudCount; i++) {
    w = cloud[i].w;
    h = cloud[i].h;
	glPushMatrix();
	glTranslatef( cloud[i].x, 
				  top + (flipped ? 600 - (cloud[i].y + h / 2.0) : cloud[i].y + 130), 
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
  h = WATER_HEIGHT;
  glLoadIdentity();
  glTranslatef( 0, top + (600 - h), 0);
  glDisable( GL_TEXTURE_2D );
  //  glDisable( GL_LIGHTING );
  //glEnable( GL_BLEND );
  //glBlendFunc( GL_ONE_MINUS_DST_COLOR, GL_ONE );
  glBegin( GL_QUADS );
  glNormal3f(0.0f, 0.0f, 1.0f);
	glColor4f(0, 0.1f, 0.4f, 1);
  glVertex3f(w, h, 0);
  glColor4f(0, 0, 0.1f, 1);
  glVertex3f(w, 0, 0);
	glColor4f(0, 0, 0.1f, 1);
  glVertex3f(0, 0, 0);
  glColor4f(0, 0.1f, 0.4f, 1);
  glVertex3f(0, h, 0);  
  glEnd();
  //glDisable( GL_BLEND );
  glPopMatrix();
}

bool MainMenu::handleEvent(Widget *widget, SDL_Event *event) {

	if( !eventsEnabled ) return false;

	if( scourge->getSaveDialog()->getWindow()->isVisible() ) {
    scourge->getSaveDialog()->handleEvent( widget, event );
		return false;
  }

  if(scourge->getOptionsMenu()->isVisible()) {
    scourge->getOptionsMenu()->handleEvent(widget, event);
    return false;
  }

  if( partyEditor && partyEditor->isVisible() ) {
    partyEditor->handleEvent( widget, event );
		//return false;
  }

	if( savegameDialog->getWindow()->isVisible() ) {
		savegameDialog->handleEvent( widget, event );
		return false;
	}

  if( aboutDialog->isVisible() ) {
    if( widget == aboutOK || widget == aboutDialog->closeButton ) {
      aboutDialog->setVisible( false );
    }
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

  if( partyEditor && widget == partyEditor->getCancelButton() ) {
    partyEditor->setVisible( false );
    return false;
  } else if( partyEditor && widget == partyEditor->getStartGameButton() ) {
    partyEditor->setVisible( false );
    value = NEW_GAME_START;
    return true;
  } else if( widget == newGameConfirmOK ) {
    newGameConfirm->setVisible( false );
    showPartyEditor();
    return false;
  } else if( widget == newGameConfirmCancel ) {
    newGameConfirm->setVisible( false );
    return false;
  }
  return false;
}

bool MainMenu::handleEvent(SDL_Event *event) {

	if( !eventsEnabled ) return false;

  if( aboutDialog->isVisible() ) {
    return false;
  }

	if( scourge->getSaveDialog()->getWindow()->isVisible() ) {
		return false;
  }

  if( savegameDialog->getWindow()->isVisible() ) {
		savegameDialog->handleEvent( NULL, event );
    return false;
  }

  if(scourge->getOptionsMenu()->isVisible()) {
    scourge->getOptionsMenu()->handleEvent(event);
    return false;
  }

  if(scourge->getMultiplayerDialog()->isVisible()) {
    scourge->getMultiplayerDialog()->handleEvent(event);
    return false;
  }

  if( partyEditor && partyEditor->isVisible() ) {
    partyEditor->handleEvent( NULL, event );
		//return false;
  }

  /*
  // esc to close new game confirmation (doesn't work)
  if((event->type == SDL_KEYDOWN || event->type == SDL_KEYUP) && 
     newGameConfirm->isVisible() && 
     event->key.keysym.sym == SDLK_ESCAPE) {
    newGameConfirm->setVisible( false );
    return false;
  }
  */

  switch(event->type) {
  //case SDL_KEYDOWN:
	case SDL_KEYUP:
    switch(event->key.keysym.sym) {
    case SDLK_1: value = NEW_GAME; showPartyEditor(); return false;
    case SDLK_2: value = CONTINUE_GAME; scourge->getSaveDialog()->show( false ); return false;
    case SDLK_3: value = MULTIPLAYER; return true;
    case SDLK_4: value = OPTIONS; return true;
    case SDLK_5: value = ABOUT; return true;
    case SDLK_6: case SDLK_ESCAPE: value = QUIT; return true;
    case SDLK_7: value = EDITOR; return true;
    default: break;
    }
	//break;
  //case SDL_KEYUP:
	if(event->key.keysym.sym == '8'){
	  Scourge::blendA++; if(Scourge::blendA >= 11) Scourge::blendA = 0;
	  fprintf(stderr, "blend: a=%d b=%d\n", Scourge::blendA, Scourge::blendB);
    }
    else if(event->key.keysym.sym == '9'){    
	  Scourge::blendB++; if(Scourge::blendB >= 11) Scourge::blendB = 0;
	  fprintf(stderr, "blend: a=%d b=%d\n", Scourge::blendA, Scourge::blendB);
	}
	break;
  case SDL_MOUSEMOTION:
  case SDL_MOUSEBUTTONUP:
  if( event->motion.x >= 50 && event->motion.x < 400 ) {
    activeMenuItem = ( event->motion.y - ( top + 240 ) ) / 50;
  }
  if( event->type == SDL_MOUSEBUTTONUP && activeMenuItem > -1 && activeMenuItem < (int)textEffects.size() ) {
    value = values[activeMenuItem];
    if( value == ABOUT ) {
      aboutDialog->setVisible( true );
      //value = ABOUT;
      return false;
		} else if( value == NEW_GAME ) {
			showPartyEditor();
			return false;
		} else if( value == CONTINUE_GAME ) {
			scourge->getSaveDialog()->show( false );
			return false;
    } else {
      return true;
    }
  }
  break;
  default: break;  
  }  
  return false;
}

int MainMenu::getValue() {
  return value;
}

void MainMenu::showNewGameConfirmationDialog() {
  newGameConfirm->setVisible( true );
}

void MainMenu::showSavegameDialog( bool inSaveMode ) {
	savegameDialog->show( inSaveMode );
}

void MainMenu::showPartyEditor() {
	if( !partyEditor ) partyEditor = new PartyEditor( scourge );
  partyEditor->setVisible( true );
}

void MainMenu::createParty( Creature **pc, int *partySize ) { 
	if( !partyEditor ) partyEditor = new PartyEditor( scourge );
  partyEditor->createParty( pc, partySize ); 
}

RenderedCreature *MainMenu::createWanderingHero( int level ) {
	if( !partyEditor ) partyEditor = new PartyEditor( scourge );
	return partyEditor->createWanderingHero( level );
}

