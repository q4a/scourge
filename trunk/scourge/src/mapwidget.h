/***************************************************************************
                          mapwidget.h  -  description
                             -------------------
    begin                : Tue Jun 18 2005
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

#ifndef MAP_WIDGET_H
#define MAP_WIDGET_H

#include "constants.h"
#include "gui/widgetview.h"
#include "gui/canvas.h"

/**
  *@author Gabor Torok
  */

class Scourge;

using namespace std;

class MapWidget : public Canvas, WidgetView {
private:
  Scourge *scourge;
  int selX, selY;
  
public:

  MapWidget( Scourge *scourge, int x, int y, int x2, int y2 );
  ~MapWidget();

  bool handleEvent(SDL_Event *event);

  inline void getSelection( int *x, int *y ) { *x = selX; *y = selY; }

  virtual void drawWidget(Widget *w);
};

#endif

