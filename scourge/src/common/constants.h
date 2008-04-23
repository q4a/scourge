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

// -=K=- since it was most popular header i made most porting here
#ifdef _MSC_VER // i only checked for MSVC 8 portability  
	// turn numeric types conversion warnings OFF; like size_t <-> int; 
#	pragma warning(disable : 4267 4244) //4800) but not odd usage of bool  
	// to be sure that these are included *before* following defines
#	include <string.h>
#	include <cstdio>
	// some common string ops have different names under MSVC 8
#	define strcasecmp _stricmp
#	define snprintf _sprintf_p
#	define strdup _strdup
	// just to avoid using sprintf
#	define sprintf please_use_snprintf
	// somewhere was error: unknown identifier "time"
#	include <time.h>
	// MSVC 8 has no rint so i improvise one
	template<class T>
	T rint(T v)
	{
		T f = floor(v); // v >= f
		T c = ceil(v);  // c >= v
		return (v-f > c-v)? c : f; //what's closer is rint(v)
	}
	// To ease debugging 
#	include <assert.h>
#endif // MSVC 8 portability 

// include sdl, opengl and glut
#include <SDL.h>
#include <SDL_opengl.h>
#include <SDL_endian.h>
#include <SDL_ttf.h>
#include <SDL_image.h>

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

// Couldn't figure out gettext for the macos x. :-(
#ifdef NO_GETTEXT
#define _(String) String
#define gettext_noop(String) String
#define N_(String) gettext_noop (String)
#else
#include <libintl.h>
#define _(String) gettext (String)
#define gettext_noop(String) String
#define N_(String) gettext_noop (String)
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

#define SCOURGE_VERSION "0.20"
#define MAX_PARTY_SIZE 4

// Max level depth per mission
#define MAX_MISSION_DEPTH 10

#define toint(x) (int)(x<0 ? (x - 0.5f) : (x + 0.5f))

extern std::string rootDir;
extern std::string localeDir;
extern std::string configDir;
extern std::string get_config_dir_name();
extern std::string get_config_file_name();
extern std::string get_file_name(const std::string fileName );

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

struct ParticleStruct {
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
};

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
private:
	static int maxMissionId;
public:

	static inline int getNextMissionId() { return maxMissionId++; }

	static std::string scourgeLocaleName;

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

	static const char *inventoryTags[];

  static const int MAP_GRID_TILE_WIDTH = 6;
  static const int MAP_GRID_TILE_HEIGHT = 5;
  static const int MAP_GRID_TILE_PIXEL_WIDTH = 256;
  static const int MAP_GRID_TILE_PIXEL_HEIGHT = 256;

  // creature movement
  enum motion {
    MOTION_MOVE_TOWARDS=0,
    MOTION_MOVE_AWAY, // flee
    MOTION_CLEAR_PATH, //rapidly make way
    MOTION_LOITER, //wander slowly
    MOTION_STAND //FREEZE!
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

#define OUTDOOR_FLOOR_TEX_SIZE 4

// outdoor floor tile
#define OUTDOORS_STEP 4

// The max value of a skill under normal circumstances.
#define MAX_SKILL 100

  // Directions (a bitfield so they can be combined)
  static const Uint16 MOVE_UP = 1;
  static const Uint16 MOVE_DOWN = 2;
  static const Uint16 MOVE_LEFT = 4;
  static const Uint16 MOVE_RIGHT = 8;
  // Short cuts - useful when working in 8 directions
  static const Uint16 MOVE_UP_RIGHT = MOVE_UP | MOVE_RIGHT;
  static const Uint16 MOVE_UP_LEFT = MOVE_UP | MOVE_LEFT;
  static const Uint16 MOVE_DOWN_RIGHT = MOVE_DOWN | MOVE_RIGHT;
  static const Uint16 MOVE_DOWN_LEFT = MOVE_DOWN | MOVE_LEFT;

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
    UNCURSE_DIALOG_TITLE,
    RECHARGE_DIALOG_TITLE,
    IDENTIFY_DIALOG_TITLE,
	// last one
	MESSAGE_COUNT
  };
  static char *messages[][100];
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
  static int getPotionSkillByName(char const* p);

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
  inline static int getEffectByName(char const* s) {
	for(int i = 0; i < EFFECT_COUNT; i++)
			if(!strcmp(s, EFFECT_NAMES[i]))
				return i;
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
    LOGLEVEL_MINIMAL=0,
    LOGLEVEL_PARTIAL,
    LOGLEVEL_VERBOSE,
    LOGLEVEL_FULL
  };

  enum {			// Message types as they show up in the log window
    MSGTYPE_NORMAL=0,		// Normal text: Descriptions etc.
    MSGTYPE_MISSION,		// Mission related and other key text
    MSGTYPE_PLAYERDAMAGE,	// Player has taken damage
    MSGTYPE_NPCDAMAGE,		// NPC/monster has taken damage
    MSGTYPE_PLAYERMAGIC,	// Player uses magic
    MSGTYPE_NPCMAGIC,		// NPC/monster uses magic
    MSGTYPE_PLAYERITEM,		// Player uses an item
    MSGTYPE_NPCITEM,		// NPC/monster uses an item
    MSGTYPE_PLAYERBATTLE,	// Player's battle actions
    MSGTYPE_NPCBATTLE,		// NPCs'/monsters' battle actions
    MSGTYPE_PLAYERDEATH,	// A player character has died
    MSGTYPE_NPCDEATH,		// An NPC or monster has died
    MSGTYPE_FAILURE,		// The player has failed at something
    MSGTYPE_STATS,		// Player's stats have changed: State mods, leveled up etc.
    MSGTYPE_SYSTEM,		// System and debug messages
    MSGTYPE_SKILL		// Skill related messages
  };

#define WEATHER_CLEAR 0x00
#define WEATHER_RAIN 0x01
#define WEATHER_THUNDER 0x02
#define WEATHER_FOG 0x04
#define MAX_WEATHER 0x08
  
  enum {
    ACTION_NO_ACTION=-1,
    ACTION_EAT_DRINK=0,
    ACTION_CAST_SPELL,
    ACTION_SPECIAL,

    // this must be the last one
    ACTION_COUNT
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

  // reserved sound channels
  enum {
    OBJECT_CHANNEL = 0,
    AMBIENT_CHANNEL,
    FOOTSTEP_CHANNEL,
    RAIN_CHANNEL
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

  // engine actions
  enum {
    ENGINE_ACTION_SCROLL_SOUTH=0,
    ENGINE_ACTION_SCROLL_NORTH,
    ENGINE_ACTION_SCROLL_EAST,
    ENGINE_ACTION_SCROLL_WEST,
    ENGINE_ACTION_PLAYER0,
    ENGINE_ACTION_PLAYER1,
    ENGINE_ACTION_PLAYER2,
    ENGINE_ACTION_PLAYER3,
    ENGINE_ACTION_GROUP_MODE,
    ENGINE_ACTION_INVENTORY,
    ENGINE_ACTION_OPTIONS,
    ENGINE_ACTION_FORMATION,
    ENGINE_ACTION_MINIMAP,
    ENGINE_ACTION_ZOOM_IN,
    ENGINE_ACTION_ZOOM_OUT,
    ENGINE_ACTION_ALWAYS_CENTER,
    ENGINE_ACTION_INCREASE_SPEED,
    ENGINE_ACTION_DECREASE_SPEED,
    ENGINE_ACTION_NEXT_ROUND,
    ENGINE_ACTION_FLOATING_UI,
    ENGINE_ACTION_BOTTOM_UI,
    ENGINE_ACTION_INVENTORY_UI,
    ENGINE_ACTION_COMBAT_MODE,
    ENGINE_ACTION_NEXT_WEAPON,
    ENGINE_ACTION_QUICKSPELL1,
    ENGINE_ACTION_QUICKSPELL2,
    ENGINE_ACTION_QUICKSPELL3,
    ENGINE_ACTION_QUICKSPELL4,
    ENGINE_ACTION_QUICKSPELL5,
    ENGINE_ACTION_QUICKSPELL6,
    ENGINE_ACTION_QUICKSPELL7,
    ENGINE_ACTION_QUICKSPELL8,
    ENGINE_ACTION_QUICKSPELL9,
    ENGINE_ACTION_QUICKSPELL10,
    ENGINE_ACTION_QUICKSPELL11,
    ENGINE_ACTION_QUICKSPELL12,
    ENGINE_ACTION_QUICK_SAVE,
    ENGINE_ACTION_QUICK_LOAD,
    ENGINE_ACTION_AUTO_LOAD
  };

  static const char *npcTypeName[];
	static const char *npcTypeDisplayName[];

  // the speed when hand fighting is used instead of a weapon
  static const int HAND_WEAPON_SPEED = 5;

  Constants();
  ~Constants();

  static char *getMessage(int index);

  // shortest distance between two rectangles
	static float distance(float x1, float y1, float w1, float h1, float x2, float y2, float w2, float h2);

  static void checkTexture(char *message, int w, int h);

  // read until EOL into line. Exclude EOL from LINE.
  // returns the next char after the EOL.
  static int readLine(char *line, FILE *fp);

  inline static float toRadians(float angle) {
		// 2.13.3(1) The type of a floating literal is double unless explicitly specified by a suffix.
		// explicit float conversion to silence warning
		return static_cast<float>(3.14159 * (angle / 180.0f));
  }

  inline static float toAngle(float rad) {
		return static_cast<float>((180.0f * rad) / 3.14159);
  }

	static void getQuadrantAndAngle( float nx, float ny, int *q, float *angle );

	static int initRootDir( int argc, char *argv[] );

	static int findLocaleDir();

	static void generateTrigTables();
	static float sinFromAngle(int angle);
	static float cosFromAngle(int angle);

private:
  static bool checkFile(const std::string& dir, const std::string& file);
  // used to run scourge with local resources
  static std::string findLocalResources(const std::string& appPath);
};

std::string GetDataPath(const std::string& file);

class CVectorTex
{
public:
    float x, y, z, u, v, r, g, b, a;
		GLuint tex;
};

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

	/**
		* This computes the magnitude of a normal.
		* (magnitude = sqrt(x^2 + y^2 + z^2)
		*/
	double magnitude() const;
	void normalize();

	/**
		* @return This returns the cross product between 2 vectors
		*/
	CVector3 cross(const CVector3& vVector1);

	CVector3 operator+(const CVector3& vVector1) const;
	void operator+=(const CVector3& vVector1);

	CVector3 operator-(const CVector3& vPoint1) const;

	CVector3 operator/(const float scalar) const;
	void operator/=(const float scalar);

	bool operator==(const CVector3& compare) const;
	void operator=(const CVector3& copy);

	CVector3();
	CVector3(float x, float y, float z);
	CVector3(const CVector3& copy);
};

// This is our 2D point class.  This will be used to store the UV coordinates.
class CVector2
{
public:
    float x, y;
};


// This file includes all of the model structures that are needed to load
// in a .Md2 file.  When it comes to skeletal animation, we need to add quite
// a bit more variables to these structures.  Not all of the data will be used
// because Quake2 models don't have such a need.  I decided to keep the structures
// the same as the rest of the model loaders on our site so that we could eventually
// use a base class in the future for a library.
//
typedef unsigned char BYTE;
#define MAX_TEXTURES 100								// The maximum amount of textures to load

// -=K=-: to avoid using mallocs lets keep Texture data in vectors
typedef std::vector<GLubyte> TextureData;

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
	std::string strFile;			// The texture file name (If this is set it's a texture map)
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
	std::vector<t3DModel*> pLinks;				// This stores a list of pointers that are linked to this model
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

char *getAn( const char *name );

typedef unsigned char byte;

extern void ComputeNormals(t3DModel *pModel);
extern void CreateTexture(GLuint textureArray[],char *strFileName,int textureID);
extern void swap(unsigned char & a, unsigned char & b);
extern void findNormal( CVector3 *p1, CVector3 *p2, CVector3 *p3, CVector3 *normal );


#endif
