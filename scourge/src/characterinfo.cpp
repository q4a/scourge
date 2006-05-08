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
  int initiative;
  creature->getInitiative( &initiative );
  sprintf(s, "Initiative: %d", initiative );
  scourge->getSDLHandler()->texPrint( 5, y + 90, s );

  glColor4f( 0, 1, 0.25f, 1 );
  float totalArmor;
  p->getACPercent( &totalArmor, NULL, -1, NULL );
  sprintf(s, "DEF: %d", toint( totalArmor ) );
  scourge->getSDLHandler()->texPrint(5, y + 105, s);


  glColor4f( 1, 0.35f, 0, 1 );
  float max, min;

  Item *left = p->getItemAtLocation( Constants::INVENTORY_LEFT_HAND );
  Item *right = p->getItemAtLocation( Constants::INVENTORY_RIGHT_HAND );
  Item *ranged = p->getItemAtLocation( Constants::INVENTORY_WEAPON_RANGED );

  char buff[80];
  int yy = 120;
  bool hasWeapon = false;
  if( left && left->getRpgItem()->isWeapon() ) {
    p->getAttack( left, &max, &min );
    if( toint( max ) > toint( min ) ) 
      sprintf(s, "ATK: %d - %d (%s) %s", 
              toint( min ), toint( max ), getAPRDescription(p, left, buff),
              left->getRpgItem()->getName() );
    else
      sprintf(s, "ATK: %d (%s) %s", 
              toint( min ), getAPRDescription(p, left, buff),
              left->getRpgItem()->getName() );
    scourge->getSDLHandler()->texPrint(5, y + yy, s);
    yy += 15;
    hasWeapon = true;
  } 
  if( right && right->getRpgItem()->isWeapon() ) {
    p->getAttack( right, &max, &min );
    if( toint( max ) > toint( min ) ) 
      sprintf(s, "ATK: %d - %d (%s) %s", 
              toint( min ), toint( max ), getAPRDescription(p, right, buff),
              right->getRpgItem()->getName() );
    else
      sprintf(s, "ATK: %d (%s) %s", 
              toint( min ), getAPRDescription(p, right, buff),
              right->getRpgItem()->getName() );
    scourge->getSDLHandler()->texPrint(5, y + yy, s);
    yy += 15;
    hasWeapon = true;
  }
  if( !hasWeapon ) {
    p->getAttack( NULL, &max, &min );
    if( toint( max ) > toint( min ) ) 
      sprintf(s, "ATK: %d - %d (%s) Bare Hands", 
              toint( min ), toint( max ), getAPRDescription(p, NULL, buff) );
    else
      sprintf(s, "ATK: %d (%s) Bare Hands", 
              toint( min ), getAPRDescription(p, NULL, buff) );
    scourge->getSDLHandler()->texPrint(5, y + yy, s);
    yy += 15;
  }
  if( ranged ) {
    p->getAttack( ranged, &max, &min );
    if( toint( max ) > toint( min ) ) 
      sprintf(s, "ATK: %d - %d (%s) %s", 
              toint( min ), toint( max ), getAPRDescription(p, ranged, buff),
              ranged->getRpgItem()->getName() );
    else
      sprintf(s, "ATK: %d (%s) %s", 
              toint( min ), getAPRDescription(p, ranged, buff),
              ranged->getRpgItem()->getName() );
    scourge->getSDLHandler()->texPrint(5, y + yy, s);
    yy += 15;
  }
  
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

