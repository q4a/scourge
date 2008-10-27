/***************************************************************************
               glshape.cpp  -  Class representing any 3D shape
                             -------------------
    begin                : Thu Jul 10 2003
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
#include "glshape.h"
#include "shapes.h"
#include "../util.h"
#include "virtualshape.h"

using namespace std;

class Map;

// poor man's dynamic lightmaps: shaded sides
static GLuint lightmap_tex_num = 0;
static GLuint lightmap_tex_num2 = 0;
static unsigned char data[LIGHTMAP_SIZE * LIGHTMAP_SIZE * 3];
static unsigned char data2[LIGHTMAP_SIZE * LIGHTMAP_SIZE * 3];

GLShape::GLShape( Texture tex[], int width, int depth, int height, char const* name, int descriptionGroup,
                  Uint32 color, Uint8 shapePalIndex )
		: Shape( width, depth, height, name, descriptionGroup ) {
	commonInit( tex, color, shapePalIndex );
}

void GLShape::commonInit( Texture tex[], Uint32 color, Uint8 shapePalIndex ) {
	this->alpha = 1.0f;
	this->tex = tex;
	this->color = color;
	this->shapePalIndex = shapePalIndex;
	this->skipside = 0;
	this->useShadow = false;
	this->useTexture = true;
	this->lightBlocking = false;
	this->initialized = false;
	this->effectType = -1;
	this->wallShape = false;
	this->iconRotX = this->iconRotY = this->iconRotZ = 0;
	this->icon.clear();
	this->iconWidth = this->iconHeight = 0;
	this->ambient = "";
	this->ignoreHeightMap = false;

	this->occurs.rooms_only = false;
	this->occurs.max_count = 0;
	strcpy( this->occurs.placement, "center" );
	strcpy( this->occurs.use_function, "" );
	strcpy( this->occurs.theme, "" );

	surfaces[LEFT_SURFACE] = NULL;
	surfaces[BOTTOM_SURFACE] = NULL;
	surfaces[RIGHT_SURFACE] = NULL;
	surfaces[FRONT_SURFACE] = NULL;
	surfaces[TOP_SURFACE] = NULL;
	initSurfaces();

	initialize();
}

void GLShape::setTexture( Texture* textureGroup ) {
	this->tex = textureGroup;
	setTextureIndex( -1 );
}

void GLShape::initialize() {
	//cerr << "multitexture=" << Constants::multitexture << " lightmap1=" << lightmap_tex_num << " lightmap2=" << lightmap_tex_num2 << endl;

	displayListStart = glGenLists( 8 );
	if ( !displayListStart ) {
		cerr << "*** Error: couldn't generate display lists for shape: " << getName() << endl;
		exit( 1 );
	}

	createShadowList( displayListStart );
	createTopList( displayListStart + 1 );
	for ( int i = 0; i < 6; i++ ) {
		createBodyList( i, displayListStart + 2 + i );
	}

	initialized = true;
}


void GLShape::createShadowList( GLuint listName ) {
	glNewList( listName, GL_COMPILE );

	// left
	if ( !( skipside & ( 1 << GLShape::LEFT_RIGHT_SIDE ) ) ) {
		//glNormal3f(-1.0f, 0.0f, 0.0f);
		glBegin( GL_TRIANGLE_STRIP );
		glVertex3fv( surfaces[LEFT_SURFACE]->vertices[1] );
		glVertex3fv( surfaces[LEFT_SURFACE]->vertices[2] );
		glVertex3fv( surfaces[LEFT_SURFACE]->vertices[0] );
		glVertex3fv( surfaces[LEFT_SURFACE]->vertices[3] );
		glEnd();
	}

	// bottom
	if ( !( skipside & ( 1 << GLShape::FRONT_SIDE ) ) ) {
		//glNormal3f(0.0f, -1.0f, 0.0f);
		glBegin( GL_TRIANGLE_STRIP );
		glVertex3fv( surfaces[BOTTOM_SURFACE]->vertices[1] );
		glVertex3fv( surfaces[BOTTOM_SURFACE]->vertices[2] );
		glVertex3fv( surfaces[BOTTOM_SURFACE]->vertices[0] );
		glVertex3fv( surfaces[BOTTOM_SURFACE]->vertices[3] );
		glEnd();
	}

	// right
	if ( !( skipside & ( 1 << GLShape::LEFT_RIGHT_SIDE ) ) ) {
		//glNormal3f(1.0f, 0.0f, 0.0f);
		glBegin( GL_TRIANGLE_STRIP );
		glVertex3fv( surfaces[RIGHT_SURFACE]->vertices[1] );
		glVertex3fv( surfaces[RIGHT_SURFACE]->vertices[2] );
		glVertex3fv( surfaces[RIGHT_SURFACE]->vertices[0] );
		glVertex3fv( surfaces[RIGHT_SURFACE]->vertices[3] );
		glEnd( );
	}

	// front
	if ( !( skipside & ( 1 << GLShape::FRONT_SIDE ) ) ) {
		//glNormal3f(0.0f, 1.0f, 0.0f);
		glBegin( GL_TRIANGLE_STRIP );
		glVertex3fv( surfaces[FRONT_SURFACE]->vertices[1] );
		glVertex3fv( surfaces[FRONT_SURFACE]->vertices[2] );
		glVertex3fv( surfaces[FRONT_SURFACE]->vertices[0] );
		glVertex3fv( surfaces[FRONT_SURFACE]->vertices[3] );
		glEnd();
	}

	// top
	//glNormal3f(0.0f, 0.0f, 1.0f);
	glBegin( GL_TRIANGLE_STRIP );
	glVertex3fv( surfaces[TOP_SURFACE]->vertices[1] );
	glVertex3fv( surfaces[TOP_SURFACE]->vertices[2] );
	glVertex3fv( surfaces[TOP_SURFACE]->vertices[0] );
	glVertex3fv( surfaces[TOP_SURFACE]->vertices[3] );
	glEnd();

	glEndList();
}

void GLShape::createBodyList( int side, GLuint listName ) {
	glNewList( listName, GL_COMPILE );

	// --------------------------------------------
	if ( side == Shape::W_SIDE ) {
		//glNormal3f(-1.0f, 0.0f, 0.0f);
		glBegin( GL_TRIANGLE_STRIP );
		glTexCoord2f( 0.0f, 0.0f );
		glVertex3fv( surfaces[LEFT_SURFACE]->vertices[1] );
		glTexCoord2f( 1.0f, 0.0f );
		glVertex3fv( surfaces[LEFT_SURFACE]->vertices[2] );
		glTexCoord2f( 0.0f, 1.0f );
		glVertex3fv( surfaces[LEFT_SURFACE]->vertices[0] );
		glTexCoord2f( 1.0f, 1.0f );
		glVertex3fv( surfaces[LEFT_SURFACE]->vertices[3] );
		glEnd();
	}

	// --------------------------------------------
	if ( side == Shape::N_SIDE ) {
		// bottom
		//glNormal3f(0.0f, -1.0f, 0.0f);
		glBegin( GL_TRIANGLE_STRIP );
		if ( Constants::multitexture ) {
			glSDLMultiTexCoord2fARB( GL_TEXTURE0_ARB, 0.0f, 0.0f );
			glSDLMultiTexCoord2fARB( GL_TEXTURE1_ARB, 0.0f, 0.0f );
			glVertex3fv( surfaces[BOTTOM_SURFACE]->vertices[1] );
			glSDLMultiTexCoord2fARB( GL_TEXTURE0_ARB, 1.0f, 0.0f );
			glSDLMultiTexCoord2fARB( GL_TEXTURE1_ARB, 1.0f, 0.0f );
			glVertex3fv( surfaces[BOTTOM_SURFACE]->vertices[2] );
			glSDLMultiTexCoord2fARB( GL_TEXTURE0_ARB, 0.0f, 1.0f );
			glSDLMultiTexCoord2fARB( GL_TEXTURE1_ARB, 0.0f, 1.0f );
			glVertex3fv( surfaces[BOTTOM_SURFACE]->vertices[0] );
			glSDLMultiTexCoord2fARB( GL_TEXTURE0_ARB, 1.0f, 1.0f );
			glSDLMultiTexCoord2fARB( GL_TEXTURE1_ARB, 1.0f, 1.0f );
			glVertex3fv( surfaces[BOTTOM_SURFACE]->vertices[3] );
		} else {
			glTexCoord2f( 0.0f, 0.0f );
			glVertex3fv( surfaces[BOTTOM_SURFACE]->vertices[1] );
			glTexCoord2f( 1.0f, 0.0f );
			glVertex3fv( surfaces[BOTTOM_SURFACE]->vertices[2] );
			glTexCoord2f( 0.0f, 1.0f );
			glVertex3fv( surfaces[BOTTOM_SURFACE]->vertices[0] );
			glTexCoord2f( 1.0f, 1.0f );
			glVertex3fv( surfaces[BOTTOM_SURFACE]->vertices[3] );
		}
		glEnd();
		if ( Constants::multitexture ) {
			glSDLActiveTextureARB( GL_TEXTURE1_ARB );
			glEnable( GL_TEXTURE_2D );
			glBindTexture( GL_TEXTURE_2D, 0 );
			glSDLActiveTextureARB( GL_TEXTURE0_ARB );
			glEnable( GL_TEXTURE_2D );
		}
	}




	// --------------------------------------------
	if ( side == Shape::E_SIDE ) {
		// right
		//glNormal3f(1.0f, 0.0f, 0.0f);
		glBegin( GL_TRIANGLE_STRIP );
		if ( Constants::multitexture ) {
			glSDLMultiTexCoord2fARB( GL_TEXTURE0_ARB, 0.0f, 0.0f );
			glSDLMultiTexCoord2fARB( GL_TEXTURE1_ARB, 0.0f, 0.0f );
			glVertex3fv( surfaces[RIGHT_SURFACE]->vertices[1] );
			glSDLMultiTexCoord2fARB( GL_TEXTURE0_ARB, 1.0f, 0.0f );
			glSDLMultiTexCoord2fARB( GL_TEXTURE1_ARB, 1.0f, 0.0f );
			glVertex3fv( surfaces[RIGHT_SURFACE]->vertices[2] );
			glSDLMultiTexCoord2fARB( GL_TEXTURE0_ARB, 0.0f, 1.0f );
			glSDLMultiTexCoord2fARB( GL_TEXTURE1_ARB, 0.0f, 1.0f );
			glVertex3fv( surfaces[RIGHT_SURFACE]->vertices[0] );
			glSDLMultiTexCoord2fARB( GL_TEXTURE0_ARB, 1.0f, 1.0f );
			glSDLMultiTexCoord2fARB( GL_TEXTURE1_ARB, 1.0f, 1.0f );
			glVertex3fv( surfaces[RIGHT_SURFACE]->vertices[3] );
		} else {
			glTexCoord2f( 0.0f, 0.0f );
			glVertex3fv( surfaces[RIGHT_SURFACE]->vertices[1] );
			glTexCoord2f( 1.0f, 0.0f );
			glVertex3fv( surfaces[RIGHT_SURFACE]->vertices[2] );
			glTexCoord2f( 0.0f, 1.0f );
			glVertex3fv( surfaces[RIGHT_SURFACE]->vertices[0] );
			glTexCoord2f( 1.0f, 1.0f );
			glVertex3fv( surfaces[RIGHT_SURFACE]->vertices[3] );
		}
		glEnd( );
		if ( Constants::multitexture ) {
			glSDLActiveTextureARB( GL_TEXTURE1_ARB );
			glEnable( GL_TEXTURE_2D );
			glBindTexture( GL_TEXTURE_2D, 0 );
			glSDLActiveTextureARB( GL_TEXTURE0_ARB );
			glEnable( GL_TEXTURE_2D );
		}
	}



	// --------------------------------------------
	if ( side == Shape::S_SIDE ) {
		// front
		//glNormal3f(0.0f, 1.0f, 0.0f);
		glBegin( GL_TRIANGLE_STRIP );
		if ( Constants::multitexture ) {
			glSDLMultiTexCoord2fARB( GL_TEXTURE0_ARB, 0.0f, 0.0f );
			glSDLMultiTexCoord2fARB( GL_TEXTURE1_ARB, 0.0f, 0.0f );
			glVertex3fv( surfaces[FRONT_SURFACE]->vertices[1] );
			glSDLMultiTexCoord2fARB( GL_TEXTURE0_ARB, 1.0f, 0.0f );
			glSDLMultiTexCoord2fARB( GL_TEXTURE1_ARB, 1.0f, 0.0f );
			glVertex3fv( surfaces[FRONT_SURFACE]->vertices[2] );
			glSDLMultiTexCoord2fARB( GL_TEXTURE0_ARB, 0.0f, 1.0f );
			glSDLMultiTexCoord2fARB( GL_TEXTURE1_ARB, 0.0f, 1.0f );
			glVertex3fv( surfaces[FRONT_SURFACE]->vertices[0] );
			glSDLMultiTexCoord2fARB( GL_TEXTURE0_ARB, 1.0f, 1.0f );
			glSDLMultiTexCoord2fARB( GL_TEXTURE1_ARB, 1.0f, 1.0f );
			glVertex3fv( surfaces[FRONT_SURFACE]->vertices[3] );
		} else {
			glTexCoord2f( 0.0f, 0.0f );
			glVertex3fv( surfaces[FRONT_SURFACE]->vertices[1] );
			glTexCoord2f( 1.0f, 0.0f );
			glVertex3fv( surfaces[FRONT_SURFACE]->vertices[2] );
			glTexCoord2f( 0.0f, 1.0f );
			glVertex3fv( surfaces[FRONT_SURFACE]->vertices[0] );
			glTexCoord2f( 1.0f, 1.0f );
			glVertex3fv( surfaces[FRONT_SURFACE]->vertices[3] );
		}
		glEnd();

		if ( Constants::multitexture ) {
			glSDLActiveTextureARB( GL_TEXTURE1_ARB );
			glEnable( GL_TEXTURE_2D );
			glBindTexture( GL_TEXTURE_2D, 0 );
			glSDLActiveTextureARB( GL_TEXTURE0_ARB );
			glEnable( GL_TEXTURE_2D );
		}
	}
	glEndList();
}

void GLShape::createTopList( GLuint listName ) {
	glNewList( listName, GL_COMPILE );

	//glNormal3f(0.0f, 0.0f, 1.0f);
	glBegin( GL_TRIANGLE_STRIP );

	if ( getWidth() > getHeight() ) {
		glTexCoord2f( 0.0f, 0.0f );
		glVertex3fv( surfaces[TOP_SURFACE]->vertices[1] );
		glTexCoord2f( 1.0f, 0.0f );
		glVertex3fv( surfaces[TOP_SURFACE]->vertices[2] );
		glTexCoord2f( 0.0f, 1.0f );
		glVertex3fv( surfaces[TOP_SURFACE]->vertices[0] );
		glTexCoord2f( 1.0f, 1.0f );
		glVertex3fv( surfaces[TOP_SURFACE]->vertices[3] );
	} else {
		glTexCoord2f( 0.0f, 0.0f );
		glVertex3fv( surfaces[TOP_SURFACE]->vertices[1] );
		glTexCoord2f( 1.0f, 0.0f );
		glVertex3fv( surfaces[TOP_SURFACE]->vertices[2] );
		glTexCoord2f( 0.0f, 1.0f );
		glVertex3fv( surfaces[TOP_SURFACE]->vertices[0] );
		glTexCoord2f( 1.0f, 1.0f );
		glVertex3fv( surfaces[TOP_SURFACE]->vertices[3] );
	}
	/*
	  glTexCoord2f( 1.0f, 1.0f );
	  glVertex3fv(surfaces[TOP_SURFACE]->vertices[0]);
	  glTexCoord2f( 1.0f, 0.0f );
	  glVertex3fv(surfaces[TOP_SURFACE]->vertices[1]);
	  glTexCoord2f( 0.0f, 0.0f );
	  glVertex3fv(surfaces[TOP_SURFACE]->vertices[2]);
	  glTexCoord2f( 0.0f, 1.0f );
	  glVertex3fv(surfaces[TOP_SURFACE]->vertices[3]);
	*/
	glEnd();
	glEndList();
}

GLShape::~GLShape() {
	delete surfaces[LEFT_SURFACE];
	delete surfaces[BOTTOM_SURFACE];
	delete surfaces[RIGHT_SURFACE];
	delete surfaces[FRONT_SURFACE];
	delete surfaces[TOP_SURFACE];
	clearVirtualShapes( true );
	if ( initialized ) glDeleteLists( displayListStart, 3 );
}

void GLShape::drawShadow() {
	// cull back faces
	glEnable( GL_CULL_FACE );
	glCullFace( GL_BACK );

	glCallList( displayListStart );

	// reset shadow flag
	useShadow = false;
}

void GLShape::draw() {

	if ( !initialized ) {
		cerr << "*** Warning: shape not intialized. name=" << getName() << endl;
	}

	if ( useShadow ) {
		drawShadow();
		return;
	}

	// cull back faces
	glEnable( GL_CULL_FACE );
	glCullFace( GL_BACK );
	GLboolean textureWasEnabled = glIsEnabled( GL_TEXTURE_2D );
	glEnable( GL_TEXTURE_2D );
	bool isFloorShape = ( height < 1 );
	bool *sides = getOccludedSides();
	int textureIndexTop = isFloorShape && getTextureIndex() > -1 ? getTextureIndex() : GLShape::TOP_SIDE;
	int textureIndexFront = ( !isFloorShape && depth < width && getTextureIndex() > -1 ? getTextureIndex() : GLShape::FRONT_SIDE );
	int textureIndexSide = ( !isFloorShape && depth > width && getTextureIndex() > -1 ? getTextureIndex() : GLShape::LEFT_RIGHT_SIDE );

	if ( sides[Shape::TOP_SIDE] ) {
		tex[textureIndexTop].glBind();
		glCallList( displayListStart + 1 );
	}
	if ( sides[Shape::N_SIDE] ) {
		if ( Constants::multitexture ) {
			glSDLActiveTextureARB( GL_TEXTURE0_ARB );
			glEnable( GL_TEXTURE_2D );
			tex[textureIndexFront].glBind();
			glSDLActiveTextureARB( GL_TEXTURE1_ARB );
			glEnable( GL_TEXTURE_2D );
			glBindTexture( GL_TEXTURE_2D, lightmap_tex_num );
		} else {
			tex[textureIndexFront].glBind();
		}
		glCallList( displayListStart + 2 + Shape::N_SIDE );
	}
	if ( sides[Shape::S_SIDE] ) {
		if ( Constants::multitexture ) {
			glSDLActiveTextureARB( GL_TEXTURE0_ARB );
			glEnable( GL_TEXTURE_2D );
			tex[textureIndexFront].glBind();
			glSDLActiveTextureARB( GL_TEXTURE1_ARB );
			glEnable( GL_TEXTURE_2D );
			glBindTexture( GL_TEXTURE_2D, lightmap_tex_num );
		} else {
			tex[textureIndexFront].glBind();
		}
		glCallList( displayListStart + 2 + Shape::S_SIDE );
	}
	if ( sides[Shape::E_SIDE] ) {
		if ( Constants::multitexture ) {
			glSDLActiveTextureARB( GL_TEXTURE0_ARB );
			glEnable( GL_TEXTURE_2D );
			tex[textureIndexSide].glBind();
			glSDLActiveTextureARB( GL_TEXTURE1_ARB );
			glEnable( GL_TEXTURE_2D );
			glBindTexture( GL_TEXTURE_2D, lightmap_tex_num2 );
		} else {
			tex[textureIndexSide].glBind();
		}
		glCallList( displayListStart + 2 + Shape::E_SIDE );
	}
	if ( sides[Shape::W_SIDE] ) {
		tex[textureIndexSide].glBind();
		glCallList( displayListStart + 2 + Shape::W_SIDE );
	}

	if ( !textureWasEnabled ) glDisable( GL_TEXTURE_2D );
}

void GLShape::outline( float r, float g, float b ) {

	float colors[4];
	glGetFloatv( GL_CURRENT_COLOR, colors );

	useShadow = true;
	GLboolean blend;
	glGetBooleanv( GL_BLEND, &blend );
	glEnable( GL_BLEND );
	glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );
	GLboolean texture = glIsEnabled( GL_TEXTURE_2D );
	glDisable( GL_TEXTURE_2D );
	glPolygonMode( GL_FRONT, GL_LINE );
	glLineWidth( 4 );
	glEnable( GL_CULL_FACE );
	glCullFace( GL_BACK );
	//glEnable( GL_DEPTH_TEST );
	//GLint df;
	//glGetIntegerv( GL_DEPTH_FUNC, &df );
	//glDepthFunc( GL_GEQUAL );
	glColor3f( r, g, b );
	glCallList( displayListStart );
	glLineWidth( 1 );
	//glDepthFunc( df );
	//glCullFace( GL_BACK );
	glDisable( GL_CULL_FACE );
	glPolygonMode( GL_FRONT, GL_FILL );
	if ( !blend ) glDisable( GL_BLEND );
	if ( texture ) glEnable( GL_TEXTURE_2D );
	useShadow = false;
	//glColor4f(1, 1, 1, 0.9f);

	glColor4fv( colors );
}

void GLShape::setupBlending() {
	glBlendFunc( GL_ONE, GL_ONE );
	//glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	//Scourge::setBlendFunc();
}

void GLShape::createDarkTexture( WallTheme *theme ) {
	if ( !Constants::multitexture ) {
		return;
	}

	// delete the previous texture
	if ( lightmap_tex_num != 0 ) {
		glDeleteTextures( 1, &lightmap_tex_num );
		glDeleteTextures( 1, &lightmap_tex_num2 );
		lightmap_tex_num = lightmap_tex_num2 = 0;
	}

	/*
	cerr << "*** Creating multitexture overlay." << endl;
	if( theme ) {
	  cerr << "*** theme=" << theme->getName() <<
	    " color=" << theme->getMultiTexRed(0) << "," << theme->getMultiTexGreen(0) << "," << theme->getMultiTexBlue(0) <<
	    "," << theme->getMultiTexInt(0) <<
	    " color2=" << theme->getMultiTexRed(1) << "," << theme->getMultiTexGreen(1) << "," << theme->getMultiTexBlue(1) <<
	    "," << theme->getMultiTexInt(1) <<
	    " smooth? " << theme->getMultiTexSmooth( 0 ) << ", " << theme->getMultiTexSmooth( 1 ) << endl;
	} else {
	  cerr << "*** no theme." << endl;
	}
	*/

	// create the dark texture
	unsigned int i, j;
	glGenTextures( 1, &lightmap_tex_num );
	glGenTextures( 1, &lightmap_tex_num2 );
	float tmp = ( theme ? theme->getMultiTexInt( 0 ) : 0.95f );
	float tmp2 = ( theme ? theme->getMultiTexInt( 1 ) : 0.8f );
	for ( i = 0; i < LIGHTMAP_SIZE; i++ ) {
		for ( j = 0; j < LIGHTMAP_SIZE; j++ ) {

			float d = 255.0f;
			if ( !theme || !theme->getMultiTexSmooth( 0 ) ) d = Util::roll( 127.0f, 255.0f );

			// purple
			data[i * LIGHTMAP_SIZE * 3 + j * 3 + 0] =
			  ( unsigned char )( d * tmp * ( theme ? theme->getMultiTexRed( 0 ) : 0.8f ) );
			data[i * LIGHTMAP_SIZE * 3 + j * 3 + 1] =
			  ( unsigned char )( d * tmp * ( theme ? theme->getMultiTexGreen( 0 ) : 0.4f ) );
			data[i * LIGHTMAP_SIZE * 3 + j * 3 + 2] =
			  ( unsigned char )( d * tmp * ( theme ? theme->getMultiTexBlue( 0 ) : 1.0f ) );

			d = 255.0f;
			if ( !theme || !theme->getMultiTexSmooth( 1 ) ) d = Util::roll( 127.0f, 255.0f );

			// dark
			data2[i * LIGHTMAP_SIZE * 3 + j * 3 + 0] =
			  ( unsigned char )( d * tmp2 * ( theme ? theme->getMultiTexRed( 1 ) : 0.5f ) );
			data2[i * LIGHTMAP_SIZE * 3 + j * 3 + 1] =
			  ( unsigned char )( d * tmp2 * ( theme ? theme->getMultiTexGreen( 1 ) : 0.6f ) );
			data2[i * LIGHTMAP_SIZE * 3 + j * 3 + 2] =
			  ( unsigned char )( d * tmp2 * ( theme ? theme->getMultiTexBlue( 1 ) : 0.8f ) );
		}
	}


	glBindTexture( GL_TEXTURE_2D, lightmap_tex_num );
	glPixelStorei( GL_UNPACK_ALIGNMENT, 1 );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR );
	glTexEnvi( GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE );
	glTexImage2D( GL_TEXTURE_2D, 0, GL_RGB, LIGHTMAP_SIZE, LIGHTMAP_SIZE, 0, GL_RGB, GL_UNSIGNED_BYTE, data );
	gluBuild2DMipmaps( GL_TEXTURE_2D, GL_RGB, LIGHTMAP_SIZE, LIGHTMAP_SIZE, GL_RGB, GL_UNSIGNED_BYTE, data );

	glBindTexture( GL_TEXTURE_2D, lightmap_tex_num2 );
	glPixelStorei( GL_UNPACK_ALIGNMENT, 1 );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR );
	glTexEnvi( GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE );
	glTexImage2D( GL_TEXTURE_2D, 0, GL_RGB, LIGHTMAP_SIZE, LIGHTMAP_SIZE, 0, GL_RGB, GL_UNSIGNED_BYTE, data2 );
	gluBuild2DMipmaps( GL_TEXTURE_2D, GL_RGB, LIGHTMAP_SIZE, LIGHTMAP_SIZE, GL_RGB, GL_UNSIGNED_BYTE, data2 );

}

bool GLShape::isLightBlocking() {
	return lightBlocking;
}

void GLShape::setLightBlocking( bool b ) {
	lightBlocking = b;
}

// loosely interpreted...
bool GLShape::fitsInside( GLShape *smaller, bool relaxedRules ) {
	return ( smaller->getHeight() < 2 ||
	         ( width + 1 > ( relaxedRules ? smaller->getWidth() / 2 : smaller->getWidth() ) &&
	           depth + 1 > ( relaxedRules ? smaller->getDepth() / 2 : smaller->getDepth() ) &&
	           height + 1 > ( relaxedRules ? smaller->getHeight() / 2 : smaller->getHeight() ) ) );
}

void GLShape::setCurrentAnimation ( int numAnim, bool force ) {
	cout << "GLShape::setCurrentAnimation : Hey this should call MD2Shape function!" << endl;
}

void GLShape::setPauseAnimation ( bool pause ) {
	cout << "GLShape::setPauseAnimation : Hey this should call MD2Shape function!" << endl;
}

Surface *GLShape::new_surface( float vertices[4][3] ) {

	Surface *surf = new Surface;

	if ( !surf ) {
		fprintf( stderr, "Error: Couldn't allocate memory for surface\n" );
		return NULL;
	}

	int i, j;
	for ( i = 0; i < 4; i++ ) {
		for ( j = 0; j < 3; j++ )
			surf->vertices[i][j] = vertices[i][j];
	}
	/*
	// x axis of matrix points in world space direction of the s texture axis
	// top,right <- top,left = 3,0
	// added div by 10.0f, otherwise light is a pinpoint. Why?
	for(i = 0; i < 3; i++)
	surf->matrix[0 + i] = (surf->vertices[1][i] - surf->vertices[2][i]) / 10.0f;
	surf->s_dist = sqrt(Util::dot_product(surf->matrix, surf->matrix));
	Util::normalize(surf->matrix);

	// y axis of matrix points in world space direction of the t texture axis
	// bottom,left <- top,left = 3,0
	// added div by 10.0f, otherwise light is a pinpoint. Why?
	for(i = 0; i < 3; i++)
	surf->matrix[3 + i] = (surf->vertices[3][i] - surf->vertices[2][i]) / 10.0f;
	surf->t_dist = sqrt(Util::dot_product(surf->matrix + 3, surf->matrix + 3));
	Util::normalize(surf->matrix + 3);

	// z axis of matrix is the surface's normal
	Util::cross_product(surf->matrix, surf->matrix + 3, surf->matrix + 6);
	*/
	return surf;
}

void GLShape::initSurfaces() {
	// initialize the surfaces
	float w = static_cast<float>( width ) * MUL;
	float d = static_cast<float>( depth ) * MUL;
	float h = static_cast<float>( height ) * MUL;
	if ( h == 0 )
		h = 0.25f * MUL;

	float v[4][3];

	/**
	Vertices:
	1 - top left (texture mapped)
	2 - bottom left
	3 - bottom right
	4 - top right
	 */

	v[0][0] = 0.0f; v[0][1] = d;    v[0][2] = 0.0f;
	v[1][0] = 0.0f; v[1][1] = d;    v[1][2] = h;
	v[2][0] = 0.0f; v[2][1] = 0.0f; v[2][2] = h;
	v[3][0] = 0.0f; v[3][1] = 0.0f; v[3][2] = 0.0f;
	delete surfaces[LEFT_SURFACE];
	surfaces[LEFT_SURFACE] = new_surface( v );

	v[0][0] = 0.0f; v[0][1] = 0.0f; v[0][2] = 0.0f;
	v[1][0] = 0.0f; v[1][1] = 0.0f; v[1][2] = h;
	v[2][0] = w;    v[2][1] = 0.0f; v[2][2] = h;
	v[3][0] = w;    v[3][1] = 0.0f; v[3][2] = 0.0f;
	delete surfaces[BOTTOM_SURFACE];
	surfaces[BOTTOM_SURFACE] = new_surface( v );

	v[0][0] = w;    v[0][1] = d;    v[0][2] = h;
	v[1][0] = w;    v[1][1] = d;    v[1][2] = 0.0f;
	v[2][0] = w;    v[2][1] = 0.0f; v[2][2] = 0.0f;
	v[3][0] = w;    v[3][1] = 0.0f; v[3][2] = h;
	delete surfaces[RIGHT_SURFACE];
	surfaces[RIGHT_SURFACE] = new_surface( v );

	v[0][0] = w;    v[0][1] = d;    v[0][2] = 0.0f;
	v[1][0] = w;    v[1][1] = d;    v[1][2] = h;
	v[2][0] = 0.0f; v[2][1] = d;    v[2][2] = h;
	v[3][0] = 0.0f; v[3][1] = d;    v[3][2] = 0.0f;
	delete surfaces[FRONT_SURFACE];
	surfaces[FRONT_SURFACE] = new_surface( v );

	v[0][0] = w;    v[0][1] = d;    v[0][2] = h;
	v[1][0] = w;    v[1][1] = 0.0f; v[1][2] = h;
	v[2][0] = 0.0f; v[2][1] = 0.0f; v[2][2] = h;
	v[3][0] = 0.0f; v[3][1] = d;    v[3][2] = h;
	delete surfaces[TOP_SURFACE];
	surfaces[TOP_SURFACE] = new_surface( v );
}

void GLShape::setOccurs( Occurs *o ) {
	occurs = *o;
}

void GLShape::addVirtualShape( int x, int y, int z, int w, int d, int h, bool draws ) {
	int const TMPLEN = 255;
	char tmp[TMPLEN];
	snprintf( tmp, TMPLEN, "%s_%d", getName(), ( int )virtualShapes.size() );
	GLShape *shape = new VirtualShape( tmp, w, d, h, x, y, z, draws, this, getShapePalIndex() );
	virtualShapes.push_back( shape );
}

void GLShape::clearVirtualShapes( bool freeMemory ) {
	if ( freeMemory ) {
		for ( unsigned int n = 0; n < virtualShapes.size(); n++ ) {
			delete virtualShapes[n];
		}
	}
	virtualShapes.clear();
}
