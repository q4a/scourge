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
#include "calendar.h"
#include "events/statemodexpirationevent.h"

using namespace std;

/**
  *@author Gabor Torok
  */

class Session;
class Battle;

class SpellCaster {
 private:
  Battle *battle;
  Spell *spell;
  bool projectileHit;
  float power;

 public:

  SpellCaster(Battle *battle, Spell *spell, bool projectileHit);
  virtual ~SpellCaster();
  
  void spellFailed();

  void spellSucceeded();

 protected:
  float getPower();
  void viewInfo();
  void increaseHP();
  void increaseAC();
  // count==0 means that count depends on level
  void launchProjectile(int count, bool stopOnImpact=true);
  void causeDamage( GLuint delay=0, GLfloat mult=1.0f );
  void setStateMod(int mod, bool setting=true);
  void circleAttack();
  void hailAttack();


  int getRadius( int spellEffectSize, float *sx, float *sy );
};

#endif
