/***************************************************************************
                          scrollinglabel.h  -  description
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

#ifndef SCROLLING_LABEL_H
#define SCROLLING_LABEL_H

#include "gui.h"
#include "widget.h"
#include "button.h"
#include "window.h"
#include "draganddrop.h"
#include <vector>

/**
  *@author Gabor Torok
  */


class WordClickedHandler {
public:
	WordClickedHandler() {
  }

	virtual ~WordClickedHandler() {
  }

  virtual void wordClicked( char *word ) = 0;
  virtual void showingWord( char *word ) = 0;
};

class ScrollingLabel : public Widget {
 protected:
   static const int TEXT_SIZE = 3000;
  char text[ TEXT_SIZE ];
  int lineWidth;
  std::vector<std::string> lines;

  //  int count;
  //  const char **list;
  //  const Color *colors;
  //  const GLuint *icons;
  int value;
  int scrollerWidth, scrollerHeight;
  int listHeight;
  bool willSetScrollerHeight, willScrollToBottom;
  float alpha, alphaInc;
  GLint lastTick;
  bool inside;
  int scrollerY;
  bool dragging;
  int dragX, dragY;
  int selectedLine;
  //  DragAndDropHandler *dragAndDropHandler;
  bool innerDrag;
  int innerDragX, innerDragY;
  //  bool highlightBorders;
  // GLuint highlight;
  bool canGetFocusVar;

  std::map<char, Color> coloring;

  typedef struct _WordPos {
    int x, y, w, h;
    char word[255];
  } WordPos;
  WordPos wordPos[1000];
  int wordPosCount;
  WordClickedHandler *handler;

  std::map< std::string, int > textWidthCache;

 public: 

   bool debug;

   ScrollingLabel(int x, int y, int w, int h, char *text );
   virtual ~ScrollingLabel();

   inline void setWordClickedHandler( WordClickedHandler *handler ) { this->handler = handler; }

   inline void addColoring( char c, Color color ) { coloring[c]=color; }

   inline char *getText() { return text; }
   void setText(char *s);

   /**
    * Append text by scrolling off the top if it won't fit in the buffer.
    */
   void appendText( const char *s );

   //  inline int getLineCount() { return count; }
   //  void setLines(int count, const char *s[], const Color *colors=NULL, const GLuint *icon=NULL);
   //  inline const char *getLine(int index) { return list[index]; }

   //  inline int getSelectedLine() { return selectedLine; }
   //  void setSelectedLine(int n);

  void drawWidget(Widget *parent);

  /**
	 Return true, if the event activated this widget. (For example, button push, etc.)
	 Another way to think about it is that if true, the widget fires an "activated" event
	 to the outside world.
   */
  bool handleEvent(Widget *parent, SDL_Event *event, int x, int y);

  void removeEffects(Widget *parent);

  int getTextWidth( Widget *parent, const char *s );  

  // don't play sound when the value changes
  virtual inline bool hasSound() { return false; }

  inline bool canGetFocus() { return canGetFocusVar; }
  inline void setCanGetFocus(bool b) { this->canGetFocusVar = b; }

 private:
   char *printLine( Widget *parent, int x, int y, char *s );
   int getWordPos( int x, int y );
  //  void selectLine(int x, int y);
  //  void drawIcon( int x, int y, GLuint icon );
};

#endif

