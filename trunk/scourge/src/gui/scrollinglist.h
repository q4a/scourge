/***************************************************************************
                          scrollinglist.h  -  description
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

#ifndef SCROLLING_LIST_H
#define SCROLLING_LIST_H

#include "../constants.h"
#include "widget.h"
#include "button.h"
#include "window.h"

/**
  *@author Gabor Torok
  */

class ScrollingList : public Widget {
 protected:
  int count;
  const char **list;
  int value;
  int scrollerWidth, scrollerHeight;
  int listHeight;
  float alpha, alphaInc;
  GLint lastTick;
  bool inside;
  int scrollerY;

 public: 
  ScrollingList(int x, int y, int w, int h);
  virtual ~ScrollingList();

  inline int getLineCount() { return count; }
  void setLines(int count, const char *s[]);
  inline const char *getLine(int index) { return list[index]; }

  void drawWidget(Widget *parent);

  /**
	 Return true, if the event activated this widget. (For example, button push, etc.)
	 Another way to think about it is that if true, the widget fires an "activated" event
	 to the outside world.
   */
  bool handleEvent(Widget *parent, SDL_Event *event, int x, int y);

};

#endif

