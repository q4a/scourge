/***************************************************************************
            shapepalette.cpp  -  Loads and inits textures/models
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
#include "sound.h"

using namespace std;

#define UPDATE_MESSAGE N_("Loading Shapes")

ShapePalette::ShapePalette( Session *session ) 
             : Shapes( session ) 
             , paperDoll( NULL )
             , paperDollImage( NULL )
             , logo( NULL )
             , logoImage( NULL )
             , chain( NULL )
             , chainImage( NULL )
{
  this->session = session;
	this->loader = NULL;
  strcpy( aboutText, "" );
}

void ShapePalette::preInitialize() {
  SDL_Surface *tmpSurface = NULL;
  GLubyte *tmpImage = NULL;
  setupAlphaBlendedBMP("/textures/process.bmp", tmpSurface, tmpImage);
  progressTexture = loadGLTextureBGRA(tmpSurface, tmpImage, true);
  delete [] tmpImage;
  if(tmpSurface) SDL_FreeSurface( tmpSurface );

  setupAlphaBlendedBMP("/textures/bar.bmp", tmpSurface, tmpImage);
  progressHighlightTexture = loadGLTextureBGRA(tmpSurface, tmpImage, true);
  delete [] tmpImage;
  if(tmpSurface) SDL_FreeSurface( tmpSurface );



  highlight = loadGLTextures("/textures/highlight.bmp", true);


  // configure
  ConfigLang *config = ConfigLang::load( "config/scourge.cfg" );  
  initAbout( config );  
  delete config;

	config = ConfigLang::load( "config/ui.cfg" );  
	initFonts( config );
	initCursor( config );
	initNamedTextures( config );
	initInventory( config );
	delete config;

  // set up the logo
  setupAlphaBlendedBMP("/textures/logo2.bmp", logo, logoImage);
  logo_texture = loadGLTextureBGRA(logo, logoImage, true);
  GLclampf pri = 0.1f; glPrioritizeTextures(1, &logo_texture, &pri);

  setupAlphaBlendedBMP("/textures/chain.bmp", chain, chainImage);
  chain_texture = loadGLTextureBGRA(chain, chainImage, true);
  pri = 0.1f; glPrioritizeTextures(1, &chain_texture, &pri);

  // set up the scourge
  //setupAlphaBlendedBMP("/textures/scourge.bmp", scourge, scourgeImage);
  //scourge_texture = loadGLTextureBGRA(scourge, scourgeImage, GL_LINEAR);

  // set up the backdrop image
  setupAlphaBlendedBMP("/textures/scourge-backdrop.bmp", scourgeBackdrop, scourgeImageBackdrop);
  scourgeBackdrop_texture = loadGLTextureBGRA(scourgeBackdrop, scourgeImageBackdrop, true);
  pri = 0.1f; glPrioritizeTextures(1, &scourgeBackdrop_texture, &pri);

  gui_texture = loadGLTextures("/textures/gui.bmp", true);
  gui_texture2 = loadGLTextures("/textures/gui2.bmp", true);
//  paper_doll_texture = loadGLTextures("/paperdoll.bmp");
  cloud = loadGLTextures("/textures/cloud.bmp");
  candle = loadGLTextures("/textures/candle.bmp");

  border = loadGLTextures("/textures/border.bmp", true);
  border2 = loadGLTextures("/textures/border2.bmp", true);

  gui_wood_texture = this->findTextureByName( "gui-wood.bmp", true );
}

void ShapePalette::initialize() {
  ConfigLang *config = ConfigLang::load( "config/pcmodel.cfg" );
	initPcPortraits( config );
	initPcModels( config );
  delete config;

  config = ConfigLang::load( "config/map.cfg" );
	initRugs( config );
  initSystemTextures( config );
	initDescriptions( config );
	initNativeShapes( config );
	init3dsShapes( config );
	//initVirtualShapes( config );
	initSounds( config );
  delete config;

	config = ConfigLang::load( "config/themes.cfg" );
	initThemes( config );
	delete config;



  // call "super"
  Shapes::initialize();

	loader = new ModelLoader( isHeadless(), textureGroup[ 14 ] );

  // load textures
  SDL_Surface *tmpSurface = NULL;
  
  GLubyte *tmpImage = NULL;
  setupAlphaBlendedBMP("/textures/dragon.bmp", tmpSurface, tmpImage);
  gargoyle = loadGLTextureBGRA(tmpSurface, tmpImage, true);
  delete [] tmpImage;
  if(tmpSurface) SDL_FreeSurface( tmpSurface );

  tmpSurface = NULL;
  tmpImage = NULL;
  setupAlphaBlendedBMP("/textures/minimap.bmp", tmpSurface, tmpImage);
  minimap = loadGLTextureBGRA(tmpSurface, tmpImage, true);
  GLclampf pri = 0.9f; glPrioritizeTextures(1, &minimap, &pri);
  delete [] tmpImage;
  if(tmpSurface) SDL_FreeSurface( tmpSurface );

  tmpSurface = NULL;
  tmpImage = NULL;
  setupAlphaBlendedBMP("/textures/minimask.bmp", tmpSurface, tmpImage);
  minimapMask = loadGLTextureBGRA(tmpSurface, tmpImage, true);
  pri = 0.9f; glPrioritizeTextures(1, &minimapMask, &pri);
  delete [] tmpImage;
  if(tmpSurface) SDL_FreeSurface( tmpSurface );

	tmpSurface = NULL;
  tmpImage = NULL;
  setupAlphaBlendedBMP("/textures/exit.bmp", tmpSurface, tmpImage);
	exitTexture = loadGLTextureBGRA(tmpSurface, tmpImage, true);
  delete [] tmpImage;
  if(tmpSurface) SDL_FreeSurface( tmpSurface );

	tmpSurface = NULL;
  tmpImage = NULL;
  setupAlphaBlendedBMP( "/textures/dismiss.bmp", tmpSurface, tmpImage );
  dismiss = loadGLTextureBGRA( tmpSurface, tmpImage, true );
  delete [] tmpImage;
  if( tmpSurface ) SDL_FreeSurface( tmpSurface );

	tmpSurface = NULL;
  tmpImage = NULL;
  setupAlphaBlendedBMP( "/textures/options.bmp", tmpSurface, tmpImage );
  options = loadGLTextureBGRA( tmpSurface, tmpImage, true );
  delete [] tmpImage;
  if( tmpSurface ) SDL_FreeSurface( tmpSurface );

	tmpSurface = NULL;
  tmpImage = NULL;
  setupAlphaBlendedBMP( "/textures/group.bmp", tmpSurface, tmpImage );
  group = loadGLTextureBGRA( tmpSurface, tmpImage, true );
  delete [] tmpImage;
  if( tmpSurface ) SDL_FreeSurface( tmpSurface );

	tmpSurface = NULL;
  tmpImage = NULL;
  setupAlphaBlendedBMP( "/textures/inventory.bmp", tmpSurface, tmpImage );
  inventory = loadGLTextureBGRA( tmpSurface, tmpImage, true );
  delete [] tmpImage;
  if( tmpSurface ) SDL_FreeSurface( tmpSurface );

  tmpSurface = NULL;
  tmpImage = NULL;
  setupAlphaBlendedBMP( "/textures/wait.bmp", tmpSurface, tmpImage );
  waitTexture = loadGLTextureBGRA( tmpSurface, tmpImage, true );
  delete [] tmpImage;
  if( tmpSurface ) SDL_FreeSurface( tmpSurface );

  tmpSurface = NULL;
  tmpImage = NULL;
  setupAlphaBlendedBMP( "/textures/io.bmp", tmpSurface, tmpImage );
  ioTexture = loadGLTextureBGRA( tmpSurface, tmpImage, true );
  delete [] tmpImage;
  if( tmpSurface ) SDL_FreeSurface( tmpSurface );
	
  tmpSurface = NULL;
  tmpImage = NULL;
  setupAlphaBlendedBMP( "/textures/system.bmp", tmpSurface, tmpImage );
	systemTexture = loadGLTextureBGRA( tmpSurface, tmpImage, true );
  delete [] tmpImage;
  if( tmpSurface ) SDL_FreeSurface( tmpSurface );

	tmpSurface = NULL;
  tmpImage = NULL;
  setupAlphaBlendedBMP( "/textures/start.bmp", tmpSurface, tmpImage );
	startTexture = loadGLTextureBGRA( tmpSurface, tmpImage, true );
  delete [] tmpImage;
  if( tmpSurface ) SDL_FreeSurface( tmpSurface );

	tmpSurface = NULL;
  tmpImage = NULL;
  setupAlphaBlendedBMP( "/textures/paused.bmp", tmpSurface, tmpImage );
	pausedTexture = loadGLTextureBGRA( tmpSurface, tmpImage, true );
  delete [] tmpImage;
  if( tmpSurface ) SDL_FreeSurface( tmpSurface );

	tmpSurface = NULL;
  tmpImage = NULL;
  setupAlphaBlendedBMP( "/textures/realtime.bmp", tmpSurface, tmpImage );
	realTimeTexture = loadGLTextureBGRA( tmpSurface, tmpImage, true );
  delete [] tmpImage;
  if( tmpSurface ) SDL_FreeSurface( tmpSurface );

  tmpSurface = NULL;
  tmpImage = NULL;
  setupAlphaBlendedBMP( "/textures/raindrop.bmp", tmpSurface, tmpImage, 0, 0, 0, false, false, true );
  raindropTexture = loadGLTextureBGRA( tmpSurface, tmpImage );
  delete [] tmpImage;
  if( tmpSurface ) SDL_FreeSurface( tmpSurface );

  tmpSurface = NULL;
  tmpImage = NULL;
  setupAlphaBlendedBMP( "/textures/lightning.bmp", tmpSurface, tmpImage );
  lightningTexture = loadGLTextureBGRA( tmpSurface, tmpImage );
  delete [] tmpImage;
  if( tmpSurface ) SDL_FreeSurface( tmpSurface );

	string s = "/textures/hands.png";
	hand_attack_icon = loadAlphaTexture( s, NULL, NULL, true );

  // load map textures
  initMapGrid();

  // FIXME: do something with these...
  formationTexIndex = texture_count;
  textures[texture_count++].filename = "textures/formation1.bmp";
  textures[texture_count++].filename = "textures/formation2.bmp";
  textures[texture_count++].filename = "textures/formation3.bmp";
  textures[texture_count++].filename = "textures/formation4.bmp";
  textures[texture_count++].filename = "textures/formation5.bmp";
  textures[texture_count++].filename = "textures/formation6.bmp";

  // load the status modifier icons
  for(int i = 0; i < StateMod::STATE_MOD_COUNT; i++) {
    stringstream path;
    path << "/icons/i" << i << ".bmp";
    GLuint icon = loadGLTextures(path.str(), true);
//    cerr << "Loading stat mod icon: " << path << " found it? " << (icon ? "yes" : "no") << endl;
    if(icon) statModIcons[i] = icon;
  }

	thirstIcon = loadGLTextures( "/icons/t.bmp", true );
	hungerIcon = loadGLTextures( "/icons/h.bmp", true );

  // set up the inventory tiles
  setupAlphaBlendedBMPGrid( "/textures/tiles.bmp", &tiles, tilesImage, 20, 18, 
							32, 32, 71, 108, 108, 80, 80, 80 );
  for( int x = 0; x < 20; x++ ) {
    for( int y = 0; y < 18; y++ ) {
      tilesTex[x][y] = loadGLTextureBGRA( 32, 32, tilesImage[x][y], true );
    }
  }

  // set up the spell tiles
  setupAlphaBlendedBMPGrid( "/textures/spells.bmp", &spells, spellsImage, 20, 18, 
							32, 32, 71, 108, 108, 80, 80, 80 );
  for( int x = 0; x < 20; x++ ) {
    for( int y = 0; y < 18; y++ ) {
      spellsTex[x][y] = loadGLTextureBGRA( 32, 32, spellsImage[x][y], true );
    }
  }
  
  setupAlphaBlendedBMP("/textures/paperdoll.bmp", paperDoll, paperDollImage);
}

ShapePalette::~ShapePalette() {
	delete loader;
	if( logo ) SDL_FreeSurface( logo );
	delete [] logoImage;
	if( chain ) SDL_FreeSurface( chain );
	delete [] chainImage;
	if( paperDoll ) SDL_FreeSurface( paperDoll );
	delete [] paperDollImage;
  //    for(int i =0; i < static_cast<int>(creature_models.size()); i++){
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
    	ConfigNode *faceNode = (*faces)[0];
    	
    	// find the current locale or "all"
    	vector<ConfigNode*> *locales = faceNode->getChildrenByName( "locale" );
    	ConfigNode *specific = NULL;
    	ConfigNode *all = NULL;    	    	
    	for( unsigned int loc = 0; loc < locales->size(); loc++ ) {
    		ConfigNode *current = (*locales)[loc];    		
    		string locale = current->getValueAsString( "locale" );
    		if( locale == "" || locale == "all" || locale == "*" ) {
    			all = current;
    		} else if( Constants::scourgeLocaleName.find( locale, 0 ) == 0 ) {
    			specific = current;
    		}
    	}
    		
    	ConfigNode *node = ( specific ? specific : all );
    	if( node ) {
    		cerr << "For font >" << fontNames[i] << "< using locale >" << node->getValueAsString( "locale" ) << "<" << endl;
				SDLHandler::FontInfo *info = new SDLHandler::FontInfo();
				info->path = node->getValueAsString( "path" );
				info->size = static_cast<int>(node->getValueAsFloat( "size" ));
				info->style = static_cast<int>(node->getValueAsFloat( "style" ));
				info->yoffset = static_cast<int>(node->getValueAsFloat( "yoffset" ));
				info->shadowX = static_cast<int>(node->getValueAsFloat( "shadowX" ));
				info->shadowY = static_cast<int>(node->getValueAsFloat( "shadowY" ));
	      info->font = NULL;
	      info->fontMgr = NULL;
	  
	      SDLHandler::fontInfos.push_back( info );
    	}
    }
  }
}

void ShapePalette::initNamedTextures( ConfigLang *config ) {
//	Shapes::debugFileLoad = true;
	vector<ConfigNode*> *v = config->getDocument()->getChildrenByName( "texture" );
	for( unsigned int i = 0; v && i < v->size(); i++ ) {
		ConfigNode *node = (*v)[i];
		string name = node->getValueAsString( "name" );
		string value = node->getValueAsString( "value" );
		bool grayscale = node->getValueAsBool( "grayscale" );
		bool outdoors = node->getValueAsBool( "outdoors" );
		if( value.substr( value.size() - 4 ) == ".png" ) {
		  int w, h;		  
			if( outdoors ) {
				NamedOutdoorTexture ot;
				ot.tex = loadAlphaTexture( value, &w, &h );
				ot.width = node->getValueAsInt( "width" );
				ot.height = node->getValueAsInt( "width" );
				outdoorNamedTextures[ name ] = ot;
			} else {
				namedTextures[ name ] = loadAlphaTexture( value, &w, &h );
			}
		} else {
			if( outdoors ) { 
				NamedOutdoorTexture ot;
				ot.tex = loadTextureWithAlpha( value, 0, 0, 0, false, false, grayscale );
				ot.width = node->getValueAsInt( "width" );
				ot.height = node->getValueAsInt( "width" );
				outdoorNamedTextures[ name ] = ot;
			} else {
				namedTextures[ name ] = loadTextureWithAlpha( value, 0, 0, 0, false, false, grayscale );
			}
		}
	}
//	Shapes::debugFileLoad = false;
}

void ShapePalette::initInventory( ConfigLang *config ) {
	for( int i = 0; i < Constants::INVENTORY_COUNT; i++ ) {
		inventoryHoles[ i ].x = inventoryHoles[ i ].y = inventoryHoles[ i ].w = inventoryHoles[ i ].h = 0;
	}
	vector<ConfigNode*> *v = config->getDocument()->getChildrenByName( "inventory" );
	if( v ) {
		char tmp[255];
		for( int i = 0; i < Constants::INVENTORY_COUNT; i++ ) {
			char const* s = (*v)[0]->getValueAsString( Constants::inventoryTags[ i ] );
			if( s ) {
				strcpy( tmp, s );
				char *p = strtok( tmp, "," );
				inventoryHoles[ i ].x = atoi( p );
				p = strtok( NULL, "," );
				inventoryHoles[ i ].y = atoi( p );
				p = strtok( NULL, "," );
				inventoryHoles[ i ].w = atoi( p );
				p = strtok( NULL, "," );
				inventoryHoles[ i ].h = atoi( p );
			} else {
				cerr << "*** Error: Can't find inventory tag: " << Constants::inventoryTags[ i ] << endl;
			}
		}
	}
}

void ShapePalette::initCursor( ConfigLang *config ) {
	vector<ConfigNode*> *v = config->getDocument()->getChildrenByName( "cursor" );
	strcpy( cursorDir, (*v)[0]->getValueAsString( "path" ) );

	cursorWidth = static_cast<int>((*v)[0]->getValueAsFloat( "width" ));
	if( cursorWidth <= 0 )
		cursorWidth = 48;

	cursorHeight = static_cast<int>((*v)[0]->getValueAsFloat( "height" ));
	if( cursorHeight <= 0 )
		cursorHeight = 48;

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

		session->getGameAdapter()->setUpdate( _( "Loading Shapes" ), i, vv->size() );

		string image = node->getValueAsString( "image" );
		string sex = node->getValueAsString( "sex" );
		if( strstr( image.c_str(), "death" ) ) {
			deathPortraitTexture = loadGLTextures( image, true );
		} else {
			int sexNum = ( sex == "M" ? 
										 Constants::SEX_MALE : 
										 Constants::SEX_FEMALE );
			portraitTextures[sexNum].push_back( loadGLTextures( image, true ) );
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

		session->getGameAdapter()->setUpdate( _( "Loading Shapes" ), i, vv->size() );

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

		session->getGameAdapter()->setUpdate( _( "Loading Shapes" ), i, vv->size() );

		SDL_Surface *tmpSurface = NULL;
		GLubyte *tmpImage = NULL;
		setupAlphaBlendedBMP( node->getValueAsString( "path" ), tmpSurface, tmpImage );
		rugs.push_back( loadGLTextureBGRA( tmpSurface, tmpImage ) );
		delete [] tmpImage;
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
		getChildrenByName( "theme" );
	for( unsigned int i = 0; i < v->size(); i++ ) {
		ConfigNode *node = (*v)[i];

		session->getGameAdapter()->setUpdate( _( "Loading Shapes" ), i, v->size() );

		bool special = node->getValueAsBool( "special" );
		bool cave = node->getValueAsBool( "cave" );

		WallTheme *theme = new WallTheme( node->getValueAsString( "name" ), this );
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

		// read the outdoor theme info
		vector<ConfigNode*> *outv = node->getChildrenByName( "outdoors" );
		if( outv && outv->size() ) {
			theme->setHasOutdoor( true );
			ConfigNode *outnode = (*outv)[0];
			for(int ref = 0; ref < WallTheme::OUTDOOR_THEME_REF_COUNT; ref++) {
				if( !outnode->hasValue( WallTheme::outdoorThemeRefName[ ref ] ) ) {
					cerr << "Error: can't find outdoor line for ref=" << WallTheme::outdoorThemeRefName[ ref ] << endl;
					continue;
				}
				strcpy( line, outnode->getValueAsString( WallTheme::outdoorThemeRefName[ ref ] ) );
				char *p = strtok( line, "," );
				int w = atoi( p );
				int h = atoi( strtok( NULL, "," ) );
				theme->setOutdoorTextureDimensions( ref, w, h );
				p = strtok( NULL, "," );
				int face;
				for( face = 0; face < MAX_TEXTURE_COUNT && p; face++ ) {
					theme->addOutdoorTextureName( ref, face, p );
					p = strtok( NULL, "," );
				}
				theme->setOutdoorFaceCount( ref, face );
			}
		}

		if( node->hasValue( "alt_walls" ) ) {
			strcpy( line, node->getValueAsString( "alt_walls" ) );
			char *p = strtok( line, "," );
			while( p ) {
				theme->addAltWallTheme( p );
				p = strtok( NULL, "," );
			}
		}
		
		if( theme->getHasOutdoor() ) {
			outdoorThemes.push_back( theme );
		} else if( cave ) {
			caveThemes[ caveThemeCount++ ] = theme;			
		} else if( !special ) {
			themes[ themeCount++ ] = theme;
		}
		allThemes[ allThemeCount++ ] = theme;
	}
}

// initialize to default values
ShapeValues *ShapePalette::createShapeValues( ConfigNode *node ) {
	ShapeValues *sv = new ShapeValues();
	sv->theme[0] = 0;
	sv->wallShape = false;
	sv->textures[0] = 0;
	sv->width = sv->height = sv->depth = 0;
	sv->name[0] = 0;
	sv->descriptionIndex = 14; // no description
	sv->color = 0;
	sv->skipSide = sv->stencil = sv->blocksLight = 0;
	sv->torch = -1;
	sv->m3ds_name = "";
	sv->m3ds_scale = 0;
	sv->m3ds_x = sv->m3ds_y = sv->m3ds_z = 0;
	sv->o3ds_x = sv->o3ds_y = sv->o3ds_z = 0;  
	sv->teleporter = 0;
	sv->xrot = sv->yrot = sv->zrot = 0;
	sv->xrot3d = sv->yrot3d = sv->zrot3d = 0;
	sv->effectType = -1;
	sv->effectWidth = sv->effectDepth = sv->effectHeight = 0;
	sv->effectX = sv->effectY = sv->effectZ = 0;
	sv->interactive = false;
	sv->outdoorsWeight = 0;
	sv->outdoorShadow = false;
	sv->wind = false;
	sv->ambient = node->getValueAsString( "ambient" );
	if( sv->ambient != "" ) {
		session->getSound()->storeAmbientObjectSound( sv->ambient );
	}
	sv->draws = false;
	strcpy( sv->refs, "" );

	// load some common values
	strcpy( sv->name, node->getValueAsString( "name" ) );
	
	string id = node->getValueAsString( "description" );
	if( id.length() > 0 ) {
		if( descriptionIndex.find( id ) != descriptionIndex.end() ) {
			sv->descriptionIndex = descriptionIndex[ id ];
		} else {
			cerr << "*** Warning: could not find description id=" << id << endl;
		}
	}

	sv->color = strtoul( node->getValueAsString( "color" ), NULL, 16 );
	sv->interactive = node->getValueAsBool( "interactive" );
	sv->ignoreHeightMap = node->getValueAsBool( "ignores_height_map" );

	if( node->hasValue( "rotate" ) ) {
		char rotation[128];
		strcpy( rotation, node->getValueAsString( "rotate" ) );
		sv->xrot = atof( strtok( rotation, "," ) );
		sv->yrot = atof( strtok( NULL, "," ) );
		sv->zrot = atof( strtok( NULL, "," ) );
	}
	
	if( node->hasValue( "effect" ) ) {
		char effect[128];
		strcpy( effect, node->getValueAsString( "effect" ) );
		char *p = strtok( effect, "," );
		for( int i = 0; i < Constants::EFFECT_COUNT; i++ ) {
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

	sv->outdoorsWeight = node->getValueAsFloat( "outdoors_weight" );	
	sv->outdoorShadow = node->getValueAsBool( "outdoor_shadow" );
	sv->wind = node->getValueAsBool( "wind" );	
	sv->lighting = GLShape::NORMAL_LIGHTING;
	sv->base_w = sv->base_h = 0;

	sv->occurs.rooms_only = false;
	sv->occurs.max_count = 0;
	strcpy( sv->occurs.placement, "center" );
	strcpy( sv->occurs.use_function, "" );
	strcpy( sv->occurs.theme, "" );
	
	char temp[100];
	strcpy( temp, node->getValueAsString( "icon_rotate" ) );
	if( strlen( temp ) ) {
		sv->iconRotX = atoi( strtok( temp, "," ) );
		sv->iconRotY = atoi( strtok( NULL, "," ) );
		sv->iconRotZ = atoi( strtok( NULL, "," ) );
	} else {
		sv->iconRotX = sv->iconRotY = sv->iconRotZ = 0;
	}

	strcpy( temp, node->getValueAsString( "icon" ) );
	if( strlen( temp ) ) {
	  string s = temp;
//	  debugFileLoad = true;
	  int w, h;
	  sv->icon = loadAlphaTexture( s, &w, &h, true );
	  sv->iconWidth = w / 32;
	  sv->iconHeight = h / 32;
//	  debugFileLoad = false;
	} else {
	  sv->icon = sv->iconWidth = sv->iconHeight = 0;
	}
	sv->roof = node->getValueAsBool( "roof" );
	
	sv->usesAlpha = node->getValueAsBool( "uses_alpha" );
	
	return sv;
}

void ShapePalette::initDescriptions( ConfigLang *config ) {
	vector<ConfigNode*> *v = config->getDocument()->
		getChildrenByName( "descriptions" );
	vector<ConfigNode*> *vv = (*v)[0]->
		getChildrenByName( "description_group" );
	for( unsigned int i = 0; i < vv->size(); i++ ) {
		ConfigNode *node = (*vv)[i];

		session->getGameAdapter()->setUpdate( _( "Loading Shapes" ), i, vv->size() );
		
		string id = node->getValueAsString( "id" );
		descriptionIndex[ id ] = descriptions.size();
		vector<string> *list = new vector<string>();
		descriptions.push_back( list );

		vector<ConfigNode*> *vvv = 
			node->getChildrenByName( "description" );
		for( unsigned int t = 0; t < vvv->size(); t++ ) {
			ConfigNode *node = (*vvv)[t];
			string s = node->getValueAsString( "text" );
			list->push_back( s );
		}
	}
}

void ShapePalette::initSounds( ConfigLang *config ) {
	vector<ConfigNode*> *v = config->getDocument()->
		getChildrenByName( "sounds" );
	vector<ConfigNode*> *vv = (*v)[0]->
		getChildrenByName( "sound" );
	for( unsigned int i = 0; i < vv->size(); i++ ) {
		ConfigNode *node = (*vv)[i];

		session->getGameAdapter()->setUpdate( _( "Loading Sounds" ), i, vv->size() );
		
		string name = node->getValueAsString( "name" );
		string sound = node->getValueAsString( "sound" );
		getSession()->getSound()->storeSound( name, sound );
	}
}

void ShapePalette::initVirtualShapes( ConfigLang *config ) {
	vector<ConfigNode*> *v = config->getDocument()->
		getChildrenByName( "3ds_shapes" );
	vector<ConfigNode*> *vv = (*v)[0]->
		getChildrenByName( "virtual_shape" );

	for( unsigned int i = 0; i < vv->size(); i++ ) {
		ConfigNode *node = (*vv)[i];
		
		session->getGameAdapter()->setUpdate( _( "Loading Shapes" ), i, vv->size() );

		ShapeValues *sv = createShapeValues( node );

		// dimensions
		char tmp[100];
		strcpy( tmp, node->getValueAsString( "dimensions" ) );
		sv->width = atoi( strtok( tmp, "," ) );
		sv->depth = atoi( strtok( NULL, "," ) );
		sv->height = atoi( strtok( NULL, "," ) );

		// hack: reuse 3ds offsets for map offset of virtual shapes
		strcpy( tmp, node->getValueAsString( "offset" ) );
    if( strlen( tmp ) ) {
      sv->o3ds_x = atof( strtok( tmp, "," ) );
      sv->o3ds_y = atof( strtok( NULL, "," ) );
      sv->o3ds_z = atof( strtok( NULL, "," ) );
    }
    
    strcpy( sv->refs, node->getValueAsString( "refs" ) );
    sv->draws = node->getValueAsBool( "draws" );

    // store it for now
    shapeValueVector.push_back(sv);
	}	
}

void ShapePalette::init3dsShapes( ConfigLang *config ) {
	vector<ConfigNode*> *v = config->getDocument()->
		getChildrenByName( "3ds_shapes" );
	vector<ConfigNode*> *vv = (*v)[0]->
		getChildrenByName( "3ds_shape" );

	for( unsigned int i = 0; i < vv->size(); i++ ) {
		ConfigNode *node = (*vv)[i];

		session->getGameAdapter()->setUpdate( _( "Loading Shapes" ), i, vv->size() );

		ShapeValues *sv = createShapeValues( node );

		sv->m3ds_name = node->getValueAsString( "path" );

		// dimensions
		char tmp[100];
		strcpy( tmp, node->getValueAsString( "dimensions" ) );
		sv->m3ds_x = atof( strtok( tmp, "," ) );
		sv->m3ds_y = atof( strtok( NULL, "," ) );
		sv->m3ds_z = atof( strtok( NULL, "," ) );
    
		strcpy( tmp, node->getValueAsString( "offset" ) );
    if( strlen( tmp ) ) {
      sv->o3ds_x = atof( strtok( tmp, "," ) );
      sv->o3ds_y = atof( strtok( NULL, "," ) );
      sv->o3ds_z = atof( strtok( NULL, "," ) );
    }

		if( node->hasValue( "rotate_3d" ) ) {
			strcpy( tmp, node->getValueAsString( "rotate_3d" ) );
			if( strlen( tmp ) ) {
				sv->xrot3d = atof( strtok( tmp, "," ) );
				sv->yrot3d = atof( strtok( NULL, "," ) );
				sv->zrot3d = atof( strtok( NULL, "," ) );
			}
		}

		if( node->hasValue( "lighting" ) ) {
			strcpy( tmp, node->getValueAsString( "lighting" ) );
			if( strlen( tmp ) ) {
				if( !strcmp( tmp, "outdoors" ) ) {
					sv->lighting = GLShape::OUTDOOR_LIGHTING;
				}
			}
		}
		if( node->hasValue( "base" ) ) {
			strcpy( tmp, node->getValueAsString( "base" ) );
			if( strlen( tmp ) ) {
				sv->base_w = atof( strtok( tmp, "," ) );
				sv->base_h = atof( strtok( NULL, "," ) );
			}
		}
		
		sv->stencil = toint( node->getValueAsFloat( "stencil" ) );

		initOccurance( node, sv );
    
    // store it for now
    shapeValueVector.push_back(sv);
	}
}

void ShapePalette::initNativeShapes( ConfigLang *config ) {
	vector<ConfigNode*> *v = config->getDocument()->
		getChildrenByName( "native_shapes" );
	vector<ConfigNode*> *vv = (*v)[0]->
		getChildrenByName( "native_shape" );

	for( unsigned int i = 0; i < vv->size(); i++ ) {
		ConfigNode *node = (*vv)[i];
		
		session->getGameAdapter()->setUpdate( _( "Loading Shapes" ), i, vv->size() );

		ShapeValues *sv = createShapeValues( node );

		// dimensions
		char tmp[100];
		strcpy( tmp, node->getValueAsString( "dimensions" ) );
		sv->width = atoi( strtok( tmp, "," ) );
		sv->depth = atoi( strtok( NULL, "," ) );
		sv->height = atoi( strtok( NULL, "," ) );

		strcpy( sv->theme, node->getValueAsString( "theme" ) );
		strcpy( sv->textures, node->getValueAsString( "textures" ) );
		sv->wallShape = ( !strcmp( node->getValueAsString( "wall_shape" ), "true" ) ? true : false );

    // extra for torches:
		sv->torch = toint( node->getValueAsFloat( "torch" ) ) - 1;
		sv->teleporter = toint( node->getValueAsFloat( "teleporter" ) );
		sv->m3ds_name[0] = '\0';

    sv->skipSide = toint( node->getValueAsFloat( "skip_side" ) );
    sv->stencil = toint( node->getValueAsFloat( "stencil" ) );
    sv->blocksLight = toint( node->getValueAsFloat( "light_blocking" ) );

		initOccurance( node, sv );

    // store it for now
    shapeValueVector.push_back(sv);
	}
}

void ShapePalette::initOccurance( ConfigNode *parent_node, ShapeValues *sv ) {
	vector<ConfigNode*> *v = parent_node->getChildrenByName( "occurs" );
	if( v && !v->empty() ) {
		ConfigNode *node = (*v)[0];
		sv->occurs.rooms_only = node->getValueAsBool( "rooms_only" );
		sv->occurs.max_count = node->getValueAsInt( "max_count" );
		strcpy( sv->occurs.placement, node->getValueAsString( "placement" ) );
		strcpy( sv->occurs.use_function, node->getValueAsString( "use_function" ) );
		strcpy( sv->occurs.theme, node->getValueAsString( "theme" ) );
	} else {
		sv->occurs.rooms_only = false;
		sv->occurs.max_count = 0;
		strcpy( sv->occurs.placement, "" );
		strcpy( sv->occurs.use_function, "" );
		strcpy( sv->occurs.theme, "" );
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
      m->setPortraitTexture( this->loadGLTextures( m->getPortrait(), true ) );
      if( !m->getPortraitTexture() ) {
        cerr << "*** Warning: couldn't load monster portrait: " << m->getPortrait() << endl;
      }
    }
  }
}

void ShapePalette::initMapGrid() {
  // load the textures
  for( int x = 0; x < Constants::MAP_GRID_TILE_WIDTH; x++ ) {
    for( int y = 0; y < Constants::MAP_GRID_TILE_HEIGHT; y++ ) {
      char textureName[80];
      snprintf( textureName, 80, "/mapgrid/map%d-%d.bmp", x, y );
      //cerr << "loading: " << textureName << endl;
      mapGrid[ x ][ y ] = loadGLTextures( textureName, true );
    }
  }

  char tmp[200];
	ConfigLang *config = ConfigLang::load( "config/location.cfg" );
	vector<ConfigNode*> *v = config->getDocument()->
		getChildrenByName( "location" );

	for( unsigned int i = 0; i < v->size(); i++ ) {
		ConfigNode *node = (*v)[i];

		config->setUpdate( _("Loading Locations"), i, v->size() );

    MapGridLocation *loc = (MapGridLocation*)malloc( sizeof( MapGridLocation ) );
    strcpy( loc->name, node->getValueAsString( "name" ) );
    strcpy( tmp, node->getValueAsString( "position" ) );
    loc->x = atoi( strtok( tmp, "," ) );                   
    loc->y = atoi( strtok( NULL, "," ) );
    loc->type = node->getValueAsString( "type" )[0] + ( 'A' - 'a' );
    loc->random = node->getValueAsBool( "random" );
    
    if( mapGridLocationByType.find( loc->type ) == mapGridLocationByType.end() ) {
      mapGridLocationByType[ loc->type ] = new vector<MapGridLocation*>();
    }
    mapGridLocationByType[ loc->type ]->push_back( loc );
  }
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
    MapGridLocation *pos = (*positions)[ Util::dice( positions->size() ) ];
    if( name ) {
      *name = pos->name;
    }
    *x = pos->x;
    *y = pos->y;
    return true;
  }
}

