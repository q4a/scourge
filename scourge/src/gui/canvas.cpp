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

Canvas::Canvas(int x, int y, int x2, int y2, WidgetView *view) : 
  Widget(x, y, x2 - x, y2 - y) {
	this->view = view;
	this->x2 = x2;
	this->y2 = y2;
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
