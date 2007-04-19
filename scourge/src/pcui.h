/***************************************************************************
                          pcui.h  -  description
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

#ifndef PCUI_H
#define PCUI_H

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
class Equip;
class Inven;
class Portrait;
class MissionInfoUI;
class CardContainer;

class PcUi : public WindowListener {
private:
	Scourge *scourge;
	Creature *creature;
	Window *mainWin;
	Equip *equip;
	CardContainer *cc;
	MissionInfoUI *missionInfo;
	Inven *inven;
	Portrait *portrait;
	Button *use, *transcribe, *enchant, *info, *store;
	Button *prev, *next, *stats, *skills, *statemods;
	Button *up, *down, *applyMods, *poolMoney;
	Button *equipButton, *spellsButton, *capabilitiesButton, *missionButton;
	Button *cast, *storeSpell;
	Label *status;

public:
	PcUi( Scourge *scourge );
	~PcUi();

	virtual void windowClosing();

  inline Window *getWindow() { return mainWin; }
	void show();
	void hide();
	void refresh();
	inline Scourge *getScourge() { return scourge; }
	bool handleEvent( SDL_Event *event );
	bool handleEvent( Widget *widget, SDL_Event *event );
	void setCreature( Creature *creature );

	bool isUseSelected();
	bool isEnchantSelected();
	bool isTranscribeSelected();
	bool isInfoSelected();
	bool isStoreSelected();
	void unselectButtons();
	bool isCastSelected();
	bool isStoreSpellSelected();
	void unselectSpellButtons();
	void receiveInventory();
	bool receiveInventory(Item *item);

protected:
	void toggleButtons( Button *button );
	void toggleLeftButtons( Button *button );
	void toggleSpellButtons( Button *widget );
};

#endif

