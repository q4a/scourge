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
class ShapePalette;

// temporary information when constructing shapes from a file
typedef struct _ShapeValues {
  char textureGroupIndex[100]; // index or theme ref.
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
  char name[100];
  char filename[100];
  float scale;
} Md2ModelInfo;

class WallTheme {
 public:

  // types of theme section references
  enum {
    THEME_REF_WALL,
    THEME_REF_CORNER,
    THEME_REF_DOOR_EW,
    THEME_REF_DOOR_NS,
    THEME_REF_PASSAGE_FLOOR,
    THEME_REF_ROOM_FLOOR,

    // must be the last one
    THEME_REF_COUNT
  };
  static char themeRefName[THEME_REF_COUNT][40];

 private:
  static const int NAME_LENGTH = 40;
  char *name;
  char textures[THEME_REF_COUNT][3][NAME_LENGTH]; // holds the text of a theme
  GLuint textureGroup[THEME_REF_COUNT][3];
  map<string,GLuint> loadedTextures;
  map<string,int> themeRefMap;

 public:
  WallTheme( char *name );
  ~WallTheme();

  inline void addTextureName(int themeRef, int face, const char *name) { 
    if( themeRef < 0 || themeRef > THEME_REF_COUNT ) {
      cerr << "*** Error: theme ref is out of bounds: theme=" << getName() << endl;
    } else {
      strncpy( textures[themeRef][face], name, NAME_LENGTH - 1 ); 
      textures[themeRef][face][NAME_LENGTH - 1] = '\0';
      cerr << 
        "\ttheme: " << getName() << 
        " texture: ref=" << themeRef << 
        " name=" << name << endl;
    }
  }

  GLuint *getTextureGroup( string themeRefName );
  inline char *getName() { return name; }
  void load( ShapePalette *shapePal );
  void unload();

 protected:
  void loadTextureGroup( int ref, int face, char *texture, ShapePalette *shapePal );
  void debug();
};
  
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
  map<string, GLuint> creature_skins;
  map<GLuint, int> loaded_skins;

  static ShapePalette *instance;
  
  // Md2 shapes
  CLoadMD2 g_LoadMd2; 
  t3DModel * LoadMd2Model(char *file_name);

  Session *session;
  WallTheme *themes[100];
  int themeCount;
  WallTheme *currentTheme;
  vector<GLShape*> themeShapes;
  vector<string> themeShapeRef;

public: 
  ShapePalette(Session *session);
  ~ShapePalette();

  void initialize();

  void loadTheme( const WallTheme *theme );
  void loadTheme( const char *name );
  void loadRandomTheme();

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
  GLShape *getCreatureShape(char *model_name, char *skin_name, float scale=0.0f);
  void decrementSkinRefCount(char *skin_name);

  char *getRandomDescription(int descriptionGroup);

  GLuint loadGLTextures(char *fileName);

protected:
  GLuint loadGLTextureBGRA(SDL_Surface *surface, GLubyte *image, int gl_scale=GL_NEAREST);
  void setupAlphaBlendedBMP(char *filename, SDL_Surface **surface, GLubyte **image, int red=0, int green=0, int blue=0);
  void swap(unsigned char & a, unsigned char & b);
};

#endif
