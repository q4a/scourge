/***************************************************************************
                    shapes.cpp  -  Shape/texture loader
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

#include "../common/constants.h"
#include "shapes.h"
#include "glshape.h"
#include "gltorch.h"
#include "glteleporter.h"
#include "3dsshape.h"
#include "md2shape.h"
#include "Md2.h"
#include "glcaveshape.h"
#include "virtualshape.h"
#include "../session.h"
#include "../sqbinding/sqbinding.h"
#include <fstream>
#include <iostream>
#include "texture.h"


using namespace std;

// ###### MS Visual C++ specific ###### 
#if defined(_MSC_VER) && defined(_DEBUG)
# define new DEBUG_NEW
# undef THIS_FILE
  static char THIS_FILE[] = __FILE__;
#endif 

#define ALWAYS_RELOAD_THEME 1

bool Shapes::debugFileLoad = false;

char WallTheme::themeRefName[THEME_REF_COUNT][40] = {
	"wall",
	"corner",
	"door_ew",
	"door_ns",
	"passage_floor",
	"room_floor",
	"headboard",
};

char WallTheme::outdoorThemeRefName[OUTDOOR_THEME_REF_COUNT][40] = {
	"grass", "street", "street_cross", "street_end", "trail", "trail_turn", "trail_end",
	"water", "rock", "grass_edge", "grass_corner", "grass_tip", "grass_narrow", "snow",
	"snow_big", "lakebed", "extra", "street_90", "street_end_90", "street_end_180", "street_end_270"
};

WallTheme::WallTheme( char const* name, Shapes *shapePal ) {
	strcpy( this->name, name );
	this->shapePal = shapePal;
	this->hasOutdoor = false;
	for ( int i = 0; i < THEME_REF_COUNT; i++ ) {
		themeRefMap[ themeRefName[i] ] = i;
		faceCount[ i ] = 0;
	}
	for ( int i = 0; i < OUTDOOR_THEME_REF_COUNT; i++ ) {
		outdoorThemeRefMap[ outdoorThemeRefName[ i ] ] = i;
		outdoorFaceCount[ i ] = 0;
	}
}

WallTheme::~WallTheme() {
	unload();
}

void WallTheme::load() {
//  cerr << "*** Loading theme: " << getName() << endl;
//  debug();
	for ( int ref = 0; ref < THEME_REF_COUNT; ref++ ) {
		if ( faceCount[ ref ] == 0 ) {
			cerr << "No textures defined for theme. Ref=" << themeRefName[ ref ] << endl;
			continue;
		}
		for ( int face = 0; face < faceCount[ ref ]; face++ ) {
			loadTextureGroup( ref, face, textureNames[ ref ][ face ] );
		}
	}
	if ( getHasOutdoor() ) {
		for ( int ref = 0; ref < OUTDOOR_THEME_REF_COUNT; ref++ ) {
			if ( outdoorFaceCount[ ref ] == 0 ) {
				cerr << "No textures defined for outdoor theme. Ref=" << outdoorThemeRefName[ ref ] << endl;
				continue;
			}
			for ( int face = 0; face < outdoorFaceCount[ ref ]; face++ ) {
				loadTextureGroup( ref, face, outdoorTextures[ ref ][ face ], true );
			}
		}
		// create edge textures
		createOutdoorEdgeTexture( OUTDOOR_THEME_REF_GRASS_CORNER );
		createOutdoorEdgeTexture( OUTDOOR_THEME_REF_GRASS_EDGE );
		createOutdoorEdgeTexture( OUTDOOR_THEME_REF_GRASS_NARROW );
		createOutdoorEdgeTexture( OUTDOOR_THEME_REF_GRASS_TIP );
	}
}

// Overlay the current 'grass' texture on the alpha-blended edge texture to create a theme-specific blend.
void WallTheme::createOutdoorEdgeTexture( int ref ) {
	Texture tex;
	tex.createAlpha( outdoorTextureGroup[ ref ][ 0 ], outdoorTextureGroup[ OUTDOOR_THEME_REF_GRASS ][ 0 ] );
	GLclampf pri = 0.9f;
	tex.glPrioritize( pri );
	outdoorTextureGroup[ ref ][ 0 ] = tex;
}

void WallTheme::loadTextureGroup( int ref, int face, char *texture, bool outdoor ) {
	//cerr << "Loading theme texture. Theme: " << getName() << " ref=" << ( outdoor ? outdoorThemeRefName[ ref ] : themeRefName[ ref ] ) << " face=" << face << " texture=" << texture << endl;
	string path;
	Texture id;
	if ( texture && strcmp( texture, "null" ) ) {
		string s = texture;
		if ( loadedTextures.find( s ) == loadedTextures.end() ) {

			// see if it's a system texture (no need to double load it)
			string bmp = texture;
			if ( bmp.find( ".", 0 ) == string::npos ) {
				bmp += string( ".png" );
			}
			path = "/" + bmp;

			/* lavaData is nowhere used
			// keep lava texture data
			if ( isCave() &&
			        ref == THEME_REF_PASSAGE_FLOOR &&
			        face == GLShape::FRONT_SIDE ) {
					shapePal->getBMPData( path, lavaData );
			} */
			if ( isCave() &&
			        ref == THEME_REF_PASSAGE_FLOOR &&
			        face == GLShape::TOP_SIDE ) {
				shapePal->getBMPData( path, floorData );
			}

			id = shapePal->findTextureByName( bmp );
			if ( !id.isSpecified() ) {
				id.load( path, false, false, true );
				GLclampf pri = 0.9f; id.glPrioritize( pri );
				loadedTextures[s] = id;
			}
		} else {
			id = loadedTextures[s];
		}
	}
	if ( outdoor ) {
		outdoorTextureGroup[ref][face] = id;
	} else {
		textureGroup[ref][face] = id;
	}
}

void WallTheme::unload() {
//  cerr << "*** Dumping theme: " << getName() << endl;
	/* just let them go
	for ( map<string, Texture>::iterator i = loadedTextures.begin(); i != loadedTextures.end(); ++i ) {
	 string s = i->first;
	 Texture id = i->second;

	 // don't delete system textures!
	 string bmp = s;
	 if ( bmp.find( ".", 0 ) == string::npos ) {
	  bmp += string( ".png" );
	 }
	 if ( !shapePal->findTextureByName( bmp ).isSpecified() ) {
	  //cerr << "Unloading texture: " << bmp << endl;
	  delete(id);
	 }
	}*/
	loadedTextures.clear();
}

Texture* WallTheme::getTextureGroup( string themeRefName ) {
	int ref = themeRefMap[ themeRefName ];
	return textureGroup[ ref ];
}

int WallTheme::getFaceCount( string themeRefName ) {
	int ref = themeRefMap[ themeRefName ];
	return faceCount[ ref ];
}

int WallTheme::getOutdoorFaceCount( string themeRefName ) {
	int ref = outdoorThemeRefMap[ themeRefName ];
	return outdoorFaceCount[ ref ];
}

void WallTheme::debug() {
	cerr << "THEME=" << getName() << endl;
	for ( int ref = 0; ref < THEME_REF_COUNT; ref++ ) {
		cerr << "\tref=" << themeRefName[ref] << endl;
		for ( int face = 0; face < faceCount[ref]; face++ ) {
			cerr << "\t\t" << textureNames[ref][face] << endl;
		}
	}
}

Shapes *Shapes::instance = NULL;

Shapes::Shapes( Session *session ) {
	texture_count = 0;
	textureGroupCount = 1;
	themeCount = allThemeCount = caveThemeCount = 0;
	currentTheme = NULL;
	this->session = session;
	if ( !instance ) instance = this;
	shapeCount = 0;
	shapes[0] = NULL;
	for (int i = 0; i < STENCIL_COUNT; ++i ) {
		stencilImage[ i ] = NULL;
	}
}

Shapes::~Shapes() {
	for (int i = 0; i < STENCIL_COUNT; ++i ) {
		delete stencilImage[ i ];
	}
	for ( int i = 0; i < shapeCount; i++ ) {
		delete shapes[i];
	}
}

Texture* Shapes::findOrMakeTextureGroup( char *s ) {

	if ( !strlen( s ) ) {
		return textureGroup[ 0 ];
	}

	char tmp[255];
	strcpy( tmp, s );

	Texture tg[3];

	int c = 0;
	char *token = strtok( tmp, "," );
	while ( token && c < 3 ) {
		tg[ c ] = findTextureByName( token, true );
		tg[ c ].goDiligent();
		++c;
		token = strtok( NULL, "," );
	}

	// try to find this texture group
	for ( int i = 0; i < textureGroupCount; i++ ) {
		if ( tg[ 0 ] == textureGroup[ i ][ 0 ] &&
		        tg[ 1 ] == textureGroup[ i ][ 1 ] &&
		        tg[ 2 ] == textureGroup[ i ][ 2 ] ) {
			//cerr << "*** Found existing texture group: " << i << endl;
			return textureGroup[ i ];
		}
	}

	// make a new one if not found
	textureGroup[ textureGroupCount ][ 0 ] = tg[ 0 ];
	textureGroup[ textureGroupCount ][ 1 ] = tg[ 1 ];
	textureGroup[ textureGroupCount ][ 2 ] = tg[ 2 ];
	//cerr << "*** Created new texture group: " << textureGroupCount << endl;
	textureGroupCount++;

	return textureGroup[ textureGroupCount - 1 ];
}

void Shapes::initialize() {
	// load textures
	ripple_texture.load( "/textures/ripple.png" );
	torchback.load( "/textures/torchback.png" );

	areaTex.load( "/textures/area.png", false, false );
	//areaTex = loadGLTextures("/area.bmp");

	// load as a grayscale (use gray value as alpha)
	selection.load( "/textures/sel.png", false, true);

	// default to textures
	strcpy( cursorDir, "/textures" );

	// resolve texture groups
	for ( int i = 0; i < textureGroupCount; i++ ) {
		for ( int c = 0; c < 3; c++ ) {
			textureGroup[i][c] = textures[0].texture;
		}
	}

	for ( int i = 0; i < static_cast<int>( shapeValueVector.size() ); i++ ) {
		shapes[i + 1] = NULL;
		ShapeValues *sv = shapeValueVector[i];
		// Resolve the texture group.
		// For theme-based shapes, leave texture NULL, they will be resolved later.
		if ( strlen( sv->theme ) ) {
			string name = sv->name;
			themeShapes.push_back( name );
			string s = sv->theme;
			themeShapeRef.push_back( s );
		}
	}

	// remember the number of shapes
	shapeCount = static_cast<int>( shapeValueVector.size() ) + 1;

	// add some special, "internal" shapes
	shapes[shapeCount] =
	  new GLTorch( textureGroup[ 0 ], findTextureByName( "flame.png", true ),
	               1, 1, 2,
	               "SPELL_FIREBALL",
	               0,
	               strtoul( "6070ffff", NULL, 16 ),
	               shapeCount,
	               torchback, Constants::SOUTH ); // Hack: use SOUTH for a spell
	shapes[shapeCount]->setSkipSide( false );
	shapes[shapeCount]->setStencil( false );
	shapes[shapeCount]->setLightBlocking( false );
	if ( !isHeadless() ) shapes[shapeCount]->initialize();
	string nameStr = shapes[shapeCount]->getName();
	shapeMap[nameStr] = shapes[shapeCount];
	shapeCount++;

	// maybe Shapes should contain the statics in GLCaveShape? 
	// add cave shapes (1 per dimension, flat and corner each)
	GLCaveShape::createShapes( textureGroup[0], shapeCount, this );
	for ( int i = 0; i < GLCaveShape::CAVE_INDEX_COUNT; i++ ) {
		shapes[shapeCount] = GLCaveShape::getShape( i );
		string nameStr = shapes[shapeCount]->getName();
		shapeMap[nameStr] = shapes[shapeCount];
		shapeCount++;
	}

	// load the lava stencils
	loadStencil( "/cave/stencil-side.bmp", STENCIL_SIDE );
	loadStencil( "/cave/stencil-u.bmp", STENCIL_U );
	loadStencil( "/cave/stencil-all.bmp", STENCIL_ALL );
	loadStencil( "/cave/stencil-turn.bmp", STENCIL_OUTSIDE_TURN );
	loadStencil( "/cave/stencil-sides.bmp", STENCIL_SIDES );

}

void Shapes::loadCursors() {
	for ( int i = 0; i < Constants::CURSOR_COUNT; i++ ) {
		string path = string( cursorDir ) + "/" + string( Constants::cursorTextureName[ i ] );
		//cursorTexture[i] = loadAlphaTexture( path, NULL, NULL, true );
		cursorTexture[i].load( path );
	}
}

void Shapes::loadDebugTheme() {
	loadTheme( "debug" );
}

void Shapes::loadRandomCaveTheme() {
	loadTheme( caveThemes[ Util::dice( caveThemeCount ) ] );
	GLCaveShape::initializeShapes( this );
}

void Shapes::loadCaveTheme( char *name ) {
	for ( int i = 0; i < caveThemeCount; i++ ) {
		if ( !strcmp( name, caveThemes[i]->getName() ) ) {
			loadTheme( caveThemes[i] );
			GLCaveShape::initializeShapes( this );
			return;
		}
	}
	cerr << "*** error: can't find cave theme: " << name << endl;
	loadRandomCaveTheme();
}

void Shapes::loadRandomTheme() {
	loadTheme( themes[ Util::dice( themeCount ) ] );
}

void Shapes::loadRandomOutdoorTheme() {
	loadTheme( outdoorThemes[ Util::dice( outdoorThemes.size() ) ] );
}

void Shapes::loadTheme( const char *themeName ) {
	//cerr << "*** Using theme: >" << themeName << "<" << endl;

	// find that theme!
	WallTheme *theme = NULL;
	for ( int i = 0; i < allThemeCount; i++ ) {
		//cerr << "\tconsidering >>>" << allThemes[i]->getName() << "<<" << endl;
		if ( !strcmp( allThemes[i]->getName(), themeName ) ) {
			theme = allThemes[i];
			break;
		}
	}
	assert( theme );

	loadTheme( theme );
}

void Shapes::loadTheme( WallTheme *theme ) {
	//cerr << "*** Loading theme: " << theme->getName() << " current=" << (!currentTheme ? "null" : currentTheme->getName()) << endl;
	if ( ALWAYS_RELOAD_THEME || currentTheme != theme ) {

		// unload the previous theme
		// FIXME: This could be optimized to not unload shared textures.
		if ( currentTheme ) currentTheme->unload();

		// load new theme
		currentTheme = ( WallTheme* )theme;
		currentTheme->load();

		if ( !currentTheme->isCave() ) {
			// apply theme to shapes
			//    cerr << "*** Applying theme to shapes: ***" << endl;
			GLShape::createDarkTexture( currentTheme );
			for ( int i = 0; i < static_cast<int>( themeShapes.size() ); i++ ) {
				string name = themeShapes[i];
				GLShape *shape = findShapeByName( name.c_str() );
				string ref = themeShapeRef[i];
				Texture* textureGroup = currentTheme->getTextureGroup( ref );
				//      cerr << "\tshape=" << shape->getName() << " ref=" << ref <<
				//        " tex=" << textureGroup[0] << "," << textureGroup[1] << "," << textureGroup[2] << endl;
				if ( !isHeadless() ) {
					//cerr << "\tshape=" << shape->getName() << " ref=" << ref << " count=" << currentTheme->getFaceCount( ref ) << endl;
					shape->setTexture( textureGroup );
					shape->setTextureCount( currentTheme->getFaceCount( ref ) );
				}
			}
			//    cerr << "**********************************" << endl;
		}
	}
}

char const* Shapes::getRandomDescription( int descriptionGroup ) {
	if ( descriptionGroup >= 0 && descriptionGroup < static_cast<int>( descriptions.size() ) ) {
		vector<string> *list = descriptions[descriptionGroup];
		int n = Util::dice( list->size() );
		return ( *list )[n].c_str();
	}
	return NULL;
}

// the next two methods are slow, only use during initialization
Texture const& Shapes::findTextureByName( const string& filename, bool loadIfMissing ) {
	for ( int i = 0; i < texture_count; i++ ) {
		if ( Util::StringCaseCompare( textures[i].filename, filename ) )
			return textures[i].texture;
	}
	if ( loadIfMissing ) {
		return loadSystemTexture( filename );
	}
	return Texture::none();
}

GLShape *Shapes::getShape( int index ) {
	if ( !shapes[ index ] ) {
		loadShape( shapeValueVector[ index - 1 ]->name );
	}
	return shapes[ index ];
}

GLShape *Shapes::findShapeByName( const char *name ) {
	if ( !name || !strlen( name ) ) return NULL;
	string s = name;
	if ( shapeMap.find( s ) == shapeMap.end() ) {
		loadShape( name );
		if ( shapeMap.find( s ) == shapeMap.end() ) {
			cerr << "&&& warning: could not find shape by name " << s << endl;
			return NULL;
		}
	}
	return shapeMap[s];
}

// defaults to SWORD for unknown shapes
int Shapes::findShapeIndexByName( const char *name ) {
	string s;
	if ( !name || !strlen( name ) ) s = "SWORD";
	else s = name;

	int index;
	ShapeValues *sv = getShapeValueByName( s.c_str(), &index );
	return( sv ? index + 1 : 0 );
}

void Shapes::getShapeDimensions( const char *name, int *w, int *d, int *h ) {
	int index;
	ShapeValues *sv = getShapeValueByName( name, &index );
	if ( sv ) {
		*w = sv->width;
		*d = sv->depth;
		*h = sv->height;
	} else {
		*w = *d = *h = 0;
	}
}

ShapeValues *Shapes::getShapeValueByName( const char *name, int *index ) {
	for ( int i = 0; i < static_cast<int>( shapeValueVector.size() ); i++ ) {
		ShapeValues *sv = shapeValueVector[ i ];
		if ( !strcmp( sv->name, name ) ) {
			*index = i;
			return sv;
		}
	}
	cerr << "&&& warning: could not find shape values by name " << name << endl;
	return NULL;
}

void Shapes::loadAllShapes() {
	for ( int i = 0; i < static_cast<int>( shapeValueVector.size() ); i++ ) {
		ShapeValues *sv = shapeValueVector[i];
		loadShape( sv->name );
	}
}

void Shapes::loadShape( const char *name ) {
	// already loaded?
	string s = name;
	if ( shapeMap.find( s ) != shapeMap.end() ) {
		return;
	}


	for ( int i = 0; i < static_cast<int>( shapeValueVector.size() ); i++ ) {
		//session->getGameAdapter()->setUpdate( _( "Creating Shapes" ), i, shapeValueVector.size() );
		ShapeValues *sv = shapeValueVector[i];

		// find the correct name
		if ( strcmp( sv->name, name ) ) {
			continue;
		}

		//cerr << "+++ Loading shape: " << name << endl;

		// Resolve the texture group.
		// For theme-based shapes, leave texture NULL, they will be resolved later.
		Texture* texture = textureGroup[ 0 ];
		if ( !strlen( sv->theme ) ) {
			texture = findOrMakeTextureGroup( sv->textures );
		}

		if ( sv->teleporter ) {
			shapes[( i + 1 )] =
			  new GLTeleporter( texture, findTextureByName( "flame.png", true ),
			                    sv->width, sv->depth, sv->height,
			                    sv->name,
			                    sv->descriptionIndex,
			                    sv->color,
			                    ( i + 1 ),
			                    sv->teleporter );
		} else if ( sv->m3ds_name.length() ) {
			shapes[( i + 1 )] =
			  new C3DSShape( sv->m3ds_name, sv->m3ds_scale, this,
			                 texture,
			                 sv->name,
			                 sv->descriptionIndex,
			                 sv->color,
			                 ( i + 1 ),
			                 sv->m3ds_x, sv->m3ds_y, sv->m3ds_z,
			                 sv->o3ds_x, sv->o3ds_y, sv->o3ds_z,
			                 sv->xrot3d, sv->yrot3d, sv->zrot3d,
			                 sv->lighting, sv->base_w, sv->base_h );
			if ( sv->usesAlpha ) {
				( ( C3DSShape* )shapes[( i + 1 )] )->setHasAlphaValues( true );
			}
		} else if ( sv->torch > -1 ) {
			if ( sv->torch == 5 ) {
				shapes[( i + 1 )] =
				  new GLTorch( texture, findTextureByName( "flame.png", true ),
				               //textures[9].id,
				               sv->width, sv->depth, sv->height,
				               sv->name,
				               sv->descriptionIndex,
				               sv->color,
				               ( i + 1 ) );
			} else {
				shapes[( i + 1 )] =
				  new GLTorch( texture, findTextureByName( "flame.png", true ),
				               //textures[9].id,
				               sv->width, sv->depth, sv->height,
				               sv->name,
				               sv->descriptionIndex,
				               sv->color,
				               ( i + 1 ),
				               torchback, sv->torch );
			}
		} else if ( strlen( sv->refs ) ) {
			// recursive call:
			GLShape *refShape = findShapeByName( sv->refs );
			if ( !refShape ) {
				cerr << "*** Error: can't find referenced shape: " << sv->refs << " for shape " << sv->name << endl;
			}
			shapes[( i + 1 )] =
			  new VirtualShape( sv->name,
			                    sv->width, sv->height, sv->depth,
			                    sv->o3ds_x, sv->o3ds_y, sv->o3ds_z,
			                    sv->draws,
			                    refShape,
			                    i + 1 );
		} else {
			shapes[( i + 1 )] =
			  new GLShape( texture,
			               sv->width, sv->depth, sv->height,
			               sv->name,
			               sv->descriptionIndex,
			               sv->color,
			               ( i + 1 ) );
		}
		shapes[( i + 1 )]->setSkipSide( sv->skipSide );
		shapes[( i + 1 )]->setStencil( sv->stencil == 1 );
		shapes[( i + 1 )]->setLightBlocking( sv->blocksLight == 1 );
		shapes[( i + 1 )]->setIconRotation( sv->xrot, sv->yrot, sv->zrot );
		shapes[( i + 1 )]->setIgnoreHeightMap( sv->ignoreHeightMap );
		shapes[( i + 1 )]->setNoFloor( sv->noFloor );
		shapes[( i + 1 )]->setNotFlatten( sv->notFlatten );

		if ( sv->wallShape ) shapes[( i + 1 )]->setIsWallShape( true );

		// Call this when all other intializations are done.
		if ( !( strlen( sv->theme ) || isHeadless() ) ) {
			shapes[( i + 1 )]->initialize();
		}

		// set the effect
		shapes[ ( i + 1 ) ]->setEffectType( sv->effectType,
		                                    sv->effectWidth, sv->effectDepth, sv->effectHeight,
		                                    sv->effectX, sv->effectY, sv->effectZ );

		shapes[ ( i + 1 ) ]->setInteractive( sv->interactive );

		shapes[ ( i + 1 ) ]->setOutdoorWeight( sv->outdoorsWeight );
		shapes[ ( i + 1 ) ]->setOutdoorShadow( sv->outdoorShadow );
		shapes[ ( i + 1 ) ]->setWind( sv->wind );

		shapes[ ( i + 1 ) ]->setOccurs( &( sv->occurs ) );
		shapes[ ( i + 1 ) ]->setIconRotation( sv->iconRotX, sv->iconRotY, sv->iconRotZ );
		shapes[ ( i + 1 ) ]->setIcon( sv->icon, sv->iconWidth, sv->iconHeight );
		shapes[ ( i + 1 ) ]->setAmbientName( sv->ambient );
		shapes[ ( i + 1 ) ]->setRoof( sv->roof );

		string s = sv->name;
		shapeMap[s] = shapes[( i + 1 )];

		// init its virtual shapes
		session->getSquirrel()->callMapMethod( "addVirtualShapes", shapes[( i + 1 )]->getName() );

		break;
	}
}

bool inline isPowerOfTwo( const GLuint n ) {
	return ( n != 0 && ( ( n & ( n - 1 ) ) == 0 ) );
}

// FIXME: this should be similar to loadTexture but adding alpha
// screws up the stencils in caves. No time to debug now.
GLuint Shapes::getBMPData( const string& filename, TextureData& data, int *imgwidth, int *imgheight ) {
	string fn = rootDir + filename;

  cerr << "loading lava data: " << fn << endl;

	// Load the texture
	//SDL_Surface* textureImage = SDL_LoadBMP( fn.c_str() );
	SDL_Surface* textureImage = IMG_Load( fn.c_str() );

	// If Bitmap's Not Found Quit
	if ( textureImage == NULL ) {
		cerr << "Unable to load " << fn << endl;
		return 0;
	}

	// Check The Bitmap For Errors

	/*  if( textureImage->w != textureImage->h &&
	    ( !isPowerOfTwo( textureImage->w ) ||
	     !isPowerOfTwo( textureImage->h ) ) ) {
	   fprintf(stderr, "*** Possible error: Width or Heigth not a power of 2: name=%s pitch=%d width=%d height=%d\n",
	       fn.c_str(), (textureImage->pitch/3), textureImage->w, textureImage->h);
	  }*/

	int width = textureImage->w;
	int height = textureImage->h;

	Constants::checkTexture( "Shapes::loadGLTextures", width, height );

// cerr << "\tdim=" << textureImage->w << "," << textureImage->h << endl;

	if ( imgwidth ) *imgwidth = width;
	if ( imgheight ) *imgheight = height;

	unsigned char* pixels = ( unsigned char * )( textureImage->pixels ); // the pixel data

	if ( data.empty() ) {
		data.resize( 3 * width * height );
	}

	// the following lines extract R,G and B values from any bitmap
	int count = 0;
	int c = 0;
	for ( int i = 0; i < width * height; ++i ) {
		if ( i > 0 && i % width == 0 ) {
			c += (  textureImage->pitch - ( width * textureImage->format->BytesPerPixel ) );
		}
		data[ count++ ] = pixels[ c++ ]; // red
		data[ count++ ] = pixels[ c++ ]; // green
		data[ count++ ] = pixels[ c++ ]; // blue
	}

	// Free up texture
	SDL_FreeSurface( textureImage );

	return 1;
}

/// Returns a tile of an SDL surface as a texture.

/// This function allows you to define the size of a tile and which tile you want. It
/// is then created as a texture from the appropriate part of the SDL surface.
/* unused:
GLuint Shapes::createTileTexture( SDL_Surface **surface, int tileX, int tileY, int tileWidth, int tileHeight ) {
 if ( isHeadless() ) return 0;

 // The raw data of the source image.
 unsigned char * data = ( unsigned char * ) ( ( *surface ) -> pixels );
 // The destination image (a single tile)
 std::vector<GLubyte> image( tileWidth * tileHeight * 4 );

 int count = 0;
 // where the tile starts in a line
 int offs = tileX * tileWidth * 4;
 // where the tile ends in a line
 int rest = ( tileX + 1 ) * tileWidth * 4;
 // Current position in the source data
 int c = offs + ( tileY * tileHeight * ( *surface )->pitch );
 // the following lines extract R,G and B values from any bitmap

 for ( int i = 0; i < tileWidth * tileHeight; ++i ) {

  if ( i > 0 && i % tileWidth == 0 ) {
   // skip the rest of the line
   c += ( ( *surface )->pitch - rest );
   // skip the offset (go to where the tile starts)
   c += offs;
  }

  for ( int p = 0; p < 4; p++ ) {
   image[count++] = data[c++];
  }

 }

 int bpp = session->getPreferences()->getBpp();

 lastTextureWidth = tileWidth;
 lastTextureHeight = tileHeight;
 lastTextureAlpha = true;

 GLuint texture[1];

 // Create The Texture
 glGenTextures( 1, &texture[0] );

 // Typical Texture Generation Using Data From The Bitmap
 glBindTexture( GL_TEXTURE_2D, texture[0] );

 glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
 glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );

 gluBuild2DMipmaps( GL_TEXTURE_2D, ( bpp > 16 ? GL_RGBA : GL_RGBA4 ), tileWidth, tileHeight, GL_RGBA, GL_UNSIGNED_BYTE, &image[0] );

 return texture[0];
}*/

void Shapes::swap( unsigned char & a, unsigned char & b ) {
	unsigned char temp;

	temp = a;
	a    = b;
	b    = temp;

	return;
}

/// Grand unified generic texture loader.

/// Creates an OpenGL texture from a file and tries to set up everything
/// correctly according to the properties of the source image.
/// Returns an OpenGL texture name.

/* unused:
GLuint Shapes::loadTexture( const string& filename, bool absolutePath, bool isSprite, bool anisotropy ) {
 string fn = ( absolutePath ? filename : rootDir + filename );
 GLuint destFormat;
 GLuint srcFormat;
 GLuint minFilter;
 int bpp = session->getPreferences()->getBpp();

 lastTextureWidth = lastTextureHeight = 0;
 lastTextureAlpha = false;

 SDL_Surface* surface = IMG_Load( fn.c_str() );
 if ( surface == NULL ) {
  cerr << "*** Error loading image (" << fn << "): " << IMG_GetError() << endl;
  return NULL;
 }

 Constants::checkTexture( "Shapes::loadTexture", surface->w, surface->h );

 GLuint texture;
 glPixelStorei( GL_UNPACK_ALIGNMENT, 4 );
 glGenTextures( 1, &texture );
 glBindTexture( GL_TEXTURE_2D, texture );

 SDL_PixelFormat *format = surface->format;

 lastTextureWidth = surface->w; lastTextureHeight = surface->h;
 lastTextureAlpha = format->Amask != 0;

 if ( format->Amask ) {
  srcFormat = GL_RGBA;
  destFormat = ( bpp > 16 ? GL_RGBA : GL_RGBA4 );
  minFilter = ( isSprite ? GL_LINEAR : GL_LINEAR_MIPMAP_LINEAR );
 } else {
  srcFormat = GL_RGB;
  destFormat = ( bpp > 16 ? GL_RGB : GL_RGB5 );
  minFilter = GL_LINEAR_MIPMAP_NEAREST;
 }

 glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, minFilter );
 glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );

 // Enable anisotropic filtering if requested, mipmapping is enabled
 // and the hardware supports it.
 if ( anisotropy && !format->Amask && strstr( ( char* )glGetString( GL_EXTENSIONS ),
                                              "GL_EXT_texture_filter_anisotropic" ) && session->getPreferences()->getAnisoFilter() ) {
  float maxAnisotropy;
  glGetFloatv( GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT, &maxAnisotropy );
  glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, maxAnisotropy );
 } else {
  glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, 1.0f );
 }

//  glTexImage2D( GL_TEXTURE_2D, 0, destFormat, surface->w, surface->h, 0, srcFormat, GL_UNSIGNED_BYTE, surface->pixels );

 gluBuild2DMipmaps( GL_TEXTURE_2D, destFormat, surface->w, surface->h, srcFormat, GL_UNSIGNED_BYTE, surface->pixels );

 SDL_FreeSurface( surface );
 return texture;
}
*/

void Shapes::loadStencil( const string& filename, int index ) {
	if ( isHeadless() ) return;

	string fn = rootDir + filename;
//  fprintf(stderr, "setupAlphaBlendedBMP, rootDir=%s\n", rootDir);
	//stencil[ index ] = IMG_Load( fn.c_str() );
	stencil[ index ] = SDL_LoadBMP( fn.c_str() );

	if ( stencil[ index ] == NULL ) {
		stencilImage[ index ] = NULL;
		return;
	}

	// Rearrange the pixelData
	int width  = stencil[ index ]->w;
	int height = stencil[ index ]->h;

	unsigned char * data = ( unsigned char * )( stencil[ index ]->pixels );     // the pixel data

	GLubyte* p = new GLubyte[ width * height ];
	int count = 0;
	int c = 0;
	unsigned char r, g, b;
	// the following lines extract R,G and B values from any bitmap
	for ( int i = 0; i < width * height; ++i ) {
		if ( i > 0 && i % width == 0 ) {
			c += (  stencil[ index ]->pitch - ( width * stencil[ index ]->format->BytesPerPixel ) );
		}
		b = data[c++]; 	 
		g = data[c++];
		r = data[c++];
//		g = data[c++];
//		b = data[c++];

		p[count++] = ( !( r || g || b ) ? 0 :
		               ( r == 0xff && g == 0xff && b == 0xff ? 2 : 1 ) );
	}

	stencilImage[ index ] = p;
}

/// Loads an image containing tiles from disk into a SDL surface.

void Shapes::loadTiles( const string& filename, SDL_Surface **surface ) {
	if ( isHeadless() ) return;

	string fn = rootDir + filename;
	if ( !( ( *surface ) = IMG_Load( fn.c_str() ) ) ) {
		cerr << "*** Error loading tiles (" << fn << "): " << IMG_GetError() << endl;
	}
}

// places to look for system textures (must end in "")
char *textureDirs[] = { "/textures/", "/cave/default/", "/objects/houses/", "" };

Texture const& Shapes::loadSystemTexture( const string& line ) {
	if ( isHeadless() ) return Texture::none();

	if ( line == "none.png" ) {
		return Texture::none();
	}

	if ( texture_count >= MAX_SYSTEM_TEXTURE_COUNT ) {
		cerr << "Error: *** no more room for system textures!. Not loading: " << line << endl;
		return Texture::none();
	}

	Texture const& existing = findTextureByName( line );
	if ( existing.isSpecified() ) {
		return existing;
	}

	textures[texture_count].filename = line;
	int dirCount = 0;
	while ( strlen( textureDirs[dirCount] ) ) {
		string path = textureDirs[dirCount] + textures[texture_count].filename;
		// file exists?
		string fn( rootDir + path );
		FILE *fp = fopen( fn.c_str(), "rb" );
		if ( fp ) {
			fclose( fp );
			// FIXME: Anisotropic filtering for system textures freezes X for some reason.
			// id = loadTexture( path, false, false, true );
			textures[ texture_count ].texture.load( path, false, false );
		}
		dirCount++;
	}

	if ( !textures[ texture_count ].texture.isSpecified() ) {
		cerr << "*** Error: Unable to find texture: " << line << endl;
		return Texture::none();
	}

	texture_count++;
	return textures[ texture_count-1 ].texture;
}

Texture const& Shapes::getCursorTexture( int cursorMode ) {
	return cursorTexture[ cursorMode ];
}

Texture GroundTexture::getRandomTexture() {
	int n = (int)( Util::mt_rand() * textures.size() );
	map<string, Texture>::iterator iter = textures.begin();
	// can I just do i += n ?
	for( int i = 0; i < n; i++ ) iter++;
	string filename = iter->first;
	Texture tex = iter->second;
	
	if( !tex.isSpecified() ) {
		tex.load( filename );
	}
	return tex;
}

