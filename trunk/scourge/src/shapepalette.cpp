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
#include "session.h"

#define ALWAYS_RELOAD_THEME 1

//#define VARIATION_BASE 10.0f
#define VARIATION_BASE 4.0f

char WallTheme::themeRefName[THEME_REF_COUNT][40] = {
  "wall",
  "corner",
  "door_ew",
  "door_ns",
  "passage_floor",
  "room_floor"  
};

WallTheme::WallTheme( char *name, ShapePalette *shapePal ) {
  this->name = name;
  this->shapePal = shapePal;
  for(int i = 0; i < THEME_REF_COUNT; i++)
    themeRefMap[ themeRefName[i] ] = i;
}

WallTheme::~WallTheme() {
  unload();
}

void WallTheme::load() {
  cerr << "*** Loading theme: " << getName() << endl;
  debug();
  for(int ref = 0; ref < THEME_REF_COUNT; ref++) {
    for(int face = 0; face < faceCount[ ref ]; face++) {
      loadTextureGroup( ref, face, textures[ ref ][ face ] );
    }
  }
}

void WallTheme::loadTextureGroup( int ref, int face, char *texture ) {
  char path[300], bmp[300];  
  if( texture ) {
    string s = texture;
    GLuint id;
    if( loadedTextures.find(s) == loadedTextures.end() ) {

	  // see if it's a system texture (no need to double load it)
	  sprintf( bmp, "%s.bmp", texture );
	  id = shapePal->findTextureByName( bmp );
	  if( id == 0 ) {
		sprintf( path, "/%s", bmp );
		id = shapePal->loadGLTextures(path);
		loadedTextures[s] = id;
	  }
    } else {
      id = loadedTextures[s];
    }
    textureGroup[ref][face] = id;  
  } else {
    textureGroup[ref][face] = 0;
  }
}

void WallTheme::unload() {
  char bmp[300];
  cerr << "*** Dumping theme: " << getName() << endl;
  for(map<string,GLuint>::iterator i=loadedTextures.begin(); i!=loadedTextures.end(); ++i) {
	string s = i->first;
    GLuint id = i->second;

	// don't delete system textures!
	sprintf( bmp, "%s.bmp", s.c_str() );
	id = shapePal->findTextureByName( bmp );
	if( id == 0 ) {
	  glDeleteTextures( 1, &id );
	}
  }
  loadedTextures.clear();
}

GLuint *WallTheme::getTextureGroup( string themeRefName ) {
  int ref = themeRefMap[ themeRefName ];
  return textureGroup[ ref ];
}

int WallTheme::getFaceCount( string themeRefName ) {
  int ref = themeRefMap[ themeRefName ];
  return faceCount[ ref ];
}
    
void WallTheme::debug() {
  cerr << "THEME=" << getName() << endl;
  for(int ref = 0; ref < THEME_REF_COUNT; ref++) {
    cerr << "\tref=" << themeRefName[ref] << endl;
    for(int face = 0; face < faceCount[ref]; face++) {
      cerr << "\t\t" << textures[ref][face] << endl;
    }
  }
}


ShapePalette *ShapePalette::instance = NULL;

ShapePalette::ShapePalette(Session *session){
  texture_count = 0;
  textureGroupCount = 0;
  themeCount = 0;
  currentTheme = NULL;
  this->session = session;
}

void ShapePalette::initialize() {
  // load textures
  ripple_texture = loadGLTextures("/ripple.bmp");
  gui_texture = loadGLTextures("/gui.bmp");
  gui_texture2 = loadGLTextures("/gui2.bmp");
  paper_doll_texture = loadGLTextures("/paperdoll.bmp");
  cloud = loadGLTextures("/cloud.bmp");
  candle = loadGLTextures("/candle.bmp");
  torchback = loadGLTextures("/torchback.bmp");
  border = loadGLTextures("/border.bmp");
  border2 = loadGLTextures("/border2.bmp");
  highlight = loadGLTextures("/highlight.bmp");
  SDL_Surface *tmpSurface = NULL;
  GLubyte *tmpImage = NULL;
  setupAlphaBlendedBMP("/gargoyle.bmp", &tmpSurface, &tmpImage);
  gargoyle = loadGLTextureBGRA(tmpSurface, tmpImage, GL_LINEAR);
  if(tmpImage) free(tmpImage);
  if(tmpSurface) SDL_FreeSurface( tmpSurface );

  // load the texture info
  char errMessage[500];
  char s[200];
  sprintf(s, "%s/world/shapes.txt", rootDir);
  FILE *fp = fopen(s, "r");
  if(!fp) {        
    sprintf(errMessage, "Unable to find the file: %s!", s);
    cerr << errMessage << endl;
    exit(1);
  }
  //int sum = 0;
  char path[300];
  char line[255];
  int n = fgetc(fp);
  while(n != EOF) {
    if(n == 'T') {
      // skip ':'
      fgetc(fp);
      n = Constants::readLine(line, fp);

      // texture declaration
	  loadSystemTexture( line );
      /*
    } else if(n == 'S') {
      fgetc(fp);
      n = Constants::readLine(line, fp);
      // load md2 skin 
      GLuint skin;
      sprintf(path, "%s", line);
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
        vector<string> *list = new vector<string>();
        descriptions.push_back(list);
        for(int i = 0; i < count; i++) {
          n = Constants::readLine(line, fp);
          string s = line + 1;
          list->push_back(s);
        }
        cerr << "added " << count << " lines of description. # of description groups=" << descriptions.size() << endl;
      }

    } else if(n == 'N') {
      fgetc(fp);
      n = Constants::readLine(line, fp);

      // texture group
      ShapeValues *sv = new ShapeValues();
      strcpy( sv->textureGroupIndex, line );

      sv->xrot = sv->yrot = sv->zrot = 0.0f;

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
      //	  cerr << "*** shape description group (string): " << line << endl;
      sv->descriptionIndex = atoi(line + 1);

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
        sv->m3ds_scale = strtod(line + 1, NULL);
      } else if(n == 'L') {
        n = Constants::readLine(line, fp);
        sv->teleporter = 1;
      } 
      
      // icon rotation
      if(n == 'R') {
        n = Constants::readLine(line, fp);
        sv->xrot = atof(strtok(line + 1, ","));
        sv->yrot = atof(strtok(NULL, ","));
        sv->zrot = atof(strtok(NULL, ","));
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
      info->scale = strtod(strtok(line + 1, ","), NULL);

      // store the md2 model and info
      sprintf(path, "%s%s", rootDir, info->filename);
      cerr << "Loading md2 model: " << path << " scale: " << info->scale << endl;
      info->model = LoadMd2Model(path);

      // store it
      string s = info->name;
      old_creature_models[s] = info;

    } else if(n == 'P') {
      fgetc(fp);
      n = Constants::readLine(line, fp);

      int index = atoi(strtok(line, ","));
      cerr << "options for shape, index=" << index << " size=" << shapeValueVector.size() << endl;

      ShapeValues *sv = shapeValueVector[index];
      sv->skipSide = atoi(strtok(NULL, ","));
      sv->stencil = atoi(strtok(NULL, ","));
      sv->blocksLight = atoi(strtok(NULL, ","));
    } else if( n == 'H' ) {
      fgetc(fp);
      n = Constants::readLine(line, fp);
      WallTheme *theme = new WallTheme( strdup(line), this );
      // read the shape ref-s
      for(int ref = 0; ref < WallTheme::THEME_REF_COUNT; ref++) {
        n = Constants::readLine(line, fp);
        char *p = strtok( line + 1, "," );
        int i = 0;
        while( p && i < MAX_TEXTURE_COUNT ) {
          theme->addTextureName( ref, i, (const char *)p );
          p = strtok( NULL, "," );
          i++;
        }
        if( i < 3 ) {
          cerr << "*** Error: theme=" << theme->getName() << " has wrong number of textures for line=" << (ref + 1) << endl;
        }
        theme->setFaceCount( ref, i );
      }

      // read the multitexture info
      for(int i = 0; i < WallTheme::MULTI_TEX_COUNT; i++) {
        n = Constants::readLine(line, fp);
        char *p = strtok( line + 1, "," );
        theme->setMultiTexRed( i, atof( p ) );
        p = strtok( NULL, "," );
        theme->setMultiTexGreen( i, atof( p ) );
        p = strtok( NULL, "," );
        theme->setMultiTexBlue( i, atof( p ) );
        p = strtok( NULL, "," );
        theme->setMultiTexInt( i, atof( p ) );
        p = strtok( NULL, "," );
        theme->setMultiTexSmooth( i, ( atoi( p ) != 0 ) );
      }

      themes[themeCount++] = theme;
      cerr << "&&& Added theme: " << theme->getName() << " count=" << themeCount << endl;
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

  // create shapes
  for(int i = 0; i < (int)shapeValueVector.size(); i++) {
    cerr << "Creating shape i=" << i << endl;
    ShapeValues *sv = shapeValueVector[i];
    cerr << "\t" <<
    " width=" << sv->width << 
    " depth=" << sv->depth << 
    " height=" << sv->height << endl;

    // Resolve the texture group.
    // For theme-based shapes, leave texture NULL, they will be resolved later.
    bool themeBasedShape = false;
    GLuint *texture = NULL;
    if( strlen(sv->textureGroupIndex) < 5 ) {
      texture = textureGroup[ atoi( sv->textureGroupIndex) ];
    } else {
      texture = textureGroup[ 0 ];
      themeBasedShape = true;
    }

    if(sv->teleporter) {
      shapes[(i + 1)] =
      new GLTeleporter(texture, textures[9].id,
                       sv->width, sv->depth, sv->height,
                       strdup(sv->name), 
                       sv->descriptionIndex,
                       sv->color,
                       (i + 1));
    } else if(strlen(sv->m3ds_name)) {
      shapes[(i + 1)] =
      new C3DSShape(sv->m3ds_name, sv->m3ds_scale, this,
                    texture,                     
                    strdup(sv->name), 
                    sv->descriptionIndex,
                    sv->color,
                    (i + 1));
    } else if(sv->torch > -1) {
      if(sv->torch == 5) {
        shapes[(i + 1)] =
        new GLTorch(texture, textures[9].id,
                    sv->width, sv->depth, sv->height,
                    strdup(sv->name),
                    sv->descriptionIndex,
                    sv->color,
                    (i + 1));
      } else {
        shapes[(i + 1)] =
        new GLTorch(texture, textures[9].id,
                    sv->width, sv->depth, sv->height,
                    strdup(sv->name),
                    sv->descriptionIndex,
                    sv->color,
                    (i + 1), 
                    torchback, sv->torch);
      }
    } else {
      shapes[(i + 1)] =
      new GLShape(texture,
                  sv->width, sv->depth, sv->height,
                  strdup(sv->name),
                  sv->descriptionIndex,
                  sv->color,
                  (i + 1));
    }
    shapes[(i + 1)]->setSkipSide(sv->skipSide);
    shapes[(i + 1)]->setStencil(sv->stencil == 1);
    shapes[(i + 1)]->setLightBlocking(sv->blocksLight == 1);
    shapes[(i + 1)]->setIconRotation(sv->xrot, sv->yrot, sv->zrot);

    // Call this when all other intializations are done.
    if(themeBasedShape) {
      themeShapes.push_back( shapes[(i + 1)] );
      string s = ( sv->textureGroupIndex + 6 );
      themeShapeRef.push_back( s );
    } else {
      shapes[(i + 1)]->initialize();
    }

    string s = sv->name;
    shapeMap[s] = shapes[(i + 1)];
  }

  // remember the number of shapes
  shapeCount = (int)shapeValueVector.size() + 1;

  // clean up temp. shape objects 
  // FIXME: do we need to free the vector's elements?
  if(shapeValueVector.size()) shapeValueVector.erase(shapeValueVector.begin(), 
                                                     shapeValueVector.end());

  // add some special, "internal" shapes
  shapes[shapeCount] = 
  new GLTorch(textureGroup[14], textures[9].id,
              1, 1, 2,
              strdup("SPELL_FIREBALL"),
              0,
              strtoul("6070ffff", NULL, 16),
              shapeCount, 
              torchback, Constants::SOUTH); // Hack: use SOUTH for a spell
  shapes[shapeCount]->setSkipSide(false);
  shapes[shapeCount]->setStencil(false);
  shapes[shapeCount]->setLightBlocking(false);  
  shapes[shapeCount]->initialize();
  string nameStr = shapes[shapeCount]->getName();
  shapeMap[nameStr] = shapes[shapeCount];
  shapeCount++;

  // FIXME: do something with these...
  formationTexIndex = texture_count;
  strcpy(textures[texture_count++].filename, "formation1.bmp");
  strcpy(textures[texture_count++].filename, "formation2.bmp");
  strcpy(textures[texture_count++].filename, "formation3.bmp");
  strcpy(textures[texture_count++].filename, "formation4.bmp");
  strcpy(textures[texture_count++].filename, "formation5.bmp");
  strcpy(textures[texture_count++].filename, "formation6.bmp");

  // load the status modifier icons
  for(int i = 0; i < Constants::STATE_MOD_COUNT; i++) {
    sprintf(path, "/icons/i%d.bmp", i);
    GLuint icon = loadGLTextures(path);
    cerr << "Loading stat mod icon: " << path << " found it? " << (icon ? "yes" : "no") << endl;
    if(icon) statModIcons[i] = icon;
  }

  // set up the cursor
  setupAlphaBlendedBMP("/cursor.bmp", &cursor, &cursorImage);
  cursor_texture = loadGLTextureBGRA(cursor, cursorImage, GL_LINEAR);
  setupAlphaBlendedBMP("/crosshair.bmp", &crosshair, &crosshairImage);
  crosshair_texture = loadGLTextureBGRA(crosshair, crosshairImage);
  setupAlphaBlendedBMP("/paperdoll.bmp", &paperDoll, &paperDollImage);

  // set up the logo
  setupAlphaBlendedBMP("/logo.bmp", &logo, &logoImage);
  logo_texture = loadGLTextures("/logo.bmp");

  // set up the scourge
  setupAlphaBlendedBMP("/scourge.bmp", &scourge, &scourgeImage);

  gui_wood_texture = this->findTextureByName("gui-wood.bmp");

  if(!instance) instance = this;
}

ShapePalette::~ShapePalette(){
  //    for(int i =0; i < (int)creature_models.size(); i++){
  //        delete creature_models[i];    
  //    }
}

void ShapePalette::loadRandomTheme() {
  loadTheme( themes[ (int)( (float)(themeCount - 1) * rand()/RAND_MAX ) + 1 ] );
}

void ShapePalette::loadTheme(const char *themeName) {
  cerr << "*** Using theme: " << themeName << endl;

  // find that theme!
  WallTheme *theme = NULL;
  for(int i = 0; i < themeCount; i++) {
    if( !strcmp( themes[i]->getName(), themeName ) ) {
      theme = themes[i];
      break;
    }
  }
  if( !theme ) {
    cerr << "*** Error: Can't find theme: " << themeName << endl;
    exit(1);
  }

  loadTheme( theme );
}

void ShapePalette::loadTheme( WallTheme *theme ) {
  cerr << "*** Using theme: " << theme->getName() << " current=" << (!currentTheme ? "null" : currentTheme->getName()) << endl;
  if( ALWAYS_RELOAD_THEME || currentTheme != theme) {

    // unload the previous theme
    // FIXME: This could be optimized to not unload shared textures.
    if( currentTheme ) currentTheme->unload();
    
    // load new theme
    currentTheme = (WallTheme*)theme;
    currentTheme->load();

    // apply theme to shapes
    cerr << "*** Applying theme to shapes: ***" << endl;
    GLShape::createDarkTexture( currentTheme );
    for(int i = 0; i < (int)themeShapes.size(); i++) {
      GLShape *shape = themeShapes[i];
      string ref = themeShapeRef[i];
      GLuint *textureGroup = currentTheme->getTextureGroup( ref );
      cerr << "\tshape=" << shape->getName() << " ref=" << ref << 
        " tex=" << textureGroup[0] << "," << textureGroup[1] << "," << textureGroup[2] << endl;  
      shape->setTexture( textureGroup );

      // create extra shapes for variations
      shape->deleteVariationShapes();
      for( int i = 3; i < currentTheme->getFaceCount( ref ); i++ ) {
        shape->createVariationShape( i, textureGroup );
      }
    }

    cerr << "**********************************" << endl;
  }
}

char *ShapePalette::getRandomDescription(int descriptionGroup) {
  if(descriptionGroup >= 0 && descriptionGroup < (int)descriptions.size()) {
    vector<string> *list = descriptions[descriptionGroup];
    int n = (int)((float)list->size() * rand()/RAND_MAX);
    return(char*)((*list)[n].c_str());
  }
  return NULL;
}

t3DModel * ShapePalette::LoadMd2Model(char *file_name){
  t3DModel *t3d = new t3DModel;    
  g_LoadMd2.ImportMD2(t3d, file_name); 
  return t3d;   
}

void ShapePalette::UnloadMd2Model( t3DModel *model ) {
  g_LoadMd2.DeleteMD2( model );
  delete model;
}


GLShape *ShapePalette::getCreatureShape(char *model_name, char *skin_name, 
										float scale, Monster *monster) {

  // find the model the old way
  Md2ModelInfo *model_info;
  char realSkinName[300];

  string model_name_str = model_name;
  if(old_creature_models.find(model_name_str) != old_creature_models.end()) {  
    model_info = old_creature_models[model_name_str];
	strcpy( realSkinName, skin_name );
  } else {
	// load the model the new way
    if( creature_models.find( model_name_str ) == creature_models.end() ) {
      model_info = (Md2ModelInfo*)malloc( sizeof( Md2ModelInfo ) );

      char path[300];
      sprintf(path, "%s%s/tris.md2", rootDir, model_name);
      cerr << "&&&&&&&&&& Loading md2 model: " << path << endl;
      model_info->model = LoadMd2Model(path);
      strcpy( model_info->name, model_name );
      model_info->scale = 1.0f;

      creature_models[ model_name_str ] = model_info;
    } else {
	  //	  cerr << "&&&&&&&&&& Using already loaded md2 model: " << model_name_str << endl;
      model_info = creature_models[ model_name_str ];
    }

    // increment its ref. count
    if(loaded_models.find(model_info) == loaded_models.end()) {
      loaded_models[model_info] = 1;
    } else {
      loaded_models[model_info] = loaded_models[model_info] + 1;
    }

	// expand the skin name location
	sprintf( realSkinName, "%s/%s", model_name, skin_name );

	// load monster sounds
	if( monster )
	  session->getGameAdapter()->loadMonsterSounds( model_name, 
													monster->getSoundMap( model_name ) );
  }

  // find or load the skin
  string skin = realSkinName;
  GLuint skin_texture;
  char path[300];
  if(creature_skins.find(skin) == creature_skins.end()){
    if(!session->getGameAdapter()->isHeadless()) {
      sprintf(path, "%s%s", rootDir, realSkinName);
      cerr << "&&&&&&&&&& Loading texture: " << path << endl;
      CreateTexture(&skin_texture, path, 0);
      cerr << "&&&&&&&&&& Loaded texture: " << skin_texture << endl;
      creature_skins[skin] = skin_texture;
    }
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
  // FIXME: shapeindex is always FIGHTER. Does it matter?
  MD2Shape *shape = 
    MD2Shape::createShape(model_info->model, skin_texture, 
                          (scale == 0.0f ? model_info->scale : scale),
                          textureGroup[14], model_info->name, -1, 0xf0f0ffff, 0);
  shape->setSkinName(realSkinName);
  return shape;
}

void ShapePalette::decrementSkinRefCount(char *model_name, char *skin_name,
										 Monster *monster) {
  string skin = skin_name;
  GLuint skin_texture;
  // look for old-style skin
  if(creature_skins.find(skin) == creature_skins.end()){
	// look for new-style skin
	char realSkinName[300];
	sprintf( realSkinName, "%s/%s", model_name, skin_name );
	skin = realSkinName;
	if(creature_skins.find(skin) == creature_skins.end()){
	  cerr << "&&&&&&&&&& WARNING: could not find skin: " << realSkinName << endl;
	  return;
	} else {
	  skin_texture = creature_skins[skin];
	}
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

  // ------------------------------------------------------
  string model = model_name;
  Md2ModelInfo *model_info;
  if(creature_models.find(model) == creature_models.end()){
	// this is ok. It could be an old-style model (or non-monster)
    cerr << "&&&&&&&&&& Not unloading model: " << model << endl;
    return;
  } else {
    model_info = creature_models[model];
  }

  if(loaded_models.find(model_info) == loaded_models.end()) {
    cerr << "&&&&&&&&&& WARNING: could not find model id=" << model << endl;
    return;
  }

  loaded_models[model_info] = loaded_models[model_info] - 1;
  // unload model if no more references  
  if(loaded_models[model_info] == 0) {
    cerr << "&&&&&&&&&& Deleting model: " << model << endl;
    loaded_models.erase(model_info);
    creature_models.erase(model);

	UnloadMd2Model( model_info->model );

    free(model_info);
  }

  // unload monster sounds
  if( monster )
	session->getGameAdapter()->unloadMonsterSounds( model_name, 
													monster->getSoundMap( model_name ) );
}

// the next two methods are slow, only use during initialization
GLuint ShapePalette::findTextureByName(const char *filename) {
  for(int i = 0; i < texture_count; i++) {
    if(!strcmp(textures[i].filename, filename)) return textures[i].id;
  }
  return 0;
}

GLShape *ShapePalette::findShapeByName(const char *name, bool variation) {
  if(!name || !strlen(name)) return NULL;
  string s = name;
  if(shapeMap.find(s) == shapeMap.end()) {
    cerr << "&&& warning: could not find shape by name " << s << endl;
    return NULL;
  }
  GLShape *shape = shapeMap[s];
  if( !variation || shape->getVariationShapesCount() == 0 ) return shape;
  int n = (int)( ( VARIATION_BASE + (float)( shape->getVariationShapesCount() )) * rand()/RAND_MAX );
  if( n >= (int)(VARIATION_BASE) ) {
    //cerr << "SHAPEPALETTE: Using variation for: " << name << endl;
    return shape->getVariationShape( n - (int)(VARIATION_BASE) );
  }
  return shape;
}

// defaults to SWORD for unknown shapes
int ShapePalette::findShapeIndexByName(const char *name) {
  string s;
  if(!name || !strlen(name)) s = "SWORD";
  else s = name;
  if(shapeMap.find(s) == shapeMap.end()) {
    cerr << "&&& warning: could not find shape INDEX by name " << s << endl;
    return 0;
  }
  return shapeMap[s]->getShapePalIndex();
}

/* function to load in bitmap as a GL texture */
GLuint ShapePalette::loadGLTextures(char *filename) {

  if(session->getGameAdapter()->isHeadless()) return 0;

  char fn[300];
  strcpy(fn, rootDir);
  strcat(fn, filename);

  GLuint texture[1];

  /* Create storage space for the texture */
  SDL_Surface *TextureImage[1];

  /* Load The Bitmap, Check For Errors, If Bitmap's Not Found Quit */
  fprintf(stderr, "Loading texture: %s\n", fn);
  if( ( TextureImage[0] = SDL_LoadBMP( fn ) ) ) {
    fprintf(stderr, "\tFound it. pitch=%d width=%d height=%d\n", (TextureImage[0]->pitch/3), TextureImage[0]->w, TextureImage[0]->h);

    Constants::checkTexture("ShapePalette::loadGLTextures", 
                            TextureImage[0]->w, TextureImage[0]->h);

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
    fprintf(stderr, "\tNot found.\n");
  }
  fprintf(stderr, "\tStored texture at: %u\n", texture[0]);

  /* Free up any memory we may have used */
  if( TextureImage[0] )
    SDL_FreeSurface( TextureImage[0] );

  return texture[0];
}

/* function to load in bitmap as a GL texture */
GLuint ShapePalette::loadGLTextureBGRA(SDL_Surface *surface, GLubyte *image, int glscale) {

  if(session->getGameAdapter()->isHeadless()) return 0;

  GLuint texture[1];

  Constants::checkTexture("ShapePalette::loadGLTextureBGRA", 
                          surface->w, surface->h);

  /* Create The Texture */
  glGenTextures( 1, &texture[0] );

  /* Typical Texture Generation Using Data From The Bitmap */
  glBindTexture( GL_TEXTURE_2D, texture[0] );

  /* Use faster filtering here */
  glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, glscale );
  glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, glscale );
//  glTexImage2D( GL_TEXTURE_2D, 0, 4,
//                surface->w, surface->h, 0, 
//                GL_BGRA, GL_UNSIGNED_BYTE, image );
  gluBuild2DMipmaps(GL_TEXTURE_2D, 4,
                    surface->w, surface->h,
                    GL_BGRA, GL_UNSIGNED_BYTE, image);
  return texture[0];
}

void ShapePalette::swap(unsigned char & a, unsigned char & b) {
  unsigned char temp;

  temp = a;
  a    = b;
  b    = temp;

  return;
}

void ShapePalette::setupAlphaBlendedBMP(char *filename, SDL_Surface **surface, 
                                        GLubyte **image, int red, int green, int blue) {

  if(session->getGameAdapter()->isHeadless()) return;

  cerr << "file: " << filename << " red=" << red << " green=" << green << " blue=" << blue << endl;

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
        c += (  (*surface)->pitch - (width * (*surface)->format->BytesPerPixel) );
      r = data[c++];
      g = data[c++];
      b = data[c++];

      (*image)[count++] = r;
      (*image)[count++] = g;
      (*image)[count++] = b;
      //(*image)[count++] = (GLubyte)( (float)(b + g + r) / 3.0f );
      //(*image)[count++] = (GLubyte)( (b + g + r == 0 ? 0x00 : 0xff) );
      (*image)[count++] = (GLubyte)( ((int)r == blue && (int)g == green && (int)b == red ? 0x00 : 0xff) );
    }
  }
}

GLuint ShapePalette::loadSystemTexture( char *line ) {
  if( texture_count >= MAX_SYSTEM_TEXTURE_COUNT ) {
	cerr << "Error: *** no more room for system textures!. Not loading: " << line << endl;
	return 0;
  }
  char path[300];
  strcpy( textures[texture_count].filename, line );
  sprintf( path, "/%s", textures[texture_count].filename );
  // load the texture
  GLuint id = textures[ texture_count ].id = loadGLTextures( path );
  texture_count++;
  return id;
}
