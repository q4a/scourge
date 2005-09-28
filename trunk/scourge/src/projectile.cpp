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
#include "render/renderlib.h"
#include "rpg/rpglib.h"
#include "creature.h"
#include "item.h"
#include "session.h"

Uint32 Projectile::lastProjectileTick = 0;

#define DELTA 1.0f

// Uncomment line below for debug trace
//#define DEBUG_MOVEMENT 1

Projectile::Projectile(Creature *creature, Creature *target, Item *item, Shape *shape, float parabolic, bool stopOnImpact, bool seeker) {
  this->creature = creature;
  this->tx = target->getX();
  this->ty = target->getY();
  this->tw = target->getShape()->getWidth();
  this->td = target->getShape()->getDepth();
  this->target = target;
  this->item = item;
  this->spell = NULL;
  this->shape = shape;
  this->parabolic = parabolic;
  this->stopOnImpact = stopOnImpact;
  this->seeker = seeker;

  commonInit();
}

Projectile::Projectile(Creature *creature, Creature *target, Spell *spell, Shape *shape, 
                       float parabolic, bool stopOnImpact, bool seeker) {
  this->creature = creature;
  this->tx = target->getX();
  this->ty = target->getY();
  this->tw = target->getShape()->getWidth();
  this->td = target->getShape()->getDepth();
  this->target = target;
  this->item = NULL;
  this->spell = spell;
  this->shape = shape;
  this->parabolic = parabolic;
  this->stopOnImpact = stopOnImpact;
  this->seeker = seeker;

  commonInit();
}

Projectile::Projectile(Creature *creature, int x, int y, int w, int d, 
                       Spell *spell, Shape *shape, float parabolic, bool stopOnImpact) {
  this->creature = creature;
  this->tx = x;
  this->ty = y;
  this->tw = w;
  this->td = d;
  this->target = NULL;
  this->item = NULL;
  this->spell = spell;
  this->shape = shape;
  this->parabolic = parabolic;
  this->stopOnImpact = stopOnImpact;
  this->seeker = false;

  commonInit();
}

void Projectile::commonInit() {
  this->sx = creature->getX() + (creature->getShape()->getWidth() / 2);
  this->sy = creature->getY() - (creature->getShape()->getDepth() / 2);

  calculateAngle();

  //  cerr << "NEW PROJECTILE: (" << sx << "," << sy << ")-(" << ex << "," << ey << ") angle=" << angle << " q=" << q << endl;
  cx = cy = 0;
  steps = 0;

  maxDist = (spell ? spell->getDistance() : item->getDistance()) + tw;
  startX = sx;
  startY = sy;
  distToTarget = Constants::distance(startX,  startY, 
                                     1, 1,
                                     tx, ty, tw, td);
}

Projectile::~Projectile() {
}

bool Projectile::atTargetLocation() {
  return ( fabs( ex - sx ) <= 1.0f + DELTA &&
           fabs( ey - sy ) <= 1.0f + DELTA );
/*
  return( toint(ex) == toint(sx) && 
          toint(ey) == toint(sy) );
*/
  /*
  int dx = abs(toint(ex - sx));
  int dy = abs(toint(ey - sy));
  return (dx < DELTA && dy < DELTA);
  */
}

void Projectile::debug() {
  cerr << "Projectile at: " << sx << "," << sy << " target: " << ex << "," << ey <<
    " at target? " << atTargetLocation() << endl;
}

bool Projectile::move() {
  
  
  // are we at the target location?
  // return false to let the map class handle the attack.
  if(this->atTargetLocation()) {
    // clamp to target
    sx = ex;
    sy = ey;
    return false;
  }

    
  // return true to let this class handle the attack
  if( steps++ >= maxDist + 2 ) return true;


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
      /*
      cerr << "before: " << sx << "," << sy << 
        " angle=" << angle << 
        " rad=" << Constants::toRadians(angle) << 
        " cos=" << cos(Constants::toRadians(angle)) <<
        " sin=" << sin(Constants::toRadians(angle)) << endl;
        */
      sx += (cos(Constants::toRadians(angle)) * DELTA);
      sy += (sin(Constants::toRadians(angle)) * DELTA);
      //cerr << "after: " << sx << "," << sy << endl;
    }
  }

  // target-seeking missiles re-adjust their flight paths
  if(seeker && target) {
    this->tx = target->getX();
    this->ty = target->getY();
    this->tw = target->getShape()->getWidth();
    this->td = target->getShape()->getDepth();
    if(!(sx == ex && sy == ey)) calculateAngle();
  }

  // recalculate the distance
  distToTarget = Constants::distance(startX,  startY, 
                                     1, 1,
                                     tx, ty, tw, td);

  // we're not at the target yet
  return false;
}

void Projectile::calculateAngle() {
  this->ex = tx + (tw / 2);
  this->ey = ty - (td / 2);

  int x = (int)(ex - sx);
  int y = (int)(ey - sy);
  if(!x) this->angle = (y <= 0 ? (90.0f + 180.0f) : 90.0f);
  else this->angle = Constants::toAngle(atan((float)y / (float)x));
  //cerr << "x=" << x << " y=" << y << " angle=" << angle << endl;

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
}

// return null if the projectile cannot be launched
Projectile *Projectile::addProjectile(Creature *creature, Creature *target, 
                                      Item *item, Shape *shape, 
                                      int maxProjectiles, bool stopOnImpact) {
  Projectile *p = new Projectile(creature, target, item, shape, 0.0f, stopOnImpact);
  RenderedProjectile::addProjectile( p );
  return p;
}

Projectile *Projectile::addProjectile(Creature *creature, Creature *target, 
                                      Spell *spell, Shape *shape, 
                                      int maxProjectiles, bool stopOnImpact) {
  Projectile *p = new Projectile(creature, target, spell, shape, 0.0f, stopOnImpact, true);
  RenderedProjectile::addProjectile( p );

  // add extra projectiles w. parabolic curve
  float r = 0.5f;
  for(int i = 0; i < maxProjectiles - 1; i+=2) {
    if(i < maxProjectiles - 1) {
      RenderedProjectile::addProjectile( new Projectile( creature, target, spell, shape, r, stopOnImpact, true ) );
    }
    if((i + 1) < maxProjectiles - 1) {
      RenderedProjectile::addProjectile( new Projectile( creature, target, spell, shape, -r, stopOnImpact, true ) );
    }
    r += (r/2.0f);
  }

  return p;
}

Projectile *Projectile::addProjectile(Creature *creature, int x, int y, int w, int d, 
                                      Spell *spell, Shape *shape, 
                                      int maxProjectiles, bool stopOnImpact) {
  // add a straight-flying projectile
  Projectile *p = new Projectile(creature, x, y, w, d, spell, shape, 0.0f, stopOnImpact);
  RenderedProjectile::addProjectile( p );

  // add extra projectiles w. parabolic curve
  float r = 0.5f;
  for(int i = 0; i < maxProjectiles - 1; i+=2) {
    if(i < maxProjectiles - 1) {
      RenderedProjectile::addProjectile( new Projectile( creature, x, y, w, d, spell, shape, r, stopOnImpact ) );
    }
    if((i + 1) < maxProjectiles - 1) {
      RenderedProjectile::addProjectile( new Projectile( creature, x, y, w, d, spell, shape, -r, stopOnImpact ) );
    }
    r += (r/2.0f);
  }

  return p;
}

void Projectile::moveProjectiles(Session *session) {
  Uint32 t = SDL_GetTicks();
  if(lastProjectileTick == 0 || 
     t - lastProjectileTick > (Uint32)(session->getPreferences()->getGameSpeedTicks() / 50)) {
    lastProjectileTick = t;

    if( getProjectileMap()->size() == 0 ) return;

    map<Projectile*, Creature*> battleProjectiles;

    // draw the projectiles
    vector<Projectile*> removedProjectiles;
#ifdef DEBUG_MOVEMENT 
    cerr << "Projectiles:" << endl;
#endif
    for( map<RenderedCreature *, vector<RenderedProjectile*>*>::iterator i = getProjectileMap()->begin(); 
         i != getProjectileMap()->end(); 
         ++i ) {
      vector<RenderedProjectile*> *p = i->second;
      for(vector<RenderedProjectile*>::iterator e=p->begin(); e!=p->end(); ++e) {
        Projectile *proj = (Projectile*)(*e);
#ifdef DEBUG_MOVEMENT 
        cerr << "\t\tprojectile at: (" << proj->getX() << "," << proj->getY() << 
          ") target: (" << proj->ex << "," << proj->ey << ")" << 
          " target creature: " << ( proj->target ? proj->target->getName() : "NULL" ) << endl;
#endif
        if(proj->move()) {
#ifdef DEBUG_MOVEMENT 
          cerr << "PROJ: max steps, from=" << proj->getCreature()->getName() << endl;                     
#endif
          session->getMap()->addDescription( "Projectile did not reach the target (max steps)." );
          removedProjectiles.push_back(proj);
        }
        
        // collision detection
        bool blocked = false;
        
        // proj reached the target
        if( proj->atTargetLocation() ) {
          if( proj->getSpell() &&
              proj->getSpell()->isLocationTargetAllowed()) {
#ifdef DEBUG_MOVEMENT 
            cerr << "PROJ: reached location target, from=" << proj->getCreature()->getName() << endl;                                
#endif
            session->getGameAdapter()->fightProjectileHitTurn(proj, (int)proj->getX(), (int)proj->getY());        
            blocked = true;
          } else if( proj->target ) {

            bool b = SDLHandler::intersects( toint( proj->ex ), toint( proj->ey ), 
                                             toint( 1 + DELTA ), toint( 1 + DELTA ),
                                             toint( proj->target->getX() ), toint( proj->target->getY() ),
                                             proj->target->getShape()->getWidth(),
                                             proj->target->getShape()->getHeight() );
#ifdef DEBUG_MOVEMENT             
            cerr << "PROJ: checking intersection of " << 
              "proj: (" << toint( proj->ex ) << "," << toint( proj->ey ) << "," << 
              toint( 1 + DELTA ) << "," << toint( 1 + DELTA ) << 
              " and creature: " << toint( proj->target->getX() ) << "," << toint( proj->target->getY() ) << 
              "," << proj->target->getShape()->getWidth() << "," << proj->target->getShape()->getHeight() <<
              " intersects? " << b << endl;
#endif            
            if( b ) {
#ifdef DEBUG_MOVEMENT 
              cerr << "PROJ: attacks target creature, from=" << proj->getCreature()->getName() << endl;
#endif
              battleProjectiles[ proj ] = proj->target;
              blocked = true;
#ifdef DEBUG_MOVEMENT 
            } else {
              cerr << "PROJ: target creature moved?" << endl;
#endif
            }
          }
        } 

        // proj stopped, due to something else
        if( !blocked ) {

          Location *loc = 
            session->getMap()->getLocation( toint(proj->getX()), 
                                            toint(proj->getY()), 
                                            0);
          if(loc) {
            if( loc->creature && 
                proj->getCreature()->canAttack( loc->creature ) ) {
#ifdef DEBUG_MOVEMENT 
              cerr << "PROJ: attacks non-target creature: " << loc->creature->getName() << ", from=" << proj->getCreature()->getName() << endl;
#endif
              battleProjectiles[ proj ] = (Creature*)(loc->creature);
              blocked = true;
            } else if( proj->doesStopOnImpact() &&
                       ( ( loc->item && 
                           loc->item->getShape()->getHeight() >= 6 ) ||
                         ( !loc->creature && 
                           !loc->item && loc->shape && 
                           loc->shape->getHeight() >= 6 ) ) ) {
#ifdef DEBUG_MOVEMENT 
              cerr << "PROJ: blocked by item or shape, from=" << proj->getCreature()->getName() << endl;                     
#endif
              // hit something
              session->getMap()->addDescription( "Projectile did not reach the target (blocked)." );
              blocked = true;
            }
          }
        }
        
        // remove finished projectiles
        if( blocked || proj->atTargetLocation() ) {
          
#ifdef DEBUG_MOVEMENT 
          // DEBUG INFO
          if( !blocked ) {
            cerr << "*** Warning: projectile didn't hit target ***" << endl;
            cerr << "Creature: " << proj->getCreature()->getName() << endl;
            proj->debug();
          }
#endif
          
          removedProjectiles.push_back( proj );
        }

      }
    }
    
    // fight battles
    for (map<Projectile*, Creature*>::iterator i=battleProjectiles.begin(); i!=battleProjectiles.end(); ++i) {
      Projectile *proj = i->first;
      Creature *creature = i->second;
      session->getGameAdapter()->fightProjectileHitTurn( proj, creature );
    }

    // remove projectiles
    for(vector<Projectile*>::iterator e=removedProjectiles.begin(); e!=removedProjectiles.end(); ++e) {
      Projectile *proj = *e;
      Projectile::removeProjectile(proj);
    }
  }
}

