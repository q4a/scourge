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

Projectile::Projectile(Creature *creature, Creature *target, Item *item, Shape *shape, float parabolic, bool stopOnImpact) {
  this->creature = creature;
  this->tx = target->getX();
  this->ty = target->getY();
  this->tw = target->getShape()->getWidth();
  this->td = target->getShape()->getDepth();
  this->item = item;
  this->spell = NULL;
  this->shape = shape;
  this->parabolic = parabolic;
  this->stopOnImpact = stopOnImpact;

  commonInit();
}

Projectile::Projectile(Creature *creature, Creature *target, Spell *spell, Shape *shape, float parabolic, bool stopOnImpact) {
  this->creature = creature;
  this->tx = target->getX();
  this->ty = target->getY();
  this->tw = target->getShape()->getWidth();
  this->td = target->getShape()->getDepth();
  this->item = NULL;
  this->spell = spell;
  this->shape = shape;
  this->parabolic = parabolic;
  this->stopOnImpact = stopOnImpact;

  commonInit();
}

Projectile::Projectile(Creature *creature, int x, int y, int w, int d, 
                       Spell *spell, Shape *shape, float parabolic, bool stopOnImpact) {
  this->creature = creature;
  this->tx = x;
  this->ty = y;
  this->tw = w;
  this->td = d;
  this->item = NULL;
  this->spell = spell;
  this->shape = shape;
  this->parabolic = parabolic;
  this->stopOnImpact = stopOnImpact;

  commonInit();
}

void Projectile::commonInit() {
  this->sx = creature->getX() + (creature->getShape()->getWidth() / 2);
  this->sy = creature->getY() - (creature->getShape()->getDepth() / 2);
  this->ex = tx + (tw / 2);
  this->ey = ty - (td / 2);

  int x = (int)(ex - sx);
  int y = (int)(ey - sy);
  this->angle = Constants::toAngle(atan((float)y / (float)x));

  // read about the arctan problem: 
  // http://hyperphysics.phy-astr.gsu.edu/hbase/ttrig.html#c3
  q = 1;
  if(x < 0) {     // Quadrant 2 & 3
    q = ( y >= 0 ? 2 : 3);
    angle += 180;
  } else if(y < 0) { // Quadrant 4
    q = 4;
    angle += 360;
  }
  //  cerr << "NEW PROJECTILE: (" << sx << "," << sy << ")-(" << ex << "," << ey << ") angle=" << angle << " q=" << q << endl;
  cx = cy = 0;
  steps = 0;

  maxDist = (spell ? spell->getDistance() : item->getRpgItem()->getDistance()) + tw;
  startX = sx;
  startY = sy;
  distToTarget = Constants::distance(startX,  startY, 
                                     1, 1,
                                     tx, ty, tw, td);
}

Projectile::~Projectile() {
}

bool Projectile::move() {
  // are we at the target location?
  if(steps >= maxDist ||
     (sx == ex && sy == ey)) return true;
  steps++;

  float oldAngle = angle;
  if(parabolic != 0.0f) {
    float a = (179.0f * steps) / distToTarget;
    angle = angle + parabolic * 40 * sin(Constants::toRadians(a));
    sx += (cos(Constants::toRadians(angle)) * DELTA);
    sy += (sin(Constants::toRadians(angle)) * DELTA);
    angle = oldAngle;
  } else {
    // angle-based floating pt. movement
    if(sx == ex) {
      // vertical movement
      if(sy < ey) sy+=DELTA;
      else sy-=DELTA; 
    } else if(sy == ey) {
      // horizontal movement
      if(sx < ex) sx+=DELTA;
      else sx-=DELTA; 
    } else {        
      sx += (cos(Constants::toRadians(angle)) * DELTA);
      sy += (sin(Constants::toRadians(angle)) * DELTA);
    }
  }

  // recalculate the distance
  distToTarget = Constants::distance(startX,  startY, 
                                     1, 1,
                                     tx, ty, tw, td);


  // we're not at the target yet
  return false;
}

// return null if the projectile cannot be launched
Projectile *Projectile::addProjectile(Creature *creature, Creature *target, 
                                      Item *item, Shape *shape, 
                                      int maxProjectiles, bool stopOnImpact) {
  vector<Projectile*> *v;
  if(projectiles.find(creature) == projectiles.end()) {
    v = new vector<Projectile*>();
    projectiles[creature] = v;
  } else {
    v = projectiles[creature];
  }
  // for items this is the max number of proj.-s in the air
  if((int)v->size() > maxProjectiles) return NULL;
  Projectile *p = new Projectile(creature, target, item, shape, 0.0f, stopOnImpact);
  v->push_back(p);
  return p;
}

Projectile *Projectile::addProjectile(Creature *creature, Creature *target, 
                                      Spell *spell, Shape *shape, 
                                      int maxProjectiles, bool stopOnImpact) {
  return addProjectile(creature, target->getX(), target->getY(), 
                       target->getShape()->getWidth(), target->getShape()->getDepth(), 
                       spell, shape, maxProjectiles, stopOnImpact);
}

Projectile *Projectile::addProjectile(Creature *creature, int x, int y, int w, int d, 
                                      Spell *spell, Shape *shape, 
                                      int maxProjectiles, bool stopOnImpact) {
  vector<Projectile*> *v;
  if(projectiles.find(creature) == projectiles.end()) {
    v = new vector<Projectile*>();
    projectiles[creature] = v;
  } else {
    v = projectiles[creature];
  }
  // FIXME: for spells, it's how many to launch at once...
  if((int)v->size() > maxProjectiles) return NULL;


  // add a straight-flying projectile
  Projectile *p = new Projectile(creature, x, y, w, d, spell, shape, 0.0f, stopOnImpact);
  v->push_back(p);

  // add extra projectiles w. parabolic curve
  float r = 0.5f;
  for(int i = 0; i < maxProjectiles - 1; i+=2) {
    if(i < maxProjectiles - 1) {
      v->push_back(new Projectile(creature, x, y, w, d, spell, shape, r, stopOnImpact));
    }
    if((i + 1) < maxProjectiles - 1) {
      v->push_back(new Projectile(creature, x, y, w, d, spell, shape, -r, stopOnImpact));
    }
    r += (r/2.0f);
  }

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

void Projectile::moveProjectiles(Scourge *scourge) {
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
    Projectile *proj = *e;
    // a location-bound projectile reached its target
    if(!proj->doesStopOnImpact()) {
      Battle::projectileHitTurn(scourge, proj, (int)proj->getX(), (int)proj->getY());
    }
    Projectile::removeProjectile(proj);
  }
}
