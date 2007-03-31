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
#include "pcui.h"

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

Portrait::Portrait( PcUi *pcUi, int x, int y, int w, int h ) {
	this->pcUi = pcUi;
	this->creature = NULL;
	this->backgroundTexture = pcUi->getScourge()->getShapePalette()->getNamedTexture( "portrait" );
	this->barTexture = pcUi->getScourge()->getShapePalette()->getNamedTexture( "bar" );
	this->x = x;
	this->y = y;
	this->w = w;
	this->h = h;
	this->mode = STATS_MODE;
	this->skillOffset = 1;

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
		pcUi->getScourge()->drawPortrait( creature, 62, 77 );
		glPopMatrix();
	}

	int y = 25;
	glColor4f( 1, 0.8f, 0, 1 );
	pcUi->getScourge()->getSDLHandler()->setFontType( Constants::SCOURGE_LARGE_FONT );
	pcUi->getScourge()->getSDLHandler()->texPrint( 80, y, creature->getName() );
	pcUi->getScourge()->getSDLHandler()->setFontType( Constants::SCOURGE_DEFAULT_FONT );
	y += 17;

	glColor4f( 1, 1, 1, 1 );
	pcUi->getScourge()->getSDLHandler()->texPrint( 80, y, "%s %s", 
																			( creature->getSex() == Constants::SEX_FEMALE ? _( "Female" ) : _( "Male" ) ),
																			creature->getCharacter()->getDisplayName() );
	y += 15;
	pcUi->getScourge()->getSDLHandler()->texPrint( 80, y, "%s:%d %s:%d", _( "Level" ), creature->getLevel(), _( "XP" ), creature->getExp() );

	y += 15;
	pcUi->getScourge()->describeAttacks( creature, 80, y, true );
	pcUi->getScourge()->describeDefense( creature, 200, y );
	glColor4f( 1, 1, 1, 1 );

	if( mode == STATS_MODE ) {
		showStats();
	} else {
		showSkills();
	}

	glDisable( GL_TEXTURE_2D );

}

void Portrait::showStats() {
	// hp/mp
  int y = 117;	
//	y += 18;
	drawHorizontalLine( y - 12 );
	glColor4f( 1, 1, 1, 1 );
  pcUi->getScourge()->getSDLHandler()->texPrint( 10, y, _( "HP" ) );
  drawBar( 120, y - 10, creature->getHp(), creature->getMaxHp(), 1, 0, 0, 1 );
  pcUi->getScourge()->getSDLHandler()->texPrint( 240, y, "%d/%d", creature->getHp(), creature->getMaxHp() );

  y += 15;
  pcUi->getScourge()->getSDLHandler()->texPrint( 10, y, _( "MP" ) );
  drawBar( 120, y - 10, creature->getMp(), creature->getMaxMp(), 0, 0, 1, 1 );
  pcUi->getScourge()->getSDLHandler()->texPrint( 240, y, "%d/%d", creature->getMp(), creature->getMaxMp() );

	y += 15;
  pcUi->getScourge()->getSDLHandler()->texPrint( 10, y, _( "AP" ) );
	int maxAp = (int)creature->getMaxAP();
	int ap = ( pcUi->getScourge()->inTurnBasedCombatPlayerTurn() ? creature->getBattle()->getAP() : maxAp );
  drawBar( 120, y - 10, ap, maxAp, 1, 0, 1, 1 );
  pcUi->getScourge()->getSDLHandler()->texPrint( 240, y, "%d/%d", ap, maxAp );

  y += 15;
  drawHorizontalLine( y - 9 );
  
	// stats
  y += 5;
	for( int i = 0; i < SkillGroup::stats->getSkillCount(); i++ ) {
		int yy = y + ( i * 15 );
		Skill *skill = SkillGroup::stats->getSkill( i );
		drawSkill( skill, yy );
	}
}

void Portrait::scrollSkillsUp() {
	skillOffset--;
	if( skillOffset < 1 ) {
		skillOffset = (int)SkillGroup::groups.size() - 1;
	}
}

void Portrait::scrollSkillsDown() {
	skillOffset++;
	if( skillOffset >= (int)SkillGroup::groups.size() ) {
		skillOffset = 1;
	}
}

void Portrait::showSkills() {
	int y = 110;
	glColor4f( 1, 1, 1, 1 );
	SkillGroup *sg = SkillGroup::groups[ skillOffset ];
	pcUi->getScourge()->getSDLHandler()->texPrint( 10, y, "%s:%d",
																								 _( "Available Skill Points" ), 
																								 creature->getAvailableSkillMod() );
	drawHorizontalLine( y + 4 );
	y += 18;

	glColor4f( 1, 0.35f, 0, 1 );
	pcUi->getScourge()->getSDLHandler()->texPrint( 160, y, sg->getDisplayName() );
	glColor4f( 1, 1, 1, 1 );

	for( int i = 0; i < sg->getSkillCount(); i++, y += 30 ) {
		Skill *skill = sg->getSkill( i );
		glColor4f( 1, 1, 1, 1 );
		drawSkill( skill, y );
	}
}

// #define SKILL_NAME_LENGTH 16
#define SKILL_BUTTON_SIZE 11
void Portrait::drawSkill( Skill *skill, int yy ) {
	int bonus = creature->getSkillBonus( skill->getIndex() );
	int value = creature->getSkill( skill->getIndex() ) - bonus;
	/*
	char tmp[SKILL_NAME_LENGTH + 1];
	strncpy( tmp, skill->getDisplayName(), SKILL_NAME_LENGTH );
	tmp[SKILL_NAME_LENGTH] = '\0';
	if( strlen( skill->getDisplayName() ) > SKILL_NAME_LENGTH ) {
		strcat( tmp, "." );
	}
	*/
	pcUi->getScourge()->getSDLHandler()->texPrint( 10, yy, skill->getDisplayName() );
	
	

	if( !skill->getGroup()->isStat() ) {
		yy += 15;
		x = 50;
		drawBar( x, yy - 10, value, ( skill->getGroup()->isStat() ? 20 : 100 ), 0, 1, 0, 1, bonus );
		if( bonus > 0 ) {
			pcUi->getScourge()->getSDLHandler()->texPrint( x + 130, yy, "%d(%d)", ( value + bonus ), bonus );
		} else {
			pcUi->getScourge()->getSDLHandler()->texPrint( x + 130, yy, "%d", value );
		}
		if( !skill->getGroup()->isStat() ) {
			for( int i = 0; i < 2; i++ ) {
				if( creature->getAvailableSkillMod() > 0 ) glColor3f( 1, 1, 1 );
				else glColor3f( 0.5, 0.5, 0.5 );
				glPushMatrix();
				int xx = ( i == 0 ? x - 20 : x + 115 );
				glBegin( GL_LINE_LOOP );
				glVertex2f( xx, yy + SKILL_BUTTON_SIZE - 10 );
				glVertex2f( xx, yy - 10 );
				glVertex2f( xx + SKILL_BUTTON_SIZE, yy - 10 );
				glVertex2f( xx + SKILL_BUTTON_SIZE, yy + SKILL_BUTTON_SIZE - 10 );
				glEnd();
				pcUi->getScourge()->getSDLHandler()->texPrint( xx + 1, yy, ( i == 0 ? "-" : "+" ) );
				glPopMatrix();
			}
		}
	} else {
		drawBar( 120, yy - 10, value, ( skill->getGroup()->isStat() ? 20 : 100 ), 0, 1, 0, 1, bonus );
		if( bonus > 0 ) {
			pcUi->getScourge()->getSDLHandler()->texPrint( 240, yy, "%d(%d)", ( value + bonus ), bonus );
		} else {
			pcUi->getScourge()->getSDLHandler()->texPrint( 240, yy, "%d", value );
		}
	}
}

void Portrait::drawBar( int x, int y, int value, int maxValue, int r, int g, int b, int a, int mod ) {
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
		if( v > 0 ) {
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
	}

	v = value;
	if( v > maxValue ) v = maxValue;
	if( v > 0 ) {
		n = (int)( (float)v * (float)BAR_INNER_WIDTH / (float)maxValue );
		glDisable( GL_TEXTURE_2D );
		glColor4f( r, g, b, a );
		glBegin( GL_QUADS );
		glVertex2d( BAR_X, BAR_Y + BAR_INNER_HEIGHT );
		glVertex2d( BAR_X, BAR_Y );
		glVertex2d( BAR_X + n, BAR_Y );
		glVertex2d( BAR_X + n, BAR_Y + BAR_INNER_HEIGHT );
		glEnd();
	}

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

void Portrait::drawHorizontalLine( int y ) {
  glLineWidth( 2 );
  glDisable( GL_TEXTURE_2D );
  //pcUi->getWindow()->setTopWindowBorderColor();
	glEnable( GL_BLEND );
  glBlendFunc( GL_SRC_COLOR, GL_DST_COLOR );
  glBegin( GL_LINES );
  glColor4f( 0.45, 0.25f, 0, 0.75f );
  glVertex2d( 20, y );
  glColor4f( 1, 0.75f, 0, 0.75f );
  glVertex2d( ( canvas->getWidth() - 20 ) / 2, y );
  glVertex2d( ( canvas->getWidth() - 20 ) / 2, y );
  glColor4f( 0.45, 0.25f, 0, 0.75f );
  glVertex2d( canvas->getWidth() - 20, y );
  glEnd();
  glDisable( GL_BLEND );
  glColor4f( 1, 1, 1, 1 );
  glEnable( GL_TEXTURE_2D );
  glLineWidth( 1 );
}
