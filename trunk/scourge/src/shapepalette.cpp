/***************************************************************************
                          shapepalette.cpp  -  description
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

#include "shapepalette.h"

ShapePalette *ShapePalette::instance = NULL;

char *ShapePalette::wallDescription[] = {
  "Pale weeds grow over these ancient walls",
  "The walls crumble with age, yet appear sturdy",
  "Dark limestone walls mark these corridors"
};
int ShapePalette::wallDescriptionCount = 3;

char *ShapePalette::doorDescription[] = {
  "It appears to be a thick wooden door",
  "A dungeon door made of wood",
  "The aged oak door is scarred by knife points and torch flame"
};
int ShapePalette::doorDescriptionCount = 3;

char *ShapePalette::doorFrameDescription[] = {
  "The door frames seem to be made of some hard wood",
  "The wood around the doors is reinforced by metal nails"
};
int ShapePalette::doorFrameDescriptionCount = 2;

char *ShapePalette::torchDescription[] = {
  "A torch blazes nearby",
  "The light is firmly attached to the wall, you cannot get it",
  "It is a burning torch"
};
int ShapePalette::torchDescriptionCount = 3;



ShapePalette::ShapePalette(){

  // set up the cursor
  setupAlphaBlendedBMP("data/cursor.bmp", &cursor, &cursorImage);

  // set up the logo
  setupAlphaBlendedBMP("data/logo.bmp", &logo, &logoImage);

  // set up the scourge
  setupAlphaBlendedBMP("data/scourge.bmp", &scourge, &scourgeImage);
    
  // load textures
  loadTextures();

  // Create the display lists
  display_list = glGenLists((Constants::SHAPE_INDEX_COUNT + Constants::CREATURE_INDEX_COUNT) * 3);
 
 	// init the shapes
  initShapes();

  if(!instance) instance = this;
}

ShapePalette::~ShapePalette(){
}

void ShapePalette::initShapes() {
	Uint32 color = 0xac8060ff;
  bool debug = false;
  int count = 0;
  shapes[Constants::EW_WALL_INDEX] =
    new GLShape(ew_tex,
                unitOffset, unitSide - (unitOffset * 2), wallHeight,
                "EAST WALL", wallDescription, wallDescriptionCount,
                (debug ? 0xff0000ff : color),
                display_list + (count++ * 3), Constants::EW_WALL_INDEX);
  shapes[Constants::EW_WALL_EXTRA_INDEX] =
		new GLShape(ew_tex,
				  unitOffset, unitSide - unitOffset, wallHeight,
				  "EAST WALL EXTRA", wallDescription, wallDescriptionCount,
				  (debug ? 0xff0000ff : color),
				  display_list + (count++ * 3), Constants::EW_WALL_EXTRA_INDEX);
  shapes[Constants::EW_WALL_TWO_EXTRAS_INDEX] =
		new GLShape(ew_tex,
				  unitOffset, unitSide, wallHeight,
				  "EAST WALL TWO EXTRAS", wallDescription, wallDescriptionCount,
				  (debug ? 0xff0000ff : color),
				  display_list + (count++ * 3), Constants::EW_WALL_TWO_EXTRAS_INDEX);
  
  shapes[Constants::NS_WALL_INDEX] =
		new GLShape(ns_tex,
				  unitSide - (unitOffset * 2), unitOffset, wallHeight,
				  "SOUTH WALL", wallDescription, wallDescriptionCount,
				  (debug ? 0xff0000ff : color),
				  display_list + (count++ * 3), Constants::NS_WALL_INDEX);  
  shapes[Constants::NS_WALL_EXTRA_INDEX] =
    new GLShape(ns_tex,
                unitSide - unitOffset, unitOffset, wallHeight,
                "SOUTH WALL EXTRA", wallDescription, wallDescriptionCount,
                (debug ? 0xff0000ff : color),
                display_list + (count++ * 3), Constants::NS_WALL_EXTRA_INDEX);
  shapes[Constants::NS_WALL_TWO_EXTRAS_INDEX] =
    new GLShape(ns_tex,
                unitSide, unitOffset, wallHeight,
                "SOUTH WALL TWO EXTRAS", wallDescription, wallDescriptionCount,
                (debug ? 0xff0000ff : color),
                display_list + (count++ * 3), Constants::NS_WALL_TWO_EXTRAS_INDEX);

  // make the walls seethru
  ((GLShape*)shapes[Constants::EW_WALL_INDEX])->setSkipSide( 1 << GLShape::FRONT_SIDE );
  ((GLShape*)shapes[Constants::EW_WALL_EXTRA_INDEX])->setSkipSide( 1 << GLShape::FRONT_SIDE );  
  ((GLShape*)shapes[Constants::EW_WALL_TWO_EXTRAS_INDEX])->setSkipSide( 1 << GLShape::FRONT_SIDE );  
  ((GLShape*)shapes[Constants::NS_WALL_INDEX])->setSkipSide( 1 << GLShape::LEFT_RIGHT_SIDE );
  ((GLShape*)shapes[Constants::NS_WALL_EXTRA_INDEX])->setSkipSide( 1 << GLShape::LEFT_RIGHT_SIDE );
  ((GLShape*)shapes[Constants::NS_WALL_TWO_EXTRAS_INDEX])->setSkipSide( 1 << GLShape::LEFT_RIGHT_SIDE );

  shapes[Constants::EW_WALL_INDEX]->setStencil(true);
  shapes[Constants::EW_WALL_EXTRA_INDEX]->setStencil(true);
  shapes[Constants::EW_WALL_TWO_EXTRAS_INDEX]->setStencil(true);
  shapes[Constants::NS_WALL_INDEX]->setStencil(true);
  shapes[Constants::NS_WALL_EXTRA_INDEX]->setStencil(true);
  shapes[Constants::NS_WALL_TWO_EXTRAS_INDEX]->setStencil(true);


  // corners
  shapes[Constants::CORNER_INDEX] =
    new GLShape(wood_tex,
                unitOffset, unitOffset, wallHeight,
                "CORNER",
                (debug ? 0xff0000ff : color),
                display_list + (count++ * 3), Constants::CORNER_INDEX);

	// ew door
  Uint32 doorColor = 0xaa6633ff;
	int doorSize = unitSide - unitOffset * 3 - 4;
  shapes[Constants::EW_DOOR_INDEX] =
    new GLShape(doorEWtex,
                1, doorSize, wallHeight - 2,
                "EW DOOR", doorDescription, doorDescriptionCount,
                (debug ? 0xff0000ff : doorColor),
                display_list + (count++ * 3), Constants::EW_DOOR_INDEX);
	shapes[Constants::DOOR_SIDE_INDEX] =
    new GLShape(wood_tex,
                unitOffset, 2, wallHeight - 2,
                "CORNER SIDE", doorFrameDescription, doorFrameDescriptionCount,
                (debug ? 0xff0000ff : color),
                display_list + (count++ * 3), Constants::DOOR_SIDE_INDEX);
	shapes[Constants::EW_DOOR_TOP_INDEX] =
    new GLShape(wood_tex,
                2, unitSide - unitOffset * 2, 2,
                "EW DOOR TOP", doorFrameDescription, doorFrameDescriptionCount,
                (debug ? 0xff0000ff : color),
                display_list + (count++ * 3), Constants::EW_DOOR_TOP_INDEX);

	// ns door
  shapes[Constants::NS_DOOR_INDEX] =
    new GLShape(doorNStex,
                doorSize, 1, wallHeight - 2,
                "NS DOOR", doorDescription, doorDescriptionCount,
                (debug ? 0xff0000ff : doorColor),
                display_list + (count++ * 3), Constants::NS_DOOR_INDEX);
	shapes[Constants::NS_DOOR_TOP_INDEX] =
    new GLShape(wood_tex,
                unitSide - unitOffset * 2, 2, 2,
                "NS DOOR TOP", doorFrameDescription, doorFrameDescriptionCount,
                (debug ? 0xff0000ff : color),
                display_list + (count++ * 3), Constants::NS_DOOR_TOP_INDEX);

  // floor tile 1
	shapes[Constants::FLOOR_TILE_INDEX] =
    new GLShape(floor2_tex,
                unitSide, unitSide, 0,
                "FLOOR TILE",
                (debug ? 0xff0000ff : 0x806040ff),
                display_list + (count++ * 3), Constants::FLOOR_TILE_INDEX);
	shapes[Constants::ROOM_FLOOR_TILE_INDEX] =
    new GLShape(floor_tex,
                unitSide, unitSide, 0,
                "ROOM FLOOR TILE",
                (debug ? 0xff0000ff : 0xa08040ff),
                display_list + (count++ * 3), Constants::ROOM_FLOOR_TILE_INDEX);
  
  ((GLShape*)shapes[Constants::FLOOR_TILE_INDEX])->setSkipSide( 1 << GLShape::FRONT_SIDE );  
  ((GLShape*)shapes[Constants::FLOOR_TILE_INDEX])->setSkipSide( 1 << GLShape::LEFT_RIGHT_SIDE );
  ((GLShape*)shapes[Constants::ROOM_FLOOR_TILE_INDEX])->setSkipSide( 1 << GLShape::FRONT_SIDE );  
  ((GLShape*)shapes[Constants::ROOM_FLOOR_TILE_INDEX])->setSkipSide( 1 << GLShape::LEFT_RIGHT_SIDE );

	shapes[Constants::LAMP_NORTH_INDEX] =
    new GLTorch(notex, textures[9],
                1, 1, 2,
                "LAMP", torchDescription, torchDescriptionCount,
                (debug ? 0xff0000ff : 0xf0f0ffff),
                display_list + (count++ * 3),
                Constants::LAMP_NORTH_INDEX, torchback, Constants::NORTH);
	shapes[Constants::LAMP_SOUTH_INDEX] =
    new GLTorch(notex, textures[9],
                1, 1, 2,
                "LAMP", torchDescription, torchDescriptionCount,
                (debug ? 0xff0000ff : 0xf0f0ffff),
                display_list + (count++ * 3),
                Constants::LAMP_SOUTH_INDEX, torchback, Constants::SOUTH);
	shapes[Constants::LAMP_WEST_INDEX] =
    new GLTorch(notex, textures[9],
                1, 1, 2,
                "LAMP", torchDescription, torchDescriptionCount,
                (debug ? 0xff0000ff : 0xf0f0ffff),
                display_list + (count++ * 3),
                Constants::LAMP_WEST_INDEX, torchback, Constants::WEST);
	shapes[Constants::LAMP_EAST_INDEX] =
    new GLTorch(notex, textures[9],
                1, 1, 2,
                "LAMP", torchDescription, torchDescriptionCount,
                (debug ? 0xff0000ff : 0xf0f0ffff),
                display_list + (count++ * 3),
                Constants::LAMP_EAST_INDEX, torchback, Constants::EAST);


	shapes[Constants::LAMP_BASE_INDEX] =
    new GLShape(wood_tex, 
                1, 1, 2,
                "LAMP", torchDescription, torchDescriptionCount,
                (debug ? 0xff0000ff : 0xf0f0ffff),
                display_list + (count++ * 3),
                Constants::LAMP_BASE_INDEX);                

	shapes[Constants::DEBUG_INDEX] =
    new DebugShape(wood_tex, 
                2, 1, 2,
                "DEBUG", torchDescription, torchDescriptionCount,
                (debug ? 0xff0000ff : 0xf0f0ffff),
                display_list + (count++ * 3),
                Constants::DEBUG_INDEX);                

	shapes[Constants::LOCATOR_INDEX] =
        new GLLocator(notex, 
                3, 3, 1,
                "LOCATOR", torchDescription, torchDescriptionCount,
                (debug ? 0xff0000ff : 0xf0f0ffff),
                display_list + (count++ * 3),
                Constants::LOCATOR_INDEX);


  // creatures
  creature_display_list_start = display_list + (count * 3);                

  // creatures
  creature_shapes[Constants::FIGHTER_INDEX] =
    new MD2Shape("data/models/m2.md2", "data/models/m2.bmp", 2.0f,
                 notex,
                 3, 3, 6,
                 "FIGHTER",
                 (debug ? 0xff0000ff : 0xf0f0ffff),
                 display_list + (count++ * 3),
                 Constants::FIGHTER_INDEX);
  
  creature_shapes[Constants::ROGUE_INDEX] =
    new MD2Shape("data/models/m1.md2", "data/models/m1.bmp", 2.0f,
                 notex,
                 3, 3, 6,
                 "ROGUE",
                 (debug ? 0xff0000ff : 0xf0f0ffff),
                 display_list + (count++ * 3),
                 Constants::ROGUE_INDEX);
  creature_shapes[Constants::CLERIC_INDEX] =
    new MD2Shape("data/models/m3.md2", "data/models/m3.bmp", 2.5f,
                 notex,
                 3, 3, 6,
                 "CLERIC",
                 (debug ? 0xff0000ff : 0xf0f0ffff),
                 display_list + (count++ * 3),
                 Constants::CLERIC_INDEX);  
  creature_shapes[Constants::WIZARD_INDEX] =
    new MD2Shape("data/models/m4.md2", "data/models/m4.bmp", 2.5f,
                 notex,
                 3, 3, 6,
                 "WIZARD",
                 (debug ? 0xff0000ff : 0xf0f0ffff),
                 display_list + (count++ * 3),
                 Constants::WIZARD_INDEX);  
  
  // items
  item_display_list_start = display_list + (count * 3);                                

	item_shapes[Constants::SWORD_INDEX] =
	  new C3DSShape("data/objects/sword.3ds", 0.3f,
					notex, 2, 4, 1,
					"SWORD",
					0xffffffff,
					display_list + (count++ * 3), Constants::SWORD_INDEX);
	item_shapes[Constants::BOOKSHELF_INDEX] =
    new GLShape(shelftex,
                2, 5, 7,
                "BOOKSHELF", 
                0x0000ffff,
                display_list + (count++ * 3), Constants::BOOKSHELF_INDEX);
	item_shapes[Constants::CHEST_INDEX] =
	  new GLShape(chesttex,
				  2, 3, 2,
				  "CHEST", 
				  0xffaa80ff,
				  display_list + (count++ * 3), Constants::CHEST_INDEX);
	item_shapes[Constants::BOOKSHELF2_INDEX] =
    new GLShape(shelftex2,
                5, 2, 7,
                "BOOKSHELF", 
                0x0000ffff,
                display_list + (count++ * 3), Constants::BOOKSHELF2_INDEX);
	item_shapes[Constants::CHEST2_INDEX] =
    new GLShape(chesttex2,
                3, 2, 2,
                "CHEST", 
                0xffaa80ff,
                display_list + (count++ * 3), Constants::CHEST2_INDEX);

  
  max_display_list = display_list + (count * 3);
}

void ShapePalette::loadTextures() {
  gui_texture = loadGLTextures("data/gui.bmp");

  textures[0] = loadGLTextures("data/front.bmp");
  textures[1] = loadGLTextures("data/top.bmp");
  textures[2] = loadGLTextures("data/side.bmp");
  textures[3] = loadGLTextures("data/wood.bmp");
  textures[4] = loadGLTextures("data/floor.bmp");
  textures[5] = loadGLTextures("data/floor2.bmp");
  textures[6] = loadGLTextures("data/woodtop.bmp");
  textures[7] = loadGLTextures("data/fronttop.bmp");
  textures[8] = loadGLTextures("data/light.bmp");
  textures[9] = loadGLTextures("data/flame.bmp");
  textures[10] = loadGLTextures("data/doorNS.bmp");
  textures[11] = loadGLTextures("data/doorEW.bmp");
  textures[12] = loadGLTextures("data/bookshelf.bmp");
  textures[13] = loadGLTextures("data/chestfront.bmp");
  textures[14] = loadGLTextures("data/chestside.bmp");
  textures[15] = loadGLTextures("data/chesttop.bmp");
  

  // set up the scourge
  cloud = loadGLTextures("data/cloud.bmp");
  candle = loadGLTextures("data/candle.bmp");
  torchback = loadGLTextures("data/torchback.bmp");


  ns_tex[GLShape::FRONT_SIDE] = textures[0];
  ns_tex[GLShape::TOP_SIDE] = textures[7];
  ns_tex[GLShape::LEFT_RIGHT_SIDE] = textures[2];

  ew_tex[GLShape::FRONT_SIDE] = textures[2];
  ew_tex[GLShape::TOP_SIDE] = textures[7];
  ew_tex[GLShape::LEFT_RIGHT_SIDE] = textures[0];

  wood_tex[GLShape::FRONT_SIDE] = textures[3];
  wood_tex[GLShape::TOP_SIDE] = textures[6];
  wood_tex[GLShape::LEFT_RIGHT_SIDE] = textures[3];

  floor_tex[GLShape::FRONT_SIDE] = 0; //textures[4];
  floor_tex[GLShape::TOP_SIDE] = textures[4];
  floor_tex[GLShape::LEFT_RIGHT_SIDE] = 0; //textures[4];

  floor2_tex[GLShape::FRONT_SIDE] = 0; //textures[4];
  floor2_tex[GLShape::TOP_SIDE] = textures[5];
  floor2_tex[GLShape::LEFT_RIGHT_SIDE] = 0; //textures[4];

  lamptex[GLShape::FRONT_SIDE] = textures[8];
  lamptex[GLShape::TOP_SIDE] = 0;
  lamptex[GLShape::LEFT_RIGHT_SIDE] = 0;

  doorNStex[GLShape::FRONT_SIDE] = textures[10];
  doorNStex[GLShape::TOP_SIDE] = textures[6];
  doorNStex[GLShape::LEFT_RIGHT_SIDE] = textures[6];

  doorEWtex[GLShape::FRONT_SIDE] = textures[6];
  doorEWtex[GLShape::TOP_SIDE] = textures[6];
  doorEWtex[GLShape::LEFT_RIGHT_SIDE] = textures[11];

  shelftex[GLShape::FRONT_SIDE] = textures[6];
  shelftex[GLShape::TOP_SIDE] = textures[6];
  shelftex[GLShape::LEFT_RIGHT_SIDE] = textures[12];

  chesttex[GLShape::FRONT_SIDE] = textures[14];
  chesttex[GLShape::TOP_SIDE] = textures[15];
  chesttex[GLShape::LEFT_RIGHT_SIDE] = textures[13];

  shelftex2[GLShape::FRONT_SIDE] = textures[12];
  shelftex2[GLShape::TOP_SIDE] = textures[6];
  shelftex2[GLShape::LEFT_RIGHT_SIDE] = textures[6];

  chesttex2[GLShape::FRONT_SIDE] = textures[13];
  chesttex2[GLShape::TOP_SIDE] = textures[15];
  chesttex2[GLShape::LEFT_RIGHT_SIDE] = textures[14];

  
  notex[0] = 0;
  notex[1] = 0;
  notex[2] = 0;  
}

/* function to load in bitmap as a GL texture */
GLuint ShapePalette::loadGLTextures(char *filename) {
  char fn[300];
  strcpy(fn, rootDir);
  strcat(fn, filename);
  
  GLuint texture[1];
  
  /* Create storage space for the texture */
  SDL_Surface *TextureImage[1];
  
  /* Load The Bitmap, Check For Errors, If Bitmap's Not Found Quit */
  fprintf(stderr, "Loading texture: %s\n", fn);
  if ( ( TextureImage[0] = SDL_LoadBMP( fn ) ) ) {
	fprintf(stderr, "\tFound it.\n");
	
	/* Create The Texture */
	glGenTextures( 1, &texture[0] );
	
	/* Typical Texture Generation Using Data From The Bitmap */
	glBindTexture( GL_TEXTURE_2D, texture[0] );
	
	/* Generate The Texture */
	//	    glTexImage2D( GL_TEXTURE_2D, 0, 3,
	//                    TextureImage[0]->w, TextureImage[0]->h, 0, GL_BGR,
	//            			  GL_UNSIGNED_BYTE, TextureImage[0]->pixels );
	
	/* Linear Filtering */
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
	gluBuild2DMipmaps(GL_TEXTURE_2D, 3,
					  TextureImage[0]->w, TextureImage[0]->h,
					  GL_BGR, GL_UNSIGNED_BYTE, TextureImage[0]->pixels);
  } else {
	texture[0] = 0;
  }
  fprintf(stderr, "\tStored texture at: %u\n", texture[0]);
  
  /* Free up any memory we may have used */
  if ( TextureImage[0] )
	SDL_FreeSurface( TextureImage[0] );
  
  return texture[0];
}

void ShapePalette::swap(unsigned char & a, unsigned char & b) {
    unsigned char temp;

    temp = a;
    a    = b;
    b    = temp;

    return;
}

void ShapePalette::setupAlphaBlendedBMP(char *filename, SDL_Surface **surface, GLubyte **image) {
  *image = NULL;
  char fn[300];
  fprintf(stderr, "setupAlphaBlendedBMP, rootDir=%s\n", rootDir);
  strcpy(fn, rootDir);
  strcat(fn, filename);
  if(((*surface) = SDL_LoadBMP( fn ))) {

    // Rearrange the pixelData
    int width  = (*surface) -> w;
    int height = (*surface) -> h;

	fprintf(stderr, "*** file=%s w=%d h=%d bpp=%d byte/pix=%d scanline=%d\n", 
			fn, width, height, (*surface)->format->BitsPerPixel,
			(*surface)->format->BytesPerPixel, (*surface)->pitch);

    unsigned char * data = (unsigned char *) ((*surface) -> pixels);         // the pixel data

	(*image) = (unsigned char*)malloc(width * height * 4);
    int count = 0;
	int c = 0;
	unsigned char r,g,b;
    // the following lines extract R,G and B values from any bitmap
    for(int i = 0; i < width * height; ++i) {
	  if(i > 0 && i % width == 0) 
		c += (	(*surface)->pitch - (width * (*surface)->format->BytesPerPixel) );
	  r = data[c++];
	  g = data[c++];
	  b = data[c++];
	  
	  (*image)[count++] = r;
	  (*image)[count++] = g;
	  (*image)[count++] = b;
	  //(*image)[count++] = (GLubyte)( (float)(b + g + r) / 3.0f );
	  (*image)[count++] = (GLubyte)( (b + g + r == 0 ? 0x00 : 0xff) );
    }
  }
}
