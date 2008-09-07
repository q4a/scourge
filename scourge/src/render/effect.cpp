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
#pragma bug_probably("2000 does not fit into Uint8 (0..255)")
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
  flameTex = shapePal->findTextureByName( "flame.bmp", true );
  ringTex = shapePal->findTextureByName( "ring2.bmp", true );
  rippleTex = shapePal->getRippleTexture();
  rippleRadius = 2.0f;
  rippleAlpha = 0.4f;
  ringRadius = 0.25f;
  ringRotate = 0.0f;
  lastTimeStamp = 0;

  // reset the display info
  di.reset();
  diWasSet = false;
}

void Effect::setDisplayInfo( DisplayInfo *di ) {
  this->di.copy( di );    
  diWasSet = true;
}

Effect::~Effect() {
  if(deleteShape) delete shape;
  deleteParticles();
}

void Effect::setSize( int width, int height ) {
  if( !deleteShape ) return;
  delete shape;
#pragma bug_probably("2000 does not fit into Uint8 (0..255)")
  shape = new GLShape(0, width, height, width, NULL,0, 0, 2000);
}

void Effect::deleteParticles() {
  for(int i = 0; i < PARTICLE_COUNT; i++) {
  	particle[i].active = false;
  }
}

void Effect::draw(int effect, int startTime, float percent) {
  GLint t = SDL_GetTicks();

  bool proceed = (lastTimeStamp == 0 || 
                  t - lastTimeStamp >= preferences->getGameSpeedTicks() / 2);
  if(proceed) lastTimeStamp = t;
  
  if( diWasSet ) {
  	glPushMatrix();
  	glTranslatef( this->di.offset_x / DIV, 
  	              this->di.offset_y / DIV, 
  	              this->di.offset_z / DIV );
  }

  if(effect == Constants::EFFECT_FLAMES) {
    drawFlames(proceed);
  } else if(effect == Constants::EFFECT_TELEPORT) {
    drawTeleport(proceed);
  } else if(effect == Constants::EFFECT_GREEN) {
    drawGreen(proceed);
  } else if(effect == Constants::EFFECT_SMOKE) {
    drawSmoke(proceed);
  } else if(effect == Constants::EFFECT_FIRE) {
  	drawFire(proceed);    
  } else if(effect == Constants::EFFECT_EXPLOSION) {
    drawExplosion(proceed);
  } else if(effect == Constants::EFFECT_BLAST) {
    drawBlast(proceed, percent);
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
  
  if( diWasSet ) {
    	glPopMatrix();
  }
}

void Effect::glowShape(bool proceed, int startTime) {
  glColor4f( 1, 0, 0, 1 );
  int t = SDL_GetTicks();
  float scale = 1.0f + static_cast<float>(t - startTime) / Constants::DAMAGE_DURATION;
  if(scale > 2.0f) scale = 2.0f;
  glScalef(scale, scale, scale);
  shape->draw();
}

void Effect::drawFlames(bool proceed) {
  // manage particles
  for(int i = 0; i < PARTICLE_COUNT; i++) {
    if(!particle[i].active) {
    	particle[i].reset();
    	particle[i].x = Util::roll( 0.0f, shape->getWidth() / DIV );
    	particle[i].y = Util::roll( 0.0f, shape->getDepth() / DIV );
      particle[i].zoom = 1.4f;
      particle[i].tail = true;
    } else if(proceed) {
    	particle[i].move();
    }
    
    // draw it      
    if(particle[i].active) {            
      
      float gg = 1 - (static_cast<float>(particle[i].life) / 3.0f);
      if(gg < 0) gg = 0;
      glColor4f(1, gg, 1, 0.5);

      particle[i].tailColor.r = 1;
      particle[i].tailColor.g = gg;
      particle[i].tailColor.b = 1;
      particle[i].tailColor.a = 0.5f;
      
      drawParticle(&(particle[i]));
    }
  }
}

void Effect::drawTeleport(bool proceed) {
  // manage particles
  for(int i = 0; i < PARTICLE_COUNT; i++) {
    if(!particle[i].active) {
      // create a new particle
      particle[i].reset();
      particle[i].x = Util::roll( 0.0f, shape->getWidth() / DIV );
      particle[i].y = Util::roll( 0.0f, shape->getDepth() / DIV );
      particle[i].z = Util::pickOne( 7, 8 );
      particle[i].moveDelta = Util::roll( 0.15f, 0.3f );
      if(particle[i].z < 8) particle[i].moveDelta *= -1.0f;
      particle[i].maxLife = 10000;
      particle[i].trail = 4;
      particle[i].zoom = 1.4f;
      particle[i].tail = true;
    } else if(proceed) {
      particle[i].move();
    }

    // draw it
    if(particle[i].active) {

      float c = fabs(particle[i].z - 8) / 8.0f;
      if(c > 1) c = 1;
      glColor4f(c / 2.0f, c, 1.0f, 0.5);

      particle[i].tailColor.r = c / 2.0f;
      particle[i].tailColor.g = c;
      particle[i].tailColor.b = 1;
      particle[i].tailColor.a = 0.5f;
      
      drawParticle(&(particle[i]));
    }
  }
}

void Effect::drawGreen(bool proceed) {
  // manage particles
  for(int i = 0; i < PARTICLE_COUNT / 4; i++) {
    if(!particle[i].active) {
      // create a new particle
      particle[i].reset();
      particle[i].x = Util::roll( 0.0f, shape->getWidth() / DIV );
      particle[i].y = Util::roll( 0.0f, shape->getDepth() / DIV );
      particle[i].z = Util::roll( 0.0f, 1.0f );
      //	  particle[i].moveDelta = 0.15f + (0.15f * rand()/RAND_MAX);
      particle[i].moveDelta = 0.15f;
      particle[i].rotate = Util::roll( 0.0f, 180.0f );
      particle[i].maxLife = Util::pickOne( 20, 50 );
      particle[i].trail = 2;
      particle[i].zoom = Util::roll( 1.0f, 3.0f );
    } else if(proceed) {
      particle[i].rotate -= Util::roll( 3.0f, 6.0f );

      // this causes an explosion!
      //particle[i].zoom += 0.3f;
      particle[i].move();
    }

    // draw it      
    if(particle[i].active) {            

      float max = particle[i].maxLife / 4;
      float c = 1.0f;
      if( particle[i].life <= max ) {
        c = static_cast<float>( particle[i].life ) / max;
      } else if( particle[i].life > particle[i].maxLife - max ) {
        c = static_cast<float>( particle[i].maxLife - particle[i].life ) / max;
      }
      if( !diWasSet ) {
        glColor4f( 0.15f, 1, 0.15f, 0.2f * c );
      } else {
        glColor4f( di.red, di.green, di.blue, 0.2f * c );
      }

      drawParticle(&(particle[i]));
    }
  }
}

void Effect::drawExplosion(bool proceed) {
  // manage particles
  for(int i = 0; i < PARTICLE_COUNT; i++) {
    if(!particle[i].active) {
      // create a new particle
      particle[i].reset();
      particle[i].x = Util::roll( 0.0f, shape->getWidth() / DIV );
      particle[i].y = Util::roll( 0.0f, shape->getDepth() / DIV );      
		  particle[i].z = Util::pickOne( 3, 4 );
		  //	  particle[i].moveDelta = 0.15f + (0.15f * rand()/RAND_MAX);
		  particle[i].moveDelta = 0;
		  particle[i].rotate = Util::roll( 0.0f, 180.0f );
		  particle[i].maxLife = 5000;
		  particle[i].trail = 4;
    } else if(proceed) {
		  particle[i].rotate = Util::roll( 0.0f, 360.0f );
	
		  // this causes an explosion!
		  if(particle[i].zoom < 4.0f) particle[i].zoom += 0.5f;
	  	particle[i].move();
    }

    // draw it
    if(particle[i].active) {

	  float c = fabs(particle[i].z - 8) / 8.0f;
	  if(c > 1) c = 1;
      glColor4f(c, c / 2.0f, c / 2.0f, 0.5);

      drawParticle(&(particle[i]));
    }
  }
}

void Effect::drawBlast(bool proceed, float percent ) {

  // manage particles
  for(int i = 0; i < 15; i++) {
    if(!particle[i].active) {
      particle[i].reset();
      particle[i].x = Util::roll( 0.0f, shape->getWidth() / DIV );
      particle[i].y = Util::roll( 0.0f, shape->getDepth() / DIV );      
      particle[i].z = static_cast<int>(Util::roll( 0.5f * percent, 2.5f * percent ));
      particle[i].moveDelta = Util::roll( 0.05f, 0.1f );
      particle[i].maxLife = static_cast<int>(Util::roll( 5.0f * percent, 15.0f * percent ));
      particle[i].zoom = 3;
      particle[i].tail = true;
      particle[i].trail = 2;
    } else if(proceed) {
      particle[i].move();
    }
    
    // draw it      
    if(particle[i].active) {            
      
      float p = particle[i].life / ( particle[i].maxLife / 100.0f );
      float a = ( 0.5f / 100.0f ) * ( 100.0f - p );
      float gg = Util::roll( 0.8f, 1.0f );

      particle[i].tailColor.r = gg / ( particle[i].life );
      particle[i].tailColor.g = gg / ( particle[i].life / 10.0f );
      particle[i].tailColor.b = gg;
      particle[i].tailColor.a = a;

      glColor4f( particle[i].tailColor.r, 
                 particle[i].tailColor.g, 
                 particle[i].tailColor.b, 
                 particle[i].tailColor.a );
      
      drawParticle(&(particle[i]));
    }
  }
}

void Effect::drawDust(bool proceed) {
  // manage particles
  for(int i = 0; i < PARTICLE_COUNT; i++) {
    if(!particle[i].active) {
      // create a new particle
      particle[i].reset();
      particle[i].x = Util::roll( 0.0f, shape->getWidth() / DIV );
      particle[i].y = Util::roll( 0.0f, shape->getDepth() / DIV );      
		  particle[i].z = static_cast<int>(Util::dice( 2 ));
		  //	  particle[i].moveDelta = 0.15f + (0.15f * rand()/RAND_MAX);
		  particle[i].moveDelta = 0;
		  particle[i].rotate = Util::roll( 0.0f, 180.0f );
		  particle[i].maxLife = 5000;
		  particle[i].trail = 4;
    } else if(proceed) {
		  particle[i].rotate = Util::roll( 0.0f, 360.0f );
	
		  // this causes an explosion!
		  if(particle[i].zoom < 4.0f) particle[i].zoom += 0.5f;
	  	particle[i].move();
    }

    // draw it
    if(particle[i].active) {

	  float c = fabs(particle[i].z - 8) / 8.0f;
	  if(c > 1.0f) c = 1.0f;
      glColor4f(c / 4.0f, c / 4.0f, c / 4.0f, 0.35f);

      drawParticle(&(particle[i]));
    }
  }
}

void Effect::drawHail(bool proceed) {
  // manage particles
  for(int i = 0; i < PARTICLE_COUNT / 4; i++) {
    if(!particle[i].active) {
      // create a new particle
      particle[i].reset();
      particle[i].x = Util::roll( 0.0f, shape->getWidth() / DIV );
      particle[i].y = Util::roll( 0.0f, shape->getDepth() / DIV );      
      particle[i].z = Util::pickOne( 5, 6 );
      particle[i].moveDelta = Util::roll( 0.5f, 0.8f );
      particle[i].moveDelta *= -1.0f;
      particle[i].maxLife = 40000;
      particle[i].trail = 2;
      particle[i].zoom = 4.0f;
      particle[i].tail = true;
      particle[i].untilGround = true;
    } else if(proceed) {
      particle[i].move();
    }

    // draw it
    if(particle[i].active) {

	  float c = fabs(particle[i].z - 8) / 8.0f;
	  if(c > 1) c = 1;
    glColor4f( 0, c / 4.0f, 1.0f, 0.75 );
    
    particle[i].tailColor.r = 0.15f;
    particle[i].tailColor.g = c / 3.0f;
    particle[i].tailColor.b = 0.85f;
    particle[i].tailColor.a = 0.5f;

	  drawParticle(&(particle[i]));
    }
  }
}

void Effect::drawTower(bool proceed) {
  // manage particles
  for(int i = 0; i < PARTICLE_COUNT; i++) {
    if(!particle[i].active) {
      // create a new particle
      particle[i].reset();
      particle[i].x = Util::roll( 0.0f, shape->getWidth() / DIV );
      particle[i].y = Util::roll( 0.0f, shape->getDepth() / DIV );      
      if( (i % 3) ) {      	
        particle[i].z = Util::roll( 0.0f, 1.0f );
        //	  particle[i].moveDelta = 0.15f + (0.15f * rand()/RAND_MAX);
        particle[i].moveDelta = 0.15f;
        particle[i].rotate = Util::roll( 0.0f, 180.0f );
        particle[i].maxLife = 5000;
        particle[i].trail = 2;
        particle[i].zoom = 1.5f;
      } else {
        particle[i].z = Util::dice( 2 );
        particle[i].moveDelta = Util::roll( 0.5f, 0.8f );
        particle[i].maxLife = 40000;
        particle[i].trail = 8;
        particle[i].zoom = 4.0f;
        particle[i].tail = true;
      }
    } else if(proceed) {
      if( ( i % 3 ) ) {
        particle[i].rotate -= Util::roll( 3.0f, 6.0f );
      }
      particle[i].move();
    }

    // draw it
    if(particle[i].active) {

      float c = fabs(particle[i].z - 8) / 8.0f;
      if(c > 1) c = 1;
      glColor4f( 1.0f, c / 4.0f, 0, 0.75 );

      particle[i].tailColor.r = 0.85f;
      particle[i].tailColor.g = c / 3.0f;
      particle[i].tailColor.b = 0.15f;    
      particle[i].tailColor.a = 0.25f;

      drawParticle(&(particle[i]));
    }
  }
}

void Effect::drawSwirl(bool proceed) {
  // manage particles
  for(int i = 0; i < PARTICLE_COUNT; i++) {
	float angle = static_cast<float>(i) * (360.0f / static_cast<float>(PARTICLE_COUNT));
    if(!particle[i].active) {
      // create a new particle
      particle[i].reset();
		  particle[i].x = ((static_cast<float>(shape->getWidth()) / 2.0f) / DIV) +
			((static_cast<float>(shape->getWidth()) / 2.0f) / DIV) * cos(angle);
		  particle[i].y = ((static_cast<float>(shape->getDepth()) / 2.0f) / DIV) +
			((static_cast<float>(shape->getDepth()) / 2.0f) / DIV) * sin(angle);
		  particle[i].z = 1;
		  particle[i].moveDelta = 0.15f;
		  particle[i].rotate = angle;
		  particle[i].maxLife = 5000;
		  //particle[i].trail = 2;
    } else if(proceed) {
		  particle[i].zoom += 0.01f;
		  particle[i].rotate += 5.0f;
		  particle[i].x = ((static_cast<float>(shape->getWidth()) / 2.0f) / DIV) + 
			((static_cast<float>(shape->getWidth()) / 2.0f) / DIV) * cos(particle[i].rotate);
		  particle[i].y = ((static_cast<float>(shape->getDepth()) / 2.0f) / DIV) +
			((static_cast<float>(shape->getDepth()) / 2.0f) / DIV) * sin(particle[i].rotate);
		  particle[i].move();
    }

    // draw it
    if(particle[i].active) {
		  float c = fabs(particle[i].z - 8) / 8.0f;
		  if(c > 1) c = 1;
	    glColor4f(c / 2.0f, c / 4.0f, c, 0.5);	
		  drawParticle(&(particle[i]));
    }
  }
}

void Effect::drawCastSpell(bool proceed) {
  // manage particles
  for(int i = 0; i < PARTICLE_COUNT; i++) {
    if(!particle[i].active) {
      // create a new particle
      particle[i].reset();
      particle[i].x = Util::roll( 0.0f, shape->getWidth() / DIV );
      particle[i].y = Util::roll( 0.0f, shape->getDepth() / DIV );      
		  particle[i].z = Util::pickOne( 3, 4 );
		  //	  particle[i].moveDelta = 0.15f + (0.2f * rand()/RAND_MAX);
		  particle[i].moveDelta = 0;
		  particle[i].maxLife = 10000;
		  particle[i].trail = 1;
		  particle[i].zoom = 0.5f;
    } else if(proceed) {
    	particle[i].move();
    }

    // draw it
    if(particle[i].active) {
		  float c = fabs(particle[i].z - 8) / 8.0f;
		  if(c > 1) c = 1;

		  //	  particle[i].rotate += 5.0f;
		  particle[i].zoom = ((particle[i].life % 200) / 60.0f + 1) * 1.5f;

      glColor4f(c / 2.0f, c / 4.0f, 1.0f, 0.25);

      drawParticle(&(particle[i]));
    }
  }
}

void Effect::drawRing(bool proceed) {

  float r = ringRadius / DIV;

  float n = Util::roll( 1.0f, 1.05f );
  for(int i = 0; i < 2; i++) {
    glPushMatrix();
    glRotatef((i == 0 ? ringRotate : -ringRotate), 0, 0, 1);
    glColor4f( Util::roll( 0.85f, 1.0f ), 
              Util::roll( 0.85f, 1.0f ), 
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

void Effect::drawSmoke(bool proceed) {
  // manage particles
  for(int i = 0; i < PARTICLE_COUNT; i++) {
    if(!particle[i].active) {
      // create a new particle
      particle[i].reset();
      particle[i].x = Util::roll( 0.0f, shape->getWidth() / DIV );
      particle[i].y = Util::roll( 0.0f, shape->getDepth() / DIV );      
      particle[i].z = Util::roll( 0.0f, 1.0f );      
      particle[i].moveDelta = Util::roll( 0.1f, 0.3f );      
      //particle[i].moveDelta = 0.15f + (0.15f * rand()/RAND_MAX);
      // particle[i].moveDelta = 0.15f;
      particle[i].rotate = Util::roll( 0.0f, 180.0f );
      particle[i].maxLife = Util::pickOne( 50, 70 );
      particle[i].trail = 2;
      particle[i].zoom = Util::roll( 1.0f, 2.0f );
    } else if(proceed) {
      particle[i].rotate -= Util::roll( 3.0f, 6.0f );

      // this causes an explosion!
      //particle[i].zoom += 0.3f;
      particle[i].move();
    }

    // draw it      
    if(particle[i].active) {            

      float max = particle[i].maxLife / 4;
      float c = 1.0f;
      if( particle[i].life <= max / 2 ) {
        c = static_cast<float>( particle[i].life ) / ( max / 2 );
      } else if( particle[i].life > particle[i].maxLife - max ) {
        c = static_cast<float>( particle[i].maxLife - particle[i].life ) / max;
      }
      float r = ( diWasSet ? di.red : 0.2f );
      float g = ( diWasSet ? di.green : 0.2f );
      float b = ( diWasSet ? di.blue : 0.5f );
      glColor4f( r, g, b, 0.2f * c );
      if( particle[i].zoom < 5 ) {
      	particle[i].zoom *= 1.05f;
      }      

      drawParticle(&(particle[i]));
    }
  }
}

void Effect::drawFire(bool proceed) {
	for( int i = 0; i < PARTICLE_COUNT; i++ ) {
		if(!particle[i].active) {
      // create a new particle
			particle[i].reset();
      particle[i].x = Util::roll( 0.0f, shape->getWidth() / DIV );
      particle[i].y = Util::roll( 0.0f, shape->getDepth() / DIV );			
      particle[i].z = Util::roll( 0.0f, 0.5f );
      particle[i].maxLife = Util::roll( 4.0f, 10.0f );
      particle[i].moveDelta = Util::roll( 0.1f, 0.4f );
      particle[i].zoom = Util::roll( 1.8f, 2.5f );
      particle[i].trail = 2;
    } else if(proceed) {
      // move this particle
    	particle[i].move();
    }

    // draw it
    if(particle[i].active) {
				float max = particle[i].maxLife / 5;
	      float c = 1.0f;
	      if( particle[i].life <= max ) {
	        c = static_cast<float>( particle[i].life ) / max;
	      } else if( particle[i].life > particle[i].maxLife - max ) {
	        c = static_cast<float>( particle[i].maxLife - particle[i].life ) / max;
	      }
	      
	      if( particle[i].zoom < 3 ) {
        	particle[i].zoom *= 1.05f;
        }	      
    	
        float color = 1.0f / ((GLfloat)particle[i].maxLife / (GLfloat)particle[i].life);
        float red = Util::roll( 0.0f, (1.0f - color) / 4.0 );
        float green = Util::roll( 0.0f, (1.0f - color) / 8.0 );
        float blue = Util::roll( 0.0f, (1.0f - color) / 10.0 );
        glColor4f(color + red, color + green, color + blue, c);
        drawParticle(&(particle[i]));
    }
	}
}

Particle::Particle() {
	reset();
}

void Particle::reset() {
	active = true;
  x = y = 0;
  //  z = static_cast<int>(6.0 * rand()/RAND_MAX) + 10;
  z = Util::roll( 0.0f, 0.8f );
  startZ = z;
  height = Util::pickOne( 10, 24 );
  life = 0;
  moveDelta = Util::roll( 0.2f, 0.5f );
  maxLife = 10;
  trail = 1;
  rotate = 0.0f;
  zoom = 1.0f;
  tail = false;
  untilGround = false;
}

Particle::~Particle() {
}

void Particle::move() {
  // move this particle
  life++;
  z+=moveDelta;
  if(z < 0 || z > MAP_VIEW_HEIGHT || 
     (!(untilGround) && life >= maxLife)) {
  	active = false;
  }
}

void Effect::drawParticle(Particle *particle) {
  float w, h, sh;

  w = static_cast<float>(shape->getWidth() / DIV) / 4.0f;
  h = static_cast<float>(shape->getHeight() / DIV) / 3.0f;
  if(h == 0) h = 0.25f / DIV;
  sh = ( fabs( particle->z - particle->startZ ) / DIV) / 3.0f;
  if(h == 0) h = 0.25f / DIV;

  glDisable( GL_CULL_FACE );
  glEnable( GL_TEXTURE_2D );
  for(int i = 0; i < particle->trail; i++) {
    glPushMatrix();
    
    // position the particle
    GLfloat z = (particle->z + i) / DIV;
    glTranslatef( particle->x, particle->y, z );  
    
    // rotate each particle to face viewer
    glRotatef(-levelMap->getZRot(), 0.0f, 0.0f, 1.0f);
    glRotatef(-levelMap->getYRot(), 1.0f, 0.0f, 0.0f);
    
    if(flameTex) glBindTexture( GL_TEXTURE_2D, flameTex );
    
    // rotate particles
    glRotatef( particle->rotate, 0, 0, 1 );
    
    // zoom
    glScalef(particle->zoom, particle->zoom, particle->zoom);
    
    if( particle->tail ) {
      glColor4f( particle->tailColor.r, particle->tailColor.g, particle->tailColor.b, particle->tailColor.a );
      glBegin( GL_QUADS );
      //glNormal3f(0.0f, 1.0f, 0.0f);
      glNormal3f( 0, 0, 1 );
      if(flameTex) glTexCoord2f( 1.0f, 1.0f );
      glVertex2f(w/2.0f, -sh );
      if(flameTex) glTexCoord2f( 0.0f, 1.0f );
      glVertex2f(-w/2.0f, -sh );
      if(flameTex) glTexCoord2f( 0.0f, 0.0f );
      glVertex2f(-w/2.0f, 0 );
      if(flameTex) glTexCoord2f( 1.0f, 0.0f );
      glVertex2f(w/2.0f, 0 );  
      glEnd();    
    }
    glBegin( GL_QUADS );
    //glNormal3f(0.0f, 1.0f, 0.0f);
    glNormal3f( 0, 0, 1 );
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
