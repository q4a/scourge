/***************************************************************************
                          portrait.cpp  -  description
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
#include "portrait.h"
#include "render/renderlib.h"
#include "rpg/rpglib.h"
#include "sound.h"
#include "sdlhandler.h"
#include "sdleventhandler.h"
#include "sdlscreenview.h"
#include "scourge.h"
#include "item.h"
#include "creature.h"
#include "tradedialog.h"
#include "sqbinding/sqbinding.h"
#include "characterinfo.h"
#include "shapepalette.h"
#include "skillsview.h"
#include "gui/confirmdialog.h"

/**
  *@author Gabor Torok
  */

using namespace std;

#define OFFSET_X 5
#define OFFSET_Y 5

#define BAR_WIDTH 110
#define BAR_HEIGHT 12
#define BAR_X 4
#define BAR_Y 4
#define BAR_INNER_WIDTH 102
#define BAR_INNER_HEIGHT 4

Portrait::Portrait( Scourge *scourge, Window *window, int x, int y, int w, int h ) {
	this->scourge = scourge;
	this->creature = NULL;
	this->backgroundTexture = scourge->getShapePalette()->getNamedTexture( "portrait" );
	this->barTexture = scourge->getShapePalette()->getNamedTexture( "bar" );
	this->window = window;
	this->x = x;
	this->y = y;
	this->w = w;
	this->h = h;

	canvas = new Canvas( x, y, x + w, y + h, this );
	canvas->setDrawBorders( false );
}

Portrait::~Portrait() {
}

bool Portrait::handleEvent( SDL_Event *event ) {
	return false;
}

bool Portrait::handleEvent( Widget *widget, SDL_Event *event ) {
  return false;
}

void Portrait::drawWidgetContents( Widget *widget ) {
	glEnable( GL_TEXTURE_2D );
	glBindTexture( GL_TEXTURE_2D, backgroundTexture );
	glColor4f( 1, 1, 1, 1 );
	glBegin( GL_QUADS );
	glTexCoord2d( 0, 1 );
	glVertex2d( 0, h );
	glTexCoord2d( 0, 0 );
	glVertex2d( 0, 0 );
	glTexCoord2d( 1, 0 );
	glVertex2d( w, 0 );
	glTexCoord2d( 1, 1 );
	glVertex2d( w, h );
	glEnd();

	
	if( creature ) {
		glPushMatrix();
		glTranslatef( 12, 14, 0 );
		scourge->drawPortrait( creature, 62, 77 );
		glPopMatrix();
	}

	glColor4f( 1, 0.8f, 0, 1 );
	scourge->getSDLHandler()->setFontType( Constants::SCOURGE_LARGE_FONT );
	scourge->getSDLHandler()->texPrint( 80, 30, creature->getName() );
	scourge->getSDLHandler()->setFontType( Constants::SCOURGE_DEFAULT_FONT );

	glColor4f( 1, 1, 1, 1 );
	scourge->getSDLHandler()->texPrint( 80, 50, "%s %s (%d)", 
																			( creature->getSex() == Constants::SEX_FEMALE ? _( "Female" ) : _( "Male" ) ),
																			creature->getCharacter()->getDisplayName(),
																			creature->getLevel() );
	scourge->getSDLHandler()->texPrint( 80, 65, "%s:%d %s:%d", _( "Level" ), creature->getLevel(), _( "XP" ), creature->getExp() );

	// stats
	for( int i = 0; i < SkillGroup::stats->getSkillCount(); i++ ) {
		int y = 110 + ( i * 15 );
		Skill *skill = SkillGroup::stats->getSkill( i );
		int bonus = creature->getSkillBonus( skill->getIndex() );
		int value = creature->getSkill( skill->getIndex() ) - bonus;
		scourge->getSDLHandler()->texPrint( 10, y, skill->getDisplayName() );
		drawBar( 110, y - 10, value, 20, bonus );
		if( bonus > 0 ) {
			scourge->getSDLHandler()->texPrint( 230, y, "%d(%d)", ( value + bonus ), bonus );
		} else {
			scourge->getSDLHandler()->texPrint( 230, y, "%d", value );
		}
	}

	glPushMatrix();
	glTranslatef( OFFSET_X, OFFSET_Y, 0 );
	
	glPopMatrix();
	glDisable( GL_TEXTURE_2D );
}

void Portrait::drawBar( int x, int y, int value, int maxValue, int mod ) {
	glPushMatrix();
	glEnable( GL_TEXTURE_2D );
	glTranslatef( x, y, 0 );
	glBindTexture( GL_TEXTURE_2D, barTexture );
	glColor4f( 1, 1, 1, 1 );
	glBegin( GL_QUADS );
	glTexCoord2d( 0, 1 );
	glVertex2d( 0, BAR_HEIGHT );
	glTexCoord2d( 0, 0 );
	glVertex2d( 0, 0 );
	glTexCoord2d( 1, 0 );
	glVertex2d( BAR_WIDTH, 0 );
	glTexCoord2d( 1, 1 );
	glVertex2d( BAR_WIDTH, BAR_HEIGHT );
	glEnd();	

	glDisable( GL_TEXTURE_2D );

	int v, n;
	if( mod > 0 ) {
		v = value + mod;
		if( v > maxValue ) v = maxValue;
		n = (int)( (float)v * (float)BAR_INNER_WIDTH / (float)maxValue );
		glDisable( GL_TEXTURE_2D );
		glColor4f( 0, 0.5f, 1, 1 );
		glBegin( GL_QUADS );
		glVertex2d( BAR_X, BAR_Y + BAR_INNER_HEIGHT );
		glVertex2d( BAR_X, BAR_Y );
		glVertex2d( BAR_X + n, BAR_Y );
		glVertex2d( BAR_X + n, BAR_Y + BAR_INNER_HEIGHT );
		glEnd();	
	}

	v = value;
	if( v > maxValue ) v = maxValue;
	n = (int)( (float)v * (float)BAR_INNER_WIDTH / (float)maxValue );
	glDisable( GL_TEXTURE_2D );
	glColor4f( 0, 1, 0, 1 );
	glBegin( GL_QUADS );
	glVertex2d( BAR_X, BAR_Y + BAR_INNER_HEIGHT );
	glVertex2d( BAR_X, BAR_Y );
	glVertex2d( BAR_X + n, BAR_Y );
	glVertex2d( BAR_X + n, BAR_Y + BAR_INNER_HEIGHT );
	glEnd();

	glEnable( GL_BLEND );
	glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );
	glBegin( GL_QUADS );
	glColor4f( 0, 0, 0, 0.2f );
	glVertex2d( BAR_X, BAR_Y + BAR_INNER_HEIGHT );
	glColor4f( 0, 0, 0, 0.9f );
	glVertex2d( BAR_X, BAR_Y );
	glColor4f( 0, 0, 0, 0.85f );
	glVertex2d( BAR_X + BAR_INNER_WIDTH, BAR_Y );
	glColor4f( 0, 0, 0, 0.2f );
	glVertex2d( BAR_X + BAR_INNER_WIDTH, BAR_Y + BAR_INNER_HEIGHT );
	glEnd();
	glDisable( GL_BLEND );

	glColor4f( 1, 1, 1, 1 );
	glPopMatrix();
}

void Portrait::setCreature( Creature *creature ) { 
	this->creature = creature; 
}

