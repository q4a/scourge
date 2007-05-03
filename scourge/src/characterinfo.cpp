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

  //GuiTheme *theme = win->getTheme();
  Creature *p = creature;
	glDisable( GL_TEXTURE_2D );
	glColor4f( 1, 1, 1, 1 );

  int y = 0;
	glColor4f( 1, 0.35f, 0, 1 );
	scourge->getSDLHandler()->texPrint( 10, y + 15, _( "Basic Statistics:" ) );

	glColor4f( 1, 1, 1, 1 );
	scourge->getSDLHandler()->texPrint( 10, y + 30, "%s: %d", _( "HP" ), p->getMaxHp() );
  scourge->getSDLHandler()->texPrint( 10, y + 45, "%s: %d", _( "MP" ), p->getMaxMp() );
  scourge->getSDLHandler()->texPrint( 10, y + 60, "%s: %d", _( "AP" ), toint( p->getMaxAP() ) );
  
	glColor4f( 1, 0.35f, 0, 1 );
	scourge->getSDLHandler()->texPrint( 10, y + 75, _( "Attack:" ) );
  scourge->describeAttacks( p, 10, y + 90, true );

	glColor4f( 1, 0.35f, 0, 1 );
	scourge->getSDLHandler()->texPrint( 160, y + 75, _( "Unarmed Defense:" ) );
	scourge->describeDefense( p, 160, y + 90 );
}

void CharacterInfoUI::setCreature( Window *win, 
																	 Creature *creature ) { 
	this->win = win;
	this->creature = creature;
}

