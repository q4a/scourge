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
#include "glshape.h"
#include "map.h"
#include "shapes.h"

using namespace std;

Effect::Effect( Map *levelMap, Preferences *preferences, Shapes *shapePal, int width, int height ) {
  this->levelMap = levelMap;
  this->preferences = preferences;
  this->shapePal = shapePal;
  this->shape = new GLShape(0, width, height, width, NULL,0, 0, 2000);
  this->shape->initialize();
  this->deleteShape = true;
  commonInit();
}

void Effect::reset() {
  if( !deleteShape ) {
    cerr << "ERROR: Effect::reset() should only be called when the shape is created internally in the effect class!" << endl;
    // cause a segfault (so you can debug the stacktrace)
    ((Effect*)NULL)->getShape();
  }  
  deleteParticles();
  commonInit();
}  

Effect::Effect( Map *levelMap, Preferences *preferences, Shapes *shapePal, GLShape *shape ) {
  this->levelMap = levelMap;
  this->preferences = preferences;
  this->shapePal = shapePal;
  this->shape = shape;
  this->deleteShape = false;
  commonInit();
}

void Effect::commonInit() {
  flameTex = shapePal->getTexture(9);
  ringTex = shapePal->getTexture(18);
  rippleTex = shapePal->getRippleTexture();
  for(int i = 0; i < PARTICLE_COUNT; i++) {
    particle[i] = NULL;
  }
  rippleRadius = 2.0f;
  rippleAlpha = 0.4f;
  ringRadius = 0.25f;
  ringRotate = 0.0f;
  lastTimeStamp = 0;
}

Effect::~Effect() {
  if(deleteShape) delete shape;
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

void Effect::draw(int effect, int startTime) {
  GLint t = SDL_GetTicks();

  bool proceed = (lastTimeStamp == 0 || 
                  t - lastTimeStamp >= preferences->getGameSpeedTicks() / 2);
  if(proceed) lastTimeStamp = t;

  if(effect == Constants::EFFECT_FLAMES) {
    drawFlames(proceed);
  } else if(effect == Constants::EFFECT_TELEPORT) {
    drawTeleport(proceed);
  } else if(effect == Constants::EFFECT_GREEN) {
    drawGreen(proceed);
  } else if(effect == Constants::EFFECT_EXPLOSION) {
    drawExplosion(proceed);
  } else if(effect == Constants::EFFECT_SWIRL) {
    drawSwirl(proceed);
  } else if(effect == Constants::EFFECT_CAST_SPELL) {
    drawCastSpell(proceed);
  } else if(effect == Constants::EFFECT_RING) {
    drawRing(proceed);
  } else if(effect == Constants::EFFECT_RIPPLE) {
    drawRipple(proceed);
  } else if(effect == Constants::EFFECT_DUST) {
    drawDust(proceed);
  } else if(effect == Constants::EFFECT_HAIL) {
    drawHail(proceed);
  } else if(effect == Constants::EFFECT_TOWER) {
    drawTower(proceed);
  } else {
    glowShape(proceed, startTime);
  }
}

void Effect::glowShape(bool proceed, int startTime) {
  glColor4f( 1, 0, 0, 1 );
  int t = SDL_GetTicks();
  float scale = 1.0f + (float)(t - startTime) / Constants::DAMAGE_DURATION;
  if(scale > 2.0f) scale = 2.0f;
  glScalef(scale, scale, scale);
  shape->draw();
}

void Effect::drawFlames(bool proceed) {
  // manage particles
  for(int i = 0; i < PARTICLE_COUNT; i++) {
    if(!particle[i]) {
      createParticle(&(particle[i]));
      particle[i]->zoom = 1.4f;
      particle[i]->tail = true;
    } else if(proceed) {
      moveParticle(&(particle[i]));
    }
    
    // draw it      
    if(particle[i]) {            
      
      float gg = 1 - ((float)particle[i]->life / 3.0f);
      if(gg < 0) gg = 0;
      glColor4f(1, gg, 1, 0.5);

      particle[i]->tailColor.r = 1;
      particle[i]->tailColor.g = gg;
      particle[i]->tailColor.b = 1;
      particle[i]->tailColor.a = 0.5f;
      
      drawParticle(particle[i]);
    }
  }
}

void Effect::drawTeleport(bool proceed) {
  // manage particles
  for(int i = 0; i < PARTICLE_COUNT; i++) {
    if(!particle[i]) {
      // create a new particle
      createParticle(&(particle[i]));
      particle[i]->z = (int)(2.0f * rand()/RAND_MAX) + 7.0f;
      particle[i]->moveDelta = 0.15f + (0.15f * rand()/RAND_MAX);
      if(particle[i]->z < 8) particle[i]->moveDelta *= -1.0f;
      particle[i]->maxLife = 10000;
      particle[i]->trail = 4;
      particle[i]->zoom = 1.4f;
      particle[i]->tail = true;
    } else if(proceed) {
      moveParticle(&(particle[i]));
    }
    
    // draw it      
    if(particle[i]) {            
      
      //	  float c = (((float)particle[i]->life) / ((float)particle[i]->maxLife));
      float c = fabs(particle[i]->z - 8) / 8.0f;
      if(c > 1) c = 1;
      glColor4f(c / 2.0f, c, 1.0f, 0.5);

      particle[i]->tailColor.r = c / 2.0f;
      particle[i]->tailColor.g = c;
      particle[i]->tailColor.b = 1;
      particle[i]->tailColor.a = 0.5f;
      
      drawParticle(particle[i]);
    }
  }
}

void Effect::drawGreen(bool proceed) {
  // manage particles
  for(int i = 0; i < PARTICLE_COUNT / 4; i++) {
    if(!particle[i]) {
      // create a new particle
      createParticle(&(particle[i]));
	  particle[i]->z = (int)(1.0f * rand()/RAND_MAX);
	  //	  particle[i]->moveDelta = 0.15f + (0.15f * rand()/RAND_MAX);
	  particle[i]->moveDelta = 0.15f;
	  particle[i]->rotate = (180.0f * rand()/RAND_MAX);
	  particle[i]->maxLife = toint( 30.0f * rand() / RAND_MAX ) + 20;
	  particle[i]->trail = 2;
	  particle[i]->zoom = ( 2.0f * rand() / RAND_MAX ) + 1.0f;
    } else if(proceed) {
	  particle[i]->rotate += (3.0f * rand()/RAND_MAX) - 6.0f;

	  // this causes an explosion!
	  //particle[i]->zoom += 0.3f;
	  moveParticle(&(particle[i]));
    }

    // draw it      
    if(particle[i]) {            

	  //	  float c = (((float)particle[i]->life) / ((float)particle[i]->maxLife));
      float max = particle[i]->maxLife / 4;
      float c = 1.0f;
      if( particle[i]->life <= max ) {
        c = (float)( particle[i]->life ) / max;
      } else if( particle[i]->life > particle[i]->maxLife - max ) {
        c = (float)( particle[i]->maxLife - particle[i]->life ) / max;
      }
      //float c = (float)( particle[i]->maxLife - particle[i]->life ) / (float)(particle[i]->maxLife);
      glColor4f( 0.15f, 1, 0.15f, 0.2f * c );
      
	  //float c = fabs(particle[i]->z - 8) / 8.0f;
	  //if(c > 1) c = 1;
    //glColor4f(c / 4.0f, c, c / 4.0f, 0.15);

	  drawParticle(particle[i]);
    }
  }
}

void Effect::drawExplosion(bool proceed) {
  // manage particles
  for(int i = 0; i < PARTICLE_COUNT; i++) {
    if(!particle[i]) {
      // create a new particle
      createParticle(&(particle[i]));
	  particle[i]->z = (int)(2.0f * rand()/RAND_MAX) + 3.0f;
	  //	  particle[i]->moveDelta = 0.15f + (0.15f * rand()/RAND_MAX);
	  particle[i]->moveDelta = 0;
	  particle[i]->rotate = (180.0f * rand()/RAND_MAX);
	  particle[i]->maxLife = 5000;
	  particle[i]->trail = 4;
    } else if(proceed) {
	  particle[i]->rotate = (360.0f * rand()/RAND_MAX);

	  // this causes an explosion!
	  if(particle[i]->zoom < 4.0f) particle[i]->zoom += 0.5f;
	  moveParticle(&(particle[i]));
    }

    // draw it      
    if(particle[i]) {            

	  //	  float c = (((float)particle[i]->life) / ((float)particle[i]->maxLife));
	  float c = fabs(particle[i]->z - 8) / 8.0f;
	  if(c > 1) c = 1;
      glColor4f(c, c / 2.0f, c / 2.0f, 0.5);

	  drawParticle(particle[i]);
    }
  }
}

void Effect::drawDust(bool proceed) {
  // manage particles
  for(int i = 0; i < PARTICLE_COUNT; i++) {
    if(!particle[i]) {
      // create a new particle
      createParticle(&(particle[i]));
	  particle[i]->z = (int)(2.0f * rand()/RAND_MAX);
	  //	  particle[i]->moveDelta = 0.15f + (0.15f * rand()/RAND_MAX);
	  particle[i]->moveDelta = 0;
	  particle[i]->rotate = (180.0f * rand()/RAND_MAX);
	  particle[i]->maxLife = 5000;
	  particle[i]->trail = 4;
    } else if(proceed) {
	  particle[i]->rotate = (360.0f * rand()/RAND_MAX);

	  // this causes an explosion!
	  if(particle[i]->zoom < 4.0f) particle[i]->zoom += 0.5f;
	  moveParticle(&(particle[i]));
    }

    // draw it      
    if(particle[i]) {            

	  //	  float c = (((float)particle[i]->life) / ((float)particle[i]->maxLife));
	  float c = fabs(particle[i]->z - 8) / 8.0f;
	  if(c > 1.0f) c = 1.0f;
      glColor4f(c / 4.0f, c / 4.0f, c / 4.0f, 0.35);

	  drawParticle(particle[i]);
    }
  }
}

void Effect::drawHail(bool proceed) {
  // manage particles
  for(int i = 0; i < PARTICLE_COUNT / 4; i++) {
    if(!particle[i]) {
      // create a new particle
      createParticle(&(particle[i]));
      particle[i]->z = (int)(2.0f * rand()/RAND_MAX) + 5.0f;
      particle[i]->moveDelta = 0.5f + (0.3f * rand()/RAND_MAX);
      particle[i]->moveDelta *= -1.0f;
      particle[i]->maxLife = 40000;
      particle[i]->trail = 2;
      particle[i]->zoom = 4.0f;
      particle[i]->tail = true;
      particle[i]->untilGround = true;
    } else if(proceed) {
      moveParticle(&(particle[i]));
    }

    // draw it      
    if(particle[i]) {            

	  //	  float c = (((float)particle[i]->life) / ((float)particle[i]->maxLife));
	  float c = fabs(particle[i]->z - 8) / 8.0f;
	  if(c > 1) c = 1;
    glColor4f( 0, c / 4.0f, 1.0f, 0.75 );
    
    particle[i]->tailColor.r = 0.15f;
    particle[i]->tailColor.g = c / 3.0f;
    particle[i]->tailColor.b = 0.85f;
    particle[i]->tailColor.a = 0.5f;

	  drawParticle(particle[i]);
    }
  }
}

void Effect::drawTower(bool proceed) {
  // manage particles
  for(int i = 0; i < PARTICLE_COUNT; i++) {
    if(!particle[i]) {
      // create a new particle
      createParticle(&(particle[i]));
      if( (i % 3) ) {
        particle[i]->z = (int)(1.0f * rand()/RAND_MAX);
        //	  particle[i]->moveDelta = 0.15f + (0.15f * rand()/RAND_MAX);
        particle[i]->moveDelta = 0.15f;
        particle[i]->rotate = (180.0f * rand()/RAND_MAX);
        particle[i]->maxLife = 5000;
        particle[i]->trail = 2;
        particle[i]->zoom = 1.5f;
      } else {
        particle[i]->z = (int)(2.0f * rand()/RAND_MAX);
        particle[i]->moveDelta = 0.5f + (0.3f * rand()/RAND_MAX);
        particle[i]->maxLife = 40000;
        particle[i]->trail = 8;
        particle[i]->zoom = 4.0f;
        particle[i]->tail = true;
      }
    } else if(proceed) {
      if( ( i % 3 ) ) {
        particle[i]->rotate += (3.0f * rand()/RAND_MAX) - 6.0f;
      }
      moveParticle(&(particle[i]));
    }
    
    // draw it      
    if(particle[i]) {            
      
      //	  float c = (((float)particle[i]->life) / ((float)particle[i]->maxLife));
      float c = fabs(particle[i]->z - 8) / 8.0f;
      if(c > 1) c = 1;
      glColor4f( 1.0f, c / 4.0f, 0, 0.75 );
      
      particle[i]->tailColor.r = 0.85f;
      particle[i]->tailColor.g = c / 3.0f;
      particle[i]->tailColor.b = 0.15f;    
      particle[i]->tailColor.a = 0.25f;
      
      drawParticle(particle[i]);
    }
  }
}

void Effect::drawSwirl(bool proceed) {
  // manage particles
  for(int i = 0; i < PARTICLE_COUNT; i++) {
	float angle = (float)i * (360.0f / (float)PARTICLE_COUNT);
    if(!particle[i]) {
      // create a new particle
      createParticle(&(particle[i]));
	  particle[i]->x = (((float)(shape->getWidth()) / 2.0f) / DIV) +
		(((float)(shape->getWidth()) / 2.0f) / DIV) * cos(angle);
	  particle[i]->y = (((float)(shape->getDepth()) / 2.0f) / DIV) +
		(((float)(shape->getDepth()) / 2.0f) / DIV) * sin(angle);
	  particle[i]->z = 1;
	  particle[i]->moveDelta = 0.15f;
	  particle[i]->rotate = angle;
	  particle[i]->maxLife = 5000;
	  //particle[i]->trail = 2;
    } else if(proceed) {
	  particle[i]->zoom += 0.01f;
	  particle[i]->rotate += 5.0f;
	  particle[i]->x = (((float)(shape->getWidth()) / 2.0f) / DIV) + 
		(((float)(shape->getWidth()) / 2.0f) / DIV) * cos(particle[i]->rotate);
	  particle[i]->y = (((float)(shape->getDepth()) / 2.0f) / DIV) +
		(((float)(shape->getDepth()) / 2.0f) / DIV) * sin(particle[i]->rotate);
	  moveParticle(&(particle[i]));
    }

    // draw it      
    if(particle[i]) {            

	  //	  float c = (((float)particle[i]->life) / ((float)particle[i]->maxLife));
	  float c = fabs(particle[i]->z - 8) / 8.0f;
	  if(c > 1) c = 1;
      glColor4f(c / 2.0f, c / 4.0f, c, 0.5);

	  drawParticle(particle[i]);
    }
  }
}

void Effect::drawCastSpell(bool proceed) {
  // manage particles
  for(int i = 0; i < PARTICLE_COUNT; i++) {
    if(!particle[i]) {
      // create a new particle
      createParticle(&(particle[i]));
	  particle[i]->z = (int)(2.0f * rand()/RAND_MAX) + 3.0f;
	  //	  particle[i]->moveDelta = 0.15f + (0.2f * rand()/RAND_MAX);
	  particle[i]->moveDelta = 0;
	  particle[i]->maxLife = 10000;
	  particle[i]->trail = 1;
	  particle[i]->zoom = 0.5f;
    } else if(proceed) {
	  moveParticle(&(particle[i]));
    }

    // draw it      
    if(particle[i]) {            
	  //	  float c = (((float)particle[i]->life) / ((float)particle[i]->maxLife));
	  float c = fabs(particle[i]->z - 8) / 8.0f;
	  if(c > 1) c = 1;

	  //	  particle[i]->rotate += 5.0f;
	  particle[i]->zoom = ((particle[i]->life % 200) / 60.0f + 1) * 1.5f;

      glColor4f(c / 2.0f, c / 4.0f, 1.0f, 0.25);

	  drawParticle(particle[i]);
    }
  }
}

void Effect::drawRing(bool proceed) {

  float r = ringRadius / DIV;

  float n = (0.05f * rand()/RAND_MAX) + 1.0f;
  for(int i = 0; i < 2; i++) {
    glPushMatrix();
    glRotatef((i == 0 ? ringRotate : -ringRotate), 0, 0, 1);
    float c = 0.15f;
    glColor4f(0.85f + (c * rand()/RAND_MAX), 
              1.0f - (c * rand()/RAND_MAX), 
              1.0f, 0.7f);
    if(ringTex) glBindTexture( GL_TEXTURE_2D, ringTex );
    glScalef( n, n, n );
    glBegin( GL_QUADS );
    // front
    glNormal3f(0.0f, 1.0f, 0.0f);
    if(ringTex) glTexCoord2f( 1.0f, 1.0f );
    glVertex3f(r, -r, 0);
    if(ringTex) glTexCoord2f( 0.0f, 1.0f );
    glVertex3f(-r, -r, 0);
    if(ringTex) glTexCoord2f( 0.0f, 0.0f );
    glVertex3f(-r, r, 0);
    if(ringTex) glTexCoord2f( 1.0f, 0.0f );
    glVertex3f(r, r, 0);  
    glEnd();
    glPopMatrix();
  }

  if(proceed) {
    if(ringRadius < shape->getWidth()) ringRadius += 0.8f;
    ringRotate += 5.0f;
  }
}

void Effect::drawRipple(bool proceed) {

  float r = rippleRadius / DIV;
  float z = 0.3f / DIV;

  glDisable( GL_CULL_FACE );
  glPushMatrix();
  glColor4f( 0.3f, 0.25f, 0.17f, rippleAlpha );
  if(rippleTex) glBindTexture( GL_TEXTURE_2D, rippleTex );
  glBegin( GL_QUADS );
  // front
  glNormal3f(0.0f, 1.0f, 0.0f);
  if(ringTex) glTexCoord2f( 1.0f, 1.0f );
  glVertex3f(r, -r, z);
  if(ringTex) glTexCoord2f( 0.0f, 1.0f );
  glVertex3f(-r, -r, z);
  if(ringTex) glTexCoord2f( 0.0f, 0.0f );
  glVertex3f(-r, r, z);
  if(ringTex) glTexCoord2f( 1.0f, 0.0f );
  glVertex3f(r, r, z);  
  glEnd();
  glPopMatrix();

  if(proceed) {
    rippleRadius += 0.1f;
    rippleAlpha /= 1.1f;
  }
}




void Effect::createParticle(ParticleStruct **particle) {
  // create a new particle
  *particle = new ParticleStruct();
  (*particle)->x = ((float)(shape->getWidth() / DIV) * rand()/RAND_MAX);
  (*particle)->y = ((float)(shape->getDepth() / DIV) * rand()/RAND_MAX);
  //  (*particle)->z = (int)(6.0 * rand()/RAND_MAX) + 10;
  (*particle)->z = (int)(0.8 * rand()/RAND_MAX);
  (*particle)->startZ = (*particle)->z;
  (*particle)->height = (int)(15.0 * rand()/RAND_MAX) + 10;
  (*particle)->life = 0;
  (*particle)->moveDelta = (0.3f * rand()/RAND_MAX) + 0.2f;
  (*particle)->maxLife = 10;
  (*particle)->trail = 1;
  (*particle)->rotate = 0.0f;
  (*particle)->zoom = 1.0f;
  (*particle)->tail = false;
  (*particle)->untilGround = false;
}

void Effect::moveParticle(ParticleStruct **particle) {
  // move this particle
  (*particle)->life++;
  (*particle)->z+=(*particle)->moveDelta;
  if((*particle)->z < 0 || (*particle)->z > MAP_VIEW_HEIGHT || 
     (!((*particle)->untilGround) && (*particle)->life >= (*particle)->maxLife)) {
    delete((*particle));
    (*particle) = 0;
  }
}

void Effect::drawParticle(ParticleStruct *particle) {
  float w, h, sh;

  w = (float)(shape->getWidth() / DIV) / 4.0f;
  //float d = (float)(shape->getDepth() / DIV) / 2.0;
  h = (float)(shape->getHeight() / DIV) / 3.0f;
  if(h == 0) h = 0.25 / DIV;
  sh = ( fabs( particle->z - particle->startZ ) / DIV) / 3.0f;
  if(h == 0) h = 0.25 / DIV;


  for(int i = 0; i < particle->trail; i++) {
    glPushMatrix();
    
    // position the particle
    //  GLfloat z = (float)(particle->z * h) / 10.0;
    GLfloat z = (particle->z + i) / DIV;
    glTranslatef( particle->x, particle->y, z );  
    
    // rotate each particle to face viewer
    //glRotatef(-shape->getZRot(), 0.0f, 0.0f, 1.0f);
    //glRotatef(-( 90 + shape->getYRot() ), 1.0f, 0.0f, 0.0f);      
    glRotatef(-levelMap->getZRot(), 0.0f, 0.0f, 1.0f);
    glRotatef(-levelMap->getYRot(), 1.0f, 0.0f, 0.0f);      
    
    if(flameTex) glBindTexture( GL_TEXTURE_2D, flameTex );
    
    // rotate particles
    glRotatef( particle->rotate, 0, 0, 1 );
    
    // zoom
    glScalef(particle->zoom, particle->zoom, particle->zoom);
    
    if( particle->tail ) {
      glColor4f( particle->tailColor.r, particle->tailColor.g, particle->tailColor.b, particle->tailColor.a );
      //glDisable( GL_TEXTURE_2D );
      glBegin( GL_QUADS );
      // front
      /*
      glNormal3f(0.0f, 1.0f, 0.0f);
      if(flameTex) glTexCoord2f( 1.0f, 1.0f );
      glVertex3f(w/2.0f, 0, -sh);
      if(flameTex) glTexCoord2f( 0.0f, 1.0f );
      glVertex3f(-w/2.0f, 0, -sh);
      if(flameTex) glTexCoord2f( 0.0f, 0.0f );
      glVertex3f(-w/2.0f, 0, 0);
      if(flameTex) glTexCoord2f( 1.0f, 0.0f );
      glVertex3f(w/2.0f, 0, 0);  
      */

      glNormal3f(0.0f, 1.0f, 0.0f);
      if(flameTex) glTexCoord2f( 1.0f, 1.0f );
      glVertex2f(w/2.0f, -sh );
      if(flameTex) glTexCoord2f( 0.0f, 1.0f );
      glVertex2f(-w/2.0f, -sh );
      if(flameTex) glTexCoord2f( 0.0f, 0.0f );
      glVertex2f(-w/2.0f, 0 );
      if(flameTex) glTexCoord2f( 1.0f, 0.0f );
      glVertex2f(w/2.0f, 0 );  

      glEnd();    
      //glEnable( GL_TEXTURE_2D );
    }
      
    /*
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
    */


    glBegin( GL_QUADS );
    glNormal3f(0.0f, 1.0f, 0.0f);
    if(flameTex) glTexCoord2f( 1.0f, 1.0f );
    glVertex2f(w/2.0f, -h/2.0f);
    if(flameTex) glTexCoord2f( 0.0f, 1.0f );
    glVertex2f(-w/2.0f, -h/2.0f );
    if(flameTex) glTexCoord2f( 0.0f, 0.0f );
    glVertex2f(-w/2.0f, h/2.0f );
    if(flameTex) glTexCoord2f( 1.0f, 0.0f );
    glVertex2f(w/2.0f, h/2.0f );  
    glEnd();
    
    // reset the model_view matrix
    glPopMatrix();
  }
}
