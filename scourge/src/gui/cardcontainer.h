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
class Button;
class Label;
class Checkbox;

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

  // widget managment functions
  Button    * createButton(int x1, int y1, int x2, int y2, char *label, int card, bool toggle=false);   
  Label     * createLabel(int x1, int x2, char * label, int card, int color=Constants::DEFAULT_COLOR); 
  Checkbox  * createCheckbox(int x1, int y1, int x2, int y2, char *label, int card);  

  void setActiveCard(int card);
  inline int getActiveCard() { return activeCard; }
  void addWidget(Widget *w, int card, bool addToWindow=true); 
};

#endif

