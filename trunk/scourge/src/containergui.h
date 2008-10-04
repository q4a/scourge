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

#include <iostream>
#include <string>
#include "common/constants.h"
#include "scourge.h"
#include "gui/window.h"
#include "gui/widget.h"
#include "gui/button.h"
#include "gui/scrollinglist.h"
#include "gui/draganddrop.h"

class Item;

/// The "container contents" window (for open chests etc.)

class ContainerGui : public DragAndDropHandler {

 private:
  Scourge *scourge;
  Item *container;
  Window *win;
  Button *openButton, *infoButton, *closeButton, *getAllButton;
  ScrollingList *list;
  Label *label;
  std::string containedItemNames[MAX_CONTAINED_ITEMS];
  Color *itemColor;
  GLuint *itemIcon;

 public:
  ContainerGui(Scourge *scourge, Item *container, int x, int y);
  ~ContainerGui();

  bool handleEvent(SDL_Event *event);
  bool handleEvent(Widget *widget, SDL_Event *event);

  inline Item *getContainer() { return container; }
  inline Window *getWindow() { return win; }
  inline void refresh() { showContents(); }

  // drag and drop handling
  void receive(Widget *widget);
  bool startDrag(Widget *widget, int x=0, int y=0);

 private:
  void showContents();
  void dropItem();
  
};

#endif
