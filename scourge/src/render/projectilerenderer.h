/***************************************************************************
                          projectilerenderer.h  -  description
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

#ifndef PROJECTILE_RENDERER_H
#define PROJECTILE_RENDERER_H

#include "render.h"
#include "glshape.h"
#include "effect.h"
#include <vector>
#include <set>

class Map;
class RenderedProjectile;
class Preferences;
class Shapes;

class ProjectileRenderer {
private:
	std::set<RenderedProjectile*> proj;
public:
  ProjectileRenderer() {
  }
  virtual ~ProjectileRenderer() {
		proj.clear();
  }

	virtual void drawPath( Map *map, RenderedProjectile *proj, std::vector<CVector3> *path ) = 0;
	virtual int getTimeToLiveAfterImpact() = 0;
	virtual float getOffsetX() = 0;
	virtual float getOffsetY() = 0;
	virtual float getOffsetZ() = 0;

	inline void addProjectile( RenderedProjectile *p ) { 
		proj.insert( p );
	}
	inline void removeProjectile( RenderedProjectile *p ) { 
		if( proj.find( p ) != proj.end() ) proj.erase( p );
	}
	inline bool hasProjectiles() {
		return !( proj.empty() );
	}
};

class ShapeProjectileRenderer : public ProjectileRenderer {
private:
  Shape *shape;

public:
  ShapeProjectileRenderer( Shape *shape ) {
    this->shape = shape;
  }
  virtual ~ShapeProjectileRenderer() {
  }

	virtual void drawPath( Map *map, RenderedProjectile *proj, std::vector<CVector3> *path );
	virtual inline int getTimeToLiveAfterImpact() { return 0; }
	virtual inline float getOffsetX() { return 0; }
	virtual inline float getOffsetY() { return 0; }
	virtual inline float getOffsetZ() { return 7; }
};

#define MAX_EFFECT_COUNT 100

class EffectProjectileRenderer : public ProjectileRenderer {
private:
	Effect** effects;
  int effectType;
  int timeToLive;
public:
  EffectProjectileRenderer( Map *map, Preferences *prefs, Shapes *shapes, int effectType, int timeToLive );
  virtual ~EffectProjectileRenderer();

	virtual void drawPath( Map *map, RenderedProjectile *proj, std::vector<CVector3> *path );
	virtual inline int getTimeToLiveAfterImpact() { return timeToLive; }
	virtual inline float getOffsetX() { return -1; }
	virtual inline float getOffsetY() { return 0; }
	virtual inline float getOffsetZ() { return 1; }
};

#endif


