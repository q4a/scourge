/***************************************************************************
                          shapepalette.h  -  description
                             -------------------
    begin                : Sat Jun 14 2003
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

#ifndef SHAPEPALETTE_H
#define SHAPEPALETTE_H

#include <string>
#include <vector>
#include <map>
#include "constants.h"
#include "shape.h"
#include "glshape.h"
#include "gltorch.h"
#include "glteleporter.h"
#include "gllocator.h"
#include "md2shape.h"
#include "3dsshape.h"
#include "Md2.h"

using namespace std;

/**
  *@author Gabor Torok
  */

class GLShape;
class GLTorch;
class Session;

// temporary information when constructing shapes from a file
typedef struct _ShapeValues {
  int textureGroupIndex;
  int width, height, depth;
  char name[100];
  int descriptionIndex;
  long color;
  int skipSide, stencil, blocksLight;
  int torch;
  char m3ds_name[100];
  float m3ds_scale;
  int teleporter;
  float xrot, yrot, zrot;
} ShapeValues;

typedef struct _Md2ModelInfo {
  t3DModel *model;
  int width, height, depth;
  char name[100];
  char filename[100];
  float scale;
} Md2ModelInfo;
  
class ShapePalette {
private:
  GLShape *shapes[256];
  map<string, GLShape *> shapeMap;
  int shapeCount;
  GLuint gui_texture, gui_wood_texture, paper_doll_texture;
  map<int, GLuint> statModIcons;
  
  typedef struct _Texture {
	GLuint id;
	char filename[80];
  } Texture;

  Texture textures[100]; // store textures
  int texture_count;
  GLShape *shapeNameArray[256];

  // native texture groups
  GLuint textureGroup[100][3];
  int textureGroupCount;

  GLuint md2_tex[6];

  // how big to make the walls
  const static Sint16 unitSide = MAP_UNIT;
  const static Sint16 unitOffset = MAP_UNIT_OFFSET;
  const static Sint16 wallHeight = MAP_WALL_HEIGHT;

  // shape descriptions
  vector<vector<string>*> descriptions;

  // temp. shape data
  vector<ShapeValues*> shapeValueVector;

  // md2 data
  map<string, Md2ModelInfo *> creature_models; 
  map<string, GLShape *> creature_block_shapes;
  map<string, GLuint> creature_skins;
  map<GLuint, int> loaded_skins;

  static ShapePalette *instance;
  
  // Md2 shapes
  CLoadMD2 g_LoadMd2; 
  t3DModel * LoadMd2Model(char *file_name);

  Session *session;

public: 
  ShapePalette(Session *session);
  ~ShapePalette();

  void initialize();

  GLuint formationTexIndex;
  inline GLuint getTexture(int index) { return textures[index].id; }

  inline GLuint getStatModIcon(int statModIndex) { if(statModIcons.find(statModIndex) == statModIcons.end()) return (GLuint)0; else return statModIcons[statModIndex]; }

  // singleton
  inline static ShapePalette *getInstance() { return instance; }

  // cursor
  SDL_Surface *cursor, *crosshair, *paperDoll;
  GLubyte *cursorImage, *crosshairImage, *paperDollImage;
  GLuint cursor_texture, crosshair_texture;

  SDL_Surface *logo;
  GLubyte *logoImage;   
  GLuint logo_texture;

  SDL_Surface *scourge;
  GLubyte *scourgeImage;

  GLuint cloud, candle, torchback, highlight;

  GLuint border, border2, gargoyle;
       
  inline GLShape *getShape(int index) { return shapes[index]; }  

  inline Sint16 getUnitSide() { return unitSide; }
  inline Sint16 getUnitOffset() { return unitOffset; }
  inline Sint16 getWallHeight() { return wallHeight; }

  inline GLuint getGuiTexture() { return gui_texture; }
  inline GLuint getGuiWoodTexture() { return gui_wood_texture; }
  inline GLuint getPaperDollTexture() { return paper_doll_texture; }
  inline GLuint getBorderTexture() { return border; }
  inline GLuint getBorder2Texture() { return border2; }
  inline GLuint getGargoyleTexture() { return gargoyle; }
  inline GLuint getHighlightTexture() { return highlight; }

  GLuint findTextureByName(const char *filename);
  GLShape *findShapeByName(const char *name);
  int findShapeIndexByName(const char *name);
  
  // Md2 shapes
  GLShape *getCreatureShape(char *model_name, char *skin_name, float scale=0.0f, int w=0, int d=0, int h=0);
  void decrementSkinRefCount(char *skin_name);
  // use this method to get a meta-shape for the creature (good for mearuring 'fit'-s in dungeongenerator
  inline GLShape *getCreatureBlockShape(char *name) { string s = name; return creature_block_shapes[s]; }

  char *getRandomDescription(int descriptionGroup);

protected:
  GLuint loadGLTextures(char *fileName);
  GLuint loadGLTextureBGRA(SDL_Surface *surface, GLubyte *image, int gl_scale=GL_NEAREST);
  void setupAlphaBlendedBMP(char *filename, SDL_Surface **surface, GLubyte **image, int red=0, int green=0, int blue=0);
  void swap(unsigned char & a, unsigned char & b);
};

#endif
