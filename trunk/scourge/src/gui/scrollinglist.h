/***************************************************************************
                  scrollinglist.h  -  Scrollable list widget
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
#pragma once

#include "gui.h"
#include "widget.h"
#include "button.h"
#include "window.h"
#include "draganddrop.h"

/**
  *@author Gabor Torok
  */

/// A flexible list widget that can also display icons next to entries.
class ScrollingList : public Widget {
protected:
	std::vector<std::string> list;
	const Color *colors;
	const Texture **icons;
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
	Texture* highlight;
	bool canGetFocusVar;
	int lineHeight;
	int eventType;
	bool allowMultipleSelection;
	int tooltipLine;
	bool linewrap;
	bool iconBorder;

public:

	enum {
		EVENT_DRAG = 0,
		EVENT_ACTION
	};

	bool debug;

	ScrollingList( int x, int y, int w, int h, Texture* highlight, DragAndDropHandler *dragAndDropHandler = NULL, int lineHeight = 15 );
	virtual ~ScrollingList();

	inline void setTextLinewrap( bool b ) {
		linewrap = b;
	}
	inline void setIconBorder( bool b ) {
		iconBorder = b;
	}

	inline void setAllowMultipleSelection( bool b ) {
		allowMultipleSelection = b;
	}
	inline bool getAllowMultipleSelection() {
		return allowMultipleSelection;
	}
	//unused: inline int getLineCount() { return list.size(); }
	void setLines( int count, std::string const s[], const Color *colors = NULL, const Texture** icons = NULL );
	void setLines( const std::vector<std::string>::iterator begin, const std::vector<std::string>::iterator end, const Color *colors = NULL, Texture const** icons = NULL );
	void setLine( const std::string& toPush );
	void setLine( size_t pos, const std::string& toPush );
	inline const std::string& getLine( int index ) {
		return list[index];
	}
	void unselectAllLines();

	inline int getSelectedLine() {
		return ( selectedLine != NULL ? selectedLine[ 0 ] : -1 );
	}
	void setSelectedLine( size_t n );
	inline bool isSelected( int line ) {
		if ( selectedLine == NULL ) {
			return false;
		} else {
			for ( int i = 0; i < selectedLineCount; i++ ) {
				if ( selectedLine[i] == line ) return true;
			}
		}
		return false;
	}
	inline int getSelectedLineCount() {
		return selectedLineCount;
	}
	inline int getSelectedLine( int index ) {
		return selectedLine[ index ];
	}

	void drawWidget( Widget *parent );

	inline int getEventType() {
		return eventType;
	}

	/**
	Return true, if the event activated this widget. (For example, button push, etc.)
	Another way to think about it is that if true, the widget fires an "activated" event
	to the outside world.
	*/
	bool handleEvent( Widget *parent, SDL_Event *event, int x, int y );

	void removeEffects( Widget *parent );

	// don't play sound when the value changes
	virtual inline bool hasSound() {
		return false;
	}

	inline bool canGetFocus() {
		return canGetFocusVar;
	}
	inline void setCanGetFocus( bool b ) {
		this->canGetFocusVar = b;
	}

	int getLineAtPoint( int x, int y );

private:
	void selectLine( int x, int y, bool addToSelection = false, bool mouseDown = false );
	void drawIcon( int x, int y, Texture const* icon, Widget *parent );
	void moveSelectionUp();
	void moveSelectionDown();

	/**
	* a common function that sets up the internal state after altering the list
	*/
	void setupHeight();

	/**
	* Prints s
	* if linewrap is true it breaks the line so that no line is longer that getWidth()
	* else it just prints it
	*/
	void printLine( Widget *parent, int x, int y, const std::string& s );
};

#endif

