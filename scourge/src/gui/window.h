/***************************************************************************
                         window.h  -  Window manager
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

#ifndef WINDOW_H
#define WINDOW_H
#pragma once

#include "gui.h"
#include "widget.h"
#include "button.h"
#include "label.h"
#include "checkbox.h"
#include "textfield.h"
#include "guitheme.h"
#include <vector>

/**
  *@author Gabor Torok
  */

class Widget;
class Button;
class Label;
class Checkbox;
class TextField;
class EventHandler;
class RawEventHandler;

#define TITLE_HEIGHT 21

/// Looks whether a window is closing.
class WindowListener {
public:
	WindowListener() {
	}

	virtual ~WindowListener() {
	}

	virtual void windowClosing() = 0;
};

/// A GUI window.
class Window : public Widget {
private:
	// the image is 510x270
	static const int TILE_W = 510 / 2;
	static const int TILE_H = 270 / 2;

	static const int MAX_WIDGET = 1000;

	char title[255];
	GLuint texture, texture2;
	ScourgeGui *scourgeGui;
	Widget *widget[MAX_WIDGET];
	int tileWidth, tileHeight;
	int widgetCount;
	bool dragging;
	int dragX, dragY;
	int openHeight, currentY;
	GLint lastTick;
	int z;
	bool modal;
	int type;
	bool locked;
	GuiTheme *theme;
	bool opening;
	int gutter;
	RawEventHandler *rawEventHandler;

	Widget *lastWidget;
	int animation;

	std::vector<WindowListener*> listeners;
	Widget *escapeHandler;

public:

	enum {
		DEFAULT_ANIMATION = 0,
		SLIDE_UP
	};

	static const char ROLL_OVER_SOUND[80];
	static const char ACTION_SOUND[80];
	static const char DROP_SUCCESS[80];
	static const char DROP_FAILED[80];

	Button *closeButton;

	enum {
		BASIC_WINDOW = 0,
		SIMPLE_WINDOW,
		INVISIBLE_WINDOW
	};

	static const int SCREEN_GUTTER = 0;

	Window( ScourgeGui *scourgeGui, int x, int y, int w, int h, char const* title = NULL,
		Texture const& texture = Texture::none(), bool hasCloseButton = true, int type = BASIC_WINDOW,
		Texture const& texture2 = Texture::none() );

	Window( ScourgeGui *scourgeGui, int x, int y, int w, int h, char const* title = NULL,
	        bool hasCloseButton = true, int type = BASIC_WINDOW, const char *themeName = NULL );

	~Window();
  
	virtual void resize( int w, int h );
	
	inline void setEscapeHandler( Widget *widget ) { this->escapeHandler = widget; }
	inline Widget *getEscapeHandler() { return this->escapeHandler; }
	void registerEventHandler( EventHandler *eventHandler );
	void unregisterEventHandler();
	inline void setRawEventHandler( RawEventHandler *rawEventHandler ) {
		this->rawEventHandler = rawEventHandler;
	}
	inline RawEventHandler *getRawEventHandler() {
		return rawEventHandler;
	}


	void addWindowListener( WindowListener *listener ) {
		listeners.push_back( listener );
	}

	void setMouseLock( Widget *widget );

	inline void setAnimation( int a ) {
		animation = a;
	}
	inline int getAnimation() {
		return animation;
	}

	inline GuiTheme *getTheme() {
		return theme;
	}

	inline void setTitle( char const* s ) {
		strncpy( title, ( s ? s : "" ), 255 ); title[254] = '\0';
	}
  
	inline char *getTitle() { return title; }

	inline bool isOpening() {
		return ( opening );
	}

	inline void setBackgroundTileWidth( int n ) {
		tileWidth = n;
	}
	inline void setBackgroundTileHeight( int n ) {
		tileHeight = n;
	}

	inline void setZ( int z ) {
		this->z = z;
	}
	inline int getZ() {
		return z;
	}

	inline void setModal( bool b ) {
		modal = b;
	}
	inline bool isModal() {
		return modal;
	}

	void setVisible( bool b, bool animate = true );
	inline ScourgeGui *getScourgeGui() {
		return scourgeGui;
	}
	void toTop() {scourgeGui->toTop( this );}

	void toBottom() {scourgeGui->toBottom( this );}

	inline void setLocked( bool locked ) {
		this->locked = locked; if ( locked ) toBottom();
	}
	inline bool isLocked() {
		return locked;
	}

	// crop view to window area. Don't forget to call glsDisable( GLS_SCISSOR_TEST ) after!
	void scissorToWindow( bool insideOnly = true );

	// widget managment functions
	Button    * createButton( int x1, int y1, int x2, int y2, char const* label, bool toggle = false, Texture const& texture = Texture::none() );
	Label     * createLabel( int x1, int x2, char const* label, int color = Constants::DEFAULT_COLOR );
	Checkbox  * createCheckbox( int x1, int y1, int x2, int y2, char *label );
	TextField * createTextField( int x, int y, int numChars );
	void addWidget( Widget *widget );
	void removeWidget( Widget *widget );
	Widget *handleWindowEvent( SDL_Event *event, int x, int y );
	void setFocus( Widget *w );
	void nextFocus();
	void prevFocus();

	// from Widget
	virtual void drawWidget( Window* );
	virtual bool handleEvent( Window* parent, SDL_Event* event, int x, int y );
	/// Called when no events are sent to this widget. (ie. the mouse is elsewhere.)
	/// Window forwards it to its own widgets 
	virtual void removeEffects();
	bool isInside( int x, int y );

	void move( int x, int y );

	void setLastWidget( Widget *w );

	inline int getGutter() {
		return gutter;
	}

	void setTopWindowBorderColor();
	void setWindowBorderColor();

protected:
	void commonInit( ScourgeGui *scourgeGui, int x, int y, int w, int h, char const* title, bool hasCloseButton, int type );
	void drawBorder( int topY, int openHeight );
	void drawLineBorder( int topY, int openHeight );
	void drawTitle( int topY, int openHeight );
	void drawCloseButton( int topY, int openHeight );
	void drawDropShadow( int topY, int openHeight );
	void drawBackground( int topY, int openHeight );
};

#endif

