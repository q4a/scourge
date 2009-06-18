/***************************************************************************
                    mainmenu.cpp  -  The game's main menu
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
#include "common/constants.h"
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

// ###### MS Visual C++ specific ###### 
#if defined(_MSC_VER) && defined(_DEBUG)
# define new DEBUG_NEW
# undef THIS_FILE
  static char THIS_FILE[] = __FILE__;
#endif 

#define LOGO_DELTA 0.05f
#define LOGO_ROT_POS 30.0f
#define LOGO_ROT_NEG 0
#define LOGO_ZOOM 0.8f

#define LOGO_SPRITE_DELTA 1.0f
//#define PI 3.14159f

#define WATER_HEIGHT_MAIN_MENU 130

#define MENU_ITEM_WIDTH 256
#define MENU_ITEM_HEIGHT 32
#define MENU_ITEM_ZOOM 1.5f
#define MENU_ITEM_PARTICLE_ZOOM 1.1f
#define MAX_PARTICLE_LIFE 50

bool eventsEnabled = false;

const char *MainMenu::menuText[] = {
	N_( "New Game" ),
	N_( "Continue Game" ),
//	N_( "Multiplayer Game" ), ...skip multiplayer for now
	N_( "Options" ),
	N_( "About S.c.o.u.r.g.e." ),
	N_( "Quit" ),
	""
};

int activeMenuItem = -1;

const int MainMenu::values[] = {
	NEW_GAME, CONTINUE_GAME, 
	// MULTIPLAYER, ...skip multiplayer for now
	OPTIONS, ABOUT, QUIT, 0
};

Texture slide;

MainMenu::MainMenu( Scourge *scourge ) {
	this->scourge = scourge;
	this->cloudCount = 30;
	this->lastMenuTick = 0;
	this->savegameDialog = new SavegameDialog( scourge );
	for ( int i = 0; i < cloudCount; i++ ) {
		cloud[i].x = Util::dice( scourge->getSDLHandler()->getScreen()->w );
		cloud[i].y = Util::dice( 50 );
		cloud[i].w = Util::pickOne( 200, 254 );
		cloud[i].h = Util::pickOne( 100, 127 );
		cloud[i].speed = Util::pickOne( 1, 2 );
	}

	//logoRot = -scourge->getShapePalette()->logo->h;
	logoRot = -173;
	logoRotDelta = LOGO_DELTA;
	logoTicks = 0;
	logoTicksDelta = 50;
	logoSpriteCount = 0;
	candleFlameX = candleFlameY = 0;
	slideMode = false;

	top = ( scourge->getSDLHandler()->getScreen()->h - 600 ) / 2;
	openingTop = scourge->getSDLHandler()->getScreen()->h / 2;
	musicStarted = false;
	lastTick = 0;
	lastTickMenu = 0;
	initTextures = false;

	starCount = 200;
	for ( int i = 0; i < starCount; i++ ) {
		star[i].x = Util::dice( scourge->getSDLHandler()->getScreen()->w );
		star[i].y = Util::pickOne( top, top + 599 - WATER_HEIGHT_MAIN_MENU - 160 );
	}
	// The new style gui
	int w = 250;
	int h = 120;
	newGameConfirm = new Window( scourge->getSDLHandler(),
	                             ( scourge->getSDLHandler()->getScreen()->w / 2 ) - ( w / 2 ),
	                             ( scourge->getSDLHandler()->getScreen()->h / 2 ) - ( h / 2 ),
	                             w, h,
	                             _( "New Game Confirmation" ),
	                             scourge->getShapePalette()->getGuiTexture(), false );
	newGameConfirmOK = newGameConfirm->createButton( 40, 55, 110, 75, Constants::getMessage( Constants::OK_LABEL ) );
	newGameConfirmCancel = newGameConfirm->createButton( 140, 55, 210, 75, Constants::getMessage( Constants::CANCEL_LABEL ) );
	newGameConfirm->createLabel( 20, 20, Constants::getMessage( Constants::DELETE_OLD_SAVED_GAME ) );
	newGameConfirm->setVisible( false );
	newGameConfirm->setModal( true );

	partyEditor = NULL;

	// about dialog
	w = 500;
	h = 350;
	aboutDialog = new Window( scourge->getSDLHandler(),
	                          ( scourge->getSDLHandler()->getScreen()->w / 2 ) - ( w / 2 ),
	                          ( scourge->getSDLHandler()->getScreen()->h / 2 ) - ( h / 2 ),
	                          w, h,
	                          _( "About S.c.o.u.r.g.e." ),
	                          scourge->getShapePalette()->getGuiTexture(), true );
	aboutText = new ScrollingLabel( 8, 0,
	                                w - 18,
	                                h - 65,
	                                scourge->getShapePalette()->getAboutText() );
	aboutDialog->addWidget( aboutText );
	aboutOK = aboutDialog->createButton( ( w / 2 ) - 40,
	                                     ( h - 55 ),
	                                     ( w / 2 ) + 40,
	                                     ( h - 35 ),
	                                     Constants::getMessage( Constants::OK_LABEL ) );
	aboutDialog->setVisible( false );


	progress = new Progress( scourge->getSDLHandler(),
	                         scourge->getSession()->getShapePalette()->getProgressTexture(),
	                         scourge->getSession()->getShapePalette()->getProgressHighlightTexture(),
	                         100, false, false );
}

MainMenu::~MainMenu() {
	for ( unsigned int i = 0; i < textEffects.size(); i++ ) {
		TextEffect *textEffect = textEffects[i];
		delete textEffect;
	}
	textEffects.clear();
	delete partyEditor;
	delete savegameDialog;
	delete newGameConfirm;
	delete aboutDialog;
	delete progress;
}

/// Draws the whole main menu view.

void MainMenu::drawView() {
	int tickNow = SDL_GetTicks();
	if ( ( tickNow - lastMenuTick ) < 15 ) SDL_Delay( 15 - ( tickNow - lastMenuTick ) );
	lastMenuTick = SDL_GetTicks();

	glsDisable( GLS_DEPTH_TEST );

	if ( !slideMode ) {
		glsDisable( GLS_CULL_FACE );
		
		drawStars();

		glsDisable( GLS_TEXTURE_2D );
		glColor4f( 1.0f, 1.0f, 1.0f, 1.0f );

		drawBackdrop();

		glsEnable( GLS_TEXTURE_2D );

		// create a stencil for the water
		glColorMask( 0, 0, 0, 0 );

		if ( scourge->getUserConfiguration()->getStencilbuf() && scourge->getUserConfiguration()->getStencilBufInitialized() ) {
			glsEnable( GLS_STENCIL_TEST );
			glStencilOp( GL_KEEP, GL_KEEP, GL_REPLACE );
			glStencilFunc( GL_ALWAYS, 1, 1 );
		}

		drawWater();

		// Use the stencil to draw
		glColorMask( GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE );

		if ( scourge->getUserConfiguration()->getStencilbuf() && scourge->getUserConfiguration()->getStencilBufInitialized() ) {
			glStencilOp( GL_KEEP, GL_KEEP, GL_KEEP );
			glStencilFunc( GL_EQUAL, 1, 0xffffffff );  // draw if stencil==1
		}

		drawClouds( false, true );

		glsDisable( GLS_STENCIL_TEST );

		// draw the blended water
		glsEnable( GLS_BLEND );
		glsDisable( GLS_DEPTH_MASK );
		glBlendFunc( GL_ONE, GL_ONE_MINUS_SRC_COLOR );
		drawWater();
		glsEnable( GLS_DEPTH_MASK );
		glsDisable( GLS_BLEND );


		drawClouds( true, false );

		glsDisable( GLS_TEXTURE_2D );
		glColor4f( 1.0f, 1.0f, 1.0f, 1.0f );

		drawScourge();

		glsEnable( GLS_TEXTURE_2D );

		drawMenu();
	}

	// draw the boards
	if ( openingTop > 0 ) {
		glPushMatrix();
		glColor3f( 1, 1, 1 );

		//    float TILE_W = 510 / 2.0f;
		float TILE_W = 256.0f;
		float TILE_H = 256.0f;
		glsEnable( GLS_TEXTURE_2D );
		Texture const& yellow = scourge->getShapePalette()->getNamedTexture( "menu" );

		glPushMatrix();
		glLoadIdentity();
		glTranslatef( 0, openingTop, 0 );
		yellow.glBind();
		glBegin( GL_TRIANGLE_STRIP );
		glTexCoord2f( 0, scourge->getSDLHandler()->getScreen()->h / TILE_H );
		glVertex2i( 0, 0 );
		glTexCoord2f( scourge->getSDLHandler()->getScreen()->w / TILE_W, scourge->getSDLHandler()->getScreen()->h / TILE_H );
		glVertex2i( scourge->getSDLHandler()->getScreen()->w, 0 );
		glTexCoord2f( 0, 0 );
		glVertex2i( 0, -scourge->getSDLHandler()->getScreen()->h / 2 );
		glTexCoord2f( scourge->getSDLHandler()->getScreen()->w / TILE_W, 0 );
		glVertex2i( scourge->getSDLHandler()->getScreen()->w, -scourge->getSDLHandler()->getScreen()->h / 2 );
		glEnd();
		glPopMatrix();

		glPushMatrix();
		glLoadIdentity();
		glTranslatef( 0, scourge->getSDLHandler()->getScreen()->h - openingTop, 0 );
		yellow.glBind();
		glBegin( GL_TRIANGLE_STRIP );
		glTexCoord2f( 0, 0 );
		glVertex2i( 0, 0 );
		glTexCoord2f( scourge->getSDLHandler()->getScreen()->w / TILE_W, 0 );
		glVertex2i( scourge->getSDLHandler()->getScreen()->w, 0 );
		glTexCoord2f( 0, scourge->getSDLHandler()->getScreen()->h / TILE_H );
		glVertex2i( 0, scourge->getSDLHandler()->getScreen()->h / 2 );
		glTexCoord2f( scourge->getSDLHandler()->getScreen()->w / TILE_W, scourge->getSDLHandler()->getScreen()->h / TILE_H );
		glVertex2i( scourge->getSDLHandler()->getScreen()->w, scourge->getSDLHandler()->getScreen()->h / 2 );
		glEnd();
		glPopMatrix();
		glsDisable( GLS_TEXTURE_2D );

		for ( int i = 0; i < 2; i++ ) {
			glLoadIdentity();
			glTranslatef( 0, ( i == 0 ? openingTop : scourge->getSDLHandler()->getScreen()->h - openingTop ), 0 );
			glColor4f( 1, 0.7f, 0, 1 );
			glBegin( GL_LINES );
			glVertex2i( 0, 0 );
			glVertex2i( scourge->getSDLHandler()->getScreen()->w, 0 );
			glEnd();
		}

		if ( slideMode ) {
			int w = scourge->getSDLHandler()->getScreen()->w;
			int h = ( scourge->getSDLHandler()->getScreen()->w / 2 ) - 1;

			glsEnable( GLS_TEXTURE_2D );
			//glPushMatrix();
			glLoadIdentity();
			glTranslatef( 0, openingTop + 1, 0 );
			slide.glBind();
			glBegin( GL_TRIANGLE_STRIP );
			glTexCoord2i( 0, 0 );
			glVertex2i( 0, 0 );
			glTexCoord2i( 1, 0 );
			glVertex2i( w, 0 );
			glTexCoord2i( 0, 1 );
			glVertex2i( 0, h );
			glTexCoord2i( 1, 1 );
			glVertex2i( w, h );
			glEnd();
			//glPopMatrix();
			glsDisable( GLS_TEXTURE_2D );
		} else {
			glLoadIdentity();
			glTranslatef( 10, scourge->getSDLHandler()->getScreen()->h - openingTop + 12, 0 );
			char version[100];
			snprintf( version, 100, _( "Scourge version %s" ), SCOURGE_VERSION );
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

			if ( openingTop > top && scourge->getSession()->isDataInitialized() ) {
				Uint32 t = SDL_GetTicks();
				if ( t - lastTick > 40 ) {
					int d = ( scourge->getSDLHandler()->getScreen()->h - openingTop ) / 20;
					openingTop -= ( 10 + static_cast<int>( d * 1.2 ) );
					if ( openingTop < top ) openingTop = top;
					lastTick = t;
				}
			}
		}
	}

	if ( openingTop <= top ) {
		drawLogo();
		if ( !musicStarted ) {
			scourge->getSession()->getSound()->playMusicMenu();
			musicStarted = true;
		}
	}

	// initialize universe (nice how this is hidden here...)
	scourge->getSession()->initData();
}

/// Draws the loading screen if still loading data.

void MainMenu::drawAfter() {
	if ( strlen( getUpdate() ) ) {
		glPushMatrix();

		/*
		glLoadIdentity();
		glsEnable( GLS_BLEND );
		glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );
		glsDisable( GLS_TEXTURE_2D );
		glColor4f( 0, 0, 0, 0.75f );
		glBegin( GL_QUADS );
		glVertex3f( 0, 0, 0 );
		glVertex3f( 0, scourge->getScreenHeight(), 0 );
		glVertex3f( scourge->getScreenWidth(), scourge->getScreenHeight(), 0 );
		glVertex3f( scourge->getScreenWidth(), 0, 0 );
		glEnd();
		glsEnable( GLS_TEXTURE_2D );
		glsDisable( GLS_BLEND );
		*/

		glLoadIdentity();
		glTranslatef( 10, scourge->getSDLHandler()->getScreen()->h - openingTop + 12, 0 );
		int y = 70;
		int x = 100;
		float maxStatus = ( scourge->getScreenWidth() - 200 ) / 20.0f;
		glColor3f( 0.8f, 0.75f, 0.65f );
		if ( getUpdateTotal() > -1 ) {
			scourge->getSDLHandler()->
			texPrint( x, y - 3, "%s: %d%%",
			          getUpdate(),
			          static_cast<int>( ( getUpdateValue() + 1 ) / ( getUpdateTotal() / 100.0f ) ) );
			glTranslatef( x + 150, y - 15, 0 );

			progress->updateStatusLight( NULL,
			                             static_cast<int>( ( getUpdateValue() + 1 ) / ( getUpdateTotal() / maxStatus ) ),
			                             static_cast<int>( maxStatus ) );
		} else {
			scourge->getSDLHandler()->texPrint( x, y - 3, getUpdate() );
		}
		glPopMatrix();
	}
	eventsEnabled = scourge->getSession()->isDataInitialized();
	if ( eventsEnabled ) scourge->getSDLHandler()->setCursorMode( Constants::CURSOR_NORMAL );
	else scourge->getSDLHandler()->setCursorMode( Constants::CURSOR_FORBIDDEN );
}

void MainMenu::show() {
	//logoRot = -scourge->getShapePalette()->logo->h;
	logoRot = -173;
	setValue( -1 );
}

void MainMenu::hide() {
	musicStarted = false;
}

/// Draws the menu.

void MainMenu::drawMenu() {

	if ( textEffects.empty() && openingTop <= top ) {
		int x = 50;
		int y = top + 230;
		for ( int i = 0; strlen( menuText[i] ); i++ ) {
			TextEffect *textEffect = new TextEffect( scourge, x - 40, y - 20, _( menuText[i] ) );
			textEffects.push_back( textEffect );
			y += 50;
		}
	}

	glsDisable( GLS_STENCIL_TEST | GLS_SCISSOR_TEST | GLS_CULL_FACE );

	for ( int i = 0; i < static_cast<int>( textEffects.size() ); i++ ) {
		if ( this->scourge->getSession()->getPreferences()->getFlaky() == false ) {
			TextEffect *textEffect = textEffects[i];
			textEffect->setActive( i == activeMenuItem );
			textEffect->draw();
		} else {
			scourge->getSDLHandler()->setFontType( Constants::SCOURGE_LARGE_FONT );
			if ( i == activeMenuItem ) {
				glColor4f( 1, 1, 0, 1 );
			} else {
				glColor4f( 1, 1, 1, 1 );
			}
			scourge->getSDLHandler()->texPrint( 50, top + 230 + ( i * 50 ), _( menuText[i] ) );
			scourge->getSDLHandler()->setFontType( Constants::SCOURGE_DEFAULT_FONT );
		}
	}
}

/// Draws the logo in the upper left.

void MainMenu::drawLogo() {

	glsEnable( GLS_TEXTURE_2D | GLS_BLEND | GLS_ALPHA_TEST );
	glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );

	//Draw the Scourge logo
	glPushMatrix();
	glLoadIdentity();
	glTranslatef( 70, logoRot, 0 );
//  float w = scourge->getShapePalette()->logo->w;
//  float h = scourge->getShapePalette()->logo->h;
	int w = 352;
	int h = 173;
	glColor4f( 1, 1, 1, 1 );
	scourge->getShapePalette()->logo_texture.glBind();

	glBegin( GL_TRIANGLE_STRIP );
	glTexCoord2i( 0, 0 );
	glVertex2i( 0, 0 );
	glTexCoord2i( 1, 0 );
	glVertex2i( w, 0 );
	glTexCoord2i( 0, 1 );
	glVertex2i( 0, h );
	glTexCoord2i( 1, 1 );
	glVertex2i( w, h );
	glEnd();
	glPopMatrix();

	for ( int i = 0; i < 2; i++ ) {
		glPushMatrix();
		glLoadIdentity();
//    glTranslatef( ( !i ? 100 :
//                    70 + scourge->getShapePalette()->logo->w - 30 -
//                    scourge->getShapePalette()->chain->w ),
//                  logoRot - scourge->getShapePalette()->chain->h, 0 );
//    float w = scourge->getShapePalette()->chain->w;
//    float h = scourge->getShapePalette()->chain->h;
		glTranslatef( ( !i ? 100 : 70 + 352 - 30 - 32 ), logoRot - 256, 0 );
		float w = 32.0f;
		float h = 256.0f;
		glColor4f( 1, 1, 1, 1 );
		scourge->getShapePalette()->chain_texture.glBind();

		glBegin( GL_TRIANGLE_STRIP );
		glTexCoord2i( 0, 0 );
		glVertex2i( 0, 0 );
		glTexCoord2i( 1, 0 );
		glVertex2i( w, 0 );
		glTexCoord2i( 0, 1 );
		glVertex2i( 0, h );
		glTexCoord2i( 1, 1 );
		glVertex2i( w, h );
		glEnd();
		glPopMatrix();
	}

	GLint t = SDL_GetTicks();
	if ( t - logoTicks > logoTicksDelta ) {
		if ( logoRot < 120 - ( ( 1024 - scourge->getScreenHeight() ) / 4 ) ) {
			logoTicks = t;
			logoRot += 8;
		}
		candleFlameX = scourge->getSDLHandler()->getScreen()->w - 215 + Util::dice( 4 ) - 4;
		candleFlameY = top + 385 + Util::dice( 4 ) - 4;
	}

	glBlendFunc( GL_SRC_COLOR, GL_ONE );

	scourge->getShapePalette()->candle.glBind();
	glColor4f( 0.7f, 0.7f, 0.3f, 0.5f );
	glPushMatrix();
	glLoadIdentity();
	glTranslatef( candleFlameX, candleFlameY, 0 );
	w = 64;
	h = 64;

	glBegin( GL_TRIANGLE_STRIP );
	glTexCoord2i( 0, 0 );
	glVertex2i( 0, 0 );
	glTexCoord2i( 1, 0 );
	glVertex2i( w, 0 );
	glTexCoord2i( 0, 1 );
	glVertex2i( 0, h );
	glTexCoord2i( 1, 1 );
	glVertex2i( w, h );
	glEnd();
	glPopMatrix();

	glsDisable( GLS_TEXTURE_2D | GLS_BLEND | GLS_ALPHA_TEST );
}

/// Draws the stars.

void MainMenu::drawStars() {
	glsDisable( GLS_TEXTURE_2D );
	for ( int i = 0; i < starCount; i++ ) {
		glPushMatrix();
		glLoadIdentity();
		glTranslatef( star[i].x, star[i].y, -200 );
		glColor3f( Util::roll( 0.2f, 0.99f ),
		           Util::roll( 0.2f, 0.99f ),
		           Util::roll( 0.2f, 0.99f ) );
		int n = 1;
		glBegin( GL_TRIANGLE_STRIP );
		glVertex2i( 0, 0 );
		glVertex2i( n, 0 );
		glVertex2i( 0, n );
		glVertex2i( n, n );
		glEnd();
		glPopMatrix();
	}
	glsEnable( GLS_TEXTURE_2D );

}

/// Draws the scourge (the house entrance to the right).

void MainMenu::drawScourge() {

	glsEnable( GLS_TEXTURE_2D | GLS_BLEND | GLS_ALPHA_TEST );
	glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );

	float w = 554; //scourge->getShapePalette()->scourge->w;
	float h = 600; //scourge->getShapePalette()->scourge->h;

	//Draw the scourge
	glPushMatrix();
	glLoadIdentity();
	glTranslatef( scourge->getSDLHandler()->getScreen()->w - w, top, 0 );
	glColor4f( 1, 1, 1, 1 );
	scourge->getShapePalette()->getNamedTexture( "scourge" ).glBind();

	glBegin( GL_TRIANGLE_STRIP );
	glTexCoord2i( 0, 0 );
	glVertex2i( 0, 0 );
	glTexCoord2i( 1, 0 );
	glVertex2i( w, 0 );
	glTexCoord2i( 0, 1 );
	glVertex2i( 0, h );
	glTexCoord2i( 1, 1 );
	glVertex2i( w, h );
	glEnd();
	glPopMatrix();

	glsDisable( GLS_TEXTURE_2D | GLS_BLEND | GLS_ALPHA_TEST );
}

/// Draws the mountains in the back.

void MainMenu::drawBackdrop() {
	glsEnable( GLS_TEXTURE_2D | GLS_BLEND | GLS_ALPHA_TEST );
	glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );

//  float w = scourge->getShapePalette()->scourgeBackdrop->w;
	float w = scourge->getSDLHandler()->getScreen()->w;
	//float h = scourge->getShapePalette()->scourgeBackdrop->h;
	float h = 256.0f;

	//Draw the backdrop image
	glPushMatrix();
	glLoadIdentity();
	glTranslatef( 0, top + ( 600 - WATER_HEIGHT_MAIN_MENU - h ), 0 );
	glColor4f( 1, 1, 1, 1 );
	scourge->getShapePalette()->scourgeBackdrop_texture.glBind();

	glBegin( GL_TRIANGLE_STRIP );
	glTexCoord2i( 0, 0 );
	glVertex2i( 0, 0 );
	glTexCoord2i( 1, 0 );
	glVertex2i( w, 0 );
	glTexCoord2i( 0, 1 );
	glVertex2i( 0, h );
	glTexCoord2i( 1, 1 );
	glVertex2i( w, h );
	glEnd();
	glPopMatrix();

	glsDisable( GLS_TEXTURE_2D | GLS_BLEND | GLS_ALPHA_TEST );

}

/// Draws the clouds.

void MainMenu::drawClouds( bool moveClouds, bool flipped ) {
	// draw clouds
	int w, h;

	glsEnable( GLS_TEXTURE_2D | GLS_BLEND | GLS_ALPHA_TEST );
	glBlendFunc( GL_ONE_MINUS_DST_COLOR, GL_ONE );

	scourge->getShapePalette()->cloud.glBind();

	for ( int i = 0; i < cloudCount; i++ ) {
		w = cloud[i].w;
		h = cloud[i].h;
		glPushMatrix();
		glTranslatef( cloud[i].x,
		              top + ( flipped ? 600 - ( cloud[i].y + h / 2.0 ) : cloud[i].y + 130 ),
		              0 );

		glBegin( GL_TRIANGLE_STRIP );
		if ( flipped ) {
			glColor4f( 0.1f, 0.1f, 0.3f, 0.5f );
		} else {
			glColor4f( 1, 1, 1, 1 );
		}
		glTexCoord2i( 0, ( flipped ? 1 : 0 ) );
		glVertex2i( 0, 0 );

		if ( flipped ) {
			glColor4f( 0.1f, 0.1f, 0.3f, 0.5f );
		} else {
			glColor4f( 1, 1, 1, 1 );
		}		
		glTexCoord2i( 1, ( flipped ? 1 : 0 ) );
		glVertex2i( w, 0 );
		
		glColor4f( 1, 0.3f, 0, 0.5f );
		glTexCoord2i( 0, ( flipped ? 0 : 1 ) );
		glVertex2i( 0, ( flipped ? h / 2 : h ) );
		
		glColor4f( 1, 0.3f, 0, 0.5f );
		glTexCoord2i( 1, ( flipped ? 0 : 1 ) );
		glVertex2i( w, ( flipped ? h / 2 : h ) );
		glEnd();
		glPopMatrix();

		if ( moveClouds ) {
			cloud[i].x += cloud[i].speed;
			if ( cloud[i].x >= scourge->getSDLHandler()->getScreen()->w ) {
				cloud[i].x = -cloud[i].w;
			}
		}
	}

	glsDisable( GLS_TEXTURE_2D | GLS_BLEND | GLS_ALPHA_TEST );
}

/// Draws the water.

void MainMenu::drawWater() {
	int w, h;
	// draw the water
	glPushMatrix();
	w = scourge->getSDLHandler()->getScreen()->w;
	h = WATER_HEIGHT_MAIN_MENU;
	glLoadIdentity();
	glTranslatef( 0, top + ( 600 - h ), 0 );
	glsDisable( GLS_TEXTURE_2D );
	//  glDisable( GL_LIGHTING );
	//glsEnable( GLS_BLEND );
	//glBlendFunc( GL_ONE_MINUS_DST_COLOR, GL_ONE );

	glBegin( GL_TRIANGLE_STRIP );
	glColor4f( 0, 0, 0.1f, 1 );
	glVertex2i( 0, 0 );
	glColor4f( 0, 0, 0.1f, 1 );
	glVertex2i( w, 0 );
	glColor4f( 0, 0.1f, 0.4f, 1 );
	glVertex2i( 0, h );
	glColor4f( 0, 0.1f, 0.4f, 1 );
	glVertex2i( w, h );
	glEnd();
	//glsDisable( GLS_BLEND );
	glPopMatrix();
}

bool MainMenu::handleEvent( Widget *widget, SDL_Event *event ) {

	if ( !eventsEnabled ) return false;

	if ( aboutDialog->isVisible() ) {
		if ( widget == aboutOK || widget == aboutDialog->closeButton ) {
			aboutDialog->setVisible( false );
		}
		return false;
	}

//	if ( scourge->getMultiplayerDialog()->isVisible() ) {
//		scourge->getMultiplayerDialog()->handleEvent( widget, event );
//		if ( !scourge->getMultiplayerDialog()->isVisible() ) {
//			if ( scourge->getMultiplayerDialog()->getValue() == MultiplayerDialog::START_SERVER ) {
//				value = MULTIPLAYER_START;
//				return true;
//			} else if ( scourge->getMultiplayerDialog()->getValue() == MultiplayerDialog::JOIN_SERVER ) {
//				if ( !( strlen( scourge->getMultiplayerDialog()->getServerName() ) &&
//				        strlen( scourge->getMultiplayerDialog()->getServerPort() ) &&
//				        strlen( scourge->getMultiplayerDialog()->getUserName() ) ) ) {
//					scourge->showMessageDialog( Constants::getMessage( Constants::JOIN_SERVER_ERROR ) );
//				} else {
//					value = MULTIPLAYER_START;
//					return true;
//				}
//			}
//		}
//		return false;
//	}

	return false;
}

bool MainMenu::handleEvent( SDL_Event *event ) {

	if ( !eventsEnabled ) return false;

	if ( aboutDialog->isVisible() ) {
		return false;
	}

	if ( scourge->getSaveDialog()->getWindow()->isVisible() ) {
		return false;
	}

	if ( savegameDialog->getWindow()->isVisible() ) {
//		savegameDialog->handleEvent( NULL, event );
		return false;
	}

	if ( scourge->getOptionsMenu()->isVisible() ) {
//		scourge->getOptionsMenu()->handleEvent( event );
		return false;
	}

	if ( scourge->getMultiplayerDialog()->isVisible() ) {
//		scourge->getMultiplayerDialog()->handleEvent( event );
		return false;
	}

	if ( partyEditor && partyEditor->isVisible() ) {
//		partyEditor->handleEvent( NULL, event );
		return false;
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

	switch ( event->type ) {
		//case SDL_KEYDOWN:
	case SDL_KEYUP:
		switch ( event->key.keysym.sym ) {
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
		if ( event->key.keysym.sym == '8' ) {
			Scourge::blendA++; if ( Scourge::blendA >= 11 ) Scourge::blendA = 0;
			fprintf( stderr, "blend: a=%d b=%d\n", Scourge::blendA, Scourge::blendB );
		} else if ( event->key.keysym.sym == '9' ) {
			Scourge::blendB++; if ( Scourge::blendB >= 11 ) Scourge::blendB = 0;
			fprintf( stderr, "blend: a=%d b=%d\n", Scourge::blendA, Scourge::blendB );
		}
		break;
	case SDL_MOUSEMOTION:
	case SDL_MOUSEBUTTONUP:
		if ( event->motion.x >= 50 && event->motion.x < 400 ) {
			if ( this->scourge->getSession()->getPreferences()->getFlaky() == false ) {
				activeMenuItem = ( event->motion.y - ( top + 240 ) ) / 50;
			} else {
				activeMenuItem = ( event->motion.y - ( top + 230 + SDLHandler::fontInfos[ Constants::SCOURGE_LARGE_FONT ]->yoffset ) ) / 50;
			}
		}
		if ( event->button.button != SDL_BUTTON_LEFT ) return false;
		if ( event->type == SDL_MOUSEBUTTONUP && activeMenuItem > -1 && activeMenuItem < static_cast<int>( textEffects.size() ) ) {
			value = values[activeMenuItem];
			if ( value == ABOUT ) {
				aboutDialog->setVisible( true );
				//value = ABOUT;
				return false;
			} else if ( value == NEW_GAME ) {
				showPartyEditor();
				return false;
			} else if ( value == CONTINUE_GAME ) {
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

/// Returns the selected menu entry.

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
	if ( !partyEditor ) partyEditor = new PartyEditor( scourge );
	partyEditor->setVisible( true );
}

void MainMenu::createParty( Creature **pc, int *partySize ) {
	if ( !partyEditor ) partyEditor = new PartyEditor( scourge );
	partyEditor->createParty( pc, partySize );
}

RenderedCreature *MainMenu::createWanderingHero( int level ) {
	if ( !partyEditor ) partyEditor = new PartyEditor( scourge );
	return partyEditor->createWanderingHero( level );
}

void MainMenu::setSlideMode( bool b ) {
  slideMode = b;

  if ( slideMode ) {
    openingTop = ( scourge->getSDLHandler()->getScreenHeight() / 2 ) - ( scourge->getSDLHandler()->getScreenWidth() / 4 );
    slide = scourge->getShapePalette()->getRandomSlide();

  } else {
    openingTop = scourge->getSDLHandler()->getScreenHeight() / 2;
  }
}
