/***************************************************************************
                          sdlscreenview.h  -  description
                             -------------------
    begin                : Sat May 3 2003
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

#ifndef SDLSCREENVIEW_H
#define SDLSCREENVIEW_H

#include "common/constants.h"

/**
  *@author Gabor Torok
  */

class SDLScreenView {
private:
	char updateEvent[300];

public: 
	SDLScreenView();
	virtual ~SDLScreenView();

  /** Draw shapes, etc. */
  virtual void drawView() = 0;

  /** Draw stuff on top of the gui */
  virtual void drawAfter() = 0;

	virtual void setUpdate( char *p ) { strcpy( updateEvent, p ); }
	virtual char *getUpdate() { return updateEvent; }
};

#endif
