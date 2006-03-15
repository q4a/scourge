/***************************************************************************
                          glshape.cpp  -  description
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

#include "glshape.h"
#include "map.h"

class Map;

GLShape::GLShape(GLuint tex[],
                 int width, int depth, int height,
                 char *name,
                 Uint32 color, GLuint display_list,
                 Uint8 shapePalIndex) :
    Shape(width, depth, height, name, NULL, 0) {
  commonInit(tex, color, display_list, shapePalIndex);
}

GLShape::GLShape(GLuint tex[],
                 int width, int depth, int height,
                 char *name, char **description, int descriptionCount,
                 Uint32 color,
                 GLuint display_list,
                 Uint8 shapePalIndex) :
    Shape(width, depth, height, name, description, descriptionCount) {
  commonInit(tex, color, display_list, shapePalIndex);
}

void GLShape::commonInit(GLuint tex[], Uint32 color, GLuint display_list, Uint8 shapePalIndex) {
  this->tex = tex;
  this->color = color;
  this->display_list = display_list;
  this->shapePalIndex = shapePalIndex; 
  this->skipside = 0;
}

GLShape::~GLShape(){
}

void GLShape::draw() {

    glEnable( GL_CULL_FACE );
    glCullFace( GL_BACK );

    float w = (float)width / DIV;
    float d = (float)depth / DIV;
    float h = (float)height / DIV;
    if (h == 0) h = 0.25 / DIV;

    float red = 0.72f;
    float green = 0.65f;
    float blue = 0.55f;
    float alpha = 0.5f;

    glColor4f(red, green, blue, alpha);
    if(!(skipside & ( 1 << GLShape::LEFT_RIGHT_SIDE ))) {    
        if (tex[LEFT_RIGHT_SIDE]) glBindTexture( GL_TEXTURE_2D, tex[LEFT_RIGHT_SIDE] );
        glBegin( GL_QUADS );

        // left    
        glNormal3f(-1.0f, 0.0f, 0.0f);
        //glColor4ub((color >> 24) & 0xff, (color >> 16) & 0xff, (color >> 8) & 0xff, color & 0xff);
        glTexCoord2f( 1.0f, 0.0f );
        glVertex3f(0, 0, h);
        glTexCoord2f( 1.0f, 1.0f );
        glVertex3f(0, 0, 0);
        glTexCoord2f( 0.0f, 1.0f );
        glVertex3f(0, d, 0);
        glTexCoord2f( 0.0f, 0.0f );
        glVertex3f(0, d, h);
        glEnd();
    }

    if(!(skipside & (1 << GLShape::FRONT_SIDE))) {    
        if (tex[FRONT_SIDE]) glBindTexture( GL_TEXTURE_2D, tex[FRONT_SIDE] );
        glBegin( GL_QUADS );
        // bottom
        //glColor3f(1.0, 0.0, 0.0);
        glNormal3f(0.0f, -1.0f, 0.0f);
        //glColor4ub((color >> 24) & 0xff, (color >> 16) & 0xff, (color >> 8) & 0xff, color & 0xff);
        glTexCoord2f( 1.0f, 1.0f );
        glVertex3f(w, 0, 0);
        glTexCoord2f( 0.0f, 1.0f );
        glVertex3f(0, 0, 0);
        glTexCoord2f( 0.0f, 0.0f );
        glVertex3f(0, 0, h);
        glTexCoord2f( 1.0f, 0.0f );
        glVertex3f(w, 0, h);
        glEnd();
    }

    if(!(skipside & ( 1 << GLShape::LEFT_RIGHT_SIDE ))) {    
        if (tex[LEFT_RIGHT_SIDE]) glBindTexture( GL_TEXTURE_2D, tex[LEFT_RIGHT_SIDE] );
        glBegin(GL_QUADS);
        // right
        glNormal3f(1.0f, 0.0f, 0.0f);
        //glColor4ub((color >> 24) & 0xff, (color >> 16) & 0xff, (color >> 8) & 0xff, color & 0xff);
        glTexCoord2f( 0.0f, 0.0f );
        glVertex3f(w, d, h);
        glTexCoord2f( 0.0f, 1.0f );
        glVertex3f(w, d, 0);
        glTexCoord2f( 1.0f, 1.0f );
        glVertex3f(w, 0, 0);
        glTexCoord2f( 1.0f, 0.0f );
        glVertex3f(w, 0, h);
        glEnd( );
    }

    if(!(skipside & (1 << GLShape::FRONT_SIDE))) {    
        if (tex[FRONT_SIDE]) glBindTexture( GL_TEXTURE_2D, tex[FRONT_SIDE] );
        glBegin( GL_QUADS );
        // front
        glNormal3f(0.0f, 1.0f, 0.0f);
        //glColor4ub((color >> 24) & 0xff, (color >> 16) & 0xff, (color >> 8) & 0xff, color & 0xff);
        glTexCoord2f( 1.0f, 1.0f );
        glVertex3f(w, d, 0);
        glTexCoord2f( 1.0f, 0.0f );
        glVertex3f(w, d, h);        
        glTexCoord2f( 0.0f, 0.0f );
        glVertex3f(0, d, h);
        glTexCoord2f( 0.0f, 1.0f );
        glVertex3f(0, d, 0);
        glEnd();
    }

    GLboolean blending = glIsEnabled(GL_BLEND);
    if(blending) {
        glDisable(GL_BLEND);
        glDepthMask(GL_TRUE);
        glEnable(GL_LIGHTING);
    }
    if (tex[TOP_SIDE]) glBindTexture( GL_TEXTURE_2D, tex[TOP_SIDE] );
    glBegin( GL_QUADS );
    // top
    glNormal3f(0.0f, 0.0f, 1.0f);
    //glColor4ub((color >> 24) & 0xff, (color >> 16) & 0xff, (color >> 8) & 0xff, color & 0xff);
    glTexCoord2f( 1.0f, 1.0f );
    glVertex3f(w, d, h);
    glTexCoord2f( 1.0f, 0.0f );
    glVertex3f(w, 0, h);
    glTexCoord2f( 0.0f, 0.0f );
    glVertex3f(0, 0, h);
    glTexCoord2f( 0.0f, 1.0f );
    glVertex3f(0, d, h);
    glEnd();
    if(blending) {
        glEnable(GL_BLEND);
        glDepthMask(GL_FALSE);
        glDisable(GL_LIGHTING);
    }
}

void GLShape::setupBlending() { 
    glBlendFunc(GL_ONE, GL_ONE); 
    //glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA); 
    //Scourge::setBlendFunc();
}
