/***************************************************************************
                          label.h  -  description
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
#include "canvas.h"

/**
  *@author Gabor Torok
  */

Canvas::Canvas(int x, int y, int x2, int y2, WidgetView *view, 
               DragAndDropHandler *dragAndDropHandler) : 
  Widget(x, y, x2 - x, y2 - y) {
	this->view = view;
  this->dragAndDropHandler = dragAndDropHandler;
	this->x2 = x2;
	this->y2 = y2;
  this->dragging = false;
}

Canvas::~Canvas() {
}

void Canvas::drawWidget(Widget *parent) {
  if(view) {
    glPushMatrix();
    view->drawWidget(this);
    glPopMatrix();
  }
  // draw the border
  applyBorderColor();
  glBegin(GL_LINES);
  glVertex2d(0, 0);
  glVertex2d(0, y2 - y);
  glVertex2d(x2 - x, 0);
  glVertex2d(x2 - x, y2 - y);
  glVertex2d(0, 0);
  glVertex2d(x2 - x, 0);
  glVertex2d(0, y2 - y);
  glVertex2d(x2 - x, y2 - y);
  glEnd();
}

bool Canvas::handleEvent(Widget *parent, SDL_Event *event, int x, int y) {
	bool inside = (x >= getX() && x < x2 && y >= getY() && y < y2);
	switch(event->type) {
	case SDL_MOUSEMOTION:
  if((abs(dragX - x) > DragAndDropHandler::DRAG_START_DISTANCE ||
      abs(dragY - y) > DragAndDropHandler::DRAG_START_DISTANCE) &&
     dragAndDropHandler) {
    if(!dragAndDropHandler->startDrag(this, dragX, dragY)) {
      // cancel drag
      dragging = false;
    }
  }
  break;
  case SDL_MOUSEBUTTONUP:
  if(!dragging && inside) {
    if(dragAndDropHandler) dragAndDropHandler->receive(this);
  }
  dragging = false;
  return inside;
  case SDL_MOUSEBUTTONDOWN:
  dragging = inside;
  dragX = x - getX();
  dragY = y - getY();
  break;
  }
  return false;
}

