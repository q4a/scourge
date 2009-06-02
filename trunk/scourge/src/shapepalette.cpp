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

#include "common/constants.h"
#include "shapepalette.h"
#include "session.h"
#include "rpg/rpglib.h"
#include "render/renderlib.h"
#include "configlang.h"
#include "sound.h"

using namespace std;

// ###### MS Visual C++ specific ###### 
#if defined(_MSC_VER) && defined(_DEBUG)
# define new DEBUG_NEW
# undef THIS_FILE
  static char THIS_FILE[] = __FILE__;
#endif 

#define UPDATE_MESSAGE N_("Loading Shapes")

ShapePalette::ShapePalette( Session *session )
		: Shapes( session ) {
	this->session = session;
	this->loader = NULL;
	strcpy( aboutText, "" );
}

void ShapePalette::preInitialize() {
	progressTexture.load( "/textures/process.png" );
	progressHighlightTexture.load( "/textures/bar.png" );
	highlight.load( "/textures/highlight.png" );

	// configure
	ConfigLang *config = ConfigLang::load( "config/scourge.cfg" );
	initAbout( config );
	delete config;

	config = ConfigLang::load( "config/ui.cfg" );
	initFonts( config );
	initCursor( config );
	initNamedTextures( config );
	initBackpack( config );
	initSlides( config );
	delete config;

	// set up the logo
	logo_texture.load( "/textures/logo2.png" );
	GLclampf pri = 0.1f; logo_texture.glPrioritize( pri );

	chain_texture.load( "/textures/chain.png" );
	pri = 0.1f; chain_texture.glPrioritize( pri );

	// set up the scourge
	//setupAlphaBlendedBMP("/textures/scourge.bmp", scourge, scourgeImage);
	//scourge_texture = getTileTexture(scourge, scourgeImage, GL_LINEAR);

	// set up the backdrop image
	scourgeBackdrop_texture.load( "/textures/scourge-backdrop.png" );
	pri = 0.1f; scourgeBackdrop_texture.glPrioritize( pri );

	gui_texture.load( "/textures/gui.png" );
	gui_texture2.load( "/textures/gui2.png" );
//  paper_doll_texture = loadGLTextures("/paperdoll.bmp");
	cloud.load( "/textures/cloud.png", false, false );
	candle.load( "/textures/candle.png" );

	border.load( "/textures/border.png" );
	border2.load( "/textures/border2.png" );

	gui_wood_texture = this->findTextureByName( "gui-wood.png", true );
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

	loader = new ModelLoader( this, isHeadless(), textureGroup[ 14 ] );

	// load textures
	gargoyle.load( "/textures/dragon.png" );

	minimap.load( "/textures/minimap.png" );
	GLclampf pri = 0.9f; minimap.glPrioritize( pri );

	minimapMask.load( "/textures/minimask.png" );
	pri = 0.9f; minimapMask.glPrioritize( pri );

	exitTexture.load( "/textures/exit.png" );
	dismiss.load( "/textures/dismiss.png" );
	options.load( "/textures/options.png" );
	group.load( "/textures/group.png" );
	backpack.load( "/textures/inventory.png" );
	waitTexture.load( "/textures/wait.png" );
	ioTexture.load( "/textures/io.png" );
	systemTexture.load( "/textures/system.png" );
	startTexture.load( "/textures/start.png" );
	pausedTexture.load( "/textures/paused.png" );
	realTimeTexture.load( "/textures/realtime.png" );
	raindropTexture.load( "/textures/raindrop.png", false, false );
	fogCloudTexture.load( "/textures/fogcloud.png", false, false );
	snowFlakeTexture.load( "/textures/snowflake.png", false, false );
	hand_attack_icon.load( "/textures/hands.png" );

	// load map textures
	initMapGrid();

	// FIXME: do something with these...
	formationTexIndex = texture_count;
	textures[texture_count++].filename = "textures/formation1.png";
	textures[texture_count++].filename = "textures/formation2.png";
	textures[texture_count++].filename = "textures/formation3.png";
	textures[texture_count++].filename = "textures/formation4.png";
	textures[texture_count++].filename = "textures/formation5.png";
	textures[texture_count++].filename = "textures/formation6.png";

	// load the status modifier icons
	for ( int i = 0; i < StateMod::STATE_MOD_COUNT; i++ ) {
		stringstream path;
		path << "/icons/i" << i << ".png";
		Texture icon;
		icon.load( path.str() );
//    cerr << "Loading stat mod icon: " << path << " found it? " << (icon ? "yes" : "no") << endl;
		if ( icon.isSpecified() ) statModIcons[i] = icon;
	}

	thirstIcon.load( "/icons/t.png" );
	hungerIcon.load( "/icons/h.png" );

	if ( !isHeadless() ) {
		// set up the backpack tiles
		loadTiles( "/textures/tiles.png", &tiles );
		for ( int x = 0; x < 20; x++ ) {
			for ( int y = 0; y < 18; y++ ) {
				tilesTex[x][y].createTile( tiles, x, y, 32, 32 );
			}
		}

		// set up the spell tiles
		loadTiles( "/textures/spells.png", &spells );
		for ( int x = 0; x < 20; x++ ) {
			for ( int y = 0; y < 18; y++ ) {
				spellsTex[x][y].createTile( spells, x, y, 32, 32 );
			}
		}
	}

	//setupAlphaBlendedBMP("/textures/paperdoll.bmp", paperDoll, paperDollImage);
}

ShapePalette::~ShapePalette() {
	delete loader;
	typedef vector<MapGridLocation*> MglVec; 
	for ( map<char, MglVec*>::iterator itm=mapGridLocationByType.begin(); itm != mapGridLocationByType.end(); ++itm ) {
		for ( MglVec::iterator itv = itm->second->begin(); itv != itm->second->end(); ++itv ) {
			delete *itv;
		}
		delete itm->second;
	}
	for ( int i = 0; i < allThemeCount; ++i ) {
		delete allThemes[ i ];
	}
	for ( size_t i = 0; i < shapeValueVector.size(); ++i ) {
		delete shapeValueVector[ i ];
	}
	for ( size_t i = 0; i < descriptions.size(); ++i ) {
		delete descriptions[ i ];
	}
	for ( size_t i = 0; i < character_models[ Constants::SEX_MALE ].size(); ++i ) {
		delete character_models[ Constants::SEX_MALE ][ i ];
	}
	for ( size_t i = 0; i < character_models[ Constants::SEX_FEMALE ].size(); ++i ) {
		delete character_models[ Constants::SEX_FEMALE ][ i ];
	}
}

// XXX: initializing something somewhere far
void ShapePalette::initFonts( ConfigLang *config ) {
	vector<ConfigNode*> *fonts = config->getDocument()->getChildrenByName( "fonts" );
	ConfigNode *fontsNode = ( *fonts )[0];

	string fontNames[] = { "normal", "ui", "mono", "large" };
	for ( int i = 0; i < 4; i++ ) {
		vector<ConfigNode*> *faces = fontsNode->getChildrenByName( fontNames[i] );
		if ( faces ) {
			ConfigNode *faceNode = ( *faces )[0];

			// find the current locale or "all"
			vector<ConfigNode*> *locales = faceNode->getChildrenByName( "locale" );
			ConfigNode *specific = NULL;
			ConfigNode *all = NULL;
			for ( unsigned int loc = 0; loc < locales->size(); loc++ ) {
				ConfigNode *current = ( *locales )[loc];
				string locale = current->getValueAsString( "locale" );
				if ( locale == "" || locale == "all" || locale == "*" ) {
					all = current;
				} else if ( Constants::scourgeLocaleName.find( locale, 0 ) == 0 ) {
					specific = current;
				}
			}

			ConfigNode *node = ( specific ? specific : all );
			if ( node ) {
				cerr << "For font >" << fontNames[i] << "< using locale >" << node->getValueAsString( "locale" ) << "<" << endl;
				SDLHandler::FontInfo *info = new SDLHandler::FontInfo();
				info->path = node->getValueAsString( "path" );
				info->size = static_cast<int>( node->getValueAsFloat( "size" ) );
				info->style = static_cast<int>( node->getValueAsFloat( "style" ) );
				info->yoffset = static_cast<int>( node->getValueAsFloat( "yoffset" ) );
				info->shadowX = static_cast<int>( node->getValueAsFloat( "shadowX" ) );
				info->shadowY = static_cast<int>( node->getValueAsFloat( "shadowY" ) );
				info->font = NULL;
				info->fontMgr = NULL;

				SDLHandler::fontInfos.push_back( info );
			}
		}
	}
}

void ShapePalette::initNamedTextures( ConfigLang *config ) {
	vector<ConfigNode*> *v = config->getDocument()->getChildrenByName( "texture" );
	for ( unsigned int i = 0; v && i < v->size(); i++ ) {
		ConfigNode *node = ( *v )[i];
		string name = node->getValueAsString( "name" );
		string value = node->getValueAsString( "value" );
		//bool grayscale = node->getValueAsBool( "grayscale" );
		bool outdoors = node->getValueAsBool( "outdoors" );
		if ( outdoors ) {
			NamedOutdoorTexture ot;
			ot.tex.load( value );
			ot.width = node->getValueAsInt( "width" );
			ot.height = node->getValueAsInt( "width" );
			outdoorNamedTextures[ name ] = ot;
		} else {
			namedTextures[ name ].load( value );
		}
	}
}

void ShapePalette::initBackpack( ConfigLang *config ) {
	for ( int i = 0; i < Constants::EQUIP_LOCATION_COUNT; i++ ) {
		equipLocationHoles[ i ].x = equipLocationHoles[ i ].y = equipLocationHoles[ i ].w = equipLocationHoles[ i ].h = 0;
	}
	vector<ConfigNode*> *v = config->getDocument()->getChildrenByName( "inventory" );
	if ( v ) {
		char tmp[255];
		for ( int i = 0; i < Constants::EQUIP_LOCATION_COUNT; i++ ) {
			char const* s = ( *v )[0]->getValueAsString( Constants::equipLocationTags[ i ] );
			if ( s ) {
				strcpy( tmp, s );
				char *p = strtok( tmp, "," );
				equipLocationHoles[ i ].x = atoi( p );
				p = strtok( NULL, "," );
				equipLocationHoles[ i ].y = atoi( p );
				p = strtok( NULL, "," );
				equipLocationHoles[ i ].w = atoi( p );
				p = strtok( NULL, "," );
				equipLocationHoles[ i ].h = atoi( p );
			} else {
				cerr << "*** Error: Can't find inventory tag: " << Constants::equipLocationTags[ i ] << endl;
			}
		}
	}
}

void ShapePalette::initCursor( ConfigLang *config ) {
	vector<ConfigNode*> *v = config->getDocument()->getChildrenByName( "cursor" );
	strcpy( cursorDir, ( *v )[0]->getValueAsString( "path" ) );

	cursorWidth = static_cast<int>( ( *v )[0]->getValueAsFloat( "width" ) );
	if ( cursorWidth <= 0 )
		cursorWidth = 48;

	cursorHeight = static_cast<int>( ( *v )[0]->getValueAsFloat( "height" ) );
	if ( cursorHeight <= 0 )
		cursorHeight = 48;

	loadCursors();
}

void ShapePalette::initSlides( ConfigLang *config ) {
  char slidesPath[255];
  char slidesPrefix[255];
  int slidesCount;

  vector<ConfigNode*> *v = config->getDocument()->getChildrenByName( "slides" );
  strcpy( slidesPath, ( *v )[0]->getValueAsString( "path" ) );
  strcpy( slidesPrefix, ( *v )[0]->getValueAsString( "prefix" ) );
  slidesCount = ( *v )[0]->getValueAsInt( "count" );

  GLclampf pri = 0.1f;

  for ( int i = 1; i < ( slidesCount + 1 ); i++ ) {
    stringstream image;
    image << string( slidesPath ) << "/" << string( slidesPrefix ) << i << ".png";
    Texture tex;
    tex.load( image.str() );
    tex.glPrioritize( pri );
    slides.push_back( tex );
  }
}

void ShapePalette::initAbout( ConfigLang *config ) {
	vector<ConfigNode*> *v = config->getDocument()->getChildrenByName( "about" );
	string about;
	about += ( *v )[0]->getValueAsString( "text1" );
	about += ( *v )[0]->getValueAsString( "text2" );
	about += ( *v )[0]->getValueAsString( "text3" );
	about += ( *v )[0]->getValueAsString( "text4" );
	strcpy( aboutText, about.c_str() );
}

void ShapePalette::initPcPortraits( ConfigLang *config ) {
	vector<ConfigNode*> *v = config->getDocument()->
	                         getChildrenByName( "portraits" );
	vector<ConfigNode*> *vv = ( *v )[0]->
	                          getChildrenByName( "portrait" );

	for ( unsigned int i = 0; i < vv->size(); i++ ) {
		ConfigNode *node = ( *vv )[i];

		session->getGameAdapter()->setUpdate( _( "Loading Shapes" ), i, vv->size() );

		string image = node->getValueAsString( "image" );
		string sex = node->getValueAsString( "sex" );
		if ( strstr( image.c_str(), "death" ) ) {
			deathPortraitTexture.load( image );
		} else {
			int sexNum = ( sex == "M" ?
			               Constants::SEX_MALE :
			               Constants::SEX_FEMALE );
			Texture tex;
			tex.load( image );
			portraitTextures[sexNum].push_back( tex );
		}
	}
}

void ShapePalette::initPcModels( ConfigLang *config ) {
	vector<ConfigNode*> *v = config->getDocument()->
	                         getChildrenByName( "models" );
	vector<ConfigNode*> *vv = ( *v )[0]->
	                          getChildrenByName( "model" );

	for ( unsigned int i = 0; i < vv->size(); i++ ) {
		ConfigNode *node = ( *vv )[i];

		session->getGameAdapter()->setUpdate( _( "Loading Shapes" ), i, vv->size() );

		CharacterModelInfo *cmi = new CharacterModelInfo;
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
	vector<ConfigNode*> *vv = ( *v )[0]->
	                          getChildrenByName( "rug" );

	for ( unsigned int i = 0; i < vv->size(); i++ ) {
		ConfigNode *node = ( *vv )[i];

		session->getGameAdapter()->setUpdate( _( "Loading Shapes" ), i, vv->size() );

		//SDL_Surface *tmpSurface = NULL;
		//GLubyte *tmpImage = NULL;
		//setupAlphaBlendedBMP( node->getValueAsString( "path" ), tmpSurface, tmpImage );
		//rugs.push_back( getTileTexture( tmpSurface, tmpImage ) );
		Texture rug;
		rug.load( node->getValueAsString( "path" ), false, false );
		rugs.push_back( rug );
		//delete [] tmpImage;
		//if( tmpSurface ) SDL_FreeSurface( tmpSurface );
	}
}

void ShapePalette::initSystemTextures( ConfigLang *config ) {
	vector<ConfigNode*> *v = config->getDocument()->
	                         getChildrenByName( "system_textures" );
	ConfigNode *node = ( *v )[0];

	char tmp[3000];
	strcpy( tmp, node->getValueAsString( "path" ) );

	char *p = strtok( tmp, "," );
	while ( p ) {
		loadSystemTexture( p );
		p = strtok( NULL, "," );
	}
}

void ShapePalette::initThemes( ConfigLang *config ) {
	vector<ConfigNode*> *v = config->getDocument()->
	                         getChildrenByName( "theme" );
	for ( unsigned int i = 0; i < v->size(); i++ ) {
		ConfigNode *node = ( *v )[i];

		session->getGameAdapter()->setUpdate( _( "Loading Shapes" ), i, v->size() );

		bool special = node->getValueAsBool( "special" );
		bool cave = node->getValueAsBool( "cave" );

		WallTheme *theme = new WallTheme( node->getValueAsString( "name" ), this );
		theme->setSpecial( special );
		theme->setCave( cave );

		// read the shape ref-s
		char line[1000];
		for ( int ref = 0; ref < WallTheme::THEME_REF_COUNT; ref++ ) {
			strcpy( line, node->getValueAsString( WallTheme::themeRefName[ ref ] ) );

			char *p = strtok( line, "," );
			int i = 0;
			while ( p && i < MAX_TEXTURE_COUNT ) {
				theme->addTextureName( ref, i, ( const char * )p );
				p = strtok( NULL, "," );
				i++;
			}
			if ( i < 3 ) {
				cerr << "*** Error: theme=" << theme->getName() << " has wrong number of textures for line=" << ( ref + 1 ) << endl;
			}
			theme->setFaceCount( ref, i );
		}

		// read the multitexture info
		for ( int i = 0; i < WallTheme::MULTI_TEX_COUNT; i++ ) {
			if ( i == 0 ) strcpy( line, node->getValueAsString( "medium_multitexture" ) );
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

		if ( node->hasValue( "alt_walls" ) ) {
			strcpy( line, node->getValueAsString( "alt_walls" ) );
			char *p = strtok( line, "," );
			while ( p ) {
				theme->addAltWallTheme( p );
				p = strtok( NULL, "," );
			}
		}

		if ( cave ) {
			caveThemes[ caveThemeCount++ ] = theme;
		} else if ( !special ) {
			themes[ themeCount++ ] = theme;
		}
		allThemes[ allThemeCount++ ] = theme;
	}
	
	// load the outdoor textures info
	char line[3000];
	v = config->getDocument()->getChildrenByName( "outdoor_textures" );
	for ( unsigned int i = 0; i < v->size(); i++ ) {
		ConfigNode *node = ( *v )[i];

		session->getGameAdapter()->setUpdate( _( "Loading Shapes" ), i, v->size() );
		
		for( map<string, ConfigValue*>::iterator i = node->getValues()->begin(); i != node->getValues()->end(); ++i ) {
			string name = i->first;
			strcpy( line, node->getValueAsString( name ) );
			char *p = strtok( line, "," );
			int w = atoi( p );
			int h = atoi( strtok( NULL, "," ) );
			GroundTexture *gt = new GroundTexture( name, w, h );
			while( ( p = strtok( NULL, "," ) ) ) {
				string texture_path = p;
				gt->addTexture( texture_path );
			}
			groundTextures[ name ] = gt;
		}
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
	sv->blocksLight = toint( node->getValueAsFloat( "light_blocking" ) );
	if ( sv->ambient != "" ) {
		session->getSound()->storeAmbientObjectSound( sv->ambient );
	}
	sv->draws = false;
	strcpy( sv->refs, "" );

	// load some common values
	strcpy( sv->name, node->getValueAsString( "name" ) );

	string id = node->getValueAsString( "description" );
	if ( id.length() > 0 ) {
		if ( descriptionIndex.find( id ) != descriptionIndex.end() ) {
			sv->descriptionIndex = descriptionIndex[ id ];
		} else {
			cerr << "*** Warning: could not find description id=" << id << endl;
		}
	}

	sv->color = strtoul( node->getValueAsString( "color" ), NULL, 16 );
	sv->interactive = node->getValueAsBool( "interactive" );
	sv->ignoreHeightMap = node->getValueAsBool( "ignores_height_map" );
	sv->notFlatten = node->getValueAsBool( "not_flatten" );

	if ( node->hasValue( "rotate" ) ) {
		char rotation[128];
		strcpy( rotation, node->getValueAsString( "rotate" ) );
		sv->xrot = atof( strtok( rotation, "," ) );
		sv->yrot = atof( strtok( NULL, "," ) );
		sv->zrot = atof( strtok( NULL, "," ) );
	}

	if ( node->hasValue( "effect" ) ) {
		char effect[128];
		strcpy( effect, node->getValueAsString( "effect" ) );
		char *p = strtok( effect, "," );
		for ( int i = 0; i < Constants::EFFECT_COUNT; i++ ) {
			if ( !strcmp( p, Constants::EFFECT_NAMES[ i ] ) ) {
				sv->effectType = i;
				break;
			}
		}
		if ( sv->effectType > -1 ) {
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
	if ( strlen( temp ) ) {
		sv->iconRotX = atoi( strtok( temp, "," ) );
		sv->iconRotY = atoi( strtok( NULL, "," ) );
		sv->iconRotZ = atoi( strtok( NULL, "," ) );
	} else {
		sv->iconRotX = sv->iconRotY = sv->iconRotZ = 0;
	}

	strcpy( temp, node->getValueAsString( "icon" ) );
	if ( strlen( temp ) ) {
		string s = temp;
		sv->icon.load( s );
		sv->iconWidth = sv->icon.width() / 32;
		sv->iconHeight = sv->icon.height() / 32;
	} else {
		//sv->icon.clear();
		sv->iconWidth = sv->iconHeight = 0;
	}
	sv->roof = node->getValueAsBool( "roof" );
	sv->noFloor = node->getValueAsBool( "no_floor" );

	sv->usesAlpha = node->getValueAsBool( "uses_alpha" );

	return sv;
}

void ShapePalette::initDescriptions( ConfigLang *config ) {
	vector<ConfigNode*> *v = config->getDocument()->
	                         getChildrenByName( "descriptions" );
	vector<ConfigNode*> *vv = ( *v )[0]->
	                          getChildrenByName( "description_group" );
	for ( unsigned int i = 0; i < vv->size(); i++ ) {
		ConfigNode *node = ( *vv )[i];

		session->getGameAdapter()->setUpdate( _( "Loading Shapes" ), i, vv->size() );

		string id = node->getValueAsString( "id" );
		descriptionIndex[ id ] = descriptions.size();
		vector<string> *list = new vector<string>();
		descriptions.push_back( list );

		vector<ConfigNode*> *vvv =
		  node->getChildrenByName( "description" );
		for ( unsigned int t = 0; t < vvv->size(); t++ ) {
			ConfigNode *node = ( *vvv )[t];
			string s = node->getValueAsString( "text" );
			list->push_back( s );
		}
	}
}

void ShapePalette::initSounds( ConfigLang *config ) {
	vector<ConfigNode*> *v = config->getDocument()->
	                         getChildrenByName( "sounds" );
	vector<ConfigNode*> *vv = ( *v )[0]->
	                          getChildrenByName( "sound" );
	for ( unsigned int i = 0; i < vv->size(); i++ ) {
		ConfigNode *node = ( *vv )[i];

		session->getGameAdapter()->setUpdate( _( "Loading Sounds" ), i, vv->size() );

		string name = node->getValueAsString( "name" );
		string sound = node->getValueAsString( "sound" );
		getSession()->getSound()->storeSound( name, sound );
	}
}

void ShapePalette::initVirtualShapes( ConfigLang *config ) {
	vector<ConfigNode*> *v = config->getDocument()->
	                         getChildrenByName( "3ds_shapes" );
	vector<ConfigNode*> *vv = ( *v )[0]->
	                          getChildrenByName( "virtual_shape" );

	for ( unsigned int i = 0; i < vv->size(); i++ ) {
		ConfigNode *node = ( *vv )[i];

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
		if ( strlen( tmp ) ) {
			sv->o3ds_x = atof( strtok( tmp, "," ) );
			sv->o3ds_y = atof( strtok( NULL, "," ) );
			sv->o3ds_z = atof( strtok( NULL, "," ) );
		}

		strcpy( sv->refs, node->getValueAsString( "refs" ) );
		sv->draws = node->getValueAsBool( "draws" );

		// store it for now
		shapeValueVector.push_back( sv );
	}
}

void ShapePalette::init3dsShapes( ConfigLang *config ) {
	vector<ConfigNode*> *v = config->getDocument()->
	                         getChildrenByName( "3ds_shapes" );
	vector<ConfigNode*> *vv = ( *v )[0]->
	                          getChildrenByName( "3ds_shape" );

	for ( unsigned int i = 0; i < vv->size(); i++ ) {
		ConfigNode *node = ( *vv )[i];

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
		if ( strlen( tmp ) ) {
			sv->o3ds_x = atof( strtok( tmp, "," ) );
			sv->o3ds_y = atof( strtok( NULL, "," ) );
			sv->o3ds_z = atof( strtok( NULL, "," ) );
		}

		if ( node->hasValue( "rotate_3d" ) ) {
			strcpy( tmp, node->getValueAsString( "rotate_3d" ) );
			if ( strlen( tmp ) ) {
				sv->xrot3d = atof( strtok( tmp, "," ) );
				sv->yrot3d = atof( strtok( NULL, "," ) );
				sv->zrot3d = atof( strtok( NULL, "," ) );
			}
		}

		if ( node->hasValue( "lighting" ) ) {
			strcpy( tmp, node->getValueAsString( "lighting" ) );
			if ( strlen( tmp ) ) {
				if ( !strcmp( tmp, "outdoors" ) ) {
					sv->lighting = GLShape::OUTDOOR_LIGHTING;
				}
			}
		}
		if ( node->hasValue( "base" ) ) {
			strcpy( tmp, node->getValueAsString( "base" ) );
			if ( strlen( tmp ) ) {
				sv->base_w = atof( strtok( tmp, "," ) );
				sv->base_h = atof( strtok( NULL, "," ) );
			}
		}

		sv->stencil = toint( node->getValueAsFloat( "stencil" ) );
		
		initOccurance( node, sv );

		// store it for now
		shapeValueVector.push_back( sv );
	}
}

void ShapePalette::initNativeShapes( ConfigLang *config ) {
	vector<ConfigNode*> *v = config->getDocument()->
	                         getChildrenByName( "native_shapes" );
	vector<ConfigNode*> *vv = ( *v )[0]->
	                          getChildrenByName( "native_shape" );

	for ( unsigned int i = 0; i < vv->size(); i++ ) {
		ConfigNode *node = ( *vv )[i];

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

		initOccurance( node, sv );

		// store it for now
		shapeValueVector.push_back( sv );
	}
}

void ShapePalette::initOccurance( ConfigNode *parent_node, ShapeValues *sv ) {
	vector<ConfigNode*> *v = parent_node->getChildrenByName( "occurs" );
	if ( v && !v->empty() ) {
		ConfigNode *node = ( *v )[0];
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

GLShape *ShapePalette::getCreatureShape( char const* model_name,
                                         char const* skin_name,
                                         float scale,
                                         Monster *monster ) {
	// load monster sounds
	if ( monster ) {
		session->getGameAdapter()->
		loadMonsterSounds( model_name,
		                   monster->getSoundMap( model_name ) );
	} else {
		session->getGameAdapter()->loadCharacterSounds( model_name );
	}
	return loader->getCreatureShape( model_name, skin_name, scale );
}

void ShapePalette::decrementSkinRefCountAndDeleteShape( char const* model_name,
                                                        char const* skin_name,
                                                        GLShape *shape,
                                                        Monster *monster ) {
	shape->cleanup();
	loader->decrementSkinRefCount( model_name, skin_name );
	// unload monster sounds
	if ( monster ) {
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
	for ( map<string, Monster*>::iterator i = Monster::monstersByName.begin();
	        i != Monster::monstersByName.end(); ++i ) {
		Monster *m = i->second;
		if ( strlen( m->getPortrait() ) > 0 ) {
			//m->setPortraitTexture( this->loadGLTextures( m->getPortrait(), true ) );
			Texture tex;
			tex.load( m->getPortrait() );
			m->setPortraitTexture( tex );
			if ( !m->getPortraitTexture().isSpecified() ) {
				cerr << "*** Warning: couldn't load monster portrait: " << m->getPortrait() << endl;
			}
		}
	}
}

void ShapePalette::initMapGrid() {
	// load the textures
	for ( int x = 0; x < Constants::MAP_GRID_TILE_WIDTH; x++ ) {
		for ( int y = 0; y < Constants::MAP_GRID_TILE_HEIGHT; y++ ) {
			char textureName[80];
			snprintf( textureName, 80, "/mapgrid/map%d-%d.png", x, y );
			//cerr << "loading: " << textureName << endl;
			mapGrid[ x ][ y ].load( textureName );
		}
	}

	char tmp[200];
	ConfigLang *config = ConfigLang::load( "config/location.cfg" );
	vector<ConfigNode*> *v = config->getDocument()->
	                         getChildrenByName( "location" );

	for ( unsigned int i = 0; i < v->size(); i++ ) {
		ConfigNode *node = ( *v )[i];

		config->setUpdate( _( "Loading Locations" ), i, v->size() );

		MapGridLocation *loc = new MapGridLocation;
		strcpy( loc->name, node->getValueAsString( "name" ) );
		strcpy( tmp, node->getValueAsString( "position" ) );
		loc->x = atoi( strtok( tmp, "," ) );
		loc->y = atoi( strtok( NULL, "," ) );
		loc->type = node->getValueAsString( "type" )[0] + ( 'A' - 'a' );
		loc->random = node->getValueAsBool( "random" );

		if ( mapGridLocationByType.find( loc->type ) == mapGridLocationByType.end() ) {
			mapGridLocationByType[ loc->type ] = new vector<MapGridLocation*>();
		}
		mapGridLocationByType[ loc->type ]->push_back( loc );
	}
	delete config;
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
	if ( mapGridLocationByType.find( type ) == mapGridLocationByType.end() ) {
		return false;
	} else {
		vector<MapGridLocation*> *positions = mapGridLocationByType[ type ];
		MapGridLocation *pos = ( *positions )[ Util::dice( positions->size() ) ];
		if ( name ) {
			*name = pos->name;
		}
		*x = pos->x;
		*y = pos->y;
		return true;
	}
}

struct ShapeLimit {
	GLShape *shape;
	float start, end;
};
vector<ShapeLimit> trees;

GLShape *ShapePalette::getRandomTreeShape( ShapePalette *shapePal ) {
	if ( trees.empty() ) {
		float offs = 0;
		for ( int i = 1; i < shapePal->getShapeCount(); i++ ) {
			Shape *shape = shapePal->getShape( i );
			if ( shape->getOutdoorWeight() > 0 ) {
				ShapeLimit limit;
				limit.start = offs;
				offs += shape->getOutdoorWeight();
				limit.end = offs;
				limit.shape = ( GLShape* )shape;
				trees.push_back( limit );
			}
		}
	}
	assert( !trees.empty() );

	float roll = Util::roll( 0.0f, trees[ trees.size() - 1 ].end - 0.001f );

	// FIXME: implement binary search here
	for ( unsigned int i = 0; i < trees.size(); i++ ) {
		if ( trees[i].start <= roll && roll < trees[i].end ) {
			return trees[i].shape;
		}
	}
	cerr << "Unable to find tree shape! roll=" << roll << " max=" << trees[ trees.size() - 1 ].end << endl;
	cerr << "--------------------" << endl;
	for ( unsigned int i = 0; i < trees.size(); i++ ) {
		cerr << "\t" << trees[i].shape->getName() << " " << trees[i].start << "-" << trees[i].end << endl;
	}
	cerr << "--------------------" << endl;
	return NULL;
}
