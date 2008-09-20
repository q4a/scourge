/***************************************************************************
          renderedprojectile.h  -  Class representing a projectile
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

#ifndef RENDERED_PROJECTILE_H
#define RENDERED_PROJECTILE_H

#include <map>
#include <vector>
#include "render.h"

/**
 * @author Gabor Torok
 * 
 */

class RenderedCreature;
class ProjectileRenderer;

class RenderedProjectile {
protected:
  static std::map<RenderedCreature*, std::vector<RenderedProjectile*>*> projectiles;
  
 public:
   RenderedProjectile();
   virtual ~RenderedProjectile();
  
   virtual int getStepCount() = 0;
   virtual float getX( int index ) = 0;
   virtual float getY( int index ) = 0;
   virtual float getZ( int index ) = 0;
   virtual float getCurrentX() = 0;
   virtual float getCurrentY() = 0;
   virtual float getCurrentZ() = 0;
   virtual float getAngle() = 0;
   virtual ProjectileRenderer *getRenderer() = 0;
   virtual RenderedCreature *getCreature() = 0;

   inline static std::map<RenderedCreature *, std::vector<RenderedProjectile*>*> *getProjectileMap() { return &projectiles; }
   static void resetProjectiles();
   static void removeProjectile( RenderedProjectile *p );

protected:
   static void addProjectile( RenderedProjectile *proj );
   
};

#endif
