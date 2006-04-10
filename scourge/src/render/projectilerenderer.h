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

class Map;
class RenderedProjectile;
class Preferences;
class Shapes;

class ProjectileRenderer {
public:
  ProjectileRenderer() {
  }
  virtual ~ProjectileRenderer() {
  }

	virtual void drawPath( Map *map, RenderedProjectile *proj, std::vector<CVector3> *path ) = 0;
	virtual int getTimeToLiveAfterImpact() = 0;
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
};

#endif


