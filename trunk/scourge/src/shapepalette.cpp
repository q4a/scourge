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
	this->loader = NULL;
  strcpy( aboutText, "" );
}

void ShapePalette::preInitialize() {
  SDL_Surface *tmpSurface = NULL;
  GLubyte *tmpImage = NULL;
  setupAlphaBlendedBMP("/textures/process.bmp", &tmpSurface, &tmpImage);
  progressTexture = loadGLTextureBGRA(tmpSurface, tmpImage, GL_NEAREST);
  if(tmpImage) free(tmpImage);
  if(tmpSurface) SDL_FreeSurface( tmpSurface );

  setupAlphaBlendedBMP("/textures/bar.bmp", &tmpSurface, &tmpImage);
  progressHighlightTexture = loadGLTextureBGRA(tmpSurface, tmpImage, GL_LINEAR);
  if(tmpImage) free(tmpImage);
  if(tmpSurface) SDL_FreeSurface( tmpSurface );



  highlight = loadGLTextures("/textures/highlight.bmp");
}

void ShapePalette::initialize() {
  // call "super"
  Shapes::initialize();

	loader = new ModelLoader( headless, 
														 textureGroup[ 14 ] );

  // load textures
  gui_texture = loadGLTextures("/textures/gui.bmp");
  gui_texture2 = loadGLTextures("/textures/gui2.bmp");
//  paper_doll_texture = loadGLTextures("/paperdoll.bmp");
  cloud = loadGLTextures("/textures/cloud.bmp");
  candle = loadGLTextures("/textures/candle.bmp");

  border = loadGLTextures("/textures/border.bmp");
  border2 = loadGLTextures("/textures/border2.bmp");
  SDL_Surface *tmpSurface = NULL;
  
  GLubyte *tmpImage = NULL;
  setupAlphaBlendedBMP("/textures/dragon.bmp", &tmpSurface, &tmpImage);
  gargoyle = loadGLTextureBGRA(tmpSurface, tmpImage, GL_NEAREST);
  if(tmpImage) free(tmpImage);
  if(tmpSurface) SDL_FreeSurface( tmpSurface );

  tmpSurface = NULL;
  tmpImage = NULL;
  setupAlphaBlendedBMP("/textures/minimap.bmp", &tmpSurface, &tmpImage);
  minimap = loadGLTextureBGRA(tmpSurface, tmpImage, GL_LINEAR);
  if(tmpImage) free(tmpImage);
  if(tmpSurface) SDL_FreeSurface( tmpSurface );

  tmpSurface = NULL;
  tmpImage = NULL;
  setupAlphaBlendedBMP("/textures/minimask.bmp", &tmpSurface, &tmpImage);
  minimapMask = loadGLTextureBGRA(tmpSurface, tmpImage, GL_LINEAR);
  if(tmpImage) free(tmpImage);
  if(tmpSurface) SDL_FreeSurface( tmpSurface );

	tmpSurface = NULL;
  tmpImage = NULL;
  setupAlphaBlendedBMP("/textures/exit.bmp", &tmpSurface, &tmpImage);
	exitTexture = loadGLTextureBGRA(tmpSurface, tmpImage, GL_LINEAR);
  if(tmpImage) free(tmpImage);
  if(tmpSurface) SDL_FreeSurface( tmpSurface );

	tmpSurface = NULL;
  tmpImage = NULL;
  setupAlphaBlendedBMP( "/textures/dismiss.bmp", &tmpSurface, &tmpImage );
  dismiss = loadGLTextureBGRA( tmpSurface, tmpImage, GL_LINEAR );
  if( tmpImage ) free( tmpImage );
  if( tmpSurface ) SDL_FreeSurface( tmpSurface );

	tmpSurface = NULL;
  tmpImage = NULL;
  setupAlphaBlendedBMP( "/textures/options.bmp", &tmpSurface, &tmpImage );
  options = loadGLTextureBGRA( tmpSurface, tmpImage, GL_LINEAR );
  if( tmpImage ) free( tmpImage );
  if( tmpSurface ) SDL_FreeSurface( tmpSurface );

	tmpSurface = NULL;
  tmpImage = NULL;
  setupAlphaBlendedBMP( "/textures/group.bmp", &tmpSurface, &tmpImage );
  group = loadGLTextureBGRA( tmpSurface, tmpImage, GL_LINEAR );
  if( tmpImage ) free( tmpImage );
  if( tmpSurface ) SDL_FreeSurface( tmpSurface );

	tmpSurface = NULL;
  tmpImage = NULL;
  setupAlphaBlendedBMP( "/textures/inventory.bmp", &tmpSurface, &tmpImage );
  inventory = loadGLTextureBGRA( tmpSurface, tmpImage, GL_LINEAR );
  if( tmpImage ) free( tmpImage );
  if( tmpSurface ) SDL_FreeSurface( tmpSurface );

	tmpSurface = NULL;
  tmpImage = NULL;
  setupAlphaBlendedBMP( "/textures/wait.bmp", &tmpSurface, &tmpImage );
	waitTexture = loadGLTextureBGRA( tmpSurface, tmpImage, GL_LINEAR );
  if( tmpImage ) free( tmpImage );
  if( tmpSurface ) SDL_FreeSurface( tmpSurface );

	tmpSurface = NULL;
  tmpImage = NULL;
  setupAlphaBlendedBMP( "/textures/start.bmp", &tmpSurface, &tmpImage );
	startTexture = loadGLTextureBGRA( tmpSurface, tmpImage, GL_LINEAR );
  if( tmpImage ) free( tmpImage );
  if( tmpSurface ) SDL_FreeSurface( tmpSurface );

	tmpSurface = NULL;
  tmpImage = NULL;
  setupAlphaBlendedBMP( "/textures/paused.bmp", &tmpSurface, &tmpImage );
	pausedTexture = loadGLTextureBGRA( tmpSurface, tmpImage, GL_LINEAR );
  if( tmpImage ) free( tmpImage );
  if( tmpSurface ) SDL_FreeSurface( tmpSurface );

	tmpSurface = NULL;
  tmpImage = NULL;
  setupAlphaBlendedBMP( "/textures/realtime.bmp", &tmpSurface, &tmpImage );
	realTimeTexture = loadGLTextureBGRA( tmpSurface, tmpImage, GL_LINEAR );
  if( tmpImage ) free( tmpImage );
  if( tmpSurface ) SDL_FreeSurface( tmpSurface );

  // load map textures
  initMapGrid();

  // FIXME: do something with these...
  formationTexIndex = texture_count;
  strcpy(textures[texture_count++].filename, "textures/formation1.bmp");
  strcpy(textures[texture_count++].filename, "textures/formation2.bmp");
  strcpy(textures[texture_count++].filename, "textures/formation3.bmp");
  strcpy(textures[texture_count++].filename, "textures/formation4.bmp");
  strcpy(textures[texture_count++].filename, "textures/formation5.bmp");
  strcpy(textures[texture_count++].filename, "textures/formation6.bmp");

  // load the status modifier icons
  char path[ 255 ];
  for(int i = 0; i < Constants::STATE_MOD_COUNT; i++) {
    sprintf(path, "/icons/i%d.bmp", i);
    GLuint icon = loadGLTextures(path);
//    cerr << "Loading stat mod icon: " << path << " found it? " << (icon ? "yes" : "no") << endl;
    if(icon) statModIcons[i] = icon;
  }

  // set up the inventory tiles
  setupAlphaBlendedBMPGrid( "/textures/tiles.bmp", &tiles, tilesImage, 20, 18, 
							32, 32, 71, 108, 108, 80, 80, 80 );
  for( int x = 0; x < 20; x++ ) {
    for( int y = 0; y < 18; y++ ) {
      tilesTex[x][y] = loadGLTextureBGRA( 32, 32, tilesImage[x][y], GL_LINEAR );
    }
  }

  // set up the spell tiles
  setupAlphaBlendedBMPGrid( "/textures/spells.bmp", &spells, spellsImage, 20, 18, 
							32, 32, 71, 108, 108, 80, 80, 80 );
  for( int x = 0; x < 20; x++ ) {
    for( int y = 0; y < 18; y++ ) {
      spellsTex[x][y] = loadGLTextureBGRA( 32, 32, spellsImage[x][y], GL_LINEAR );
    }
  }
  
  setupAlphaBlendedBMP("/textures/paperdoll.bmp", &paperDoll, &paperDollImage);

  // set up the logo
  setupAlphaBlendedBMP("/textures/logo2.bmp", &logo, &logoImage);
  logo_texture = loadGLTextureBGRA(logo, logoImage, GL_LINEAR);
  setupAlphaBlendedBMP("/textures/chain.bmp", &chain, &chainImage);
  chain_texture = loadGLTextureBGRA(chain, chainImage, GL_LINEAR);

  // set up the scourge
  setupAlphaBlendedBMP("/textures/scourge.bmp", &scourge, &scourgeImage);

  gui_wood_texture = this->findTextureByName("gui-wood.bmp");
}

ShapePalette::~ShapePalette() {
	delete loader;
  //    for(int i =0; i < (int)creature_models.size(); i++){
  //        delete creature_models[i];    
  //    }
}

int ShapePalette::interpretShapesLine( FILE *fp, int n ) {

  // call super
  int super = Shapes::interpretShapesLine( fp, n );
  if( super != -1 ) return super;

  char line[255];
	if( n == 'F' ) {
		fgetc(fp);
    n = Constants::readLine(line, fp);
		char *p = strtok( line, "," );
		if( SDLHandler::NORMAL_FONT_SIZE == 0 ) {
			strcpy( SDLHandler::NORMAL_FONT_NAME, p );
			SDLHandler::NORMAL_FONT_SIZE = atoi( strtok( NULL, "," ) );
        } else if( SDLHandler::UI_FONT_SIZE == 0 ) {
			strcpy( SDLHandler::UI_FONT_NAME, p );
			SDLHandler::UI_FONT_SIZE = atoi( strtok( NULL, "," ) );
		} else if( SDLHandler::FIXED_FONT_SIZE == 0 ) {
			strcpy( SDLHandler::FIXED_FONT_NAME, p );
			SDLHandler::FIXED_FONT_SIZE = atoi( strtok( NULL, "," ) );
		} else if( SDLHandler::LARGE_FONT_SIZE == 0 ) {
			strcpy( SDLHandler::LARGE_FONT_NAME, p );
			SDLHandler::LARGE_FONT_SIZE = atoi( strtok( NULL, "," ) );
		} else {
			cerr << "*** Error: extra font lines ignored in shapes.txt" << endl;
		}
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
    else {
			int sex = Constants::SEX_MALE;
			if( line[ strlen( line ) - 2 ] == ',' ) {
				if( line[ strlen( line ) - 1 ] == 'F' ) sex = Constants::SEX_FEMALE;
				line[ strlen( line ) - 2 ] = '\0';
			}
			portraitTextures[sex].push_back( loadGLTextures( line ) );
		}
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
  } else {
    session->getGameAdapter()->loadCharacterSounds( model_name );
  }
  return loader->getCreatureShape( model_name, skin_name, scale );
}

void ShapePalette::decrementSkinRefCount( char *model_name, 
                                          char *skin_name,
                                          Monster *monster ) {
  loader->decrementSkinRefCount( model_name, skin_name );
  // unload monster sounds
  if( monster ) {
    session->getGameAdapter()->
      unloadMonsterSounds( model_name, 
                           monster->getSoundMap( model_name ) );
  } else {
    session->getGameAdapter()->unloadCharacterSounds( model_name );
  }
}

void ShapePalette::debugLoadedModels() {
	loader->debugModelLoader();
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

