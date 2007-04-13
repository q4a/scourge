/***************************************************************************
                          equip.h  -  description
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

#ifndef EQUIP_H
#define EQUIP_H

#include <iostream>
#include <vector>
#include "common/constants.h"
#include "gui/window.h"
#include "gui/button.h"
#include "gui/draganddrop.h"
#include "gui/canvas.h"
#include "gui/widgetview.h"

/**
  *@author Gabor Torok
  */

class Creature;
class Scourge;
class Storable;
class ConfirmDialog;
class Item;
class PcUi;
class Spell;
class SpecialSkill;
class ScrollingLabel;
class ScrollingList;
class Label;
class Button;

class Equip : public DragAndDropHandler, WidgetView {
private:
	Creature *creature;
	GLuint backgroundTexture, scrollTexture;
  int currentHole;
	PcUi *pcUi;
  Canvas *canvas;
	int x, y, w, h;
	Item *lastItem;
	int mode;
	int schoolIndex;
	int spellIndex;
	SpecialSkill *specialSkill;

public:

	enum {
		EQUIP_MODE=0,
		SPELLS_MODE,
		CAPABILITIES_MODE,
		MISSION_MODE
	};

	Equip( PcUi *pcUi, int x, int y, int w, int h );
	~Equip();

  inline Widget *getWidget() { return canvas; }
	bool handleEvent( SDL_Event *event );
	bool handleEvent( Widget *widget, SDL_Event *event );
	void setCreature( Creature *creature );
	inline void setMode( int mode ) { this->mode = mode; }
	inline int getMode() { return mode; }

	// drag-n-drop
	void receive( Widget *widget );
	bool startDrag( Widget *widget, int x=0, int y=0 );

	void drawWidgetContents( Widget *w );

protected:
  Item *getItemAtPos( int x, int y );
  int getHoleAtPos( int x, int y );
	Item *getItemInHole( int hole );
	int getSchoolIndex( int x, int y );
	int getSpellIndex( int x, int y, int schoolIndex );
	void castSpell( Spell *spell );
	void storeSpell( Spell *spell );
	void drawCapabilities();
	void drawSpells();
	void drawEquipment();
	void storeSpecialSkill( SpecialSkill *ss );
	void storeStorable( Storable *storable );
	void useSpecialSkill( SpecialSkill *ss );
};

class MissionInfoUI {
private:
	PcUi *pcUi;
	int x, y, w, h;
	ScrollingLabel *description;
	ScrollingList *objectiveList;
	Label *objectivesLabel;
	Button *consoleButton;
	char **objectiveText;
	Color *missionColor;

public:
	MissionInfoUI( PcUi *pcUi, int x, int y, int w, int h );
	~MissionInfoUI();
	void refresh();
	void show();
	void hide();

	inline Button *getConsoleButton() { return consoleButton; }
};

#endif

