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
  //if(p->getStateMod(Constants::leveled)) {
  if( p->getAvailableSkillPoints() > 0 ) {
    glColor4f( 1.0f, 0.2f, 0.0f, 1.0f );
  } else {
    if( theme->getWindowText() ) {
      glColor4f( theme->getWindowText()->r,
                 theme->getWindowText()->g,
                 theme->getWindowText()->b,
                 theme->getWindowText()->a );
    } else {
      w->applyColor();
    }
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
  float totalArmor, skilledArmor;
  skilledArmor = p->getACPercent( &totalArmor );
  sprintf(s, "AC: %.2f (%.2f)", skilledArmor, totalArmor );
  scourge->getSDLHandler()->texPrint(5, y + 45, s);
  sprintf(s, "Thirst: %d (10)", p->getThirst());
  scourge->getSDLHandler()->texPrint(5, y + 60, s);
  sprintf(s, "Hunger: %d (10)", p->getHunger());
  scourge->getSDLHandler()->texPrint(5, y + 75, s);
  sprintf(s, "AP: %d (%d)", p->getBattle()->getAP(), toint( p->getMaxAP() ) );
  scourge->getSDLHandler()->texPrint(5, y + 90, s);
  sprintf(s, "Initiative: %d", creature->getInitiative() );
  scourge->getSDLHandler()->texPrint( 5, y + 105, s );

  float max, min;

  Item *left = p->getItemAtLocation( Constants::INVENTORY_LEFT_HAND );
  Item *right = p->getItemAtLocation( Constants::INVENTORY_RIGHT_HAND );
  Item *ranged = p->getItemAtLocation( Constants::INVENTORY_WEAPON_RANGED );

  int yy = 120;
  bool hasWeapon = false;
  if( left && left->getRpgItem()->isWeapon() ) {
    p->getAttackPercent( left, &max, &min );
    sprintf(s, "ATK: %.2f - %.2f (%.2f APR) %s", 
            min, max, p->getAttacksPerRound( left ),
            left->getRpgItem()->getName() );
    scourge->getSDLHandler()->texPrint(5, y + yy, s);
    yy += 15;
    hasWeapon = true;
  } 
  if( right && right->getRpgItem()->isWeapon() ) {
    p->getAttackPercent( left, &max, &min );
    sprintf(s, "ATK: %.2f - %.2f (%.2f APR) %s", 
            min, max, p->getAttacksPerRound( right ),
            right->getRpgItem()->getName() );
    scourge->getSDLHandler()->texPrint(5, y + yy, s);
    yy += 15;
    hasWeapon = true;
  }
  if( !hasWeapon ) {
    p->getAttackPercent( NULL, &max, &min );
    sprintf(s, "ATK: %.2f - %.2f (%.2f APR) Bare Hands", 
            min, max, p->getAttacksPerRound( NULL ) );
    scourge->getSDLHandler()->texPrint(5, y + yy, s);
    yy += 15;
  }
  if( ranged ) {
    p->getAttackPercent( left, &max, &min );
    sprintf(s, "ATK: %.2f - %.2f (%.2f APR) %s", 
            min, max, p->getAttacksPerRound( ranged ),
            ranged->getRpgItem()->getName() );
    scourge->getSDLHandler()->texPrint(5, y + yy, s);
    yy += 15;
  }
  
  Util::drawBar( 160,  y - 3, 120, (float)p->getExp(), (float)p->getExpOfNextLevel(), 1.0f, 0.65f, 1.0f, false, theme );
  Util::drawBar( 160, y + 12, 120, (float)p->getHp(), (float)p->getMaxHp(), -1, -1, -1, true, theme );
  Util::drawBar( 160, y + 27, 120, (float)p->getMp(), (float)p->getMaxMp(), 0.45f, 0.65f, 1.0f, false, theme );
  Util::drawBar( 160, y + 42, 120, skilledArmor, totalArmor, 0.45f, 0.65f, 1.0f, false, theme );
  Util::drawBar( 160, y + 57, 120, (float)p->getThirst(), 10.0f, 0.45f, 0.65f, 1.0f, false, theme );
  Util::drawBar( 160, y + 72, 120, (float)p->getHunger(), 10.0f, 0.45f, 0.65f, 1.0f, false, theme );
  Util::drawBar( 160, y + 87, 120, (float)p->getBattle()->getAP(), p->getMaxAP(), 0.45f, 0.65f, 1.0f, false, theme );
}

