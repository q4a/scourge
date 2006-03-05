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
#include "rpg/rpglib.h"
#include "render/renderlib.h"

using namespace std;

ShapePalette::ShapePalette( Session *session ) : Shapes( session->getGameAdapter()->isHeadless() ) {
  this->session = session;
  skillCount = 0;
  strcpy( aboutText, "" );
}

void ShapePalette::preInitialize() {
  SDL_Surface *tmpSurface = NULL;
  GLubyte *tmpImage = NULL;
  setupAlphaBlendedBMP("/process.bmp", &tmpSurface, &tmpImage);
  progressTexture = loadGLTextureBGRA(tmpSurface, tmpImage, GL_NEAREST);
  if(tmpImage) free(tmpImage);
  if(tmpSurface) SDL_FreeSurface( tmpSurface );

  setupAlphaBlendedBMP("/bar.bmp", &tmpSurface, &tmpImage);
  progressHighlightTexture = loadGLTextureBGRA(tmpSurface, tmpImage, GL_LINEAR);
  if(tmpImage) free(tmpImage);
  if(tmpSurface) SDL_FreeSurface( tmpSurface );



  highlight = loadGLTextures("/highlight.bmp");
}

void ShapePalette::initialize() {
  // call "super"
  Shapes::initialize();

  // load textures
  gui_texture = loadGLTextures("/gui.bmp");
  gui_texture2 = loadGLTextures("/gui2.bmp");
//  paper_doll_texture = loadGLTextures("/paperdoll.bmp");
  cloud = loadGLTextures("/cloud.bmp");
  candle = loadGLTextures("/candle.bmp");

  border = loadGLTextures("/border.bmp");
  border2 = loadGLTextures("/border2.bmp");
  SDL_Surface *tmpSurface = NULL;
  
  GLubyte *tmpImage = NULL;
  setupAlphaBlendedBMP("/dragon.bmp", &tmpSurface, &tmpImage);
  gargoyle = loadGLTextureBGRA(tmpSurface, tmpImage, GL_NEAREST);
  if(tmpImage) free(tmpImage);
  if(tmpSurface) SDL_FreeSurface( tmpSurface );

  tmpSurface = NULL;
  tmpImage = NULL;
  setupAlphaBlendedBMP("/minimap.bmp", &tmpSurface, &tmpImage);
  minimap = loadGLTextureBGRA(tmpSurface, tmpImage, GL_LINEAR);
  if(tmpImage) free(tmpImage);
  if(tmpSurface) SDL_FreeSurface( tmpSurface );

  tmpSurface = NULL;
  tmpImage = NULL;
  setupAlphaBlendedBMP("/minimask.bmp", &tmpSurface, &tmpImage);
  minimapMask = loadGLTextureBGRA(tmpSurface, tmpImage, GL_LINEAR);
  if(tmpImage) free(tmpImage);
  if(tmpSurface) SDL_FreeSurface( tmpSurface );

  // load map textures
  initMapGrid();

  // FIXME: do something with these...
  formationTexIndex = texture_count;
  strcpy(textures[texture_count++].filename, "formation1.bmp");
  strcpy(textures[texture_count++].filename, "formation2.bmp");
  strcpy(textures[texture_count++].filename, "formation3.bmp");
  strcpy(textures[texture_count++].filename, "formation4.bmp");
  strcpy(textures[texture_count++].filename, "formation5.bmp");
  strcpy(textures[texture_count++].filename, "formation6.bmp");

  // load the status modifier icons
  char path[ 255 ];
  for(int i = 0; i < Constants::STATE_MOD_COUNT; i++) {
    sprintf(path, "/icons/i%d.bmp", i);
    GLuint icon = loadGLTextures(path);
//    cerr << "Loading stat mod icon: " << path << " found it? " << (icon ? "yes" : "no") << endl;
    if(icon) statModIcons[i] = icon;
  }

  // set up the inventory tiles
  setupAlphaBlendedBMPGrid( "/tiles.bmp", &tiles, tilesImage, 20, 18, 
							32, 32, 71, 108, 108, 80, 80, 80 );
  for( int x = 0; x < 20; x++ ) {
    for( int y = 0; y < 18; y++ ) {
      tilesTex[x][y] = loadGLTextureBGRA( 32, 32, tilesImage[x][y], GL_LINEAR );
    }
  }

  // set up the spell tiles
  setupAlphaBlendedBMPGrid( "/spells.bmp", &spells, spellsImage, 20, 18, 
							32, 32, 71, 108, 108, 80, 80, 80 );
  for( int x = 0; x < 20; x++ ) {
    for( int y = 0; y < 18; y++ ) {
      spellsTex[x][y] = loadGLTextureBGRA( 32, 32, spellsImage[x][y], GL_LINEAR );
    }
  }
  
  setupAlphaBlendedBMP("/paperdoll.bmp", &paperDoll, &paperDollImage);

  // set up the logo
  setupAlphaBlendedBMP("/logo2.bmp", &logo, &logoImage);
  logo_texture = loadGLTextureBGRA(logo, logoImage, GL_LINEAR);
  setupAlphaBlendedBMP("/chain.bmp", &chain, &chainImage);
  chain_texture = loadGLTextureBGRA(chain, chainImage, GL_LINEAR);

  // set up the scourge
  setupAlphaBlendedBMP("/scourge.bmp", &scourge, &scourgeImage);

  gui_wood_texture = this->findTextureByName("gui-wood.bmp");
}

ShapePalette::~ShapePalette() {
  //    for(int i =0; i < (int)creature_models.size(); i++){
  //        delete creature_models[i];    
  //    }
}

int ShapePalette::interpretShapesLine( FILE *fp, int n ) {

  // call super
  int super = Shapes::interpretShapesLine( fp, n );
  if( super != -1 ) return super;

  char line[255];
  if(n == 'S') {
    // skip ':'
    fgetc(fp);
    n = Constants::readLine(line, fp);
    
    char *p = strtok( line, "," );
    Constants::SKILL_NAMES[ skillCount ] = (char*)malloc( sizeof( char ) * strlen( p ) + 1 );
    strcpy( Constants::SKILL_NAMES[ skillCount ], p );
    
    p = strtok( NULL, "," );
    Constants::SKILL_DESCRIPTION[ skillCount ] = (char*)malloc( sizeof( char ) * strlen( p ) + 1 );
    strcpy( Constants::SKILL_DESCRIPTION[ skillCount ], p );
    
    skillCount++;
    return n;
  } else if( n == 'A' ) {
    fgetc(fp);
    n = Constants::readLine(line, fp);
    if( strlen( aboutText ) ) strcat( aboutText, " " );
    strcat( aboutText, line );
    return n;
  } else if( n == 'O' ) {
    fgetc(fp);
    n = Constants::readLine(line, fp);
    //      cerr << "*** Loading portrait: " << line << endl;
    if( strstr( line, "death" ) ) deathPortraitTexture = loadGLTextures( line );
    else portraitTextures.push_back( loadGLTextures( line ) );
    return n;
  }
  return -1;
}

GLShape *ShapePalette::getCreatureShape( char *model_name, 
                                         char *skin_name, 
                                         float scale, 
                                         Monster *monster ) {
	// load monster sounds
	if( monster ) {
    session->getGameAdapter()->
    loadMonsterSounds( model_name, 
                       monster->getSoundMap( model_name ) );
  }
  return Shapes::getCreatureShape( model_name, skin_name, scale );
}

void ShapePalette::decrementSkinRefCount( char *model_name, 
                                          char *skin_name,
                                          Monster *monster ) {
  Shapes::decrementSkinRefCount( model_name, skin_name );
  // unload monster sounds
  if( monster )
	session->getGameAdapter()->unloadMonsterSounds( model_name, 
													monster->getSoundMap( model_name ) );
}

void ShapePalette::loadNpcPortraits() {
  for( map<string, Monster*>::iterator i = Monster::monstersByName.begin(); 
       i != Monster::monstersByName.end(); ++i ) {
    Monster *m = i->second;
    if( m->getPortrait() ) {
      m->setPortraitTexture( this->loadGLTextures( m->getPortrait() ) );
      if( !m->getPortraitTexture() ) {
        cerr << "*** Warning: couldn't load monster portrait: " << m->getPortrait() << endl;
      }
    }
  }
}

void ShapePalette::initMapGrid() {
  // load the textures
  char textureName[80];
  for( int x = 0; x < Constants::MAP_GRID_TILE_WIDTH; x++ ) {
    for( int y = 0; y < Constants::MAP_GRID_TILE_HEIGHT; y++ ) {
      sprintf( textureName, "/mapgrid/map%d-%d.bmp", x, y );
      //cerr << "loading: " << textureName << endl;
      mapGrid[ x ][ y ] = loadGLTextures( textureName );
    }
  }

  // load the locations
  char errMessage[500];
  char s[200];
  sprintf(s, "%s/world/locations.txt", rootDir);
  FILE *fp = fopen(s, "r");
  if(!fp) {        
    sprintf(errMessage, "Unable to find the file: %s!", s);
    cerr << errMessage << endl;
    exit(1);
  }

  char line[255];
  int n = fgetc(fp);
  while(n != EOF) {
    if(n == 'L') {
      // skip ':'
      fgetc(fp);
      n = Constants::readLine(line, fp);

      MapGridLocation *loc = (MapGridLocation*)malloc( sizeof( MapGridLocation ) );
      strcpy( loc->name, strtok( line, "," ) );
      loc->x = atoi( strtok( NULL, "," ) );
      loc->y = atoi( strtok( NULL, "," ) );
      char *p = strtok( NULL, "," );
      loc->type = *p;
      loc->random = ( p[ 1 ] == 'R' );

      if( mapGridLocationByType.find( loc->type ) == mapGridLocationByType.end() ) {
        mapGridLocationByType[ loc->type ] = new vector<MapGridLocation*>();
      }
      mapGridLocationByType[ loc->type ]->push_back( loc );
    } else {
      // skip this line
      n = Constants::readLine(line, fp);
    }
  }
  fclose(fp);
}

/**
 * Find a random location on the scourge map.
 * @param type a char depicting an arbitrary map type (eg.: C-city, D-dungeon, etc.)
 * @param name will point to the name of the location found
 * @param x the x coordinate
 * @param y the y coordinate
 * @return true if a location of type was found.
 */
bool ShapePalette::getRandomMapLocation( char type, char **name, int *x, int *y ) {
  if( mapGridLocationByType.find( type ) == mapGridLocationByType.end() ) {
    return false;
  } else {
    vector<MapGridLocation*> *positions = mapGridLocationByType[ type ];
    MapGridLocation *pos = (*positions)[ (int)( (float)(positions->size()) * rand() / RAND_MAX ) ];
    if( name ) {
      *name = pos->name;
    }
    *x = pos->x;
    *y = pos->y;
    return true;
  }
}

