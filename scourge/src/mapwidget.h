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
//#include "gui/draganddrop.h"
#include "gui/canvas.h"

/**
  *@author Gabor Torok
  */

class Scourge;
//class Canvas;

using namespace std;

class MapWidget : public Canvas, WidgetView {
private:
  Scourge *scourge;
  Widget *parent;
  int markedX, markedY;
  int selX, selY;
  int oldSelX, oldSelY;
  int oldx, oldy;
  int gx, gy, tx, ty;
  bool dragging;
  bool editable;
  
public:

  MapWidget( Scourge *scourge, Widget *parent, int x, int y, int x2, int y2, bool editable=true );
  ~MapWidget();

  inline void getSelection( int *x, int *y ) { *x = markedX; *y = markedY; }
  void setSelection( int x, int y );

  virtual void drawWidgetContents(Widget *w);

  /**
    The widget received a dragged item
  */
//  virtual void receive(Widget *widget);

  /**
	 The widget initiated a drag
   * return true if there's something to drag at x,y
   */
//  virtual bool startDrag(Widget *widget, int x=0, int y=0);


  inline Canvas *getCanvas() { return this; }

  bool handleEvent(Widget *parent, SDL_Event *event, int x, int y);

protected:
  void calculateValues();
};

#endif

