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
#include <fstream>
#include <iostream>

using namespace std;

#define ALWAYS_RELOAD_THEME 1

//#define VARIATION_BASE 10.0f
#define VARIATION_BASE 4

bool Shapes::debugFileLoad = false;

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
  strcpy( this->name, name );
  this->shapePal = shapePal;
  for(int i = 0; i < THEME_REF_COUNT; i++)
    themeRefMap[ themeRefName[i] ] = i;
  lavaData = NULL;
  floorData = NULL;
}

WallTheme::~WallTheme() {
  unload();
  if( lavaData ) {
    free( lavaData );
    lavaData = NULL;
  }
  if( floorData ) {
    free( floorData );
    floorData = NULL;
  }
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
  string path, bmp;  
  if ( texture && strcmp( texture, "null" ) ) {
    string s = texture;
    GLuint id;
    if ( loadedTextures.find(s) == loadedTextures.end() ) {

      // see if it's a system texture (no need to double load it)
      bmp = texture + string(".bmp");
      path = "/" + bmp;

      // keep lava texture data
      if( isCave() && 
          ref == THEME_REF_PASSAGE_FLOOR && 
          face == GLShape::FRONT_SIDE ) {
        shapePal->getBMPData( path, &lavaData );
      }
      if( isCave() && 
          ref == THEME_REF_PASSAGE_FLOOR && 
          face == GLShape::TOP_SIDE ) {
        shapePal->getBMPData( path, &floorData );
      }

      id = shapePal->findTextureByName( bmp );
      if ( id == 0 ) {
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
  string bmp;
//  cerr << "*** Dumping theme: " << getName() << endl;
  for (map<string,GLuint>::iterator i=loadedTextures.begin(); i!=loadedTextures.end(); ++i) {
    string s = i->first;
    GLuint id = i->second;

    // don't delete system textures!
    bmp = s + ".bmp";
    id = shapePal->findTextureByName( bmp );
    if ( id == 0 ) {
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

Shapes *Shapes::instance = NULL;

Shapes::Shapes( bool headless ){
  texture_count = 0;
  textureGroupCount = 1;
	textureGroup[0][0] = textureGroup[0][1] = textureGroup[0][2] = 0;
  themeCount = allThemeCount = caveThemeCount = 0;
  currentTheme = NULL;
  this->headless = headless;
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
  GLubyte *areaImage;
	string areabmp("/textures/area.bmp");
  setupAlphaBlendedBMP(areabmp, &area, &areaImage);
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

  // create shapes
  for(int i = 0; i < (int)shapeValueVector.size(); i++) {
    ShapeValues *sv = shapeValueVector[i];
    // Resolve the texture group.
    // For theme-based shapes, leave texture NULL, they will be resolved later.
    bool themeBasedShape = false;
    GLuint *texture = textureGroup[ 0 ];
		if( strlen( sv->theme ) ) {
			themeBasedShape = true;
		} else if( strlen( sv->textures ) ) {
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
    if(themeBasedShape) {
      themeShapes.push_back( shapes[(i + 1)] );
      //string s = ( sv->textureGroupIndex + 6 );
			string s = sv->theme;
      themeShapeRef.push_back( s );
    } else {
      if( !headless ) shapes[(i + 1)]->initialize();
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

    string s = sv->name;
    shapeMap[s] = shapes[(i + 1)];
  }
  // remember the number of shapes
  shapeCount = (int)shapeValueVector.size() + 1;

  // clean up temp. shape objects 
  // FIXME: do we need to free the vector's elements?
  if ( !shapeValueVector.empty() ) shapeValueVector.clear();

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
  if( !headless ) shapes[shapeCount]->initialize();
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
  //loadTheme( themes[ (int)( (float)(themeCount - 1) * rand()/RAND_MAX ) + 1 ] );
  loadTheme( themes[ Util::dice( themeCount ) ] );
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
//  cerr << "*** Using theme: " << theme->getName() << " current=" << (!currentTheme ? "null" : currentTheme->getName()) << endl;
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
}

char const* Shapes::getRandomDescription(int descriptionGroup) {
  if(descriptionGroup >= 0 && descriptionGroup < (int)descriptions.size()) {
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

GLShape *Shapes::findShapeByName(const char *name, bool variation) {
  if(!name || !strlen(name)) return NULL;
  string s = name;
  if(shapeMap.find(s) == shapeMap.end()) {
    cerr << "&&& warning: could not find shape by name " << s << endl;
    return NULL;
  }
  GLShape *shape = shapeMap[s];
  if( !variation || shape->getVariationShapesCount() == 0 ) return shape;

  int n = Util::dice( VARIATION_BASE + shape->getVariationShapesCount() );
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

bool inline isPowerOfTwo( const GLuint n ) {
	return (n != 0 && ((n & (n - 1)) == 0));
}

// FIXME: this should be similar to loadTextureWithAlpha but adding alpha
// screws up the stencils in caves. No time to debug now.
GLuint Shapes::getBMPData( const string& filename, GLubyte **buf, int *imgwidth, int *imgheight  ) {
	GLuint ret = 0;
	string fn = rootDir + filename;

//  cerr << "loading lava data: " << fn << endl;

	// Create storage space for the texture
	SDL_Surface *TextureImage[1];

	// Load The Bitmap, Check For Errors, If Bitmap's Not Found Quit
	if( ( TextureImage[0] = SDL_LoadBMP( fn.c_str() ) ) ) {

/*		if( TextureImage[0]->w != TextureImage[0]->h && 
				( !isPowerOfTwo( TextureImage[0]->w ) ||
					!isPowerOfTwo( TextureImage[0]->h ) ) ) {
			fprintf(stderr, "*** Possible error: Width or Heigth not a power of 2: name=%s pitch=%d width=%d height=%d\n", 
							fn.c_str(), (TextureImage[0]->pitch/3), TextureImage[0]->w, TextureImage[0]->h);
		}*/

		Constants::checkTexture("Shapes::loadGLTextures", 
														TextureImage[0]->w, TextureImage[0]->h);

//    cerr << "\tdim=" << TextureImage[0]->w << "," << TextureImage[0]->h << endl;

		int width = TextureImage[0]->w;
		int height = TextureImage[0]->h;

		if( imgwidth ) *imgwidth = width;
		if( imgheight )	*imgheight = height;

		unsigned char * data = (unsigned char *)(TextureImage[0]->pixels);				 // the pixel data

		//if( *buf ) free( *buf );
		if( !( *buf ) )
			*buf = (GLubyte*)malloc( 3 * width * height * sizeof( GLubyte ));
		int count = 0;
		int c = 0;
		unsigned char r,g,b;
		// the following lines extract R,G and B values from any bitmap
		for( int i = 0; i < width * height; ++i ) {
			if( i > 0 && i % width == 0 )
				c += (  TextureImage[0]->pitch - ( width * TextureImage[0]->format->BytesPerPixel ) );
			r = data[c++];
			g = data[c++];
			b = data[c++];

			(*buf)[count++] = b;
			(*buf)[count++] = g;
			(*buf)[count++] = r;
		}

		ret = 1;
	} else {
		cerr << "Unable to load " << fn << endl;
	}

	// Free up any memory we may have used
	if( TextureImage[0] )
		SDL_FreeSurface( TextureImage[0] );

	return ret;
}


/* function to load in bitmap as a GL texture */
GLuint Shapes::loadGLTextures(const string& filename) {

	if( headless ) 
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

void Shapes::loadStencil( const string& filename, int index ) {
  if( headless ) return;

  GLubyte *p = NULL;
  string fn = rootDir + filename;
//  fprintf(stderr, "setupAlphaBlendedBMP, rootDir=%s\n", rootDir);
  if( ( stencil[ index ] = SDL_LoadBMP( fn.c_str() ) ) != NULL ) {

    // Rearrange the pixelData
    int width  = stencil[ index ]->w;
    int height = stencil[ index ]->h;

/*    if( width != height && 
        ( !isPowerOfTwo( width ) ||
          !isPowerOfTwo( height ) ) ) {
      fprintf(stderr, "*** Possible error: Width or Heigth not a power of 2: file=%s w=%d h=%d bpp=%d byte/pix=%d pitch=%d\n", 
              fn.c_str(), width, height, 
              stencil[ index ]->format->BitsPerPixel,
              stencil[ index ]->format->BytesPerPixel, 
              stencil[ index ]->pitch);
    } */

    unsigned char * data = (unsigned char *)(stencil[ index ]->pixels);         // the pixel data

    p = (GLubyte*)malloc( width * height * sizeof( GLubyte ));
    int count = 0;
    int c = 0;
    unsigned char r,g,b;
    // the following lines extract R,G and B values from any bitmap
    for(int i = 0; i < width * height; ++i) {
      if(i > 0 && i % width == 0)
        c += (  stencil[ index ]->pitch - ( width * stencil[ index ]->format->BytesPerPixel ) );
      b = data[c++];
      g = data[c++];
      r = data[c++];

      p[count++] = ( !( r || g || b ) ? 0 : 
                     ( r == 0xff && g == 0xff && b == 0xff ? 2 : 1 ) );
    }
  }
  stencilImage[ index ] = p;
}

void Shapes::setupAlphaBlendedBMP( const string& filename, 
																	 SDL_Surface **surface, 
																	 GLubyte **image, 
																	 int red, int green, int blue,
																	 bool isAbsPath,
																	 bool swapImage,
																	 bool grayscale ) {

	if( headless ) return;

	GLubyte *p = NULL;
	string fn;
	if( isAbsPath ) {
		fn = filename;
	} else {
		fn = rootDir;
		fn.append( filename );
	}

	if( debugFileLoad ) {
		cerr << "file: " << fn << " red=" << red << " green=" << green << " blue=" << blue << endl;
	}

	if(Util::StringCaseCompare(fn.substr(fn.length() - 4, 4), ".bmp")) {
		(*surface) = SDL_LoadBMP( fn.c_str() );
	} else {
		*surface = IMG_Load( fn.c_str() );
		if( !(*surface) ) {
			cerr << "Problem loading image(" << fn << "): " << IMG_GetError() << endl;
		}
	}
  if( *surface ) {
		if( debugFileLoad ) {
			cerr << "...loaded! Bytes per pixel=" << (int)((*surface)->format->BytesPerPixel) << endl;
		}

    // Rearrange the pixelData
    int width  = (*surface) -> w;
    int height = (*surface) -> h;

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

    unsigned char * data = (unsigned char *) ((*surface) -> pixels);         // the pixel data

		if( swapImage ) {
			int width  = (*surface)->w;
			int height = (*surface)->h;		
			int BytesPerPixel = (*surface)->format->BytesPerPixel;
			for( int i = 0 ; i < (height / 2) ; ++i )
				for( int j = 0 ; j < width * BytesPerPixel; j += BytesPerPixel )
					for(int k = 0; k < BytesPerPixel; ++k)
						swap( data[ (i * width * BytesPerPixel) + j + k], data[ ( (height - i - 1) * width * BytesPerPixel ) + j + k]);
		}

    p = (GLubyte*)malloc(width * height * 4 * sizeof( GLubyte ));
    int count = 0;
    int c = 0;
    unsigned char r,g,b,a;
    // the following lines extract R,G and B values from any bitmap
    for(int i = 0; i < width * height; ++i) {
      if(i > 0 && i % width == 0) {
        c += (  (*surface)->pitch - (width * (*surface)->format->BytesPerPixel) );
			}

			if( (*surface)->format->BytesPerPixel == 1 ) {
				Uint32 pixel_value = (Uint32)data[c++];
				SDL_GetRGB( pixel_value, (*surface)->format, (Uint8*)&b, (Uint8*)&g, (Uint8*)&r);
				a = (GLubyte)( ((int)r == blue && (int)g == green && (int)b == red ? 0x00 : 0xff) );
			} else {
				r = data[c++];
				g = data[c++];
				b = data[c++];
				if( (*surface)->format->BytesPerPixel == 4 ) {
					a = data[c++];
				} else {
					if( grayscale ) {
						a = (GLubyte)( (float)( r + b + g ) / 3.0f );
						if( a <= 0.05f ) a = 0;
					} else {
						a = (GLubyte)( ((int)r == blue && (int)g == green && (int)b == red ? 0x00 : 0xff) );
					}
				}
			}
			
			p[count++] = r;
			p[count++] = g;
			p[count++] = b;
			p[count++] = a;
		}
  } else {
		if( debugFileLoad ) {
			cerr << "...not found:" << filename << endl;
		}
	}

  (*image) = p;
}

void Shapes::setupAlphaBlendedBMPGrid( const string& filename, SDL_Surface **surface, 
                                             GLubyte *image[20][20], int imageWidth, int imageHeight,
                                             int tileWidth, int tileHeight, 
                                             int red, int green, int blue,
                                             int nred, int ngreen, int nblue ) {
  if( headless ) return;
  
//  cerr << "file: " << filename << " red=" << red << " green=" << green << " blue=" << blue << endl;

  //  *image = NULL;
  string fn = rootDir + filename;
  if(((*surface) = SDL_LoadBMP( fn.c_str() ))) {

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

GLuint Shapes::loadSystemTexture( const string& line ) {
  if( headless ) return 0;

  if( texture_count >= MAX_SYSTEM_TEXTURE_COUNT ) {
    cerr << "Error: *** no more room for system textures!. Not loading: " << line << endl;
    return 0;
  }

  GLuint id = findTextureByName( line );
  if( !id ) {
    textures[texture_count].filename = line;
    string path = "/textures/" + textures[texture_count].filename;
    // load the texture
		/*
    id = textures[ texture_count ].id = loadGLTextures( path );
    texture_count++;
		*/

		SDL_Surface *tmpSurface;
		GLubyte *tmpImage;
		setupAlphaBlendedBMP( path, &tmpSurface, &tmpImage );
		if( tmpSurface ) {
			id = textures[ texture_count ].id = 
				loadGLTextureBGRA( tmpSurface, tmpImage, GL_LINEAR );			
			SDL_FreeSurface( tmpSurface );
			free( tmpImage );
		} else {
			id = textures[ texture_count ].id = 0;
		}
		texture_count++;
  }
  return id;
}

GLuint Shapes::getCursorTexture( int cursorMode ) {
	return cursorTexture[ cursorMode ];
/*
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
	*/
}

GLuint Shapes::loadTextureWithAlpha( string& filename, int r, int g, int b, bool isAbsPath, bool swapImage, bool grayscale ) {
  SDL_Surface *tmpSurface = NULL;
  GLubyte *tmpImage = NULL;
  instance->setupAlphaBlendedBMP( filename, &tmpSurface, &tmpImage, r, g, b, isAbsPath, swapImage, grayscale );
  GLuint texId = 0;
  if( tmpImage ) texId = instance->loadGLTextureBGRA( tmpSurface, tmpImage, GL_LINEAR );
  if( tmpImage ) free( tmpImage );
  if( tmpSurface ) SDL_FreeSurface( tmpSurface );
  return texId;
}

GLuint Shapes::loadAlphaTexture( string& filename, int *width, int *height ) {
  SDL_Surface *tmpSurface = NULL;
  GLubyte *tmpImage = NULL;

  if( filename.substr( filename.size() - 4 ) == ".png" ) {
	instance->setupPNG( filename, &tmpSurface, &tmpImage );
  } else {
	instance->setupAlphaBlendedBMP( filename, &tmpSurface, &tmpImage );
  }

  GLuint texId = 0;
  if( width && height ) {
	*width = tmpSurface->w;
	*height = tmpSurface->h;
  }
  if( tmpImage ) texId = instance->loadGLTextureBGRA( tmpSurface, tmpImage, GL_LINEAR );
  if( tmpImage ) free( tmpImage );
  if( tmpSurface ) SDL_FreeSurface( tmpSurface );
  return texId;
}

void Shapes::setupPNG( const string& filename, SDL_Surface **surface, GLubyte **image, bool isAbsPath ) {

  if( headless ) return;

  GLubyte *p = NULL;
  string fn;
	if( isAbsPath ) {
		fn = filename;
	} else {
		fn = rootDir + filename;
	}

	if( debugFileLoad ) {
			cerr << "file: " << fn << endl;
	}

	*surface = IMG_Load( fn.c_str() );
	if( !(*surface) ) {
		cerr << "Problem loading image( " << fn << " ): " <<  IMG_GetError() << endl;
	}
  if( *surface ) {

	if( debugFileLoad ) {
		cerr << "...loaded! Bytes per pixel=" << (int)((*surface)->format->BytesPerPixel) << endl;
	}

    // Rearrange the pixelData
    int width  = (*surface) -> w;
    int height = (*surface) -> h;

/*    if( width != height && ( !isPowerOfTwo( width ) || !isPowerOfTwo( height ) ) ) {
      cerr  << "*** Possible error: Width or Heigth not a power of 2: file=" << fn 
						<< "w=" << width 
						<< " h=" << height 
						<< " bpp=" << (*surface)->format->BitsPerPixel 
						<< " byte/pix=" << (*surface)->format->BytesPerPixel 
						<< " pitch=" <<  (*surface)->pitch << endl;
    } */

    unsigned char * data = (unsigned char *) ((*surface) -> pixels);         // the pixel data

    p = (GLubyte*)malloc(width * height * 4 * sizeof( GLubyte ));
    int count = 0;
    int c = 0;
    unsigned char r,g,b,a;
    // the following lines extract R,G and B values from any bitmap
    for(int i = 0; i < width * height; ++i) {
      if(i > 0 && i % width == 0) {
        c += (  (*surface)->pitch - (width * (*surface)->format->BytesPerPixel) );
			}

			r = data[c++];
			g = data[c++];
			b = data[c++];
			a = data[c++];
			
			p[count++] = b;
			p[count++] = g;
			p[count++] = r;
			p[count++] = a;
		}
  } else {
		if( debugFileLoad ) {
			cerr << "...not found:" << filename << endl;
		}
	}

  (*image) = p;
}

