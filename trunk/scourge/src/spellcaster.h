/***************************************************************************
                          spellcaster.h  -  description
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

#ifndef SPELL_CASTER_H
#define SPELL_CASTER_H

#include <iostream>
#include <string>
#include <vector>
#include "constants.h"
#include "session.h"
#include "battle.h"

/**
  *@author Gabor Torok
  */

class Session;
class Battle;
class Projectile;
class ProjectileRenderer;

class SpellCaster {
 private:
  Battle *battle;
  Spell *spell;
  bool projectileHit;
  float power;
  int level;

 public:

  SpellCaster( Battle *battle, Spell *spell, bool projectileHit, int level );
  virtual ~SpellCaster();
  
  void spellFailed();

  void spellSucceeded();

 protected:
  inline int getLevel() { return level; }
  float getPower();
  void viewInfo();
  void increaseHP();
  void increaseAC();
  // count==0 means that count depends on level
  Projectile *launchProjectile( int count, bool stopOnImpact=true, ProjectileRenderer *renderer=NULL );
  void causeDamage( bool multiplyByLevel=true, GLuint delay=0, GLfloat mult=1.0f );
  void setStateMod(int mod, bool setting=true);
  void circleAttack();
  void hailAttack();
  void resurrect();

  int getRadius( int spellEffectSize, float *sx, float *sy );
};

#endif
