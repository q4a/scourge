/***************************************************************************
                          effect.cpp  -  description
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

#include "effect.h"

Effect::Effect(GLuint flameTex) {
  this->flameTex = flameTex;
  for(int i = 0; i < PARTICLE_COUNT; i++) {
    particle[i] = NULL;
  }
}

Effect::~Effect() {
  for(int i = 0; i < PARTICLE_COUNT; i++) {
    if(particle[i]) {
      delete(particle[i]);
      particle[i] = 0;
    }
  }
}

void Effect::draw(GLShape *shape, int effect, int startTime) {
  if(effect == Constants::EFFECT_FLAMES) {
	drawFlames(shape);
  } else {
	glowShape(shape, startTime);
  }
}

void Effect::glowShape(GLShape *shape, int startTime) {
  glColor4f( 1, 0, 0, 1 );
  int t = SDL_GetTicks();
  float scale = 1.0f + (float)(t - startTime) / Constants::DAMAGE_DURATION;
  if(scale > 2.0f) scale = 2.0f;
  glScalef(scale, scale, scale);
  shape->draw();
}

void Effect::drawFlames(GLShape *shape) {
  float w, d, h;

  // manage particles
  for(int i = 0; i < PARTICLE_COUNT; i++) {
    if(!particle[i]) {
      // create a new particle
      particle[i] = new ParticleStruct();
      particle[i]->x = ((float)(shape->getWidth() / GLShape::DIV) * rand()/RAND_MAX);
      particle[i]->y = ((float)(shape->getDepth() / GLShape::DIV) * rand()/RAND_MAX);
      particle[i]->z = (int)(6.0 * rand()/RAND_MAX) + 10;
      particle[i]->height = (int)(15.0 * rand()/RAND_MAX) + 10;
	  particle[i]->life = 0;
    } else {
      // move this particle
	  particle[i]->life++;
	  particle[i]->z+=0.3f;
      if(particle[i]->life >= 10) {
        delete(particle[i]);
        particle[i] = 0;
      }
    }

    // draw it      
    if(particle[i]) {            

      // save the model_view matrix
      glPushMatrix();

      w = (float)(shape->getWidth() / GLShape::DIV) / 4.0f;
      //float d = (float)(shape->getDepth() / GLShape::DIV) / 2.0;
      h = (float)(shape->getHeight() / GLShape::DIV) / 3.0f;
      if(h == 0) h = 0.25 / GLShape::DIV;

      // position the particle
      GLfloat z = (float)(particle[i]->z * h) / 10.0;
      glTranslatef( particle[i]->x, particle[i]->y, z );

      // rotate each particle to face viewer
      glRotatef(-shape->getZRot(), 0.0f, 0.0f, 1.0f);
      glRotatef(-(90.0 + shape->getYRot()), 1.0f, 0.0f, 0.0f);      

      if(flameTex) glBindTexture( GL_TEXTURE_2D, flameTex );
	  float gg = 1 - ((float)particle[i]->life / 3.0f);
	  if(gg < 0) gg = 0;
      glColor4f(1, gg, 1, 0.5);
      glBegin( GL_QUADS );
        // front
        glNormal3f(0.0f, 1.0f, 0.0f);
        if(flameTex) glTexCoord2f( 1.0f, 1.0f );
        glVertex3f(w/2.0f, 0, -h/2.0f);
        if(flameTex) glTexCoord2f( 0.0f, 1.0f );
        glVertex3f(-w/2.0f, 0, -h/2.0f);
        if(flameTex) glTexCoord2f( 0.0f, 0.0f );
        glVertex3f(-w/2.0f, 0, h/2.0f);
        if(flameTex) glTexCoord2f( 1.0f, 0.0f );
        glVertex3f(w/2.0f, 0, h/2.0f);

      glEnd();

      // reset the model_view matrix
      glPopMatrix();
    }
  }
}
