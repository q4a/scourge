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
  sprintf(s, _( "Exp: %u(%u)" ), p->getExp(), p->getExpOfNextLevel());
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
  sprintf(s, _( "HP: %d (%d)" ), p->getHp(), p->getMaxHp());
  scourge->getSDLHandler()->texPrint(5, y + 15, s);
  sprintf(s, _( "MP: %d (%d)" ), p->getMp(), p->getMaxMp());
  scourge->getSDLHandler()->texPrint(5, y + 30, s);
  sprintf(s, _( "Thirst: %d (10)" ), p->getThirst());
  scourge->getSDLHandler()->texPrint(5, y + 45, s);
  sprintf(s, _( "Hunger: %d (10)" ), p->getHunger());
  scourge->getSDLHandler()->texPrint(5, y + 60, s);
  sprintf(s, _( "AP: %d (%d)" ), p->getBattle()->getAP(), toint( p->getMaxAP() ) );
  scourge->getSDLHandler()->texPrint(5, y + 75, s);

  scourge->describeAttacks( p, 0, y + 90 );
  
  Util::drawBar( 160,  y - 3, 120, (float)p->getExp(), (float)p->getExpOfNextLevel(), 1.0f, 0.65f, 1.0f, false, theme );
  Util::drawBar( 160, y + 12, 120, (float)p->getHp(), (float)p->getMaxHp(), -1, -1, -1, true, theme );
  Util::drawBar( 160, y + 27, 120, (float)p->getMp(), (float)p->getMaxMp(), 0.45f, 0.65f, 1.0f, false, theme );
  Util::drawBar( 160, y + 42, 120, (float)p->getThirst(), 10.0f, 0.45f, 0.65f, 1.0f, false, theme );
  Util::drawBar( 160, y + 57, 120, (float)p->getHunger(), 10.0f, 0.45f, 0.65f, 1.0f, false, theme );
  Util::drawBar( 160, y + 72, 120, (float)p->getBattle()->getAP(), p->getMaxAP(), 0.45f, 0.65f, 1.0f, false, theme );
}


