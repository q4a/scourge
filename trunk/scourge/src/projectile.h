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
#include "render/renderedprojectile.h"

class Creature;
class Item;
class Spell;
class Session;
class ProjectileRenderer;

/**
  *@author Gabor Torok
  */

class Projectile : public RenderedProjectile {
 private:
  Creature *creature, *target;
  float tx, ty;
  int tw, td;
  Item *item;
  Spell *spell;
  std::vector<float> sx, sy;
  float ex, ey; 
  float angle;
  float parabolic;
  int q;
  int cx, cy;
  int steps;
  ProjectileRenderer *renderer;
  float maxDist;
  float startX, startY, distToTarget;
  bool stopOnImpact;
  bool seeker;
  Uint32 timeToLive;
	bool reachedTarget;

  static Uint32 lastProjectileTick;
  
 public:
  Projectile(Creature *creature, Creature *target, Item *item, ProjectileRenderer *renderer, float parabolic=0.0f, bool stopOnImpact=true, bool seeker=false);
  Projectile(Creature *creature, Creature *target, Spell *spell, ProjectileRenderer *renderer, float parabolic=0.0f, bool stopOnImpact=true, bool seeker=false);
  Projectile(Creature *creature, int x, int y, int w, int d, Spell *spell, ProjectileRenderer *renderer, float parabolic=0.0f, bool stopOnImpact=true);
  virtual ~Projectile();

  inline bool doesStopOnImpact() { return stopOnImpact; }

  // return true when out of moves
  bool move();

  inline int getStepCount() { return sx.size(); }
  inline float getX( int index ) { return( index < 0 ? sx[0] : sx[ index ] ); }
  inline float getY( int index ) { return( index < 0 ? sy[0] : sy[ index ] ); }
  inline float getZ( int index ) { return 0; }
  inline float getCurrentX() { return sx.back(); }
  inline float getCurrentY() { return sy.back(); }
  inline float getCurrentZ() { return 0; }
  inline float getAngle() { return angle; }
  inline ProjectileRenderer *getRenderer() { return renderer; }
  inline RenderedCreature *getCreature() { return (RenderedCreature*)creature; }
  inline Item *getItem() { return item; }
  inline Spell *getSpell() { return spell; }

  static Projectile *addProjectile(Creature *creature, Creature *target, 
								   Item *item, ProjectileRenderer *renderer,
								   int maxProjectiles, bool stopOnImpact=true);
  static Projectile *addProjectile(Creature *creature, Creature *target, 
								   Spell *spell, ProjectileRenderer *renderer, 
								   int maxProjectiles, bool stopOnImpact=true);
  static Projectile *addProjectile(Creature *creature, int x, int y, int w, int d, 
								   Spell *spell, ProjectileRenderer *renderer, 
								   int maxProjectiles, bool stopOnImpact=true);
  static void moveProjectiles(Session *session);
  bool atTargetLocation();
  void debug();                                           

 protected:
  void commonInit();
  void calculateAngle( float sx, float sy );
};

#endif
