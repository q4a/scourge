/***************************************************************************
                          button.h  -  description
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

#ifndef BUTTON_H
#define BUTTON_H

#include "../constants.h"
#include "widget.h"
#include "label.h"

/**
  *@author Gabor Torok
  */

class Button : public Widget {
 private:
  int x2, y2;
  Label *label;
  bool inside; // was the last event inside the button?
  float alpha, alphaInc;
  GLint lastTick;
  int labelPos;
  bool toggle;
  bool selected;

 public: 

  enum {
	TOP = 0,
	CENTER,
	BOTTOM
  };

  Button(int x1, int y1, int x2, int y2, char *label=NULL);
  ~Button();
  /**
	 Set if this button is a toggle button.
  */
  inline void setToggle(bool b) { toggle = b; }
  inline bool isToggle() { return toggle; }
  /**
	 For toggle buttons, this returns true if the button has been toggled.
  */
  inline bool isSelected() { return selected; }
  inline void setSelected(bool b) { selected = b; }
  inline void setLabelPosition(int p) { labelPos = p; }
  inline int getLabelPosition() { return labelPos; }
  inline Label *getLabel() { return label; }
  bool handleEvent(Widget *parent, SDL_Event *event, int x, int y);
  void removeEffects(Widget *parent);
  void drawWidget(Widget *parent);

};

#endif

