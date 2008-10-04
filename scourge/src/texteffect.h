/***************************************************************************
         texteffect.h  -  Class for large text with animated effects
                             -------------------
    begin                : Sept 13, 2005
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

#ifndef TEXT_EFFECT_H
#define TEXT_EFFECT_H
#pragma once

#include <vector>

/**
  *@author Gabor Torok
  */

class Scourge;

using namespace std;

/// An individual effect particle of a TextEffect.
struct TextItemParticle {
	int life;
	float x, y;
	int r, g, b;
	float dir, step;
	float zoom;
};

/// Displays large text with a nice animated effect.
class TextEffect {
private:
	Scourge *scourge;
	char text[255];
	GLuint texture[1];
	unsigned char *textureInMemory;
	int x;
	int y;
	bool active;
	TextItemParticle particle[100];
	Uint32 lastTickMenu;

public:
	TextEffect( Scourge *scourge, int x, int y, char const* text );
	~TextEffect();

	inline void setActive( bool b ) {
		this->active = b;
	}
	inline bool isActive() {
		return active;
	}

	inline char *getText() {
		return text;
	}

	void draw();

protected:
	void drawEffect( float divisor, int count );
	void buildTextures();
};

#endif

