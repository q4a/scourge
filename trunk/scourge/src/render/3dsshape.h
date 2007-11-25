/**
 * Credit for this code is mainly due to:
 * http://nehe.gamedev.net
 */


/***************************************************************************
                          3dsshape.h  -  description
                             -------------------
    begin                : Fri Oct 3 2003
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

#ifndef C3DSSHAPE_H
#define C3DSSHAPE_H

//#define DEBUG_3DS

#include <iostream>
#include <stdio.h>
#include <stdlib.h>

# include <string>
# include <vector>

#include "render.h"
#include "glshape.h"
#include "3ds.h"

class Shapes;

typedef unsigned char BYTE;


class WindInfo {
private:
	float windAmp;
	float windSpeed;
	float windAngle;
	Uint32 lastWindStep;
	float ymod;
	float value;


public:
	WindInfo() {
		windAngle = lastWindStep = 0;
		windSpeed = 0.10f * rand() / RAND_MAX + 0.01f;
		windAmp = 0.5f;
		ymod = 1.2f * rand() / RAND_MAX;
		value = 0;
	}

	~WindInfo() {
	}

	inline bool update() {
		bool ret = false;
		Uint32 now = SDL_GetTicks();
		if( now - lastWindStep > 50 ) {
			lastWindStep = now;
			windAngle += windSpeed;
			if( windAngle >= 360.0f ) windAngle -= 360.0f;
			value = sin( windAngle ) * windAmp;
			ret = true;
		}
		return ret;
	}

	inline float getValue() { return value; }
	inline float getYMod() { return ymod; }
};

class C3DSShape : public GLShape  {

private:
  float divx, divy, divz;
  // This holds the texture info, referenced by an ID
  unsigned int g_Texture[MAX_TEXTURES];
  // This is 3DS class.  This should go in a good model class.
  CLoad3DS g_Load3ds;
  // This holds the 3D Model info that we load in
  t3DModel g_3DModel;
  // We want the default drawing mode to be normal
  int g_ViewMode;
  float movex, movey, movez;
  Shapes *shapePal;
	float size_x, size_y, size_z;
  float offs_x, offs_y, offs_z;
  GLShape *debugShape;
  GLuint displayListStart;
  bool initialized;
	WindInfo windInfo;
	float xrot3d, yrot3d, zrot3d;

public:   
  C3DSShape(const std::string& file_name, float div, Shapes *shapePal,
						GLuint texture[], char *name, int descriptionGroup,
						Uint32 color, Uint8 shapePalIndex=0, 
						float size_x=0, float size_y=0, float size_z=0,
						float offs_x=0, float offs_y=0, float offs_z=0,
						float xrot3d=0, float yrot3d=0, float zrot3d=0);
  ~C3DSShape();

  void initialize();
  void draw();
  void outline( float r, float g, float b );
  
  bool drawFirst();
  // if true, the next two functions are called
  bool drawLater();
  void setupBlending();
  void endBlending();
	inline float getWindValue() { return windInfo.getValue(); }

protected:
  void commonInit(const std::string& file_name, float div, Shapes *shapePal, float size_x, float size_y, float size_z, float offs_x, float offs_y, float offs_z, float xrot3d, float yrot3d, float zrot3d );
  void preRenderLight();
  void resolveTextures();
  void normalizeModel();
  void createDisplayList( GLuint listName, bool isShadow );
	void drawShape( bool isShadow );

	inline GLfloat getXRot3d() { return xrot3d; }
	inline GLfloat getYRot3d() { return yrot3d; }
	inline GLfloat getZRot3d() { return zrot3d; }

};

#endif
