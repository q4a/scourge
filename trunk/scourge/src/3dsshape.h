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

#include "constants.h"
#include "glshape.h"
#include "3ds.h"
#include "shapepalette.h"

using namespace std;

typedef unsigned char BYTE;

class C3DSShape : public GLShape  {

private:
  float div;
  // This holds the texture info, referenced by an ID
  unsigned int g_Texture[MAX_TEXTURES];
  // This is 3DS class.  This should go in a good model class.
  CLoad3DS g_Load3ds;
  // This holds the 3D Model info that we load in
  t3DModel g_3DModel;
  // We want the default drawing mode to be normal
  int g_ViewMode;
  float movex, movey, movez;
  ShapePalette *shapePal;
  int offsetx, offsety;
  GLShape *debugShape;
  GLuint displayListStart;
  bool initialized;

public:   
  C3DSShape(char *file_name, float div, ShapePalette *shapePal,
			GLuint texture[], char *name, int descriptionGroup,
			Uint32 color, Uint8 shapePalIndex=0, int offsetx=0, int offsety=0);
  ~C3DSShape();

  void initialize();
  void draw();
  
  bool drawFirst();
  // if true, the next two functions are called
  bool drawLater();
  void setupBlending();
  void endBlending();

protected:
  void commonInit(char *file_name, float div, ShapePalette *shapePal, int offsetx, int offsety);
  void preRenderLight();
  void resolveTextures();
  void normalizeModel();
  void createDisplayList( GLuint listName, bool isShadow );
};

#endif
