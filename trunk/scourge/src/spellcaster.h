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
#include "scourge.h"
#include "map.h"
#include "creature.h"
#include "item.h"
#include "rpg/character.h"
#include "rpg/monster.h"
#include "effect.h"
#include "projectile.h"
#include "battle.h"

using namespace std;

/**
  *@author Gabor Torok
  */

class Scourge;
class Creature;
class Item;
class Projectile;
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
  void increaseHP();
  void increaseAC();
  // count==0 means that count depends on level
  void launchProjectile(int count);
  void causeDamage();
};

#endif
