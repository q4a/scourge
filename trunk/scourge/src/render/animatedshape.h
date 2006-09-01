/***************************************************************************
                          animatedshape.h  -  description
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

#ifndef ANIMATED_SHAPE_H
#define ANIMATED_SHAPE_H

#include <iostream>
#include <stdio.h>
#include <stdlib.h>

# include <string>
# include <vector>
# include <map>

#include "render.h"
#include "glshape.h"

class ModelWrapper;

class AnimatedShape : public GLShape  {

// uncomment to show debug shapes
//#define DEBUG_MD2 1

private:
  bool attackEffect;
  int g_ViewMode;
  int dir;
  float angle;
  bool debug;  
  float creatureSpeed;
  GLShape *debugShape;
  bool pauseAnimation;
  char *skin_name;  
  
protected:
  int currentAnim;  

  AnimatedShape( int width, int depth, int height,
           			 char *name, int descriptionGroup,
           			 Uint32 color, Uint8 shapePalIndex=0 );

public:     
	virtual ~AnimatedShape();

  virtual void draw() = 0;
  virtual void outline( float r, float g, float b ) = 0;
  virtual void setupToDraw() = 0;

  void setDir(int dir);  
  bool drawFirst();
  // if true, the next two functions are called
  bool drawLater();
  void setupBlending();
  void endBlending();  
    
  inline GLShape *getDebugShape() { return debugShape; }
  inline void setCreatureSpeed( float n ) { creatureSpeed = n; }
  inline float getCreatureSpeed() { return creatureSpeed; }
  inline void setDebug(bool b) { debug = b; }
  inline bool getDebug() { return debug; }
  inline bool getAttackEffect() { return attackEffect; }
  inline void setAttackEffect(bool b) { attackEffect = b; }
  virtual void setCurrentAnimation(int numAnim, bool force=false) = 0;
  inline int getCurrentAnimation() { return currentAnim; }  
  inline void setPauseAnimation(bool pause) { pauseAnimation = pause; }
  inline bool isAnimationPaused() { return pauseAnimation; }    
  inline void setSkinName(char *s) { skin_name = s; }
  inline char *getSkinName() { return skin_name; }  

  inline void setAngle(float angle) { 
    float a = angle + 90.0f;
    if(a >= 360.0f) a -= 360.0f;
    this->angle = a; 
  }
  inline float getAngle() {
    return angle;
  }  

  // Factory method to create shape. It computes the dimensions and normalizes the
  // model's points.
  static AnimatedShape *createShape( ModelWrapper *modelWrapper, GLuint textureId, float div,
  																	GLuint texture[], char *name, int descriptionGroup,
  																	Uint32 color, Uint8 shapePalIndex);
};

#endif
