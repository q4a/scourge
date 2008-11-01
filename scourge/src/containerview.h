/***************************************************************************
              containerview.h  -  The container contents widget
                             -------------------
    begin                : Mon Oct 20 2008
    copyright            : (C) 2008 by Gabor Torok
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

#ifndef CONTAINER_VIEW_H
#define CONTAINER_VIEW_H
#pragma once

#include <iostream>
#include <string>
#include "scourge.h"
#include "gui/window.h"
#include "gui/widget.h"
#include "gui/button.h"
#include "gui/draganddrop.h"
#include "gui/widgetview.h"
#include "gui/canvas.h"

class Item;
class Creature;

/// The "container contents" window (for open chests etc.)
class ContainerView : public Canvas, DragAndDropHandler, WidgetView {

private:
	Scourge *scourge;
	Item *container;
	int x, y;
	Window *win;
	Item *lastItem, *selectedItem;
	Creature *creature;

public:
	ContainerView( Scourge *scourge, Item *container, Window *win, int x, int y );
	~ContainerView();

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
	void setSelectedItem( Item *item );	

	bool receiveItem( Item *item, bool atCursor );	
	// drag and drop handling
	void receive( Widget *widget );
	bool startDrag( Widget *widget, int x = 0, int y = 0 );
	
	void drawWidgetContents( Widget *w );
	
	void showInfo( Item *item );
	
	void setItem( Item *item, Creature *creature=NULL );

private:
	bool receiveInternal( Item *item, bool atCursor );
	void showContents();
	void dropItem();
	void convertMousePos( int x, int y, int *invX, int *invY ); 
	Item *getItemAtPos( int x, int y );
	bool checkInventoryLocation( Item *item, bool useExistingLocationForSameItem, int xx, int yy );
	bool findInventoryPosition( Item *item, int x, int y, bool useExistingLocationForSameItem = true );
	bool addToContainer( Item *item );
	bool removeFromContainer( Item *item );
};

#endif
