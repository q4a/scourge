/***************************************************************************
                      shapes.h  -  Shape/texture loader
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
#pragma once

#include "render.h"
#include <string>
#include <vector>
#include <map>
#include "modelwrapper.h"
#include "../session.h" // -=K=-: can't declare inline bool Shapes::isHeadless() without
#include "texture.h"

/**
  *@author Gabor Torok
  */

class GLShape;
class GLTorch;
class Shapes;
class Session;

/// Defines where a static shape can occur.
struct Occurs {
	bool rooms_only;
	int max_count;
	char placement[100];
	char use_function[255];
	char theme[255];
};

/// Temporary information when constructing shapes from a file.
/// -=K=-: turning that struct into class, (complex members, so its class, not PODS)
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
	bool ignoreHeightMap;
	float outdoorsWeight;
	bool outdoorShadow;
	bool wind;
	Occurs occurs;
	int iconRotX;
	int iconRotY;
	int iconRotZ;
	Texture icon;
	int iconWidth, iconHeight;
	std::string ambient;
	int lighting;
	float base_w, base_h;
	char refs[100];
	bool draws;
	bool roof;
	bool noFloor;
	bool usesAlpha;
};

/// Basic info about a character model.
struct CharacterModelInfo {
	char model_name[100];
	char skin_name[300];
	float scale;
};

#define MAX_TEXTURE_COUNT 10

/// A level theme (custom textures for walls and floors).
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

	// outdoor theme sections
	enum {
		OUTDOOR_THEME_REF_GRASS,
		OUTDOOR_THEME_REF_STREET,
		OUTDOOR_THEME_REF_STREET_CROSS,
		OUTDOOR_THEME_REF_STREET_END,
		OUTDOOR_THEME_REF_TRAIL,
		OUTDOOR_THEME_REF_TRAIL_TURN,
		OUTDOOR_THEME_REF_TRAIL_END,
		OUTDOOR_THEME_REF_WATER,
		OUTDOOR_THEME_REF_ROCK,
		OUTDOOR_THEME_REF_GRASS_EDGE,
		OUTDOOR_THEME_REF_GRASS_CORNER,
		OUTDOOR_THEME_REF_GRASS_TIP,
		OUTDOOR_THEME_REF_GRASS_NARROW,
		OUTDOOR_THEME_REF_SNOW,
		OUTDOOR_THEME_REF_SNOW_BIG,
		OUTDOOR_THEME_REF_LAKEBED,
		OUTDOOR_THEME_REF_EXTRA,
		OUTDOOR_THEME_REF_STREET_90,
		OUTDOOR_THEME_REF_STREET_END_90,
		OUTDOOR_THEME_REF_STREET_END_180,
		OUTDOOR_THEME_REF_STREET_END_270,

		// must be the last one
		OUTDOOR_THEME_REF_COUNT
	};
	static char outdoorThemeRefName[OUTDOOR_THEME_REF_COUNT][40];

	static const int MULTI_TEX_COUNT = 2;

private:
	static const int NAME_LENGTH = 40;
	char name[80];
	char textureNames[THEME_REF_COUNT][MAX_TEXTURE_COUNT][NAME_LENGTH]; // holds the text of a theme
	Texture textureGroup[THEME_REF_COUNT][MAX_TEXTURE_COUNT];
	int faceCount[THEME_REF_COUNT];
	std::map<std::string, Texture> loadedTextures;
	std::map<std::string, int> themeRefMap;
	GLfloat r[MULTI_TEX_COUNT], g[MULTI_TEX_COUNT], b[MULTI_TEX_COUNT], intensity[MULTI_TEX_COUNT];
	bool smooth[MULTI_TEX_COUNT];
	Shapes *shapePal;
	bool special;
	bool cave;
	// unused: TextureData lavaData;
	TextureData floorData;
	bool hasOutdoor;
	int outdoorTextureWidth[OUTDOOR_THEME_REF_COUNT];
	int outdoorTextureHeight[OUTDOOR_THEME_REF_COUNT];
	char outdoorTextures[OUTDOOR_THEME_REF_COUNT][MAX_TEXTURE_COUNT][NAME_LENGTH];
	Texture outdoorTextureGroup[OUTDOOR_THEME_REF_COUNT][MAX_TEXTURE_COUNT];
	int outdoorFaceCount[OUTDOOR_THEME_REF_COUNT];
	std::map<std::string, int> outdoorThemeRefMap;
	std::vector<std::string> altWallThemes;

public:
	WallTheme( char const* name, Shapes *shapePal );
	~WallTheme();

	void addAltWallTheme( char *p ) {
		std::string s = p; altWallThemes.push_back( s );
	}
	inline TextureData& getFloorData() {
		return floorData;
	}

	inline void setSpecial( bool b ) {
		special = b;
	}
	inline bool isSpecial() {
		return special;
	}

	inline void setCave( bool b ) {
		cave = b;
	}
	inline bool isCave() {
		return cave;
	}

	inline void setFaceCount( int themeRef, int value ) {
		faceCount[ themeRef ] = value;
	}
	int getFaceCount( std::string themeRefName );

	inline void addTextureName( int themeRef, int face, const char *name ) {
		if ( themeRef < 0 || themeRef > THEME_REF_COUNT ) {
			std::cerr << "*** Error: theme ref is out of bounds: theme=" << getName() << std::endl;
		} else {
			strncpy( textureNames[themeRef][face], name, NAME_LENGTH - 1 );
			textureNames[themeRef][face][NAME_LENGTH - 1] = '\0';
			/*
			cerr <<
			  "\ttheme: " << getName() <<
			  " texture: ref=" << themeRef <<
			  " name=" << name << endl;
			*/
		}
	}

	inline void addOutdoorTextureName( int outdoorThemeRef, int face, const char *name ) {
		if ( outdoorThemeRef < 0 || outdoorThemeRef > OUTDOOR_THEME_REF_COUNT ) {
			std::cerr << "*** Error: outdoor theme ref is out of bounds: theme=" << getName() << std::endl;
		} else {
			strncpy( outdoorTextures[outdoorThemeRef][face], name, NAME_LENGTH - 1 );
			outdoorTextures[outdoorThemeRef][face][NAME_LENGTH - 1] = '\0';
		}
	}

	inline void setOutdoorTextureDimensions( int outdoorThemeRef, int w, int h ) {
		if ( outdoorThemeRef < 0 || outdoorThemeRef > OUTDOOR_THEME_REF_COUNT ) {
			std::cerr << "*** Error: outdoor theme ref is out of bounds: theme=" << getName() << std::endl;
		} else {
			outdoorTextureWidth[outdoorThemeRef] = w;
			outdoorTextureHeight[outdoorThemeRef] = h;
		}
	}

	inline int getOutdoorTextureWidth( int ref ) {
		return outdoorTextureWidth[ ref ];
	}

	inline int getOutdoorTextureHeight( int ref ) {
		return outdoorTextureHeight[ ref ];
	}

	Texture* getOutdoorTextureGroup( int ref ) {
		return outdoorTextureGroup[ ref ];
	}

	inline void setOutdoorFaceCount( int themeRef, int value ) {
		outdoorFaceCount[ themeRef ] = value;
	}
	int getOutdoorFaceCount( std::string themeRefName );
	inline int getOutdoorFaceCount( int ref ) {
		return outdoorFaceCount[ ref ];
	}
	inline void setHasOutdoor( bool b ) {
		this->hasOutdoor = b;
	}
	inline bool getHasOutdoor() {
		return this->hasOutdoor;
	}

	inline void setMultiTexRed( int index, GLfloat value ) {
		r[index] = value;
	}
	inline void setMultiTexGreen( int index, GLfloat value ) {
		g[index] = value;
	}
	inline void setMultiTexBlue( int index, GLfloat value ) {
		b[index] = value;
	}
	inline void setMultiTexInt( int index, GLfloat value ) {
		intensity[index] = value;
	}
	inline void setMultiTexSmooth( int index, bool value ) {
		smooth[index] = value;
	}

	inline GLfloat getMultiTexRed( int index ) {
		return r[index];
	}
	inline GLfloat getMultiTexGreen( int index ) {
		return g[index];
	}
	inline GLfloat getMultiTexBlue( int index ) {
		return b[index];
	}
	inline GLfloat getMultiTexInt( int index ) {
		return intensity[index];
	}
	inline bool getMultiTexSmooth( int index ) {
		return smooth[index];
	}

	Texture* getTextureGroup( std::string themeRefName );
	inline char *getName() {
		return name;
	}
	void load();
	void unload();

protected:
	void loadTextureGroup( int ref, int face, char *texture, bool outdoor = false );
	void createOutdoorEdgeTexture( int ref );
	void debug();
};


#define MAX_SYSTEM_TEXTURE_COUNT 1000

class Shapes {
public:
	enum {
		STENCIL_SIDE = 0,
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

	// TODO: can be probably replaced by Texture class itself
	struct ExtraTexture {
		Texture texture;
		std::string filename;
	};

	ExtraTexture textures[ MAX_SYSTEM_TEXTURE_COUNT ]; // store textures
	int texture_count;
	GLShape *shapeNameArray[256];

	// native texture groups
	Texture textureGroup[256][3];
	int textureGroupCount;

	GLuint md2_tex[6];

	// Info about the texture most recently loaded by loadTexture().
	// unused: int lastTextureWidth, lastTextureHeight;
	// unused: bool lastTextureAlpha;

	// how big to make the walls
	const static Sint16 unitSide = MAP_UNIT;
	const static Sint16 unitOffset = MAP_UNIT_OFFSET;
	const static Sint16 wallHeight = MAP_WALL_HEIGHT;

	// shape descriptions
	std::vector<std::vector<std::string>*> descriptions;
	std::map<std::string, int> descriptionIndex;

	// temp. shape data
	std::vector<ShapeValues*> shapeValueVector;

	// md2 data (male/female)
	std::vector<CharacterModelInfo*> character_models[2];

	WallTheme *themes[100], *caveThemes[100];
	std::vector<WallTheme*> outdoorThemes;
	WallTheme *allThemes[100];
	int themeCount, allThemeCount, caveThemeCount;
	WallTheme *currentTheme;
	std::vector<std::string> themeShapes;
	std::vector<std::string> themeShapeRef;

	// cursor
	SDL_Surface *cursor, *crosshair, *attackCursor, *talkCursor, *useCursor, *forbiddenCursor, *rangedCursor, *moveCursor;
	GLubyte *cursorImage, *crosshairImage, *attackImage, *talkImage, *useImage, *forbiddenImage, *rangedImage, *moveImage;
	Texture cursor_texture, crosshair_texture, attack_texture, talk_texture, use_texture, forbidden_texture, ranged_texture, move_texture;
	Texture ripple_texture, torchback;

	// stencils for lava
	SDL_Surface *stencil[ STENCIL_COUNT ];
	GLubyte *stencilImage[ STENCIL_COUNT ];

	Texture areaTex;

	std::vector<Texture> rugs;
	char cursorDir[255];
	int cursorWidth, cursorHeight;
	Texture cursorTexture[ Constants::CURSOR_COUNT ];

	Texture selection;

	static bool debugFileLoad;

public:
	Shapes( Session *session );
	virtual ~Shapes();

	inline Session *getSession() {
		return this->session;
	}
	inline bool isHeadless() {
		return session->getGameAdapter()->isHeadless();
	}

	inline int getRugCount() {
		return rugs.size();
	}
	inline Texture const& getRug( int index ) {
		return rugs[ index ];
	}
	inline Texture const& getRandomRug() {
		if ( rugs.empty() ) return Texture::none(); 
		return getRug( Util::dice( getRugCount() ) );
	}

	inline SDL_Surface *getStencilSurface( int index ) {
		return stencil[ index ];
	}
	inline GLubyte *getStencilImage( int index ) {
		return stencilImage[ index ];
	}

	inline int getThemeCount() {
		return themeCount;
	}
	inline char *getThemeName( int index ) {
		return themes[ index ]->getName();
	}

	inline int getAllThemeCount() {
		return allThemeCount;
	}
	inline char *getAllThemeName( int index ) {
		return allThemes[ index ]->getName();
	}
	inline bool isThemeSpecial( int index ) {
		return allThemes[ index ]->isSpecial();
	}

	inline const char *getCurrentThemeName() {
		return ( const char* )currentTheme->getName();
	}
	inline WallTheme* getCurrentTheme() {
		return currentTheme;
	}

	virtual void initialize();
	Texture const& loadSystemTexture( const std::string& line );

	inline int getCharacterModelInfoCount( int sex ) {
		return character_models[sex].size();
	}
	inline CharacterModelInfo *getCharacterModelInfo(  int sex, int index ) {
		return character_models[sex][ index ];
	}

	void loadTheme( WallTheme *theme );
	void loadTheme( const char *name );
	void loadRandomTheme();
	void loadRandomOutdoorTheme();
	void loadRandomCaveTheme();
	void loadCaveTheme( char *name );
	void loadDebugTheme();

	inline Texture getTexture( int index ) {
		return textures[index].texture;
	}

	// 1-based!
	GLShape *getShape( int index );
	inline int getShapeCount() {
		return shapeCount;
	}

	inline std::map<std::string, GLShape *> *getShapeMap() {
		return &shapeMap;
	}

	inline Sint16 getUnitSide() {
		return unitSide;
	}
	inline Sint16 getUnitOffset() {
		return unitOffset;
	}
	inline Sint16 getWallHeight() {
		return wallHeight;
	}

	inline Texture getRippleTexture() {
		return ripple_texture;
	}
	inline Texture getAreaTexture() {
		return areaTex;
	}

	Texture const& findTextureByName( const std::string& filename, bool loadIfMissing = false );
	GLShape *findShapeByName( const char *name );
	int findShapeIndexByName( const char *name );
	void getShapeDimensions( const char *name, int *w, int *d, int *h );

	char const* getRandomDescription( int descriptionGroup );

	//GLuint loadGLTextures(const std::string& fileName, bool isSprite = false);

	GLuint getBMPData( const std::string& filename, TextureData& data, int *width = NULL, int *height = NULL );

	Texture const& getCursorTexture( int cursorMode );

	// unused: GLuint loadTexture( const std::string& filename, bool absolutePath = false, bool isSprite = true, bool anisotropy = false );

	inline int getCursorWidth() {
		return cursorWidth;
	}
	inline int getCursorHeight() {
		return cursorHeight;
	}

	inline Texture getSelection() {
		return selection;
	}

	// unused: GLuint createAlphaTexture( GLuint alphaTex, GLuint sampleTex, int textureSizeW = 256, int textureSizeH = 256, int width = 256, int height = 256 );

	void loadAllShapes();

protected:
	static Shapes *instance;
	void loadShape( const char *name );
	ShapeValues *getShapeValueByName( const char *name, int *index );
	// unused: GLuint createTileTexture( SDL_Surface **surface, int tileX, int tileY, int tileWidth, int tileHeight );
	void loadTiles( const std::string& filename, SDL_Surface **surface );
	void swap( unsigned char & a, unsigned char & b );
	void loadStencil( const std::string& filename, int index );
	void loadCursors();
	Texture* findOrMakeTextureGroup( char *s );
	DECLARE_NOISY_OPENGL_SUPPORT();
};

#endif
