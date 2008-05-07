/***************************************************************************
                          canvas.h  -  description
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

#ifndef CANVAS_H
#define CANVAS_H

#include "gui.h"
#include "widget.h"
#include "widgetview.h"
#include "draganddrop.h"

/**
  *@author Gabor Torok
  */

class Canvas : public Widget {
 private:
  WidgetView *view;
  int x2, y2;
  DragAndDropHandler *dragAndDropHandler;
  int dragX, dragY;
  bool dragging;
  bool highlightBorders;
  bool highlightOnMouseOver;
  bool inside;
  bool glowing;
	bool drawBorders;

 public: 
  Canvas(int x, int y, int x2, int y2, WidgetView *view, 
         DragAndDropHandler *dragAndDropHandler = NULL, 
         bool highlightOnMouseOver=false);
  virtual ~Canvas();
  inline WidgetView *getView() { return view; }
  void drawWidget(Widget *parent);

  inline void setGlowing( bool b ) { glowing = b; }
  inline bool isGlowing() { return glowing; }

	inline void setDrawBorders( bool b ) { drawBorders = b; }
	inline bool getDrawBorders() { return drawBorders; }

  /**
	 Return true, if the event activated this widget. (For example, button push, etc.)
	 Another way to think about it is that if true, the widget fires an "activated" event
	 to the outside world.
   */
  bool handleEvent(Widget *parent, SDL_Event *event, int x, int y);
  void removeEffects(Widget *parent);
  inline void resize(int w, int h) { Widget::resize(w, h); x2 = getX() + w; y2 = getY() + h; }
  inline bool canGetFocus() { return false; }

  // don't play sound when the value changes
  virtual inline bool hasSound() { return false; }

	void cancelDrag();
};

class ImageCanvas : public Canvas, WidgetView {
private:
	GLuint image;

public:
	ImageCanvas( int x, int y, int x2, int y2, GLuint image );
  virtual ~ImageCanvas();
	virtual void drawWidgetContents(Widget *w);
};

#endif

