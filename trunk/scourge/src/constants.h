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

#ifdef WIN32
#define SEPARATOR '\\'
#else
#define SEPARATOR '/'
#endif

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

/**
  *@author Gabor Torok
  */

extern char rootDir[300];

class Constants {
public:

  // creature movement
  enum motion {
    MOTION_MOVE_TOWARDS=0,
    MOTION_MOVE_AWAY,
    MOTION_LOITER };

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
	INV_MODE_PROPERTIES,
	INV_MODE_INVENTORY,
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
	PLAYER_ONLY
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
	
	// must be the last one
	SHAPE_INDEX_COUNT
  };

  enum {
	FIGHTER_INDEX = 0,
	ROGUE_INDEX,
	CLERIC_INDEX,
	WIZARD_INDEX,

	// last one
	CREATURE_INDEX_COUNT
  };
  
  enum {
	SWORD_INDEX = 0,
	BOOKSHELF_INDEX,
	CHEST_INDEX,
	BOOKSHELF2_INDEX,
	CHEST2_INDEX,

	// should be the last one
	ITEM_INDEX_COUNT
  };

  enum {
	SWORD_WEAPON = 0,
	AXE_WEAPON,
	BOW_WEAPON,

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
	
	// must be last
	STATE_MOD_COUNT
  };
  static const char *STATE_NAMES[];

  Constants();
  ~Constants();

  static char *getMessage(int index);
};

#endif
