
/***************************************************************************
                          inventory.h  -  description
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

#ifndef INVENTORY_H
#define INVENTORY_H

#include <iostream>
#include "constants.h"
#include "sdlhandler.h"
#include "sdleventhandler.h"
#include "sdlscreenview.h"
#include "scourge.h"
#include "rpg/character.h"
#include "gui/window.h"
#include "gui/button.h"
#include "gui/cardcontainer.h"
#include "gui/scrollinglist.h"
#include "gui/draganddrop.h"

/**
  *@author Gabor Torok
  */

class Inventory : public DragAndDropHandler {
private:
    Scourge *scourge;
    int selected; // which player is selected?
    int selectedMode; // which mode is selected?
    enum mode {
        INVENTORY = 0, CHARACTER, SPELL, LOG
    };    

	// UI
	Window *mainWin;
	Button *player1Button, *player2Button, *player3Button, *player4Button;
	Button *inventoryButton, *skillsButton, *spellsButton;
	CardContainer *cards;

	// inventory screen
	Label *invEquipLabel[Character::INVENTORY_COUNT];
	Button *equipButton, *fixButton, *removeCurseButton;
	Button *combineButton, *enchantButton, *identifyButton, *openButton;
	Button *eatDrinkButton;
	ScrollingList *invList;
	char **pcInvText;

	// character info screen
	Label *nameAndClassLabel, *levelLabel, *expLabel, *hpLabel;
	Label *thirstLabel, *hungerLabel;	
	int stateCount;
	char **stateLine, **skillLine;
	ScrollingList *stateList, *skillList;
	char nameAndClassStr[80];
	char levelStr[80];
	char expStr[80];
	char hpStr[80];
	char thirstStr[80];
	char hungerStr[80];

public:
    Inventory(Scourge *scourge);
	~Inventory();
    inline void show() { mainWin->setVisible(true); setSelectedPlayerAndMode(selected, selectedMode); }
	inline void hide() { mainWin->setVisible(false); }
	inline bool isVisible() { return mainWin->isVisible(); }
    bool handleEvent(SDL_Event *event);
	bool handleEvent(Widget *widget, SDL_Event *event);
    void drawInventory();
	void refresh();

	// drag-n-drop
	void receive(Widget *widget);
	void startDrag(Widget *widget);

protected:
	void setSelectedPlayerAndMode(int player, int mode);
	void moveItemTo(int playerIndex);
	void dropItem();
};

#endif
