/***************************************************************************
                          slider.h  -  description
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

#ifndef SLIDER_H
#define SLIDER_H

#include "../constants.h"
#include "widget.h"
#include "label.h"

/**
  *@author Gabor Torok
  */

class Slider : public Widget {
 private:
  int x2, y2;
  int minValue, maxValue;
  Label *label;
  bool inside; // was the last event inside the button?
  bool dragging;
  int pos;

 public: 

  Slider(int x1, int y1, int x2, int y2, int minValue=0, int maxValue=100, char *label=NULL);
  ~Slider();
  bool handleEvent(Widget *parent, SDL_Event *event, int x, int y);
  void removeEffects(Widget *parent);
  void drawWidget(Widget *parent);

  inline int getValue() {
    return (int)((float)(pos * (maxValue - minValue)) / (float)getWidth());
  }

  inline int getStep() {
    return (int)((float)getWidth() / (float)(maxValue - minValue));
  }

};

#endif

