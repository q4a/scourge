/***************************************************************************
                      multiplelabel.h  -  description
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

#ifndef MULTIPLE_LABEL_H
#define MULTIPLE_LABEL_H

#include <vector>
#include "../constants.h"
#include "widget.h"
#include "label.h"

/**
  *@author Daroth-U
  */

class SDLHandler;
class Label;

class MultipleLabel : public Widget {
 private:
  int x2, y2;
  int dynWidth;
  vector<char *> vText;
  Label *staticLabel;
  Label *dynamicLabel;
  bool inside;
  int currentText;   
  
 public: 

  MultipleLabel(int x1, int y1, int x2, int y2, char *staticText, int dynWidth);
  ~MultipleLabel();
  bool isInside(int x, int y);
  void addText(char *s);
  void setText(int i);  
  void setNextText();
    
  bool handleEvent(Widget *parent, SDL_Event *event, int x, int y);
  void drawWidget(Widget *parent);
};

#endif

