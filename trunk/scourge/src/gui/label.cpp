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
#include "label.h"
#include "window.h"

/**
  *@author Gabor Torok
  */

Label::Label(int x, int y, char *text) : 
  Widget(x, y, 0, 0) {
  this->text = strdup(text);
}

Label::~Label() {
  free(text);
}

void Label::drawWidget(Widget *parent) {
  applyColor();
  ((Window*)parent)->getSDLHandler()->texPrint(0, 0, text);
}

