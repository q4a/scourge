/***************************************************************************
                          common/constants.h  -  description
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

// autopackage's binary relocation lib
#include "binreloc.h"

// from tuxracer
#if defined ( __MWERKS__ ) || defined( _MSC_VER ) || defined( WIN32 )
#   define NATIVE_WIN32_COMPILER 1
#else
/* Assume UNIX compatible by default */
#   define COMPILER_IS_UNIX_COMPATIBLE 1
#endif

// include sdl, opengl and glut
#include <SDL.h>
#include <SDL_opengl.h>
#include <SDL_endian.h>

#ifdef HAVE_SDL_NET
#include <SDL_net.h>
#include <SDL_thread.h>
#endif

#ifdef HAVE_SDL_MIXER
#include <SDL_mixer.h>
#endif

#if defined(__APPLE__) || defined(__MACH_O__)
// *** #include <GLUT/glut.h>
#else
// *** #include <GL/glut.h>
#ifndef WIN32
// Could not get these to include on my Mandrake9 box...
#ifndef APIENTRY
#define APIENTRY
#endif
typedef void (APIENTRY * PFNGLACTIVETEXTUREARBPROC) (GLenum texture);
typedef void (APIENTRY * PFNGLMULTITEXCOORD2FARBPROC) (GLenum target, GLfloat s, GLfloat t);
typedef void (APIENTRY * PFNGLMULTITEXCOORD2IARBPROC) (GLenum target, GLint s, GLint t);
#endif
#endif
#include <stdlib.h>
#include <math.h>
#include <vector>
#include <queue>
#include <map>
#include <iostream>
#include <errno.h>
#include <string>

#if defined( COMPILER_IS_UNIX_COMPATIBLE )
#   include <unistd.h>
#   include <pwd.h>
#   include <dirent.h>
#   include <sys/time.h>
#   include <sys/types.h>
#   include <dirent.h>
#   include <sys/stat.h>
#endif

#ifdef WIN32
#define SEPARATOR '\\'
#else
#define SEPARATOR '/'
#endif

/*
  Data and config dirs. Shamelessly borrowed from tuxracer.
 */
#if defined( WIN32 )
#  define CONFIG_DIR "."
#  define CONFIG_FILE "options.txt"
#else
#  define CONFIG_DIR ".scourge"
#  define CONFIG_FILE "options"
#endif /* defined( WIN32 ) */

#define DATA_DIR_NAME "scourge_data"

#ifndef DATA_DIR
#  if defined( WIN32 )
#    define DATA_DIR "./"DATA_DIR_NAME
#  else
#    define DATA_DIR "/usr/local/share/scourge"
#  endif /* defined( WIN32 ) */
#endif

#ifndef assert
  #define assert(x) x;
#endif

#define SCOURGE_VERSION "0.17"
#define MAX_PARTY_SIZE 4

// Max level depth per mission
#define MAX_MISSION_DEPTH 10

#define toint(x) (int)(x<0 ? (x - 0.5f) : (x + 0.5f))

//extern char rootDir[300];
extern char *rootDir;
extern char configDir[300];
extern int get_config_dir_name( char *buff, int len );
extern int get_config_file_name( char *buff, int len );
extern int get_file_name( char *buff, int len, char *fileName );

// opengl extension function ptrs for SDL (set in sdlhandler.cpp)
extern PFNGLACTIVETEXTUREARBPROC glSDLActiveTextureARB;
extern PFNGLMULTITEXCOORD2FARBPROC glSDLMultiTexCoord2fARB;
extern PFNGLMULTITEXCOORD2IARBPROC glSDLMultiTexCoord2iARB;

// some windows versions of opengl don't have this
#define GL_BGR                                  0x80E0
#define GL_BGRA                                 0x80E1


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
class Color {
 public:
  float r, g, b, a;

  Color(): r(0), g(0), b(0), a(0) {}

  Color( float r, float g, float b, float a=1.0f ) {
	this->set( r, g, b, a );
  }

  ~Color() {
  }

  inline void set( float r, float g, float b, float a=1.0f ) {
	this->r = r;
	this->g = g;
	this->b = b;
	this->a = a;
  }

	void Clear() { *this = Color(); }
};

/**
  *@author Gabor Torok
  */

typedef struct _ParticleStruct {
  GLfloat x, y, z, startZ;
  GLint height;
  int life;
  GLfloat moveDelta;
  int maxLife;
  int trail;
  float rotate;
  float zoom;
  bool tail;
  bool untilGround;
  Color tailColor;
} ParticleStruct;

#define SINGLE_TARGET 0
#define GROUP_TARGET 1

#define MAX_INVENTORY_SIZE 200
#define MAX_CONTAINED_ITEMS 100

#define MAX_LEVEL 50

#define MIN_DISTANCE 1.0f

class StatusReport {
public:
  StatusReport() {
  }
  virtual ~StatusReport() {
  }
  virtual void updateStatus( int status, int maxStatus, const char *message=NULL ) = 0;
};

class Constants {
public:

  // inventory locations
  static const int INVENTORY_HEAD = 1;
  static const int INVENTORY_NECK = 2;
  static const int INVENTORY_BACK = 4;
  static const int INVENTORY_CHEST = 8;
  static const int INVENTORY_LEFT_HAND = 16;
  static const int INVENTORY_RIGHT_HAND = 32;
  static const int INVENTORY_BELT = 64;
  static const int INVENTORY_LEGS = 128;
  static const int INVENTORY_FEET = 256;
  static const int INVENTORY_RING1 = 512;
  static const int INVENTORY_RING2 = 1024;
  static const int INVENTORY_RING3 = 2048;
  static const int INVENTORY_RING4 = 4096;
  static const int INVENTORY_WEAPON_RANGED = 8192;
  static const int INVENTORY_GLOVE = 16382;
  static const int INVENTORY_COUNT = 15;
  static char inventory_location[][80];

  static const int MAP_GRID_TILE_WIDTH = 6;
  static const int MAP_GRID_TILE_HEIGHT = 5;
  static const int MAP_GRID_TILE_PIXEL_WIDTH = 256;
  static const int MAP_GRID_TILE_PIXEL_HEIGHT = 256;

  // creature movement
  enum motion {
    MOTION_MOVE_TOWARDS=0,
    MOTION_MOVE_AWAY, // flee
    MOTION_LOITER,
    MOTION_STAND
  };
  
  enum {
  	SEX_MALE=0,
  	SEX_FEMALE
  };

// This stores the speed of the animation between each key frame for md2 models
// A higher value means a *faster* animation and NOT a *smoother* animation.
// The smoothing of the animation is only determined by fps.
// So this value should not be modified. Maybe later there will be an
// animation_speed for each creature, to give the feeling some are faster than others ?
#define ANIMATION_SPEED         5.0f

#define DEFAULT_SERVER_PORT 6543

// The map's dimensions
// Warning: if this ever changes, be sure to look at Map::createTripletKey().
// it assumes that MAP_WIDTH >= MAP_HEIGHT and that MAP_WIDTH^3 < 2^32.
#define MAP_WIDTH 600
#define MAP_DEPTH 600

// How big is the on-screen view. Should be calculated.
#define MAP_VIEW_HEIGHT 16

// How big is 1 map chunk
#define MAP_UNIT 16
#define MAP_UNIT_OFFSET 2
#define MAP_WALL_HEIGHT 12

// How far from the edge to start drawing in map
#define MAP_OFFSET 80

// cave chunk size
#define CAVE_CHUNK_SIZE 8

// The max value of a skill under normal circumstances.
#define MAX_SKILL 100

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
    EXIT_MISSION_LABEL,
    TELEPORT_TO_BASE_LABEL,
    OK_LABEL,
    CANCEL_LABEL,
    YES_LABEL,
    NO_LABEL,
    LEVEL_UP_ERROR,
    OUT_OF_POINTS_ERROR,
    NO_SKILL_ERROR,
    SCOURGE_DIALOG,
    USE_GATE_LABEL,
    DEAD_CHARACTER_ERROR,
    HP_LABEL,
    AC_LABEL,
    SPELL_FAILED_MESSAGE,
    ITEM_ACL_VIOLATION,
    JOIN_SERVER_ERROR,
    CLIENT_CANT_CONNECT_ERROR,
    DOOR_OPENED_CLOSE,
    DOOR_OPENED,
    DOOR_OPENED_FAR,
    DOOR_LOCKED,
    TELEPORTER_OFFLINE,
    INFO_GUI_TITLE,
    DELETE_OLD_SAVED_GAME,
    ITEM_LEVEL_VIOLATION,
    CHANGE_KEY,
    WAITING_FOR_KEY,
    CONVERSATION_GUI_TITLE,
    TRADE_DIALOG_TITLE,
    TRAIN_DIALOG_TITLE,
    HEAL_DIALOG_TITLE,
    DONATE_DIALOG_TITLE,
    UNMET_CAPABILITY_PREREQ_ERROR,
    CANNOT_USE_AUTO_CAPABILITY_ERROR,
    ITEM_TWO_HANDED_VIOLATION,
		TRAINING_AVAILABLE,
		SKILL_POINTS_AVAILABLE,
		LOCKED_DOOR_OPENS_MAGICALLY,
		CAUSE_OF_DEATH,

	// last one
	MESSAGE_COUNT
  };
  static char *messages[][80];
  static int messageCount[];

  static const char *localhost;
  static const char *adminUserName;

  // other things potions can act on:
  enum {
	HP=0,
	MP,
	AC,

	POTION_SKILL_COUNT
  };
  static const char *POTION_SKILL_NAMES[];
  // return -1 on failure, or (-2 - i) on success
  static int getPotionSkillByName(char *p);

  enum {
	blessed=0,
	empowered,
	enraged,
	ac_protected,
	magic_protected,
	drunk,
	poisoned,
	cursed,
	possessed,
	blinded,
	paralysed,
	invisible,
	overloaded,
	dead,
	asleep,

	// must be last
	STATE_MOD_COUNT
  };
  static const char *STATE_NAMES[];
  static const char *STATE_SYMBOLS[];
  // return -1 on failure, 0+ on success
  static int getStateModByName( const char *p );
  static std::vector<int> goodStateMod, badStateMod;
  static int getRandomGoodStateMod();
  static int getRandomBadStateMod();
  static bool isStateModTransitionWanted(int mod, bool setting);
  static void initConstants();

  enum {
    LESSER_MAGIC_ITEM=0,
    GREATER_MAGIC_ITEM,
    CHAMPION_MAGIC_ITEM,
    DIVINE_MAGIC_ITEM
  };
  static const char *MAGIC_ITEM_NAMES[];
  static const Color *MAGIC_ITEM_COLOR[];
  static const Color *SPECIAL_ITEM_COLOR;

  // special effect names
  enum {
    EFFECT_FLAMES=0,
    EFFECT_GLOW,
    EFFECT_TELEPORT,
    EFFECT_GREEN,
    EFFECT_EXPLOSION,
    EFFECT_SWIRL,
    EFFECT_CAST_SPELL,
    EFFECT_RING,
    EFFECT_RIPPLE,
    EFFECT_DUST,
    EFFECT_HAIL,
    EFFECT_TOWER,
    EFFECT_BLAST,

    // must be last
    EFFECT_COUNT
  };
  static const int DAMAGE_DURATION = 500;

  static const char *EFFECT_NAMES[];
  inline static int getEffectByName(char *s) {
	for(int i = 0; i < EFFECT_COUNT; i++)
	  if(!strcmp(s, EFFECT_NAMES[i])) return i;
	return EFFECT_FLAMES;
  }

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
    CURSOR_NORMAL=0,
    CURSOR_CROSSHAIR,
    CURSOR_ATTACK,
    CURSOR_TALK,
    CURSOR_USE,
    CURSOR_FORBIDDEN,
    CURSOR_RANGED,
    CURSOR_MOVE,

		CURSOR_COUNT // must be the last one
  };

	static const char *cursorTextureName[];

  enum {
    SCOURGE_DEFAULT_FONT=0,
    SCOURGE_UI_FONT,
    SCOURGE_MONO_FONT,
    SCOURGE_LARGE_FONT
  };

  enum {
    NO_SHADOWS=0,
    OBJECT_SHADOWS,
    ALL_SHADOWS
  };

  enum {
    ACTION_NO_ACTION=-1,
    ACTION_EAT_DRINK=0,
    ACTION_CAST_SPELL,
    ACTION_SPECIAL,

    // this must be the last one
    ACTION_COUNT
  };

  enum {
    GUI_LAYOUT_ORIGINAL=0,
    GUI_LAYOUT_BOTTOM,
    GUI_LAYOUT_SIDE,
    GUI_LAYOUT_INVENTORY,

    // must be last one
    GUI_LAYOUT_COUNT
  };

  // sound types
  enum {
    SOUND_TYPE_COMMAND=0,
    SOUND_TYPE_HIT,
    SOUND_TYPE_SELECT,
    SOUND_TYPE_ATTACK,

    // must be the last one
    SOUND_TYPE_COUNT
  };

  // npc types
  enum {
    NPC_TYPE_COMMONER=0,
    NPC_TYPE_MERCHANT,
    NPC_TYPE_HEALER,
    NPC_TYPE_SAGE,
    NPC_TYPE_TRAINER,

    // must be the last one
    NPC_TYPE_COUNT
  };

  static const char *npcTypeName[];

  // the speed when hand fighting is used instead of a weapon
  static const int HAND_WEAPON_SPEED = 5;

  Constants();
  ~Constants();

  static char *getMessage(int index);

  // shortest distance between two rectangles
  static float distance(float x1, float y1, float w1, float h1,
						float x2, float y2, float w2, float h2);

  static void checkTexture(char *message, int w, int h);

  // read until EOL into line. Exclude EOL from LINE.
  // returns the next char after the EOL.
  static int readLine(char *line, FILE *fp);

  inline static float toRadians(float angle) {
		return 3.14159 * (angle / 180.0f);
  }

  inline static float toAngle(float rad) {
		return (180.0f * rad) / 3.14159;
  }

	static void getQuadrantAndAngle( float nx, float ny, int *q, float *angle );

	static int initRootDir( int argc, char *argv[] );

private:
  static bool checkFile(const char *dir, const char *file);
  // used to run scourge with local resources
  static void findLocalResources(const char *appPath, char *dir);
};

char* GetDataPath(char *file);

class CVector5
{
public:
    float x, y, z, u, v;
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
  float *shadingColorDelta;     // 1 per normal
	CVector2  *pTexVerts;		// The texture's UV coordinates
	tFace *pFaces;				// The faces information of the object
};


// This holds our information for each animation of the Quake model.
// A STL vector list of this structure is created in our t3DModel structure below.
struct tAnimationInfo
{
    char strName[255];          // This stores the name of the animation (Jump, Pain, etc..)
    int startFrame;             // This stores the first frame number for this animation
    int endFrame;               // This stores the last frame number for this animation
		int loopingFrames;			// This stores the looping frames for this animation (not used)
		int framesPerSecond;		// This stores the frames per second that this animation runs    
};

typedef float vect3d[3];

// We added 4 new variables to our model structure.  These will help us handle
// the current animation.  As of now, the current animation will continue to loop
// from it's start from to it's end frame until we right click and change animations.
struct t3DModel
{
    int numOfObjects;                   // The number of objects in the model
    int numOfMaterials;                 // The number of materials for the model
    int numOfAnimations;                // The number of animations in this model
		//int currentAnim;					// The current index into pAnimations list 
		//int currentFrame;					// The current frame of the current animation 
		//int nextFrame;						// The next frame of animation to interpolate too
		//float t;							// The ratio of 0.0f to 1.0f between each key frame
		//float lastTime;						// This stores the last time that was stored
		
		int numOfTags;						// This stores the number of tags in the model
		t3DModel	**pLinks;				// This stores a list of pointers that are linked to this model
		struct tMd3Tag		*pTags;			// This stores all the tags for the model animations
    float movex;                        // Needed to draw the model
    float movey;
    float movez;
    std::vector<tAnimationInfo> pAnimations; // The list of animations
		std::map<std::string, int> pAnimationMap; // name->index into pAnimations.
    std::vector<tMaterialInfo> pMaterials;   // The list of material information (Textures and colors)
    std::vector<t3DObject> pObject;          // The object list for our model (frames)
    vect3d *vertices;                   // All vertices for every frame of the model
    int numVertices;                    // The number of vertices (constant for each frame)
    int *pGlCommands;                   // The glCommands used to draw the model faster
};

#define getAn( name ) ( ( name[0] == 'a' || name[0] == 'e' || name[0] == 'i' || name[0] == 'o' || name[0] == 'u' || name[0] == 'y' ? "an" : "a" ) )

typedef unsigned char byte;

extern void ComputeNormals(t3DModel *pModel);
extern void CreateTexture(GLuint textureArray[],char *strFileName,int textureID);
extern void swap(unsigned char & a, unsigned char & b);
extern void findNormal( CVector3 *p1, CVector3 *p2, CVector3 *p3, CVector3 *normal );

#endif
