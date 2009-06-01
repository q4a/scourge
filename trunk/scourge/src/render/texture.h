/***************************************************************************
                     texture.h  -  Class for 2D OpenGL textures
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
		INPROGRESS = ~0,
	};
	
	// construction / destruction / copying
	Texture();
	Texture( Texture const& that );
	~Texture();
	Texture& operator=( Texture const& that );

	// comparision
	bool operator == ( Texture const& that ) const {
		return _ref == that._ref;
	}

	bool operator != ( Texture const& that ) const {
		return !(*this == that);
	}
	// '<' does not make sense but is needed for sorted std containers
	// TODO: get rid of any external sorted containers 
	bool operator < ( Texture const& that ) const { 
		return _ref < that._ref;
	}

	// loading / unloading
	bool load( const string& filename, bool absolutePath = false, bool isSprite = true, bool anisotropy = false );
	bool createAlpha(  Texture const& alpha, Texture const& sample, int textureSizeW = 256, int textureSizeH = 256, int width = 256, int height = 256 );
	bool createAlphaQuad(  Texture const& alpha, Texture samples[], int textureSizeW = 256, int textureSizeH = 256, int width = 256, int height = 256 );
	bool createTile( SDL_Surface const* surface, int tileX, int tileY, int tileWidth, int tileHeight );
	bool loadShot( const string& dirName );
	void clear();
	bool createEdgeBlend( Texture const& original, Texture const& west, Texture const& east, Texture const& south, Texture const& north,
	                      Texture const& edge, Texture const& corner, Texture const& tip, Texture const& hole );

	// OpenGL operations
	void glBind() const {
		if ( _ref->letsToBind() )	glBindTexture( GL_TEXTURE_2D, _ref->_id );
	}

	void glPrioritize( GLclampf pri ) const {
		_ref->prioritize( pri ); 
	}

	// Stop the lazyness (some textures do not show up)
	void goDiligent() const {
		_ref->letsToBind(); 
	}

	// getters
	bool isSpecified() const {
		return _ref->_id != INVALID;
	}

	GLint width() const {
		return _ref->width();
	}

	GLint height() const {
		return _ref->height();
	}

	static Texture const& none() {
		return empty;
	}
	
	void setGroupName( std::string s ) {
		_ref->_group_name = s;
	}
	
	std::string getGroupName() {
		return _ref->_group_name;
	}
	
	static GLuint saveAreaUnder( int x, int y, int w, int h, GLuint *tex = NULL );

private:
	// all member data is in refcounted Actual
	class Actual {
	public:
		GLuint _id; // OpenGL texture name
		int _cntr; // Intrusive counter
		string _filename;
		GLint _width;
		GLint _height;
		bool _hasAlpha;
		bool _isSprite;
		GLclampf _priority;
		bool _isComplete;
		bool _wantsAnisotropy;
		// TODO: think through member/polymorphing candidates here:
		// bool _wantsMipmapping;
		// bool _isMipmapped;
		// unsigned char* _pixels;
		SDL_Surface* _surface;
		std::string _group_name;

		// construction / destruction
		Actual();
		~Actual();
		// members
		bool load( const string& path, bool isSprite, bool anisotropy );
		bool createEdgeBlended( const string& path, Actual* original, Actual* west, Actual* east, Actual* south, Actual* north,
		                        Actual *edge, Actual *corner, Actual *tip, Actual *hole );
		bool createTile( SDL_Surface const* surface, int tileX, int tileY, int tileWidth, int tileHeight );
		bool createAlpha( Actual* alpha, Actual* sample[], int sampleCount, int textureSizeW, int textureSizeH, int width, int height );
		bool loadShot( const string& dirName );
		GLint width();
		GLint height();
		bool letsToBind();
		void prioritize( GLclampf pri );
	private:
		void clear(); 
		bool loadImage();
		void unloadImage();
		// undefined default copying 
		Actual( Actual const& that ); // copy construction
		Actual& operator=( Actual const& that ); // copy assignment
		void drawQuad( GLuint id, int width, int height );
		bool matches( Actual *other );
		DECLARE_NOISY_OPENGL_SUPPORT();
	};

	Actual* _ref; // should never be NULL

	// some static things to speed and help
	static Actual emptyNode; // Actual texture that is always empty. For speeding up construction and clearing.
	static Texture empty; // Texture that is always empty. To avoid constructing empty textures.
	typedef std::vector<Actual*> NodeVec;  
	static NodeVec mainList; // List of named textures. To avoid reloading already loaded textures.
	// private constructor
	Texture( Actual* node );
	// private operations
	void swap( Texture& that );
	static NodeVec::iterator search( const string& path );
	DECLARE_NOISY_OPENGL_SUPPORT();
};



#endif
