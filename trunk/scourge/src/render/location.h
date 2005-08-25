
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

class Location {
public:
  // shapes
  Uint16 x, y, z;
  Shape *shape;
  RenderedItem *item;
  RenderedCreature *creature;
};

class EffectLocation {
public:
  Uint16 x, y, z;
  GLuint effectDuration;
  GLuint damageEffectCounter;
  Effect *effect;
  int effectType;
  GLuint effectDelay;

  // effects
  inline void setEffectType(int n) { this->effectType = n; }
  inline int getEffectType() { return effectType; }  
  inline Effect *getEffect() { return effect; }
  inline int getDamageEffect() { return damageEffectCounter; }
  inline void resetDamageEffect() { damageEffectCounter = SDL_GetTicks(); }
  inline bool isEffectOn() { 
	return (SDL_GetTicks() - damageEffectCounter < effectDuration + effectDelay ? true : false); 
  }
  inline bool isInDelay() {
	return (SDL_GetTicks() - damageEffectCounter < effectDelay ? true : false); 
  }
  inline void setEffectDelay(GLuint n) { this->effectDelay = n; }
  inline GLuint getEffectDelay() { return effectDelay; }
};

#endif

