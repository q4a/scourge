/***************************************************************************
                          widgetview.h  -  description
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

#ifndef WIDGETVIEW_H
#define WIDGETVIEW_H

#include "../constants.h"
#include "widget.h"

/**
  *@author Gabor Torok
  */

class WidgetView {
 public: 
  WidgetView();
  virtual ~WidgetView();
  
  /** Draw the widget. */
  virtual void drawWidget(Widget *w) = 0;
};

#endif
