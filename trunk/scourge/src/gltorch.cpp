/***************************************************************************
                          gltorch.cpp  -  description
                             -------------------
    begin                : Sat Sep 20 2003
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

#include "gltorch.h"

GLTorch::GLTorch(GLuint texture[], GLuint flameTex,
				 int width, int depth, int height,
				 char *name,
				 Uint32 color, GLuint display_list, Uint8 shapePalIndex, 
				 GLuint torchback, int torch_dir) :
  GLShape(texture, width, depth, height, name, color, display_list, shapePalIndex) {
  this->flameTex = flameTex;
  this->torchback = torchback;
  this->torch_dir = torch_dir;
  initParticles();                                
}

GLTorch::GLTorch(GLuint texture[], GLuint flameTex,
				 int width, int depth, int height,
				 char *name, char **description, int descriptionCount,
				 Uint32 color, GLuint display_list, Uint8 shapePalIndex, 
				 GLuint torchback, int torch_dir) :
  GLShape(texture, width, depth, height, name, description, descriptionCount, color, display_list, shapePalIndex) {
  this->flameTex = flameTex;
  this->torchback = torchback;
  this->torch_dir = torch_dir;
  initParticles();                
}

GLTorch::~GLTorch() {
  for(int i = 0; i < PARTICLE_COUNT; i++) {
    if(particle[i]) {
      delete(particle[i]);
      particle[i] = 0;
    }
  }
}

void GLTorch::initParticles() {
  for(int i = 0; i < PARTICLE_COUNT; i++) {
    particle[i] = NULL;
  }
}

void GLTorch::draw() {
  float w, d, h;

  // manage particles
  for(int i = 0; i < PARTICLE_COUNT; i++) {
    if(!particle[i]) {
      // create a new particle
      particle[i] = new ParticleStruct();
      particle[i]->x = ((float)(width / DIV) * rand()/RAND_MAX);
      particle[i]->y = ((float)(depth / DIV) * rand()/RAND_MAX);
      particle[i]->z = 0.0f;
      particle[i]->height = (int)(25.0 * rand()/RAND_MAX) + 10;
    } else {
      // move this particle
      particle[i]->z+=0.5f;
      if(particle[i]->z >= particle[i]->height) {
        delete(particle[i]);
        particle[i] = 0;
      }
    }

    // draw it      
    if(particle[i]) {            

      // save the model_view matrix
      glPushMatrix();

      w = (float)(width / DIV) / 2.0;
      //float d = (float)(depth / DIV) / 2.0;
      h = (float)(height / DIV) / 2.5;
      if(h == 0) h = 0.25 / DIV;

      // position the particle
      GLfloat z = (float)(particle[i]->z * h) / 10.0;
      glTranslatef( particle[i]->x, particle[i]->y, z );

      // rotate each particle to face viewer
      glRotatef(-zrot, 0.0f, 0.0f, 1.0f);
      glRotatef(-(90.0 + yrot), 1.0f, 0.0f, 0.0f);      

      if(flameTex) glBindTexture( GL_TEXTURE_2D, flameTex );

      float color = 1.0f / ((GLfloat)particle[i]->height / (GLfloat)particle[i]->z);
      float red = (((1.0f - color) / 4.0) * rand()/RAND_MAX);
      float green = (((1.0f - color) / 8.0) * rand()/RAND_MAX);
      float blue = (((1.0f - color) / 10.0) * rand()/RAND_MAX);      
      glColor4f(color + red, color + green, color + blue, 1.0f);      
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

  // add the flickering reflection on the wall behind
  // max. amount of movement
  float mm = 0.4f / DIV;
  // max. amount of color component variation
  float mc = 0.25f;
  float red = 1.0f + (mc * rand()/RAND_MAX) - (mc * 2.0f);
  if(red > 1.0f) red = 1.0f;
  float green = 1.0f + (mc * rand()/RAND_MAX) - (mc * 2.0f);
  if(green > 1.0f) red = 1.0f;

  float size = 5.0f/DIV;

  glPushMatrix();
  if(torch_dir == Constants::NORTH) {
	glTranslatef( -size/2.0f + (1.0f / DIV) + (mm * rand()/RAND_MAX) - (mm * 2.0f), 
				  0.0f, 
				  -size/2.0f + (1.0f / DIV) + (mm * rand()/RAND_MAX) - (mm * 2.0f) );
  } else if(torch_dir == Constants::WEST) {
	glTranslatef( 0.0f,
				  -size/2.0f + (1.0f / DIV) + (mm * rand()/RAND_MAX) - (mm * 2.0f), 
				  -size/2.0f + (1.0f / DIV) + (mm * rand()/RAND_MAX) - (mm * 2.0f) );
  } else if(torch_dir == Constants::EAST) {
	glTranslatef( 1.0f/DIV,
				  -size/2.0f + (1.0f / DIV) + (mm * rand()/RAND_MAX) - (mm * 2.0f), 
				  -size/2.0f + (1.0f / DIV) + (mm * rand()/RAND_MAX) - (mm * 2.0f) );
  }
  glBindTexture( GL_TEXTURE_2D, torchback );
  glColor4f( red, green, 0.3f, 0.4 );
  glBegin( GL_QUADS );
  if(torch_dir == Constants::NORTH) {
	w = size;
	d = 0;
	h = size;
	glNormal3f(0.0f, 1.0f, 0.0f);
	glTexCoord2f( 1.0f, 1.0f );
	glVertex3f(w, d, 0);
	glTexCoord2f( 1.0f, 0.0f );
	glVertex3f(w, d, h);  
	glTexCoord2f( 0.0f, 0.0f );
	glVertex3f(0, d, h);
	glTexCoord2f( 0.0f, 1.0f );
	glVertex3f(0, d, 0);
  } else if(torch_dir == Constants::WEST) {
	w = 0;
	d = size;
	h = size;
	glNormal3f(-1.0f, 0.0f, 0.0f);
	glTexCoord2f( 1.0f, 1.0f );
	glVertex3f(w, d, 0);
	glTexCoord2f( 0.0f, 1.0f );
	glVertex3f(w, 0, 0);
	glTexCoord2f( 0.0f, 0.0f );
	glVertex3f(w, 0, h);
	glTexCoord2f( 1.0f, 0.0f );
	glVertex3f(w, d, h);  
  } else if(torch_dir == Constants::EAST) {
	w = 0;
	d = size;
	h = size;
	glNormal3f(-1.0f, 0.0f, 0.0f);
	glTexCoord2f( 1.0f, 1.0f );
	glVertex3f(w, d, 0);
	glTexCoord2f( 1.0f, 0.0f );
	glVertex3f(w, d, h);  
	glTexCoord2f( 0.0f, 0.0f );
	glVertex3f(w, 0, h);
	glTexCoord2f( 0.0f, 1.0f );
	glVertex3f(w, 0, 0);
  }
  glEnd();
  glPopMatrix();
}

