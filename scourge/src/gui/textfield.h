
/***************************************************************************
                          textfield.h  -  description
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

#ifndef TEXTFIELD_H
#define TEXTFIELD_H

#include "../constants.h"
#include "widget.h"
#include "window.h"
#include "label.h"

/**
  *@author Gabor Torok
  */

class TextField : public  Widget {
private:
  int numChars;
  bool inside; // was the last event inside the button?
  char *text;
  int pos, maxPos;

public: 

  TextField(int x, int y, int numChars);
  ~TextField();
  bool handleEvent(Widget *parent, SDL_Event *event, int x, int y);
  void drawWidget(Widget *parent);
  inline char *getText() { text[maxPos] = '\0'; return text; }
  inline void setFocus(bool b) { Widget::setFocus(b); inside = b; }
};

#endif

