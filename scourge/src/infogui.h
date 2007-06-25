
/***************************************************************************
                          infogui.h  -  description
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

#ifndef INFO_GUI_H
#define INFO_GUI_H

#include <iostream>
#include <string>
#include <map>
#include "common/constants.h"
#include "scourge.h"
#include "gui/window.h"
#include "gui/widget.h"
#include "gui/button.h"
#include "gui/canvas.h"
#include "gui/widgetview.h"
#include "gui/scrollinglabel.h"

class Item;

class InfoGui : public WidgetView {

 private:
  Scourge *scourge;
  Item *item;
  Window *win;
  Button *openButton, *idButton, *closeButton;
  ScrollingLabel *label;
  ScrollingLabel *nameLabel;
  Canvas *image;
  char name[500], description[1000];

 public:
  InfoGui(Scourge *scourge);
  ~InfoGui();

  bool handleEvent(Widget *widget, SDL_Event *event);

  inline Item *getItem() { return item; }
  void setItem(Item *item);
  inline Window *getWindow() { return win; }

  void drawWidgetContents(Widget *w);

protected:
  void describe();
  void appendMagicItemInfo( char *description, Item *item );
  void describeRequirements( char *description, int influenceTypeCount );
};

#endif

