/***************************************************************************
                     eventhandler.h  -  Widget event handler interface
                             -------------------
    begin                : Sat Nov 28 2008
    copyright            : (C) 2008 by Gabor Torok
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
#ifndef EVENTHANDLER_H_
#define EVENTHANDLER_H_
#pragma once

#include "gui.h"
#include "widget.h"

/**
  *@author Gabor Torok
  */

/// Event handler interface for widgets
class EventHandler {
public:
	EventHandler();
	virtual ~EventHandler();

	virtual bool handleEvent( Widget *w, SDL_Event *event ) = 0;
};

class RawEventHandler {
public:
	RawEventHandler();
	virtual ~RawEventHandler();

	virtual bool handleEvent( SDL_Event *event ) = 0;	
};

#endif /*EVENTHANDLER_H_*/
