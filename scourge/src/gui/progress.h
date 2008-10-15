/***************************************************************************
                      progress.h  -  Progress bar widget
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

#ifndef PROGRESS_H
#define PROGRESS_H
#pragma once

#include "gui.h"

/**
  *@author Gabor Torok
  */

/// A progress bar widget.
class Progress {
private:
	ScourgeGui *scourgeGui;
	int maxStatus;
	int status;
	bool clearScreen;
	bool center;
	bool opaque;
	Texture const* texture;
	Texture const* highlight;

public:
	Progress( ScourgeGui *scourgeGui, Texture const* texture, Texture const* highlight, int maxStatus, bool clearScreen = false, bool center = false, bool opaque = true );
	virtual ~Progress();
	void updateStatus( const char *message, bool updateScreen = true, int status = -1, int maxStatus = -1, int altStatus = -1, bool drawScreen = false );
	void updateStatusLight( const char *message, int n, int max );
	inline void setStatus( int status ) {
		this->status = status;
	}
	inline int getStatus() {
		return status;
	}
};

#endif

