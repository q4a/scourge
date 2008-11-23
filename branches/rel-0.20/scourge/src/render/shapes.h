/***************************************************************************
                          shapes.h  -  description
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

#ifndef SHAPES_H
#define SHAPES_H

#include "render.h"
#include <string>
#include <vector>
#include <map>
#include "modelwrapper.h"
#include "../session.h" // -=K=-: can't declare inline bool Shapes::isHeadless() without

/**
  *@author Gabor Torok
  */

class GLShape;
class GLTorch;
class Shapes;
class Session;

struct Occurs {
	bool rooms_only;
	int max_count;
	char placement[100];
	char use_function[255];
	char theme[255];
};

// temporary information when constructing shapes from a file
// -=K=-: turning that struct into class, (complex members, so its class, not PODS)  
class ShapeValues {
public:
  // char textureGroupIndex[100]; // index or theme ref.
	char theme[40];
	bool wallShape;
	char textures[255];
  int width, height, depth;
  char name[100];
  int descriptionIndex;
  long color;
  int skipSide, stencil, blocksLight;
  int torch;
  std::string m3ds_name;
  float m3ds_scale;
  float m3ds_x, m3ds_y, m3ds_z;
  float o3ds_x, o3ds_y, o3ds_z;  
	float xrot3d, yrot3d, zrot3d;
  int teleporter;
  float xrot, yrot, zrot;
  int effectType;
  int effectWidth, effectDepth, effectHeight;
  int effectX, effectY, effectZ;
  bool interactive;
	float outdoorsWeight;
	bool outdoorShadow;
	bool wind;
	Occurs occurs;
	int iconRotX;
	int iconRotY;
	int iconRotZ;
	GLuint icon;
	int iconWidth, iconHeight;
	std::string ambient;
	int lighting;
	float base_w, base_h;
};

struct CharacterModelInfo {
  char model_name[100];
  char skin_name[300]; 
  float scale;
};

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
    THEME_REF_HEADBOARD,

    // must be the last one
    THEME_REF_COUNT
  };
  static char themeRefName[THEME_REF_COUNT][40];
  static const int MULTI_TEX_COUNT = 2;

 private:
  static const int NAME_LENGTH = 40;
  char name[80];
  char textures[THEME_REF_COUNT][MAX_TEXTURE_COUNT][NAME_LENGTH]; // holds the text of a theme
  GLuint textureGroup[THEME_REF_COUNT][MAX_TEXTURE_COUNT];
  int faceCount[THEME_REF_COUNT];
  std::map<std::string,GLuint> loadedTextures;
  std::map<std::string,int> themeRefMap;
  GLfloat r[MULTI_TEX_COUNT], g[MULTI_TEX_COUNT], b[MULTI_TEX_COUNT], intensity[MULTI_TEX_COUNT];
  bool smooth[MULTI_TEX_COUNT];
  Shapes *shapePal;
  bool special;
  bool cave;
  TextureData lavaData;
  TextureData floorData;

 public:
  WallTheme( char const* name, Shapes *shapePal );
  ~WallTheme();

  inline TextureData& getFloorData() { return floorData; }

  inline void setSpecial( bool b ) { special = b; }
  inline bool isSpecial() { return special; }

  inline void setCave( bool b ) { cave = b; }
  inline bool isCave() { return cave; }

  inline void setFaceCount( int themeRef, int value ) { faceCount[ themeRef ] = value; }
  int getFaceCount( std::string themeRefName );

  inline void addTextureName(int themeRef, int face, const char *name) { 
    if( themeRef < 0 || themeRef > THEME_REF_COUNT ) {
      std::cerr << "*** Error: theme ref is out of bounds: theme=" << getName() << std::endl;
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

  GLuint *getTextureGroup( std::string themeRefName );
  inline char *getName() { return name; }
  void load();
  void unload();

 protected:
  void loadTextureGroup( int ref, int face, char *texture );
  void debug();
};
  

#define MAX_SYSTEM_TEXTURE_COUNT 1000

class Shapes {
public:
  enum {
  STENCIL_SIDE=0,
  STENCIL_U,
  STENCIL_ALL,
  STENCIL_OUTSIDE_TURN,
  STENCIL_SIDES,

  STENCIL_COUNT
};

protected:
	Session *session;
  //bool headless; -=K=- it was left uninitialized but value was still used at some places.
  GLShape *shapes[512];
  std::map<std::string, GLShape *> shapeMap;
  int shapeCount;
  
  struct Texture {
    GLuint id;
    std::string filename;
  };

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
  std::vector<std::vector<std::string>*> descriptions;
	std::map<std::string,int> descriptionIndex;

  // temp. shape data
  std::vector<ShapeValues*> shapeValueVector;

  // md2 data (male/female)
  std::vector<CharacterModelInfo*> character_models[2];

  WallTheme *themes[100], *caveThemes[100];
  WallTheme *allThemes[100];
  int themeCount, allThemeCount, caveThemeCount;
  WallTheme *currentTheme;
  std::vector<GLShape*> themeShapes;
  std::vector<std::string> themeShapeRef;

  // cursor
  SDL_Surface *cursor, *crosshair, *attackCursor, *talkCursor, *useCursor, *forbiddenCursor, *rangedCursor, *moveCursor;
  GLubyte *cursorImage, *crosshairImage, *attackImage, *talkImage, *useImage, *forbiddenImage, *rangedImage, *moveImage;
  GLuint cursor_texture, crosshair_texture, attack_texture, talk_texture, use_texture, forbidden_texture, ranged_texture, move_texture;
  GLuint ripple_texture, torchback;

  // stencils for lava
  SDL_Surface *stencil[ STENCIL_COUNT ];
  GLubyte *stencilImage[ STENCIL_COUNT ];

  GLuint areaTex;

	std::vector<GLuint> rugs;
	char cursorDir[255];
	int cursorWidth, cursorHeight;
	GLuint cursorTexture[ Constants::CURSOR_COUNT ];

	GLuint selection;

	static bool debugFileLoad;

public: 
  Shapes( Session *session );
  virtual ~Shapes();

	inline bool isHeadless() { return session->getGameAdapter()->isHeadless(); }

	inline int getRugCount() { return rugs.size(); }
	inline GLuint getRug( int index ) { return rugs[ index ]; }
	inline GLuint getRandomRug() { if ( rugs.empty() ) return ~0; return getRug( Util::dice( getRugCount() ) ); }

  inline SDL_Surface *getStencilSurface( int index ) { return stencil[ index ]; }
  inline GLubyte *getStencilImage( int index ) { return stencilImage[ index ]; }
  
  inline int getThemeCount() { return themeCount; }
  inline char *getThemeName( int index ) { return themes[ index ]->getName(); }

  inline int getAllThemeCount() { return allThemeCount; }
  inline char *getAllThemeName( int index ) { return allThemes[ index ]->getName(); }
  inline bool isThemeSpecial( int index ) { return allThemes[ index ]->isSpecial(); }

  inline const char *getCurrentThemeName() { return (const char*)currentTheme->getName(); }
  inline WallTheme* getCurrentTheme() { return currentTheme; }

  virtual void initialize();
  GLuint loadSystemTexture( const std::string& line );

  inline int getCharacterModelInfoCount( int sex ) { return character_models[sex].size(); }
  inline CharacterModelInfo *getCharacterModelInfo(  int sex, int index ) { return character_models[sex][ index ]; }

  void loadTheme( WallTheme *theme );
  void loadTheme( const char *name );
  void loadRandomTheme();
  void loadRandomCaveTheme();
	void loadCaveTheme( char *name );
	void loadDebugTheme();

  inline GLuint getTexture(int index) { return textures[index].id; }

  // 1-based!
  inline GLShape *getShape(int index) { return shapes[index]; }
  inline int getShapeCount() { return shapeCount; }

  inline std::map<std::string, GLShape *> *getShapeMap() { return &shapeMap; }

  inline Sint16 getUnitSide() { return unitSide; }
  inline Sint16 getUnitOffset() { return unitOffset; }
  inline Sint16 getWallHeight() { return wallHeight; }

  inline GLuint getRippleTexture() { return ripple_texture; }
  inline GLuint getAreaTexture() { return areaTex; }

  GLuint findTextureByName(const std::string& filename, bool loadIfMissing=false );
  GLShape *findShapeByName(const char *name, bool variation=false);
  int findShapeIndexByName(const char *name);
  
  char const* getRandomDescription(int descriptionGroup);

  GLuint loadGLTextures(const std::string& fileName);

  GLuint getBMPData( const std::string& filename, TextureData& data, int *width=NULL, int *height=NULL );

  GLuint getCursorTexture( int cursorMode );

  static GLuint loadTextureWithAlpha( std::string& fileName, int r=0, int b=0, int g=0, bool isAbsPath=false, bool swapImage=false, bool grayscale=false );
  static GLuint loadAlphaTexture( std::string& filename, int *width=NULL, int *height=NULL );

	inline int getCursorWidth() { return cursorWidth; }
	inline int getCursorHeight() { return cursorHeight; }

	inline GLuint getSelection() { return selection; }

protected:
	static Shapes *instance;
  GLuint loadGLTextureBGRA(SDL_Surface *surface, GLubyte *image, int gl_scale=GL_NEAREST);
  GLuint loadGLTextureBGRA(int w, int h, GLubyte *image, int gl_scale=GL_NEAREST);
  void setupAlphaBlendedBMP(const std::string& filename, SDL_Surface*& surface, GLubyte*& image, int red=0, int green=0, int blue=0, bool isAbsFile=false, bool swapImage=false, bool grayscale=false );
  void setupAlphaBlendedBMPGrid(const std::string& filename, SDL_Surface **surface, GLubyte *tilesImage[20][20], int imageWidth, int imageHeight, int tileWidth, int tileHeight, int red=0, int green=0, int blue=0, int nred=-1, int ngreen=-1, int nblue=-1);
  void swap(unsigned char & a, unsigned char & b);
  void loadStencil( const std::string& filename, int index );
	void loadCursors();
	GLuint *findOrMakeTextureGroup( char *s );
	void setupPNG( const std::string& filename, SDL_Surface*& surface, GLubyte*& image, bool isAbsPath=false, bool hasAlpha=true );
	void setupImage( const std::string &filename, SDL_Surface*& surface, GLubyte*& image );
};

#endif