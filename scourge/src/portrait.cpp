/***************************************************************************
                          portrait.cpp  -  description
                             -------------------
    begin                : Sat May 3 2003
    copyright            : (C) 2003 by Gabor Torok
    email                : cctorok@yahoo.co
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
#define RESISTANCE_WIDTH 46

char *resistanceIcons[] = {
	"nature", "divine", "life", "history", "tricks", "confrontation"
};
int resistanceSkills[] = {
	Skill::RESIST_NATURE_MAGIC,
	Skill::RESIST_AWARENESS_MAGIC,
	Skill::RESIST_LIFE_AND_DEATH_MAGIC,
	Skill::RESIST_HISTORY_MAGIC,
	Skill::RESIST_DECEIT_MAGIC,
	Skill::RESIST_CONFRONTATION_MAGIC
};
int resistanceCount = 6;

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
	this->currentSkill = NULL;
	this->currentMode = 0;

	canvas = new Canvas( x, y, x + w, y + h, this );
	canvas->setDrawBorders( false );
}

Portrait::~Portrait() {
  for( map<string,ActionRect*>::iterator e = boxes.begin(); e != boxes.end(); ++e ) {
		ActionRect *rect = e->second;
		delete rect;
	}
	boxes.clear();
}

string getBoxKey( Skill *skill, int mode ) {
  stringstream tmp;
  tmp << skill->getName() << "," << mode;
  return tmp.str();
}

bool Portrait::findCurrentSkill( int px, int py ) {
	SkillGroup *sg = SkillGroup::groups[ skillOffset ];
	currentSkill = NULL;
	currentMode = 0;
	for( int i = 0; i < sg->getSkillCount(); i++ ) {
			 for( int t = 0; t < 2; t++ ) {
			 			string key = getBoxKey( sg->getSkill( i ), t );
			 			if( boxes.find( key ) != boxes.end() )  {
								ActionRect *rect = boxes[ key ];
								if( rect->containsPoint( px, py ) ) {
							     currentSkill = sg->getSkill( i );
							     currentMode = t;
							     return true;
								}
						 }
				}
	}
	return false;
}		 

bool Portrait::handleEvent( SDL_Event *event ) {
	if( event->type == SDL_MOUSEMOTION ) {
		int mx = event->motion.x - pcUi->getWindow()->getX() - x;
		int my = event->motion.y - pcUi->getWindow()->getY() - y - TITLE_HEIGHT;
		if( creature && mx >= 80 && mx < 200 && my >= 55 && my < 90 ) {
			setCurrentWeaponTooltip();
		} else {
			if( mode == SKILLS_MODE ) {
				findCurrentSkill( mx, my );
			} else if( mode == STATS_MODE ) {
				int index = -1;
				if( my >= 250 ) {
					index = ( mx - 10 ) / RESISTANCE_WIDTH;
				}
				canvas->setTooltip( index > -1 && index < resistanceCount ? 
														Skill::skills[ resistanceSkills[ index ] ]->getDisplayName() : 
														(char*)"" );
			}
		}
	}
	return false;
}

bool Portrait::handleEvent( Widget *widget, SDL_Event *event ) {
	if( pcUi->getScourge()->getSDLHandler()->mouseButton == SDL_BUTTON_LEFT && 
			event->type == SDL_MOUSEBUTTONUP ) {
		int mx = event->button.x - pcUi->getWindow()->getX() - x;
		int my = event->button.y - pcUi->getWindow()->getY() - y - TITLE_HEIGHT;
		if( creature && mx >= 80 && mx < 200 && my >= 55 && my < 90 ) {
			if( creature->nextPreferredWeapon() ) {
				// reset but don't pause again
				creature->getBattle()->reset( true, true );
				setCurrentWeaponTooltip();
			}
		} else if( mode == SKILLS_MODE ) {
			if( findCurrentSkill( event->button.x - pcUi->getWindow()->getX() - x, 
														event->button.y - pcUi->getWindow()->getY() - y - TITLE_HEIGHT ) ) {
				if( currentMode == 0 && creature->getSkillMod( currentSkill->getIndex() ) > 0 ) {
					creature->setAvailableSkillMod( creature->getAvailableSkillMod() + 1 );
					creature->setSkillMod( currentSkill->getIndex(), creature->getSkillMod( currentSkill->getIndex() ) - 1 );
				} else if( currentMode == 1 && creature->getAvailableSkillMod() > 0 ) {
					creature->setAvailableSkillMod( creature->getAvailableSkillMod() - 1 );
					creature->setSkillMod( currentSkill->getIndex(), creature->getSkillMod( currentSkill->getIndex() ) + 1 );
				}
			}
		}
  }
  return false;
}

void Portrait::setCurrentWeaponTooltip() {
	if( creature ) {
		char tmp[ 1000 ];
		sprintf( tmp, "%s:%s (%s)", 
						 _( "Current attack" ), 
						 ( Constants::INVENTORY_LEFT_HAND == creature->getPreferredWeapon() ? _( "Left Hand" ) :
							 ( Constants::INVENTORY_RIGHT_HAND == creature->getPreferredWeapon() ? _( "Right Hand" ) :
								 ( Constants::INVENTORY_WEAPON_RANGED == creature->getPreferredWeapon() ? _( "Ranged Weapon" ) :
									 _( "Bare Hands" ) ) ) ),
						 _( "Click to change" ) );
		canvas->setTooltip( tmp );
	}
}

void Portrait::drawWidgetContents( Widget *widget ) {
	glEnable( GL_TEXTURE_2D );
	glEnable( GL_BLEND );
	glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );
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
	glDisable( GL_BLEND );
	
	if( creature ) {
		glScissor( pcUi->getWindow()->getX() + x + 12, 
							 pcUi->getScourge()->getScreenHeight() - ( pcUi->getWindow()->getY() + y + TITLE_HEIGHT + 14 + 77 ), 
							 62, 77 );
		glEnable( GL_SCISSOR_TEST );
		glPushMatrix();
		glTranslatef( 12, 14, 0 );
		pcUi->getScourge()->drawPortrait( creature, 62 + 20, 77, 10 );
		glPopMatrix();
		glDisable( GL_SCISSOR_TEST );
	}

	int y = 25;
	glColor4f( 1, 0.8f, 0, 1 );
	pcUi->getScourge()->getSDLHandler()->setFontType( Constants::SCOURGE_LARGE_FONT );
	pcUi->getScourge()->getSDLHandler()->texPrint( 80, y, creature->getName() );
	pcUi->getScourge()->getSDLHandler()->setFontType( Constants::SCOURGE_DEFAULT_FONT );
	//y += 17;

	glColor4f( 1, 1, 1, 1 );
	y = 15;
	int x = 200;
	pcUi->getScourge()->getSDLHandler()->texPrint( x, y, "%s:%d", _( "Level" ), creature->getLevel() );
	y += 15;
	pcUi->getScourge()->getSDLHandler()->texPrint( x, y, "%s:%d", _( "XP" ), creature->getExp() );
	y += 15;
	pcUi->getScourge()->getSDLHandler()->texPrint( x, y, "%s:%d", _( "Coins" ), creature->getMoney() );

	y = 25 + 17;
	pcUi->getScourge()->getSDLHandler()->texPrint( 80, y, "%s %s", 
																								 ( creature->getSex() == Constants::SEX_FEMALE ? _( "Female" ) : _( "Male" ) ),
																								 creature->getCharacter()->getDisplayName() );	
	y += 25;

	pcUi->getScourge()->describeAttacks( creature, 80, y, true );
	pcUi->getScourge()->describeDefense( creature, 200, y );
	glColor4f( 1, 1, 1, 1 );

	if( mode == STATS_MODE ) {
		showStats();
	} else if( mode == SKILLS_MODE ) {
		showSkills();
	} else {
		showStateMods();
	}

	glDisable( GL_TEXTURE_2D );

}

void Portrait::showStats() {
	// hp/mp
  int y = 105;	
//	y += 18;
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
	y += SkillGroup::stats->getSkillCount() * 15;

	// resistances y=250
	glColor4f( 1, 0.35f, 0, 1 );
	pcUi->getScourge()->getSDLHandler()->texPrint( 10, y - 2, "%s:", _( "Resistances" ) );
	glColor4f( 1, 1, 1, 1 );
	int x = 5;
	glEnable( GL_TEXTURE_2D );
	for( int i = 0; i < resistanceCount; i++ ) {
		drawResistance( x, y, resistanceIcons[ i ], resistanceSkills[ i ] ); 
		x += RESISTANCE_WIDTH;
	}
}

void Portrait::drawResistance( int x, int y, char *icon, int skill ) {
	int size = 20;
	glPushMatrix();
	glTranslatef( x, y, 0 );
	glEnable( GL_ALPHA_TEST );
	glAlphaFunc( GL_NOTEQUAL, 0 );
	glBindTexture( GL_TEXTURE_2D, pcUi->getScourge()->getShapePalette()->getNamedTexture( icon ) );
	glColor4f( 1, 1, 1, 1 );
	glBegin( GL_QUADS );
	glTexCoord2d( 0, 1 );
	glVertex2d( 0, size );
	glTexCoord2d( 0, 0 );
	glVertex2d( 0, 0 );
	glTexCoord2d( 1, 0 );
	glVertex2d( size, 0 );
	glTexCoord2d( 1, 1 );
	glVertex2d( size, size );
	glEnd();
	glDisable( GL_ALPHA_TEST );
	if( creature ) {
		pcUi->getScourge()->getSDLHandler()->texPrint( size, 15, "%d%%", creature->getSkill( skill, false ) );
	}
	glPopMatrix();
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
	if( creature->getHasAvailableSkillPoints() ) glColor3f( 1, 0, 0 );
	else glColor3f( 1, 1, 1 );
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
#define SKILL_BUTTON_SIZE 10
void Portrait::drawSkill( Skill *skill, int yy ) {
	int bonus = creature->getSkillBonus( skill->getIndex() ) + creature->getSkillMod( skill->getIndex() );
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
		int x = 80;
		drawBar( x, yy - 10, value, ( skill->getGroup()->isStat() ? 20 : 100 ), 0, 1, 0, 1, bonus );
		if( bonus > 0 ) {
			pcUi->getScourge()->getSDLHandler()->texPrint( x + 130, yy, "%d(%d)", ( value + bonus ), bonus );
		} else {
			pcUi->getScourge()->getSDLHandler()->texPrint( x + 130, yy, "%d", value );
		}
		if( !skill->getGroup()->isStat() ) {
			for( int i = 0; i < 2; i++ ) {
				
				int dy = yy + 1 - 10;
				int xx = ( i == 0 ? x - 17 : x + 115 );
				string key = getBoxKey( skill, i );
				if( boxes.find( key ) == boxes.end() ) {
						boxes[ key ] = new ActionRect( xx, dy, xx + SKILL_BUTTON_SIZE, dy + SKILL_BUTTON_SIZE );
				}
				if( creature->getHasAvailableSkillPoints() ) {
						if( currentSkill == skill && currentMode == i ) {
						  glColor3f( 1, 1, 0 );
						} else {
							glColor3f( 1, 1, 1 );
						}
				} else glColor3f( 0.5, 0.5, 0.5 );
				
				glBegin( GL_LINE_LOOP );
				glVertex2f( xx, dy + SKILL_BUTTON_SIZE );
				glVertex2f( xx, dy );
				glVertex2f( xx + SKILL_BUTTON_SIZE, dy );
				glVertex2f( xx + SKILL_BUTTON_SIZE, dy + SKILL_BUTTON_SIZE );
				glEnd();
				glBegin( GL_LINES );
				glVertex2f( xx, dy + SKILL_BUTTON_SIZE / 2 );
				glVertex2f( xx + SKILL_BUTTON_SIZE, dy + SKILL_BUTTON_SIZE / 2 );
				if( i > 0 ) {
            glVertex2f( xx + SKILL_BUTTON_SIZE / 2, dy );
            glVertex2f( xx + SKILL_BUTTON_SIZE / 2, dy + SKILL_BUTTON_SIZE);
        }
				glEnd();
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

void Portrait::showStateMods() {
  int y = 110;
	glColor4f( 1, 1, 1, 1 );
	pcUi->getScourge()->getSDLHandler()->texPrint( 10, y, _( "Thirst" ) );
  drawBar( 100, y - 10, creature->getThirst(), 10 );
  pcUi->getScourge()->getSDLHandler()->texPrint( 215, y, "%d/%d", creature->getThirst(), 10 );
  y += 15;
	
	pcUi->getScourge()->getSDLHandler()->texPrint( 10, y, _( "Hunger" ) );
  drawBar( 100, y - 10, creature->getHunger(), 10 );
  pcUi->getScourge()->getSDLHandler()->texPrint( 215, y, "%d/%d", creature->getHunger(), 10 );
  y += 15;
  
	pcUi->getScourge()->getSDLHandler()->texPrint( 10, y, _( "Carrying (kg)" ) );
  drawBar( 100, y - 10, (int)( creature->getInventoryWeight() * 100 ), (int)( creature->getMaxInventoryWeight() * 100 ) );
  pcUi->getScourge()->getSDLHandler()->texPrint( 215, y, "%2.1f/%2.1f", creature->getInventoryWeight(), creature->getMaxInventoryWeight() );
	drawHorizontalLine( y + 4 );
	y += 18;

	glColor4f( 1, 0.35f, 0, 1 );
	pcUi->getScourge()->getSDLHandler()->texPrint( 10, y, _( "Character Conditions:" ) );
	glColor4f( 1, 1, 1, 1 );
	y += 15;
	int x = 10;

	GLuint icon;
	char name[255];
	Color color;
	int size = 12;
  for(int i = 0; i < StateMod::STATE_MOD_COUNT + 2; i++) {
		if( pcUi->getScourge()->getStateModIcon( &icon, name, &color, creature, i ) ) {

			// will it fit?
			int textWidth = pcUi->getScourge()->getSDLHandler()->textWidth( name ) + 20 + size;
			if( x + textWidth > this->w - 20 ) {
				x = 10;
				y += 15;
			}

			drawStateModIcon( icon, name, color, x, y, size );

			glPopMatrix();

			x += textWidth;
      
    }
  }
	y += 15;

	drawHorizontalLine( y - 12 );

	glColor4f( 1, 0.35f, 0, 1 );
	pcUi->getScourge()->getSDLHandler()->texPrint( 10, y, _( "Immune to Conditions:" ) );
	glColor4f( 1, 1, 1, 1 );
	y += 15;
	x = 10;
	for(int i = 0; i < StateMod::STATE_MOD_COUNT + 2; i++) {
		if( pcUi->getScourge()->getStateModIcon( &icon, name, &color, creature, i, true ) ) {

			// will it fit?
			int textWidth = pcUi->getScourge()->getSDLHandler()->textWidth( name ) + 20 + size;
			if( x + textWidth > this->w - 20 ) {
				x = 10;
				y += 15;
			}

			drawStateModIcon( icon, name, color, x, y, size );

			glPopMatrix();

			x += textWidth;
      
    }
  }
	y += 15;
}

void Portrait::drawStateModIcon( GLuint icon, char *name, Color color, int x, int y, int size ) {
	glEnable( GL_TEXTURE_2D );
	glBindTexture( GL_TEXTURE_2D, icon );
	glColor4f( color.r, color.g, color.b, color.a );
	glPushMatrix();
	glTranslatef( x, y - size, 0 );
	glBegin( GL_QUADS );
	glNormal3f( 0, 0, 1 );
	glTexCoord2f( 0, 0 );
	glVertex3f( 0, 0, 0 );
	glTexCoord2f( 0, 1 );
	glVertex3f( 0, size, 0 );
	glTexCoord2f( 1, 1 );
	glVertex3f( size, size, 0 );
	glTexCoord2f( 1, 0 );
	glVertex3f( size, 0, 0 );
	glEnd();

	glColor4f( 1, 1, 1, 1 );
	pcUi->getScourge()->getSDLHandler()->texPrint( size + 5, 10, name );
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
