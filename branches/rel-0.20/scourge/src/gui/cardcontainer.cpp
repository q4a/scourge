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

using namespace std;

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

void CardContainer::addWidget(Widget *w, int card, bool addToWindow) {
  containedWidget[card][widgetCount[card]++] = w;
  w->setVisible(card == activeCard);
  
  // Add to window only if not already done
  if(addToWindow) {
    window->addWidget(w);
  }
}


Button *CardContainer::createButton(int x1, int y1, int x2, int y2, char *label, int card, bool toggle, GLuint texture){
    if(widgetCount[card] < MAX_WIDGETS){
        Button *b;
        b = window->createButton(x1, y1, x2, y2, label, toggle, texture);
        addWidget((Widget *)b, card, false);      
        return b;    	
	}
	else{
        cerr<<"Gui/CardContainer.cpp : max widget limit reached!" << endl;
        return NULL;
	}
} 

Label * CardContainer::createLabel(int x1, int x2, char const* label, int card, int color){
    if(widgetCount[card] < MAX_WIDGETS){
        Label *l;
        l = window->createLabel(x1, x2, label, color);
        addWidget((Widget *)l, card, false);   
        return l;        
    }
	else{
        cerr<<"Gui/CardContainer.cpp : max widget limit reached!" << endl;
        return NULL;
	}
} 

Checkbox * CardContainer::createCheckbox(int x1, int y1, int x2, int y2, char *label, int card){
    if(widgetCount[card] < MAX_WIDGETS){        
        Checkbox * c;
        c = window->createCheckbox(x1, y1, x2, y2, label);        
        addWidget((Widget *)c, card, false);    
        return c;
    }
    else{
        cerr<<"Gui/CardContainer.cpp : max widget limit reached!" << endl;
        return NULL;
	}    
} 

