/***************************************************************************
                     glteleporter.h  -  Teleporter shape
                             -------------------
    begin                : Thu Jul 10 2003
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

#ifndef GLTELEPORTER_H
#define GLTELEPORTER_H
#pragma once

#include "render.h"
#include "glshape.h"

/**
  *@author Gabor Torok
  */

/// A teleporter 3D shape.
class GLTeleporter : public GLShape  {
private:
	static const int MAX_STARS = 20;
	float star[MAX_STARS][2];
	float starColor[MAX_STARS][3];
	float starAngle[MAX_STARS];
	float starSpeed[MAX_STARS];

	static const int MAX_RINGS = 20;
	float ring[MAX_RINGS];
	float delta[MAX_RINGS];
	Texture flameTex;

	int teleporterType;

public:

	enum {
		BASIC_TELEPORTER = 0,
		ABYSSAL_TELEPORTER
	};

	/**
	Passing 0 for texture disables the creation of
	shapes. (eg. torch, md2 shape)
	*/
	GLTeleporter( Texture texture[], Texture flameTex,
	              int width, int depth, int height,
	              char const* name, int descriptionGroup,
	              Uint32 color, Uint8 shapePalIndex = 0,
	              int teleporterType = BASIC_TELEPORTER );

	~GLTeleporter();

	void draw();

	inline bool drawFirst() {
		return false;
	}
	inline bool drawLater() {
		return true;
	}
	inline void setupBlending() {
		glBlendFunc( GL_SRC_ALPHA, GL_ONE );
	}
	inline int getTeleporterType() {
		return teleporterType;
	}
	inline void outline( float r, float g, float b ) {}

protected:
	void commonInit( Texture flameTex );
	DECLARE_NOISY_OPENGL_SUPPORT();

};

#endif
