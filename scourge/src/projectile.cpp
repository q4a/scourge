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

#define DELTA 1.0f

Projectile::Projectile(Creature *creature, Creature *target, Item *item, Shape *shape) {
  this->creature = creature;
  this->target = target;
  this->item = item;
  this->spell = NULL;
  this->shape = shape;

  commonInit();
}

Projectile::Projectile(Creature *creature, Creature *target, Spell *spell, Shape *shape) {
  this->creature = creature;
  this->target = target;
  this->item = NULL;
  this->spell = spell;
  this->shape = shape;

  commonInit();
}

void Projectile::commonInit() {
  this->sx = creature->getX() + (creature->getShape()->getWidth() / 2);
  this->sy = creature->getY() - (creature->getShape()->getDepth() / 2);
  this->ex = target->getX() + (target->getShape()->getWidth() / 2);
  this->ey = target->getY() - (target->getShape()->getDepth() / 2);

  int x = (int)(ex - sx);
  int y = (int)(ey - sy);
  this->angle = Constants::toAngle(atan((float)y / (float)x));

  // read about the arctan problem: 
  // http://hyperphysics.phy-astr.gsu.edu/hbase/ttrig.html#c3
  q = 1;
  if(x < 0) { 		// Quadrant 2 & 3
	q = ( y >= 0 ? 2 : 3);
	angle += 180;
  } else if(y < 0) { // Quadrant 4
	q = 4;
	angle += 360;
  }
  //  cerr << "NEW PROJECTILE: (" << sx << "," << sy << ")-(" << ex << "," << ey << ") angle=" << angle << " q=" << q << endl;
  cx = cy = 0;
  steps = 0;
}

Projectile::~Projectile() {
}

bool Projectile::move() {
  // are we at the target location?
  if((item && steps >= item->getRpgItem()->getDistance() + target->getShape()->getWidth()) || 
	 (spell && steps >= spell->getDistance() + target->getShape()->getWidth()) ||
	 (sx == ex && sy == ey)) return true;
  steps++;

  // angle-based floating pt. movement
  if(sx == ex) {
	// horizontal movement
	if(sy < ey) sy+=DELTA;
	else sy-=DELTA;
  } else if(sy == ey) {
	// vertical movement
	if(sx < ex) sx+=DELTA;
	else sx-=DELTA;
  } else {
	sx += (cos(Constants::toRadians(angle)) * DELTA);
	sy += (sin(Constants::toRadians(angle)) * DELTA);
  }
  // we're not at the target yet
  return false;
}

// return null if the projectile cannot be launched
Projectile *Projectile::addProjectile(Creature *creature, Creature *target, 
									  Item *item, Shape *shape, 
									  int maxProjectiles) {
  vector<Projectile*> *v;
  if(projectiles.find(creature) == projectiles.end()) {
	v = new vector<Projectile*>();
	projectiles[creature] = v;
  } else {
	v = projectiles[creature];
  }
  if((int)v->size() > maxProjectiles) return NULL;
  Projectile *p = new Projectile(creature, target, item, shape);
  v->push_back(p);
  return p;
}

Projectile *Projectile::addProjectile(Creature *creature, Creature *target, 
									  Spell *spell, Shape *shape, 
									  int maxProjectiles) {
  vector<Projectile*> *v;
  if(projectiles.find(creature) == projectiles.end()) {
	v = new vector<Projectile*>();
	projectiles[creature] = v;
  } else {
	v = projectiles[creature];
  }
  if((int)v->size() > maxProjectiles) return NULL;
  Projectile *p = new Projectile(creature, target, spell, shape);
  v->push_back(p);
  return p;
}


void Projectile::removeProjectile(Projectile *p) {
  if(projectiles.find(p->creature) != projectiles.end()) {
	vector<Projectile*> *v = projectiles[p->creature];
	for(vector<Projectile*>::iterator e=v->begin(); e!=v->end(); ++e) {
	  Projectile *proj = *e;	
	  if(proj == p) {
		v->erase(e);
		if(v->size() == 0) {
		  projectiles.erase(p->creature);
		}
		return;
	  }
	}
  }
}

void Projectile::moveProjectiles() {
  // draw the projectiles
  vector<Projectile*> removedProjectiles;
  //    cerr << "Projectiles:" << endl;
  map<Creature *, vector<Projectile*>*> *proj = Projectile::getProjectileMap();
  for(map<Creature *, vector<Projectile*>*>::iterator i=proj->begin(); i!=proj->end(); ++i) {
	//	  Creature *creature = i->first;
	//	  cerr << "\tcreature: " << creature->getName() << endl;
	vector<Projectile*> *p = i->second;
	for(vector<Projectile*>::iterator e=p->begin(); e!=p->end(); ++e) {
	  Projectile *proj = *e;
	  //	    cerr << "\t\tprojectile at: " << proj->getX() << "," << proj->getY() << endl;
	  if(proj->move()) {
		removedProjectiles.push_back(proj);
	  }
	}
  }
  // remove projectiles
  for(vector<Projectile*>::iterator e=removedProjectiles.begin(); e!=removedProjectiles.end(); ++e) {
	Projectile::removeProjectile(*e);
  }
}
