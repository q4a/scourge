/***************************************************************************
                          canvas.h  -  description
                             -------------------
    begin                : Thu Aug 28 2003
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

#ifndef CANVAS_H
#define CANVAS_H

#include "../constants.h"
#include "widget.h"
#include "widgetview.h"

/**
  *@author Gabor Torok
  */

class Canvas : public Widget {
 private:
  WidgetView *view;
  int x2, y2;

 public: 
  Canvas(int x, int y, int x2, int y2, WidgetView *view);
  virtual ~Canvas();
  inline WidgetView *getView() { return view; }
  void drawWidget(Widget *parent);
};

#endif

