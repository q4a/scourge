/***************************************************************************
                          projectile.h  -  description
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

#ifndef PROJECTILE_H
#define PROJECTILE_H

#include <map>
#include <vector>
#include "creature.h"
#include "item.h"
#include "rpg/character.h"
#include "rpg/monster.h"
#include "effect.h"

class Creature;
class Item;

using namespace std;

/**
  *@author Gabor Torok
  */

class Projectile {
 private:
  Creature *creature;
  Creature *target;
  Item *item;
  int sx, sy, ex, ey;

  static map<Creature*, vector<Projectile*>*> projectiles;
  
 public:
  Projectile(Creature *creature, Creature *target, Item *item);
  virtual ~Projectile();

  // return true when out of moves
  bool move();

  static Projectile *addProjectile(Creature *creature, Creature *target, 
								   Item *item, int maxProjectiles);

};

#endif
