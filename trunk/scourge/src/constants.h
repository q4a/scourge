/***************************************************************************
                          constants.h  -  description
                             -------------------
    begin                : Sun Oct 12 2003
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

#ifndef CONSTANTS_H
#define CONSTANTS_H

// include sdl, opengl and glut
#include <SDL.h>
#include <SDL_opengl.h>
#include <SDL_endian.h>
#if defined(__APPLE__) || defined(__MACH_O__)
#include <GLUT/glut.h>
#else
#include <GL/glut.h>
#ifndef WIN32
// Could not get these to include on my Mandrake9 box...
#define APIENTRY
typedef void (APIENTRY * PFNGLACTIVETEXTUREARBPROC) (GLenum texture);
typedef void (APIENTRY * PFNGLMULTITEXCOORD2FARBPROC) (GLenum target, GLfloat s, GLfloat t);
typedef void (APIENTRY * PFNGLMULTITEXCOORD2IARBPROC) (GLenum target, GLint s, GLint t);
#endif
#endif
#include <stdlib.h>
#include <math.h>
#include <vector.h>

using namespace std;

#ifdef WIN32
#define SEPARATOR '\\'
#else
#define SEPARATOR '/'
#endif

#define SCOURGE_VERSION 0.4

// opengl extension function ptrs for SDL (set in sdlhandler.cpp)
extern PFNGLACTIVETEXTUREARBPROC glSDLActiveTextureARB;
extern PFNGLMULTITEXCOORD2FARBPROC glSDLMultiTexCoord2fARB;
extern PFNGLMULTITEXCOORD2IARBPROC glSDLMultiTexCoord2iARB;

// some windows versions of opengl don't have this
#define GL_BGR                                  0x80E0
#define GL_BGRA                                 0x80E1

// show fps info
#define SHOW_DEBUG_INFO

/*
  Float swapping code by:
  Ramin Firoozye' -- rp&A Inc.
  1995/06/30 found on Google groups
 */

/*
 * Helpful macros in swapping bytes etc...
 * This union lets  us map just about any common datatype onto the
 * specified value, so we can use direct array access to do byte, word,
 * or long swapping.
 */
union NetValue
{
  short s;  /* Straight 2-byte short */
  unsigned short sU; /* unsigned short */
  unsigned char sC[2]; /* short as bytes */

  long l;  /* Straight 4-byte long */
  unsigned long lU; /* unsigned long */
  unsigned char lC[4]; /* long as bytes */
  unsigned short lS[2]; /* long as short */

  float f;  /* Straight (presumed) 4-byte single */
  unsigned char fC[4]; /* single as bytes */
  unsigned short fS[2]; /* single as short */

  double g;  /* Straight (presumed) 8-byte double */
  unsigned char gC[8]; /* double as bytes */
  unsigned short gS[4]; /* double as short */
  unsigned long gL[2]; /* double as long */
};
typedef union NetValue NetValue;

/* 
 * We define macros as mSWAPn where m is one of B=Byte, W=word and L=long
 * and n is one of B=byte, W=word, L=long, F=single float, 
 * and G=double float. We are swapping n in chunks of m. So WSWAPL is
 * a word-swap of a longword (i.e. we swap the first and second words).
 * and BSWAPG is a byte-swap of a double (i.e. total reversal of byte order
 * across all 8 bytes). For float and double values, we don't do bit
 * swapping of any sort (just B, W, and L). So if the network format of
 * the floating value is too damn obscure, something more complicated
 * has to be done...
 */

#define BSWAPW(src, dst) { NetValue _t; _t.s = src; \
 ((char *)&dst)[0] = _t.sC[1]; ((char *)&dst)[1] = _t.sC[0]; \
}
#define BSWAPL(src, dst) { NetValue _t; _t.lU = src; \
 ((char *)&dst)[0] = _t.lC[3]; ((char *)&dst)[1] = _t.lC[2]; \
 ((char *)&dst)[2] = _t.lC[1]; ((char *)&dst)[3] = _t.lC[0]; \
}
#define WSWAPL(src, dst) { NetValue _t; _t.lU = src; \
 ((short *)&dst)[0] = _t.lS[1]; ((short *)&dst)[1] = _t.lS[0]; \
}
#define BSWAPF(src, dst) { NetValue _t; _t.f = src; \
 ((char *)&dst)[0] = _t.fC[3]; ((char *)&dst)[1] = _t.fC[2]; \
 ((char *)&dst)[2] = _t.fC[1]; ((char *)&dst)[3] = _t.fC[0]; \
}
#define WSWAPF(src, dst) { NetValue _t; _t.f = src; \
 ((short *)&dst)[0] = _t.fS[1]; ((short *)&dst)[1] = _t.fS[0]; \
}
#define BSWAPG(src, dst) { NetValue _t; _t.g = src; \
 ((char *)&dst)[0] = _t.gC[7]; ((char *)&dst)[1] = _t.gC[6]; \
 ((char *)&dst)[2] = _t.gC[5]; ((char *)&dst)[3] = _t.gC[4]; \
 ((char *)&dst)[4] = _t.gC[3]; ((char *)&dst)[5] = _t.gC[2]; \
 ((char *)&dst)[6] = _t.gC[1]; ((char *)&dst)[7] = _t.gC[0]; \
}
#define WSWAPG(src, dst) { NetValue _t; _t.g = src; \
 ((short *)&dst)[0] = _t.gS[3]; ((short *)&dst)[1] = _t.gS[2]; \
 ((short *)&dst)[2] = _t.gS[1]; ((short *)&dst)[3] = _t.gS[0]; \
}
#define LSWAPG(src, dst) { NetValue _t; _t.g = src; \
 ((long *)&dst)[0] = _t.gL[1]; ((long *)&dst)[1] = _t.gL[0]; \
}

// GL color in float
typedef struct _Color {
  float r, g, b;
} Color;

/**
  *@author Gabor Torok
  */

extern char rootDir[300];

typedef struct _ParticleStruct {
  GLfloat x, y, z;
  GLint height;
  int life;
} ParticleStruct;

class Constants {
public:

  // creature movement
  enum motion {
    MOTION_MOVE_TOWARDS=0,
    MOTION_MOVE_AWAY, // flee
    MOTION_LOITER
  };

// This stores the speed of the animation between each key frame for md2 models
// A higher value means a *faster* animation and NOT a *smoother* animation.
// The smoothing of the animation is only determined by fps. 
// So this value should not be modified. Maybe later there will be an
// animation_speed for each creature, to give the feeling some are faster than others ?
#define ANIMATION_SPEED         5.0f  

// The map's dimensions
#define MAP_WIDTH 600
#define MAP_DEPTH 600

// How big is the on-screen view. Should be calculated.
#define MAP_VIEW_WIDTH 100
#define MAP_VIEW_DEPTH 110
#define MAP_VIEW_HEIGHT 16

  // How big is 1 map chunk
#define MAP_UNIT 16
#define MAP_UNIT_OFFSET 2
#define MAP_WALL_HEIGHT 12

  // How far from the edge to start drawing in map
#define MAP_OFFSET 55
  
// define some active region labels
  enum {
	INV_PLAYER_0 = 0,
	INV_PLAYER_1,
	INV_PLAYER_2,
	INV_PLAYER_3,
	INV_MODE_INVENTORY,
	INV_MODE_PROPERTIES,
	INV_MODE_SPELLS,
	INV_MODE_LOG,
	MENU_0, 
	MENU_1,
	MENU_2,
	MENU_3,
	MENU_4,
	ESCAPE,
	SHOW_INVENTORY,
	SHOW_OPTIONS,
	SKILL_LIST,
	ITEM_LIST,
	DIAMOND_FORMATION,
	STAGGERED_FORMATION,
	SQUARE_FORMATION,
	ROW_FORMATION,
	SCOUT_FORMATION,
	CROSS_FORMATION,
	PLAYER_1,
	PLAYER_2,
	PLAYER_3,
	PLAYER_4,
	PLAYER_ONLY,
	MOVE_ITEM_TO_PLAYER_0,
	MOVE_ITEM_TO_PLAYER_1,
	MOVE_ITEM_TO_PLAYER_2,
	MOVE_ITEM_TO_PLAYER_3,
	DROP_ITEM,
	EQUIP_ITEM,
	FIX_ITEM,
	ENCHANT_ITEM,
	REMOVE_CURSE_ITEM,
	COMBINE_ITEM,
	IDENTIFY_ITEM


  };

  // Directions (a bitfield so they can be combined)
  static const Uint16 MOVE_UP = 1;
  static const Uint16 MOVE_DOWN = 2;
  static const Uint16 MOVE_LEFT = 4;
  static const Uint16 MOVE_RIGHT = 8;

  enum { NORTH=0, EAST, SOUTH, WEST };

  // messages
  enum {
	WELCOME=0,
	ITEM_OUT_OF_REACH,
	DOOR_BLOCKED,
	SINGLE_MODE,
	GROUP_MODE,
	TURN_MODE,
	REAL_TIME_MODE,
	CLOSE_LABEL,
	DROP_ITEM_LABEL,
	OPEN_CONTAINER_LABEL,
	EXPLAIN_DRAG_AND_DROP,
	PLAY_MISSION_LABEL,

	// last one
	MESSAGE_COUNT
  };
  static char *messages[][80];
  static int messageCount[];

  enum { 
	EW_WALL_INDEX=1,
	EW_WALL_EXTRA_INDEX,
	EW_WALL_TWO_EXTRAS_INDEX,  		
	NS_WALL_INDEX,
	NS_WALL_EXTRA_INDEX,
	NS_WALL_TWO_EXTRAS_INDEX,  				
	CORNER_INDEX,
	DOOR_SIDE_INDEX,		
	EW_DOOR_INDEX,
	EW_DOOR_TOP_INDEX,
	NS_DOOR_INDEX,
	NS_DOOR_TOP_INDEX,
	FLOOR_TILE_INDEX,
	ROOM_FLOOR_TILE_INDEX,
	LAMP_NORTH_INDEX,
	LAMP_SOUTH_INDEX,
	LAMP_WEST_INDEX,
	LAMP_EAST_INDEX,
	LAMP_BASE_INDEX,  
	DEBUG_INDEX, 
	LOCATOR_INDEX, 
	BOARD_INDEX,
	BRAZIER_INDEX,
	BRAZIER_BASE_INDEX,
	COLUMN_INDEX,
	
	// must be the last one
	SHAPE_INDEX_COUNT
  };

  enum {
	FIGHTER_INDEX = 0,
	ROGUE_INDEX,
	CLERIC_INDEX,
	WIZARD_INDEX,

	BUGGERLING_INDEX,
	SLIME_INDEX,

	// last one
	CREATURE_INDEX_COUNT
  };
  
  enum {
	SWORD_INDEX = 0,
	AXE_INDEX,
	BOOKSHELF_INDEX,
	CHEST_INDEX,
	BOOKSHELF2_INDEX,
	CHEST2_INDEX,
	CORPSE_INDEX,
	TABLE_INDEX,
	CHAIR_INDEX,

	// should be the last one
	ITEM_INDEX_COUNT
  };

  enum {
	SWORD_WEAPON = 0,
	AXE_WEAPON,
	BOW_WEAPON,
	HAND_TO_HAND_COMBAT,

	SPEED,
	COORDINATION,
	POWER,
	IQ,
	LEADERSHIP,
	LUCK,
	PIETY,
	LORE,

	SHIELD_DEFEND,
	ARMOR_DEFEND,
	WEAPON_DEFEND,

	MATERIAL_SPELL,
	ILLUSION_SPELL,
	PSYCHIC_SPELL,

	OPEN_LOCK,
	FIND_TRAP,
	MOVE_UNDETECTED,

	SKILL_0, SKILL_1, SKILL_2, SKILL_3, SKILL_4, SKILL_5, SKILL_6, SKILL_7, SKILL_8, SKILL_9,
	
	SKILL_COUNT
  };
  static const char *SKILL_NAMES[];

  enum { 
	blessed, 
	empowered, 
	enraged, 
	ac_protected, 
	magic_protected, 
	drunk, 
	poisoned, 
	cursed, 
	possessed, 
	blinded, 
	charmed, 
	changed,
	dead,
	
	// must be last
	STATE_MOD_COUNT
  };
  static const char *STATE_NAMES[];

  // special effect names
  enum {
	EFFECT_FLAMES=0,
	EFFECT_GLOW
  };    

  static const int DAMAGE_DURATION = 500;
  
  // glColor for texts
  enum {
    RED_COLOR=0,
    BLUE_COLOR,
    YELLOW_COLOR,
    DEFAULT_COLOR // must be last for textColor[][]
  };  
  //static float textColor[][4]; 
  
  static bool multitexture;	

  enum {
    NO_SHADOWS=0,
    OBJECT_SHADOWS,
    ALL_SHADOWS
  };

  // the speed when hand fighting is used instead of a weapon
  static const int HAND_WEAPON_SPEED = 10;

  Constants();
  ~Constants();

  static char *getMessage(int index);
};

// This is our 3D point class.  This will be used to store the vertices of our model.
class CVector3 
{
public:
    float x, y, z;
};

// This is our 2D point class.  This will be used to store the UV coordinates.
class CVector2 
{
public:
    float x, y;
};

// Math functions
extern CVector3 Vector(CVector3 vPoint1, CVector3 vPoint2);

// This adds 2 vectors together and returns the result
extern CVector3 AddVector(CVector3 vVector1, CVector3 vVector2);

// This divides a vector by a single number (scalar) and returns the result
extern CVector3 DivideVectorByScaler(CVector3 vVector1, float Scaler);

// This returns the cross product between 2 vectors
extern CVector3 Cross(CVector3 vVector1, CVector3 vVector2);
extern CVector3 Normalize(CVector3 vNormal);

// This file includes all of the model structures that are needed to load
// in a .Md2 file.  When it comes to skeletal animation, we need to add quite
// a bit more variables to these structures.  Not all of the data will be used
// because Quake2 models don't have such a need.  I decided to keep the structures
// the same as the rest of the model loaders on our site so that we could eventually
// use a base class in the future for a library.
//
typedef unsigned char BYTE;
#define MAX_TEXTURES 100								// The maximum amount of textures to load

// This is our face structure.  This is is used for indexing into the vertex
// and texture coordinate arrays.  From this information we know which vertices
// from our vertex array go to which face, along with the correct texture coordinates.
struct tFace
{
	int vertIndex[3];			// indicies for the verts that make up this triangle
	int coordIndex[3];			// indicies for the tex coords to texture this face
};

// This holds the information for a material.  It may be a texture map of a color.
// Some of these are not used, but I left them because you will want to eventually
// read in the UV tile ratio and the UV tile offset for some models.
struct tMaterialInfo
{
	char  strName[255];			// The texture name
	char  strFile[255];			// The texture file name (If this is set it's a texture map)
	BYTE  color[3];				// The color of the object (R, G, B)
	int   texureId;				// the texture ID
	float uTile;				// u tiling of texture  (Currently not used)
	float vTile;				// v tiling of texture	(Currently not used)
	float uOffset;			    // u offset of texture	(Currently not used)
	float vOffset;				// v offset of texture	(Currently not used)
} ;

// This holds all the information for our model/scene.
// You should eventually turn into a robust class that
// has loading/drawing/querying functions like:
// LoadModel(...); DrawObject(...); DrawModel(...); DestroyModel(...);
struct t3DObject
{
	int  numOfVerts;			// The number of verts in the model
	int  numOfFaces;			// The number of faces in the model
	int  numTexVertex;			// The number of texture coordinates
	int  numGlCommands;         // The number of glCommands
	int  materialID;			// The texture ID to use, which is the index into our texture array
	bool bHasTexture;			// This is TRUE if there is a texture map for this object
	char strName[255];			// The name of the object
	CVector3  *pVerts;			// The object's vertices
	CVector3  *pNormals;		// The object's normals
	CVector2  *pTexVerts;		// The texture's UV coordinates
	tFace *pFaces;				// The faces information of the object
	int *pGlCommands;           // The glCommands used to draw the model faster
};


// This holds our information for each animation of the Quake model.
// A STL vector list of this structure is created in our t3DModel structure below.
struct tAnimationInfo
{
    char strName[255];          // This stores the name of the animation (Jump, Pain, etc..)
    int startFrame;             // This stores the first frame number for this animation
    int endFrame;               // This stores the last frame number for this animation
};

// We added 4 new variables to our model structure.  These will help us handle
// the current animation.  As of now, the current animation will continue to loop
// from it's start from to it's end frame until we right click and change animations.
struct t3DModel 
{
    int numOfObjects;                   // The number of objects in the model
    int numOfMaterials;                 // The number of materials for the model
    int numOfAnimations;                // The number of animations in this model (NEW)
    int currentAnim;                    // The current index into pAnimations list (NEW)
    int currentFrame;                   // The current frame of the current animation (NEW)
    vector<tAnimationInfo> pAnimations; // The list of animations (NEW)
    vector<tMaterialInfo> pMaterials;   // The list of material information (Textures and colors)
    vector<t3DObject> pObject;          // The object list for our model
};


typedef unsigned char byte;

extern void ComputeNormals(t3DModel *pModel);
extern void CreateTexture(GLuint textureArray[],char *strFileName,int textureID);
extern void swap(unsigned char & a, unsigned char & b);

#endif
