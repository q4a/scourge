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

class ProjectileRenderer {
public:
  ProjectileRenderer() {
  }
  virtual ~ProjectileRenderer() {
  }

  virtual void draw() = 0;
  virtual void setCameraRot( float x, float y, float z ) = 0;
  virtual bool drawLater() = 0;
  virtual void setupBlending() = 0;
  virtual void endBlending() = 0;

  virtual float getZ() { return 7.0f; }
  virtual int getStepsDrawn() { return 1; }
  virtual int getTimeToLiveAfterImpact() { return 0; }
  virtual bool engulfTarget() { return false; }
  virtual int getStepInc() { return 1; }
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

  virtual inline void draw() { shape->draw(); }
  virtual inline void setCameraRot( float x, float y, float z ) { ((GLShape*)shape)->setCameraRot( x, y, z ); }
  virtual inline bool drawLater() { return shape->drawLater(); }
  virtual inline void setupBlending() { shape->setupBlending(); }
  virtual inline void endBlending() { shape->endBlending(); }
};

class EffectProjectileRenderer : public ProjectileRenderer {
private:
  Effect *effect;
  int effectType;
  int timeToLive;
public:
  EffectProjectileRenderer( Effect *effect, int effectType, int timeToLive );
  virtual ~EffectProjectileRenderer();

  virtual void draw();
  virtual void setCameraRot( float x, float y, float z );
  virtual bool drawLater();
  virtual void setupBlending();
  virtual void endBlending();

  virtual inline float getZ() { return 0.25f; }
  virtual inline int getStepsDrawn() { return -1; }
  virtual inline int getTimeToLiveAfterImpact() { return timeToLive; }
  virtual inline bool engulfTarget() { return true; }
  virtual inline int getStepInc() { return 3; }
};

#endif


