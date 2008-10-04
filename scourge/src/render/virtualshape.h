/***************************************************************************
            virtualshape.h  -  Extends GLShape for virtual shapes
                             -------------------
    begin                : Tue Jul 8 2008
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

#ifndef VIRTUALSHAPE_H_
#define VIRTUALSHAPE_H_
#pragma once

#include "glshape.h"

/**
  *@author Gabor Torok
  */

/// Helper functions for virtual shapes.
class VirtualShape : public GLShape  {
private:
	int offsetX, offsetY, offsetZ;
	bool draws;
	GLShape *refShape;

public:
	VirtualShape( char *name,
	              int width, int height, int depth,
	              int offsetX, int offsetY, int offsetZ,
	              bool draws, GLShape *refShape,
	              int shapePalIndex );

	~VirtualShape();

	void draw();
	void outline( float r, float g, float b );
	virtual inline bool isVirtual() {
		return true;
	}
	inline GLShape *getRef() {
		return refShape;
	}
	virtual inline bool isShownInMapEditor() {
		return false;
	}
	virtual inline int getOffsetX() {
		return this->offsetX;
	}
	virtual inline int getOffsetY() {
		return this->offsetY;
	}
	virtual inline int getOffsetZ() {
		return this->offsetZ;
	}
	virtual inline bool isDrawn() {
		return this->draws;
	}
};

#endif /*VIRTUALSHAPE_H_*/
