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

#include "constants.h"
#include "glshape.h"

/**
  *@author Gabor Torok
  */

class Effect {
 private:  
  GLuint flameTex;
  
  static const int PARTICLE_COUNT = 30;
  ParticleStruct *particle[PARTICLE_COUNT];

 public:
  Effect(GLuint flameTex);
  ~Effect();

  void draw(GLShape *shape, int effect, int startTime);

 protected:
  void glowShape(GLShape *shape, int startTime);
  void drawFlames(GLShape *shape);


};

#endif
