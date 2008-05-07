/**
 * Credit for this code is mainly due to:
 * DigiBen     digiben@gametutorials.com
 * Look up his other great tutorials at:
 * http://www.gametutorials.com
 *
 * glCommands (and thus simplification of this file) is implemented 
 * thanks to David Henry tutorial : 
 *   http://tfc.duke.free.fr/us/tutorials/models/md2.htm 
 */


/***************************************************************************
                          md2shape.h  -  description
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

#ifndef MD2SHAPE_H
#define MD2SHAPE_H

#include <iostream>
#include <stdio.h>
#include <stdlib.h>

# include <string>
# include <vector>
# include <map>

#include "render.h"
#include "animatedshape.h"

class MD2Shape : public AnimatedShape  {

// uncomment to show debug shapes
//#define DEBUG_MD2 1

private:
  float div;
  GLuint textureId;
  t3DModel * g_3DModel;                 // This holds the 3D Model info that we load in
  vect3d *vect;     
      
  // Animation stuff
  float elapsedTime;
  float lastTime;  
  int currentFrame;     

  // This draws and animates the .md2 model by interpoloated key frame animation
  void AnimateMD2Model();
  
  // This returns time t for the interpolation between the current and next key frame
  float ReturnCurrentTime(int nextFrame);

protected:
  void commonInit(t3DModel * g_3DModel, GLuint textureId, float div);           

public:     
  MD2Shape(t3DModel * g_3DModel, GLuint textureId, float div,
           GLuint texture[], int width, int depth, int height,
           char *name, int descriptionGroup,
           Uint32 color, Uint8 shapePalIndex=0);

	virtual ~MD2Shape();

  void setModelAnimation();
  void draw();
  void outline( float r, float g, float b );
  void setupToDraw();
  inline GLuint getTextureId() { return textureId; }


};

#endif
