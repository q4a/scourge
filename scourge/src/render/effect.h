/***************************************************************************
                          effect.h  -  description
                             -------------------
    begin                : Thu Jul 10 2003
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

#ifndef EFFECT_H
#define EFFECT_H

#include "render.h"

/**
  *@author Gabor Torok
  */
  
class Preferences;
class Shapes;
class GLShape;

class Effect {
private:  
  Preferences *preferences;
  Shapes *shapePal;
  GLuint flameTex, ringTex, rippleTex;
  GLShape *shape;
  bool deleteShape;
  float ringRadius, ringRotate, rippleRadius, rippleAlpha;
  GLint lastTimeStamp;
  
  static const int PARTICLE_COUNT = 30;
  ParticleStruct *particle[PARTICLE_COUNT];

public:
  Effect( Preferences *preferences, Shapes *shapePal, GLShape *shape );
  Effect( Preferences *preferences, Shapes *shapePal, int width, int height );
  ~Effect();
  
  void deleteParticles();
  void draw(int effect, int startTime);

  inline GLShape *getShape() { return shape; }
   
protected:
  void commonInit();
  void glowShape(bool proceed, int startTime);
  void drawFlames(bool proceed);
  void drawTeleport(bool proceed);
  void drawGreen(bool proceed);
  void drawExplosion(bool proceed);
  void drawSwirl(bool proceed);
  void drawCastSpell(bool proceed);
  void drawRing(bool proceed);
  void drawRipple(bool proceed);
  void drawDust(bool proceed);
  void drawHail(bool proceed);
  void drawTower(bool proceed);
  
  // particle management
  void createParticle(ParticleStruct **particle);
  void moveParticle(ParticleStruct **particle);
  void drawParticle(ParticleStruct *particle);
   
};

#endif
