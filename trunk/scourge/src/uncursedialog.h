/***************************************************************************
uncursedialog.h  -  The uncurse item dialog
-------------------
    begin                : 8/26/2005
    copyright            : (C) 2005 by Gabor Torok
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

#ifndef UNCURSE_DIALOG_H
#define UNCURSE_DIALOG_H
#pragma once

#include "itemlist.h"
#include "pcui.h"
#include <map>

class Scourge;
class Creature;
class Window;
class Label;
class Widget;
class Item;
class PcUi;

/// The "uncurse item" dialog.
class UncurseDialog : public ItemRenderer {
private:
	Scourge *scourge;
	Creature *creature;
	Window *win;
	int tradeA;
	std::map<Item*, int> prices;

	Label *labelA;
	Label *totalA;
	ItemList *listA;
	Button *closeButton, *infoButtonA, *uncurseButton;
	Label *coinAvailA;

public:
	UncurseDialog( Scourge *scourge );
	~UncurseDialog();
	void setCreature( Creature *creature );
	void updateUI();
	inline Window *getWindow() {
		return win;
	}
	void handleEvent( Widget *widget, SDL_Event *event );

	void render( const Widget *widget, const Item *item, std::string& buffer );

protected:
	void updateLabels();
	void uncurse();
	int getSelectedTotal( ItemList *list );
	bool validateBackpack();
};

#endif
