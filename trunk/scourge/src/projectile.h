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
#include "shape.h"

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
  float sx, sy, ex, ey; 
  float angle;
  int q;
  int cx, cy;
  int steps;
  Shape *shape;

  static map<Creature*, vector<Projectile*>*> projectiles;
  
 public:
  Projectile(Creature *creature, Creature *target, Item *item, Shape *shape);
  virtual ~Projectile();

  // return true when out of moves
  bool move();

  inline float getX() { return sx; }
  inline float getY() { return sy; }
  inline float getAngle() { return angle; }
  inline Shape *getShape() { return shape; }
  inline Creature *getCreature() { return creature; }
  inline Item *getItem() { return item; }

  static Projectile *addProjectile(Creature *creature, Creature *target, 
								   Item *item, Shape *shape, 
								   int maxProjectiles);
  static void removeProjectile(Projectile *p);
  static void moveProjectiles();
  inline static map<Creature *, vector<Projectile*>*> *getProjectileMap() { return &projectiles; }

};

#endif
