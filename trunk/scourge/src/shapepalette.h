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

#include <string.h>
#include "constants.h"
#include "shape.h"
#include "glshape.h"
#include "gltorch.h"
#include "gllocator.h"
#include "debugshape.h"
#include "md2shape.h"

/**
  *@author Gabor Torok
  */

class GLShape;
class GLTorch;
  
class ShapePalette {
private:
  GLShape *shapes[256];
  GLShape *creature_shapes[256];
  GLShape *item_shapes[256];  
  GLuint display_list, creature_display_list_start, item_display_list_start, max_display_list;
  GLuint gui_texture;
  GLuint textures[100]; // store textures
  GLShape *shapeNameArray[256];

  GLuint ns_tex[3];
  GLuint ew_tex[3];
  GLuint wood_tex[3];
  GLuint floor_tex[3], floor2_tex[3];;
  GLuint notex[3];
  GLuint lamptex[3];
  GLuint doorNStex[3];
  GLuint doorEWtex[3];      
  GLuint shelftex[3];
  GLuint chesttex[3];
  GLuint shelftex2[3];
  GLuint chesttex2[3];

  // how big to make the walls
  const static Sint16 unitSide = MAP_UNIT;
  const static Sint16 unitOffset = MAP_UNIT_OFFSET;
  const static Sint16 wallHeight = MAP_WALL_HEIGHT;

  void initShapes();
  void loadTextures();

  // shape descriptions
  static char *wallDescription[], *doorDescription[], *doorFrameDescription[], *torchDescription[];
  static int wallDescriptionCount, doorDescriptionCount, doorFrameDescriptionCount, torchDescriptionCount;

  static ShapePalette *instance;

public: 
	ShapePalette();
	~ShapePalette();

  // singleton
  inline static ShapePalette *getInstance() { return instance; }

  // cursor
  SDL_Surface *cursor;
  GLubyte *cursorImage;

  SDL_Surface *logo;
  GLubyte *logoImage;   

  SDL_Surface *scourge;
  GLubyte *scourgeImage;

  GLuint cloud, candle;
 
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
	LAMP_INDEX,
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
      
  inline GLShape *getShape(int index) { return shapes[index]; }
  inline GLShape *getCreatureShape(int index) { return creature_shapes[index]; }
  inline GLShape *getItemShape(int index) { return item_shapes[index]; }

  inline bool isItem(GLShape *shape) { return shape->getDisplayList() >= item_display_list_start; }
  
  inline Sint16 getUnitSide() { return unitSide; }
  inline Sint16 getUnitOffset() { return unitOffset; }
  inline Sint16 getWallHeight() { return wallHeight; }

  inline GLuint getGuiTexture() { return gui_texture; }

protected:
  GLuint loadGLTextures(char *fileName);
  void setupAlphaBlendedBMP(char *filename, SDL_Surface **surface, GLubyte **image);
  void swap(unsigned char & a, unsigned char & b);
};

#endif
