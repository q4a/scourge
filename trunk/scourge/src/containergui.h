/***************************************************************************
              containergui.h  -  The container contents window
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

#ifndef CONTAINER_GUI_H
#define CONTAINER_GUI_H
#pragma once

#include <iostream>
#include <string>
#include "scourge.h"
#include "gui/window.h"
#include "gui/button.h"
#include "gui/eventhandler.h"

class Item;
class ContainerView;

/// The "container contents" window (for open chests etc.)
class ContainerGui : public EventHandler {

private:
	Scourge *scourge;
	Item *container;
	Window *win;
	ContainerView *view;
	Button *openButton, *infoButton, *getAllButton, *closeButton;

public:
	ContainerGui( Scourge *scourge, Item *container, int x, int y );
	~ContainerGui();

	bool handleEvent( SDL_Event *event );
	bool handleEvent( Widget *widget, SDL_Event *event );

	inline Item *getContainer() {
		return container;
	}
	inline Window *getWindow() {
		return win;
	}
	inline ContainerView *getView() {
		return view;
	}
	void refresh();
	inline Item *getSelectedItem();	
	//void setSelectedItem( Item *item );	
};

#endif
