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
  deleteParticles();
}

void Effect::deleteParticles() {
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
  } else if(effect == Constants::EFFECT_TELEPORT) {
	drawTeleport(shape);
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
  float w, h;

  // manage particles
  for(int i = 0; i < PARTICLE_COUNT; i++) {
    if(!particle[i]) {
	  createParticle(shape, &(particle[i]));
    } else {
	  moveParticle(&(particle[i]));
    }

    // draw it      
    if(particle[i]) {            

	  float gg = 1 - ((float)particle[i]->life / 3.0f);
	  if(gg < 0) gg = 0;
      glColor4f(1, gg, 1, 0.5);
	  
	  drawParticle(shape, particle[i]);
    }
  }
}

void Effect::drawTeleport(GLShape *shape) {
  // manage particles
  for(int i = 0; i < PARTICLE_COUNT; i++) {
    if(!particle[i]) {
      // create a new particle
      createParticle(shape, &(particle[i]));
	  particle[i]->z = (int)(2.0f * rand()/RAND_MAX) + 7.0f;
	  particle[i]->moveDelta = 0.5f;
	  if(particle[i]->z < 8) particle[i]->moveDelta *= -1.0f;
	  particle[i]->maxLife = 10000;
    } else {
	  moveParticle(&(particle[i]));
    }

    // draw it      
    if(particle[i]) {            

	  //	  float c = (((float)particle[i]->life) / ((float)particle[i]->maxLife));
	  float c = ((float)abs(particle[i]->z - 8)) / 8.0f;
	  if(c > 1) c = 1;
      glColor4f(c / 2.0f, c, 1.0f, 0.5);

	  drawParticle(shape, particle[i]);
    }
  }
}

void Effect::createParticle(GLShape *shape, ParticleStruct **particle) {
  // create a new particle
  *particle = new ParticleStruct();
  (*particle)->x = ((float)(shape->getWidth() / GLShape::DIV) * rand()/RAND_MAX);
  (*particle)->y = ((float)(shape->getDepth() / GLShape::DIV) * rand()/RAND_MAX);
  //  (*particle)->z = (int)(6.0 * rand()/RAND_MAX) + 10;
  (*particle)->z = (int)(0.8 * rand()/RAND_MAX);
  (*particle)->height = (int)(15.0 * rand()/RAND_MAX) + 10;
  (*particle)->life = 0;
  (*particle)->moveDelta = 0.3f;
  (*particle)->maxLife = 10;
}

void Effect::moveParticle(ParticleStruct **particle) {
  // move this particle
  (*particle)->life++;
  (*particle)->z+=(*particle)->moveDelta;
  if((*particle)->z < 0 || (*particle)->z > MAP_VIEW_HEIGHT || 
	 (*particle)->life >= (*particle)->maxLife) {
	delete((*particle));
	(*particle) = 0;
  }
}

void Effect::drawParticle(GLShape *shape, ParticleStruct *particle) {
  float w, h;

  // save the model_view matrix
  glPushMatrix();

  w = (float)(shape->getWidth() / GLShape::DIV) / 4.0f;
  //float d = (float)(shape->getDepth() / GLShape::DIV) / 2.0;
  h = (float)(shape->getHeight() / GLShape::DIV) / 3.0f;
  if(h == 0) h = 0.25 / GLShape::DIV;
  
  // position the particle
  //  GLfloat z = (float)(particle->z * h) / 10.0;
  GLfloat z = particle->z / GLShape::DIV;
  glTranslatef( particle->x, particle->y, z );  

  // rotate each particle to face viewer
  glRotatef(-shape->getZRot(), 0.0f, 0.0f, 1.0f);
  glRotatef(-(90.0 + shape->getYRot()), 1.0f, 0.0f, 0.0f);      

  if(flameTex) glBindTexture( GL_TEXTURE_2D, flameTex );

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
