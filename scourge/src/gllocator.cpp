
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

#include "gllocator.h"

GLLocator::GLLocator(GLuint texture[], 
          int width, int depth, int height,
          char *name, int descriptionGroup,
          Uint32 color, GLuint display_list, Uint8 shapePalIndex) :
  GLShape(texture, width, depth, height, name, descriptionGroup, color, display_list, shapePalIndex) {
}

GLLocator::~GLLocator() {
}

void GLLocator::draw() {
    float w = (float)width / DIV;
    float d = (float)depth / DIV;
    float h = 0.26 / DIV;

    glDisable( GL_TEXTURE_2D );
    glBlendFunc(GL_SRC_ALPHA, GL_ONE);
    glEnable(GL_BLEND);
    glBegin( GL_QUADS );
      //glNormal3f(0.0f, 0.0f, 1.0f);
      glColor4f(0.2, 0.5, 0.1, 0.7);
      glVertex3f(w, d, h);
      glVertex3f(w, 0, h);
      glVertex3f(0, 0, h);
      glVertex3f(0, d, h);
   glEnd();
   glDisable(GL_BLEND);

   glColor4f(1.0, 1.0, 0.1, 1.0);
    glBegin( GL_LINES );
      glVertex3f(w, d, h);
      glVertex3f(0, d, h);

      glVertex3f(0, d, h);
      glVertex3f(0, 0, h);

      glVertex3f(0, 0, h);
      glVertex3f(w, 0, h);

      glVertex3f(w, 0, h);
      glVertex3f(w, d, h);
   glEnd();


   glEnable( GL_TEXTURE_2D );
}

