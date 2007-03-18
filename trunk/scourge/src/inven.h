/***************************************************************************
                          inven.h  -  description
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

#ifndef INVEN_H
#define INVEN_H

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

class Inven : public DragAndDropHandler, WidgetView {
private:
	Scourge *scourge;
	Creature *creature;
	GLuint backgroundTexture;
  int currentHole;
	Window *window;
  Canvas *canvas;
	int x, y, w, h;

public:
	Inven( Scourge *scourge, Window *window, int x, int y, int w, int h );
	~Inven();

  inline Widget *getWidget() { return canvas; }
	bool handleEvent( SDL_Event *event );
	bool handleEvent( Widget *widget, SDL_Event *event );
	void setCreature( Creature *creature );

	// drag-n-drop
	void receive( Widget *widget );
	bool startDrag( Widget *widget, int x=0, int y=0 );

	void drawWidgetContents( Widget *w );

protected:
  Item *getItemAtPos( int x, int y );
	bool findInventoryPosition( Item *item, bool useExistingLocationForSameItem=true );
	bool checkInventoryLocation( Item *item, bool useExistingLocationForSameItem, int xx, int yy );
};

#endif

