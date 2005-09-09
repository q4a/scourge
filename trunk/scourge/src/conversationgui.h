/***************************************************************************
                          conversationgui.h  -  description
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

#ifndef CONVERSATION_GUI_H
#define CONVERSATION_GUI_H

#include <iostream>
#include <string>
#include <map>
#include "constants.h"
#include "scourge.h"
#include "gui/window.h"
#include "gui/widget.h"
#include "gui/button.h"
#include "gui/canvas.h"
#include "gui/widgetview.h"
#include "gui/scrollinglabel.h"

class Creature;

class ConversationGui : public WordClickedHandler, WidgetView {

 private:
  Scourge *scourge;
  Creature *creature;
  Window *win;
  bool useCreature;
  Label *label;
  ScrollingLabel *answer;
  
  ScrollingList *list;
  char **words;
  int wordCount;
  static const int MAX_WORDS = 1000;
  TextField *entry;
  Canvas *canvas;

  Button *closeButton, *tradeButton, *trainButton;
  Button *identifyButton, *uncurseItemButton, *rechargeButton;
  Button *healButton, *donateButton;
  CardContainer *cards;
  
 public:
  ConversationGui(Scourge *scourge);
  ~ConversationGui();

  bool handleEvent(Widget *widget, SDL_Event *event);

  inline Creature *getCreature() { return creature; }
  void start(Creature *creature);
  void start(Creature *creature, char *message, bool useCreature);
  inline Window *getWindow() { return win; }
  void hide();

  void wordClicked( char *word );
  void showingWord( char *word );

  void drawWidgetContents(Widget *w);
};

#endif

