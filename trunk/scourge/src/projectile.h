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
#include "rpg/spell.h"
#include "effect.h"
#include "shape.h"

class Creature;
class Item;
class Spell;

using namespace std;

/**
  *@author Gabor Torok
  */

class Projectile {
 private:
  Creature *creature, *target;
  int tx, ty, tw, td;
  Item *item;
  Spell *spell;
  float sx, sy, ex, ey; 
  float angle;
  float parabolic;
  int q;
  int cx, cy;
  int steps;
  Shape *shape;
  float maxDist;
  float startX, startY, distToTarget;
  bool stopOnImpact;
  bool seeker;

  static map<Creature*, vector<Projectile*>*> projectiles;
  static Uint32 lastProjectileTick;
  
 public:
  Projectile(Creature *creature, Creature *target, Item *item, Shape *shape, float parabolic=0.0f, bool stopOnImpact=true, bool seeker=false);
  Projectile(Creature *creature, Creature *target, Spell *spell, Shape *shape, float parabolic=0.0f, bool stopOnImpact=true, bool seeker=false);
  Projectile(Creature *creature, int x, int y, int w, int d, Spell *spell, Shape *shape, float parabolic=0.0f, bool stopOnImpact=true);
  virtual ~Projectile();

  inline bool doesStopOnImpact() { return stopOnImpact; }

  // return true when out of moves
  bool move();

  inline float getX() { return sx; }
  inline float getY() { return sy; }
  inline float getAngle() { return angle; }
  inline Shape *getShape() { return shape; }
  inline Creature *getCreature() { return creature; }
  inline Item *getItem() { return item; }
  inline Spell *getSpell() { return spell; }

  static Projectile *addProjectile(Creature *creature, Creature *target, 
								   Item *item, Shape *shape, 
								   int maxProjectiles, bool stopOnImpact=true);
  static Projectile *addProjectile(Creature *creature, Creature *target, 
								   Spell *spell, Shape *shape, 
								   int maxProjectiles, bool stopOnImpact=true);
  static Projectile *addProjectile(Creature *creature, int x, int y, int w, int d, 
								   Spell *spell, Shape *shape, 
								   int maxProjectiles, bool stopOnImpact=true);
  static void removeProjectile(Projectile *p);
  static void moveProjectiles(Scourge *scourge);
  inline static map<Creature *, vector<Projectile*>*> *getProjectileMap() { return &projectiles; }
  static void resetProjectiles();

 protected:
  void commonInit();
  void calculateAngle();
};

#endif
