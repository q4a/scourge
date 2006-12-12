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
#include <vector>
#include "common/constants.h"
#include "gui/window.h"
#include "gui/button.h"
#include "gui/cardcontainer.h"
#include "gui/scrollinglist.h"
#include "gui/scrollinglabel.h"
#include "gui/draganddrop.h"
#include "gui/canvas.h"
#include "gui/widgetview.h"

/**
  *@author Gabor Torok
  */

class Creature;
class Spell;
class MagicSchool;
class Scourge;
class SpecialSkill;
class Storable;
class CharacterInfoUI;
class SkillsView;
class ConfirmDialog;

class Inventory : public DragAndDropHandler, WidgetView {
private:
    Scourge *scourge;
    int selected; // which player is selected?
    int selectedMode; // which mode is selected?
    enum mode {
	  INVENTORY = 0, CHARACTER, SPELL, SPECIAL, LOG, MISSION, PARTY
    };    

	// UI
	Window *mainWin;
	Button *inventoryButton, *skillsButton, *spellsButton, *specialButton, *partyButton, *closeButton;
	CardContainer *cards;

	// inventory screen
	//Label *invEquipLabel[Character::INVENTORY_COUNT];
  Canvas *paperDoll;
  int posX[Constants::INVENTORY_COUNT], posY[Constants::INVENTORY_COUNT];
	Label *inventoryWeightLabel, *coinsLabel;
	char inventoryWeightStr[80], coinsStr[80];

	Button *equipButton, *fixButton, *removeCurseButton, *poolButton;
	Button *combineButton, *enchantButton, *identifyButton, *openButton;
	Button *eatDrinkButton, *castScrollButton, *transcribeButton, *infoButton;
  Button *storeItemButton;
	ScrollingList *invList;
	char **pcInvText;

  int preferredWeaponLocation[3];
  Button *preferredWeaponButton[3];

	// spell ui
	Button *castButton, *storeSpellButton;
	ScrollingList *schoolList;
	ScrollingList *spellList;
	ScrollingLabel *spellDescriptionLabel;
	char **schoolText;
	char **spellText;
	GLuint *spellIcons;

	// special skills ui
	std::vector<SpecialSkill*> knownSpecialSkills;
	Button *useSpecialButton, *storeSpecialButton;
	ScrollingList *specialList;
	ScrollingLabel *specialDescriptionLabel;
	char **specialText;
	GLuint *specialIcons;
  Color *specialColor;

	// character info screen
	Label *nameAndClassLabel, *levelLabel, *hpLabel, *mpLabel;
	Label *thirstLabel, *hungerLabel, *skillLabel, *armorLabel;	
	//int stateCount;
	char **stateLine;  
	GLuint *icons;
  char **protStateLine;
	GLuint *protIcons;
	ScrollingList *stateList, *protStateList;
  SkillsView *skillList;
	Label *skillModLabel;
	Button *addModButton, *delModButton, *acceptModButton;
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
  CharacterInfoUI *charInfoUI;
	ConfirmDialog *confirmDialog;

	// mission
	char missionText[3000];
	ScrollingLabel *missionDescriptionLabel;
	Button *missionButton;
	ScrollingList *objectiveList;
	char **objectiveText;
	Color *missionColor, *itemColor, *schoolColors;
  GLuint *itemIcon;
  Storable *storable;

  // party
  char **formationText;
  ScrollingList *formationList;
  Button *layoutButton1, *layoutButton2, *layoutButton4;
  Button *squirrelWindow;
  Button *saveGameButton;

public:
    Inventory(Scourge *scourge);
	~Inventory();

  inline bool inStoreSpellMode() { 
    return ( storeSpellButton->isSelected() || 
             storeSpecialButton->isSelected() ||
             storeItemButton->isSelected() ); 
    }
  inline void setStoreSpellMode( bool b ) { 
    storeSpellButton->setSelected( b ); 
    storeSpecialButton->setSelected( b ); 
    storeItemButton->setSelected( b );
    if( !b ) storable = NULL;
  }
  inline Storable *getStorable() { return storable; }

  void positionWindow();
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

	void drawWidgetContents(Widget *w);

  void showSpells();
  void showSkills();
  void showSpecial();

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
  void showSpecialDescription(SpecialSkill *special);
  SpecialSkill *getSelectedSpecial();
};

#endif