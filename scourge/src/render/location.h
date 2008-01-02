
/***************************************************************************
                          location.h  -  description
                             -------------------
    begin                : Mon May 12 2003
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

#ifndef LOCATION_H
#define LOCATION_H

#include "render.h"

class Effect;
class RenderedItem;
class Shape;
class RenderedCreature;

/**
  *@author Gabor Torok
  */

// extra display information
class DisplayInfo {
public:
  GLfloat red, green, blue;

  DisplayInfo() {
    reset();
  }
  ~DisplayInfo() {
  }

  inline void reset() { red = green = blue = 1.0f; }
  inline void copy( DisplayInfo *di ) { 
    if( di ) {
      red = di->red; 
      green = di->green; 
      blue = di->blue; 
    } else {
      reset();
    }
  }
};

class Location {
public:
  // shapes
  Uint16 x, y, z;
  float heightPos;
  Shape *shape;
  RenderedItem *item;
  RenderedCreature *creature;
  Color *outlineColor;
  float angleX, angleY, angleZ;
  float moveX, moveY, moveZ;

  Location() {
    this->creature = NULL;
    this->heightPos = 0;
    this->item = NULL;
    this->outlineColor = NULL;
    this->shape = NULL;
    this->x = this->y = this->z = 0;
	this->angleX = this->angleY = this->angleZ = 0;
	this->moveX = this->moveY = this->moveZ = 0;
  }
};

class EffectLocation {
public:
  Uint16 x, y, z;
  GLuint effectDuration;
  GLuint damageEffectCounter;
  Effect *effect;
  int effectType;
  GLuint effectDelay;
  bool forever;
	float heightPos;

  // effects
  inline void setEffectType(int n) { this->effectType = n; }
  inline int getEffectType() { return effectType; }  
  inline Effect *getEffect() { return effect; }
  inline int getDamageEffect() { return damageEffectCounter; }
  inline void resetDamageEffect() { damageEffectCounter = SDL_GetTicks(); }
  inline bool isEffectOn() { 
    return ( forever || SDL_GetTicks() - damageEffectCounter < effectDuration + effectDelay ? true : false ); 
  }
  inline bool isInDelay() {
	return (SDL_GetTicks() - damageEffectCounter < effectDelay ? true : false); 
  }
  inline void setEffectDelay(GLuint n) { this->effectDelay = n; }
  inline GLuint getEffectDelay() { return effectDelay; }
};

#endif

