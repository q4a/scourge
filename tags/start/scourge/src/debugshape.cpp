/***************************************************************************
                          debugshape.cpp  -  description
                             -------------------
    begin                : Tue Sep 23 2003
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

#include "debugshape.h"

DebugShape::DebugShape(GLuint texture[], 
          int width, int depth, int height,
          char *name,
          Uint32 color, GLuint display_list, Uint8 shapePalIndex) :
  GLShape(texture, width, depth, height, name, color, display_list, shapePalIndex) {
  zz = 0.0f;
  yy = 0.0f;
  xx = 0.0f;
  inner = new GLShape(texture, width, depth, height, name, NULL, 0,
                      color, display_list, shapePalIndex);  
}

DebugShape::DebugShape(GLuint texture[], 
          int width, int depth, int height,
          char *name, char **description, int descriptionCount,
          Uint32 color, GLuint display_list, Uint8 shapePalIndex) :
  GLShape(texture, width, depth, height, name, description, descriptionCount, color, display_list, shapePalIndex) {
  zz = 0.0f;
  yy = 0.0f;
  xx = 0.0f;
  inner = new GLShape(texture, width, depth, height, name, description, descriptionCount,
                      color, display_list, shapePalIndex);    
}

DebugShape::~DebugShape(){
}

void DebugShape::draw() {
//  glDisable(GL_DEPTH_TEST);
//	glBlendFunc(GL_SRC_ALPHA, GL_ONE);
//  glEnable(GL_BLEND);

  // save the model_view matrix
  glPushMatrix();

  glRotatef(-zrot, 0.0f, 0.0f, 1.0f);
  glRotatef(-(90.0 + yrot), 1.0f, 0.0f, 0.0f);

//  glRotatef(xx, 0.0f, 1.0f, 0.0f);  
//  glRotatef(yy, 1.0f, 0.0f, 0.0f);  
//  glRotatef(zz, 0.0f, 0.0f, 1.0f);
    
  inner->draw();

  // reset the model_view matrix
  glPopMatrix();

//  glDisable(GL_BLEND);
//  glEnable(GL_DEPTH_TEST);

//  zz += 0.7; if(zz >= 360.0) zz -= 360.0;
//  yy += 0.7; if(yy >= 360.0) yy -= 360.0;
//  xx += 0.7; if(xx >= 360.0) xx -= 360.0;    
}
