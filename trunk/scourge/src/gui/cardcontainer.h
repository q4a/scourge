/***************************************************************************
                          cardcontainer.h  -  description
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

#ifndef CARD_CONTAINER_H
#define CARD_CONTAINER_H

#include "../constants.h"
#include "widget.h"
#include "window.h"

/**
  *@author Gabor Torok
  */

class CardContainer {
 protected:
  static const int MAX_CARDS = 10;
  static const int MAX_WIDGETS = 100;

  Widget *containedWidget[MAX_CARDS][MAX_WIDGETS];
  int cardCount;
  int widgetCount[MAX_CARDS];
  int activeCard;
  Window *window;

 public: 
  CardContainer(Window *window);
  virtual ~CardContainer();

  void setActiveCard(int card);
  inline int getActiveCard() { return activeCard; }
  void addWidget(Widget *w, int card);
};

#endif

