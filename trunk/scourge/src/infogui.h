
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
#include "pcui.h"
#include "gui/window.h"
#include "gui/widget.h"
#include "gui/button.h"
#include "gui/canvas.h"
#include "gui/widgetview.h"
#include "gui/scrollinglabel.h"

class Item;
class Spell;
class SpecialSkill;

class InfoGui : public WidgetView {

 private:
  Scourge *scourge;
  Item *item;
  Spell *spell;
  SpecialSkill *skill;
  Window *win;
  Button *openButton, *idButton, *closeButton, *useButton, *transcribeButton, *castButton, *skillButton;
  ScrollingLabel *label;
  ScrollingLabel *nameLabel;
  Canvas *image;
  enum { NAME_SIZE = 500, DESCR_SIZE = 1000 }; 
  char name[NAME_SIZE], description[DESCR_SIZE];

 public:
  InfoGui(Scourge *scourge);
  ~InfoGui();

  bool handleEvent(Widget *widget, SDL_Event *event);

  inline Item *getItem() { return item; }
  inline Spell *getSpell() { return spell; }
  inline SpecialSkill *getSkill() { return skill; }
  inline bool hasItem() { return item != NULL; }
  inline bool hasSpell() { return spell != NULL; }
  inline bool hasSkill() { return skill != NULL; }
  void setItem(Item *item);
  void setSpell(Spell *spell);
  void setSkill(SpecialSkill *skill);
  inline Window *getWindow() { return win; }

  void drawWidgetContents(Widget *w);

protected:
  void describe();
  void appendMagicItemInfo( char *description, Item *item );
  void describeRequirements( char *description, int influenceTypeCount );
};

#endif

