/***************************************************************************
                          draganddrop.h  -  description
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

#ifndef DRAG_AND_DROP_H
#define DRAG_AND_DROP_H

#include "widget.h"

/**
  *@author Gabor Torok
  */

/**
   A simple interface for getting and sending drag-n-drop
   items.
*/
class DragAndDropHandler {
public: 
	DragAndDropHandler();
	virtual ~DragAndDropHandler();

	// the distance in pixels for a drag event to start
	// so a casual click doesn't start a drag
	static const int DRAG_START_DISTANCE = 10;

  /**
    The widget received a dragged item
  */
  virtual void receive(Widget *widget) = 0;

  /**
	 The widget initiated a drag
   * return true if there's something to drag at x,y
   */
  virtual bool startDrag(Widget *widget, int x=0, int y=0) = 0;
};

#endif
