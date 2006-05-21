/***************************************************************************
                          characterinfo.h  -  description
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
#include "characterinfo.h"
#include "item.h"
#include "creature.h"
#include "rpg/rpglib.h"
#include "gui/window.h"
#include "gui/guitheme.h"
#include "util.h"

using namespace std;

CharacterInfoUI::CharacterInfoUI( Scourge *scourge ) {
  this->scourge = scourge;
  creature = NULL;
  win = NULL;
}

CharacterInfoUI::~CharacterInfoUI() {
}

void CharacterInfoUI::drawWidgetContents( Widget *w ) {
  if( !( win && creature ) ) return;

  GuiTheme *theme = win->getTheme();
  Creature *p = creature;

  int y = 15;
  char s[80];
  sprintf(s, "Exp: %u(%u)", p->getExp(), p->getExpOfNextLevel());
	if( theme->getWindowText() ) {
		glColor4f( theme->getWindowText()->r,
							 theme->getWindowText()->g,
							 theme->getWindowText()->b,
							 theme->getWindowText()->a );
	} else {
		w->applyColor();
	}
  scourge->getSDLHandler()->texPrint(5, y, s);
  if( theme->getWindowText() ) {
    glColor4f( theme->getWindowText()->r,
               theme->getWindowText()->g,
               theme->getWindowText()->b,
               theme->getWindowText()->a );
  } else {
    w->applyColor();
  }
  sprintf(s, "HP: %d (%d)", p->getHp(), p->getMaxHp());
  scourge->getSDLHandler()->texPrint(5, y + 15, s);
  sprintf(s, "MP: %d (%d)", p->getMp(), p->getMaxMp());
  scourge->getSDLHandler()->texPrint(5, y + 30, s);
  sprintf(s, "Thirst: %d (10)", p->getThirst());
  scourge->getSDLHandler()->texPrint(5, y + 45, s);
  sprintf(s, "Hunger: %d (10)", p->getHunger());
  scourge->getSDLHandler()->texPrint(5, y + 60, s);
  sprintf(s, "AP: %d (%d)", p->getBattle()->getAP(), toint( p->getMaxAP() ) );
  scourge->getSDLHandler()->texPrint(5, y + 75, s);


  glColor4f( 0, 1, 1, 1 );
  //glColor4f( 0, 1, 0.25f, 1 );
  int initiative;
	float armor, dodgePenalty;
  creature->getInitiative( &initiative );
  sprintf(s, "Initiative: %d DEF: S:%d P:%d C:%d", 
					initiative,
					toint( p->getArmor( &armor, &dodgePenalty, RpgItem::DAMAGE_TYPE_SLASHING ) ),
					toint( p->getArmor( &armor, &dodgePenalty, RpgItem::DAMAGE_TYPE_PIERCING ) ),
					toint( p->getArmor( &armor, &dodgePenalty, RpgItem::DAMAGE_TYPE_CRUSHING ) ) );
  scourge->getSDLHandler()->texPrint( 5, y + 90, s );

  glColor4f( 1, 0.35f, 0, 1 );
  float max, min, cth, skill;

  Item *left = p->getItemAtLocation( Constants::INVENTORY_LEFT_HAND );
  Item *right = p->getItemAtLocation( Constants::INVENTORY_RIGHT_HAND );
  Item *ranged = p->getItemAtLocation( Constants::INVENTORY_WEAPON_RANGED );

  char buff[80];
  int yy = 105;
  if( left && left->getRpgItem()->isWeapon() ) {
    p->getAttack( left, &max, &min );
		p->getCth( left, &cth, &skill, false );
    if( toint( max ) > toint( min ) ) 
      sprintf(s, "ATK:%d-%d %s CTH:%d (%s) %s", 
              toint( min ), toint( max ), 
							RpgItem::DAMAGE_TYPE_NAME[ left->getRpgItem()->getDamageType() ],
							toint( skill ),
							getAPRDescription(p, left, buff),
              left->getRpgItem()->getName() );
    else
      sprintf(s, "ATK:%d %s CTH:%d (%s) %s", 
              toint( min ), 
							RpgItem::DAMAGE_TYPE_NAME[ left->getRpgItem()->getDamageType() ],
							toint( skill ),
							getAPRDescription(p, left, buff),
              left->getRpgItem()->getName() );
		if( Constants::INVENTORY_LEFT_HAND == p->getPreferredWeapon() ) {
			glColor4f( 1, 0.35f, 0, 1 );
		} else {
			glColor4f( 0.7f, 0.7f, 0.7f, 1 );
		}
    scourge->getSDLHandler()->texPrint(5, y + yy, s);
    yy += 15;
  } 
  if( right && right->getRpgItem()->isWeapon() ) {
    p->getAttack( right, &max, &min );
		p->getCth( right, &cth, &skill, false );
    if( toint( max ) > toint( min ) ) 
      sprintf(s, "ATK:%d-%d %s CTH:%d (%s) %s", 
              toint( min ), toint( max ), 
							RpgItem::DAMAGE_TYPE_NAME[ right->getRpgItem()->getDamageType() ],
							toint( skill ),
							getAPRDescription(p, right, buff),
              right->getRpgItem()->getName() );
    else
      sprintf(s, "ATK:%d %s CTH:%d (%s) %s", 
              toint( min ), 
							RpgItem::DAMAGE_TYPE_NAME[ right->getRpgItem()->getDamageType() ],
							toint( skill ),
							getAPRDescription(p, right, buff),
              right->getRpgItem()->getName() );
		if( Constants::INVENTORY_RIGHT_HAND == p->getPreferredWeapon() ) {
			glColor4f( 1, 0.35f, 0, 1 );
		} else {
			glColor4f( 0.7f, 0.7f, 0.7f, 1 );
		}
    scourge->getSDLHandler()->texPrint(5, y + yy, s);
    yy += 15;
  }	
  if( ranged ) {
    p->getAttack( ranged, &max, &min );
		p->getCth( ranged, &cth, &skill, false );
    if( toint( max ) > toint( min ) ) 
      sprintf(s, "ATK:%d-%d %s CTH:%d (%s) %s", 
              toint( min ), toint( max ), 
							RpgItem::DAMAGE_TYPE_NAME[ ranged->getRpgItem()->getDamageType() ],
							toint( skill ),
							getAPRDescription(p, ranged, buff),
              ranged->getRpgItem()->getName() );
    else
      sprintf(s, "ATK:%d %s CTH:%d (%s) %s", 
              toint( min ), 
							RpgItem::DAMAGE_TYPE_NAME[ ranged->getRpgItem()->getDamageType() ],
							toint( skill ),
							getAPRDescription(p, ranged, buff),
              ranged->getRpgItem()->getName() );
		if( Constants::INVENTORY_WEAPON_RANGED == p->getPreferredWeapon() ) {
			glColor4f( 1, 0.35f, 0, 1 );
		} else {
			glColor4f( 0.7f, 0.7f, 0.7f, 1 );
		}
    scourge->getSDLHandler()->texPrint(5, y + yy, s);
    yy += 15;
  }
	p->getAttack( NULL, &max, &min );
	p->getCth( NULL, &cth, &skill, false );
	if( toint( max ) > toint( min ) ) 
		sprintf(s, "ATK:%d-%d %s CTH:%d (%s) Bare Hands", 
						toint( min ), toint( max ), 
						RpgItem::DAMAGE_TYPE_NAME[ RpgItem::DAMAGE_TYPE_CRUSHING ],
						toint( skill ),
						getAPRDescription(p, NULL, buff) );
	else
		sprintf(s, "ATK:%d %s CTH:%d (%s) Bare Hands", 
						toint( min ), 
						RpgItem::DAMAGE_TYPE_NAME[ RpgItem::DAMAGE_TYPE_CRUSHING ],
						toint( skill ),
						getAPRDescription(p, NULL, buff) );
	if( -1 == p->getPreferredWeapon() ) {
		glColor4f( 1, 0.35f, 0, 1 );
	} else {
		glColor4f( 0.7f, 0.7f, 0.7f, 1 );
	}
	scourge->getSDLHandler()->texPrint(5, y + yy, s);
	yy += 15;
  
  Util::drawBar( 160,  y - 3, 120, (float)p->getExp(), (float)p->getExpOfNextLevel(), 1.0f, 0.65f, 1.0f, false, theme );
  Util::drawBar( 160, y + 12, 120, (float)p->getHp(), (float)p->getMaxHp(), -1, -1, -1, true, theme );
  Util::drawBar( 160, y + 27, 120, (float)p->getMp(), (float)p->getMaxMp(), 0.45f, 0.65f, 1.0f, false, theme );
  Util::drawBar( 160, y + 42, 120, (float)p->getThirst(), 10.0f, 0.45f, 0.65f, 1.0f, false, theme );
  Util::drawBar( 160, y + 57, 120, (float)p->getHunger(), 10.0f, 0.45f, 0.65f, 1.0f, false, theme );
  Util::drawBar( 160, y + 72, 120, (float)p->getBattle()->getAP(), p->getMaxAP(), 0.45f, 0.65f, 1.0f, false, theme );
}

char *CharacterInfoUI::getAPRDescription( Creature *p, Item *item, char *buff ) {
  float apr = p->getAttacksPerRound( item );
  if( apr >= 1.0f ) {
    sprintf( buff, "%.2f APR", apr );
  } else {
    sprintf( buff, "per %.2f R", ( 100.0f / (apr * 100.0f) ) );
  }
  return buff;
}

