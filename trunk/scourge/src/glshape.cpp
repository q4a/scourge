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

// poor man's dynamic lightmaps: shaded sides
static GLuint lightmap_tex_num = 0;
static GLuint lightmap_tex_num2 = 0;
static unsigned char data[LIGHTMAP_SIZE * LIGHTMAP_SIZE * 3];
static unsigned char data2[LIGHTMAP_SIZE * LIGHTMAP_SIZE * 3];

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
  this->useShadow = false;
  this->useTexture = true;

  // initialize the surfaces
  float w = (float)width / DIV;
  float d = (float)depth / DIV;
  float h = (float)height / DIV;
  if (h == 0) h = 0.25 / DIV;

  float v[4][3];

  /**
	 Vertices:
	 1 - top left (texture mapped)
	 2 - bottom left
	 3 - bottom right
	 4 - top right
   */

  v[0][0] = 0.0f; v[0][1] = d;    v[0][2] = 0.0f;
  v[1][0] = 0.0f; v[1][1] = d;    v[1][2] = h;
  v[2][0] = 0.0f; v[2][1] = 0.0f; v[2][2] = h;
  v[3][0] = 0.0f; v[3][1] = 0.0f; v[3][2] = 0.0f;
  surfaces[LEFT_SURFACE] = new_surface(v);

  v[0][0] = 0.0f; v[0][1] = 0.0f; v[0][2] = 0.0f;
  v[1][0] = 0.0f; v[1][1] = 0.0f; v[1][2] = h;
  v[2][0] = w;    v[2][1] = 0.0f; v[2][2] = h;
  v[3][0] = w;    v[3][1] = 0.0f; v[3][2] = 0.0f;
  surfaces[BOTTOM_SURFACE] = new_surface(v);

  v[0][0] = w;    v[0][1] = d;    v[0][2] = h;
  v[1][0] = w;    v[1][1] = d;    v[1][2] = 0.0f;
  v[2][0] = w;    v[2][1] = 0.0f; v[2][2] = 0.0f;
  v[3][0] = w;    v[3][1] = 0.0f; v[3][2] = h;
  surfaces[RIGHT_SURFACE] = new_surface(v);

  v[0][0] = w;    v[0][1] = d;    v[0][2] = 0.0f;
  v[1][0] = w;    v[1][1] = d;    v[1][2] = h;
  v[2][0] = 0.0f; v[2][1] = d;    v[2][2] = h;
  v[3][0] = 0.0f; v[3][1] = d;    v[3][2] = 0.0f;
  surfaces[FRONT_SURFACE] = new_surface(v);

  v[0][0] = w;    v[0][1] = d;    v[0][2] = h;
  v[1][0] = w;    v[1][1] = 0.0f; v[1][2] = h;
  v[2][0] = 0.0f; v[2][1] = 0.0f; v[2][2] = h;
  v[3][0] = 0.0f; v[3][1] = d;    v[3][2] = h;
  surfaces[TOP_SURFACE] = new_surface(v);

  if(lightmap_tex_num == 0 && Constants::multitexture) 
	createDarkTexture();
}

struct surface *GLShape::new_surface(float vertices[4][3]) {
  int i, j;
  struct surface *surf;

  surf = (struct surface *)malloc(sizeof(struct surface));
  if(!surf) {
	fprintf(stderr, "Error: Couldn't allocate memory for surface\n");
	return NULL;
  }

  for(i = 0; i < 4; i++) {
	for(j = 0; j < 3; j++)
	  surf->vertices[i][j] = vertices[i][j];
  }
  /*
  // x axis of matrix points in world space direction of the s texture axis 
  // top,right <- top,left = 3,0
  // added div by 10.0f, otherwise light is a pinpoint. Why?
  for(i = 0; i < 3; i++)
	surf->matrix[0 + i] = (surf->vertices[1][i] - surf->vertices[2][i]) / 10.0f;
  surf->s_dist = sqrt(Util::dot_product(surf->matrix, surf->matrix));
  Util::normalize(surf->matrix);
  
  // y axis of matrix points in world space direction of the t texture axis
  // bottom,left <- top,left = 3,0
  // added div by 10.0f, otherwise light is a pinpoint. Why?
  for(i = 0; i < 3; i++)
	surf->matrix[3 + i] = (surf->vertices[3][i] - surf->vertices[2][i]) / 10.0f;
  surf->t_dist = sqrt(Util::dot_product(surf->matrix + 3, surf->matrix + 3));
  Util::normalize(surf->matrix + 3);
  
  // z axis of matrix is the surface's normal
  Util::cross_product(surf->matrix, surf->matrix + 3, surf->matrix + 6);
  */
  return surf;
}


GLShape::~GLShape(){
}

void GLShape::draw() {

	// don't blend the top, but if in shadow mode, don't mess with blending
    GLboolean blending = (glIsEnabled(GL_BLEND) && !useShadow);

	// cull back faces
    glEnable( GL_CULL_FACE );
    glCullFace( GL_BACK );

    if(!(skipside & ( 1 << GLShape::LEFT_RIGHT_SIDE ))) {    
	  if (tex[LEFT_RIGHT_SIDE]) glBindTexture( GL_TEXTURE_2D, tex[LEFT_RIGHT_SIDE] );
	  glBegin( GL_QUADS );
	  // left    
	  glNormal3f(-1.0f, 0.0f, 0.0f);
	  glTexCoord2f( 0.0f, 1.0f );
	  glVertex3fv(surfaces[LEFT_SURFACE]->vertices[0]);
	  glTexCoord2f( 0.0f, 0.0f );
	  glVertex3fv(surfaces[LEFT_SURFACE]->vertices[1]);
	  glTexCoord2f( 1.0f, 0.0f );
	  glVertex3fv(surfaces[LEFT_SURFACE]->vertices[2]);
	  glTexCoord2f( 1.0f, 1.0f );
	  glVertex3fv(surfaces[LEFT_SURFACE]->vertices[3]);

	  glEnd();
    }

    if(!(skipside & (1 << GLShape::FRONT_SIDE))) {    
	  if(tex[FRONT_SIDE]) {
		if(Constants::multitexture) {
		  glSDLActiveTextureARB(GL_TEXTURE0_ARB);
		  if(!useShadow) glEnable(GL_TEXTURE_2D);
		  glBindTexture(GL_TEXTURE_2D, tex[FRONT_SIDE]);
		  glSDLActiveTextureARB(GL_TEXTURE1_ARB);
		  if(!useShadow) glEnable(GL_TEXTURE_2D);		
		  glBindTexture(GL_TEXTURE_2D, lightmap_tex_num);
		} else {
		  glBindTexture( GL_TEXTURE_2D, tex[FRONT_SIDE] );
		}
	  }

	  glBegin( GL_QUADS );
	  // bottom
	  glNormal3f(0.0f, -1.0f, 0.0f);
	  if(Constants::multitexture) {
		glSDLMultiTexCoord2fARB(GL_TEXTURE0_ARB, 0.0f, 1.0f);
		glSDLMultiTexCoord2fARB(GL_TEXTURE1_ARB, 0.0f, 1.0f);
		glVertex3fv(surfaces[BOTTOM_SURFACE]->vertices[0]);
		glSDLMultiTexCoord2fARB(GL_TEXTURE0_ARB, 0.0f, 0.0f);
		glSDLMultiTexCoord2fARB(GL_TEXTURE1_ARB, 0.0f, 0.0f);
		glVertex3fv(surfaces[BOTTOM_SURFACE]->vertices[1]);
		glSDLMultiTexCoord2fARB(GL_TEXTURE0_ARB, 1.0f, 0.0f);
		glSDLMultiTexCoord2fARB(GL_TEXTURE1_ARB, 1.0f, 0.0f);
		glVertex3fv(surfaces[BOTTOM_SURFACE]->vertices[2]);
		glSDLMultiTexCoord2fARB(GL_TEXTURE0_ARB, 1.0f, 1.0f);
		glSDLMultiTexCoord2fARB(GL_TEXTURE1_ARB, 1.0f, 1.0f);
		glVertex3fv(surfaces[BOTTOM_SURFACE]->vertices[3]);
	  } else {
		glTexCoord2f( 0.0f, 1.0f );
		glVertex3fv(surfaces[BOTTOM_SURFACE]->vertices[0]);
		glTexCoord2f( 0.0f, 0.0f );
		glVertex3fv(surfaces[BOTTOM_SURFACE]->vertices[1]);
		glTexCoord2f( 1.0f, 0.0f );
		glVertex3fv(surfaces[BOTTOM_SURFACE]->vertices[2]);
		glTexCoord2f( 1.0f, 1.0f );
		glVertex3fv(surfaces[BOTTOM_SURFACE]->vertices[3]);
	  }
	  glEnd();
	  if(Constants::multitexture) {
		glSDLActiveTextureARB(GL_TEXTURE1_ARB);
		if(!useShadow) glEnable(GL_TEXTURE_2D);		
		glBindTexture(GL_TEXTURE_2D, 0);
		glSDLActiveTextureARB(GL_TEXTURE0_ARB);
		if(!useShadow) glEnable(GL_TEXTURE_2D);
	  }
    }

    if(!(skipside & ( 1 << GLShape::LEFT_RIGHT_SIDE ))) {
	  if(tex[LEFT_RIGHT_SIDE]) {
		if(Constants::multitexture) {
		  glSDLActiveTextureARB(GL_TEXTURE0_ARB);
		  if(!useShadow) glEnable(GL_TEXTURE_2D);
		  glBindTexture(GL_TEXTURE_2D, tex[LEFT_RIGHT_SIDE]);
		  glSDLActiveTextureARB(GL_TEXTURE1_ARB);
		  if(!useShadow) glEnable(GL_TEXTURE_2D);		
		  glBindTexture(GL_TEXTURE_2D, lightmap_tex_num2);
		} else {
		  glBindTexture( GL_TEXTURE_2D, tex[LEFT_RIGHT_SIDE] );
		}
	  }

	  glBegin(GL_QUADS);
	  // right
	  glNormal3f(1.0f, 0.0f, 0.0f);
	  if(Constants::multitexture) {
		glSDLMultiTexCoord2fARB(GL_TEXTURE0_ARB, 0.0f, 0.0f);
		glSDLMultiTexCoord2fARB(GL_TEXTURE1_ARB, 0.0f, 0.0f);
		glVertex3fv(surfaces[RIGHT_SURFACE]->vertices[0]);
		glSDLMultiTexCoord2fARB(GL_TEXTURE0_ARB, 0.0f, 1.0f);
		glSDLMultiTexCoord2fARB(GL_TEXTURE1_ARB, 0.0f, 1.0f);
		glVertex3fv(surfaces[RIGHT_SURFACE]->vertices[1]);
		glSDLMultiTexCoord2fARB(GL_TEXTURE0_ARB, 1.0f, 1.0f);
		glSDLMultiTexCoord2fARB(GL_TEXTURE1_ARB, 1.0f, 1.0f);
		glVertex3fv(surfaces[RIGHT_SURFACE]->vertices[2]);
		glSDLMultiTexCoord2fARB(GL_TEXTURE0_ARB, 1.0f, 0.0f);
		glSDLMultiTexCoord2fARB(GL_TEXTURE1_ARB, 1.0f, 0.0f);
		glVertex3fv(surfaces[RIGHT_SURFACE]->vertices[3]);
	  } else {
		glTexCoord2f( 0.0f, 0.0f );
		glVertex3fv(surfaces[RIGHT_SURFACE]->vertices[0]);
		glTexCoord2f( 0.0f, 1.0f );
		glVertex3fv(surfaces[RIGHT_SURFACE]->vertices[1]);
		glTexCoord2f( 1.0f, 1.0f );
		glVertex3fv(surfaces[RIGHT_SURFACE]->vertices[2]);
		glTexCoord2f( 1.0f, 0.0f );
		glVertex3fv(surfaces[RIGHT_SURFACE]->vertices[3]);
	  }
	  glEnd( );
	  if(Constants::multitexture) {
		glSDLActiveTextureARB(GL_TEXTURE1_ARB);
		if(!useShadow) glEnable(GL_TEXTURE_2D);		
		glBindTexture(GL_TEXTURE_2D, 0);
		glSDLActiveTextureARB(GL_TEXTURE0_ARB);
		if(!useShadow) glEnable(GL_TEXTURE_2D);		
	  }
    }

    if(!(skipside & (1 << GLShape::FRONT_SIDE))) {    
	  if(tex[FRONT_SIDE]) {
		if(Constants::multitexture) {
		  glSDLActiveTextureARB(GL_TEXTURE0_ARB);
		  if(!useShadow) glEnable(GL_TEXTURE_2D);
		  glBindTexture(GL_TEXTURE_2D, tex[FRONT_SIDE]);
		  glSDLActiveTextureARB(GL_TEXTURE1_ARB);
		  if(!useShadow) glEnable(GL_TEXTURE_2D);		
		  glBindTexture(GL_TEXTURE_2D, lightmap_tex_num);
		} else {
		  glBindTexture( GL_TEXTURE_2D, tex[FRONT_SIDE] );
		}
	  }
	  glBegin( GL_QUADS );
	  // front
	  glNormal3f(0.0f, 1.0f, 0.0f);
	  if(Constants::multitexture) {
		glSDLMultiTexCoord2fARB(GL_TEXTURE0_ARB, 1.0f, 1.0f);
		glSDLMultiTexCoord2fARB(GL_TEXTURE1_ARB, 1.0f, 1.0f);
		glVertex3fv(surfaces[FRONT_SURFACE]->vertices[0]);
		glSDLMultiTexCoord2fARB(GL_TEXTURE0_ARB, 1.0f, 0.0f);
		glSDLMultiTexCoord2fARB(GL_TEXTURE1_ARB, 1.0f, 0.0f);
		glVertex3fv(surfaces[FRONT_SURFACE]->vertices[1]);
		glSDLMultiTexCoord2fARB(GL_TEXTURE0_ARB, 0.0f, 0.0f);
		glSDLMultiTexCoord2fARB(GL_TEXTURE1_ARB, 0.0f, 0.0f);
		glVertex3fv(surfaces[FRONT_SURFACE]->vertices[2]);
		glSDLMultiTexCoord2fARB(GL_TEXTURE0_ARB, 0.0f, 1.0f);
		glSDLMultiTexCoord2fARB(GL_TEXTURE1_ARB, 0.0f, 1.0f);
		glVertex3fv(surfaces[FRONT_SURFACE]->vertices[3]);
	  } else {
		glTexCoord2f( 1.0f, 1.0f );
		glVertex3fv(surfaces[FRONT_SURFACE]->vertices[0]);
		glTexCoord2f( 1.0f, 0.0f );
		glVertex3fv(surfaces[FRONT_SURFACE]->vertices[1]);
		glTexCoord2f( 0.0f, 0.0f );
		glVertex3fv(surfaces[FRONT_SURFACE]->vertices[2]);
		glTexCoord2f( 0.0f, 1.0f );
		glVertex3fv(surfaces[FRONT_SURFACE]->vertices[3]);
	  }
	  glEnd();
	  if(Constants::multitexture) {
		glSDLActiveTextureARB(GL_TEXTURE1_ARB);
		if(!useShadow) glEnable(GL_TEXTURE_2D);		
		glBindTexture(GL_TEXTURE_2D, 0);
		glSDLActiveTextureARB(GL_TEXTURE0_ARB);
		if(!useShadow) glEnable(GL_TEXTURE_2D);		
	  }
	}

    if(blending) {
	  glDisable(GL_BLEND);
	  glDepthMask(GL_TRUE);
    }
    if (tex[TOP_SIDE]) glBindTexture( GL_TEXTURE_2D, tex[TOP_SIDE] );
    glBegin( GL_QUADS );
    // top
    glNormal3f(0.0f, 0.0f, 1.0f);
    glTexCoord2f( 1.0f, 1.0f );
	glVertex3fv(surfaces[TOP_SURFACE]->vertices[0]);
    glTexCoord2f( 1.0f, 0.0f );
	glVertex3fv(surfaces[TOP_SURFACE]->vertices[1]);
    glTexCoord2f( 0.0f, 0.0f );
	glVertex3fv(surfaces[TOP_SURFACE]->vertices[2]);
    glTexCoord2f( 0.0f, 1.0f );
	glVertex3fv(surfaces[TOP_SURFACE]->vertices[3]);
    glEnd();
    if(blending) {
	  glEnable(GL_BLEND);
	  glDepthMask(GL_FALSE);
    }
	useShadow = false;
}

void GLShape::setupBlending() { 
  glBlendFunc(GL_ONE, GL_ONE); 
  //glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA); 
  //Scourge::setBlendFunc();
}

void GLShape::createDarkTexture() {
  // create the dark texture
  unsigned int i, j;
  glGenTextures(1, (GLuint*)&lightmap_tex_num);
  glGenTextures(1, (GLuint*)&lightmap_tex_num2);
  float tmp = 0.7f;
  float tmp2 = 0.5f;
  for(i = 0; i < LIGHTMAP_SIZE; i++) {
	for(j = 0; j < LIGHTMAP_SIZE; j++) {
	  data[i * LIGHTMAP_SIZE * 3 + j * 3 + 0] = (unsigned char)(255.0f * tmp * 0.8f);
	  data[i * LIGHTMAP_SIZE * 3 + j * 3 + 1] = (unsigned char)(255.0f * tmp * 0.4f);
	  data[i * LIGHTMAP_SIZE * 3 + j * 3 + 2] = (unsigned char)(255.0f * tmp * 1.0f);

	  data2[i * LIGHTMAP_SIZE * 3 + j * 3 + 0] = (unsigned char)(255.0f * tmp2 * 0.5f);
	  data2[i * LIGHTMAP_SIZE * 3 + j * 3 + 1] = (unsigned char)(255.0f * tmp2 * 0.6f);
	  data2[i * LIGHTMAP_SIZE * 3 + j * 3 + 2] = (unsigned char)(255.0f * tmp2 * 0.8f);
	}
  }
  glBindTexture(GL_TEXTURE_2D, lightmap_tex_num);
  glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
  glTexImage2D(GL_TEXTURE_2D, 0, 3, LIGHTMAP_SIZE, LIGHTMAP_SIZE, 0, 
			   GL_RGB, GL_UNSIGNED_BYTE, data);

  glBindTexture(GL_TEXTURE_2D, lightmap_tex_num2);
  glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
  glTexImage2D(GL_TEXTURE_2D, 0, 3, LIGHTMAP_SIZE, LIGHTMAP_SIZE, 0, 
			   GL_RGB, GL_UNSIGNED_BYTE, data2);

}
