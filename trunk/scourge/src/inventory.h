
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
#include "rpg/spell.h"
#include "gui/window.h"
#include "gui/button.h"
#include "gui/cardcontainer.h"
#include "gui/scrollinglist.h"
#include "gui/draganddrop.h"
#include "gui/canvas.h"
#include "gui/widgetview.h"
#include "creature.h"

/**
  *@author Gabor Torok
  */

class Creature;

class Inventory : public DragAndDropHandler, WidgetView {
private:
    Scourge *scourge;
    int selected; // which player is selected?
    int selectedMode; // which mode is selected?
    enum mode {
	  INVENTORY = 0, CHARACTER, SPELL, LOG, MISSION
    };    

	// UI
	Window *mainWin;
	Button *playerButton[4];
	Button *inventoryButton, *skillsButton, *spellsButton;
	CardContainer *cards;

	// inventory screen
	//Label *invEquipLabel[Character::INVENTORY_COUNT];
  Canvas *paperDoll;
  int posX[Constants::INVENTORY_COUNT], posY[Constants::INVENTORY_COUNT];
	Label *inventoryWeightLabel, *coinsLabel;
	char inventoryWeightStr[80], coinsStr[80];

	Button *equipButton, *fixButton, *removeCurseButton, *skillAddButton, *skillSubButton;
	Button *combineButton, *enchantButton, *identifyButton, *openButton, *levelUpButton;
	Button *eatDrinkButton, *castScrollButton, *transcribeButton, *infoButton;
	ScrollingList *invList;
	char **pcInvText;

	// spell ui
	Button *castButton;
	ScrollingList *schoolList;
	ScrollingList *spellList;
	Label *spellDescriptionLabel;
	char **schoolText;
	char **spellText;

	// character info screen
	Label *nameAndClassLabel, *levelLabel, *hpLabel, *mpLabel;
	Label *thirstLabel, *hungerLabel, *skillModLabel, *armorLabel;	
	//int stateCount;
	char **stateLine, **skillLine;
	GLuint *icons;
  char **protStateLine;
	GLuint *protIcons;
	ScrollingList *stateList, *skillList, *protStateList;
	char nameAndClassStr[80];
	char levelStr[80];
	char expStr[80];
	char hpStr[80];
	char mpStr[80];
	char thirstStr[80];
	char hungerStr[80];
	char skillModStr[80];
	char skillsStr[80];
	char armorStr[80];
	Canvas *attrCanvas;

	// mission
	char missionText[3000];
	Label *missionDescriptionLabel;
	Button *missionButton;
	ScrollingList *objectiveList;
	char **objectiveText;
	Color *missionColor, *itemColor;

public:
    Inventory(Scourge *scourge);
	~Inventory();
    void show(bool animate=true);
	inline void hide() { mainWin->setVisible(false); }
	inline bool isVisible() { return mainWin->isVisible(); }
  inline Window *getWindow() { return mainWin; }
    bool handleEvent(SDL_Event *event);
	bool handleEvent(Widget *widget, SDL_Event *event);
    //void drawInventory();
	void refresh(int player=-1);

	// drag-n-drop
	void receive(Widget *widget);
	bool startDrag(Widget *widget, int x=0, int y=0);

	void drawWidget(Widget *w);

protected:
	void setSelectedPlayerAndMode(int player, int mode);
	void moveItemTo(int playerIndex);
	void dropItem();
  // returns item's index in inventory or -1 if inv. was full
  int putItem();
  void equipItem();
	void showMemorizedSpellsInSchool(Creature *creature, MagicSchool *school);
	void showSpellDescription(Spell *spell);
	Spell *getSelectedSpell();
};

#endif
