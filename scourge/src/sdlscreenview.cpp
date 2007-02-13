/***************************************************************************
                          sdlscreenview.cpp  -  description
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

#include "sdlscreenview.h"

using namespace std;

SDLScreenView::SDLScreenView(){
	updateEvent[0] = 0;
	updateValue = -1;
	updateTotal = -1;
}
SDLScreenView::~SDLScreenView(){
}

// return true if the screen needs to be updated
bool SDLScreenView::setUpdate( char *message, int n, int total ) {
	int oldValue = updateValue;
	int percent = (int)( n / ( total / 100.0f) );

	strcpy( this->updateEvent, message );
	this->updateValue = n;
	this->updateTotal = total;

	return( oldValue <= 0 || 
					updateValue <= 0 || 
					percent == 100 || 
					( updateValue % 10 ) == 0 ? true : false );
}

