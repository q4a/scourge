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

class Equip : public DragAndDropHandler, WidgetView {
private:
	Scourge *scourge;
	Creature *creature;
	GLuint backgroundTexture;
  int currentHole;

	Window *mainWin;
  Canvas *canvas;
	Label *inventoryWeightLabel, *coinsLabel;
	char inventoryWeightStr[80], coinsStr[80];
	Button *equipButton, *fixButton, *removeCurseButton, *poolButton;
	Button *combineButton, *enchantButton, *identifyButton, *openButton;
	Button *eatDrinkButton, *castScrollButton, *transcribeButton, *infoButton;
  Button *storeItemButton;
  int preferredWeaponLocation[3];
  Button *preferredWeaponButton[3];
  Storable *storable;

public:
	Equip( Scourge *scourge );
	~Equip();

  inline Storable *getStorable() { return storable; }

  inline Window *getWindow() { return mainWin; }
	bool handleEvent( SDL_Event *event );
	bool handleEvent( Widget *widget, SDL_Event *event );
	void setCreature( Creature *creature );

	// drag-n-drop
	void receive( Widget *widget );
	bool startDrag( Widget *widget, int x=0, int y=0 );

	void drawWidgetContents( Widget *w );

protected:
	int putItem();
	void equipItem();
	void dropItem();
  Item *getItemAtPos( int x, int y );
  int getHoleAtPos( int x, int y );
};

#endif

