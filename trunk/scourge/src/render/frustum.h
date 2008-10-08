/***************************************************************************
                       frustum.h  -  Frustum culling
                             -------------------
    begin                : Thu Jul 10 2003
    copyright            : (C) 2003 by Gabor Torok
    email                : cctorok@yahoo.com

This code was originally written by:
  Ben Humphrey (DigiBen)
  Game Programmer
  DigiBen@GameTutorials.com
  Co-Web Host of www.GameTutorials.com

 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef FRUSTUM_H
#define FRUSTUM_H
#pragma once

#include "render.h"

class Shape;

/**
  *@author Gabor Torok
  */

/// This will allow us to create an object to keep track of our frustum.
/// The frustum is used to determine which objects are in our view.
class CFrustum {

public:

	// Call this every time the camera moves to update the frustum
	void CalculateFrustum();

	// This takes a 3D point and returns TRUE if it's inside of the frustum
	bool PointInFrustum( float x, float y, float z );

	// This takes a 3D point and a radius and returns TRUE if the sphere is inside of the frustum
	bool SphereInFrustum( float x, float y, float z, float radius );

	// This takes the center and half the length of the cube.
	bool CubeInFrustum( float x, float y, float z, float size );

	bool ShapeInFrustum( float x, float y, float z, Shape *shape );

private:

	// This holds the A B C and D values for each side of our frustum.
	float m_Frustum[6][4];
	DECLARE_NOISY_OPENGL_SUPPORT();
};

#endif
