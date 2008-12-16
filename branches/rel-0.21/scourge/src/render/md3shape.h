/***************************************************************************
       md3shape.h  -  Extends AnimatedShape with MD3 specific functions
                             -------------------
    begin                : Thu Aug 31 2006
    copyright            : (C) 2006 by Gabor Torok
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

#ifndef MD3SHAPE_H
#define MD3SHAPE_H
#pragma once

#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <vector>
#include <map>

#include "render.h"
#include "animatedshape.h"
#include "Md3.h"

class ModelLoader;
class Texture;

/// Info about a shape's current animation status.
/// This has to be unique per model instance (so we can reuse md3-s).
struct AnimInfo {
	int currentAnim;     // The current index into pAnimations list
	int currentFrame;     // The current frame of the current animation
	int nextFrame;      // The next frame of animation to interpolate too
	float t;       // The ratio of 0.0f to 1.0f between each key frame
	float lastTime;      // This stores the last time that was stored
};

/// .md3 specific rendering functions
class MD3Shape : public AnimatedShape  {

// uncomment to show debug shapes
//#define DEBUG_MD3 1

private:
	bool cleanupDone;
	float div;
	CModelMD3 *md3;
	ModelLoader *loader;
	AnimInfo aiUpper, aiLower, aiHead;

	// This stores the texture array for each of the textures assigned to this model
	std::vector<Texture> m_Textures;
	// The list of material information (Textures and colors)
	std::vector<tMaterialInfo> pMaterialUpper, pMaterialLower, pMaterialHead;
	int numOfMaterialsUpper, numOfMaterialsLower, numOfMaterialsHead;

public:
	MD3Shape( CModelMD3 *md3, ModelLoader *loader, float div,
	          Texture texture[], int width, int depth, int height,
	          char const* name, int descriptionGroup,
	          Uint32 color, Uint8 shapePalIndex = 0 );

	virtual ~MD3Shape();

	void cleanup();
	void setModelAnimation();
	void draw();
	void outline( float r, float g, float b );
	AnimInfo *getAnimInfo( t3DModel *model );

	inline std::vector<Texture> *getTextures() {
		return &m_Textures;
	}
	std::vector<tMaterialInfo> *getMaterialInfos( t3DModel *pModel );
	int getNumOfMaterials( t3DModel *pModel );
	void setNumOfMaterials( t3DModel *pModel, int n );

protected:
	void setCurrentAnimation( int numAnim, bool force, int whichPart );
	DECLARE_NOISY_OPENGL_SUPPORT();

};

#endif
