/***************************************************************************
                          glcaveshape.cpp  -  description
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
#include "glcaveshape.h"
#include "shapes.h"

using namespace std;

// ###### MS Visual C++ specific ###### 
#if defined(_MSC_VER) && defined(_DEBUG)
# define new DEBUG_NEW
# undef THIS_FILE
  static char THIS_FILE[] = __FILE__;
#endif 

char const* GLCaveShape::names[] = {
	"CAVE_INDEX_N",
	"CAVE_INDEX_E",
	"CAVE_INDEX_S",
	"CAVE_INDEX_W",
	"CAVE_INDEX_NE",
	"CAVE_INDEX_SE",
	"CAVE_INDEX_SW",
	"CAVE_INDEX_NW",
	"CAVE_INDEX_INV_NE",
	"CAVE_INDEX_INV_SE",
	"CAVE_INDEX_INV_SW",
	"CAVE_INDEX_INV_NW",
	"CAVE_INDEX_CROSS_NW",
	"CAVE_INDEX_CROSS_NE",
	"CAVE_INDEX_BLOCK",
	"CAVE_INDEX_FLOOR",
	"LAVA_SIDE_W",
	"LAVA_SIDE_E",
	"LAVA_SIDE_N",
	"LAVA_SIDE_S",
/*	"LAVA_INSIDE_TURN_NW",
	"LAVA_INSIDE_TURN_NE",
	"LAVA_INSIDE_TURN_SE",
	"LAVA_INSIDE_TURN_SW", */
	"LAVA_OUTSIDE_TURN_NW",
	"LAVA_OUTSIDE_TURN_NE",
	"LAVA_OUTSIDE_TURN_SE",
	"LAVA_OUTSIDE_TURN_SW",
/*	"LAVA_TURNS_NW",
	"LAVA_TURNS_NE",
	"LAVA_TURNS_SE",
	"LAVA_TURNS_SW", */
	"LAVA_U_N",
	"LAVA_U_E",
	"LAVA_U_S",
	"LAVA_U_W",
	"LAVA_SIDES_NS",
	"LAVA_SIDES_EW",
	"LAVA_ALL",
	"LAVA_NONE",

};

GLCaveShape::Common GLCaveShape::our; 

//#define DEBUG_CAVE_SHAPE 1

#define LIGHT_ANGLE_HORIZ 125.0f
#define LIGHT_ANGLE_VERT 45.0f

#define LAVA_MOVE_SPEED 80
Uint32 lavaMoveTick = 0;
#define LAVA_MOVE_DELTA 0.005f
GLfloat lavaTexX = 0;
GLfloat lavaTexY = 0;

GLCaveShape::GLCaveShape( Shapes *shapes, Texture texture[]
                          , int width, int depth, int height
                          , char const* name, int index
                          , int mode, int dir, int caveIndex
                          , int stencilIndex, int stencilAngle )
		:  GLShape( texture, width, depth, height, name, 0, color, index ) {
	this->shapes = shapes;
	this->mode = mode;
	this->dir = dir;
	this->caveIndex = caveIndex;
	this->stencilIndex = stencilIndex;
	this->stencilAngle = stencilAngle;
}

GLCaveShape::~GLCaveShape() {
}

/// Sets up the level texture theme.

void GLCaveShape::initialize() {
	assert( shapes->getCurrentTheme() &&
	        shapes->getCurrentTheme()->isCave() );
	string ref = WallTheme::themeRefName[ WallTheme::THEME_REF_WALL ];
	wallTextureGroup = shapes->getCurrentTheme()->getTextureGroup( ref );
	ref = WallTheme::themeRefName[ WallTheme::THEME_REF_CORNER ];
	topTextureGroup = shapes->getCurrentTheme()->getTextureGroup( ref );
	ref = WallTheme::themeRefName[ WallTheme::THEME_REF_PASSAGE_FLOOR ];
	floorTextureGroup = shapes->getCurrentTheme()->getTextureGroup( ref );
}

/// Draws the layer specified by the private variable "mode".

void GLCaveShape::draw() {

	float w = ( float )width * MUL;
	float d = ( float )depth * MUL;
	float h = ( float )height * MUL;
	if ( h == 0 ) h = 0.25f * MUL;

	GLboolean textureWasEnabled = glIsEnabled( GL_TEXTURE_2D );
	if ( !useShadow ) {
		glEnable( GL_TEXTURE_2D );
		glEnable( GL_CULL_FACE );
		glCullFace( GL_BACK );
	}

	switch ( mode ) {
	case MODE_FLAT: drawFaces(); break;
	case MODE_CORNER: drawFaces(); break;
	case MODE_BLOCK: drawBlock( w, h, d ); break;
	case MODE_FLOOR: drawFloor( w, h, d ); break;
	case MODE_INV: drawFaces(); break;
	case MODE_LAVA: drawLava( w, h, d ); break;
	default: cerr << "Unknown cave_shape mode: " << mode << endl;
	}


	if ( !textureWasEnabled ) glDisable( GL_TEXTURE_2D );
	//useShadow = false;

	glDisable( GL_CULL_FACE );

}

/// Draws the walls.

void GLCaveShape::drawFaces() {
	vector<CaveFace*>* face = our.polys[ caveIndex ];
	if ( !face ) cerr << "Can't find face for shape: " <<
		getName() << " caveIndex=" << caveIndex << endl;
#ifdef DEBUG_CAVE_SHAPE
	for ( int t = 0; t < 2; t++ ) {
		if ( useShadow ) return;
		if ( t == 1 ) {
			glDisable( GL_TEXTURE_2D );
			glDisable( GL_DEPTH_TEST );
		}
#endif
		for ( int i = 0; i < ( int )face->size(); i++ ) {
			CaveFace *p = ( *face )[i];
			if ( !useShadow ) {
				glColor3f( p->shade, p->shade, p->shade );
			}
			switch ( p->textureType ) {
			case CaveFace::WALL:
				wallTextureGroup[ GLShape::FRONT_SIDE ].glBind();
				break;
			case CaveFace::TOP:
				wallTextureGroup[ GLShape::TOP_SIDE ].glBind();
				break;
			case CaveFace::FLOOR:
				floorTextureGroup[ GLShape::TOP_SIDE ].glBind();
				break;
			}
#ifdef DEBUG_CAVE_SHAPE
			glBegin( t == 0 ? GL_TRIANGLES : GL_LINE_LOOP );
#else
			glBegin( GL_TRIANGLES );
#endif
			if ( p->tex[0][0] > -1 ) glTexCoord2f( p->tex[0][0], p->tex[0][1] );
			CVector3 *xyz = our.points[ p->p1 ];
			glVertex3f( xyz->x, xyz->y, xyz->z );


			if ( p->tex[1][0] > -1 ) glTexCoord2f( p->tex[1][0], p->tex[1][1] );
			xyz = our.points[ p->p2 ];
			glVertex3f( xyz->x, xyz->y, xyz->z );


			if ( p->tex[2][0] > -1 ) glTexCoord2f( p->tex[2][0], p->tex[2][1] );
			xyz = our.points[ p->p3 ];
			glVertex3f( xyz->x, xyz->y, xyz->z );
			glEnd();
		}
#ifdef DEBUG_CAVE_SHAPE
	}
	glEnable( GL_TEXTURE_2D );
	glEnable( GL_DEPTH_TEST );
#endif
}

/// Draws a rectangular tile using the wall top texture.

void GLCaveShape::drawBlock( float w, float h, float d ) {
	if ( useShadow ) return;
	wallTextureGroup[ GLShape::TOP_SIDE ].glBind();

	glBegin( GL_TRIANGLE_STRIP );
	glTexCoord2f( 0, 0 );
	glVertex3f( 0, 0, h );
	glTexCoord2f( 1, 0 );
	glVertex3f( w, 0, h );
	glTexCoord2f( 0, 1 );
	glVertex3f( 0, d, h );
	glTexCoord2f( 1, 1 );
	glVertex3f( w, d, h );
	glEnd();
}

/// Draws a rectangular tile using the floor texture.

void GLCaveShape::drawFloor( float w, float h, float d ) {
	if ( useShadow ) return;
	floorTextureGroup[ GLShape::TOP_SIDE ].glBind();

	glBegin( GL_TRIANGLE_STRIP );
	glTexCoord2f( 0, 0 );
	glVertex3f( 0, 0, h );
	glTexCoord2f( 1, 0 );
	glVertex3f( w, 0, h );
	glTexCoord2f( 0, 1 );
	glVertex3f( 0, d, h );
	glTexCoord2f( 1, 1 );
	glVertex3f( w, d, h );
	glEnd();
}

/// Draws a lava tile.

void GLCaveShape::drawLava( float w, float h, float d ) {
	if ( useShadow ) return;

	Uint32 t = SDL_GetTicks();
	if ( t - lavaMoveTick > LAVA_MOVE_SPEED ) {
		lavaMoveTick = t;
		lavaTexX += LAVA_MOVE_DELTA;
		if ( lavaTexX >= 1.0f ) lavaTexX -= 1.0f;
		lavaTexY += LAVA_MOVE_DELTA;
		if ( lavaTexY >= 1.0f ) lavaTexY -= 1.0f;
	}

	GLfloat n = 0.25f * MUL;

	glDisable( GL_DEPTH_TEST );

	// draw the lava
	glEnable( GL_BLEND );
	glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );
	floorTextureGroup[ GLShape::FRONT_SIDE ].glBind();
	glColor4f(  1, 1, 1, 0.75f );

	glBegin( GL_TRIANGLE_STRIP );
	glTexCoord2f( 0 + lavaTexX, 0 + lavaTexY );
	glVertex3f( 0, 0, n );
	glTexCoord2f( 1 + lavaTexX, 0 + lavaTexY );
	glVertex3f( w, 0, n );
	glTexCoord2f( 0 + lavaTexX, 1 + lavaTexY );
	glVertex3f( 0, d, n );
	glTexCoord2f( 1 + lavaTexX, 1 + lavaTexY );
	glVertex3f( w, d, n );
	glEnd();
	glDisable( GL_BLEND );

	if ( stencilIndex > -1 ) {
		glEnable( GL_ALPHA_TEST );
		glAlphaFunc( GL_GREATER, 0x00 );
		glPushMatrix();
		glTranslatef( ( w / 2 ), ( d / 2 ), 0 );
		glRotatef( stencilAngle, 0, 0, 1 );
		glTranslatef( -( w / 2 ), -( d / 2 ), 0 );
		glBindTexture( GL_TEXTURE_2D, our.floorTex[ stencilIndex ] );

		glBegin( GL_TRIANGLE_STRIP );
		glTexCoord2f( 0, 0 );
		glVertex3f( 0, 0, n );
		glTexCoord2f( 1, 0 );
		glVertex3f( w, 0, n );
		glTexCoord2f( 0, 1 );
		glVertex3f( 0, d, n );
		glTexCoord2f( 1, 1 );
		glVertex3f( w, d, n );
		glEnd();
		glPopMatrix();
		//glDisable( GL_BLEND );
		glDisable( GL_ALPHA_TEST );
	}
	glEnable( GL_DEPTH_TEST );
}

/// Calculates the normals (for lighting) for all the polygons.

void GLCaveShape::Common::calculateNormals() {
	for ( size_t i = 0; i < polys.size(); i++ ) {
		vector<CaveFace*> *face = polys[i];
		for ( size_t t = 0; t < face->size(); t++ ) {
			CaveFace *cf = ( *face )[t];
			findNormal( points[cf->p1],
			            points[cf->p2],
			            points[cf->p3],
			            &( cf->normal ) );
		}
	}
}

/// Calculates the lighting (shading) for all the polygons.

void GLCaveShape::Common::calculateLight() {
	for ( size_t i = 0; i < polys.size(); i++ ) {
		vector<CaveFace*>* face = polys[i];
		for ( int t = 0; t < ( int )face->size(); t++ ) {
			CaveFace *cf = ( *face )[t];

			if ( points[cf->p1]->z == points[cf->p2]->z &&
			        points[cf->p2]->z == points[cf->p3]->z ) continue;


			// Simple light rendering:
			// need the normal as mapped on the xy plane
			// it's degree is the intensity of light it gets
			float lightAngle;
			cf->shade = 0;
			for ( int a = 0; a < 2; a++ ) {
				float x = ( cf->normal.x == 0 ? 0.01f : cf->normal.x );
				float y;
				if ( a == 0 ) {
					y = cf->normal.y;
					lightAngle = LIGHT_ANGLE_HORIZ;
				} else {
					y = cf->normal.z;
					lightAngle = LIGHT_ANGLE_VERT;
				}
				float rad = atan( y / x );
				float angle = ( 180.0f * rad ) / 3.14159;

				// read about the arctan problem:
				// http://hyperphysics.phy-astr.gsu.edu/hbase/ttrig.html#c3
				int q = 1;
				if ( x < 0 ) {   // Quadrant 2 & 3
					q = ( y >= 0 ? 2 : 3 );
					angle += 180;
				} else if ( y < 0 ) { // Quadrant 4
					q = 4;
					angle += 360;
				}

				// calculate the angle distance from the light
				float delta = 0;
				if ( angle > lightAngle && angle < lightAngle + 180.0f ) {
					delta = angle - lightAngle;
				} else {
					if ( angle < lightAngle ) angle += 360.0f;
					delta = ( 360 + lightAngle ) - angle;
				}

				// reverse and convert to value between 0.2 and 1
				delta = 1.0f - ( 0.8f * ( delta / 180.0f ) );

				// store the value
				if ( a == 0 ) {
					cf->shade = delta;
				} else {
					cf->shade += delta / 10.0f;
				}
			}
		}
	}
}

void GLCaveShape::Common::updatePointIndexes( int oldIndex, int newIndex ) {
	for ( size_t i = 0; i < polys.size(); i++ ) {
		vector<CaveFace*>* face = polys[i];
		for ( size_t t = 0; t < face->size(); t++ ) {
			CaveFace *cf = ( *face )[t];
			if ( cf->p1 == oldIndex ) cf->p1 = newIndex;
			if ( cf->p2 == oldIndex ) cf->p2 = newIndex;
			if ( cf->p3 == oldIndex ) cf->p3 = newIndex;
		}
	}
}

/// Removes duplicate points from the points array.

void GLCaveShape::Common::removeDupPoints() {
	vector<CVector3*> newPoints;
	for ( int i = 0; i < ( int )points.size(); i++ ) {
		CVector3* v = points[i];

		bool copyPoint = true;
		for ( int t = 0; t < ( int )newPoints.size(); t++ ) {
			if ( *v == *newPoints[t] ) {
				copyPoint = false;
				updatePointIndexes( i, t );
			}
		}

		if ( copyPoint ) {
			CVector3* nv = new CVector3( *v );
			newPoints.push_back( nv );
			updatePointIndexes( i, newPoints.size() - 1 );
		}
	}
	for ( int i = 0; i < ( int )points.size(); i++ ) {
		delete points[i];
	}
	points.clear();
	for ( int i = 0; i < ( int )newPoints.size(); i++ ) {
		points.push_back( newPoints[ i ] );
	}
}

/// Used by dividePolys().

CVector3 *GLCaveShape::Common::divideSegment( CVector3 *v1, CVector3 *v2 ) {
	CVector3 *v = new CVector3();
	v->x = ( v1->x + v2->x ) / 2.0f;
	v->y = ( v1->y + v2->y ) / 2.0f;
	v->z = ( v1->z + v2->z ) / 2.0f;
	return v;
}

/// Warps a subdivided polygon so it's not all flat.

void GLCaveShape::Common::bulgePoints( CVector3 *n1, CVector3 *n2, CVector3 *n3 ) {

	// don't warp the top
	if ( n1->z == n2->z && n2->z == n3->z ) return;

	// find the base points where z is the same
	CVector3 *b1, *b2, *a;
	if ( n1->z == n2->z ) {
		b1 = n1;
		b2 = n2;
		a = n3;
	} else if ( n1->z == n3->z ) {
		b1 = n1;
		b2 = n3;
		a = n2;
	} else {
		b1 = n2;
		b2 = n3;
		a = n1;
	}


	// find the points' normal
	CVector3 normal;
	findNormal( n1, n2, n3, &normal );

	float f = 2.0f * MUL;
	// move base points along normal
	if ( a->z == 0 ) {
		// move the anchor out if on bottom
		a->x += f * normal.x;
		a->y += f * normal.y;
	}

	// for flat shapes only, pull out the middle point
	if ( toint( normal.x ) == 0 || toint( normal.y ) == 0 ) {
		if ( b1->x == a->x || b1->y == a->y ) {
			b1->x += f * normal.x;
			b1->y += f * normal.y;
		} else {
			b2->x += f * normal.x;
			b2->y += f * normal.y;
		}
	}
}

/// Subdivides all polygons for more detail.

void GLCaveShape::Common::dividePolys() {
	for ( size_t i = 0; i < polys.size(); i++ ) {
		vector<CaveFace*> *v = polys[i];
		size_t originalSize = v->size();
		for ( size_t t = 0; t < originalSize; t++ ) {
			CaveFace *face = ( *v )[t];
			if ( points[face->p1]->z == points[face->p2]->z &&
			        points[face->p2]->z == points[face->p3]->z ) continue;

			// create new points
			int index = points.size();
			CVector3 *n1 = divideSegment( points[face->p1], points[face->p2] );
			CVector3 *n2 = divideSegment( points[face->p2], points[face->p3] );
			CVector3 *n3 = divideSegment( points[face->p3], points[face->p1] );

			bulgePoints( n1, n2, n3 );

			points.push_back( n1 );
			points.push_back( n2 );
			points.push_back( n3 );

			// 3 new triangles
			v->push_back( new CaveFace( face->p1, index, index + 2,
			                            face->tex[0][0], face->tex[0][1],
			                            ( face->tex[0][0] + face->tex[1][0] ) / 2.0f, ( face->tex[0][1] + face->tex[1][1] ) / 2.0f,
			                            ( face->tex[0][0] + face->tex[2][0] ) / 2.0f, ( face->tex[0][1] + face->tex[2][1] ) / 2.0f,
			                            face->textureType ) );
			v->push_back( new CaveFace( index, face->p2, index + 1,
			                            ( face->tex[0][0] + face->tex[1][0] ) / 2.0f, ( face->tex[0][1] + face->tex[1][1] ) / 2.0f,
			                            face->tex[1][0], face->tex[1][1],
			                            ( face->tex[1][0] + face->tex[2][0] ) / 2.0f, ( face->tex[1][1] + face->tex[2][1] ) / 2.0f,
			                            face->textureType ) );
			v->push_back( new CaveFace( index + 2, index + 1, face->p3,
			                            ( face->tex[0][0] + face->tex[2][0] ) / 2.0f, ( face->tex[0][1] + face->tex[2][1] ) / 2.0f,
			                            ( face->tex[1][0] + face->tex[2][0] ) / 2.0f, ( face->tex[1][1] + face->tex[2][1] ) / 2.0f,
			                            face->tex[2][0], face->tex[2][1],
			                            face->textureType ) );
			// the middle one replaces the current face
			face->p1 = index;
			face->p2 = index + 1;
			face->p3 = index + 2;
			face->tex[0][0] = ( face->tex[0][0] + face->tex[1][0] ) / 2.0f;
			face->tex[0][1] = ( face->tex[0][1] + face->tex[1][1] ) / 2.0f;
			face->tex[1][0] = ( face->tex[1][0] + face->tex[2][0] ) / 2.0f;
			face->tex[1][1] = ( face->tex[1][1] + face->tex[2][1] ) / 2.0f;
			face->tex[2][0] = ( face->tex[0][0] + face->tex[2][0] ) / 2.0f;
			face->tex[2][1] = ( face->tex[0][1] + face->tex[2][1] ) / 2.0f;
		}
	}
}

/// Sets up the shapes that are used in caves.

void GLCaveShape::createShapes( Texture texture[], int shapeCount, Shapes *shapes ) {
	float w = ( float )CAVE_CHUNK_SIZE * MUL;
	float d = ( float )CAVE_CHUNK_SIZE * MUL;
	float h = ( float )MAP_WALL_HEIGHT * MUL; // fixme: for floor it should be 0
	if ( h == 0 ) h = 0.25f * MUL;

	// store the points: note, that the order matters! (when calc. normals)
	// DIR_S flat
	our.points.push_back( new CVector3( w, 0, h ) );
	our.points.push_back( new CVector3( w, d, 0 ) );
	our.points.push_back( new CVector3( 0, d, 0 ) );
	our.points.push_back( new CVector3( 0, 0, h ) );
	our.poly( CAVE_INDEX_S, 0, 2, 1, 0, 1, 1, 0, 1, 1, CaveFace::WALL );
	our.poly( CAVE_INDEX_S, 0, 3, 2, 0, 1, 0, 0, 1, 0, CaveFace::WALL );

	// DIR_N flat
	our.points.push_back( new CVector3( w, d, h ) );
	our.points.push_back( new CVector3( w, 0, 0 ) );
	our.points.push_back( new CVector3( 0, 0, 0 ) );
	our.points.push_back( new CVector3( 0, d, h ) );
	our.poly( CAVE_INDEX_N, 4, 5, 6, 0, 1, 1, 1, 1, 0, CaveFace::WALL );
	our.poly( CAVE_INDEX_N, 4, 6, 7, 0, 1, 1, 0, 0, 0, CaveFace::WALL );

	// DIR_E flat
	our.points.push_back( new CVector3( 0, d, h ) );
	our.points.push_back( new CVector3( w, d, 0 ) );
	our.points.push_back( new CVector3( w, 0, 0 ) );
	our.points.push_back( new CVector3( 0, 0, h ) );
	our.poly( CAVE_INDEX_E, 8, 9, 10, 0, 1, 1, 1, 1, 0, CaveFace::WALL );
	our.poly( CAVE_INDEX_E, 8, 10, 11, 0, 1, 1, 0, 0, 0, CaveFace::WALL );

	// DIR_W flat
	our.points.push_back( new CVector3( 0, 0, 0 ) );
	our.points.push_back( new CVector3( 0, d, 0 ) );
	our.points.push_back( new CVector3( w, d, h ) );
	our.points.push_back( new CVector3( w, 0, h ) );
	our.poly( CAVE_INDEX_W, 12, 13, 14, 0, 1, 1, 1, 1, 0, CaveFace::WALL );
	our.poly( CAVE_INDEX_W, 12, 14, 15, 0, 1, 1, 0, 0, 0, CaveFace::WALL );

	// DIR_NE corner
	our.points.push_back( new CVector3( 0, d, h ) );
	our.points.push_back( new CVector3( w, d, 0 ) );
	our.points.push_back( new CVector3( 0, 0, 0 ) );
	our.poly( CAVE_INDEX_NE, 16, 17, 18, 1, 1, 0, 1, 0.5f, 0, CaveFace::WALL );

	// DIR_SE corner
	our.points.push_back( new CVector3( 0, 0, h ) );
	our.points.push_back( new CVector3( 0, d, 0 ) );
	our.points.push_back( new CVector3( w, 0, 0 ) );
	our.poly( CAVE_INDEX_SE, 19, 20, 21, 0, 1, 1, 1, 0.5f, 0, CaveFace::WALL );

	// DIR_SW corner
	our.points.push_back( new CVector3( 0, 0, 0 ) );
	our.points.push_back( new CVector3( w, d, 0 ) );
	our.points.push_back( new CVector3( w, 0, h ) );
	our.poly( CAVE_INDEX_SW, 22, 23, 24, 0, 1, 1, 1, 0.5f, 0, CaveFace::WALL );

	// DIR_NW corner
	our.points.push_back( new CVector3( w, 0, 0 ) );
	our.points.push_back( new CVector3( 0, d, 0 ) );
	our.points.push_back( new CVector3( w, d, h ) );
	our.poly( CAVE_INDEX_NW, 25, 26, 27, 0, 1, 1, 1, 0.5f, 0, CaveFace::WALL );

	// DIR_NE inverse top
	our.points.push_back( new CVector3( 0, 0, h ) );
	our.points.push_back( new CVector3( w, d, h ) );
	our.points.push_back( new CVector3( 0, d, h ) );
	our.poly( CAVE_INDEX_INV_NE, 30, 29, 28, 0, 1, 1, 1, 0, 0, CaveFace::TOP );

	// DIR_SE inverse top
	our.points.push_back( new CVector3( w, 0, h ) );
	our.points.push_back( new CVector3( 0, d, h ) );
	our.points.push_back( new CVector3( 0, 0, h ) );
	our.poly( CAVE_INDEX_INV_SE, 33, 32, 31, 0, 0, 0, 1, 1, 0, CaveFace::TOP );

	// DIR_SW  inverse top
	our.points.push_back( new CVector3( 0, 0, h ) );
	our.points.push_back( new CVector3( w, d, h ) );
	our.points.push_back( new CVector3( w, 0, h ) );
	our.poly( CAVE_INDEX_INV_SW, 34, 35, 36, 0, 0, 1, 1, 1, 0, CaveFace::TOP );

	// DIR_NW  inverse top
	our.points.push_back( new CVector3( w, 0, h ) );
	our.points.push_back( new CVector3( 0, d, h ) );
	our.points.push_back( new CVector3( w, d, h ) );
	our.poly( CAVE_INDEX_INV_NW, 37, 38, 39, 1, 0, 0, 1, 1, 1, CaveFace::TOP );

	// DIR_NE inverse side
	our.points.push_back( new CVector3( 0, 0, h ) );
	our.points.push_back( new CVector3( w, d, h ) );
	our.points.push_back( new CVector3( w, 0, 0 ) );
	our.poly( CAVE_INDEX_INV_NE, 40, 41, 42, 1, 0, 0, 0, 0.5f, 1, CaveFace::WALL );

	// DIR_SE inverse side
	our.points.push_back( new CVector3( w, 0, h ) );
	our.points.push_back( new CVector3( 0, d, h ) );
	our.points.push_back( new CVector3( w, d, 0 ) );
	our.poly( CAVE_INDEX_INV_SE, 43, 44, 45, 1, 0, 0, 0, 0.5f, 1, CaveFace::WALL );

	// DIR_SW inverse side
	our.points.push_back( new CVector3( 0, d, 0 ) );
	our.points.push_back( new CVector3( w, d, h ) );
	our.points.push_back( new CVector3( 0, 0, h ) );
	our.poly( CAVE_INDEX_INV_SW, 46, 47, 48, 1, 0, 0, 0, 0.5f, 1, CaveFace::WALL );

	// DIR_NW inverse side
	our.points.push_back( new CVector3( 0, 0, 0 ) );
	our.points.push_back( new CVector3( 0, d, h ) );
	our.points.push_back( new CVector3( w, 0, h ) );
	our.poly( CAVE_INDEX_INV_NW, 49, 50, 51, 1, 0, 0, 0, 0.5f, 1, CaveFace::WALL );

	// DIR_CROSS_NW inverse side
	our.points.push_back( new CVector3( w, 0, h ) );
	our.points.push_back( new CVector3( 0, d, h ) );
	our.points.push_back( new CVector3( 0, 0, 0 ) );
	our.poly( CAVE_INDEX_CROSS_NW, 52, 53, 54, 1, 0, 0, 0, 0.5f, 1, CaveFace::WALL );

	our.points.push_back( new CVector3( w, 0, h ) );
	our.points.push_back( new CVector3( 0, d, h ) );
	our.points.push_back( new CVector3( w, d, 0 ) );
	our.poly( CAVE_INDEX_CROSS_NW, 55, 56, 57, 1, 0, 0, 0, 0.5f, 1, CaveFace::WALL );

	// DIR_CROSS_NE inverse side
	our.points.push_back( new CVector3( 0, 0, h ) );
	our.points.push_back( new CVector3( w, d, h ) );
	our.points.push_back( new CVector3( w, 0, 0 ) );
	our.poly( CAVE_INDEX_CROSS_NE, 58, 59, 60, 1, 0, 0, 0, 0.5f, 1, CaveFace::WALL );

	our.points.push_back( new CVector3( 0, 0, h ) );
	our.points.push_back( new CVector3( w, d, h ) );
	our.points.push_back( new CVector3( 0, d, 0 ) );
	our.poly( CAVE_INDEX_CROSS_NE, 61, 62, 63, 1, 0, 0, 0, 0.5f, 1, CaveFace::WALL );

	// Remove dup. points: this creates a triangle mesh
	our.removeDupPoints();

	our.dividePolys();

	our.removeDupPoints();

	our.calculateNormals();

	our.calculateLight();


	our.shapeList[CAVE_INDEX_N] =
	  new GLCaveShape( shapes, texture, CAVE_CHUNK_SIZE, CAVE_CHUNK_SIZE, MAP_WALL_HEIGHT,
	                   names[ CAVE_INDEX_N ], shapeCount++,
	                   MODE_FLAT, DIR_N, CAVE_INDEX_N );
	our.shapeList[CAVE_INDEX_E] =
	  new GLCaveShape( shapes, texture, CAVE_CHUNK_SIZE, CAVE_CHUNK_SIZE, MAP_WALL_HEIGHT,
	                   names[ CAVE_INDEX_E ], shapeCount++,
	                   MODE_FLAT, DIR_E, CAVE_INDEX_E );
	our.shapeList[CAVE_INDEX_S] =
	  new GLCaveShape( shapes, texture, CAVE_CHUNK_SIZE, CAVE_CHUNK_SIZE, MAP_WALL_HEIGHT,
	                   names[ CAVE_INDEX_S ], shapeCount++,
	                   MODE_FLAT, DIR_S, CAVE_INDEX_S );
	our.shapeList[CAVE_INDEX_W] =
	  new GLCaveShape( shapes, texture, CAVE_CHUNK_SIZE, CAVE_CHUNK_SIZE, MAP_WALL_HEIGHT,
	                   names[ CAVE_INDEX_W ], shapeCount++,
	                   MODE_FLAT, DIR_W, CAVE_INDEX_W );
	our.shapeList[CAVE_INDEX_NE] =
	  new GLCaveShape( shapes, texture, CAVE_CHUNK_SIZE, CAVE_CHUNK_SIZE, MAP_WALL_HEIGHT,
	                   names[ CAVE_INDEX_NE ], shapeCount++,
	                   MODE_CORNER, DIR_NE, CAVE_INDEX_NE );
	our.shapeList[CAVE_INDEX_SE] =
	  new GLCaveShape( shapes, texture, CAVE_CHUNK_SIZE, CAVE_CHUNK_SIZE, MAP_WALL_HEIGHT,
	                   names[ CAVE_INDEX_SE ], shapeCount++,
	                   MODE_CORNER, DIR_SE, CAVE_INDEX_SE );
	our.shapeList[CAVE_INDEX_SW] =
	  new GLCaveShape( shapes, texture, CAVE_CHUNK_SIZE, CAVE_CHUNK_SIZE, MAP_WALL_HEIGHT,
	                   names[ CAVE_INDEX_SW ], shapeCount++,
	                   MODE_CORNER, DIR_SW, CAVE_INDEX_SW );
	our.shapeList[CAVE_INDEX_NW] =
	  new GLCaveShape( shapes, texture, CAVE_CHUNK_SIZE, CAVE_CHUNK_SIZE, MAP_WALL_HEIGHT,
	                   names[ CAVE_INDEX_NW ], shapeCount++,
	                   MODE_CORNER, DIR_NW, CAVE_INDEX_NW );
	our.shapeList[CAVE_INDEX_INV_NE] =
	  new GLCaveShape( shapes, texture, CAVE_CHUNK_SIZE, CAVE_CHUNK_SIZE, MAP_WALL_HEIGHT,
	                   names[ CAVE_INDEX_INV_NE ], shapeCount++,
	                   MODE_INV, DIR_NE, CAVE_INDEX_INV_NE );
	our.shapeList[CAVE_INDEX_INV_SE] =
	  new GLCaveShape( shapes, texture, CAVE_CHUNK_SIZE, CAVE_CHUNK_SIZE, MAP_WALL_HEIGHT,
	                   names[ CAVE_INDEX_INV_SE ], shapeCount++,
	                   MODE_INV, DIR_SE, CAVE_INDEX_INV_SE );
	our.shapeList[CAVE_INDEX_INV_SW] =
	  new GLCaveShape( shapes, texture, CAVE_CHUNK_SIZE, CAVE_CHUNK_SIZE, MAP_WALL_HEIGHT,
	                   names[ CAVE_INDEX_INV_SW ], shapeCount++,
	                   MODE_INV, DIR_SW, CAVE_INDEX_INV_SW );
	our.shapeList[CAVE_INDEX_INV_NW] =
	  new GLCaveShape( shapes, texture, CAVE_CHUNK_SIZE, CAVE_CHUNK_SIZE, MAP_WALL_HEIGHT,
	                   names[ CAVE_INDEX_INV_NW ], shapeCount++,
	                   MODE_INV, DIR_NW, CAVE_INDEX_INV_NW );
	our.shapeList[CAVE_INDEX_CROSS_NE] =
	  new GLCaveShape( shapes, texture, CAVE_CHUNK_SIZE, CAVE_CHUNK_SIZE, MAP_WALL_HEIGHT,
	                   names[ CAVE_INDEX_CROSS_NE ], shapeCount++,
	                   MODE_INV, DIR_CROSS_NE, CAVE_INDEX_CROSS_NE );
	our.shapeList[CAVE_INDEX_CROSS_NW] =
	  new GLCaveShape( shapes, texture,
	                   CAVE_CHUNK_SIZE, CAVE_CHUNK_SIZE, MAP_WALL_HEIGHT,
	                   names[ CAVE_INDEX_CROSS_NW ], shapeCount++,
	                   MODE_INV, DIR_CROSS_NW, CAVE_INDEX_CROSS_NW );
	our.shapeList[CAVE_INDEX_BLOCK] =
	  new GLCaveShape( shapes, texture,
	                   CAVE_CHUNK_SIZE, CAVE_CHUNK_SIZE, MAP_WALL_HEIGHT,
	                   names[ CAVE_INDEX_BLOCK ], shapeCount++,
	                   MODE_BLOCK, 0, CAVE_INDEX_BLOCK );
	our.shapeList[CAVE_INDEX_FLOOR] =
	  new GLCaveShape( shapes, texture, CAVE_CHUNK_SIZE, CAVE_CHUNK_SIZE, 0,
	                   names[ CAVE_INDEX_FLOOR ], shapeCount++,
	                   MODE_FLOOR, 0, CAVE_INDEX_FLOOR );

	for ( int i = LAVA_SIDE_W; i < CAVE_INDEX_COUNT; i++ ) {
		our.shapeList[i] =
		  new GLCaveShape( shapes, texture, CAVE_CHUNK_SIZE, CAVE_CHUNK_SIZE,
		                   1,
		                   //MAP_WALL_HEIGHT,
		                   names[ i ], shapeCount++,
		                   MODE_LAVA, 0, i );
	}

	for ( int i = 0; i < CAVE_INDEX_COUNT; i++ ) {
		our.shapeList[i]->setSkipSide( false );
		if ( i < CAVE_INDEX_BLOCK ) {
			our.shapeList[i]->setStencil( false );
			our.shapeList[i]->setLightBlocking( true );
		} else {
			our.shapeList[i]->setStencil( false );
			our.shapeList[i]->setLightBlocking( false );
		}
	}
}

GLCaveShape::Common::Common()
		: points() {
	for ( int i = 0; i < Shapes::STENCIL_COUNT; i++ ) {
		floorTex[i] = 0;
		floorData[i].resize( 4 * 256 * 256 );
	}
	for ( int i = 0; i < CAVE_INDEX_COUNT; i++ ) {
		polys.push_back( new vector<CaveFace*>() );
		shapeList[i] = NULL;
	}
}

GLCaveShape::Common::~Common() {
	for ( int i = 0; i < CAVE_INDEX_COUNT; ++i ) {
		delete shapeList[i];
		for ( size_t j = 0; j < polys[i]->size(); ++j ) {
			delete (*polys[i])[j];
		}
		delete polys[i];
	}
	for ( size_t i = 0; i < points.size(); i++ ) {
		delete points[i];
	}
}

/// Creates the special cave textures.

void GLCaveShape::initializeShapes( Shapes *shapes ) {
	for ( int i = 0; i < CAVE_INDEX_COUNT; i++ ) {
		//if( !headless )
		our.shapeList[i]->initialize();
	}

	// create lava textures
	our.createLavaTexture( LAVA_SIDE_W, Shapes::STENCIL_SIDE, 0 );
	our.createLavaTexture( LAVA_SIDE_E, Shapes::STENCIL_SIDE, 180 );
	our.createLavaTexture( LAVA_SIDE_N, Shapes::STENCIL_SIDE, 90 );
	our.createLavaTexture( LAVA_SIDE_S, Shapes::STENCIL_SIDE, 270 );

	our.createLavaTexture( LAVA_OUTSIDE_TURN_NW, Shapes::STENCIL_OUTSIDE_TURN, 0 );
	our.createLavaTexture( LAVA_OUTSIDE_TURN_NE, Shapes::STENCIL_OUTSIDE_TURN, 90 );
	our.createLavaTexture( LAVA_OUTSIDE_TURN_SE, Shapes::STENCIL_OUTSIDE_TURN, 180 );
	our.createLavaTexture( LAVA_OUTSIDE_TURN_SW, Shapes::STENCIL_OUTSIDE_TURN, 270 );

	our.createLavaTexture( LAVA_U_N, Shapes::STENCIL_U, 0 );
	our.createLavaTexture( LAVA_U_E, Shapes::STENCIL_U, 90 );
	our.createLavaTexture( LAVA_U_S, Shapes::STENCIL_U, 180 );
	our.createLavaTexture( LAVA_U_W, Shapes::STENCIL_U, 270 );

	our.createLavaTexture( LAVA_SIDES_EW, Shapes::STENCIL_SIDES, 0 );
	our.createLavaTexture( LAVA_SIDES_NS, Shapes::STENCIL_SIDES, 90 );

	our.createLavaTexture( LAVA_NONE, -1, 0 );

	our.createLavaTexture( LAVA_ALL, Shapes::STENCIL_ALL, 0 );

	our.createFloorTexture( shapes, Shapes::STENCIL_SIDE );
	our.createFloorTexture( shapes, Shapes::STENCIL_U );
	our.createFloorTexture( shapes, Shapes::STENCIL_ALL );
	our.createFloorTexture( shapes, Shapes::STENCIL_OUTSIDE_TURN );
	our.createFloorTexture( shapes, Shapes::STENCIL_SIDES );
}

/// Creates a floor/lava border texture from a floor texture and a stencil texture.

void GLCaveShape::Common::createFloorTexture( Shapes *shapes, int stencilIndex ) {
	if ( floorTex[ stencilIndex ] ) {
		glDeleteTextures( 1, &floorTex[ stencilIndex ] );
	}
	glGenTextures( 1, &floorTex[ stencilIndex ] );
	GLubyte *stencil = shapes->getStencilImage( stencilIndex );
	TextureData& floorThemeData = shapes->getCurrentTheme()->getFloorData();

	for ( int x = 0; x < 256; x++ ) {
		for ( int y = 0; y < 256; y++ ) {
			GLubyte sb = stencil[ x + y * 256 ];

			int p = ( ( 3 * x ) + ( y * 256 * 3 ) );
			GLubyte b = ( floorThemeData.empty() ? ( GLubyte )0x80 : floorThemeData[ p + 0 ] );
			GLubyte g = ( floorThemeData.empty() ? ( GLubyte )0x80 : floorThemeData[ p + 1 ] );
			GLubyte r = ( floorThemeData.empty() ? ( GLubyte )0x80 : floorThemeData[ p + 2 ] );

			if ( sb == 0 ) {
				floorData[ stencilIndex ][ ( ( 4 * x ) + ( y * 256 * 4 ) ) + 0 ] = 0;
				floorData[ stencilIndex ][ ( ( 4 * x ) + ( y * 256 * 4 ) ) + 1 ] = 0;
				floorData[ stencilIndex ][ ( ( 4 * x ) + ( y * 256 * 4 ) ) + 2 ] = 0;
				floorData[ stencilIndex ][ ( ( 4 * x ) + ( y * 256 * 4 ) ) + 3 ] = ( GLubyte )0x00;
			} else if ( sb == 1 ) {
				floorData[ stencilIndex ][ ( ( 4 * x ) + ( y * 256 * 4 ) ) + 0 ] = 0x0a;
				floorData[ stencilIndex ][ ( ( 4 * x ) + ( y * 256 * 4 ) ) + 1 ] = 0x0a;
				floorData[ stencilIndex ][ ( ( 4 * x ) + ( y * 256 * 4 ) ) + 2 ] = 0x0a;
				floorData[ stencilIndex ][ ( ( 4 * x ) + ( y * 256 * 4 ) ) + 3 ] = ( GLubyte )0xff;
			} else {
				floorData[ stencilIndex ][ ( ( 4 * x ) + ( y * 256 * 4 ) ) + 0 ] = r;
				floorData[ stencilIndex ][ ( ( 4 * x ) + ( y * 256 * 4 ) ) + 1 ] = g;
				floorData[ stencilIndex ][ ( ( 4 * x ) + ( y * 256 * 4 ) ) + 2 ] = b;
				floorData[ stencilIndex ][ ( ( 4 * x ) + ( y * 256 * 4 ) ) + 3 ] = ( GLubyte )0xff;
			}
		}
	}

	glBindTexture( GL_TEXTURE_2D, floorTex[ stencilIndex ] );
	glPixelStorei( GL_UNPACK_ALIGNMENT, 1 );
	glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
	glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR );
	glTexEnvi( GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE );
	glTexImage2D( GL_TEXTURE_2D, 0, ( shapes->getSession()->getPreferences()->getBpp() > 16 ? GL_RGBA : GL_RGBA4 ), 256, 256, 0, GL_RGBA, GL_UNSIGNED_BYTE, &floorData[ stencilIndex ][ 0 ] );
	gluBuild2DMipmaps( GL_TEXTURE_2D, ( shapes->getSession()->getPreferences()->getBpp() > 16 ? GL_RGBA : GL_RGBA4 ), 256, 256, GL_RGBA, GL_UNSIGNED_BYTE, &floorData[ stencilIndex ][ 0 ] );
}

/// Stores a floor/lava border texture in the shapes array.

void GLCaveShape::Common::createLavaTexture( int index, int stencilIndex, int rot ) {

	// save in shape
	shapeList[ index ]->stencilIndex = stencilIndex;
	shapeList[ index ]->stencilAngle = rot;
}

