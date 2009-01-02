/***************************************************************************
                  maprender.cpp  -  Manages and renders the level map
                             -------------------
    begin                : Sat May 3 2003
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
#include <map>
#include <vector>
#include "maprender.h"
#include "renderedprojectile.h"
#include "renderedcreature.h"
#include "maprenderhelper.h"
#include "rendereditem.h"
#include "glshape.h"
#include "frustum.h"
#include "location.h"
#include "projectilerenderer.h"
#include "map.h"
#include "mapsettings.h"
#include "mapadapter.h"
#include "effect.h"
#include "location.h"
#include "shape.h"
#include "shapes.h"
#include "virtualshape.h"
#include "md2shape.h"
#include "../quickhull.h"
#include "../debug.h"


using namespace std;

// ###### MS Visual C++ specific ###### 
#if defined(_MSC_VER) && defined(_DEBUG)
# define new DEBUG_NEW
# undef THIS_FILE
  static char THIS_FILE[] = __FILE__;
#endif 

MapRender::MapRender( Map *map ) { 
	this->map = map;
}

MapRender::~MapRender() {
}

void MapRender::draw() {
	drawMap();
	drawProjectiles();
}

/// Draws the floor/ground of the map.

void MapRender::renderFloor() {
	glEnable( GL_TEXTURE_2D );
	//glColor4f( 1.0f, 1.0f, 1.0f, 0.9f );
	setupShapeColor();
	if ( map->floorTex.isSpecified() ) map->floorTex.glBind();
	glPushMatrix();
	
	doRenderFloor();
	
	glPopMatrix();

	// show floor in map editor
	if ( map->settings->isGridShowing() ) {
		map->setupShapes( true, false );
	}
}

/// Draws the projectiles.

void MapRender::drawProjectiles() {
	std::map<RenderedCreature *, std::vector<RenderedProjectile*>*> *projectiles = RenderedProjectile::getProjectileMap();
	for ( std::map<RenderedCreature *, std::vector<RenderedProjectile*>*>::iterator i = projectiles->begin(); i != projectiles->end(); ++i ) {
		//RenderedCreature *creature = i->first;
		vector<RenderedProjectile*> *p = i->second;
		for ( vector<RenderedProjectile*>::iterator e = p->begin(); e != p->end(); ++e ) {
			RenderedProjectile *proj = *e;

			// calculate the path
			vector<CVector3> path;
			for ( int i = 0; i < proj->getStepCount(); i++ ) {
				CVector3 v;
				v.x = ( ( proj->getX( i ) + proj->getRenderer()->getOffsetX() - static_cast<float>( map->getX() ) ) * MUL );
				v.y = ( ( proj->getY( i ) - proj->getRenderer()->getOffsetY() - static_cast<float>( map->getY() ) - 1.0f ) * MUL );
				v.z = ( proj->getZ( i ) + proj->getRenderer()->getOffsetZ() ) * MUL;
				path.push_back( v );
			}
			proj->getRenderer()->drawPath( map, proj, &path );
		}
	}
}

void MapRender::willDrawGrid() {

	glDisable( GL_CULL_FACE );
	glDisable( GL_TEXTURE_2D );

	glEnable( GL_BLEND );
	glDepthMask( GL_FALSE );
	glBlendFunc( GL_SRC_ALPHA, GL_ONE );

	// draw the starting position
	float xpos2 = static_cast<float>( map->startx - map->getX() ) * MUL;
	float ypos2 = static_cast<float>( map->starty - map->getY() - 1 ) * MUL;
	float zpos2 = 0.0f * MUL;
	float w = 2.0f * MUL;
	float h = 4.0f * MUL;
	if ( map->useFrustum && map->frustum->CubeInFrustum( xpos2, ypos2, 0.0f, w * MUL ) ) {
		for ( int i = 0; i < 2; i++ ) {
			glPushMatrix();
			glTranslatef( xpos2, ypos2, zpos2 );
			if ( i == 0 ) {
				glColor4f( 1.0f, 0.0f, 0.0f, 0.5f );
				glBegin( GL_TRIANGLES );
			} else {
				glColor4f( 1.0f, 0.7f, 0.0f, 0.5f );
				glBegin( GL_LINE_LOOP );
			}

			glVertex3f( 0.0f, 0.0f, 0.0f );
			glVertex3f( -w, -w, h );
			glVertex3f( w, -w, h );

			glVertex3f( 0.0f, 0.0f, 0.0f );
			glVertex3f( -w, w, h );
			glVertex3f( w, w, h );

			glVertex3f( 0.0f, 0.0f, 0.0f );
			glVertex3f( -w, -w, h );
			glVertex3f( -w, w, h );

			glVertex3f( 0.0f, 0.0f, 0.0f );
			glVertex3f( w, -w, h );
			glVertex3f( w, w, h );


			glVertex3f( 0.0f, 0.0f, h * 2 );
			glVertex3f( -w, -w, h );
			glVertex3f( w, -w, h );

			glVertex3f( 0.0f, 0.0f, h * 2 );
			glVertex3f( -w, w, h );
			glVertex3f( w, w, h );

			glVertex3f( 0.0f, 0.0f, h * 2 );
			glVertex3f( -w, -w, h );
			glVertex3f( -w, w, h );

			glVertex3f( 0.0f, 0.0f, h * 2 );
			glVertex3f( w, -w, h );
			glVertex3f( w, w, h );

			glEnd();
			glPopMatrix();
		}
	}

	glDisable( GL_DEPTH_TEST );
	glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );
	int chunkX = ( map->cursorFlatMapX - MAP_OFFSET ) / MAP_UNIT;
	int chunkY = ( map->cursorFlatMapY - MAP_OFFSET - 1 ) / MAP_UNIT;
	float m = 0.5f * MUL;
	int const TMPLEN = 100;
	char tmp[TMPLEN];
	for ( int i = 0; i < map->chunkCount; i++ ) {

		float n = static_cast<float>( MAP_UNIT ) * MUL;

		glPushMatrix();
		glTranslatef( map->chunks[i].x, map->chunks[i].y - ( 1.0f * MUL ), 0 );

		if ( map->chunks[i].cx == chunkX && map->chunks[i].cy == chunkY ) {
			glColor4f( 0.0f, 1.0f, 0.0f, 0.25f );
			glLineWidth( 1 );
			snprintf( tmp, TMPLEN, "%d,%d", ( chunkX * MAP_UNIT + MAP_OFFSET ), ( chunkY * MAP_UNIT + MAP_OFFSET + 1 ) );
			map->adapter->texPrint( 0, 0, tmp );
			for ( int xx = 1; xx < MAP_UNIT; xx++ ) {
				glBegin( GL_LINES );
				glVertex3f( 0, xx * MUL, m );
				glVertex3f( n, xx * MUL, m );
				glEnd();
				glBegin( GL_LINES );
				glVertex3f( xx * MUL, 0, m );
				glVertex3f( xx * MUL, n, m );
				glEnd();
			}
		} else {
			glColor4f( 1.0f, 1.0f, 1.0f, 0.25f );
			glLineWidth( 1 );
		}
		glBegin( GL_LINE_LOOP );
		glVertex3f( 0.0f, 0.0f, m );
		glVertex3f( n, 0.0f, m );
		glVertex3f( n, n, m );
		glVertex3f( 0.0f, n, m );
		glEnd();
		glPopMatrix();
	}

	glPushMatrix();

	float xp = static_cast<float>( map->cursorFlatMapX - map->getX() ) * MUL;
	float yp = ( static_cast<float>( map->cursorFlatMapY - map->getY() ) - 1.0f ) * MUL;
	float cw = static_cast<float>( map->cursorWidth ) * MUL;
	float cd = -static_cast<float>( map->cursorDepth ) * MUL;
	m = ( map->cursorZ ? map->cursorZ : 0.5f ) * MUL;
	float ch = static_cast<float>( map->cursorHeight + map->cursorZ ) * MUL;

	float red = 1.0f;
	float green = 0.9f;
	float blue = 0.15f;
	bool found = false;
	if ( map->cursorFlatMapX < MAP_WIDTH && map->cursorFlatMapY < MAP_DEPTH ) {
		for ( int xx = map->cursorFlatMapX; xx < map->cursorFlatMapX + map->cursorWidth; xx++ ) {
			for ( int yy = map->cursorFlatMapY - 1; yy >= map->cursorFlatMapY - map->cursorDepth; yy-- ) {
				for ( int zz = 0; zz < map->cursorHeight; zz++ ) {
					if ( map->pos[xx][yy + 1][zz] ) {
						found = true;
						break;
					}
				}
			}
		}
	}
	if ( found ) {
		green = 0.15f;
	}

	// draw the cursor
	glColor4f( red, green, blue, 0.25f );
	glTranslatef( xp, yp, 0.0f );
	glBegin( GL_QUADS );

	glVertex3f( 0.0f, 0.0f, m );
	glVertex3f( cw, 0.0f, m );
	glVertex3f( cw, cd, m );
	glVertex3f( 0.0f, cd, m );

	glVertex3f( 0.0f, 0.0f, ch );
	glVertex3f( cw, 0.0f, ch );
	glVertex3f( cw, cd, ch );
	glVertex3f( 0.0f, cd, ch );

	glVertex3f( 0.0f, 0.0f, m );
	glVertex3f( cw, 0.0f, m );
	glVertex3f( cw, 0.0f, ch );
	glVertex3f( 0.0f, 0.0f, ch );

	glVertex3f( 0.0f, cd, m );
	glVertex3f( cw, cd, m );
	glVertex3f( cw, cd, ch );
	glVertex3f( 0.0f, cd, ch );

	glVertex3f( 0.0f, 0.0f, m );
	glVertex3f( 0.0f, cd, m );
	glVertex3f( 0.0f, cd, ch );
	glVertex3f( 0.0f, 0.0f, ch );

	glVertex3f( cw, 0.0f, m );
	glVertex3f( cw, cd, m );
	glVertex3f( cw, cd, ch );
	glVertex3f( cw, 0.0f, ch );

	glEnd();
	glPopMatrix();

	glEnable( GL_CULL_FACE );
	glEnable( GL_TEXTURE_2D );

	glDisable( GL_BLEND );
	glDepthMask( GL_TRUE );
	glEnable( GL_DEPTH_TEST );
}

/// Draws the traps.

void MapRender::drawTraps() {
	for ( set<Uint8>::iterator i = map->trapSet.begin(); i != map->trapSet.end(); i++ ) {
		Trap *trap = map->getTrapLoc( static_cast<int>( *i ) );

		if ( trap->discovered || map->settings->isGridShowing() || DEBUG_TRAPS ) {

			glEnable( GL_BLEND );
			glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );
			glDisable( GL_CULL_FACE );
			glDisable( GL_TEXTURE_2D );

			// get the color
			// FIXME: colors should be ref-d from scourgeview.cpp colors
			if ( !trap->enabled ) {
				//ret = disabledTrapColor;
				glColor4f( 0.5, 0.5, 0.5, 0.5f );
			} else if ( trap->discovered ) {
				if ( trap == map->getTrapLoc( map->getSelectedTrapIndex() ) ) {
					//ret = outlineColor;
					glColor4f( 0.3f, 0.3f, 0.5f, 0.5f );
				} else {
					// ret = enabledTrapColor;
					glColor4f( 1.0f, 1.0f, 0.0f, 0.5f );
				}
			} else {
				// ret = debugTrapColor;
				glColor4f( 0.0f, 1.0f, 1.0f, 0.5f );
			}

			glLineWidth( 3 );
			//glBegin( GL_POLYGON );
			glBegin( GL_LINE_LOOP );
			for ( unsigned int i = 0; i < trap->hull.size(); i++ ) {
				CVector2 *p = trap->hull[ i ];
				glVertex3f( ( p->x - map->getX() ) * MUL, ( p->y - map->getY() ) * MUL, 0.5f * MUL );
			}
			glEnd();
			glLineWidth( 1 );
			glEnable( GL_TEXTURE_2D );
			//glDisable( GL_BLEND );
		}
	}
}

/// Draws the rugs in Scourge HQ.

void MapRender::drawRug( Rug *rug, float xpos2, float ypos2, int xchunk, int ychunk ) {
	glPushMatrix();
	glTranslatef( xpos2, ypos2, 0.255f * MUL );
	glRotatef( rug->angle, 0.0f, 0.0f, 1.0f );
	float f = MAP_UNIT * MUL;
	float offset = 2.5f * MUL;

	float sx, sy, ex, ey;
	// starting section
	if ( rug->isHorizontal ) {
		sx = offset;
		sy = offset * 2;
		ex = f - offset;
		ey = f - offset * 2;
	} else {
		sy = offset;
		sx = offset * 2;
		ey = f - offset;
		ex = f - offset * 2;
	}

	glDisable( GL_CULL_FACE );
	glEnable( GL_TEXTURE_2D );
	setupShapeColor();
	rug->texture.glBind();
	glBegin( GL_TRIANGLE_STRIP );
	if ( rug->isHorizontal ) {
		glTexCoord2f( 1.0f, 0.0f );
		glVertex2f( sx, sy );
		glTexCoord2f( 1.0f, 1.0f );
		glVertex2f( ex, sy );
		glTexCoord2f( 0.0f, 0.0f );
		glVertex2f( sx, ey );
		glTexCoord2f( 0.0f, 1.0f );
		glVertex2f( ex, ey );
	} else {
		glTexCoord2f( 0.0f, 0.0f );
		glVertex2f( sx, sy );
		glTexCoord2f( 1.0f, 0.0f );
		glVertex2f( ex, sy );
		glTexCoord2f( 0.0f, 1.0f );
		glVertex2f( sx, ey );
		glTexCoord2f( 1.0f, 1.0f );
		glVertex2f( ex, ey );
	}
	glEnd();
	glDisable( GL_TEXTURE_2D );
	glPopMatrix();
}

/// Draws a ground texture on outdoor maps. Uses map coordinates.

/// Draw a texture on top of the ground map. This is useful for drawing shadows or
/// selection circles on top of un-even terrain.

#define GROUND_TEX_Z_OFFSET 0.26f

void MapRender::drawGroundTex( Texture tex, float tx, float ty, float tw, float th, float angle ) {

	//glEnable( GL_DEPTH_TEST );
	glDepthMask( GL_FALSE );
	glEnable( GL_BLEND );
	glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );
	glDisable( GL_CULL_FACE );
	glEnable( GL_TEXTURE_2D );
	tex.glBind();

	//glColor4f( 1, 0, 0, 1 );
	//glDepthMask( GL_FALSE );
	//glDisable( GL_DEPTH_TEST );

	// which ground pos?
	float sx = ( tx / static_cast<float>( OUTDOORS_STEP ) );
	float sy = ( ( ty - th - 1 ) / static_cast<float>( OUTDOORS_STEP ) );
	float ex = ( ( tx + tw ) / static_cast<float>( OUTDOORS_STEP ) );
	float ey = ( ( ty - 1 ) / static_cast<float>( OUTDOORS_STEP ) );
#ifdef DEBUG_RENDER
	cerr << "s=" << sx << "," << sy << " e=" << ex << "," << ey << endl;
#endif

	// offset to our texture inside the ground pos
	float offSX = tx - ( sx * OUTDOORS_STEP );
	float offSY = ( ty - th - 1 ) - ( sy * OUTDOORS_STEP );
	float offEX = offSX + tw;
	float offEY = offSY + th;

#ifdef DEBUG_RENDER
	cerr << "tex size=" << ( ( ex - sx ) * OUTDOORS_STEP ) << "," << ( ( ey - sy ) * OUTDOORS_STEP ) << " player size=" << tw << endl;
	cerr << "tex=" << ( sx * OUTDOORS_STEP ) << "," << ( sy * OUTDOORS_STEP ) << " player=" << map->adapter->getPlayer()->getX() << "," << map->adapter->getPlayer()->getY() << endl;
	cerr << "offs: " << offSX << "," << offSY << " " << offEX << "," << offEY << endl;
#endif

	// converted to texture coordinates ( 0-1 )
	offSX = -offSX / ( ( ex - sx ) * OUTDOORS_STEP );
	offSY = -offSY / ( ( ey - sy ) * OUTDOORS_STEP );
	offEX = 1 - ( offEX / ( ( ex - sx ) * OUTDOORS_STEP ) ) + 1;
	offEY = 1 - ( offEY / ( ( ey - sy ) * OUTDOORS_STEP ) ) + 1;
#ifdef DEBUG_RENDER
	cerr << "\toffs: " << offSX << "," << offSY << " " << offEX << "," << offEY << endl;
#endif

	// don't repeat the texture
	glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP );
	glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP );

	glMatrixMode( GL_TEXTURE );
	glPushMatrix();
	glLoadIdentity();

	glTranslatef( 0.5f, 0.5f, 0.0f );
	glRotatef( angle, 0.0f, 0.0f, 1.0f );
	glTranslatef( -0.5f, -0.5f, 0.0f );

	glTranslatef( offSX, offSY, 0.0f );
	glMatrixMode( GL_MODELVIEW );


	float gx, gy;
	for ( int xx = static_cast<int>( sx ); xx <= static_cast<int>( ex ); xx++ ) {
		for ( int yy = static_cast<int>( sy ); yy <= static_cast<int>( ey ); yy++ ) {

			float texSx = ( ( xx - sx ) * ( offEX - offSX ) ) / ( ex - sx );
			float texEx = ( ( xx + 1 - sx ) * ( offEX - offSX ) ) / ( ex - sx );
			float texSy = ( ( yy - sy ) * ( offEY - offSY ) ) / ( ey - sy );
			float texEy = ( ( yy + 1 - sy ) * ( offEY - offSY ) ) / ( ey - sy );

			//glBegin( GL_LINE_LOOP );
			glBegin( GL_TRIANGLE_STRIP );

			glTexCoord2f( texSx, texSy );
			//glColor4f( 1, 0, 0, 1 );
			gx = map->groundPos[ xx ][ yy ].x - map->getX() * MUL;
			gy = map->groundPos[ xx ][ yy ].y - map->getY() * MUL;
			glVertex3f( gx, gy, map->groundPos[ xx ][ yy ].z + GROUND_TEX_Z_OFFSET * MUL );

			glTexCoord2f( texEx, texSy );
			//glColor4f( 1, 1, 1, 1 );
			gx = map->groundPos[ xx + 1 ][ yy ].x - map->getX() * MUL;
			gy = map->groundPos[ xx + 1 ][ yy ].y - map->getY() * MUL;
			glVertex3f( gx, gy, map->groundPos[ xx + 1 ][ yy ].z + GROUND_TEX_Z_OFFSET * MUL );

			glTexCoord2f( texSx, texEy );
			//glColor4f( 1, 1, 1, 1 );
			gx = map->groundPos[ xx ][ yy + 1 ].x - map->getX() * MUL;
			gy = map->groundPos[ xx ][ yy + 1 ].y - map->getY() * MUL;
			glVertex3f( gx, gy, map->groundPos[ xx ][ yy + 1 ].z + GROUND_TEX_Z_OFFSET * MUL );

			glTexCoord2f( texEx, texEy );
			//glColor4f( 1, 1, 1, 1 );
			gx = map->groundPos[ xx + 1 ][ yy + 1 ].x - map->getX() * MUL;
			gy = map->groundPos[ xx + 1 ][ yy + 1 ].y - map->getY() * MUL;
			glVertex3f( gx, gy, map->groundPos[ xx + 1 ][ yy + 1 ].z + GROUND_TEX_Z_OFFSET * MUL );

			glEnd();
		}
	}

	glMatrixMode( GL_TEXTURE );
	glPopMatrix();
	glMatrixMode( GL_MODELVIEW );


	// switch back to repeating the texture
	glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT );
	glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT );

	//glDisable( GL_DEPTH_TEST );
	glDepthMask( GL_TRUE );
	glDisable( GL_BLEND );

#ifdef DEBUG_HEIGHT_MAP
	debugGround( sx, sy, ex, ey );
#endif
}

void MapRender::debugGround( int sx, int sy, int ex, int ey ) {
	glDisable( GL_TEXTURE_2D );
	glColor4f( 0.0f, 1.0f, 0.0f, 1.0f );
	float gx, gy;
	for ( int xx = sx; xx <= ex; xx++ ) {
		for ( int yy = sy; yy <= ey; yy++ ) {
			glBegin( GL_LINE_LOOP );
			gx = map->groundPos[ xx ][ yy + 1 ].x - map->getX() * MUL;
			gy = map->groundPos[ xx ][ yy + 1 ].y - map->getY() * MUL;
			glVertex3f( gx, gy, map->groundPos[ xx ][ yy + 1 ].z + GROUND_TEX_Z_OFFSET * MUL );

			gx = map->groundPos[ xx ][ yy ].x - map->getX() * MUL;
			gy = map->groundPos[ xx ][ yy ].y - map->getY() * MUL;
			glVertex3f( gx, gy, map->groundPos[ xx ][ yy ].z + GROUND_TEX_Z_OFFSET * MUL );

			gx = map->groundPos[ xx + 1 ][ yy ].x - map->getX() * MUL;
			gy = map->groundPos[ xx + 1 ][ yy ].y - map->getY() * MUL;
			glVertex3f( gx, gy, map->groundPos[ xx + 1 ][ yy ].z + GROUND_TEX_Z_OFFSET * MUL );

			gx = map->groundPos[ xx + 1 ][ yy + 1 ].x - map->getX() * MUL;
			gy = map->groundPos[ xx + 1 ][ yy + 1 ].y - map->getY() * MUL;
			glVertex3f( gx, gy, map->groundPos[ xx + 1 ][ yy + 1 ].z + GROUND_TEX_Z_OFFSET * MUL );

			glEnd();
		}
	}
	glEnable( GL_TEXTURE_2D );
}

void MapRender::createGroundMap() {
	for ( int xx = 0; xx < MAP_TILES_X; xx++ ) {
		for ( int yy = 0; yy < MAP_TILES_Y; yy++ ) {
			map->groundPos[ xx ][ yy ].x = static_cast<float>( xx * OUTDOORS_STEP ) * MUL;
			map->groundPos[ xx ][ yy ].y = static_cast<float>( yy * OUTDOORS_STEP - 1 ) * MUL;
			map->groundPos[ xx ][ yy ].z = 0;
			map->groundPos[ xx ][ yy ].u = map->groundPos[ xx ][ yy ].v = 0;
			map->groundPos[ xx ][ yy ].tex = map->groundTex[ xx ][ yy ];
			map->groundPos[ xx ][ yy ].r = map->groundPos[ xx ][ yy ].g = 
				map->groundPos[ xx ][ yy ].b = map->groundPos[ xx ][ yy ].a = 1;
		}
	}
}
