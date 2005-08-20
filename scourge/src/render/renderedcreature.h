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

#include "../constants.h"
#include "../persist.h"

class RenderedItem;
class Effect;
class GLShape;

/**
 * @author Gabor Torok
 * 
 * A creature rendered on the map.
 */

class RenderedCreature {
protected:
  GLfloat x, y, z;

public:
  RenderedCreature() { 
    x = y = z = 0; 
  }
  virtual ~RenderedCreature() {}

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

  // effects
  virtual void startEffect( int effect_type, int duration = Constants::DAMAGE_DURATION, GLuint delay=0 ) = 0;
  virtual void setEffectType(int n) = 0;
  virtual int getEffectType() = 0;
  virtual Effect *getEffect() = 0;
  virtual int getDamageEffect() = 0;
  virtual void resetDamageEffect() = 0;
  virtual bool isEffectOn() = 0;
};


#endif

