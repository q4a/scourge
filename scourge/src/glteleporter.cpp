/***************************************************************************
                          glteleporter.cpp  -  description
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

#include "glteleporter.h"

/**
  *@author Gabor Torok
  */

GLTeleporter::GLTeleporter(GLuint texture[], GLuint flameTex,
						   int width, int depth, int height,
						   char *name,
						   Uint32 color, GLuint display_list, Uint8 shapePalIndex) : 
  GLShape(texture, width, depth, height, name, color, 
		  display_list, shapePalIndex) {
  commonInit(flameTex);
}

GLTeleporter::GLTeleporter(GLuint texture[], GLuint flameTex,
						   int width, int depth, int height,
						   char *name, char **description, int descriptionCount,
						   Uint32 color, GLuint display_list, Uint8 shapePalIndex) :
  GLShape(texture, width, depth, height, name, description, 
		  descriptionCount, color, display_list, shapePalIndex) {
  commonInit(flameTex);
}

void GLTeleporter::commonInit(GLuint flameTex) {
  this->flameTex = flameTex;
  for(int i = 0; i < MAX_RINGS; i++) {
	ring[i] = 0;
  }
  for(int i = 0; i < MAX_STARS; i++) {
	star[i][0] = star[i][1] = -1;
  }
}

GLTeleporter::~GLTeleporter() {
}

void GLTeleporter::draw() {

  float r = ((float)width / DIV) / 2.0f;
  for(int i = 0; i < MAX_STARS; i++) {
	// reposition
	if(star[i][0] == -1) {
	  starAngle[i] = (360.0f * rand()/RAND_MAX);
	}
	star[i][0] = r + (r * cos(3.14159 / (180.0f / starAngle[i])));
	star[i][1] = r + (r * sin(3.14159 / (180.0f / starAngle[i])));

	// draw
	glDisable( GL_CULL_FACE );
	glPushMatrix();
	
	
	float w = ((float)width / DIV) / 10.0f;
	float d = ((float)depth / DIV) / 10.0f;
	float h = 1.25f / DIV;
		
	if(flameTex) glBindTexture( GL_TEXTURE_2D, flameTex );
	
	//	float red = (float)((this->color & 0xff000000) >> (3 * 8)) / (float)(0xff);
	//	float green = (float)((this->color & 0x00ff0000) >> (2 * 8)) / (float)(0xff);
	//	float blue = (float)((this->color & 0x0000ff00) >> (1 * 8)) / (float)(0xff);
	//	float max = ((float)(height - 1) / DIV) / 2.0f;
	//	float dist = 1.5f - abs(max - ring[i]) / max;
	glColor4f(1, 1, 1, 1);
	
	glBegin( GL_QUADS );
	// front
	glNormal3f(0.0f, 0.0f, 1.0f);
	if(flameTex) glTexCoord2f( 0.0f, 0.0f );
	glVertex3f(star[i][0], star[i][1], h);
	if(flameTex) glTexCoord2f( 1.0f, 0.0f );
	glVertex3f(star[i][0] + w, star[i][1], h);
	if(flameTex) glTexCoord2f( 1.0f, 1.0f );
	glVertex3f(star[i][0] + w, star[i][1] + d, h);
	if(flameTex) glTexCoord2f( 0.0f, 1.0f );
	glVertex3f(star[i][0], star[i][1] + d, h);
	glEnd();
	
	glPopMatrix();
	glEnable( GL_CULL_FACE );
	
	// move
	starAngle[i]++; 
	if(starAngle[i] >= 360.0f) starAngle[i] -= 360.0f;
  }

  for(int i = 0; i < MAX_RINGS; i++) {
	// reposisition
	if(ring[i] <= (1.0f / DIV) || ring[i] >= ((float)height / DIV)) {
	  //	  ring[i] = (((float)(height - 1) / DIV) * rand()/RAND_MAX) + (1.0f / DIV);
	  //	  delta[i] = (ring[i] >= ((float)(height - 1) / DIV) / 2.0f ? 0.5f : -0.5f);
	  ring[i] = ((float)(height - 1) / DIV) / 2.0f + ((20.0f * rand()/RAND_MAX) - 10.0f);
	  delta[i] = ((int)(2.0f * rand()/RAND_MAX) == 0 ? 3.0f : -3.0f);
	}
	
	// draw 
	glDisable( GL_CULL_FACE );
	glPushMatrix();
	
	float w = ((float)width / DIV);
	float d = ((float)depth / DIV);
	float h = ring[i];
	//      if(h == 0) h = 0.25 / DIV;
		
	if(flameTex) glBindTexture( GL_TEXTURE_2D, flameTex );
	
	float red = (float)((this->color & 0xff000000) >> (3 * 8)) / (float)(0xff);
	float green = (float)((this->color & 0x00ff0000) >> (2 * 8)) / (float)(0xff);
	float blue = (float)((this->color & 0x0000ff00) >> (1 * 8)) / (float)(0xff);
	float max = ((float)(height - 1) / DIV) / 2.0f;
	float dist = 1.5f - abs(max - ring[i]) / max;
	glColor4f(red, green, blue, dist);
	
	glBegin( GL_QUADS );
	// front
	glNormal3f(0.0f, 0.0f, 1.0f);
	if(flameTex) glTexCoord2f( 0.0f, 0.0f );
	glVertex3f(0, 0, h);
	if(flameTex) glTexCoord2f( 1.0f, 0.0f );
	glVertex3f(w, 0, h);
	if(flameTex) glTexCoord2f( 1.0f, 1.0f );
	glVertex3f(w, d, h);
	if(flameTex) glTexCoord2f( 0.0f, 1.0f );
	glVertex3f(0, d, h);
	glEnd();
	
	glPopMatrix();
	glEnable( GL_CULL_FACE );
	
	// move
	ring[i] += delta[i];
  }
  
}
