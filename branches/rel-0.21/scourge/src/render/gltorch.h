/***************************************************************************
                          gltorch.h  -  Torch shape
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

#ifndef GLTORCH_H
#define GLTORCH_H
#pragma once

#include "glshape.h"

class GLShape;

/**
  *@author Gabor Torok
  */

/// A torch 3D shape.
class GLTorch : public GLShape  {
private:
	Texture flameTex;

	int PARTICLE_COUNT;
	ParticleStruct *particle[200];

	Texture torchback;
	int torch_dir;

public:
	GLTorch( Texture texture[], Texture flameTex,
	         int width, int depth, int height,
	         char const* name, int descriptionGroup,
	         Uint32 color, Uint8 shapePalIndex = 0,
			 Texture const& torchback = Texture::none(), int torch_dir = Constants::NORTH );

	~GLTorch();

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

	virtual inline bool isShownInMapEditor() {
		return false;
	}

protected:
	void initParticles();
	DECLARE_NOISY_OPENGL_SUPPORT();
};

#endif
