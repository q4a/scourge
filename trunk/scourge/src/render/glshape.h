/***************************************************************************
                          glshape.h  -  description
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

#ifndef GLSHAPE_H
#define GLSHAPE_H

#include "render.h"
#include "shape.h"
#include <vector>

#define DIV 0.06f

class WallTheme;
class Effect;

/**
  *@author Gabor Torok
  */
struct surface {
	float vertices[4][3];
  //	float matrix[9];
  //	float s_dist, t_dist;
};

class GLShape : public Shape  {
protected:
  /**
    Represents an array of 3 addresses for textures. If tex[i]==NULL, no texture is given,
    only a color.
  */
  GLuint *tex;
  Uint8 shapePalIndex;
  int skipside;
  bool useTexture;
  bool lightBlocking;
  float xIconRot, yIconRot, zIconRot;
  GLuint displayListStart;
  bool initialized;
  std::vector<GLShape*> variationShape;
  int variationTextureIndex;

  struct surface *surfaces[5];
  enum { 
    LEFT_SURFACE=0, 
    RIGHT_SURFACE, 
    FRONT_SURFACE, 
    TOP_SURFACE, 
    BOTTOM_SURFACE 
  };

#define LIGHTMAP_SIZE 16

  int effectType;
  int effectWidth, effectDepth, effectHeight;
  int effectX, effectY, effectZ;

 protected:
  Uint32 color;
  
 public:
  static const int FRONT_SIDE = 0;
  static const int TOP_SIDE = 1;
  static const int LEFT_RIGHT_SIDE = 2;  
  
  inline void setUseTexture(bool b) { useTexture = b; }
  inline bool getUseTexture() { return useTexture; }
  
  bool useShadow;

public:

  /**
     Passing 0 for texture disables the creation of
     shapes. (eg. torch, md2 shape)
  */
  GLShape(GLuint texture[],
          int width, int depth, int height,
          char *name, int descriptionGroup,
          Uint32 color, Uint8 shapePalIndex=0);
  
  virtual ~GLShape();  

  virtual void initialize();
  virtual inline void cleanup() {}

  void deleteVariationShapes();
  void createVariationShape( int textureIndex, GLuint *textureGroup );
  inline void setVariationTextureIndex( int index ) { variationTextureIndex = index; }
  inline int getVariationTextureIndex() { return variationTextureIndex; }
  inline int getVariationShapesCount() { return variationShape.size(); }
  inline GLShape *getVariationShape( int index ) { return variationShape[ index ]; }

  static void createDarkTexture( WallTheme *theme );
  void setTexture( GLuint *textureGroup );

  inline void setSkipSide(int n) { skipside = n; }
  bool fitsInside( GLShape *smaller, bool relaxedRules=false );  
  bool isLightBlocking();
  void setLightBlocking(bool b);
  void draw();   
  void outline( float r, float g, float b );
  inline void setupToDraw() {};
  
  inline Uint8 getShapePalIndex() { return shapePalIndex; }
  
  inline void setCameraRot(GLfloat xrot, GLfloat yrot, GLfloat zrot) { this->xrot = xrot; this->yrot = yrot; this->zrot = zrot; }
  inline void setCameraPos(GLfloat xpos, GLfloat ypos, GLfloat zpos, GLfloat xpos2, GLfloat ypos2, GLfloat zpos2) { this->xpos = xpos; this->ypos = ypos; this->zpos = zpos; this->xpos2 = xpos2; this->ypos2 = ypos2; this->zpos2 = zpos2;}
  inline void setLocked(bool locked) { this->locked = locked; }
  inline GLfloat getYRot() { return yrot; }
  inline GLfloat getZRot() { return zrot; }
  
  inline bool drawFirst() { return true; }
  // if true, the next two functions are called
  inline bool drawLater() { return false; }
  //inline void setupBlending() { glBlendFunc(GL_ONE_MINUS_SRC_COLOR, GL_SRC_ALPHA); }
  void setupBlending();
  inline void endBlending() { }
  inline void setIconRotation(float x, float y, float z) { xIconRot = x; yIconRot = y; zIconRot = z; }
  inline void applyIconRotation() { glRotatef(xIconRot, 1, 0, 0); glRotatef(yIconRot, 0, 1, 0); glRotatef(zIconRot, 0, 0, 1); }

  virtual void setCurrentAnimation (int numAnim, bool force=false);      
  virtual void setPauseAnimation(bool pause);

  virtual inline int getEffectType() { return effectType; }
  virtual inline int getEffectWidth() { return effectWidth; }
  virtual inline int getEffectDepth() { return effectDepth; }
  virtual inline int getEffectHeight() { return effectHeight; }
  virtual inline int getEffectX() { return effectX; }
  virtual inline int getEffectY() { return effectY; }
  virtual inline int getEffectZ() { return effectZ; }
  virtual inline void setEffectType( int n, int w, int d, int h, int x, int y, int z ) { 
    effectType = n; 
    effectWidth = w;
    effectDepth = d;
    effectHeight = h;
    effectX = x;
    effectY = y;
    effectZ = z;
  }
  
 protected:
  bool locked;
  GLfloat xrot, yrot, zrot;
  GLfloat xpos, ypos, zpos, xpos2, ypos2, zpos2;
  void commonInit(GLuint tex[], Uint32 color, Uint8 shapePalIndex);
  static struct surface *new_surface(float vertices[4][3]);
  void initSurfaces();
  void drawShadow();
  void createShadowList( GLuint listName );
  void createBodyList( GLuint listName );
  void createTopList( GLuint listName );
};

#endif
