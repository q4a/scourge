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

#include "constants.h"
#include "glshape.h"

class MD2Shape : public GLShape  {

// uncomment to show debug shapes
//#define DEBUG_MD2 1

private:
  bool attackEffect;
  float div;
  char *skin_name;
  GLuint textureId;
  //float movex, movey, movez;  
  t3DModel * g_3DModel;                 // This holds the 3D Model info that we load in
  int g_ViewMode;                       // make this GL_LINE_STRIP for outline
  int dir;      
  vect3d *vect;     
      
  // Animation stuff
  float elapsedTime;
  float lastTime;  
  bool pauseAnimation;
  int currentAnim;
  int currentFrame; 
  int animationWaiting; // memorizes one animation waiting to be played
  float angle;
  
  // To ensure that animations with few frames are played at least once entirely
  bool playedOnce; 
      
   
  // This draws and animates the .md2 model by interpoloated key frame animation
  void AnimateMD2Model();
  
  // This returns time t for the interpolation between the current and next key frame
  float ReturnCurrentTime(int nextFrame);

  GLShape *debugShape;

  MD2Shape(t3DModel * g_3DModel, GLuint textureId, float div,
           GLuint texture[], int width, int depth, int height,
           char *name, int descriptionGroup,
           Uint32 color, Uint8 shapePalIndex=0);

public:     
	~MD2Shape();

  inline bool getAttackEffect() { return attackEffect; }
  inline void setAttackEffect(bool b) { attackEffect = b; }

  void setCurrentAnimation(int numAnim, bool force=false);

  void draw();
  void setupToDraw();

  void setDir(int dir);  
  bool drawFirst();
  // if true, the next two functions are called
  bool drawLater();
  void setupBlending();
  void endBlending();  
  inline void setPauseAnimation(bool pause) { pauseAnimation = pause; }
  inline GLuint getTextureId() { return textureId; }
  inline void setSkinName(char *s) { skin_name = s; }
  inline char *getSkinName() { return skin_name; }

  inline void setAngle(float angle) { 
    float a = angle + 90.0f;
    if(a >= 360.0f) a -= 360.0f;
    this->angle = a; 
  }

  // Factory method to create shape. It computes the dimensions and normalizes the
  // model's points.
  static MD2Shape *createShape(t3DModel *g_3DModel, GLuint textureId, float div,
                               GLuint texture[], char *name, int descriptionGroup,
                               Uint32 color, Uint8 shapePalIndex);

protected:
  void commonInit(t3DModel * g_3DModel, GLuint textureId, float div);
};

#endif
