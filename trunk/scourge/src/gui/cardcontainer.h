/***************************************************************************
                   cardcontainer.h  -  Tab container widget
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

#ifndef CARD_CONTAINER_H
#define CARD_CONTAINER_H
#pragma once

#include "gui.h"
#include "widget.h"
#include "window.h"

/**
  *@author Gabor Torok
  */
class Button;
class Label;
class Checkbox;

/// A widget that contains groups of other widgets, which are switched between using tabs.
class CardContainer {
protected:
	static const int MAX_CARDS = 20;
	static const int MAX_WIDGETS = 1000;

	Widget *containedWidget[MAX_CARDS][MAX_WIDGETS];
	int cardCount;
	int widgetCount[MAX_CARDS];
	int activeCard;
	Window *window;

public:
	CardContainer( Window *window );
	virtual ~CardContainer();

	// widget managment functions
	Button    * createButton( int x1, int y1, int x2, int y2, char *label, int card, bool toggle = false, Texture* texture = NULL );
	Label     * createLabel( int x1, int x2, char const* label, int card, int color = Constants::DEFAULT_COLOR );
	Checkbox  * createCheckbox( int x1, int y1, int x2, int y2, char *label, int card );

	void setActiveCard( int card );
	inline int getActiveCard() {
		return activeCard;
	}
	void addWidget( Widget *w, int card, bool addToWindow = true );
};

#endif

