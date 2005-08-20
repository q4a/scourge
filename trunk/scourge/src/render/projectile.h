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
#include "render.h"

using namespace std;

/**
  *@author Gabor Torok
  */

class Projectile {
 private:
  RenderedCreature *creature, *target;
  float tx, ty;
  int tw, td;
  RenderedItem *item;
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

  static map<RenderedCreature*, vector<Projectile*>*> projectiles;
  static Uint32 lastProjectileTick;
  
 public:
  Projectile(RenderedCreature *creature, RenderedCreature *target, RenderedItem *item, Shape *shape, float parabolic=0.0f, bool stopOnImpact=true, bool seeker=false);
  Projectile(RenderedCreature *creature, RenderedCreature *target, Spell *spell, Shape *shape, float parabolic=0.0f, bool stopOnImpact=true, bool seeker=false);
  Projectile(RenderedCreature *creature, int x, int y, int w, int d, Spell *spell, Shape *shape, float parabolic=0.0f, bool stopOnImpact=true);
  virtual ~Projectile();

  inline bool doesStopOnImpact() { return stopOnImpact; }

  // return true when out of moves
  bool move();

  inline float getX() { return sx; }
  inline float getY() { return sy; }
  inline float getAngle() { return angle; }
  inline Shape *getShape() { return shape; }
  inline RenderedCreature *getCreature() { return creature; }
  inline RenderedItem *getItem() { return item; }
  inline Spell *getSpell() { return spell; }

  static Projectile *addProjectile(RenderedCreature *creature, RenderedCreature *target, 
								   RenderedItem *item, Shape *shape, 
								   int maxProjectiles, bool stopOnImpact=true);
  static Projectile *addProjectile(RenderedCreature *creature, RenderedCreature *target, 
								   Spell *spell, Shape *shape, 
								   int maxProjectiles, bool stopOnImpact=true);
  static Projectile *addProjectile(RenderedCreature *creature, int x, int y, int w, int d, 
								   Spell *spell, Shape *shape, 
								   int maxProjectiles, bool stopOnImpact=true);
  static void removeProjectile(Projectile *p);
  static void moveProjectiles(Session *session);
  inline static map<RenderedCreature *, vector<Projectile*>*> *getProjectileMap() { return &projectiles; }
  static void resetProjectiles();
  bool atTargetLocation();
  void debug();

 protected:
  void commonInit();
  void calculateAngle();
};

#endif
