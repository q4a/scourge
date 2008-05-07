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

#ifndef CHECKBOX_H
#define CHECKBOX_H


#include <vector>
#include "gui.h"
#include "widget.h"
#include "label.h"
#include "button.h"

/**
  *@author Daroth-U
  */
  
#define CHECKBOX_SIZE  15

class SDLHandler;
class Label;
class Button;

class Checkbox : public Widget {
 private:
  int x2, y2;
  Label *staticLabel;
  Button * checkButton;  
  bool inside;  
  bool checked;
  void toggleCheck();
  void applyCheck();
  
 public: 

  Checkbox(int x1, int y1, int x2, int y2, GLuint highlight, char *staticText);
  ~Checkbox();
  bool isInside(int x, int y); 
  inline bool isChecked() { return checked; } 
  void setCheck(bool val);
        
  bool handleEvent(Widget *parent, SDL_Event *event, int x, int y);
  void drawWidget(Widget *parent);
};

#endif

