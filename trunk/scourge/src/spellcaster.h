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

using namespace std;

/**
  *@author Gabor Torok
  */

class Scourge;
class Creature;
class Item;
class Projectile;

class SpellCaster {
 public:
  
  static void spellFailed(Scourge *scourge, Creature *creature, Spell *spell, bool projectileHit);

  static void spellSucceeded(Scourge *scourge, Creature *creature, Spell *spell, bool projectileHit);

 protected:
  static float getPower(Creature *creature, Spell *spell);
  static void increaseHP(Scourge *scourge, Creature *creature, Spell *spell, float power);
  static void increaseAC(Scourge *scourge, Creature *creature, Spell *spell, float power);
  static void launchProjectile(Scourge *scourge, Creature *creature, Spell *spell, float power);
  static void causeDamage(Scourge *scourge, Creature *creature, Spell *spell, float power);
};

#endif
