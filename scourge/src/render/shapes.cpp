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
#include "virtualshape.h"
#include "../session.h"
#include <fstream>
#include <iostream>

using namespace std;

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
	"snow_big", "lakebed"
};

WallTheme::WallTheme( char const* name, Shapes *shapePal ) 
{
  strcpy( this->name, name );
  this->shapePal = shapePal;
	this->hasOutdoor = false;
  for(int i = 0; i < THEME_REF_COUNT; i++) {
    themeRefMap[ themeRefName[i] ] = i;
		faceCount[ i ] = 0;
	}
	for( int i = 0; i < OUTDOOR_THEME_REF_COUNT; i++ ) {
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
  for(int ref = 0; ref < THEME_REF_COUNT; ref++) {
  	if( faceCount[ ref ] == 0 ) {
			cerr << "No textures defined for theme. Ref=" << themeRefName[ ref ] << endl;
			continue;
		}
    for(int face = 0; face < faceCount[ ref ]; face++) {
      loadTextureGroup( ref, face, textures[ ref ][ face ] );
    }
  }
	if( getHasOutdoor() ) {
		for(int ref = 0; ref < OUTDOOR_THEME_REF_COUNT; ref++) {
			if( outdoorFaceCount[ ref ] == 0 ) {
				cerr << "No textures defined for outdoor theme. Ref=" << outdoorThemeRefName[ ref ] << endl;
				continue;
			}
			for(int face = 0; face < outdoorFaceCount[ ref ]; face++) {
				loadTextureGroup( ref, face, outdoorTextures[ ref ][ face ], true );
			}
			// create edge textures
			createOutdoorEdgeTexture( OUTDOOR_THEME_REF_GRASS_CORNER );
			createOutdoorEdgeTexture( OUTDOOR_THEME_REF_GRASS_EDGE );
			createOutdoorEdgeTexture( OUTDOOR_THEME_REF_GRASS_NARROW );
			createOutdoorEdgeTexture( OUTDOOR_THEME_REF_GRASS_TIP );
		}
	}
}

// Overlay the current 'grass' texture on the alpha-blended edge texture to create a theme-specific blend.
void WallTheme::createOutdoorEdgeTexture( int ref ) {
	GLuint tex = shapePal->createAlphaTexture( outdoorTextureGroup[ ref ][ 0 ],
	                                           outdoorTextureGroup[ OUTDOOR_THEME_REF_GRASS ][ 0 ] );  
  glDeleteTextures( 1, outdoorTextureGroup[ ref ] );
  outdoorTextureGroup[ ref ][ 0 ] = tex;
	
}

void WallTheme::loadTextureGroup( int ref, int face, char *texture, bool outdoor ) {
	//cerr << "Loading theme texture. Theme: " << getName() << " ref=" << ( outdoor ? outdoorThemeRefName[ ref ] : themeRefName[ ref ] ) << " face=" << face << " texture=" << texture << endl;
  string path;  
	GLuint id = 0;
  if ( texture && strcmp( texture, "null" ) ) {
    string s = texture;
    if ( loadedTextures.find(s) == loadedTextures.end() ) {

      // see if it's a system texture (no need to double load it)
			string bmp = texture;
			if( bmp.find( ".", 0 ) == string::npos ) {
				bmp += string(".bmp");
			}
      path = "/" + bmp;

      // keep lava texture data
      if( isCave() && 
          ref == THEME_REF_PASSAGE_FLOOR && 
          face == GLShape::FRONT_SIDE ) {
        shapePal->getBMPData( path, lavaData );
      }
      if( isCave() && 
          ref == THEME_REF_PASSAGE_FLOOR && 
          face == GLShape::TOP_SIDE ) {
        shapePal->getBMPData( path, floorData );
      }

      id = shapePal->findTextureByName( bmp );
      if ( id == 0 ) {
				// two ways of loading for backward compat.
				if( path.find( ".bmp", 0 ) == path.length() - 4 ) {
					//cerr << "\tLoading BMP" << endl;
					id = shapePal->loadGLTextures(path);
				} else {
					//cerr << "\tLoading non-BMP" << endl;
					id = shapePal->loadAlphaTexture( path );
				}
				
				if( id == 0 ) {
			  	cerr << "Error: unable to load theme texture for: ref=" << 
			  	( outdoor ? outdoorThemeRefName[ ref ] : themeRefName[ ref ] ) << 
			  	" path=" << path << endl;
			  }				
				
        loadedTextures[s] = id;
      }
    } else {
      id = loadedTextures[s];
    }
	}  
	if( outdoor ) {
		outdoorTextureGroup[ref][face] = id;  
	} else {
		textureGroup[ref][face] = id;  		
	}
}

void WallTheme::unload() {
//  cerr << "*** Dumping theme: " << getName() << endl;
  for (map<string,GLuint>::iterator i=loadedTextures.begin(); i!=loadedTextures.end(); ++i) {
    string s = i->first;
    GLuint id = i->second;

    // don't delete system textures!
		string bmp = s;
		if( bmp.find( ".", 0 ) == string::npos ) {
			bmp += string(".bmp");
		}
    id = shapePal->findTextureByName( bmp );
    if ( id == 0 ) {
			//cerr << "Unloading texture: " << bmp << endl;
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

int WallTheme::getOutdoorFaceCount( string themeRefName ) {
  int ref = outdoorThemeRefMap[ themeRefName ];
  return outdoorFaceCount[ ref ];
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

Shapes *Shapes::instance = NULL;

Shapes::Shapes( Session *session ){
  texture_count = 0;
  textureGroupCount = 1;
	textureGroup[0][0] = textureGroup[0][1] = textureGroup[0][2] = 0;
  themeCount = allThemeCount = caveThemeCount = 0;
  currentTheme = NULL;
  this->session = session;
	if( !instance ) instance = this;
}

GLuint *Shapes::findOrMakeTextureGroup( char *s ) {
	char tmp[255];
	strcpy( tmp, s );

	GLuint tg[3];

	int c = 0;
	char *token = strtok( tmp, "," );
	while( token && c < 3 ) {
		tg[ c++ ] = findTextureByName( token, true );
		token = strtok( NULL, "," );
	}

	// try to find this texture group
	for( int i = 0; i < textureGroupCount; i++ ) {
		if( tg[ 0 ] == textureGroup[ i ][ 0 ] &&
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
  ripple_texture = loadGLTextures("/textures/ripple.bmp");
  torchback = loadGLTextures("/textures/torchback.bmp");
  
  SDL_Surface *area;
  GLubyte *areaImage = NULL;
	string areabmp("/textures/area.bmp");
  setupAlphaBlendedBMP(areabmp, area, areaImage);
  areaTex = loadGLTextureBGRA(area, areaImage, GL_LINEAR);
  //areaTex = loadGLTextures("/area.bmp");

	// load as a grayscale (use gray value as alpha)
	string sel("/textures/sel.bmp");
	selection = Shapes::loadTextureWithAlpha( sel, 0, 0, 0, false, false, true );

	// default to textures
	strcpy( cursorDir, "/textures" );

	// resolve texture groups
	for(int i = 0; i < textureGroupCount; i++) {
		for(int c = 0; c < 3; c++) {
			textureGroup[i][c] = textures[textureGroup[i][c]].id;
		}
	}

	for(int i = 0; i < static_cast<int>(shapeValueVector.size()); i++) {
		shapes[i + 1] = NULL;
		ShapeValues *sv = shapeValueVector[i];
		// Resolve the texture group.
		// For theme-based shapes, leave texture NULL, they will be resolved later.
		if( strlen( sv->theme ) ) {
			string name = sv->name;
      themeShapes.push_back( name );
			string s = sv->theme;
      themeShapeRef.push_back( s );
		}
	}

  // remember the number of shapes
  shapeCount = static_cast<int>(shapeValueVector.size()) + 1;

  // add some special, "internal" shapes
	shapes[shapeCount] =
		new GLTorch(textureGroup[ 0 ], findTextureByName( "flame.bmp", true ),
								1, 1, 2,
								strdup("SPELL_FIREBALL"),
								0,
								strtoul("6070ffff", NULL, 16),
								shapeCount, 
								torchback, Constants::SOUTH); // Hack: use SOUTH for a spell
  shapes[shapeCount]->setSkipSide(false);
  shapes[shapeCount]->setStencil(false);
  shapes[shapeCount]->setLightBlocking(false);  
  if( !isHeadless() ) shapes[shapeCount]->initialize();
  string nameStr = shapes[shapeCount]->getName();
  shapeMap[nameStr] = shapes[shapeCount];
  shapeCount++;

  // add cave shapes (1 per dimension, flat and corner each)
  GLCaveShape::createShapes( textureGroup[0], shapeCount, this );  
  for( int i = 0; i < GLCaveShape::CAVE_INDEX_COUNT; i++ ) {
    shapes[shapeCount] = GLCaveShape::getShape(i);
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
  for( int i = 0; i < Constants::CURSOR_COUNT; i++ ) {
	string path = string(cursorDir) + "/" + string(Constants::cursorTextureName[ i ]);
	cursorTexture[i] = loadAlphaTexture( path );
  }
}

Shapes::~Shapes(){
}

void Shapes::loadDebugTheme() {
	loadTheme( "debug" );
}

void Shapes::loadRandomCaveTheme() {
  loadTheme( caveThemes[ Util::dice( caveThemeCount ) ] );
  GLCaveShape::initializeShapes( this );
}

void Shapes::loadCaveTheme( char *name ) {
	for( int i = 0; i < caveThemeCount; i++ ) {
		if( !strcmp( name, caveThemes[i]->getName() ) ) {
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

void Shapes::loadTheme(const char *themeName) {
  //cerr << "*** Using theme: >" << themeName << "<" << endl;

  // find that theme!
  WallTheme *theme = NULL;
  for(int i = 0; i < allThemeCount; i++) {
		//cerr << "\tconsidering >>>" << allThemes[i]->getName() << "<<" << endl;
    if( !strcmp( allThemes[i]->getName(), themeName ) ) {
      theme = allThemes[i];
      break;
    }
  }
	assert( theme );

  loadTheme( theme );
}

void Shapes::loadTheme( WallTheme *theme ) {
  //cerr << "*** Loading theme: " << theme->getName() << " current=" << (!currentTheme ? "null" : currentTheme->getName()) << endl;
  if( ALWAYS_RELOAD_THEME || currentTheme != theme) {

    // unload the previous theme
    // FIXME: This could be optimized to not unload shared textures.
    if( currentTheme ) currentTheme->unload();
    
    // load new theme
    currentTheme = (WallTheme*)theme;
    currentTheme->load();

    if( !currentTheme->isCave() ) {
      // apply theme to shapes
      //    cerr << "*** Applying theme to shapes: ***" << endl;
      GLShape::createDarkTexture( currentTheme );
      for(int i = 0; i < static_cast<int>(themeShapes.size()); i++) {
				string name = themeShapes[i];
        GLShape *shape = findShapeByName( name.c_str() );
        string ref = themeShapeRef[i];
        GLuint *textureGroup = currentTheme->getTextureGroup( ref );
        //      cerr << "\tshape=" << shape->getName() << " ref=" << ref << 
        //        " tex=" << textureGroup[0] << "," << textureGroup[1] << "," << textureGroup[2] << endl;  
        if( !isHeadless() ) {
        	//cerr << "\tshape=" << shape->getName() << " ref=" << ref << " count=" << currentTheme->getFaceCount( ref ) << endl; 
					shape->setTexture( textureGroup );
					shape->setTextureCount( currentTheme->getFaceCount( ref ) );
				}				
      }  
      //    cerr << "**********************************" << endl;
    }
  }
}

char const* Shapes::getRandomDescription(int descriptionGroup) {
  if(descriptionGroup >= 0 && descriptionGroup < static_cast<int>(descriptions.size())) {
    vector<string> *list = descriptions[descriptionGroup];
    int n = Util::dice( list->size() ); 
    return (*list)[n].c_str();
  }
  return NULL;
}

// the next two methods are slow, only use during initialization
GLuint Shapes::findTextureByName( const string& filename, bool loadIfMissing ) {
	for(int i = 0; i < texture_count; i++) {
		if(Util::StringCaseCompare(textures[i].filename, filename))
			return textures[i].id;
	}
	if( loadIfMissing ) {
		return loadSystemTexture( filename );
	}
  return 0;
}

GLShape *Shapes::getShape( int index ) { 
	if( !shapes[ index ] ) {
		loadShape( shapeValueVector[ index - 1 ]->name );
	}
	return shapes[ index ]; 
}

GLShape *Shapes::findShapeByName(const char *name) {
  if(!name || !strlen(name)) return NULL;
  string s = name;
  if( shapeMap.find( s ) == shapeMap.end() ) {
		loadShape( name );
		if( shapeMap.find( s ) == shapeMap.end() ) {
			cerr << "&&& warning: could not find shape by name " << s << endl;
			return NULL;
		}
  }
	return shapeMap[s];
}

// defaults to SWORD for unknown shapes
int Shapes::findShapeIndexByName( const char *name ) {
  string s;
  if(!name || !strlen(name)) s = "SWORD";
  else s = name;

	int index;
	ShapeValues *sv = getShapeValueByName( s.c_str(), &index );
	return( sv ? index + 1 : 0 );
}

void Shapes::getShapeDimensions( const char *name, int *w, int *d, int *h ) {
	int index;
	ShapeValues *sv = getShapeValueByName( name, &index );
	if( sv ) {
		*w = sv->width;
		*d = sv->depth;
		*h = sv->height;
	} else {
		*w = *d = *h = 0;
	}
}

ShapeValues *Shapes::getShapeValueByName( const char *name, int *index ) {
	for( int i = 0; i < static_cast<int>(shapeValueVector.size()); i++ ) {
		ShapeValues *sv = shapeValueVector[ i ];
		if( !strcmp( sv->name, name ) ) {
			*index = i;
			return sv;
		}
	}
	cerr << "&&& warning: could not find shape values by name " << name << endl;
	return NULL;
}

void Shapes::loadAllShapes() {
	for(int i = 0; i < static_cast<int>(shapeValueVector.size()); i++) {
		ShapeValues *sv = shapeValueVector[i];
		loadShape( sv->name );
	}
}

void Shapes::loadShape( const char *name ) {
	// already loaded?
	string s = name;
	if( shapeMap.find( s ) != shapeMap.end() ) {
		return;
	}


	for(int i = 0; i < static_cast<int>(shapeValueVector.size()); i++) {
		//session->getGameAdapter()->setUpdate( _( "Creating Shapes" ), i, shapeValueVector.size() );
		ShapeValues *sv = shapeValueVector[i];

		// find the correct name
		if( strcmp( sv->name, name ) ) {
			continue;
		}

		//cerr << "+++ Loading shape: " << name << endl;

		// Resolve the texture group.
		// For theme-based shapes, leave texture NULL, they will be resolved later.
		GLuint *texture = textureGroup[ 0 ];
		if( !strlen( sv->theme ) ) {
			texture = findOrMakeTextureGroup( sv->textures );
		}

    if(sv->teleporter) {
      shapes[(i + 1)] =
				new GLTeleporter( texture, findTextureByName( "flame.bmp", true ),
													sv->width, sv->depth, sv->height,
													strdup(sv->name), 
													sv->descriptionIndex,
													sv->color,
													(i + 1),
													sv->teleporter );
    } else if(sv->m3ds_name.length()) {
      shapes[(i + 1)] =
      new C3DSShape(sv->m3ds_name, sv->m3ds_scale, this,
                    texture,                     
                    strdup(sv->name), 
                    sv->descriptionIndex,
                    sv->color,
                    (i + 1),
                    sv->m3ds_x, sv->m3ds_y, sv->m3ds_z,
                    sv->o3ds_x, sv->o3ds_y, sv->o3ds_z,
										sv->xrot3d, sv->yrot3d, sv->zrot3d,
										sv->lighting, sv->base_w, sv->base_h );
    } else if(sv->torch > -1) {
      if(sv->torch == 5) {
        shapes[(i + 1)] =
        new GLTorch(texture, findTextureByName( "flame.bmp", true ),
										//textures[9].id,
                    sv->width, sv->depth, sv->height,
                    strdup(sv->name),
                    sv->descriptionIndex,
                    sv->color,
                    (i + 1));
      } else {
        shapes[(i + 1)] =
        new GLTorch(texture, findTextureByName( "flame.bmp", true ),
										//textures[9].id,
                    sv->width, sv->depth, sv->height,
                    strdup(sv->name),
                    sv->descriptionIndex,
                    sv->color,
                    (i + 1), 
                    torchback, sv->torch);
      }
    } else if( strlen( sv->refs ) ) {
    	// recursive call:
    	GLShape *refShape = findShapeByName( sv->refs );
    	if( !refShape ) {
    		cerr << "*** Error: can't find referenced shape: " << sv->refs << " for shape " << sv->name << endl;
    	}
    	shapes[(i + 1)] = 
    		new VirtualShape( strdup( sv->name ),
    		                  sv->width, sv->height, sv->depth,
    		                  sv->o3ds_x, sv->o3ds_y, sv->o3ds_z,
    		                  sv->draws, 
    		                  refShape,
    		                  i + 1 );
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

		if( sv->wallShape ) shapes[(i + 1)]->setIsWallShape( true );

    // Call this when all other intializations are done.
    if( !( strlen( sv->theme ) || isHeadless() ) ) {
      shapes[(i + 1)]->initialize();
    }

    // set the effect
    shapes[ ( i + 1 ) ]->setEffectType( sv->effectType, 
                                        sv->effectWidth, sv->effectDepth, sv->effectHeight, 
                                        sv->effectX, sv->effectY, sv->effectZ );

    shapes[ ( i + 1 ) ]->setInteractive( sv->interactive );

		shapes[ ( i + 1 ) ]->setOutdoorWeight( sv->outdoorsWeight );
		shapes[ ( i + 1 ) ]->setOutdoorShadow( sv->outdoorShadow );
		shapes[ ( i + 1 ) ]->setWind( sv->wind );

		shapes[ ( i + 1 ) ]->setOccurs( &(sv->occurs) );
		shapes[ ( i + 1 ) ]->setIconRotation( sv->iconRotX, sv->iconRotY, sv->iconRotZ );
		shapes[ ( i + 1 ) ]->setIcon( sv->icon, sv->iconWidth, sv->iconHeight );
		shapes[ ( i + 1 ) ]->setAmbientName( sv->ambient );
		shapes[ ( i + 1 ) ]->setRoof( sv->roof );

    string s = sv->name;
    shapeMap[s] = shapes[(i + 1)];
		break;
  }
}

bool inline isPowerOfTwo( const GLuint n ) {
	return (n != 0 && ((n & (n - 1)) == 0));
}

// FIXME: this should be similar to loadTextureWithAlpha but adding alpha
// screws up the stencils in caves. No time to debug now.
GLuint Shapes::getBMPData( const string& filename, TextureData& data, int *imgwidth, int *imgheight ) {
	string fn = rootDir + filename;

//  cerr << "loading lava data: " << fn << endl;

	// Load the texture
	SDL_Surface* textureImage = SDL_LoadBMP( fn.c_str() );

	// If Bitmap's Not Found Quit
	if( textureImage == NULL ) {
		cerr << "Unable to load " << fn << endl;
		return 0;
	}

	// Check The Bitmap For Errors

/*		if( textureImage->w != textureImage->h && 
				( !isPowerOfTwo( textureImage->w ) ||
					!isPowerOfTwo( textureImage->h ) ) ) {
			fprintf(stderr, "*** Possible error: Width or Heigth not a power of 2: name=%s pitch=%d width=%d height=%d\n", 
							fn.c_str(), (textureImage->pitch/3), textureImage->w, textureImage->h);
		}*/

	int width = textureImage->w;
	int height = textureImage->h;

	Constants::checkTexture( "Shapes::loadGLTextures", width, height );

//	cerr << "\tdim=" << textureImage->w << "," << textureImage->h << endl;

	if( imgwidth ) *imgwidth = width;
	if( imgheight )	*imgheight = height;

	unsigned char* pixels = (unsigned char *)(textureImage->pixels);				 // the pixel data

	if ( data.empty() ) {
		data.resize( 3 * width * height );
	}

	// the following lines extract R,G and B values from any bitmap
	int count = 0;
	int c = 0;
	for( int i = 0; i < width * height; ++i ) {
		if( i > 0 && i % width == 0 ) {
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


/* function to load in bitmap as a GL texture */
GLuint Shapes::loadGLTextures(const string& filename) {

	if( isHeadless() ) 
		return 0;

  string fn = rootDir + filename;

  GLuint texture[1];

  /* Create storage space for the texture */
  SDL_Surface *TextureImage[1];

  /* Load The Bitmap, Check For Errors, If Bitmap's Not Found Quit */
  if( ( TextureImage[0] = SDL_LoadBMP( fn.c_str() ) ) ) {

/*    if( TextureImage[0]->w != TextureImage[0]->h && 
        ( !isPowerOfTwo( TextureImage[0]->w ) ||
          !isPowerOfTwo( TextureImage[0]->h ) ) ) {
      fprintf(stderr, "*** Possible error: Width or Heigth not a power of 2: name=%s pitch=%d width=%d height=%d\n", 
              fn.c_str(), (TextureImage[0]->pitch/3), TextureImage[0]->w, TextureImage[0]->h);
    }*/

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
  if( isHeadless() ) return 0;
  return loadGLTextureBGRA( surface->w, surface->h, image, glscale );
}

GLuint Shapes::loadGLTextureBGRA(int w, int h, GLubyte *image, int glscale) {
  if( isHeadless() ) return 0;

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

void Shapes::loadStencil( const string& filename, int index ) {
  if( isHeadless() ) return;

	string fn = rootDir + filename;
//  fprintf(stderr, "setupAlphaBlendedBMP, rootDir=%s\n", rootDir);
	stencil[ index ] = SDL_LoadBMP( fn.c_str() );

	if( stencil[ index ] == NULL ) {
		stencilImage[ index ] = NULL;
		return;
	}

	// Rearrange the pixelData
	int width  = stencil[ index ]->w;
	int height = stencil[ index ]->h;

	unsigned char * data = (unsigned char *)(stencil[ index ]->pixels);         // the pixel data

	GLubyte* p = new GLubyte[ width * height ];
	int count = 0;
	int c = 0;
	unsigned char r,g,b;
	// the following lines extract R,G and B values from any bitmap
	for(int i = 0; i < width * height; ++i) {
		if(i > 0 && i % width == 0) {
			c += (  stencil[ index ]->pitch - ( width * stencil[ index ]->format->BytesPerPixel ) );
		}
		b = data[c++];
		g = data[c++];
		r = data[c++];

		p[count++] = ( !( r || g || b ) ? 0 : 
		             ( r == 0xff && g == 0xff && b == 0xff ? 2 : 1 ) );
	}

	stencilImage[ index ] = p;
}

void Shapes::setupAlphaBlendedBMP( const string& filename, 
                                   SDL_Surface*& surface, 
                                   GLubyte*& image, 
                                   int red, int green, int blue,
                                   bool isAbsPath,
                                   bool swapImage,
                                   bool grayscale ) {

	if( isHeadless() ) return;

	string fn( isAbsPath ? filename : rootDir + filename ); 

	if( debugFileLoad ) {
		cerr << "file: " << fn << " red=" << red << " green=" << green << " blue=" << blue << endl;
	}

	if(Util::StringCaseCompare(fn.substr(fn.length() - 4, 4), ".bmp")) {
		surface = SDL_LoadBMP( fn.c_str() );
	} else {
		surface = IMG_Load( fn.c_str() );
		if( surface == NULL ) {
			cerr << "Problem loading image(" << fn << "): " << IMG_GetError() << endl;
		}
	}

	if( surface == NULL ) {
		if( debugFileLoad ) {
			cerr << "...not found:" << filename << endl;
		}
		return;
	}


	if( debugFileLoad ) {
		cerr << "...loaded! Bytes per pixel=" << static_cast<int>(surface->format->BytesPerPixel) << endl;
	}

	// Rearrange the pixelData
	int width  = surface->w;
	int height = surface->h;

	/*    if( width != height && 
		( !isPowerOfTwo( width ) ||
		  !isPowerOfTwo( height ) ) ) {
	  cerr << "*** Possible error: Width or Heigth not a power of 2: file=" << fn 
					 << " w=" << width 
					 << " h=" << height 
					 << " bpp=" << (*surface)->format->BitsPerPixel 
					 << " byte/pix=" << (*surface)->format->BytesPerPixel 
					 << " pitch=" << (*surface)->pitch << endl;
	} */

	unsigned char * data = (unsigned char *) surface->pixels;         // the pixel data

	if( swapImage ) {
		int width  = surface->w;
		int height = surface->h;		
		int BytesPerPixel = surface->format->BytesPerPixel;
		for( int i = 0 ; i < (height / 2) ; ++i ) {
			for( int j = 0 ; j < width * BytesPerPixel; j += BytesPerPixel ) {
				for(int k = 0; k < BytesPerPixel; ++k) {
					swap( data[ (i * width * BytesPerPixel) + j + k]
					    , data[ ( (height - i - 1) * width * BytesPerPixel ) + j + k]);
				}
			}
		}
	}

	image = new GLubyte[width * height * 4];
	int count = 0;
	int c = 0;
	unsigned char r,g,b,a;
	// the following lines extract R,G and B values from any bitmap
	for(int i = 0; i < width * height; ++i) {
		if(i > 0 && i % width == 0) {
			c += (  surface->pitch - (width * surface->format->BytesPerPixel) );
		}

		if( surface->format->BytesPerPixel == 1 ) {
			Uint32 pixel_value = (Uint32)data[c++];
			SDL_GetRGB( pixel_value, surface->format, (Uint8*)&b, (Uint8*)&g, (Uint8*)&r);
			a = (GLubyte)( (static_cast<int>(r) == blue && static_cast<int>(g) == green && static_cast<int>(b) == red ? 0x00 : 0xff) );
		} else {
			r = data[c++];
			g = data[c++];
			b = data[c++];
			if( surface->format->BytesPerPixel == 4 ) {
				a = data[c++];
			} else {
				if( grayscale ) {
					a = (GLubyte)( static_cast<float>( r + b + g ) / 3.0f );
					if( a <= 0.05f )
						a = 0;
				} else {
					if(static_cast<int>(r) == blue && static_cast<int>(g) == green && static_cast<int>(b) == red)
						a = static_cast<GLubyte>(0x00);
					else
						a = static_cast<GLubyte>(0xff);
				}
			}
		}
		
		if(Util::StringCaseCompare(fn.substr(fn.length() - 4, 4), ".jpg")) {
			image[count++] = b;
			image[count++] = g;
			image[count++] = r;
		} else {
			image[count++] = r;
			image[count++] = g;
			image[count++] = b;
		}
		image[count++] = a;
	}
}

void Shapes::setupAlphaBlendedBMPGrid( const string& filename, SDL_Surface **surface, 
                                             GLubyte *image[20][20], int imageWidth, int imageHeight,
                                             int tileWidth, int tileHeight, 
                                             int red, int green, int blue,
                                             int nred, int ngreen, int nblue ) {
  if( isHeadless() )
		return;

  string fn = rootDir + filename;
  if(((*surface) = SDL_LoadBMP( fn.c_str() ))) {

    // Rearrange the pixelData
    int width  = (*surface) -> w;
    int height = (*surface) -> h;

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

          image[ x ][ y ][count++] = ( r == red && nred > -1 ? nred : r );
          image[ x ][ y ][count++] = ( g == green && ngreen > -1 ? ngreen : g );
          image[ x ][ y ][count++] = ( b == blue && nblue > -1 ? nblue : b );

					if(static_cast<int>(r) == blue && static_cast<int>(g) == green && static_cast<int>(b) == red)
						image[ x ][ y ][count++] = static_cast<GLubyte>(0x00);
					else
						image[ x ][ y ][count++] = static_cast<GLubyte>(0xff);
        }
      }
    }
  }
}

// places to look for system textures (must end in "")
char *textureDirs[] = { "/textures/", "/cave/default/", "/objects/houses/", "" };

GLuint Shapes::loadSystemTexture( const string& line ) {
  if( isHeadless() ) return 0;

  if( texture_count >= MAX_SYSTEM_TEXTURE_COUNT ) {
    cerr << "Error: *** no more room for system textures!. Not loading: " << line << endl;
    return 0;
  }

  GLuint id = findTextureByName( line );
  if( !id ) {
    textures[texture_count].filename = line;
    int dirCount = 0;
    while( strlen( textureDirs[dirCount] ) ) {
	    string path = textureDirs[dirCount] + textures[texture_count].filename;
	    // load the texture
			SDL_Surface *tmpSurface;
			GLubyte *tmpImage;
			
			setupAlphaBlendedBMP( path, tmpSurface, tmpImage );				
			if( tmpSurface ) {
				id = textures[ texture_count ].id = 
					loadGLTextureBGRA( tmpSurface, tmpImage, GL_LINEAR );			
				SDL_FreeSurface( tmpSurface );
				delete [] tmpImage;
				break;
			} else {
				id = textures[ texture_count ].id = 0;
			}
			dirCount++;
    }
		texture_count++;
  }
  return id;
}

GLuint Shapes::getCursorTexture( int cursorMode ) {
	return cursorTexture[ cursorMode ];
}

GLuint Shapes::loadTextureWithAlpha( string& filename, int r, int g, int b, bool isAbsPath, bool swapImage, bool grayscale ) {
  SDL_Surface *tmpSurface = NULL;
  GLubyte *tmpImage = NULL;
  instance->setupAlphaBlendedBMP( filename, tmpSurface, tmpImage, r, g, b, isAbsPath, swapImage, grayscale );
  GLuint texId = 0;
  if( tmpImage ) texId = instance->loadGLTextureBGRA( tmpSurface, tmpImage, GL_LINEAR );
  delete [] tmpImage;
  if( tmpSurface ) SDL_FreeSurface( tmpSurface );
  return texId;
}

GLuint Shapes::loadAlphaTexture( string& filename, int *width, int *height ) {
  SDL_Surface *tmpSurface = NULL;
  GLubyte *tmpImage = NULL;

  instance->setupImage( filename, tmpSurface, tmpImage );

  GLuint texId = 0;
  if( width && height ) {
	*width = tmpSurface->w;
	*height = tmpSurface->h;
  }
  if( tmpImage ) {
  	//texId = instance->loadGLTextureBGRA( tmpSurface, tmpImage, GL_LINEAR );
  	texId = instance->loadGLTextureBGRA( tmpSurface, tmpImage, GL_NEAREST );
  }
  delete [] tmpImage;
  if( tmpSurface ) SDL_FreeSurface( tmpSurface );
  return texId;
}

void Shapes::setupImage( const string &filename, SDL_Surface*& surface, GLubyte*& image ) {
  if( filename.substr( filename.size() - 4 ) == ".png" ) {
  	setupPNG( filename, surface, image, false, true );
  } else if( filename.substr( filename.size() - 4 ) == ".jpg" ) {
  	setupPNG( filename, surface, image, false, false );
  } else {
  	setupAlphaBlendedBMP( filename, surface, image );
  }	
}

void Shapes::setupPNG( const string& filename, SDL_Surface*& surface, GLubyte*& image, bool isAbsPath, bool hasAlpha ) {

	if( isHeadless() )
		return;

	string fn( isAbsPath ? filename : rootDir + filename );

	if( !hasAlpha ) debugFileLoad = true;
	if( debugFileLoad ) {
		cerr << "file: " << fn << endl;
	}

	surface = IMG_Load( fn.c_str() );

	if( surface == NULL ) {
		cerr << "Problem loading image( " << fn << " ): " <<  IMG_GetError() << endl;
		if( debugFileLoad ) {
			cerr << "...not found:" << filename << endl;
		}
		return;
	}

	if( debugFileLoad ) {
		cerr << "...loaded! Bytes per pixel=" << static_cast<int>(surface->format->BytesPerPixel) << " BPP=" << static_cast<int>(surface->format->BitsPerPixel) << endl;
	}
	debugFileLoad = false;

	// Rearrange the pixelData
	int width  = surface->w;
	int height = surface->h;

/*    if( width != height && ( !isPowerOfTwo( width ) || !isPowerOfTwo( height ) ) ) {
      cerr  << "*** Possible error: Width or Heigth not a power of 2: file=" << fn 
						<< "w=" << width 
						<< " h=" << height 
						<< " bpp=" << (*surface)->format->BitsPerPixel 
						<< " byte/pix=" << (*surface)->format->BytesPerPixel 
						<< " pitch=" <<  (*surface)->pitch << endl;
    } */

	unsigned char * data = (unsigned char *) surface->pixels;         // the pixel data

	image = new GLubyte[width * height * 4];
	int count = 0;
	int c = 0;
	// the following lines extract R,G and B values from any bitmap
	for(int i = 0; i < width * height; ++i) {
		if(i > 0 && i % width == 0) {
			c += surface->pitch - (width * surface->format->BytesPerPixel);
		}		
	   unsigned char r = data[c++];
	   unsigned char g = data[c++];
	   unsigned char b = data[c++];
	   	
	   if( hasAlpha ) {
	  	 unsigned char a = data[c++];
		   image[count++] = b;
		   image[count++] = g;
		   image[count++] = r;
		   image[count++] = a;
	   } else {
	  	 image[count++] = b;
 		   image[count++] = g;
 		   image[count++] = r;
 		   image[count++] = 1;
	   }
	}
}

GLuint Shapes::createAlphaTexture( GLuint alphaTex, GLuint sampleTex, int textureSizeW, int textureSizeH, int width, int height ) {	
	// todo: should be next power of 2 after width/height (maybe cap-ed at 256)
//  int textureSizeW = 256; 
//  int textureSizeH = 256;
//  int width = 256;
//  int height = 256;
	
  unsigned char *texInMem = (unsigned char *) malloc( textureSizeW * textureSizeH * 4);    
  GLuint tex[1];
  
  glGenTextures(1, tex);    
  glBindTexture(GL_TEXTURE_2D, tex[ 0 ]); 
  glPixelStorei(GL_UNPACK_ALIGNMENT, 1);        
  glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_NEAREST);  
  glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_S, GL_CLAMP );
  glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_T, GL_CLAMP ); 
  glTexImage2D( GL_TEXTURE_2D, 0, GL_RGBA, textureSizeW, textureSizeH, 0,
                GL_RGBA, GL_UNSIGNED_BYTE, texInMem );                       

  glDisable( GL_CULL_FACE );
  glDisable( GL_DEPTH_TEST );	  
  
  glPushMatrix();  
  glLoadIdentity();
  
  glDisable( GL_TEXTURE_2D );
  glColor4f( 0, 0, 0, 0 );
  
  glBegin( GL_QUADS );
  glVertex3f( 0, 0, 0 );
  glVertex3f( 0, textureSizeH, 0 );
  glVertex3f( textureSizeW, textureSizeH, 0 );
  glVertex3f( textureSizeW, 0, 0 );
  glEnd();
  
  glEnable( GL_TEXTURE_2D );
  glColor4f(1, 1, 1, 1);

  // draw the grass
  //glEnable( GL_ALPHA_TEST );
  //glAlphaFunc( GL_EQUAL, 0xff );
  glEnable(GL_TEXTURE_2D);
  
  //    glTranslatef( x, y, 0 );
  glBindTexture( GL_TEXTURE_2D, sampleTex );
  glColor4f(1, 1, 1, 1);
  
  glBegin( GL_QUADS );
  glNormal3f( 0, 0, 1 );
  glTexCoord2f( 0, 0 );
  glVertex3f( 0, 0, 0 );
  glTexCoord2f( 0, 1 );
  glVertex3f( 0, height, 0 );
  glTexCoord2f( 1, 1 );
  glVertex3f( width, height, 0 );
  glTexCoord2f( 1, 0 );
  glVertex3f( width, 0, 0 );
  glEnd();

  //glDisable( GL_ALPHA_TEST );
  glDisable(GL_TEXTURE_2D);
  
  // draw the alpha pixels only
  glDisable( GL_CULL_FACE );
  glDisable( GL_DEPTH_TEST );
  glEnable( GL_TEXTURE_2D );
  glColorMask( GL_FALSE, GL_FALSE, GL_FALSE, GL_TRUE );
  glColor4f(1, 1, 1, 1);

  glBindTexture( GL_TEXTURE_2D, alphaTex );      
  glBegin( GL_QUADS );
  glNormal3f( 0, 0, 1 );
  glTexCoord2f( 0, 0 );
  glVertex3f( 0, 0, 0 );
  glTexCoord2f( 0, 1 );
  glVertex3f( 0, height, 0 );
  glTexCoord2f( 1, 1 );
  glVertex3f( width, height, 0 );
  glTexCoord2f( 1, 0 );
  glVertex3f( width, 0, 0 );
  glEnd();	  
  
  glColorMask( GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE );

  // Copy to a texture
  glLoadIdentity();
  glEnable(GL_TEXTURE_2D);
  glBindTexture(GL_TEXTURE_2D, tex[0]);
  glCopyTexSubImage2D(
                     GL_TEXTURE_2D,
                     0,      // MIPMAP level
                     0,      // x texture offset
                     0,      // y texture offset
                     0,              // x window coordinates
                     getSession()->getGameAdapter()->getScreenHeight() - textureSizeH,   // y window coordinates
                     textureSizeW,    // width
                     textureSizeH     // height
                     ); 
  //cerr << "OpenGl result for minimap texture building: " << Util::getOpenGLError() << endl;            
  //  glPopAttrib();
  
  // cover with black
  // todo: this should be the original background, not black
  glDisable( GL_TEXTURE_2D );
  glColor4f(0, 0, 0, 0);
  glBegin( GL_QUADS );
  glVertex3f( 0, 0, 0 );
  glVertex3f( 0, height, 0 );
  glVertex3f( width, height, 0 );
  glVertex3f( width, 0, 0 );
  glEnd();
  glPopMatrix();    

  glDisable( GL_BLEND );
  glEnable( GL_CULL_FACE );
  glEnable( GL_DEPTH_TEST );
  glEnable( GL_TEXTURE_2D );
  
  // copy texture to theme and clean up
  free( texInMem );
  return tex[0];
}


