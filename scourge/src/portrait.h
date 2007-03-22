/***************************************************************************
                          portrait.h  -  description
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

#ifndef PORTRAIT_H
#define PORTRAIT_H

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

class Portrait : public WidgetView {
private:
	Scourge *scourge;
	Creature *creature;
	GLuint backgroundTexture, barTexture;
	Window *window;
  Canvas *canvas;
	int x, y, w, h;

public:
	Portrait( Scourge *scourge, Window *window, int x, int y, int w, int h );
	~Portrait();

  inline Widget *getWidget() { return canvas; }
	bool handleEvent( SDL_Event *event );
	bool handleEvent( Widget *widget, SDL_Event *event );
	void setCreature( Creature *creature );

	void drawWidgetContents( Widget *w );

protected:
	void drawBar( int x, int y, int value, int maxValue=100, int mod=0 );
};

#endif

