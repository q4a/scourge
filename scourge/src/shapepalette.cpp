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

ShapePalette::ShapePalette(){
  texture_count = 0;
  textureGroupCount = 0;
  descriptionCount = 0;
  descriptionIndexCount = 0;

  // load textures
  gui_texture = loadGLTextures("data/gui.bmp");
  cloud = loadGLTextures("data/cloud.bmp");
  candle = loadGLTextures("data/candle.bmp");
  torchback = loadGLTextures("data/torchback.bmp");

  // load the texture info
  char errMessage[500];
  char s[200];
  sprintf(s, "data/world/shapes.txt");
  FILE *fp = fopen(s, "r");
  if(!fp) {        
	sprintf(errMessage, "Unable to find the file: %s!", s);
	cerr << errMessage << endl;
	exit(1);
  }
  int sum = 0;
  char path[300];
  char line[255];
  int n = fgetc(fp);
  while(n != EOF) {
	if(n == 'T') {
	  // skip ':'
	  fgetc(fp);
	  n = Constants::readLine(line, fp);

	  // texture declaration
	  strcpy(textures[texture_count].filename, line);
	  sprintf(path, "data/%s", textures[texture_count].filename);
	  // load the texture
	  textures[texture_count].id = loadGLTextures(path);
	  texture_count++;
	  /*
	} else if(n == 'S') {
	  fgetc(fp);
	  n = Constants::readLine(line, fp);
	  // load md2 skin 
	  GLuint skin;
	  sprintf(path, "data/%s", line);
	  cerr << "Loading md2 skin: " << line << endl;
	  CreateTexture(&skin, path, 0);
	  string name = line;
	  creature_skins[name] = skin;
	  */
	} else if(n == 'G') {
	  // skip ':'
	  fgetc(fp);
	  n = Constants::readLine(line, fp);

	  // parse native texture group
	  int c = 0;
	  char *token = strtok(line, ",");
	  while(token && c < 3) {
		// store the index only, resolve after all textures loaded
		textureGroup[textureGroupCount][c++] = atoi(token);
		token = strtok(NULL, ",");
	  }
	  // resolve the indexes
	  if(c == 3) {
		textureGroupCount++;
	  }
	} else if(n == 'D') {
	  fgetc(fp);
	  n = Constants::readLine(line, fp);
	  
	  // read description lines
	  int count = atoi(line);
	  if(count > 0) {
		descriptionLength[descriptionIndexCount] = count;
		descriptionIndex[descriptionIndexCount++] = sum;
		sum += count;
		for(int i = 0; i < count; i++) {
		  n = Constants::readLine(line, fp);
		  strcpy(description[descriptionCount++], line + 2);
		}
	  }

	} else if(n == 'N') {
	  fgetc(fp);
	  n = Constants::readLine(line, fp);

	  // texture group
	  ShapeValues *sv = new ShapeValues();
	  sv->textureGroupIndex = atoi(line);

	  // dimensions
	  n = Constants::readLine(line, fp);
	  sv->width = atoi(strtok(line + 1, ","));
	  sv->depth = atoi(strtok(NULL, ","));
	  sv->height = atoi(strtok(NULL, ","));

	  // name
	  n = Constants::readLine(line, fp);
	  strcpy(sv->name, line + 1);

	  // description
	  n = Constants::readLine(line, fp);
	  sv->descriptionIndex = atoi(line + 2);

	  // color
	  n = Constants::readLine(line, fp);
	  sv->color = strtoul(line + 1, NULL, 16);

	  // extra for torches:
	  sv->torch = -1;
	  sv->m3ds_name[0] = '\0';
	  sv->teleporter = 0;
	  if(n == 'T') {
		n = Constants::readLine(line, fp);
		sv->torch = atoi(line + 1);
	  } else if(n == '3') {
		n = Constants::readLine(line, fp);
		strcpy(sv->m3ds_name, line + 1);
		n = Constants::readLine(line, fp);
		sv->m3ds_scale = strtof(line + 1, NULL);
	  } else if(n == 'L') {
		n = Constants::readLine(line, fp);
		sv->teleporter = 1;
	  }

	  // store it for now
	  shapeValueVector.push_back(sv);
	
	} else if(n == '2') {	  
	  Md2ModelInfo *info = new Md2ModelInfo();
	  // name
	  n = Constants::readLine(line, fp);
	  strcpy(info->name, line + 1);
	  
	  // file
	  n = Constants::readLine(line, fp);
	  strcpy(info->filename, line + 1);
	  
	  n = Constants::readLine(line, fp);
	  // scale
	  info->scale = strtof(strtok(line + 1, ","), NULL);
	  // dimensions
	  info->width = atoi(strtok(NULL, ","));
	  info->depth = atoi(strtok(NULL, ","));
	  info->height = atoi(strtok(NULL, ","));
	  	  
	  // store the md2 model and info
	  sprintf(path, "data/%s", info->filename);
	  cerr << "Loading md2 model: " << path << " scale: " << info->scale << 
		" dim: " << info->width << ", " << info->depth << "," << info->height << endl;
	  info->model = LoadMd2Model(path);
	  
	  // store it
	  string s = info->name;
	  creature_models[s] = info;

	  // create a block shape for this model
	  // (used to measure space w/o creating a creature shape)
	  creature_block_shapes[s] = 
		new GLShape(textureGroup[14],
					info->width, info->depth, info->height,
					strdup(info->name),
					0xffffffff, 0, 0);
	  

	} else if(n == 'P') {
	  fgetc(fp);
	  n = Constants::readLine(line, fp);

	  int index = atoi(strtok(line, ","));
	  cerr << "options for shape, index=" << index << " size=" << shapeValueVector.size() << endl;

	  ShapeValues *sv = shapeValueVector[index];
	  sv->skipSide = atoi(strtok(NULL, ","));
	  sv->stencil = atoi(strtok(NULL, ","));
	  sv->blocksLight = atoi(strtok(NULL, ","));

	} else {
	  // skip this line
	  n = Constants::readLine(line, fp);
	}
  }
  fclose(fp);

  // resolve texture groups
  for(int i = 0; i < textureGroupCount; i++) {
	for(int c = 0; c < 3; c++) {
	  textureGroup[i][c] = textures[textureGroup[i][c]].id;
	}
  }

  // Create the display lists
  // FIXME: this should use shapeValueVector.size() instead once all the shapes come from shapes.txt
  //  display_list = glGenLists((Constants::SHAPE_INDEX_COUNT) * 3);

  // create shapes
  for(int i = 0; i < (int)shapeValueVector.size(); i++) {
	cerr << "Creating shape i=" << i << endl;
	ShapeValues *sv = shapeValueVector[i];
	cerr << "\t" <<
	  " width=" << sv->width << 
	  " depth=" << sv->depth << 
	  " height=" << sv->height << endl;

	/*
	int len = descriptionLength[sv->descriptionIndex];
    char **d = (char**)malloc(len * sizeof(char*));
    for(int t = 0; t < len; t++) {
	  d[t] = (char*)malloc(200 * sizeof(char));
	  strcpy(d[t], description[descriptionIndex[sv->descriptionIndex]]);
    }
	*/

	//	GLuint dl = display_list + (i * 3);
	GLuint dl = 0;
	if(sv->teleporter) {
	  shapes[(i + 1)] =
		new GLTeleporter(textureGroup[sv->textureGroupIndex], textures[9].id,
						 sv->width, sv->depth, sv->height,
						 strdup(sv->name), 
						 //d, len,
						 (sv->descriptionIndex ? 
						  (char**)&(description[descriptionIndex[sv->descriptionIndex]]) :
						  NULL),
						 (sv->descriptionIndex ?
						  descriptionLength[sv->descriptionIndex] :
						  0),
						 sv->color,
						 dl, (i + 1));
	} else if(strlen(sv->m3ds_name)) {
	  shapes[(i + 1)] =
		new C3DSShape(sv->m3ds_name, sv->m3ds_scale, this,
					  textureGroup[sv->textureGroupIndex], 
					  sv->width, sv->depth, sv->height,
					  strdup(sv->name), 
					  //d, len,
					  (sv->descriptionIndex ? 
					   (char**)&(description[descriptionIndex[sv->descriptionIndex]]) :
					   NULL),
					  (sv->descriptionIndex ?
					   descriptionLength[sv->descriptionIndex] :
					   0),
					  sv->color,
					  dl,(i + 1));
	} else if(sv->torch > -1) {
	  if(sv->torch == 5) {
		shapes[(i + 1)] =
		  new GLTorch(textureGroup[sv->textureGroupIndex], textures[9].id,
					  sv->width, sv->depth, sv->height,
					  strdup(sv->name),
					  //d, len,
					  (sv->descriptionIndex ? 
					   (char**)&(description[descriptionIndex[sv->descriptionIndex]]) :
					   NULL),
					  (sv->descriptionIndex ?
					   descriptionLength[sv->descriptionIndex] :
					   0),
					  sv->color,
					  dl, (i + 1));
	  } else {
		shapes[(i + 1)] =
		  new GLTorch(textureGroup[sv->textureGroupIndex], textures[9].id,
					  sv->width, sv->depth, sv->height,
					  strdup(sv->name),
					  //d, len,
					  (sv->descriptionIndex ? 
					   (char**)&(description[descriptionIndex[sv->descriptionIndex]]) :
					   NULL),
					  (sv->descriptionIndex ?
					   descriptionLength[sv->descriptionIndex] :
					   0),
					  sv->color,
					  dl, (i + 1), 
					  torchback, sv->torch);
	  }
	} else {
	  shapes[(i + 1)] =
		new GLShape(textureGroup[sv->textureGroupIndex],
					sv->width, sv->depth, sv->height,
					strdup(sv->name),
					//d, len,
					(sv->descriptionIndex ? 
					 (char**)&(description[descriptionIndex[sv->descriptionIndex]]) :
					 NULL),
					(sv->descriptionIndex ?
					 descriptionLength[sv->descriptionIndex] :
					 0),
					sv->color,
					dl, (i + 1));
	}
	shapes[(i + 1)]->setSkipSide(sv->skipSide);
	shapes[(i + 1)]->setStencil(sv->stencil == 1);
	shapes[(i + 1)]->setLightBlocking(sv->blocksLight == 1);
  }
  // remember the number of shapes
  shapeCount = (int)shapeValueVector.size() + 1;

  // clean up temp. shape objects 
  // FIXME: do we need to free the vector's elements?
  shapeValueVector.clear();

  // FIXME: do something with these...
  formationTexIndex = texture_count;
  strcpy(textures[texture_count++].filename, "formation1.bmp");
  strcpy(textures[texture_count++].filename, "formation2.bmp");
  strcpy(textures[texture_count++].filename, "formation3.bmp");
  strcpy(textures[texture_count++].filename, "formation4.bmp");
  strcpy(textures[texture_count++].filename, "formation5.bmp");
  strcpy(textures[texture_count++].filename, "formation6.bmp");

  // set up the cursor
  setupAlphaBlendedBMP("data/cursor.bmp", &cursor, &cursorImage);

  // set up the logo
  setupAlphaBlendedBMP("data/logo.bmp", &logo, &logoImage);
  logo_texture = loadGLTextures("data/logo.bmp");

  // set up the scourge
  setupAlphaBlendedBMP("data/scourge.bmp", &scourge, &scourgeImage);
     
  /*
  // creatures              
  // The order at which we "push back" models is important                 
  creature_models.push_back(LoadMd2Model("data/models/m2.md2"));  
  creature_models.push_back(LoadMd2Model("data/models/m1.md2"));
  creature_models.push_back(LoadMd2Model("data/models/m3.md2"));
  creature_models.push_back(LoadMd2Model("data/models/m4.md2"));
  creature_models.push_back(LoadMd2Model("data/models/m5.md2"));
  creature_models.push_back(LoadMd2Model("data/models/m6.md2"));
  cout<<"MD2 Models loaded" << endl;
  */

  if(!instance) instance = this;
}

ShapePalette::~ShapePalette(){
  //    for(int i =0; i < (int)creature_models.size(); i++){
  //        delete creature_models[i];    
  //    }
}

t3DModel * ShapePalette::LoadMd2Model(char *file_name){
    t3DModel * t3d;
    char fn[300];
    t3d = new t3DModel; 
    strcpy(fn, rootDir);
    strcat(fn, file_name);
   
    g_LoadMd2.ImportMD2(t3d, fn); 
    return t3d;   
}    

GLShape *ShapePalette::getCreatureShape(char *model_name, char *skin_name) {

  // find the model
  string model = model_name;
  if(creature_models.find(model) == creature_models.end()){
	cerr << "Can't find md2 model named " << model << endl;
	exit(1);
  }
  Md2ModelInfo *model_info = creature_models[model];

  // find or load the skin
  string skin = skin_name;
  GLuint skin_texture;
  char path[300];
  if(creature_skins.find(skin) == creature_skins.end()){
	sprintf(path, "data/%s", skin_name);
	//	cerr << "&&&&&&&&&& Loading texture: " << path << endl;
	CreateTexture(&skin_texture, path, 0);
	cerr << "&&&&&&&&&& Loaded texture: " << skin_texture << endl;
	creature_skins[skin] = skin_texture;
  } else {
	skin_texture = creature_skins[skin];
  }

  // increment its ref. count
  if(loaded_skins.find(skin_texture) == loaded_skins.end()) {
	loaded_skins[skin_texture] = 1;
  } else {
	loaded_skins[skin_texture] = loaded_skins[skin_texture] + 1;
  }
  //  cerr << "&&&&&&&&&& Texture ref count at load for id: " << skin_texture << 
  //	" count: " << loaded_skins[skin_texture] << endl;

  //  cerr << "Creating creature shape with model: " << model << " and skin: " << skin << endl;

  // create the shape.
  // FIXME: shapeindex is always FIGHTER_INDEX. Does it matter?
  MD2Shape *shape = new MD2Shape(model_info->model, skin_texture, model_info->scale,
								 textureGroup[14], 
								 model_info->width, model_info->depth, model_info->height,
								 model_info->name,
								 0xf0f0ffff, 0); //Constants::FIGHTER_INDEX);  
  shape->setSkinName(skin_name);
  return shape;
}

void ShapePalette::decrementSkinRefCount(char *skin_name) {
  string skin = skin_name;
  GLuint skin_texture;
  if(creature_skins.find(skin) == creature_skins.end()){
	cerr << "&&&&&&&&&& WARNING: could not find skin: " << skin_name << endl;
	return;
  } else {
	skin_texture = creature_skins[skin];
  }

  if(loaded_skins.find(skin_texture) == loaded_skins.end()) {
	cerr << "&&&&&&&&&& WARNING: could not find skin id=" << skin_texture << endl;
	return;
  }

  loaded_skins[skin_texture] = loaded_skins[skin_texture] - 1;
  //  cerr << "&&&&&&&&&& Texture ref count at load for id: " << skin_texture << 
  //	" count: " << loaded_skins[skin_texture] << endl;
  // unload texture if no more references
  if(loaded_skins[skin_texture] == 0) {
	cerr << "&&&&&&&&&& Deleting texture: " << skin_texture << endl;
	loaded_skins.erase(skin_texture);
	creature_skins.erase(skin);
	glDeleteTextures(1, &skin_texture);
  }
}

// the next two methods are slow, only use during initialization
GLuint ShapePalette::findTextureByName(const char *filename) {
  for(int i = 0; i < texture_count; i++) {
	if(!strcmp(textures[i].filename, filename)) return textures[i].id;
  }
  return 0;
}

GLShape *ShapePalette::findShapeByName(const char *name) {
  if(!name || !strlen(name)) return NULL;
  for(int i = 1; i < shapeCount; i++) {
	if(!strcmp(shapes[i]->getName(), name)) return shapes[i];
  }
  return NULL;
}

// defaults to SWORD_INDEX for unknown shapes
int ShapePalette::findShapeIndexByName(const char *name) {
  if(!name || !strlen(name)) return Constants::SWORD_INDEX;
  for(int i = 1; i < shapeCount; i++) {
	if(!strcmp(shapes[i]->getName(), name)) {
	  return i;
	}
  }
  return Constants::SWORD_INDEX;
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
