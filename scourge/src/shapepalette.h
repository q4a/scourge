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
#include <vector.h>
#include "constants.h"
#include "shape.h"
#include "glshape.h"
#include "gltorch.h"
#include "gllocator.h"
#include "debugshape.h"
#include "md2shape.h"
#include "3dsshape.h"
#include "Md2.h"

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
  GLuint display_list, item_display_list_start, max_display_list;
  //GLuint creature_display_list_start;
  GLuint gui_texture;
  
  typedef struct _Texture {
	GLuint id;
	char filename[80];
  } Texture;

  Texture textures[100]; // store textures
  int texture_count;
  GLShape *shapeNameArray[256];

  GLuint ns_tex[3];
  GLuint ew_tex[3];
  GLuint wood_tex[3];
  GLuint floor_tex[3], floor2_tex[3], floor3_tex[3];
  GLuint notex[3];
  GLuint lamptex[3];
  GLuint doorNStex[3];
  GLuint doorEWtex[3];      
  GLuint shelftex[3];
  GLuint chesttex[3];
  GLuint shelftex2[3];
  GLuint chesttex2[3];
  GLuint md2_tex[6];

  // how big to make the walls
  const static Sint16 unitSide = MAP_UNIT;
  const static Sint16 unitOffset = MAP_UNIT_OFFSET;
  const static Sint16 wallHeight = MAP_WALL_HEIGHT;

  void initShapes();
  void loadTextures();

  // shape descriptions
  static char *wallDescription[], *doorDescription[], *doorFrameDescription[], *torchDescription[];
  static char *boardDescription[], *brazierDescription[], *columnDescription[];
  static int wallDescriptionCount, doorDescriptionCount, doorFrameDescriptionCount, torchDescriptionCount;
  static int boardDescriptionCount, brazierDescriptionCount, columnDescriptionCount;
  

  static ShapePalette *instance;
  
  // Md2 shapes
  CLoadMD2 g_LoadMd2; 
  t3DModel * LoadMd2Model(char *file_name);                  

public: 
  ShapePalette();
  ~ShapePalette();

  GLuint formationTexIndex;
  inline GLuint getTexture(int index) { return textures[index].id; }

  // singleton
  inline static ShapePalette *getInstance() { return instance; }

  // cursor
  SDL_Surface *cursor;
  GLubyte *cursorImage;

  SDL_Surface *logo;
  GLubyte *logoImage;   

  SDL_Surface *scourge;
  GLubyte *scourgeImage;

  GLuint cloud, candle, torchback;
       
  inline GLShape *getShape(int index) { return shapes[index]; }  
  inline GLShape *getItemShape(int index) { return item_shapes[index]; }

  inline bool isItem(GLShape *shape) { return shape->getDisplayList() >= item_display_list_start; }
  
  inline Sint16 getUnitSide() { return unitSide; }
  inline Sint16 getUnitOffset() { return unitOffset; }
  inline Sint16 getWallHeight() { return wallHeight; }

  inline GLuint getGuiTexture() { return gui_texture; }

  GLuint findTextureByName(const char *filename);
  
  // Md2 shapes
  GLShape *getCreatureShape(int index);                    
  vector<t3DModel *> creature_models; 

protected:
  GLuint loadGLTextures(char *fileName);
  void setupAlphaBlendedBMP(char *filename, SDL_Surface **surface, GLubyte **image);
  void swap(unsigned char & a, unsigned char & b);
};

#endif
