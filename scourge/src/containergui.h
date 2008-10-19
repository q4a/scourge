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
#include "gui/widget.h"
#include "gui/button.h"
#include "gui/scrollinglist.h"
#include "gui/draganddrop.h"
#include "gui/widgetview.h"
#include "gui/canvas.h"

class Item;

/// The "container contents" window (for open chests etc.)
class ContainerGui : public DragAndDropHandler, WidgetView {

private:
	Scourge *scourge;
	Item *container;
	int x, y;
	Window *win;
	Button *openButton, *infoButton, *closeButton, *getAllButton;
	Canvas *canvas;
	Item *lastItem, *selectedItem;

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
	inline void refresh() {
		showContents();
	}
	inline Item *getSelectedItem() {
		return selectedItem;
	}	

	// drag and drop handling
	void receive( Widget *widget );
	bool startDrag( Widget *widget, int x = 0, int y = 0 );
	
	void drawWidgetContents( Widget *w );

private:
	void showContents();
	void dropItem();
	void convertMousePos( int x, int y, int *invX, int *invY ); 
	Item *getItemAtPos( int x, int y );
	void showInfo( Item *item );
	bool checkInventoryLocation( Item *item, bool useExistingLocationForSameItem, int xx, int yy );
	bool findInventoryPosition( Item *item, int x, int y, bool useExistingLocationForSameItem = true );
	bool receive( Item *item, bool atCursor );
};

#endif
