/***************************************************************************
                      checkbox.h  -  description
                             -------------------
    begin                : Sat Mar 13 2004
    copyright            : (C) 2004 by Daroth-U
    email                : daroth-u@ifrance.com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#include "checkbox.h"
#include "window.h"

/**
  *@author Daroth-U
  */

Checkbox::Checkbox(int x1, int y1, int x2, int y2, GLuint highlight, char *staticText) : 
  Widget(x1, y1, x2 - x1, y2 - y1) {
  
  this->x2 = x2;
  this->y2 = y2; 
  this->staticLabel  = new Label(0, 0, staticText);
  this->checkButton = new Button(x2 - CHECKBOX_SIZE, y1, x2, y2, highlight); 
  this->checkButton -> setLabelPosition(Button::CENTER);  
  this->checked = false;   
}

Checkbox::~Checkbox() {
  delete staticLabel;
  delete checkButton;
}

void Checkbox::drawWidget(Widget *parent) {      
      
  // Draw texts
  glPushMatrix();      
  glTranslated(15, 15, 0);
  staticLabel->drawWidget(parent);
  glTranslated(x2 - 15 - CHECKBOX_SIZE, -15, 0);
  checkButton->drawWidget(parent);
  glPopMatrix();
}

// inside includes only the checkbox area
bool Checkbox::isInside(int x, int y) {
  	return(x >= (getX() + x2 - CHECKBOX_SIZE) && x < (getX() + x2) && y >= getY() && y < y2);
}

void Checkbox::toggleCheck(){    
    checked = !checked;
    applyCheck();      
}

void Checkbox::setCheck(bool val){
    checked = val;
    applyCheck();
}

void Checkbox::applyCheck(){
    if(checked){
        checkButton->setLabel("x");   
    }
    else{
        checkButton->setLabel(" ");   
    }  
}



bool Checkbox::handleEvent(Widget *parent, SDL_Event *event, int x, int y) {
  inside = isInside(x, y);
  // handle it
  switch( event->type ) {
  case SDL_MOUSEMOTION:
	break;
  case SDL_MOUSEBUTTONUP:
    if(inside){toggleCheck();}
	return inside;
  case SDL_MOUSEBUTTONDOWN:
	break;
  default:
	break;
  }
  return false;
}

