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
#include "configlang.h"

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

  // configure
  ConfigLang *config = ConfigLang::load( "config/scourge.cfg" );  
  initAbout( config );  
  delete config;

	config = ConfigLang::load( "config/ui.cfg" );  
	initFonts( config );
	initCursor( config );
	delete config;

  config = ConfigLang::load( "config/pcmodel.cfg" );
	initPcPortraits( config );
	initPcModels( config );
  delete config;

  config = ConfigLang::load( "config/map.cfg" );
	initRugs( config );
  initSystemTextures( config );
	initThemes( config );
	initNativeShapes( config );
  delete config;




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
  setupAlphaBlendedBMP( "/textures/io.bmp", &tmpSurface, &tmpImage );
  ioTexture = loadGLTextureBGRA( tmpSurface, tmpImage, GL_LINEAR );
  if( tmpImage ) free( tmpImage );
  if( tmpSurface ) SDL_FreeSurface( tmpSurface );
	
  tmpSurface = NULL;
  tmpImage = NULL;
  setupAlphaBlendedBMP( "/textures/system.bmp", &tmpSurface, &tmpImage );
	systemTexture = loadGLTextureBGRA( tmpSurface, tmpImage, GL_LINEAR );
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

	thirstIcon = loadGLTextures( "/icons/t.bmp" );
	hungerIcon = loadGLTextures( "/icons/h.bmp" );

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

void ShapePalette::initFonts( ConfigLang *config ) {
  vector<ConfigNode*> *fonts = config->getDocument()->getChildrenByName( "fonts" );
  ConfigNode *fontsNode = (*fonts)[0];

  string fontNames[] = { "normal", "ui", "mono", "large" };
  for( int i = 0; i < 4; i++ ) {
    vector<ConfigNode*> *faces = fontsNode->getChildrenByName( fontNames[i] );
    if( faces ) {
      ConfigNode *node = (*faces)[0];
  
      SDLHandler::FontInfo *info = new SDLHandler::FontInfo();
      strcpy( info->path, node->getValueAsString( "path" ) );
      info->size = (int)node->getValueAsFloat( "size" );
      info->style = (int)node->getValueAsFloat( "style" );
      info->yoffset = (int)node->getValueAsFloat( "yoffset" );
  		info->shadowX = (int)node->getValueAsFloat( "shadowX" );
  		info->shadowY = (int)node->getValueAsFloat( "shadowY" );
      info->font = NULL;
      info->fontMgr = NULL;
  
      SDLHandler::fontInfos.push_back( info );
    }
  }
}

void ShapePalette::initCursor( ConfigLang *config ) {
	vector<ConfigNode*> *v = config->getDocument()->getChildrenByName( "cursor" );

	strcpy( cursorDir, (*v)[0]->getValueAsString( "path" ) );
	cursorWidth = (int)((*v)[0]->getValueAsFloat( "width" ));
	cursorHeight = (int)((*v)[0]->getValueAsFloat( "height" ));

	loadCursors();
}

void ShapePalette::initAbout( ConfigLang *config ) {
	vector<ConfigNode*> *v = config->getDocument()->getChildrenByName( "about" );
	string about;
	about += (*v)[0]->getValueAsString( "text1" );
	about += (*v)[0]->getValueAsString( "text2" );
	about += (*v)[0]->getValueAsString( "text3" );
	about += (*v)[0]->getValueAsString( "text4" );
  strcpy( aboutText, about.c_str() );
}

void ShapePalette::initPcPortraits( ConfigLang *config ) {
	vector<ConfigNode*> *v = config->getDocument()->
		getChildrenByName( "portraits" );
	vector<ConfigNode*> *vv = (*v)[0]->
		getChildrenByName( "portrait" );

	for( unsigned int i = 0; i < vv->size(); i++ ) {
		ConfigNode *node = (*vv)[i];
		string image = node->getValueAsString( "image" );
		string sex = node->getValueAsString( "sex" );
		if( strstr( image.c_str(), "death" ) ) {
			deathPortraitTexture = 
				loadGLTextures( (char*)image.c_str() );
		} else {
			int sexNum = ( sex == "M" ? 
										 Constants::SEX_MALE : 
										 Constants::SEX_FEMALE );
			portraitTextures[sexNum].push_back( 
				loadGLTextures( (char*)image.c_str() ) );
		}
	}
}

void ShapePalette::initPcModels( ConfigLang *config ) {
	vector<ConfigNode*> *v = config->getDocument()->
		getChildrenByName( "models" );
	vector<ConfigNode*> *vv = (*v)[0]->
		getChildrenByName( "model" );

	for( unsigned int i = 0; i < vv->size(); i++ ) {
		ConfigNode *node = (*vv)[i];
		CharacterModelInfo *cmi = (CharacterModelInfo*)malloc( sizeof( CharacterModelInfo ) );
		strcpy( cmi->model_name, node->getValueAsString( "path" ) );
		strcpy( cmi->skin_name, node->getValueAsString( "skin" ) );
		cmi->scale = node->getValueAsFloat( "scale" );
		string sex = node->getValueAsString( "sex" );
		int sexNum = ( sex == "M" ? 
									 Constants::SEX_MALE : 
									 Constants::SEX_FEMALE );
		character_models[sexNum].push_back( cmi );
	}
}

void ShapePalette::initRugs( ConfigLang *config ) {
  vector<ConfigNode*> *v = config->getDocument()->
		getChildrenByName( "rugs" );
	vector<ConfigNode*> *vv = (*v)[0]->
		getChildrenByName( "rug" );

	for( unsigned int i = 0; i < vv->size(); i++ ) {
		ConfigNode *node = (*vv)[i];

		SDL_Surface *tmpSurface = NULL;
		GLubyte *tmpImage = NULL;
		setupAlphaBlendedBMP( (char*)node->getValueAsString( "path" ), &tmpSurface, &tmpImage );
		rugs.push_back( loadGLTextureBGRA( tmpSurface, tmpImage, GL_LINEAR ) );
		if( tmpImage ) free( tmpImage );
		if( tmpSurface ) SDL_FreeSurface( tmpSurface );
  }
}

void ShapePalette::initSystemTextures( ConfigLang *config ) {
  vector<ConfigNode*> *v = config->getDocument()->
    getChildrenByName( "system_textures" );
  ConfigNode *node = (*v)[0];
  
  char tmp[3000];  
  strcpy( tmp, node->getValueAsString( "path" ) );

  char *p = strtok( tmp, "," );
  while( p ) {
    loadSystemTexture( p );
    p = strtok( NULL, "," );
  }
}

void ShapePalette::initThemes( ConfigLang *config ) {
	vector<ConfigNode*> *v = config->getDocument()->
		getChildrenByName( "themes" );
	vector<ConfigNode*> *vv = (*v)[0]->
		getChildrenByName( "theme" );

	for( unsigned int i = 0; i < vv->size(); i++ ) {
		ConfigNode *node = (*vv)[i];

		bool special = ( node->getValueAsFloat( "special" ) > 0 );
		bool cave = ( node->getValueAsFloat( "cave" ) > 0 );

		WallTheme *theme = new WallTheme( (char*)(node->getValueAsString( "name" )), this );
		theme->setSpecial( special );
		theme->setCave( cave );
		
		// read the shape ref-s
		char line[1000];
		for(int ref = 0; ref < WallTheme::THEME_REF_COUNT; ref++) {
			strcpy( line, node->getValueAsString( WallTheme::themeRefName[ ref ] ) );

			char *p = strtok( line, "," );
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
			if( i == 0 ) strcpy( line, node->getValueAsString( "medium_multitexture" ) );
			else strcpy( line, node->getValueAsString( "dark_multitexture" ) );

			char *p = strtok( line, "," );
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
		
		if( !special && !cave ) {
			themes[ themeCount++ ] = theme;
		} else if( cave ) {
			caveThemes[ caveThemeCount++ ] = theme;
		}
		allThemes[ allThemeCount++ ] = theme;
	}
}

void ShapePalette::initNativeShapes( ConfigLang *config ) {
	vector<ConfigNode*> *v = config->getDocument()->
		getChildrenByName( "native_shapes" );
	vector<ConfigNode*> *vv = (*v)[0]->
		getChildrenByName( "native_shape" );

	for( unsigned int i = 0; i < vv->size(); i++ ) {
		ConfigNode *node = (*vv)[i];

		ShapeValues *sv = new ShapeValues();


		strcpy( sv->theme, node->getValueAsString( "theme" ) );
		strcpy( sv->textures, node->getValueAsString( "textures" ) );

		/*
    // texture group		
		if( node->hasValue( "texture_group" ) ) {
			strcpy( sv->textureGroupIndex, node->getValueAsString( "texture_group" ) );
		} else {
			sprintf( sv->textureGroupIndex, "theme,%s", node->getValueAsString( "theme" ) );
		}
		*/

    sv->xrot = sv->yrot = sv->zrot = 0.0f;

    // dimensions
    sv->width = toint( node->getValueAsFloat( "x" ) );
    sv->depth = toint( node->getValueAsFloat( "y" ) );
    sv->height = toint( node->getValueAsFloat( "z" ) );

    // name
    strcpy( sv->name, node->getValueAsString( "name" ) );

    // description
    sv->descriptionIndex = toint( node->getValueAsFloat( "description" ) );

    // color
    sv->color = strtoul( node->getValueAsString( "color" ), NULL, 16 );

    // extra for torches:
//    sv->torch = -1;
//    sv->teleporter = 0;
		sv->torch = toint( node->getValueAsFloat( "torch" ) ) - 1;
		sv->teleporter = toint( node->getValueAsFloat( "teleporter" ) );
		sv->m3ds_name[0] = '\0';

    sv->effectType = -1;

    sv->interactive = ( node->getValueAsFloat( "interactive" ) > 0 ? true : false );

    sv->skipSide = toint( node->getValueAsFloat( "skip_side" ) );
    sv->stencil = toint( node->getValueAsFloat( "stencil" ) );
    sv->blocksLight = toint( node->getValueAsFloat( "light_blocking" ) );

    // store it for now
    shapeValueVector.push_back(sv);

		/*
		
			fixme: make sure everything is initialized in ShapeValues typedef.
		
			char theme[40];
			char textures[255];
			int width, height, depth;
			char name[100];
			int descriptionIndex;
			long color;
			int skipSide, stencil, blocksLight;
			int torch;
			char m3ds_name[100];
			float m3ds_scale;
			float m3ds_x, m3ds_y, m3ds_z;
			int teleporter;
			float xrot, yrot, zrot;
			int effectType;
			int effectWidth, effectDepth, effectHeight;
			int effectX, effectY, effectZ;
			bool interactive;
	*/
	}
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

void ShapePalette::decrementSkinRefCountAndDeleteShape( char *model_name, 
																												char *skin_name,
																												GLShape *shape,
																												Monster *monster ) {
	shape->cleanup();
  loader->decrementSkinRefCount( model_name, skin_name );
  // unload monster sounds
  if( monster ) {
    session->getGameAdapter()->
      unloadMonsterSounds( model_name, 
                           monster->getSoundMap( model_name ) );
  } else {
    session->getGameAdapter()->unloadCharacterSounds( model_name );
  }
	delete shape;
	shape = NULL;
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
