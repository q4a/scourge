/***************************************************************************
                          textscroller.h  -  description
                             -------------------
    begin                : Sat Apr 14 2007
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

#ifndef TEXT_SCROLLER_H
#define TEXT_SCROLLER_H

#include "common/constants.h"
#include <vector>
#include <set>

class Scourge;

class TextScroller {
private:
	Scourge *scourge;
	std::vector<std::string> text;
	int offset;
	Uint32 lastCheck;

public:
	TextScroller( Scourge *scourge );
	~TextScroller();

	void addDescription( char *description, float r=1.0f, float g=1.0f, float b=0.4f );
	void draw();
	void scrollUp();
	void scrollDown();
};

#endif

