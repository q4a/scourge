/***************************************************************************
                          shapes.cpp  -  description
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

#include "shapes.h"
#include "glshape.h"
#include "gltorch.h"
#include "glteleporter.h"
#include "3dsshape.h"
#include "md2shape.h"
#include "Md2.h"
#include "glcaveshape.h"

using namespace std;

#define ALWAYS_RELOAD_THEME 1

//#define VARIATION_BASE 10.0f
#define VARIATION_BASE 4.0f

char WallTheme::themeRefName[THEME_REF_COUNT][40] = {
  "wall",
  "corner",
  "door_ew",
  "door_ns",
  "passage_floor",
  "room_floor",
  "headboard"
};

WallTheme::WallTheme( char *name, Shapes *shapePal ) {
  this->name = name;
  this->shapePal = shapePal;
  for(int i = 0; i < THEME_REF_COUNT; i++)
    themeRefMap[ themeRefName[i] ] = i;
}

WallTheme::~WallTheme() {
  unload();
}

void WallTheme::load() {
//  cerr << "*** Loading theme: " << getName() << endl;
//  debug();
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
//  cerr << "*** Dumping theme: " << getName() << endl;
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


Shapes::Shapes( bool headless ){
  texture_count = 0;
  textureGroupCount = 0;
  themeCount = allThemeCount = 0;
  currentTheme = NULL;
  this->headless = headless;
}

void Shapes::initialize() {
  // load textures
  ripple_texture = loadGLTextures("/ripple.bmp");
  torchback = loadGLTextures("/torchback.bmp");

  // load the texture info
  char errMessage[500];
  char s[200];
  sprintf(s, "%s/world/shapes.txt", rootDir);
  cerr << "*** rootDir=" << rootDir << "\n*** s=" << s << endl;
  FILE *fp = fopen(s, "r");
  if(!fp) {        
    sprintf(errMessage, "Unable to find the file: %s!", s);
    cerr << errMessage << endl;
    exit(1);
  }

  //int sum = 0;
  char line[255];  
  int n = fgetc(fp);
  while(n != EOF) {
    if( ( n = interpretShapesLine( fp, n ) ) == -1 ) {
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
    ShapeValues *sv = shapeValueVector[i];
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
      if( !headless ) shapes[(i + 1)]->initialize();
    }

    // set the effect
    shapes[ ( i + 1 ) ]->setEffectType( sv->effectType, 
                                        sv->effectWidth, sv->effectDepth, sv->effectHeight, 
                                        sv->effectX, sv->effectY, sv->effectZ );

    shapes[ ( i + 1 ) ]->setInteractive( sv->interactive );

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
  if( !headless ) shapes[shapeCount]->initialize();
  string nameStr = shapes[shapeCount]->getName();
  shapeMap[nameStr] = shapes[shapeCount];
  shapeCount++;

  // add cave shapes (1 per dimension, flat and corner each)
  for( int i = 0; i < 4; i++ ) {
    {
      shapes[shapeCount] = 
        new GLCaveShape( textureGroup[ 0 ], 
                         CAVE_CHUNK_SIZE, CAVE_CHUNK_SIZE, MAP_WALL_HEIGHT,
                         strdup( GLCaveShape::names[ i ] ), 
                         shapeCount,
                         GLCaveShape::MODE_FLAT, 
                         GLCaveShape::DIR_N + i );
      shapes[shapeCount]->setSkipSide(false);
      shapes[shapeCount]->setStencil(true);
      shapes[shapeCount]->setLightBlocking(true);  
      //if( !headless ) shapes[shapeCount]->initialize();
      string nameStr = shapes[shapeCount]->getName();
      shapeMap[nameStr] = shapes[shapeCount];
      shapeCount++;
    }

    {
      shapes[shapeCount] = 
        new GLCaveShape( textureGroup[ 0 ], 
                         CAVE_CHUNK_SIZE, CAVE_CHUNK_SIZE, MAP_WALL_HEIGHT,
                         strdup( GLCaveShape::names[ GLCaveShape::DIR_NE + i ] ), 
                         shapeCount,
                         GLCaveShape::MODE_CORNER, 
                         GLCaveShape::DIR_NE + i );
      shapes[shapeCount]->setSkipSide(false);
      shapes[shapeCount]->setStencil(true);
      shapes[shapeCount]->setLightBlocking(true);  
      //if( !headless ) shapes[shapeCount]->initialize();
      string nameStr = shapes[shapeCount]->getName();
      shapeMap[nameStr] = shapes[shapeCount];
      shapeCount++;
    }

    {
      shapes[shapeCount] = 
        new GLCaveShape( textureGroup[ 0 ], 
                         CAVE_CHUNK_SIZE, CAVE_CHUNK_SIZE, MAP_WALL_HEIGHT,
                         strdup( GLCaveShape::inverseNames[ GLCaveShape::DIR_NE + i ] ), 
                         shapeCount,
                         GLCaveShape::MODE_INV, 
                         GLCaveShape::DIR_NE + i );
      shapes[shapeCount]->setSkipSide(false);
      shapes[shapeCount]->setStencil(true);
      shapes[shapeCount]->setLightBlocking(true);  
      //if( !headless ) shapes[shapeCount]->initialize();
      string nameStr = shapes[shapeCount]->getName();
      shapeMap[nameStr] = shapes[shapeCount];
      shapeCount++;
    }
  }

  {
    shapes[shapeCount] = 
      new GLCaveShape( textureGroup[ 0 ], 
                       CAVE_CHUNK_SIZE, CAVE_CHUNK_SIZE, MAP_WALL_HEIGHT,
                       strdup( GLCaveShape::names[ 8 ] ), 
                       shapeCount,
                       GLCaveShape::MODE_BLOCK, 0 );
    shapes[shapeCount]->setSkipSide(false);
    shapes[shapeCount]->setStencil(true);
    shapes[shapeCount]->setLightBlocking(true);  
    //if( !headless ) shapes[shapeCount]->initialize();
    string nameStr = shapes[shapeCount]->getName();
    shapeMap[nameStr] = shapes[shapeCount];
    shapeCount++;
  }

  {
    shapes[shapeCount] = 
      new GLCaveShape( textureGroup[ 0 ], 
                       CAVE_CHUNK_SIZE, CAVE_CHUNK_SIZE, MAP_WALL_HEIGHT,
                       strdup( GLCaveShape::names[ 9 ] ), 
                       shapeCount,
                       GLCaveShape::MODE_FLOOR, 0 );
    shapes[shapeCount]->setSkipSide(false);
    shapes[shapeCount]->setStencil(false);
    shapes[shapeCount]->setLightBlocking(false);  
    //if( !headless ) shapes[shapeCount]->initialize();
    string nameStr = shapes[shapeCount]->getName();
    shapeMap[nameStr] = shapes[shapeCount];
    shapeCount++;
  }

  {
    shapes[shapeCount] = 
      new GLCaveShape( textureGroup[ 0 ], 
                       CAVE_CHUNK_SIZE, CAVE_CHUNK_SIZE, MAP_WALL_HEIGHT,
                       strdup( GLCaveShape::inverseNames[ GLCaveShape::DIR_CROSS_NE ] ), 
                       shapeCount,
                       GLCaveShape::MODE_INV, 
                       GLCaveShape::DIR_CROSS_NE );
    shapes[shapeCount]->setSkipSide(false);
    shapes[shapeCount]->setStencil(true);
    shapes[shapeCount]->setLightBlocking(true);  
    //if( !headless ) shapes[shapeCount]->initialize();
    string nameStr = shapes[shapeCount]->getName();
    shapeMap[nameStr] = shapes[shapeCount];
    shapeCount++;
  }

  {
    shapes[shapeCount] = 
      new GLCaveShape( textureGroup[ 0 ], 
                       CAVE_CHUNK_SIZE, CAVE_CHUNK_SIZE, MAP_WALL_HEIGHT,
                       strdup( GLCaveShape::inverseNames[ GLCaveShape::DIR_CROSS_NW ] ), 
                       shapeCount,
                       GLCaveShape::MODE_INV, 
                       GLCaveShape::DIR_CROSS_NW );
    shapes[shapeCount]->setSkipSide(false);
    shapes[shapeCount]->setStencil(true);
    shapes[shapeCount]->setLightBlocking(true);  
    //if( !headless ) shapes[shapeCount]->initialize();
    string nameStr = shapes[shapeCount]->getName();
    shapeMap[nameStr] = shapes[shapeCount];
    shapeCount++;
  }


  setupAlphaBlendedBMP("/cursor.bmp", &cursor, &cursorImage);
  cursor_texture = loadGLTextureBGRA(cursor, cursorImage, GL_LINEAR);
  setupAlphaBlendedBMP("/crosshair.bmp", &crosshair, &crosshairImage);
  crosshair_texture = loadGLTextureBGRA(crosshair, crosshairImage);
  setupAlphaBlendedBMP("/attack.bmp", &attackCursor, &attackImage);
  attack_texture = loadGLTextureBGRA(attackCursor, attackImage, GL_LINEAR);
  setupAlphaBlendedBMP("/talk.bmp", &talkCursor, &talkImage);
  talk_texture = loadGLTextureBGRA(talkCursor, talkImage, GL_LINEAR);
  setupAlphaBlendedBMP("/use.bmp", &useCursor, &useImage);
  use_texture = loadGLTextureBGRA(useCursor, useImage, GL_LINEAR);
  setupAlphaBlendedBMP("/forbidden.bmp", &forbiddenCursor, &forbiddenImage);
  forbidden_texture = loadGLTextureBGRA(forbiddenCursor, forbiddenImage, GL_LINEAR);
  setupAlphaBlendedBMP("/ranged.bmp", &rangedCursor, &rangedImage);
  ranged_texture = loadGLTextureBGRA(rangedCursor, rangedImage, GL_LINEAR);
  setupAlphaBlendedBMP("/move.bmp", &moveCursor, &moveImage);
  move_texture = loadGLTextureBGRA( moveCursor, moveImage, GL_LINEAR);
}

Shapes::~Shapes(){
}

int Shapes::interpretShapesLine( FILE *fp, int n ) {

  char path[300];
  char line[255];  

  if(n == 'T') {
    // skip ':'
    fgetc(fp);
    n = Constants::readLine(line, fp);

    // texture declaration
    loadSystemTexture( line );
    return n;
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
    return n;
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
//        cerr << "added " << count << " lines of description. # of description groups=" << descriptions.size() << endl;
    }
    return n;
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

    sv->effectType = -1;
    if( n == 'E' ) {
      fgetc( fp );
      n = Constants::readLine(line, fp);
      char *p = strtok( line, "," );
      for(int i = 0; i < Constants::EFFECT_COUNT; i++) {
        if( !strcmp( p, Constants::EFFECT_NAMES[ i ] ) ) {
          sv->effectType = i;
          break;
        }
      }
      if( sv->effectType > -1 ) {
        sv->effectWidth = atoi( strtok( NULL, "," ) );
        sv->effectDepth = atoi( strtok( NULL, "," ) );
        sv->effectHeight = atoi( strtok( NULL, "," ) );
        sv->effectX = atoi( strtok( NULL, "," ) );
        sv->effectY = atoi( strtok( NULL, "," ) );
        sv->effectZ = atoi( strtok( NULL, "," ) );
      }
    }

    sv->interactive = false;
    if( n == 'I' ) {
      fgetc( fp );
      n = Constants::readLine(line, fp);
      if( line[0] == '1' ) sv->interactive = true;
    }

    // store it for now
    shapeValueVector.push_back(sv);
    return n;
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
//      cerr << "Loading md2 model: " << path << " scale: " << info->scale << endl;
    info->model = LoadMd2Model(path);

    // store it
    string s = info->name;
    old_creature_models[s] = info;
    return n;
  } else if(n == 'P') {
    fgetc(fp);
    n = Constants::readLine(line, fp);

    int index = atoi(strtok(line, ","));
//      cerr << "options for shape, index=" << index << " size=" << shapeValueVector.size() << endl;

    ShapeValues *sv = shapeValueVector[index];
    sv->skipSide = atoi(strtok(NULL, ","));
    sv->stencil = atoi(strtok(NULL, ","));
    sv->blocksLight = atoi(strtok(NULL, ","));
    return n;
  } else if( n == 'H' ) {
    fgetc(fp);
    n = Constants::readLine(line, fp);
    //cerr << "theme: " << line << " allThemeCount=" << allThemeCount << endl;
    bool special = false;
    char *m = strrchr( line, ',' );
    if( m ) {
      *m = 0;
      special = true;
    }
    WallTheme *theme = new WallTheme( strdup(line), this );
    theme->setSpecial( special );
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

    if( !special ) {
      themes[ themeCount++ ] = theme;
    }
    allThemes[ allThemeCount++ ] = theme;
    return n;
  } else if( n == 'M' ) {
    // load the character models
    fgetc( fp );
    n = Constants::readLine(line, fp);
    CharacterModelInfo *cmi = (CharacterModelInfo*)malloc( sizeof( CharacterModelInfo ) );
    strcpy( cmi->model_name, strtok( line, "," ) );
    strcpy( cmi->skin_name, strtok( NULL, "," ) );
    cmi->scale = atof( strtok( NULL, "," ) );
    character_models.push_back( cmi );
    return n;  
  }
  return -1;
}

void Shapes::loadRandomTheme() {
  //loadTheme( themes[ (int)( (float)(themeCount - 1) * rand()/RAND_MAX ) + 1 ] );
  loadTheme( themes[ (int)( (float)themeCount * rand()/RAND_MAX ) ] );
}

void Shapes::loadTheme(const char *themeName) {
//  cerr << "*** Using theme: " << themeName << endl;

  // find that theme!
  WallTheme *theme = NULL;
  for(int i = 0; i < allThemeCount; i++) {
    if( !strcmp( allThemes[i]->getName(), themeName ) ) {
      theme = allThemes[i];
      break;
    }
  }
  if( !theme ) {
    cerr << "*** Error: Can't find theme: " << themeName << endl;
    exit(1);
  }

  loadTheme( theme );
}

void Shapes::loadTheme( WallTheme *theme ) {
//  cerr << "*** Using theme: " << theme->getName() << " current=" << (!currentTheme ? "null" : currentTheme->getName()) << endl;
  if( ALWAYS_RELOAD_THEME || currentTheme != theme) {

    // unload the previous theme
    // FIXME: This could be optimized to not unload shared textures.
    if( currentTheme ) currentTheme->unload();
    
    // load new theme
    currentTheme = (WallTheme*)theme;
    currentTheme->load();

    // apply theme to shapes
//    cerr << "*** Applying theme to shapes: ***" << endl;
    GLShape::createDarkTexture( currentTheme );
    for(int i = 0; i < (int)themeShapes.size(); i++) {
      GLShape *shape = themeShapes[i];
      string ref = themeShapeRef[i];
      GLuint *textureGroup = currentTheme->getTextureGroup( ref );
//      cerr << "\tshape=" << shape->getName() << " ref=" << ref << 
//        " tex=" << textureGroup[0] << "," << textureGroup[1] << "," << textureGroup[2] << endl;  
      if( !headless ) shape->setTexture( textureGroup );

      // create extra shapes for variations
      shape->deleteVariationShapes();
      for( int i = 3; i < currentTheme->getFaceCount( ref ); i++ ) {
        shape->createVariationShape( i, textureGroup );
      }
    }

//    cerr << "**********************************" << endl;
  }
}

char *Shapes::getRandomDescription(int descriptionGroup) {
  if(descriptionGroup >= 0 && descriptionGroup < (int)descriptions.size()) {
    vector<string> *list = descriptions[descriptionGroup];
    int n = (int)((float)list->size() * rand()/RAND_MAX);
    return(char*)((*list)[n].c_str());
  }
  return NULL;
}

CLoadMD2 g_LoadMd2; 

t3DModel *Shapes::LoadMd2Model(char *file_name) {
  t3DModel *t3d = new t3DModel;    
  g_LoadMd2.ImportMD2(t3d, file_name); 
  return t3d;   
}

void Shapes::UnloadMd2Model( t3DModel *model ) {
  g_LoadMd2.DeleteMD2( model );
  delete model;
}

GLShape *Shapes::getCreatureShape( char *model_name, 
                                   char *skin_name, 
                                   float scale ) {

  //cerr << "getCreatureShape: model_name=" << model_name << " skin=" << skin_name << endl;

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
//      cerr << "&&&&&&&&&& Loading md2 model: " << path << endl;
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

  /*      
	// load monster sounds
	if( monster )
	  session->getGameAdapter()->loadMonsterSounds( model_name, 
													monster->getSoundMap( model_name ) );
  */
  }
  

  // find or load the skin
  string skin = realSkinName;
  GLuint skin_texture;
  char path[300];
  if(creature_skins.find(skin) == creature_skins.end()){
    if( !headless ) {
      sprintf(path, "%s%s", rootDir, realSkinName);
//      cerr << "&&&&&&&&&& Loading texture: " << path << endl;
      CreateTexture(&skin_texture, path, 0);
      //cerr << "&&&&&&&&&& Loaded texture: " << skin_texture << endl;
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

void Shapes::decrementSkinRefCount( char *model_name, 
                                    char *skin_name ) {
  string skin = skin_name;
  GLuint skin_texture;
  // look for old-style skin
  if (creature_skins.find(skin) == creature_skins.end()) {
    // look for new-style skin
    char realSkinName[300];
    sprintf( realSkinName, "%s/%s", model_name, skin_name );
    skin = realSkinName;
    if (creature_skins.find(skin) == creature_skins.end()) {
      cerr << "&&&&&&&&&& WARNING: could not find skin: " << realSkinName << endl;
      return;
    } else {
      skin_texture = creature_skins[skin];
    }
  } else {
    skin_texture = creature_skins[skin];
  }

  if (loaded_skins.find(skin_texture) == loaded_skins.end()) {
    cerr << "&&&&&&&&&& WARNING: could not find skin id=" << skin_texture << endl;
    return;
  }

  loaded_skins[skin_texture] = loaded_skins[skin_texture] - 1;
  //  cerr << "&&&&&&&&&& Texture ref count at load for id: " << skin_texture << 
  //	" count: " << loaded_skins[skin_texture] << endl;
  // unload texture if no more references
  if (loaded_skins[skin_texture] == 0) {
//    cerr << "&&&&&&&&&& Deleting texture: " << skin_texture << endl;
    loaded_skins.erase(skin_texture);
    creature_skins.erase(skin);
    glDeleteTextures(1, &skin_texture);
  }

  // ------------------------------------------------------
  string model = model_name;
  Md2ModelInfo *model_info;
  if (creature_models.find(model) == creature_models.end()) {
    // this is ok. It could be an old-style model (or non-monster)
//    cerr << "&&&&&&&&&& Not unloading model: " << model << endl;
    return;
  } else {
    model_info = creature_models[model];
  }

  if (loaded_models.find(model_info) == loaded_models.end()) {
    cerr << "&&&&&&&&&& WARNING: could not find model id=" << model << endl;
    return;
  }

  loaded_models[model_info] = loaded_models[model_info] - 1;
  // unload model if no more references  
  if (loaded_models[model_info] == 0) {
//    cerr << "&&&&&&&&&& Deleting model: " << model << endl;
    loaded_models.erase(model_info);
    creature_models.erase(model);

    UnloadMd2Model( model_info->model );

    free(model_info);
  }

  /*
  // unload monster sounds
  if( monster )
  session->getGameAdapter()->unloadMonsterSounds( model_name, 
                          monster->getSoundMap( model_name ) );
  */
}

// the next two methods are slow, only use during initialization
GLuint Shapes::findTextureByName(const char *filename) {
  for(int i = 0; i < texture_count; i++) {
    if(!strcasecmp(textures[i].filename, filename)) return textures[i].id;
  }
  return 0;
}

GLShape *Shapes::findShapeByName(const char *name, bool variation) {
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
    return shape->getVariationShape( n - (int)(VARIATION_BASE) );
  }
  return shape;
}

// defaults to SWORD for unknown shapes
int Shapes::findShapeIndexByName(const char *name) {
  string s;
  if(!name || !strlen(name)) s = "SWORD";
  else s = name;
  if(shapeMap.find(s) == shapeMap.end()) {
    cerr << "&&& warning: could not find shape INDEX by name " << s << endl;
    return 0;
  }
  return shapeMap[s]->getShapePalIndex();
}

bool isPowerOfTwo( GLuint n ) {
  //cerr << "n=" << n << endl;
  if( !n ) return false;
  if( n == 1 ) return true;
  int count = 0;
  for( int i = 0; i < 8; i++ ) {
    GLuint shifted = (GLuint)( n >> i );
    count += ( shifted & 1 );
    //cerr << "\tshifted=" << shifted << " &=" << ( shifted & 1 ) << " count=" << count << endl;
    if( count > 1 ) return false;
  }
  return true;
}

/* function to load in bitmap as a GL texture */
GLuint Shapes::loadGLTextures(char *filename) {

  if( headless ) return 0;

  char fn[300];
  strcpy(fn, rootDir);
  strcat(fn, filename);

  GLuint texture[1];

  /* Create storage space for the texture */
  SDL_Surface *TextureImage[1];

  /* Load The Bitmap, Check For Errors, If Bitmap's Not Found Quit */
  if( ( TextureImage[0] = SDL_LoadBMP( fn ) ) ) {

    if( TextureImage[0]->w != TextureImage[0]->h && 
        ( !isPowerOfTwo( TextureImage[0]->w ) ||
          !isPowerOfTwo( TextureImage[0]->h ) ) ) {
      fprintf(stderr, "*** Possible error: Width or Heigth not a power of 2: name=%s pitch=%d width=%d height=%d\n", 
              fn, (TextureImage[0]->pitch/3), TextureImage[0]->w, TextureImage[0]->h);
    }

    Constants::checkTexture("Shapes::loadGLTextures", 
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
//    fprintf(stderr, "\tNot found.\n");
  }
//  fprintf(stderr, "\tStored texture at: %u\n", texture[0]);

  /* Free up any memory we may have used */
  if( TextureImage[0] )
    SDL_FreeSurface( TextureImage[0] );

  return texture[0];
}

/* function to load in bitmap as a GL texture */
GLuint Shapes::loadGLTextureBGRA(SDL_Surface *surface, GLubyte *image, int glscale) {
  if( headless ) return 0;
  return loadGLTextureBGRA( surface->w, surface->h, image, glscale );
}

GLuint Shapes::loadGLTextureBGRA(int w, int h, GLubyte *image, int glscale) {
  if( headless ) return 0;

  GLuint texture[1];

  //Constants::checkTexture("Shapes::loadGLTextureBGRA", w, h);

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
                    w, h,
                    GL_BGRA, GL_UNSIGNED_BYTE, image);
  return texture[0];
}

void Shapes::swap(unsigned char & a, unsigned char & b) {
  unsigned char temp;

  temp = a;
  a    = b;
  b    = temp;

  return;
}

void Shapes::setupAlphaBlendedBMP(char *filename, SDL_Surface **surface, 
                                        GLubyte **image, int red, int green, int blue) {

  if( headless ) return;

//  cerr << "file: " << filename << " red=" << red << " green=" << green << " blue=" << blue << endl;

  GLubyte *p = NULL;
  char fn[300];
//  fprintf(stderr, "setupAlphaBlendedBMP, rootDir=%s\n", rootDir);
  strcpy(fn, rootDir);
  strcat(fn, filename);
  if(((*surface) = SDL_LoadBMP( fn ))) {

    // Rearrange the pixelData
    int width  = (*surface) -> w;
    int height = (*surface) -> h;

    if( width != height && 
        ( !isPowerOfTwo( width ) ||
          !isPowerOfTwo( height ) ) ) {
      fprintf(stderr, "*** Possible error: Width or Heigth not a power of 2: file=%s w=%d h=%d bpp=%d byte/pix=%d pitch=%d\n", 
              fn, width, height, (*surface)->format->BitsPerPixel,
              (*surface)->format->BytesPerPixel, (*surface)->pitch);
    }

    unsigned char * data = (unsigned char *) ((*surface) -> pixels);         // the pixel data

    p = (GLubyte*)malloc(width * height * 4 * sizeof( GLubyte ));
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

      p[count++] = r;
      p[count++] = g;
      p[count++] = b;
      //(*image)[count++] = (GLubyte)( (float)(b + g + r) / 3.0f );
      //(*image)[count++] = (GLubyte)( (b + g + r == 0 ? 0x00 : 0xff) );
      p[count++] = (GLubyte)( ((int)r == blue && (int)g == green && (int)b == red ? 0x00 : 0xff) );
    }
  }

  (*image) = p;
}

void Shapes::setupAlphaBlendedBMPGrid( char *filename, SDL_Surface **surface, 
                                             GLubyte *image[20][20], int imageWidth, int imageHeight,
                                             int tileWidth, int tileHeight, 
                                             int red, int green, int blue,
                                             int nred, int ngreen, int nblue ) {
  if( headless ) return;
  
//  cerr << "file: " << filename << " red=" << red << " green=" << green << " blue=" << blue << endl;

  //  *image = NULL;
  char fn[300];
  strcpy(fn, rootDir);
  strcat(fn, filename);
  if(((*surface) = SDL_LoadBMP( fn ))) {

    // Rearrange the pixelData
    int width  = (*surface) -> w;
    int height = (*surface) -> h;

//    fprintf(stderr, "*** file=%s w=%d h=%d bpp=%d byte/pix=%d pitch=%d\n", 
//            fn, width, height, (*surface)->format->BitsPerPixel,
//            (*surface)->format->BytesPerPixel, (*surface)->pitch);

//    fprintf( stderr, "*** w/tileWidth=%d h/tileHeight=%d\n",
//             ( width/tileWidth ), ( height/tileHeight ) );

    unsigned char * data = (unsigned char *) ((*surface) -> pixels);         // the pixel data
    
    for( int x = 0; x < width / tileWidth; x++ ) {
      if( x >= imageWidth ) continue;
      for( int y = 0; y < height / tileHeight; y++ ) {
        if( y >= imageHeight ) continue;

        image[ x ][ y ] = (unsigned char*)malloc( tileWidth * tileHeight * 4 );
        int count = 0;
        // where the tile starts in a line
        int offs = x * tileWidth * (*surface)->format->BytesPerPixel;
        // where the tile ends in a line
        int rest = ( x + 1 ) * tileWidth * (*surface)->format->BytesPerPixel;
        int c = offs + ( y * tileHeight * (*surface)->pitch );
        unsigned char r,g,b,n;
        // the following lines extract R,G and B values from any bitmap
        for(int i = 0; i < tileWidth * tileHeight; ++i) {

          if( i > 0 && i % tileWidth == 0 ) {
            // skip the rest of the line
            c += ( (*surface)->pitch - rest );
            // skip the offset (go to where the tile starts)
            c += offs;
          }

          // FIXME: make this more generic...
          if( (*surface)->format->BytesPerPixel == 1 ) {
            n = data[c++];
            r = (*surface)->format->palette->colors[n].b;
            g = (*surface)->format->palette->colors[n].g;
            b = (*surface)->format->palette->colors[n].r;
          } else {
            r = data[c++];
            g = data[c++];
            b = data[c++];
          }

          //if( i == 0 ) cerr << "r=" << (int)(r) << " g=" << (int)(g) << " b=" << (int)(b) << endl;
          
          image[ x ][ y ][count++] = ( r == red && nred > -1 ? nred : r );
          image[ x ][ y ][count++] = ( g == green && ngreen > -1 ? ngreen : g );
          image[ x ][ y ][count++] = ( b == blue && nblue > -1 ? nblue : b );

          //(*image)[count++] = (GLubyte)( (float)(b + g + r) / 3.0f );
          //(*image)[count++] = (GLubyte)( (b + g + r == 0 ? 0x00 : 0xff) );
          image[ x ][ y ][count++] = (GLubyte)( ((int)r == blue && 
                                          (int)g == green && 
                                          (int)b == red ? 
                                          0x00 : 
                                          0xff) );
        }
      }
    }
  }
}

GLuint Shapes::loadSystemTexture( char *line ) {
  if( texture_count >= MAX_SYSTEM_TEXTURE_COUNT ) {
    cerr << "Error: *** no more room for system textures!. Not loading: " << line << endl;
    return 0;
  }

  GLuint id = findTextureByName( line );
  if( !id ) {
    char path[300];
    strcpy( textures[texture_count].filename, line );
    sprintf( path, "/%s", textures[texture_count].filename );
    // load the texture
    id = textures[ texture_count ].id = loadGLTextures( path );
    texture_count++;
  }
  return id;
}

GLuint Shapes::getCursorTexture( int cursorMode ) {
  switch( cursorMode ) {
  case Constants::CURSOR_NORMAL: 
    return cursor_texture;
  case Constants::CURSOR_ATTACK:
    return attack_texture;
  case Constants::CURSOR_TALK:
    return talk_texture;
  case Constants::CURSOR_USE:
    return use_texture;
  case Constants::CURSOR_FORBIDDEN:
    return forbidden_texture;
  case Constants::CURSOR_RANGED:
    return ranged_texture;
  case Constants::CURSOR_MOVE:
    return move_texture;
  default:
  return crosshair_texture;
  }
}

