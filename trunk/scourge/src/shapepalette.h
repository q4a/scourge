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
#include "rpg/monster.h"

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

typedef struct _CharacterModelInfo {
  char model_name[100];
  char skin_name[300]; 
  float scale;
} CharacterModelInfo;

#define MAX_TEXTURE_COUNT 10
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
  static const int MULTI_TEX_COUNT = 2;

 private:
  static const int NAME_LENGTH = 40;
  char *name;
  char textures[THEME_REF_COUNT][MAX_TEXTURE_COUNT][NAME_LENGTH]; // holds the text of a theme
  GLuint textureGroup[THEME_REF_COUNT][MAX_TEXTURE_COUNT];
  int faceCount[THEME_REF_COUNT];
  map<string,GLuint> loadedTextures;
  map<string,int> themeRefMap;
  GLfloat r[MULTI_TEX_COUNT], g[MULTI_TEX_COUNT], b[MULTI_TEX_COUNT], intensity[MULTI_TEX_COUNT];
  bool smooth[MULTI_TEX_COUNT];
  ShapePalette *shapePal;

 public:
  WallTheme( char *name, ShapePalette *shapePal );
  ~WallTheme();

  inline void setFaceCount( int themeRef, int value ) { faceCount[ themeRef ] = value; }
  int getFaceCount( string themeRefName );

  inline void addTextureName(int themeRef, int face, const char *name) { 
    if( themeRef < 0 || themeRef > THEME_REF_COUNT ) {
      cerr << "*** Error: theme ref is out of bounds: theme=" << getName() << endl;
    } else {
      strncpy( textures[themeRef][face], name, NAME_LENGTH - 1 ); 
      textures[themeRef][face][NAME_LENGTH - 1] = '\0';
      /*
      cerr << 
        "\ttheme: " << getName() << 
        " texture: ref=" << themeRef << 
        " name=" << name << endl;
      */        
    }
  }
  inline void setMultiTexRed( int index, GLfloat value ) { r[index] = value; }
  inline void setMultiTexGreen( int index, GLfloat value ) { g[index] = value; }
  inline void setMultiTexBlue( int index, GLfloat value ) { b[index] = value; }
  inline void setMultiTexInt( int index, GLfloat value ) { intensity[index] = value; }
  inline void setMultiTexSmooth( int index, bool value ) { smooth[index] = value; }

  inline GLfloat getMultiTexRed( int index ) { return r[index]; }
  inline GLfloat getMultiTexGreen( int index ) { return g[index]; }
  inline GLfloat getMultiTexBlue( int index ) { return b[index]; }
  inline GLfloat getMultiTexInt( int index ) { return intensity[index]; }
  inline bool getMultiTexSmooth( int index ) { return smooth[index]; }

  GLuint *getTextureGroup( string themeRefName );
  inline char *getName() { return name; }
  void load();
  void unload();

 protected:
  void loadTextureGroup( int ref, int face, char *texture );
  void debug();
};
  

#define MAX_SYSTEM_TEXTURE_COUNT 1000

class ShapePalette {
private:
  GLShape *shapes[256];
  map<string, GLShape *> shapeMap;
  int shapeCount;
  GLuint gui_texture, gui_wood_texture, paper_doll_texture, gui_texture2, ripple_texture;
  map<int, GLuint> statModIcons;
  
  typedef struct _Texture {
	GLuint id;
	char filename[80];
  } Texture;

  Texture textures[ MAX_SYSTEM_TEXTURE_COUNT ]; // store textures
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
  map<string, Md2ModelInfo*> old_creature_models; 
  map<string, GLuint> creature_skins;
  map<GLuint, int> loaded_skins;
  map<string, Md2ModelInfo*> creature_models;
  map<Md2ModelInfo*, int> loaded_models;
  vector<CharacterModelInfo*> character_models;

  static ShapePalette *instance;
  
  // Md2 shapes
  CLoadMD2 g_LoadMd2; 
  t3DModel * LoadMd2Model(char *file_name);
  void UnloadMd2Model( t3DModel *model );

  Session *session;
  WallTheme *themes[100];
  int themeCount;
  WallTheme *currentTheme;
  vector<GLShape*> themeShapes;
  vector<string> themeShapeRef;

  vector<GLuint> portraitTextures;
  GLuint deathPortraitTexture;

public: 
  ShapePalette(Session *session);
  ~ShapePalette();

  void initialize();
  GLuint loadSystemTexture( char *line );

  inline int getCharacterModelInfoCount() { return character_models.size(); }
  inline CharacterModelInfo *getCharacterModelInfo( int index ) { return character_models[ index ]; }

  void loadTheme( WallTheme *theme );
  void loadTheme( const char *name );
  void loadRandomTheme();

  GLuint formationTexIndex;
  inline GLuint getTexture(int index) { return textures[index].id; }

  inline GLuint getStatModIcon(int statModIndex) { if(statModIcons.find(statModIndex) == statModIcons.end()) return (GLuint)0; else return statModIcons[statModIndex]; }

  // singleton
  inline static ShapePalette *getInstance() { return instance; }

  // cursor
  SDL_Surface *tiles, *spells;
  GLubyte *tilesImage[20][20], *spellsImage[20][20];
  GLuint tilesTex[20][20], spellsTex[20][20];
  SDL_Surface *cursor, *crosshair, *attackCursor, *talkCursor, *paperDoll;
  GLubyte *cursorImage, *crosshairImage, *attackImage, *talkImage, *paperDollImage;
  GLuint cursor_texture, crosshair_texture, attack_texture, talk_texture;

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

  inline GLuint getRippleTexture() { return ripple_texture; }
  inline GLuint getGuiTexture() { return gui_texture; }
  inline GLuint getGuiTexture2() { return gui_texture2; }
  inline GLuint getGuiWoodTexture() { return gui_wood_texture; }
  inline GLuint getPaperDollTexture() { return paper_doll_texture; }
  inline GLuint getBorderTexture() { return border; }
  inline GLuint getBorder2Texture() { return border2; }
  inline GLuint getGargoyleTexture() { return gargoyle; }
  inline GLuint getHighlightTexture() { return highlight; }

  inline int getPortraitCount() { return portraitTextures.size(); }
  inline GLuint getPortraitTexture( int index ) { return portraitTextures[ index ]; }
  inline GLuint getDeathPortraitTexture() { return deathPortraitTexture; }

  GLuint findTextureByName(const char *filename);
  GLShape *findShapeByName(const char *name, bool variation=false);
  int findShapeIndexByName(const char *name);
  
  // Md2 shapes
  GLShape *getCreatureShape(char *model_name, char *skin_name, float scale=0.0f, 
							Monster *monster=NULL);
  void decrementSkinRefCount(char *model_name, char *skin_name, 
							 Monster *monster=NULL);

  char *getRandomDescription(int descriptionGroup);

  GLuint loadGLTextures(char *fileName);

protected:
  GLuint loadGLTextureBGRA(SDL_Surface *surface, GLubyte *image, int gl_scale=GL_NEAREST);
  GLuint loadGLTextureBGRA(int w, int h, GLubyte *image, int gl_scale=GL_NEAREST);
  void setupAlphaBlendedBMP(char *filename, SDL_Surface **surface, GLubyte **image, int red=0, int green=0, int blue=0);
  void setupAlphaBlendedBMPGrid(char *filename, SDL_Surface **surface, GLubyte *tilesImage[20][20], int imageWidth, int imageHeight, int tileWidth, int tileHeight, int red=0, int green=0, int blue=0, int nred=-1, int ngreen=-1, int nblue=-1);
  void swap(unsigned char & a, unsigned char & b);
};

#endif
