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
#include "shapepalette.h"

class Map;

// poor man's dynamic lightmaps: shaded sides
static GLuint lightmap_tex_num = 0;
static GLuint lightmap_tex_num2 = 0;
static unsigned char data[LIGHTMAP_SIZE * LIGHTMAP_SIZE * 3];
static unsigned char data2[LIGHTMAP_SIZE * LIGHTMAP_SIZE * 3];

GLShape::GLShape(GLuint tex[],
                 int width, int depth, int height,
                 char *name, int descriptionGroup,
                 Uint32 color,
                 Uint8 shapePalIndex) :
  Shape(width, depth, height, name, descriptionGroup) {
  commonInit(tex, color, shapePalIndex);
}

void GLShape::commonInit(GLuint tex[], Uint32 color, Uint8 shapePalIndex) {
  this->tex = tex;
  this->color = color;
  this->shapePalIndex = shapePalIndex; 
  this->skipside = 0;
  this->useShadow = false;
  this->useTexture = true;
  this->lightBlocking = false;
  this->initialized = false;
  this->variationTextureIndex = 0;

  surfaces[LEFT_SURFACE] = NULL;
  surfaces[BOTTOM_SURFACE] = NULL;
  surfaces[RIGHT_SURFACE] = NULL;
  surfaces[FRONT_SURFACE] = NULL;
  surfaces[TOP_SURFACE] = NULL;
  initSurfaces();
}

void GLShape::setTexture( GLuint *textureGroup ) {
  if( initialized ) {
    glDeleteLists( displayListStart, 3 );
  }
  this->tex = textureGroup;
  initialize();
}

void GLShape::initialize() {
  //cerr << "multitexture=" << Constants::multitexture << " lightmap1=" << lightmap_tex_num << " lightmap2=" << lightmap_tex_num2 << endl;

  displayListStart = glGenLists( 3 );
  if( !displayListStart ) {
    cerr << "*** Error: couldn't generate display lists for shape: " << getName() << endl;
    exit(1);
  }

  createShadowList( displayListStart );
  createBodyList( displayListStart + 1 );
  createTopList( displayListStart + 2 );

  initialized = true;
}


void GLShape::createShadowList( GLuint listName ) {
  glNewList( listName, GL_COMPILE );
  
  // left
  if(!(skipside & ( 1 << GLShape::LEFT_RIGHT_SIDE ))) {    
    glBegin( GL_QUADS );
    glNormal3f(-1.0f, 0.0f, 0.0f);
    glVertex3fv(surfaces[LEFT_SURFACE]->vertices[0]);
    glVertex3fv(surfaces[LEFT_SURFACE]->vertices[1]);
    glVertex3fv(surfaces[LEFT_SURFACE]->vertices[2]);
    glVertex3fv(surfaces[LEFT_SURFACE]->vertices[3]);
    glEnd();
  }

  // bottom
  if(!(skipside & (1 << GLShape::FRONT_SIDE))) {    
    glBegin( GL_QUADS );
    glNormal3f(0.0f, -1.0f, 0.0f);
    glVertex3fv(surfaces[BOTTOM_SURFACE]->vertices[0]);
    glVertex3fv(surfaces[BOTTOM_SURFACE]->vertices[1]);
    glVertex3fv(surfaces[BOTTOM_SURFACE]->vertices[2]);
    glVertex3fv(surfaces[BOTTOM_SURFACE]->vertices[3]);    
    glEnd();
  }

  // right
  if(!(skipside & ( 1 << GLShape::LEFT_RIGHT_SIDE ))) {
    glBegin(GL_QUADS);
    glNormal3f(1.0f, 0.0f, 0.0f);
    glVertex3fv(surfaces[RIGHT_SURFACE]->vertices[0]);
    glVertex3fv(surfaces[RIGHT_SURFACE]->vertices[1]);
    glVertex3fv(surfaces[RIGHT_SURFACE]->vertices[2]);
    glVertex3fv(surfaces[RIGHT_SURFACE]->vertices[3]);
    glEnd( );
  }

  // front
  if(!(skipside & (1 << GLShape::FRONT_SIDE))) {    
    glBegin( GL_QUADS );
    glNormal3f(0.0f, 1.0f, 0.0f);
    glVertex3fv(surfaces[FRONT_SURFACE]->vertices[0]);
    glVertex3fv(surfaces[FRONT_SURFACE]->vertices[1]);
    glVertex3fv(surfaces[FRONT_SURFACE]->vertices[2]);
    glVertex3fv(surfaces[FRONT_SURFACE]->vertices[3]);
    glEnd();
  }

  // top
  glBegin( GL_QUADS );
  glNormal3f(0.0f, 0.0f, 1.0f);
  glVertex3fv(surfaces[TOP_SURFACE]->vertices[0]);
  glVertex3fv(surfaces[TOP_SURFACE]->vertices[1]);
  glVertex3fv(surfaces[TOP_SURFACE]->vertices[2]);
  glVertex3fv(surfaces[TOP_SURFACE]->vertices[3]);
  glEnd();

  glEndList();
}

void GLShape::createBodyList( GLuint listName ) {
  glNewList( listName, GL_COMPILE );

  // hack...
  bool isFloorShape = ( height < 1 );

  // left
  int textureIndex = ( !isFloorShape && this->getVariationTextureIndex() > 0 && depth > width ? this->getVariationTextureIndex() : GLShape::LEFT_RIGHT_SIDE );
 // cerr << "LEFT_RIGHT_SIDE: textureIndex=" << textureIndex << " variation index=" << getVariationTextureIndex() << endl;
  if(!(skipside & ( 1 << GLShape::LEFT_RIGHT_SIDE ))) {    
    if(tex && tex[textureIndex]) 
      glBindTexture( GL_TEXTURE_2D, tex[textureIndex] );
    glBegin( GL_QUADS );
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

  textureIndex = ( !isFloorShape && this->getVariationTextureIndex() > 0 && depth < width ? this->getVariationTextureIndex() : GLShape::FRONT_SIDE );
//  cerr << "FRONT_SIDE: textureIndex=" << textureIndex << " variation index=" << getVariationTextureIndex() << endl;
  if(!(skipside & (1 << GLShape::FRONT_SIDE))) {    
    if(tex && tex[textureIndex]) {
      if(Constants::multitexture) {
        glSDLActiveTextureARB(GL_TEXTURE0_ARB);
        glEnable(GL_TEXTURE_2D);
        glBindTexture(GL_TEXTURE_2D, tex[textureIndex]);
        glSDLActiveTextureARB(GL_TEXTURE1_ARB);
        glEnable(GL_TEXTURE_2D);
        glBindTexture(GL_TEXTURE_2D, lightmap_tex_num);
      } else {
        glBindTexture( GL_TEXTURE_2D, tex[textureIndex] );
      }
    }

    // bottom
    glBegin( GL_QUADS );
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
      glEnable(GL_TEXTURE_2D);
      glBindTexture(GL_TEXTURE_2D, 0);
      glSDLActiveTextureARB(GL_TEXTURE0_ARB);
      glEnable(GL_TEXTURE_2D);
    }
  }

  textureIndex = ( !isFloorShape && this->getVariationTextureIndex() > 0 && depth > width ? this->getVariationTextureIndex() : GLShape::LEFT_RIGHT_SIDE );
//  cerr << "LEFT_RIGHT_SIDE: textureIndex=" << textureIndex << " variation index=" << getVariationTextureIndex() << endl;
  if(!(skipside & ( 1 << GLShape::LEFT_RIGHT_SIDE ))) {
    if(tex && tex[textureIndex]) {
      if(Constants::multitexture) {
        glSDLActiveTextureARB(GL_TEXTURE0_ARB);
        glEnable(GL_TEXTURE_2D);
        glBindTexture(GL_TEXTURE_2D, tex[textureIndex]);
        glSDLActiveTextureARB(GL_TEXTURE1_ARB);
        glEnable(GL_TEXTURE_2D);
        glBindTexture(GL_TEXTURE_2D, lightmap_tex_num2);
      } else {
        glBindTexture( GL_TEXTURE_2D, tex[textureIndex] );
      }
    }

    // right
    glBegin(GL_QUADS);
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
      glEnable(GL_TEXTURE_2D);
      glBindTexture(GL_TEXTURE_2D, 0);
      glSDLActiveTextureARB(GL_TEXTURE0_ARB);
      glEnable(GL_TEXTURE_2D);
    }
  }

  textureIndex = ( !isFloorShape && this->getVariationTextureIndex() > 0 && depth < width ? this->getVariationTextureIndex() : GLShape::FRONT_SIDE );
//  cerr << "FRONT_SIDE: textureIndex=" << textureIndex << " variation index=" << getVariationTextureIndex() << endl;
  if(!(skipside & (1 << GLShape::FRONT_SIDE))) {    
    if(tex && tex[textureIndex]) {
      if(Constants::multitexture) {
        glSDLActiveTextureARB(GL_TEXTURE0_ARB);
        glEnable(GL_TEXTURE_2D);
        glBindTexture(GL_TEXTURE_2D, tex[textureIndex]);
        glSDLActiveTextureARB(GL_TEXTURE1_ARB);
        glEnable(GL_TEXTURE_2D);
        glBindTexture(GL_TEXTURE_2D, lightmap_tex_num);
      } else {
        glBindTexture( GL_TEXTURE_2D, tex[textureIndex] );
      }
    }

    // front
    glBegin( GL_QUADS );
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
      glEnable(GL_TEXTURE_2D);
      glBindTexture(GL_TEXTURE_2D, 0);
      glSDLActiveTextureARB(GL_TEXTURE0_ARB);
      glEnable(GL_TEXTURE_2D);
    }
  }

  glEndList();
}

void GLShape::createTopList( GLuint listName ) {
  glNewList( listName, GL_COMPILE );

  // hack...
  bool isFloorShape = ( height < 1 );

  int textureIndex = (  isFloorShape && this->getVariationTextureIndex() > 0 ? this->getVariationTextureIndex() : GLShape::TOP_SIDE );

  //cerr << "TOP: textureIndex=" << textureIndex << " variation index=" << getVariationTextureIndex() << endl;

  if(tex && tex[textureIndex]) glBindTexture( GL_TEXTURE_2D, tex[textureIndex] );
  glBegin( GL_QUADS );
  glNormal3f(0.0f, 0.0f, 1.0f);

  if( getWidth() > getHeight() ) {
    glTexCoord2f( 1, 0 );
    glVertex3fv(surfaces[TOP_SURFACE]->vertices[0]);
    glTexCoord2f( 1, 1 );
    glVertex3fv(surfaces[TOP_SURFACE]->vertices[1]);
    glTexCoord2f( 0, 1 );
    glVertex3fv(surfaces[TOP_SURFACE]->vertices[2]);
    glTexCoord2f( 0, 0 );
    glVertex3fv(surfaces[TOP_SURFACE]->vertices[3]);
  } else {
    glTexCoord2f( 0, 0 );
    glVertex3fv(surfaces[TOP_SURFACE]->vertices[0]);
    glTexCoord2f( 1, 0 );
    glVertex3fv(surfaces[TOP_SURFACE]->vertices[1]);
    glTexCoord2f( 1, 1 );
    glVertex3fv(surfaces[TOP_SURFACE]->vertices[2]);
    glTexCoord2f( 0, 1 );
    glVertex3fv(surfaces[TOP_SURFACE]->vertices[3]);
  }

/*
  glTexCoord2f( 1.0f, 1.0f );
  glVertex3fv(surfaces[TOP_SURFACE]->vertices[0]);
  glTexCoord2f( 1.0f, 0.0f );
  glVertex3fv(surfaces[TOP_SURFACE]->vertices[1]);
  glTexCoord2f( 0.0f, 0.0f );
  glVertex3fv(surfaces[TOP_SURFACE]->vertices[2]);
  glTexCoord2f( 0.0f, 1.0f );
  glVertex3fv(surfaces[TOP_SURFACE]->vertices[3]);
*/
  glEnd();  

  glEndList();
}

GLShape::~GLShape(){
  free(surfaces[LEFT_SURFACE]);
  free(surfaces[BOTTOM_SURFACE]);
  free(surfaces[RIGHT_SURFACE]);
  free(surfaces[FRONT_SURFACE]);
  free(surfaces[TOP_SURFACE]);
  if( initialized ) glDeleteLists( displayListStart, 3 );
  deleteVariationShapes();
}

void GLShape::drawShadow() {
  // cull back faces
  glEnable( GL_CULL_FACE );
  glCullFace( GL_BACK );

  glCallList( displayListStart );

  // reset shadow flag
  useShadow = false;
}

void GLShape::draw() {

  if(!initialized) {
    cerr << "*** Warning: shape not intialized. name=" << getName() << endl;
  }

  if(useShadow) {
    drawShadow();
    return;
  }

  // don't blend the top, but if in shadow mode, don't mess with blending
  GLboolean blending = glIsEnabled(GL_BLEND);

  // cull back faces
  glEnable( GL_CULL_FACE );
  glCullFace( GL_BACK );
  bool textureWasEnabled = glIsEnabled( GL_TEXTURE_2D );
  glEnable( GL_TEXTURE_2D );

  glCallList( displayListStart + 1 );

  // top
  if(blending) {
    glDisable(GL_BLEND);
    glDepthMask(GL_TRUE);
  }

  glCallList( displayListStart + 2 );

  if(blending) {
    glEnable(GL_BLEND);
    glDepthMask(GL_FALSE);
  }
  if( !textureWasEnabled ) glDisable( GL_TEXTURE_2D );
}

void GLShape::outline( float r, float g, float b ) {
  useShadow = true;
  GLboolean blend;
  glGetBooleanv( GL_BLEND, &blend );
  glEnable( GL_BLEND );
  glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );
  GLboolean texture = glIsEnabled( GL_TEXTURE_2D );
  glDisable( GL_TEXTURE_2D );
  glPolygonMode( GL_FRONT, GL_LINE );
  glLineWidth( 4 );
  glEnable( GL_CULL_FACE );
  glCullFace( GL_BACK );
  //glEnable( GL_DEPTH_TEST );
  //GLint df;
  //glGetIntegerv( GL_DEPTH_FUNC, &df );
  //glDepthFunc( GL_GEQUAL );
  glColor3f( r, g, b );  
  glCallList( displayListStart );
  glLineWidth( 1 );
  //glDepthFunc( df );
  //glCullFace( GL_BACK );
  glDisable( GL_CULL_FACE );
  glPolygonMode( GL_FRONT, GL_FILL );
  if( !blend ) glDisable( GL_BLEND );
  if( texture ) glEnable( GL_TEXTURE_2D );
  useShadow = false;
  glColor4f(1, 1, 1, 0.9f);
}

void GLShape::setupBlending() { 
  glBlendFunc(GL_ONE, GL_ONE); 
  //glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA); 
  //Scourge::setBlendFunc();
}

void GLShape::createDarkTexture( WallTheme *theme ) {
  if( !Constants::multitexture ) {
    return;
  }

  // delete the previous texture
  if(lightmap_tex_num != 0) {
    glDeleteTextures( 1, (GLuint*)&lightmap_tex_num );
    glDeleteTextures( 1, (GLuint*)&lightmap_tex_num2 );
    lightmap_tex_num = lightmap_tex_num2 = 0;
  }  

  cerr << "*** Creating multitexture overlay." << endl;
  if( theme ) {
    cerr << "*** theme=" << theme->getName() << 
      " color=" << theme->getMultiTexRed(0) << "," << theme->getMultiTexGreen(0) << "," << theme->getMultiTexBlue(0) << 
      "," << theme->getMultiTexInt(0) << 
      " color2=" << theme->getMultiTexRed(1) << "," << theme->getMultiTexGreen(1) << "," << theme->getMultiTexBlue(1) << 
      "," << theme->getMultiTexInt(1) << 
      " smooth? " << theme->getMultiTexSmooth( 0 ) << ", " << theme->getMultiTexSmooth( 1 ) << endl;
  } else {
    cerr << "*** no theme." << endl;
  }

  // create the dark texture
  unsigned int i, j;
  glGenTextures(1, (GLuint*)&lightmap_tex_num);
  glGenTextures(1, (GLuint*)&lightmap_tex_num2);
  float tmp = (theme ? theme->getMultiTexInt( 0 ) : 0.95f);
  float tmp2 = (theme ? theme->getMultiTexInt( 1 ) : 0.8f);
  for(i = 0; i < LIGHTMAP_SIZE; i++) {
    for(j = 0; j < LIGHTMAP_SIZE; j++) {

      float d = 255.0f;
      if(!theme || !theme->getMultiTexSmooth( 0 )) d = (128.0f * rand()/RAND_MAX) + 127.0f;

      // purple
      data[i * LIGHTMAP_SIZE * 3 + j * 3 + 0] = 
        (unsigned char)(d * tmp * (theme ? theme->getMultiTexRed(0) : 0.8f));
      data[i * LIGHTMAP_SIZE * 3 + j * 3 + 1] = 
        (unsigned char)(d * tmp * (theme ? theme->getMultiTexGreen(0) : 0.4f));
      data[i * LIGHTMAP_SIZE * 3 + j * 3 + 2] = 
        (unsigned char)(d * tmp * (theme ? theme->getMultiTexBlue(0) : 1.0f));

      d = 255.0f;
      if(!theme || !theme->getMultiTexSmooth( 1 )) d = (128.0f * rand()/RAND_MAX) + 127.0f;

      // dark
      data2[i * LIGHTMAP_SIZE * 3 + j * 3 + 0] = 
        (unsigned char)(d * tmp2 * (theme ? theme->getMultiTexRed(1) : 0.5f));
      data2[i * LIGHTMAP_SIZE * 3 + j * 3 + 1] = 
        (unsigned char)(d * tmp2 * (theme ? theme->getMultiTexGreen(1) : 0.6f));
      data2[i * LIGHTMAP_SIZE * 3 + j * 3 + 2] = 
        (unsigned char)(d * tmp2 * (theme ? theme->getMultiTexBlue(1) : 0.8f));        
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

bool GLShape::isLightBlocking() { 
	return lightBlocking; 
}

void GLShape::setLightBlocking(bool b) { 
	lightBlocking = b; 
}

// loosely interpreted...
bool GLShape::fitsInside(GLShape *smaller) {
  return (smaller->getHeight() < 2 ||
          (width + 1 > smaller->getWidth() && 
           depth + 1 > smaller->getDepth() &&
           height + 1 > smaller->getHeight()));
}

void GLShape::setCurrentAnimation (int numAnim, bool force){
    cout<<"GLShape::setCurrentAnimation : Hey this should call MD2Shape function!"<<endl;
}

void GLShape::setPauseAnimation (bool pause){
    cout<<"GLShape::setPauseAnimation : Hey this should call MD2Shape function!"<<endl;
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

void GLShape::initSurfaces() {
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
  if(surfaces[LEFT_SURFACE]) free(surfaces[LEFT_SURFACE]);
  surfaces[LEFT_SURFACE] = new_surface(v);

  v[0][0] = 0.0f; v[0][1] = 0.0f; v[0][2] = 0.0f;
  v[1][0] = 0.0f; v[1][1] = 0.0f; v[1][2] = h;
  v[2][0] = w;    v[2][1] = 0.0f; v[2][2] = h;
  v[3][0] = w;    v[3][1] = 0.0f; v[3][2] = 0.0f;
  if(surfaces[BOTTOM_SURFACE]) free(surfaces[BOTTOM_SURFACE]);
  surfaces[BOTTOM_SURFACE] = new_surface(v);

  v[0][0] = w;    v[0][1] = d;    v[0][2] = h;
  v[1][0] = w;    v[1][1] = d;    v[1][2] = 0.0f;
  v[2][0] = w;    v[2][1] = 0.0f; v[2][2] = 0.0f;
  v[3][0] = w;    v[3][1] = 0.0f; v[3][2] = h;
  if(surfaces[RIGHT_SURFACE]) free(surfaces[RIGHT_SURFACE]);
  surfaces[RIGHT_SURFACE] = new_surface(v);

  v[0][0] = w;    v[0][1] = d;    v[0][2] = 0.0f;
  v[1][0] = w;    v[1][1] = d;    v[1][2] = h;
  v[2][0] = 0.0f; v[2][1] = d;    v[2][2] = h;
  v[3][0] = 0.0f; v[3][1] = d;    v[3][2] = 0.0f;
  if(surfaces[FRONT_SURFACE]) free(surfaces[FRONT_SURFACE]);
  surfaces[FRONT_SURFACE] = new_surface(v);

  v[0][0] = w;    v[0][1] = d;    v[0][2] = h;
  v[1][0] = w;    v[1][1] = 0.0f; v[1][2] = h;
  v[2][0] = 0.0f; v[2][1] = 0.0f; v[2][2] = h;
  v[3][0] = 0.0f; v[3][1] = d;    v[3][2] = h;
  if(surfaces[TOP_SURFACE]) free(surfaces[TOP_SURFACE]);
  surfaces[TOP_SURFACE] = new_surface(v);
}

void GLShape::deleteVariationShapes() {
  for( int i = 0; i < (int)variationShape.size(); i++ ) {
    GLShape *shape = variationShape[i];
    delete shape;
  }
  variationShape.clear();
}

void GLShape::createVariationShape( int textureIndex, GLuint *textureGroup ) {
  // create a duplicate of this shape and set the texture index
  GLShape *dup = new GLShape( tex, width, depth, height,
                              getName(), getDescriptionGroup(),
                              color, shapePalIndex );
  dup->setVariationTextureIndex( textureIndex );
  dup->setTexture( textureGroup );
  variationShape.push_back( dup );
}

