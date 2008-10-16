/***************************************************************************
                     texture.h  -  Class for 2D textures
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
#ifndef TEXTURE_H
#define TEXTURE_H
#pragma once

using std::string;

class Texture {
public:
	enum {
		INVALID = 0,
	};

	// construction / destruction
	Texture();
	~Texture();

	// loading / unloading
	bool load( const string& filename, bool absolutePath = false, bool isSprite = true, bool anisotropy = false );
	bool createAlpha(  Texture const* alpha, Texture const* sample, int textureSizeW = 256, int textureSizeH = 256, int width = 256, int height = 256 );
	bool createTile( SDL_Surface **surface, int tileX, int tileY, int tileWidth, int tileHeight );
	bool loadShot( const string& dirName );
	void clear();

	// OpenGL operations
	void glBind() const {/*assert(isSpecified());*/
		glBindTexture( GL_TEXTURE_2D, _id );
	}
	void glPrioritize( GLclampf pri ) const {
		assert( isSpecified() ); glPrioritizeTextures( 1, &_id, &pri );
	}

	// getters
	bool isSpecified() const {
		return _id != INVALID;
	}
	GLint width() const {
		return _width;
	}
	GLint height() const {
		return _height;
	}

private:
	// member data
	GLuint _id; //OpenGL texture name
	string _filename;
	GLint _width;
	GLint _height;
	// unused: bool _hasAbsolutePath;
	bool _hasAlpha;
	bool _isSprite;
	bool _wantsAnisotropy;
	// Debug member to check we do not destroy textures multiple times (UDB).
	bool _isDestoyed;
	// TODO: think through member/polymorphing candidates here:
	// bool _wantsMipmapping;
	// bool _isMipmapped;
	// unsigned char* _pixels;
	SDL_Surface* _surface;

	// undefine default copying since this class "owns" GPU resources
	Texture( Texture const& that ); // copy construction
	Texture& operator=( Texture const& that ); // copy assignment

	// private member functions
	bool loadImage();
	void unloadImage();


};



#endif
