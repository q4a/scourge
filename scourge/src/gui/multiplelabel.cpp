/***************************************************************************
                    multiplelabel.cpp  -  description
                             -------------------
    begin                : Thu Mar 09 2004
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
#include "multiplelabel.h"
#include "window.h"

/**
  *@author Daroth-U  
  */

MultipleLabel::MultipleLabel(int x1, int y1, int x2, int y2, char *staticText, int dynWidth) : 
  Widget(x1, y1, x2 - x1, y2 - y1) {
  
  this->x2 = x2;
  this->y2 = y2; 
  this->staticLabel  = new Label(0, 0, staticText);
  this->dynamicLabel = new Label(0, 0, NULL);   
  this->dynWidth = dynWidth;
  this->currentText = 0;        
}

MultipleLabel::~MultipleLabel() {
  delete staticLabel;
  delete dynamicLabel;
}

void MultipleLabel::drawWidget(Widget *parent) { 
      
  // Draw rectangle
  glColor3f(0.5f, 0.5f, 0.1f);
  //glColor3f(0.0f, 0.3f, 0.9f);   
 /* glBegin(GL_LINE_STRIP);  
  glVertex2d(x2 - dynWidth, getY());
  glVertex2d(x2 - dynWidth, y2);
  glVertex2d(x2, y2);
  glVertex2d(x2, getY());
  glVertex2d(x2 - dynWidth, getY());
  glEnd();  */
  
  
  // Draw texts
  glPushMatrix();
  glTranslated( 15, 15, 0);
  staticLabel->drawWidget(parent);
  glTranslated(x2 - dynWidth, 0, 0);
  dynamicLabel->drawWidget(parent);
  glPopMatrix();
}

void MultipleLabel::addText(char * s){
    vText.push_back(s);     
}

void MultipleLabel::setText(int i){
    if(i >= 0 && i < vText.size()){
        dynamicLabel->setText(vText[i]);
    }   
}

void MultipleLabel::setNextText(){
    currentText++;
    if(currentText >= vText.size()){
        currentText = 0;
    }
    setText(currentText);
}

// inside includes only the dynamic text
bool MultipleLabel::isInside(int x, int y) {
  /*return(x >= getX() && x < getX() + w &&
  		  y >= getY() && y < getY() + h);*/  
  /*cout << " x , y = " << x << ", " << y <<endl;
  cout << (x2 - dynWidth) << " , " << x2<< endl;
  cout << getY() << ", " << y2 - getY() << endl;*/
  	return(x >= (x2 - dynWidth) && x < x2 && y >= getY() && y < y2);
}

bool MultipleLabel::handleEvent(Widget *parent, SDL_Event *event, int x, int y) {
  inside = isInside(x, y);
  // handle it
  switch( event->type ) {
  case SDL_MOUSEMOTION:
	break;
  case SDL_MOUSEBUTTONUP:
    if(inside){setNextText();}
	//return inside;
  case SDL_MOUSEBUTTONDOWN:
	break;
  default:
	break;
  }
  return false;
}

