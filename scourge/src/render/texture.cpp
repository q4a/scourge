/***************************************************************************
                     texture.cpp  -  Class for 2D textures
                             -------------------
    created              : Mon Oct 13 2008
    author               : Vambola Kotkas
    email                : vambola.kotkas@proekspert.ee
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
#include "texture.h"
#include "../session.h"

using std::cerr;
using std::endl;

Texture::Texture()
		: _id( INVALID )
		, _filename()
		, _width( 0 )
		, _height( 0 )
		, _hasAbsolutePath( false )
		, _hasAlpha( false )
		, _isSprite( false )
		, _wantsAnisotropy( false )
		, _isDestoyed( false )
		// TODO: think through member/polymorphing candidates here:
		//, _wantsMipmapping( false )
		//, _isMipmapped( false )
		//, _pixels( NULL )
		, _surface( NULL ) {
}

Texture::~Texture() {
	// debug checks
	if ( _isDestoyed ) {
		cerr << "*** Texture::~Texture() destroyed multiple times." << endl;
		return; // actually should exit() here
	}

	clear();
	_isDestoyed = true;
}

/// Grand unified generic texture loader.

/// Creates an OpenGL texture from a file and tries to set up everything
/// correctly according to the properties of the source image.
/// @return true on success.
/// original was Shapes::loadTexture()

bool Texture::load( const string& filename, bool absolutePath, bool isSprite, bool anisotropy ) {
	// debug checks
	if ( _isDestoyed ) {
		cerr << "*** Texture::load() on destroyed texture." << endl;
		return false; // actually should exit() here
	}
	if ( isSpecified() ) {
		cerr << "*** Texture::load() generating texture (" << _filename << ") without releasing old image." << endl;
		clear();
	}

	_hasAbsolutePath = absolutePath;
	_filename = filename;
	while (_filename[0] == '/' ) {
		_filename = &_filename[1];
	}

	if ( !loadImage() ) return false;

	_isSprite = isSprite;
	_wantsAnisotropy = anisotropy;
	Preferences *prefs = Session::instance->getPreferences();

	GLuint destFormat;
	GLuint srcFormat;
	GLuint minFilter;

	if ( _hasAlpha ) {
		srcFormat = GL_RGBA;
		destFormat = ( prefs->getBpp() > 16 ? GL_RGBA : GL_RGBA4 );
		minFilter = ( _isSprite ? GL_LINEAR : GL_LINEAR_MIPMAP_LINEAR );
	} else {
		srcFormat = GL_RGB;
		destFormat = ( prefs->getBpp() > 16 ? GL_RGB : GL_RGB5 );
		minFilter = GL_LINEAR_MIPMAP_NEAREST;
	}

	glPixelStorei( GL_UNPACK_ALIGNMENT, 4 );
	glGenTextures( 1, &_id );
	glBindTexture( GL_TEXTURE_2D, _id );

	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, minFilter );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );

	// Enable anisotropic filtering if requested, mipmapping is enabled
	// and the hardware supports it.
	if ( _wantsAnisotropy && !_hasAlpha
	        && strstr( ( char* )glGetString( GL_EXTENSIONS ), "GL_EXT_texture_filter_anisotropic" )
	        && prefs->getAnisoFilter() ) {
		float maxAnisotropy;
		glGetFloatv( GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT, &maxAnisotropy );
		glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, maxAnisotropy );
	} else {
		glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, 1.0f );
	}

	// lordthoran said glTexImage2D causes white textures
	// glTexImage2D( GL_TEXTURE_2D, 0, destFormat, _width, _height, 0, srcFormat, GL_UNSIGNED_BYTE, surface->pixels );

	gluBuild2DMipmaps( GL_TEXTURE_2D, destFormat, _width, _height, srcFormat, GL_UNSIGNED_BYTE, _surface->pixels );

	unloadImage();
	return true;
}

void Texture::clear() {
	// debug checks
	if ( _isDestoyed ) {
		cerr << "*** Texture::clear() on destroyed texture." << endl;
		return; // actually should exit() here
	}

	unloadImage();

	if ( isSpecified() ) {
		glDeleteTextures( 1, &_id );
		_id = INVALID;
		_filename.clear();
		_width = 0;
		_height = 0;
	}
}

/// Loads image using SDL_image and extracts all interesting data from it
/// assumes _filename is set
/// @return true on success

bool Texture::loadImage() {
	// debug checks
	if ( _isDestoyed ) {
		cerr << "*** Texture::loadImage() on destroyed texture." << endl;
		return false; // actually should exit() here
	}
	// just silently unload or make noise?
	unloadImage();

	string path = ( _hasAbsolutePath ? _filename : rootDir + "/" + _filename );

	_surface = IMG_Load( path.c_str() );
	if ( _surface == NULL ) {
		cerr << "*** Error loading image (" << path << "): " << IMG_GetError() << endl;
		return false;
	}

	_width = _surface->w;
	_height = _surface->h;
	_hasAlpha = _surface->format->Amask != 0;

	// TODO: refactor Constants::checkTexture into Texture::check
	Constants::checkTexture( "Texture::load", _width, _height );
	return true;
}

/// Creatres a tile of an SDL surface as a texture.
/// This function allows you to define the size of a tile and which tile you want. It
/// is then created as a texture from the appropriate part of the SDL surface.
/// @return true on success.
/// used to be Shapes::createTileTexture()

bool Texture::createTile( SDL_Surface **surface, int tileX, int tileY, int tileWidth, int tileHeight ) {
	// debug checks
	if ( _isDestoyed ) {
		cerr << "*** Texture::createTile() on destroyed texture." << endl;
		return false; // actually should exit() here
	}
	if ( isSpecified() ) {
		cerr << "*** Texture::createTile() generating texture (" << tileX << "," << tileY << ") without releasing old image." << endl;
		clear();
	}

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


	_width = tileWidth;
	_height = tileHeight;
	_hasAlpha = true;

	Preferences *prefs = Session::instance->getPreferences();

	// Create The Texture
	glGenTextures( 1, &_id );

	// Typical Texture Generation Using Data From The Bitmap
	glBindTexture( GL_TEXTURE_2D, _id );

	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );

	gluBuild2DMipmaps( GL_TEXTURE_2D, ( prefs->getBpp() > 16 ? GL_RGBA : GL_RGBA4 ), tileWidth, tileHeight, GL_RGBA, GL_UNSIGNED_BYTE, &image[0] );

	return true;
}


/// Unloads loaded image if any

void Texture::unloadImage() {
	if ( _surface != NULL ) {
		SDL_FreeSurface( _surface );
		_surface = NULL;
	}
}


/// Adds the alpha channel of the alphaTex texture to the sampleTex texture.
/// @return true on success.
/// used to be Shapes::createAlphaTexture()

bool Texture::createAlpha( Texture const* alpha, Texture const* sample, int textureSizeW, int textureSizeH, int width, int height ) {
	// debug checks
	if ( _isDestoyed ) {
		cerr << "*** Texture::createAlpha() on destroyed texture." << endl;
		return false; // actually should exit() here
	}
	if ( isSpecified() ) {
		cerr << "*** Texture::createAlpha() generating texture without releasing old image." << endl;
		clear();
	}
	if ( alpha == NULL ) {
		cerr << "*** Texture::createAlpha() called with alpha texture == NULL." << endl;
	}

	if ( sample == NULL ) {
		cerr << "*** Texture::createAlpha() called with sample texture == NULL." << endl;
	}

// todo: should be next power of 2 after width/height (maybe cap-ed at 256)
//  int textureSizeW = 256;
//  int textureSizeH = 256;
//  int width = 256;
//  int height = 256;

	unsigned char *texInMem = ( unsigned char * ) malloc( textureSizeW * textureSizeH * 4 );
	//GLuint tex[1];

	glGenTextures( 1, &_id );
	glBindTexture( GL_TEXTURE_2D, _id );
	glPixelStorei( GL_UNPACK_ALIGNMENT, 1 );

	Preferences *prefs = Session::instance->getPreferences();

	/**
	 * This method should not create mip-maps. They don't work well with alpha-tested textures and cause flickering.
	 */
	//glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, isSprite ? GL_NEAREST : GL_LINEAR_MIPMAP_NEAREST );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP );
	glTexImage2D( GL_TEXTURE_2D, 0, ( prefs->getBpp() > 16 ? GL_RGBA : GL_RGBA4 ), textureSizeW, textureSizeH, 0, GL_RGBA, GL_UNSIGNED_BYTE, texInMem );
	//if( !isSprite ) gluBuild2DMipmaps(GL_TEXTURE_2D, 4, textureSizeW, textureSizeH, GL_BGRA, GL_UNSIGNED_BYTE, texInMem);

	glDisable( GL_CULL_FACE );
	glDisable( GL_DEPTH_TEST );

	glPushMatrix();
	glLoadIdentity();

	glDisable( GL_TEXTURE_2D );
	glColor4f( 0, 0, 0, 0 );

	glBegin( GL_TRIANGLE_STRIP );
	glVertex3f( 0, 0, 0 );
	glVertex3f( textureSizeW, 0, 0 );
	glVertex3f( 0, textureSizeH, 0 );
	glVertex3f( textureSizeW, textureSizeH, 0 );
	glEnd();

	glEnable( GL_TEXTURE_2D );
	glColor4f( 1, 1, 1, 1 );

	// draw the grass
	//glEnable( GL_ALPHA_TEST );
	//glAlphaFunc( GL_EQUAL, 0xff );
	glEnable( GL_TEXTURE_2D );

	//    glTranslatef( x, y, 0 );
	if ( sample != NULL ) sample->glBind();
	glColor4f( 1, 1, 1, 1 );
//  glNormal3f( 0, 0, 1 );

	glBegin( GL_TRIANGLE_STRIP );
	glTexCoord2f( 0, 0 );
	glVertex3f( 0, 0, 0 );
	glTexCoord2f( 1, 0 );
	glVertex3f( width, 0, 0 );
	glTexCoord2f( 0, 1 );
	glVertex3f( 0, height, 0 );
	glTexCoord2f( 1, 1 );
	glVertex3f( width, height, 0 );
	glEnd();

	//glDisable( GL_ALPHA_TEST );
	glDisable( GL_TEXTURE_2D );

	// draw the alpha pixels only
	glDisable( GL_CULL_FACE );
	glDisable( GL_DEPTH_TEST );
	glEnable( GL_TEXTURE_2D );
	glColorMask( GL_FALSE, GL_FALSE, GL_FALSE, GL_TRUE );
	glColor4f( 1, 1, 1, 1 );

	if ( alpha != NULL ) alpha->glBind();
//  glNormal3f( 0, 0, 1 );
	glBegin( GL_TRIANGLE_STRIP );
	glTexCoord2f( 0, 0 );
	glVertex3f( 0, 0, 0 );
	glTexCoord2f( 1, 0 );
	glVertex3f( width, 0, 0 );
	glTexCoord2f( 0, 1 );
	glVertex3f( 0, height, 0 );
	glTexCoord2f( 1, 1 );
	glVertex3f( width, height, 0 );
	glEnd();

	glColorMask( GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE );

	// Copy to a texture
	glLoadIdentity();
	glEnable( GL_TEXTURE_2D );
	glBindTexture( GL_TEXTURE_2D, _id );
	glCopyTexSubImage2D(
	  GL_TEXTURE_2D,
	  0,      // MIPMAP level
	  0,      // x texture offset
	  0,      // y texture offset
	  0,              // x window coordinates
	  Session::instance->getGameAdapter()->getScreenHeight() - textureSizeH,   // y window coordinates
	  textureSizeW,    // width
	  textureSizeH     // height
	);
	//cerr << "OpenGl result for minimap texture building: " << Util::getOpenGLError() << endl;
	//  glPopAttrib();

	// cover with black
	// todo: this should be the original background, not black
	glDisable( GL_TEXTURE_2D );
	glColor4f( 0, 0, 0, 0 );
	glBegin( GL_TRIANGLE_STRIP );
	glVertex3f( 0, 0, 0 );
	glVertex3f( width, 0, 0 );
	glVertex3f( 0, height, 0 );
	glVertex3f( width, height, 0 );
	glEnd();
	glPopMatrix();

	glDisable( GL_BLEND );
	glEnable( GL_CULL_FACE );
	glEnable( GL_DEPTH_TEST );
	glEnable( GL_TEXTURE_2D );

	// copy texture to theme and clean up
	free( texInMem );
	return true;
}

/// loads texture from screen.bmp file in given directory
/// @return true on success.
/// used to be SavegameDialog::loadScreenshot()

bool Texture::loadShot( const string& dirName ) {

	// debug checks
	if ( _isDestoyed ) {
		cerr << "*** Texture::loadShot() on destroyed texture." << endl;
		return false; // actually should exit() here
	}
	if ( isSpecified() ) {
		cerr << "*** Texture::loadShot() generating texture without releasing old image (" << _filename << ")." << endl;
		clear();
	}

	_filename = get_file_name( dirName + "/screen.bmp" );
	SDL_Surface* surface = SDL_LoadBMP( _filename.c_str() );

	if ( surface == NULL ) {
		cerr << "*** Error loading screenshot image (" << _filename << "): " << IMG_GetError() << endl;
		return false;
	}

	glPixelStorei( GL_UNPACK_ALIGNMENT, 4 );
	glGenTextures( 1, &_id );
	glBindTexture( GL_TEXTURE_2D, _id );

	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );

	gluBuild2DMipmaps( GL_TEXTURE_2D, GL_RGB, surface->w, surface->h, GL_BGR, GL_UNSIGNED_BYTE, surface->pixels );

	SDL_FreeSurface( surface );
	return true;
}

