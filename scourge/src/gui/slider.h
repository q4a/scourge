/***************************************************************************
                       slider.h  -  Drag slider widget
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
#pragma once

#include "gui.h"
#include "widget.h"
#include "label.h"

/**
  *@author Gabor Torok
  */

/// A slider widget which lets you select a value by dragging.
class Slider : public Widget {
private:
	int x2;
	int minValue, maxValue;
	Label *label;
	bool inside; // was the last event inside the button?
	bool dragging;
	int pos;
	float alpha, alphaInc;
	GLint lastTick;
	GLuint highlight;

public:

	Slider( int x1, int y1, int x2, Texture const* highlight, int minValue = 0, int maxValue = 100, char *label = NULL );
	~Slider();
	bool handleEvent( Widget *parent, SDL_Event *event, int x, int y );
	void removeEffects( Widget *parent );
	void drawWidget( Widget *parent );

	inline int getValue() {
		return static_cast<int>( static_cast<float>( pos * ( maxValue - minValue ) ) / static_cast<float>( getWidth() ) );
	}

	void setValue( int n );

	inline int getStep() {
		return static_cast<int>( static_cast<float>( getWidth() ) / static_cast<float>( maxValue - minValue ) );
	}

	// don't play sound when the value changes
	virtual inline bool hasSound() {
		return false;
	}

};

#endif

