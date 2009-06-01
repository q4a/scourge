/***************************************************************************
                     texture.cpp  -  Class for 2D OpenGL textures
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
#include "vector"

using namespace std;

Texture::NodeVec Texture::mainList;
Texture::Actual Texture::emptyNode;
Texture Texture::empty;

Texture::Actual::Actual()
		: _id( INVALID )
		, _cntr( 0 )
		, _filename()
		, _width( 0 )
		, _height( 0 )
		, _hasAlpha( false )
		, _isSprite( false )
		, _priority( -1.0F )
		, _isComplete( false )
		, _wantsAnisotropy( false )
		// TODO: think through member/polymorphing candidates here:
		//, _wantsMipmapping( false )
		//, _isMipmapped( false )
		//, _pixels( NULL )
		, _surface( NULL ) {
}

Texture::Actual::~Actual() {
	clear();

	NodeVec::iterator it = mainList.begin();
	while ( it != mainList.end() && *it != this ) {
		++it;
	}
	if ( it != mainList.end() ) mainList.erase( it );
}

/// Clears Texture::Actual into unspecified state.

void Texture::Actual::clear() {
	unloadImage();

	if ( _isComplete ) {
		glDeleteTextures( 1, &_id );
		_id = INVALID;
		_isComplete = false;
	}

	_filename.clear();
	_width = 0;
	_height = 0;
	_priority = -1.0F;
}

/// Unloads loaded image (if there is).

void Texture::Actual::unloadImage() {
	if ( _surface != NULL ) {
		SDL_FreeSurface( _surface );
		_surface = NULL;
	}
}

/// Checks if file at path exists, sets Texture::Actual into INPROGRESS state

bool Texture::Actual::load( const string& path, bool isSprite, bool anisotropy ) {
	// debug checks
	if ( _filename.compare( "" ) != 0 ) {
		cerr << "*** Texture::Actual::load() over (" << _filename << ") with (" << path << ") refused." << endl;
		return false;
	}
	FILE* fp = fopen( path.c_str(), "rb" );
	if ( fp == NULL) {
		cerr << "*** Texture::Actual::load() cannot open (" << path << ")." << endl;
		return false;
	}
	fclose( fp );

	_filename = path;
	_id = INPROGRESS;
	_width = -1;
	_height = -1;
	_isSprite = isSprite;
	_wantsAnisotropy = anisotropy;
//	if ( _filename == "scourge_data/textures/wood.png" )
//		return letsToBind();

	return true;
}

bool Texture::Actual::createTile( SDL_Surface const* surface, int tileX, int tileY, int tileWidth, int tileHeight ) {
	// debug checks
	if ( _filename.compare( "" ) != 0 ) {
		cerr << "*** Texture::Actual::createTile() over (" << _filename << ") refused." << endl;
		return false;
	}

	_filename = "a tile";
	_width = tileWidth;
	_height = tileHeight;
	_hasAlpha = true;


	// The raw data of the source image.
	unsigned char * data = ( unsigned char * ) ( surface->pixels );
	// The destination image (a single tile)
	std::vector<GLubyte> image( tileWidth * tileHeight * 4 );

	int count = 0;
	// where the tile starts in a line
	int offs = tileX * tileWidth * 4;
	// where the tile ends in a line
	int rest = ( tileX + 1 ) * tileWidth * 4;
	// Current position in the source data
	int c = offs + ( tileY * tileHeight * surface->pitch );
	// the following lines extract R,G and B values from any bitmap

	for ( int i = 0; i < tileWidth * tileHeight; ++i ) {

		if ( i > 0 && i % tileWidth == 0 ) {
			// skip the rest of the line
			c += ( surface->pitch - rest );
			// skip the offset (go to where the tile starts)
			c += offs;
		}

		for ( int p = 0; p < 4; p++ ) {
			image[count++] = data[c++];
		}
	}

	Preferences *prefs = Session::instance->getPreferences();

	// Create The Texture
	glGenTextures( 1, &_id );

	// Typical Texture Generation Using Data From The Bitmap
	glBindTexture( GL_TEXTURE_2D, _id );

	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );

	gluBuild2DMipmaps( GL_TEXTURE_2D, ( prefs->getBpp() > 16 ? GL_RGBA : GL_RGBA4 ), tileWidth, tileHeight, GL_RGBA, GL_UNSIGNED_BYTE, &image[0] );

	assert( _id != INVALID && _id != INPROGRESS );  
	_isComplete = true;
	return true;
}

bool Texture::Actual::matches( Actual *other ) {
	return( this->_id != INVALID && other->_id != INVALID && this->_id == other->_id );
}

bool Texture::Actual::createEdgeBlended( const string& path, Actual* original, Actual* west, Actual* east, Actual* south, Actual* north,
                                         Actual* edge, Actual* corner, Actual* tip, Actual* hole ) {
	// debug checks
	if ( original == NULL || !original->letsToBind() ) {
		cerr << "*** Texture::Actual::createEdgeBlended() with missing original texture." << endl;
		return false;
	}
	if ( west == NULL || !west->letsToBind() ) {
//		cerr << "*** Texture::Actual::createEdgeBlended() with missing west texture." << endl;
//		return false;
	}
	if ( east == NULL || !east->letsToBind() ) {
//		cerr << "*** Texture::Actual::createEdgeBlended() with missing east texture." << endl;
//		return false;
	}
	if ( south == NULL || !south->letsToBind() ) {
//		cerr << "*** Texture::Actual::createEdgeBlended() with missing south texture." << endl;
//		return false;
	}
	if ( north == NULL || !north->letsToBind() ) {
//		cerr << "*** Texture::Actual::createEdgeBlended() with missing north texture." << endl;
//		return false;
	}
	if ( edge == NULL || !edge->letsToBind() ) {
		cerr << "*** Texture::Actual::createEdgeBlended() with missing edge texture." << endl;
		return false;
	}
	if ( corner == NULL || !corner->letsToBind() ) {
		cerr << "*** Texture::Actual::createEdgeBlended() with missing corner texture." << endl;
		return false;
	}
	if ( tip == NULL || !tip->letsToBind() ) {
		cerr << "*** Texture::Actual::createEdgeBlended() with missing tip texture." << endl;
		return false;
	}
	if ( hole == NULL || !hole->letsToBind() ) {
		cerr << "*** Texture::Actual::createEdgeBlended() with missing hole texture." << endl;
		return false;
	}
	
	_filename = path;
	
	Preferences *prefs = Session::instance->getPreferences();
	int textureSizeW = 256;
	int textureSizeH = 256;

	cerr << "creating blended edge: " << _filename << endl;

	_width = textureSizeW;
	_height = textureSizeH;
	_isSprite = original->_isSprite;
	_wantsAnisotropy = original->_wantsAnisotropy;	
	_hasAlpha = true;
	
	std::vector<unsigned char*> texInMem( textureSizeW * textureSizeH * 4 );
			
	glGenTextures( 1, &_id );
	glBindTexture( GL_TEXTURE_2D, _id );
	glPixelStorei( GL_UNPACK_ALIGNMENT, 1 );
	
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST );
	
	glTexImage2D( GL_TEXTURE_2D, 0, ( prefs->getBpp() > 16 ? GL_RGBA : GL_RGBA4 ), textureSizeW, textureSizeH, 0, GL_RGBA, GL_UNSIGNED_BYTE, &texInMem[0] );
	

	glPushMatrix();
	GLuint background = saveAreaUnder( 0, 0, textureSizeW, textureSizeH );

	// create a tmp alpha texture
	vector<float> angles;
	vector<Actual*> tmps;
	
	if( !north->matches( original ) && north->matches( east ) ) {
		Actual *tmp2 = new Actual;
		tmp2->createAlpha( corner, &north, 1, _width, _height, _width, _height );
		tmps.push_back( tmp2 );
		angles.push_back( 180.0f );
	}
	if( !east->matches( original ) && east->matches( south ) ) {
		Actual *tmp2 = new Actual;
		tmp2->createAlpha( corner, &east, 1, _width, _height, _width, _height );
		tmps.push_back( tmp2 );
		angles.push_back( 90.0f );
	}
	if( !south->matches( original ) && south->matches( west ) ) {
		Actual *tmp2 = new Actual;
		tmp2->createAlpha( corner, &south, 1, _width, _height, _width, _height );
		tmps.push_back( tmp2 );
		angles.push_back( 0.0f );
	}	
	if( !west->matches( original ) && west->matches( north ) ) {
		Actual *tmp2 = new Actual;
		tmp2->createAlpha( corner, &west, 1, _width, _height, _width, _height );
		tmps.push_back( tmp2 );
		angles.push_back( 270.0f );
	}	
	
	if( !north->matches( original ) && north->_id != INVALID ) {
		Actual *tmp2 = new Actual;
		tmp2->createAlpha( edge, &north, 1, _width, _height, _width, _height );
		tmps.push_back( tmp2 );
		angles.push_back( 270.0f );
	}
	if( !west->matches( original ) && west->_id != INVALID ) {
		Actual *tmp2 = new Actual;
		tmp2->createAlpha( edge, &west, 1, _width, _height, _width, _height );
		tmps.push_back( tmp2 );
		angles.push_back( 0.0f );
	}
	if( !east->matches( original ) && east->_id != INVALID ) {
		Actual *tmp2 = new Actual;
		tmp2->createAlpha( edge, &east, 1, _width, _height, _width, _height );
		tmps.push_back( tmp2 );
		angles.push_back( 180.0f );
	}	
	if( !south->matches( original ) && south->_id != INVALID ) {
		Actual *tmp2 = new Actual;
		tmp2->createAlpha( edge, &south, 1, _width, _height, _width, _height );
		tmps.push_back( tmp2 );
		angles.push_back( 90.0f );
	}		
	
//	// four sides match (disabled for now)
//	if( north->matches( east ) && east->matches( south ) && south->matches( west ) && west->matches( north ) ) {
////		Actual *tmp = new Actual;
////		tmp->createAlpha( hole, &north, 1, _width, _height, _width, _height );
////		tmps.push_back( tmp );
////		angles.push_back( 0.0f );
//	} else if( !west->matches( original ) && west->matches( north ) && north->matches( east ) ) {
//		Actual *tmp = new Actual;
//		tmp->createAlpha( tip, &west, 1, _width, _height, _width, _height );
//		tmps.push_back( tmp );
//		angles.push_back( 180.0f );
//		
//		if( south->_id != INVALID && south->_id != original->_id ) {
//			Actual *tmp2 = new Actual;
//			tmp2->createAlpha( edge, &south, 1, _width, _height, _width, _height );
//			tmps.push_back( tmp2 );
//			angles.push_back( 270.0f );			
//		}
//	} else if( !north->matches( original ) && north->matches( east ) && east->matches( south ) ) {
//		Actual *tmp = new Actual;
//		tmp->createAlpha( tip, &north, 1, _width, _height, _width, _height );
//		tmps.push_back( tmp );
//		angles.push_back( 90.0f );
//		
//		if( west->_id != INVALID && west->_id != original->_id ) {
//			Actual *tmp2 = new Actual;
//			tmp2->createAlpha( edge, &west, 1, _width, _height, _width, _height );
//			tmps.push_back( tmp2 );
//			angles.push_back( 0.0f );			
//		}
//	} else if( !east->matches( original ) && east->matches( south ) && south->matches( west ) ) {
//		Actual *tmp = new Actual;
//		tmp->createAlpha( tip, &east, 1, _width, _height, _width, _height );
//		tmps.push_back( tmp );
//		angles.push_back( 0.0f );
//		
//		if( north->_id != INVALID && north->_id != original->_id ) {
//			Actual *tmp2 = new Actual;
//			tmp2->createAlpha( edge, &north, 1, _width, _height, _width, _height );
//			tmps.push_back( tmp2 );
//			angles.push_back( 90.0f );			
//		}		
//	} else if( !south->matches( original ) && south->matches( west ) && west->matches( north ) ) {
//		Actual *tmp = new Actual;
//		tmp->createAlpha( tip, &south, 1, _width, _height, _width, _height );
//		tmps.push_back( tmp );
//		angles.push_back( 270.0f );
//		
//		if( east->_id != INVALID && east->_id != original->_id ) {
//			Actual *tmp2 = new Actual;
//			tmp2->createAlpha( edge, &east, 1, _width, _height, _width, _height );
//			tmps.push_back( tmp2 );
//			angles.push_back( 180.0f );
//		}		
//	}
	
//	// 2 sides match
//	if( west->matches( north ) && !north->matches( east ) && !east->matches( south ) && !south->matches( west ) ) {
//		Actual *tmp = new Actual;
//		tmp->createAlpha( corner, &west, 1, _width, _height, _width, _height );
//		tmps.push_back( tmp );
//		angles.push_back( 270.0f );
//	} else if( !west->matches( north ) && north->matches( east ) && !east->matches( south ) && !south->matches( west ) ) {
//		Actual *tmp = new Actual;
//		tmp->createAlpha( corner, &north, 1, _width, _height, _width, _height );
//		tmps.push_back( tmp );
//		angles.push_back( 180.0f );
//	} else if( !west->matches( north ) && !north->matches( east ) && east->matches( south ) && !south->matches( west ) ) {
//		Actual *tmp = new Actual;
//		tmp->createAlpha( corner, &east, 1, _width, _height, _width, _height );
//		tmps.push_back( tmp );
//		angles.push_back( 90.0f );
//	} else if( !west->matches( north ) && !north->matches( east ) && !east->matches( south ) && south->matches( west ) ) {
//		Actual *tmp = new Actual;
//		tmp->createAlpha( corner, &east, 1, _width, _height, _width, _height );
//		tmps.push_back( tmp );
//		angles.push_back( 0.0f );
//	}
	
	glDisable( GL_CULL_FACE );
	glDisable( GL_DEPTH_TEST );
	glEnable( GL_TEXTURE_2D );
	glDisable( GL_BLEND );
	
	
	glLoadIdentity();
	
	glColor4f( 1, 1, 1, 1 );
	
	// draw the original
	drawQuad( original->_id, _width, _height );
	
	
	// blend in the tmp
	glDepthMask( GL_FALSE );
	glEnable( GL_BLEND );
	glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );
	
	for( unsigned int i = 0; i < tmps.size(); i++ ) {
		glPushMatrix();
		glLoadIdentity();
		glTranslatef( _width / 2, _height / 2, 0 );
		glRotatef( angles[i], 0, 0, 1 );
		glTranslatef( -_width / 2, -_height / 2, 0 );
		drawQuad( tmps[i]->_id, _width, _height );
		glPopMatrix();
	}
	
	glDepthMask( GL_TRUE );
	glDisable( GL_BLEND );
	
	// Copy to a texture
	glLoadIdentity();
	glEnable( GL_TEXTURE_2D );
	glBindTexture( GL_TEXTURE_2D, _id );
	glCopyTexSubImage2D( GL_TEXTURE_2D,
	                     0,      // MIPMAP level
	                     0,      // x texture offset
	                     0,      // y texture offset
	                     0,              // x window coordinates
	                     Session::instance->getGameAdapter()->getScreenHeight() - textureSizeH,   // y window coordinates
	                     textureSizeW,    // width
	                     textureSizeH     // height
	                   );
	
	// delete the tmps
	for( unsigned int i = 0; i < tmps.size(); i++ ) {
		delete tmps[i];
	}

	// cover with the original
	drawQuad( background, _width, _height );
	glPopMatrix();
	glDeleteTextures( 1, &background );

	glDisable( GL_BLEND );
	glEnable( GL_CULL_FACE );
	glEnable( GL_DEPTH_TEST );

	assert( _id != INVALID && _id != INPROGRESS );  
	_isComplete = true;
	return true;

}

void Texture::Actual::drawQuad( GLuint id, int width, int height ) {
	glBindTexture( GL_TEXTURE_2D, id );
	glBegin( GL_TRIANGLE_STRIP );
	glTexCoord2i( 0, 0 );
	glVertex2i( 0, 0 );
	glTexCoord2i( 1, 0 );
	glVertex2i( width, 0 );
	glTexCoord2i( 0, 1 );
	glVertex2i( 0, height );
	glTexCoord2i( 1, 1 );
	glVertex2i( width, height );
	glEnd();	
}

/// Adds the alpha channel of the alpha texture to the sample texture.
/// @return true on success.
/// used to be Shapes::createAlphaTexture()

bool Texture::Actual::createAlpha( Actual* alpha, Actual* sample[], int sampleCount, int textureSizeW, int textureSizeH, int width, int height ) {
	// debug checks
	if ( _filename.compare( "" ) != 0 ) {
		cerr << "*** Texture::Actual::createAlpha() over (" << _filename << ") refused." << endl;
		return false;
	}
	if ( alpha == NULL || !alpha->letsToBind() ) {
		cerr << "*** Texture::Actual::createAlpha() with missing alpha texture." << endl;
		return false;
	}
	for( int i = 0; i < sampleCount; i++ ) {
		if ( sample[i] == NULL || !sample[i]->letsToBind() ) {
			cerr << "*** Texture::Actual::createAlpha() with missing sample texture. i=" << i << endl;
			return false;
		}
	}
	
	Preferences *prefs = Session::instance->getPreferences();
	
	GLuint background = saveAreaUnder( 0, 0, textureSizeW, textureSizeH );

	cerr << "*** samples=" << sampleCount << endl;
	_filename = "";
	for( int i = 0; i < sampleCount; i++ ) {
		if( i > 0 ) _filename += ",";
		_filename += sample[i]->_filename;
	}
	_filename += " with added alpha";
	cerr << "creating alpha: " << _filename << endl;
	
	_width = width; 
	_height = height;
	_hasAlpha = true;

	std::vector<unsigned char*> texInMem( textureSizeW * textureSizeH * 4 );
	//GLuint tex[1];

	glDisable( GL_CULL_FACE );
	glDisable( GL_DEPTH_TEST );
	glEnable( GL_TEXTURE_2D );

	glGenTextures( 1, &_id );
	glBindTexture( GL_TEXTURE_2D, _id );
	glPixelStorei( GL_UNPACK_ALIGNMENT, 1 );

	/**
	 * This method should not create mip-maps. They don't work well with alpha-tested textures and cause flickering.
	 */
	//glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, isSprite ? GL_NEAREST : GL_LINEAR_MIPMAP_NEAREST );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP );
	glTexImage2D( GL_TEXTURE_2D, 0, ( prefs->getBpp() > 16 ? GL_RGBA : GL_RGBA4 ), textureSizeW, textureSizeH, 0, GL_RGBA, GL_UNSIGNED_BYTE, &texInMem[0] );
	//if( !isSprite ) gluBuild2DMipmaps(GL_TEXTURE_2D, 4, textureSizeW, textureSizeH, GL_BGRA, GL_UNSIGNED_BYTE, texInMem);

	glPushMatrix();
	glLoadIdentity();

	glDisable( GL_TEXTURE_2D );
	glColor4f( 0, 0, 0, 0 );

	glBegin( GL_TRIANGLE_STRIP );
	glVertex2i( 0, 0 );
	glVertex2i( textureSizeW, 0 );
	glVertex2i( 0, textureSizeH );
	glVertex2i( textureSizeW, textureSizeH );
	glEnd();

	glColor4f( 1, 1, 1, 1 );

	// draw the grass
	glEnable( GL_TEXTURE_2D );

	for( int i = 0; i < sampleCount; i++ ) {
		glColor4f( 1, 1, 1, 1 );
		drawQuad( sample[i]->_id, width, height );
	}

	// draw the alpha pixels only
	glColorMask( GL_FALSE, GL_FALSE, GL_FALSE, GL_TRUE );
	glColor4f( 1, 1, 1, 1 );
	drawQuad( alpha->_id, width, height );
	glColorMask( GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE );

	// Copy to a texture
	glLoadIdentity();
	glEnable( GL_TEXTURE_2D );
	glBindTexture( GL_TEXTURE_2D, _id );
	glCopyTexSubImage2D( GL_TEXTURE_2D,
	                     0,      // MIPMAP level
	                     0,      // x texture offset
	                     0,      // y texture offset
	                     0,              // x window coordinates
	                     Session::instance->getGameAdapter()->getScreenHeight() - textureSizeH,   // y window coordinates
	                     textureSizeW,    // width
	                     textureSizeH     // height
	                   );

	// cover with the original
	drawQuad( background, width, height );
	glPopMatrix();
	glDeleteTextures( 1, &background );

	glDisable( GL_BLEND );
	glEnable( GL_CULL_FACE );
	glEnable( GL_DEPTH_TEST );

	assert( _id != INVALID && _id != INPROGRESS );  
	_isComplete = true;
	return true;
}

bool Texture::Actual::loadShot( const string& dirName ) {
	// debug checks
	if ( _filename.compare( "" ) != 0 ) {
		cerr << "*** Texture::Actual::loadShot() over (" << _filename << ") refused." << endl;
		return false;
	}

	_surface = SDL_LoadBMP( dirName.c_str() );

	if ( _surface == NULL ) {
		cerr << "*** Texture::Actual::loadShot() Error (" << dirName << "): " << IMG_GetError() << endl;
		return false;
	}

	_filename = dirName;
	_width = _surface->w;
	_height = _surface->h;

	glPixelStorei( GL_UNPACK_ALIGNMENT, 4 );
	glGenTextures( 1, &_id );
	glBindTexture( GL_TEXTURE_2D, _id );

	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );

	gluBuild2DMipmaps( GL_TEXTURE_2D, GL_RGB, _width, _height, GL_BGR, GL_UNSIGNED_BYTE, _surface->pixels );

	unloadImage();

	assert( _id != INVALID && _id != INPROGRESS );  
	_isComplete = true;
	return true;
}

GLint Texture::Actual::width()
{
	if ( _width == -1 && !loadImage() ) {
		clear();
	}
	
	return _width;
}

GLint Texture::Actual::height()
{
	if ( _height == -1 && !loadImage() ) {
		clear();
	}
	
	return _height;
}


/// Loads image using SDL_image and extracts all interesting data from it
/// assumes _filename is set
/// @return true on success

bool Texture::Actual::loadImage() {
	// debug checks
	assert( _surface == NULL && _width == -1 && _height == -1 && _filename.compare( "" ) != 0  );

	_surface = IMG_Load( _filename.c_str() );
	if ( _surface == NULL ) {
		cerr << "*** Texture::Actual::loadImage() error (" << _filename << "): " << IMG_GetError() << endl;
		return false; // caller must clear up
	}

	_width = _surface->w;
	_height = _surface->h;
	_hasAlpha = _surface->format->Amask != 0;

	// TODO: refactor Constants::checkTexture into Texture::check
	Constants::checkTexture( "Texture::Actual::loadImage", _width, _height );
	return true;
}


bool Texture::Actual::letsToBind() {
	if ( _isComplete ) return true;
	if ( _id == INVALID ) return false;

	if ( _width == -1 && !loadImage() ) {
		clear();
		return false;
	}

	assert( _surface != NULL );

	Preferences *prefs = Session::instance->getPreferences();

	GLuint destFormat;
	GLuint srcFormat;
	GLuint minFilter;
	GLuint magFilter;

	if ( _hasAlpha ) {
		srcFormat = GL_RGBA;
		destFormat = ( prefs->getBpp() > 16 ? GL_RGBA : GL_RGBA4 );
		minFilter = ( _isSprite ? GL_LINEAR : ( prefs->getBpp() > 16 ? GL_LINEAR_MIPMAP_LINEAR : GL_NEAREST_MIPMAP_NEAREST ) );
		magFilter = GL_LINEAR;
	} else {
		srcFormat = GL_RGB;
		destFormat = ( prefs->getBpp() > 16 ? GL_RGB : GL_RGB5 );
		minFilter = GL_LINEAR_MIPMAP_LINEAR;
		magFilter = prefs->getBpp() > 16 ? GL_LINEAR : GL_NEAREST;
	}

	glPixelStorei( GL_UNPACK_ALIGNMENT, 4 );
	glGenTextures( 1, &_id );
	glBindTexture( GL_TEXTURE_2D, _id );

	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, minFilter );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, magFilter );

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

	if ( _priority > 0 ) glPrioritizeTextures( 1, &_id, &_priority );

	assert( _id != INVALID && _id != INPROGRESS );  
	_isComplete = true;
	return true;
}

void Texture::Actual::prioritize( GLclampf pri ) {
	// debug checks
	if ( _id == INVALID ) {
		cerr << "*** Texture::Actual::prioritize() unexisting texture." << endl;
		return;
	}
	if ( pri <= 0 ) {
		cerr << "*** Texture::Actual::prioritize() negative priority." << endl;
		return;
	}

	_priority = pri;

	if ( _isComplete )  glPrioritizeTextures( 1, &_id, &_priority );
}

/// Texture construction

Texture::Texture()
		: _ref( &emptyNode ), group_name("") {
	++( _ref->_cntr );
}

Texture::Texture( Texture const& that )
		: _ref( that._ref ), group_name("") {
	++( _ref->_cntr );
}

Texture::Texture( Actual* node )
		: _ref( node ), group_name( "" ) {
	++( _ref->_cntr );
}

Texture::~Texture() {
	--( _ref->_cntr );
	if ( _ref->_cntr == 0 && _ref != &emptyNode ) {
		delete _ref;
	}
}

Texture& Texture::operator=( Texture const & that ) {
	Texture tmp( that );
	swap( tmp );
	return *this;
}


void Texture::clear() {
	if ( _ref != &emptyNode ) { // if not already clear
		Texture tmp( &emptyNode );
		swap( tmp );
	}
}


void Texture::swap( Texture& that ) {
	Actual* tmp( that._ref );
	that._ref = _ref;
	_ref = tmp;
}


/// search for named texture
/// @return  iterator to texture when found
/// @return  iterator where to insert it when not found
Texture::NodeVec::iterator Texture::search( const string& path ) {
	// search quickly assuming the mainList is sorted
	int before = -1; 
	int after = mainList.size(); 
	for ( ;; ) {
		// middle is between after and before
		int middle = ( before + after ) / 2; 
		// when stuck tell to insert after
		if ( middle == before || middle == after ) return mainList.begin() + after; 
		// evaluate middle
		int diff = path.compare( mainList[middle]->_filename );
		// get closer if not found
		if ( diff > 0 ) before = middle;
		else if ( diff < 0 ) after = middle;
		else return mainList.begin() + middle; // found
	}
}


/// Grand unified generic texture loader.

/// Creates an OpenGL texture from a file and tries to set up everything
/// correctly according to the properties of the source image.
/// @return true on success.
/// original was Shapes::loadTexture()

bool Texture::load( const string& filename, bool absolutePath, bool isSprite, bool anisotropy ) {
	// search for existing with the path
	std::string path = ( absolutePath ? filename : rootDir + filename );
	NodeVec::iterator it = search( path );
	// not found?
	if ( it == mainList.end() || path.compare( ( *it )->_filename ) != 0 ) {
		Actual* node = new Actual;
		// not loadable? refuse
		if ( !node->load( path, isSprite, anisotropy ) ) {
			delete node;
			return false;
		}
		// create
		it = mainList.insert( it, node );
	}
	// found it or created it ... now swap it out and done
	Texture tmp( *it );
	swap( tmp );
	return isSpecified();
}

/// Creates a tile of an SDL surface as a texture.
/// This function allows you to define the size of a tile and which tile you want. It
/// is then created as a texture from the appropriate part of the SDL surface.
/// @return true on success.
/// used to be Shapes::createTileTexture()

bool Texture::createTile( SDL_Surface const* surface, int tileX, int tileY, int tileWidth, int tileHeight ) {
	// this one adds new node always
	Actual* node = new Actual;
	node->createTile( surface, tileX, tileY, tileWidth, tileHeight );
	Texture tmp( node );
	swap( tmp );
	return isSpecified();
}

/// Adds the alpha channel of the alphaTex texture to the sampleTex texture.
/// @return true on success.
/// used to be Shapes::createAlphaTexture()

bool Texture::createAlpha( Texture const& alpha, Texture const& sample, int textureSizeW, int textureSizeH, int width, int height ) {
	// this one adds new node always
	Actual* node = new Actual;
	Actual* refs[1];
	refs[0] = sample._ref;
	node->createAlpha( alpha._ref, refs, 1, textureSizeW, textureSizeH, width, height );
	Texture tmp( node );
	swap( tmp );
	return isSpecified();
}

bool Texture::createAlphaQuad( Texture const& alpha, Texture samples[], int textureSizeW, int textureSizeH, int width, int height ) {
	// this one adds new node always
	Actual* node = new Actual;
	Actual* refs[4];
	for( int i = 0; i < 4; i++ ) {
		refs[i] = samples[i]._ref;
	}
	node->createAlpha( alpha._ref, refs, 4, textureSizeW, textureSizeH, width, height );
	Texture tmp( node );
	swap( tmp );
	return isSpecified();
}

/// loads texture from screen.bmp file in given directory
/// @return true on success.
/// used to be SavegameDialog::loadScreenshot()

bool Texture::loadShot( const string& dirName ) {
	std::string path = get_file_name( dirName + "/screen.bmp" );
	// search if there are textures with same name
	NodeVec::iterator it = search( path );
	// not found?
	if ( it == mainList.end() || path.compare( ( *it )->_filename ) != 0 ) {
		Actual* node = new Actual;
		// not loadable? refuse
		if ( !node->loadShot( path ) ) {
			delete node;
			return false;
		}
		// create
		it = mainList.insert( it, node );
	}
	// found it or created it ... now swap to it
	Texture tmp( *it );
	swap( tmp );
	return isSpecified();
}

GLuint Texture::saveAreaUnder( int x, int y, int w, int h, GLuint *tex ) {
	// Copy to a texture the original image
	glLoadIdentity();
	glEnable( GL_TEXTURE_2D );
	GLuint background;
	if( !tex || *tex == 0 ) {
		glGenTextures( 1, &background );
	} else {
		background = *tex;
	}
	std::vector<unsigned char*> backgroundInMem( w * h * 4 );
	glBindTexture( GL_TEXTURE_2D, background );
	glPixelStorei( GL_UNPACK_ALIGNMENT, 1 );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP );
	Preferences *prefs = Session::instance->getPreferences();
	glTexImage2D( GL_TEXTURE_2D, 0, ( prefs->getBpp() > 16 ? GL_RGBA : GL_RGBA4 ), w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, &backgroundInMem[0] );
	glBindTexture( GL_TEXTURE_2D, background );
	glCopyTexSubImage2D( GL_TEXTURE_2D,
	                     0,      // MIPMAP level
	                     0,      // x texture offset
	                     0,      // y texture offset
	                     0,              // x window coordinates
	                     Session::instance->getGameAdapter()->getScreenHeight() - h,   // y window coordinates
	                     w,    // width
	                     h     // height
	                   );
	return background;
}

bool Texture::createEdgeBlend( Texture const& original, Texture const& west, Texture const& east, Texture const& south, Texture const& north,
                               Texture const& edge, Texture const& corner, Texture const& tip, Texture const& hole ) {
	// search for existing with the path
	std::string path = original.group_name + "_" + 
		west.group_name + "_" + 
		east.group_name + "_" + 
		south.group_name + "_" + 
		north.group_name;
	cerr << "+++ looking for blend: " << path << endl;
	NodeVec::iterator it = search( path );
	// not found?
	if ( it == mainList.end() || path.compare( ( *it )->_filename ) != 0 ) {
		cerr << "\t+++ creating it" << endl;
		Actual* node = new Actual;
		// not loadable? refuse
		if ( !node->createEdgeBlended( path, original._ref, west._ref, east._ref, south._ref, north._ref,
		                               edge._ref, corner._ref, tip._ref, hole._ref ) ) {
			cerr << "\t\t+++ failed" << endl;
			delete node;
			return false;
		} else {
			cerr << "\t\t+++ success: id=" << node->_id << endl;
		}
		// create
		it = mainList.insert( it, node );
	} else {
		cerr << "\t+++ found it" << endl;
	}
	// found it or created it ... now swap it out and done
	Texture tmp( *it );
	swap( tmp );
	return isSpecified();	
}
