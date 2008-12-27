/***************************************************************************
               characterinfo.h  -  Basic character info widget
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

#ifndef CHARACTER_INFO_H
#define CHARACTER_INFO_H
#pragma once

#include <iostream>
#include <string>
#include <vector>
#include "session.h"
#include "gui/widgetview.h"

/**
  *@author Gabor Torok
  */

class Session;
class Creature;
class Window;
class Creature;
class Item;
class Scourge;

/// A widget that shows basic character info.
class CharacterInfoUI : public WidgetView {
private:
	Scourge *scourge;
	Creature *creature;
	Window *win;

public:
	CharacterInfoUI( Scourge *scourge );
	~CharacterInfoUI();

	void setCreature( Window *win, Creature *creature );
	inline Creature *getCreature() {
		return creature;
	}
	inline Window *getWindow() {
		return win;
	}

	// WidgetView interface
	virtual void drawWidgetContents( Canvas *w );

private:
	//unused: char *getAPRDescription( Creature *p, Item *item, char *buff );

};

#endif

