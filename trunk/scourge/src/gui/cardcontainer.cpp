/***************************************************************************
                          cardcontainer.cpp  -  description
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
#include "cardcontainer.h"

/**
  *@author Gabor Torok
  */

CardContainer::CardContainer(Window *window) {
  this->window = window;
  cardCount = 0;
  for(int i = 0; i < MAX_CARDS; i++) {
	widgetCount[i] = 0;
  }
  activeCard = 0;
}

CardContainer::~CardContainer() {
}

void CardContainer::setActiveCard(int card) {
  for(int i = 0; i < widgetCount[activeCard]; i++) {
	containedWidget[activeCard][i]->setVisible(false);
  }
  activeCard = card;
  for(int i = 0; i < widgetCount[activeCard]; i++) {
	containedWidget[activeCard][i]->setVisible(true);
  }
}

void CardContainer::addWidget(Widget *w, int card) {
  containedWidget[card][widgetCount[card]++] = w;
  w->setVisible(card == activeCard);
  window->addWidget(w);
}

