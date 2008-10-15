
/***************************************************************************
                    gllocator.h  -  Some unused 3D shape
                             -------------------
    begin                : Sat Sep 20 2003
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

#ifndef GLLOCATOR_H
#define GLLOCATOR_H
#pragma once

#include "glshape.h"

class GLShape;

/**
  *@author Gabor Torok
  */

/// This shape is unused in the game.
class GLLocator : public GLShape  {
private:

public:
	GLLocator( Texture* texture[],
	           int width, int depth, int height,
	           char *name, int descriptionGroup,
	           Uint32 color, Uint8 shapePalIndex = 0 );

	~GLLocator();

	void draw();

	inline bool drawFirst() {
		return false;
	}
	// if true, the next two functions are called
	inline bool drawLater() {
		return true;
	}
	inline void setupBlending() {
		glBlendFunc( GL_SRC_ALPHA, GL_ONE );
	}


protected:
	DECLARE_NOISY_OPENGL_SUPPORT();
};

#endif
