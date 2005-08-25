/***************************************************************************
                          renderedcreature.h  -  description
                             -------------------
    begin                : Sat May 3 2003
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

#ifndef RENDERED_CREATURE_H
#define RENDERED_CREATURE_H

#include "render.h"

class RenderedItem;
class Effect;
class GLShape;
class Shapes;
class Map;

/**
 * @author Gabor Torok
 * 
 * A creature rendered on the map.
 */

class RenderedCreature {
protected:
  GLfloat x, y, z;
  GLuint effectDuration;
  GLuint damageEffectCounter;
  Effect *effect;
  int effectType;
  Preferences *preferences;
  Shapes *shapes;
  Map *levelMap;

public:
  RenderedCreature( Preferences *preferences, 
                    Shapes *shapes, 
                    Map *levelMap );
  virtual ~RenderedCreature();

  virtual inline GLfloat getX() { return x; }
  virtual inline GLfloat getY() { return y; }
  virtual inline GLfloat getZ() { return z; }
  virtual inline void moveTo(GLfloat x, GLfloat y, GLfloat z) { this->x = x; this->y = y; this->z = z; }

  virtual bool getStateMod(int mod) = 0;
  virtual void pickUpOnMap( RenderedItem *item ) = 0;
  virtual GLShape *getShape() = 0;
  virtual char *getName() = 0;
  virtual bool isMonster() = 0;
  virtual CreatureInfo *save() = 0;
  virtual bool canAttack( RenderedCreature *creature ) = 0;
  virtual void setSelXY( int x, int y, bool force=false ) = 0;

  // effects
  virtual Effect *getEffect();
  virtual void startEffect( int effect_type, int duration = Constants::DAMAGE_DURATION, GLuint delay=0 );
  virtual inline void setEffectType(int n) { this->effectType = n; }
  virtual inline int getEffectType() { return effectType; }  
  virtual inline int getDamageEffect() { return damageEffectCounter; }
  virtual inline void resetDamageEffect() { damageEffectCounter = SDL_GetTicks(); }
  virtual inline bool isEffectOn() { return (SDL_GetTicks() - damageEffectCounter < effectDuration ? true : false); }

};


#endif

