/***************************************************************************
                          projectile.cpp  -  description
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

#include "projectile.h"

map<Creature*, vector<Projectile*>*> Projectile::projectiles;

Projectile::Projectile(Creature *creature, Creature *target, Item *item) {
  this->creature = creature;
  this->target = target;
  this->item = item;
  this->sx = creature->getX();
  this->sy = creature->getY();
  this->ex = target->getX();
  this->ey = target->getY();
}

Projectile::~Projectile() {
}

bool Projectile::move() {
  return false;
}

// return null if the projectile cannot be launched
Projectile *Projectile::addProjectile(Creature *creature, Creature *target, 
									  Item *item, int maxProjectiles) {
  vector<Projectile*> *v;
  if(projectiles.find(creature) == projectiles.end()) {
	v = new vector<Projectile*>();
	projectiles[creature] = v;
  } else {
	v = projectiles[creature];
  }
  if(v->size() > maxProjectiles) return NULL;
  Projectile *p = new Projectile(creature, target, item);
  v->push_back(p);
  return p;
}
