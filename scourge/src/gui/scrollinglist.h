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
#include "draganddrop.h"

/**
  *@author Gabor Torok
  */

class ScrollingList : public Widget {
 protected:
  int count;
  const char **list;
  const Color *colors;
  const GLuint *icons;
  int value;
  int scrollerWidth, scrollerHeight;
  int listHeight;
  float alpha, alphaInc;
  GLint lastTick;
  bool inside;
  int scrollerY;
  bool dragging;
  int dragX, dragY;
  int selectedLine;
  DragAndDropHandler *dragAndDropHandler;
  bool innerDrag;
  int innerDragX, innerDragY;
  bool highlightBorders;
  GLuint highlight;
  bool canGetFocusVar;
  int lineHeight;
  int eventType;

 public: 

   enum {
     EVENT_DRAG=0,
     EVENT_ACTION
   };

   bool debug;

  ScrollingList(int x, int y, int w, int h, GLuint highlight, DragAndDropHandler *dragAndDropHandler = NULL, int lineHeight=15);
  virtual ~ScrollingList();

  inline int getLineCount() { return count; }
  void setLines(int count, const char *s[], const Color *colors=NULL, const GLuint *icon=NULL);
  inline const char *getLine(int index) { return list[index]; }

  inline int getSelectedLine() { return selectedLine; }
  void setSelectedLine(int n);

  void drawWidget(Widget *parent);

  inline int getEventType() { return eventType; }

  /**
	 Return true, if the event activated this widget. (For example, button push, etc.)
	 Another way to think about it is that if true, the widget fires an "activated" event
	 to the outside world.
   */
  bool handleEvent(Widget *parent, SDL_Event *event, int x, int y);

  void removeEffects(Widget *parent);

  // don't play sound when the value changes
  virtual inline bool hasSound() { return false; }

  inline bool canGetFocus() { return canGetFocusVar; }
  inline void setCanGetFocus(bool b) { this->canGetFocusVar = b; }

 private:
  void selectLine(int x, int y);
  void drawIcon( int x, int y, GLuint icon );
};

#endif

