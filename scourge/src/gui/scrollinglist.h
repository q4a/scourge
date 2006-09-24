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

#include "gui.h"
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
  int *selectedLine;
  int selectedLineCount;
  DragAndDropHandler *dragAndDropHandler;
  bool innerDrag;
  int innerDragX, innerDragY;
  bool highlightBorders;
  GLuint highlight;
  bool canGetFocusVar;
  int lineHeight;
  int eventType;
  bool allowMultipleSelection;
  int tooltipLine;

 public: 

   enum {
     EVENT_DRAG=0,
     EVENT_ACTION
   };

   bool debug;

  ScrollingList(int x, int y, int w, int h, GLuint highlight, DragAndDropHandler *dragAndDropHandler = NULL, int lineHeight=15);
  virtual ~ScrollingList();

  inline void setAllowMultipleSelection( bool b ) { allowMultipleSelection = b; }
  inline bool getAllowMultipleSelection() { return allowMultipleSelection; }
  inline int getLineCount() { return count; }
  void setLines(int count, const char *s[], const Color *colors=NULL, const GLuint *icon=NULL);
  inline const char *getLine(int index) { return list[index]; }

  inline int getSelectedLine() { return ( selectedLine ? selectedLine[ 0 ] : -1 ); }
  void setSelectedLine( int n );
  inline bool isSelected( int line ) { 
    if( !selectedLine ) {
      return false; 
    } else {
      for( int i = 0; i < selectedLineCount; i++ ) {
        if( selectedLine[i] == line ) return true; 
      }
    }
    return false; 
  }
  inline int getSelectedLineCount() { return selectedLineCount; }
  inline int getSelectedLine( int index ) { return selectedLine[ index ]; }

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

  int getLineAtPoint( int x, int y );

 private:
  void selectLine(int x, int y, bool addToSelection = false, bool mouseDown=false );
  void drawIcon( int x, int y, GLuint icon );  
	void moveSelectionUp();
	void moveSelectionDown();
};

#endif

