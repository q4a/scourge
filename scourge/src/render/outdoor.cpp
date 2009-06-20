/***************************************************************************
                outdoor.cpp  -  Manages and renders the level map
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
 *   (at your option) any map->later version.                              *
 *                                                                         *
 ***************************************************************************/

#include "outdoor.h"
#include "map.h"
#include "mapsettings.h"
#include "mapadapter.h"
#include "maprenderhelper.h"
#include "renderedprojectile.h"
#include "renderedcreature.h"
#include "rendereditem.h"
#include "glshape.h"
#include "virtualshape.h"
#include "../debug.h"

#include "../scourge.h"

using namespace std;

// comment-out!!!
//#define DEBUG_OUTDOOR 1

// ###### MS Visual C++ specific ###### 
#if defined(_MSC_VER) && defined(_DEBUG)
# define new DEBUG_NEW
# undef THIS_FILE
  static char THIS_FILE[] = __FILE__;
#endif 
  
#define WATER_MOVE_SPEED 80
Uint32 waterMoveTick = 0;
#define WATER_MOVE_DELTA 0.005f
GLfloat waterTexX = 0;
GLfloat waterTexY = 0;

Outdoor::Outdoor( Map *map ) : MapRender( map ) { 
	useDisplayList = false; 
	hasDisplayList = false;
	GroundTexture *gt = map->getShapes()->getGroundTexture( "water" );
	waterTexture = gt->getRandomTexture();
}

Outdoor::~Outdoor() {
	waterTexture.clear();
}

/// Renders the 3D view for outdoor levels.

void Outdoor::drawMap() {
	vector<RenderedLocation*> shades;
	bool stencilOn = !map->isCurrentlyUnderRoof && map->preferences->getStencilbuf() && map->preferences->getStencilBufInitialized();
	
	if( stencilOn ) {
		glClear( GL_STENCIL_BUFFER_BIT );
		glsEnable( GLS_STENCIL_TEST );
		glStencilFunc( GL_ALWAYS, 1, 0xffffffff );
		glStencilOp( GL_REPLACE, GL_REPLACE, GL_REPLACE );
	}
	
	// draw the ground
	renderFloor();
	
	if( map->isCurrentlyUnderRoof ) {
		drawObjects( &shades );	
	}
	
	glsEnable( GLS_TEXTURE_2D );
	
	// doors
	for ( int i = 0; i < map->stencilCount; i++ ) map->stencil[i].draw();
	
	drawWalls();

	// roofs
	drawRoofs();
	
	// draw the creatures/objects/trees/etc.
	if( !map->isCurrentlyUnderRoof ) {
		drawObjects( &shades );	
	}
	
	// draw the player and remove it from the stencil (so walls behind the player don't show thru)
	if( !map->isCurrentlyUnderRoof ) {

		if( stencilOn ) {
			glStencilFunc( GL_ALWAYS, 1, 0xffffffff );
			glStencilOp( GL_KEEP, GL_KEEP, GL_ZERO );
		}

		for( unsigned int i = 0; i < shades.size(); i++ ) {
			shades[i]->draw();
		}

	}
		
	if( !map->isCurrentlyUnderRoof && stencilOn ) {
		glStencilOp( GL_KEEP, GL_KEEP, GL_KEEP );
	}
 
	drawEffects();
	
	glsDisable( GLS_BLEND );
	glsEnable( GLS_TEXTURE_2D | GLS_DEPTH_MASK );
		
	if( !map->isCurrentlyUnderRoof && stencilOn ) {
		glsDisable( GLS_DEPTH_TEST );

		glStencilFunc( GL_EQUAL, 1, 0xffffffff );
		glStencilOp( GL_ZERO, GL_ZERO, GL_ZERO );
		glColor4f( 0.25f, 0.25f, 0.25f, 0.5f );
		
		// player
		for( unsigned int i = 0; i < shades.size(); i++ ) {
			shades[i]->shade();
		}
		
		glsDisable( GLS_BLEND | GLS_STENCIL_TEST );
		glsEnable( GLS_DEPTH_TEST | GLS_DEPTH_MASK );
	}
	
	// draw water last so things like bridges show partially under-water
	drawWaterLevel();
	
	// draw the fog of war or shading
#ifndef DEBUG_OUTDOOR
	if ( map->helper && !map->adapter->isInMovieMode() && !( map->isCurrentlyUnderRoof && !map->groundVisible ) ) {
		glsDisable( GLS_DEPTH_MASK );
		glsEnable( GLS_BLEND );

		map->helper->draw( map->getX(), map->getY(), map->mapViewWidth, map->mapViewDepth );

		glsDisable( GLS_BLEND );
		glsEnable( GLS_DEPTH_MASK );
	}
#endif	
}

void Outdoor::drawObjects( vector<RenderedLocation*> *shades ) {
	for ( int i = 0; i < map->otherCount; i++ ) {

		GLShape *shape = (GLShape*)(map->other[i].pos->shape);

		if( shape->isVirtual() ) {
			shape = ((VirtualShape*)shape)->getRef();
		}
		
		if( shape->isLightBlocking() ) {
			continue;
		}
		
		if ( map->selectedDropTarget && ( ( map->selectedDropTarget->creature && map->selectedDropTarget->creature == map->other[i].pos->creature ) ||
		                             ( map->selectedDropTarget->item && map->selectedDropTarget->item == map->other[i].pos->item ) ) ) {
			RenderedLocation::colorAlreadySet = true;
			setupDropLocationColor();
		}
		
		// don't draw the player
		if( map->isCurrentlyUnderRoof || !map->other[i].pos->creature || map->other[i].pos->creature != map->getAdapter()->getPlayer() ) { 
			// only draw inside of houses when under roof
			int px = map->other[i].pos->x + map->other[i].pos->shape->getWidth() / 2;
			int py = map->other[i].pos->y - 1 - map->other[i].pos->shape->getDepth() / 2;

			if( map->isCurrentlyUnderRoof || !map->isOnFloorTile( px, py ) ) {
				map->other[i].draw();
			}

		} else {

			// don't draw party members yet
			shades->push_back( &map->other[i] );

		}

		// FIXME: if feeling masochistic, try using stencil buffer to remove shadow-on-shadow effect.
		// draw simple shadow in outdoors
		if ( map->other[i].pos->creature ) {
			setupShadowColor();
			drawGroundTex( map->outdoorShadow, map->other[i].pos->creature->getX() + 0.25f, map->other[i].pos->creature->getY() + 0.25f, ( map->other[i].pos->creature->getShape()->getWidth() + 2 ) * 0.7f, map->other[i].pos->creature->getShape()->getDepth() * 0.7f );
		} else if ( map->other[i].pos && map->other[i].pos->shape && map->other[i].pos->shape->isOutdoorShadow() ) {
			setupShadowColor();
			drawGroundTex( map->outdoorShadowTree, static_cast<float>( map->other[i].pos->x ) - ( map->other[i].pos->shape->getWidth() / 2.0f ) + ( map->other[i].pos->shape->getWindValue() / 2.0f ), static_cast<float>( map->other[i].pos->y ) + ( map->other[i].pos->shape->getDepth() / 2.0f ), map->other[i].pos->shape->getWidth() * 1.7f, map->other[i].pos->shape->getDepth() * 1.7f );
		}

	}

}

/// Draws creature effects and damage counters.

void Outdoor::drawEffects() {
	glsDisable( GLS_DEPTH_MASK );	
	glsEnable( GLS_BLEND );

	for ( int i = 0; i < map->laterCount; i++ ) {
		// skip roof-top effects when roof is off
		if( map->later[i].zpos < 10 * MUL || !map->isCurrentlyUnderRoof ) {
			map->later[i].pos->shape->setupBlending();
			map->later[i].draw();
			map->later[i].pos->shape->endBlending();
		}
	}

	glBlendFunc( GL_SRC_ALPHA, GL_ONE );

	for ( int i = 0; i < map->damageCount; i++ ) {
		if( map->damage[i].zpos < 12 * MUL || !map->isCurrentlyUnderRoof ) {
			map->damage[i].draw();
		}
	}
}


void Outdoor::drawWalls() {
	float alpha;

	for ( int i = 0; i < map->otherCount; i++ ) {

		GLShape *shape = (GLShape*)(map->other[i].pos->shape);

		if( shape->isVirtual() ) {
			shape = ((VirtualShape*)shape)->getRef();
		}

		if( shape->isLightBlocking() ) {
			map->other[i].updateWallAlpha();
				
			alpha = map->other[i].getRoofAlpha();
			if( alpha <= 0.5f ) alpha = 0.5f;
			
			if( alpha >= 1.0f ) {

				glsDisable( GLS_BLEND );

			} else {
				glsEnable( GLS_BLEND );
				glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );

				shape->setAlpha( alpha );
				RenderedLocation::colorAlreadySet = true;

			}

			map->other[i].draw();
		}

	}

}

/// Draws the roofs on outdoor levels, including the fading.

void Outdoor::drawRoofs() {
	if ( map->roofAlpha > 0 ) {

		for ( int i = 0; i < map->roofCount; i++ ) {
			
			map->roof[i].updateRoofAlpha();
			
			if( map->roof[i].getRoofAlpha() <= 0 ) continue;
			
			if( map->roof[i].getRoofAlpha() < 1 ) {

				glsEnable( GLS_BLEND );
				glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );

				( ( GLShape* )map->roof[i].pos->shape )->setAlpha( map->roof[i].getRoofAlpha() );
				RenderedLocation::colorAlreadySet = true;

			} else {

				glsDisable( GLS_BLEND );

				( ( GLShape* )map->roof[i].pos->shape )->setAlpha( 1.0f );
			}

			map->roof[i].draw();
		}

	}

}

/// Draws a shape sitting on the ground at the specified map coordinates.

void Outdoor::drawGroundPosition( int posX, int posY, float xpos2, float ypos2, Shape *shape ) {
	GLuint name;

	// encode this shape's map location in its name
	name = posX + ( MAP_WIDTH * posY );

	glTranslatef( xpos2, ypos2, 0.0f );

	glPushName( name );
	setupShapeColor();
	shape->setGround( true );
	shape->draw();
	shape->setGround( false );
	glPopName();

	glTranslatef( -xpos2, -ypos2, 0.0f );
}

void Outdoor::doRenderFloor() {
	( map->isViewChanging() || map->getAdapter()->isInMovieMode() || map->getPreferences()->getAlwaysCenterMap() || map->refreshGround ) ? useDisplayList = false : useDisplayList = true;
	map->refreshGround = false;

	if ( map->groundVisible || map->settings->isGridShowing() ) {

		if ( useDisplayList ) {

			if ( hasDisplayList ) {

				glCallList(floorDisplayList);

			} else {

				floorDisplayList = glGenLists( 1 );
				glNewList( floorDisplayList, GL_COMPILE );
				drawHeightMapFloor();
				glEndList();
				hasDisplayList = true;
				glCallList(floorDisplayList);

			}

		} else {

			if ( hasDisplayList ) {

				glDeleteLists( floorDisplayList, 1 );
				hasDisplayList = false;

			}

			drawHeightMapFloor();

		}

		//drawWaterLevel();
	}

	map->setupShapes( true, false );
}

/// Draws the ground on outdoor maps.

bool Outdoor::drawHeightMapFloor() {
	CVectorTex *p[4];
	float gx, gy;

	bool ret = true;

	glsDisable( GLS_CULL_FACE );
	glsEnable( GLS_TEXTURE_2D );

	int startX = ( map->getX() / OUTDOORS_STEP );
	int startY = ( map->getY() / OUTDOORS_STEP );
	int endX = ( ( map->getX() + map->mapViewWidth ) / OUTDOORS_STEP ) - 1;
	int endY = ( ( map->getY() + map->mapViewDepth ) / OUTDOORS_STEP ) - 1;

	for ( int yy = startY; yy < endY; yy++ ) {

		for ( int xx = startX; xx < endX; xx++ ) {

			map->groundPos[ xx ][ yy ].tex.glBind();

			p[0] = &( map->groundPos[ xx ][ yy ] );
			p[1] = &( map->groundPos[ xx + 1 ][ yy ] );
			p[2] = &( map->groundPos[ xx ][ yy + 1 ] );
			p[3] = &( map->groundPos[ xx + 1 ][ yy + 1 ] );

			glBegin( GL_TRIANGLE_STRIP );

			for ( int i = 0; i < 4; i++ ) {
				glTexCoord2f( p[i]->u, p[i]->v );
				glColor4f( p[i]->r, p[i]->g, p[i]->b, p[i]->a );
				gx = p[i]->x - map->getX() * MUL;
				gy = p[i]->y - map->getY() * MUL;
				glVertex3f( gx, gy, p[i]->z );
			}

			glEnd();
		}

	}

	glsDisable( GLS_DEPTH_MASK );
	glsEnable( GLS_BLEND );
	glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );

	// draw outdoor textures
	for ( int z = 0; z < MAX_OUTDOOR_LAYER; z++ ) {

		for ( int yy = startY; yy < endY; yy++ ) {

			for ( int xx = startX; xx < endX; xx++ ) {

				if ( map->outdoorTex[xx][yy][z].texture.isSpecified() ) {

					drawOutdoorTex( map->outdoorTex[xx][yy][z].texture, xx + map->outdoorTex[xx][yy][z].offsetX, yy + map->outdoorTex[xx][yy][z].offsetY, map->outdoorTex[xx][yy][z].width, map->outdoorTex[xx][yy][z].height, map->outdoorTex[xx][yy][z].angle );

				}

			}

		}

	}

	glsDisable( GLS_BLEND );
	glsEnable( GLS_DEPTH_MASK );

	if ( DEBUG_MOUSE_POS || ( map->settings->isGridShowing() && map->gridEnabled ) ) {
		glsDisable( GLS_TEXTURE_2D );
		glsEnable( GLS_BLEND );
		glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );

		glColor4f( 0.4f, 0.4f, 0.4f, 0.3f );

		for ( int yy = startY; yy < endY; yy++ ) {

			for ( int xx = startX; xx < endX; xx++ ) {

				p[0] = &( map->groundPos[ xx ][ yy + 1 ] );
				p[1] = &( map->groundPos[ xx ][ yy ] );
				p[2] = &( map->groundPos[ xx + 1 ][ yy ] );
				p[3] = &( map->groundPos[ xx + 1 ][ yy + 1 ] );

				glBegin( GL_LINE_LOOP );
				for ( int i = 0; i < 4; i++ ) {
					gx = p[i]->x - map->getX() * MUL;
					gy = p[i]->y - map->getY() * MUL;
					glVertex3f( gx, gy, p[i]->z + 0.05f * MUL );
				}
				glEnd();

			}

		}

		glsEnable( GLS_TEXTURE_2D );
	}

	return ret;
}

/// Draws a ground texture on outdoor maps. Uses OUTDOORS_STEP coordinates.

void Outdoor::drawOutdoorTex( Texture tex, float tx, float ty, float tw, float th, float angle ) {
	int sx = tx;
	int sy = ty;
	int ex = tx + tw;
	if ( ex == sx ) ex++;
	int ey = ty + th;
	if ( ey == sy ) ey++;

	int DIFF_Z = 0.01f * MUL;
	CVectorTex *p;

	tex.glBind();

	glMatrixMode( GL_TEXTURE );

	glPushMatrix();
	glLoadIdentity();

	glTranslatef( 0.5f, 0.5f, 0.0f );
	glRotatef( angle, 0.0f, 0.0f, 1.0f );
	glTranslatef( -0.5f, -0.5f, 0.0f );

	glMatrixMode( GL_MODELVIEW );

	for ( int xx = sx; xx < ex; xx++ ) {

		for ( int yy = sy; yy < ey; yy++ ) {

			float texSx = ( ( xx - sx ) ) / ( float )( ex - sx );
			float texEx = ( ( xx + 1 - sx ) ) / ( float )( ex - sx );
			float texSy = ( ( yy - sy ) ) / ( float )( ey - sy );
			float texEy = ( ( yy + 1 - sy ) ) / ( float )( ey - sy );

			glBegin( GL_TRIANGLE_STRIP );

			p = &map->groundPos[ xx ][ yy ];
			glColor4f( p->r, p->g, p->b, p->a );
			glTexCoord2f( texSx, texSy );
			glVertex3f( p->x - map->getX() * MUL, p->y - map->getY() * MUL, p->z + DIFF_Z );

			p = &map->groundPos[ xx + 1 ][ yy ];
			glColor4f( p->r, p->g, p->b, p->a );
			glTexCoord2f( texEx, texSy );
			glVertex3f( p->x - map->getX() * MUL, p->y - map->getY() * MUL, p->z + DIFF_Z );

			p = &map->groundPos[ xx ][ yy + 1 ];
			glColor4f( p->r, p->g, p->b, p->a );
			glTexCoord2f( texSx, texEy );
			glVertex3f( p->x - map->getX() * MUL, p->y - map->getY() * MUL, p->z + DIFF_Z );

			p = &map->groundPos[ xx + 1 ][ yy + 1 ];
			glColor4f( p->r, p->g, p->b, p->a );
			glTexCoord2f( texEx, texEy );
			glVertex3f( p->x - map->getX() * MUL, p->y - map->getY() * MUL, p->z + DIFF_Z );

			glEnd();
		}
	}

	glMatrixMode( GL_TEXTURE );
	glPopMatrix();
	glMatrixMode( GL_MODELVIEW );

#ifdef DEBUG_HEIGHT_MAP
	debugGround( sx, sy, ex, ey );
#endif
}

/// Draws the water level on outdoor maps.

void Outdoor::drawWaterLevel() {
	Uint32 t = SDL_GetTicks();

	if ( t - waterMoveTick > WATER_MOVE_SPEED ) {
		waterMoveTick = t;
		waterTexX += WATER_MOVE_DELTA;
		if ( waterTexX >= 1.0f ) waterTexX -= 1.0f;
		waterTexY += WATER_MOVE_DELTA;
		if ( waterTexY >= 1.0f ) waterTexY -= 1.0f;
	}

	glsEnable( GLS_TEXTURE_2D | GLS_BLEND );
	glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );

	waterTexture.glBind();

	GLfloat ratio = MAP_UNIT / CAVE_CHUNK_SIZE;
	float w = static_cast<float>( map->mapViewWidth ) * MUL;
	float d = static_cast<float>( map->mapViewDepth ) * MUL;

	glColor4f( 1.0f, 1.0f, 1.0f, 0.35f );

	glBegin( GL_TRIANGLE_STRIP );
	glTexCoord2f( map->getX() / MUL * ratio + waterTexX, map->getY() / MUL * ratio + waterTexY );
	glVertex3f( 0.0f, 0.0f, -0.3f );
	glTexCoord2f( ( map->getX() + map->mapViewWidth ) / MUL * ratio + waterTexX, map->getY() / MUL * ratio + waterTexY );
	glVertex3f( w, 0.0f, -0.3f );
	glTexCoord2f( map->getX() / MUL * ratio + waterTexX, ( map->getY() + map->mapViewDepth ) / MUL * ratio + waterTexY );
	glVertex3f( 0.0f, d, -0.3f );
	glTexCoord2f( ( map->getX() + map->mapViewWidth ) / MUL * ratio + waterTexX, ( map->getY() + map->mapViewDepth ) / MUL * ratio + waterTexY );
	glVertex3f( w, d, -0.3f );
	glEnd();
}

/// Sets up the outdoor ground heightfield including texturing and lighting.

void Outdoor::createGroundMap() {
	float w, d, h;
	for ( int xx = 0; xx < MAP_TILES_X; xx++ ) {
		for ( int yy = 0; yy < MAP_TILES_Y; yy++ ) {
			w = static_cast<float>( xx * OUTDOORS_STEP ) * MUL;
			d = static_cast<float>( yy * OUTDOORS_STEP - 1 ) * MUL;
			h = ( map->ground[ xx ][ yy ] ) * MUL;

			map->groundPos[ xx ][ yy ].x = w;
			map->groundPos[ xx ][ yy ].y = d;
			map->groundPos[ xx ][ yy ].z = h;
			//groundPos[ xx ][ yy ].u = ( xx * OUTDOORS_STEP * 32 ) / static_cast<float>(MAP_WIDTH);
			//groundPos[ xx ][ yy ].v = ( yy * OUTDOORS_STEP * 32 ) / static_cast<float>(MAP_DEPTH);

			map->groundPos[ xx ][ yy ].u = ( ( xx % OUTDOOR_FLOOR_TEX_SIZE ) / static_cast<float>( OUTDOOR_FLOOR_TEX_SIZE ) ) + ( xx / OUTDOOR_FLOOR_TEX_SIZE );
			map->groundPos[ xx ][ yy ].v = ( ( yy % OUTDOOR_FLOOR_TEX_SIZE ) / static_cast<float>( OUTDOOR_FLOOR_TEX_SIZE ) ) + ( yy / OUTDOOR_FLOOR_TEX_SIZE );

			map->groundPos[ xx ][ yy ].tex = map->groundTex[ xx ][ yy ];

			// height-based light
			if ( map->ground[ xx ][ yy ] >= 10 ) {
				// ground (rock)
				float n = ( h / ( 13.0f * MUL ) );
				map->groundPos[ xx ][ yy ].r = n * 0.5f;
				map->groundPos[ xx ][ yy ].g = n * 0.6f;
				map->groundPos[ xx ][ yy ].b = n * 1.0f;
				map->groundPos[ xx ][ yy ].a = 1;
			} else if ( map->ground[ xx ][ yy ] <= -10 ) {
				// water
				float n = ( -h / ( 13.0f * MUL ) );
				map->groundPos[ xx ][ yy ].r = n * 0.05f;
				map->groundPos[ xx ][ yy ].g = n * 0.4f;
				map->groundPos[ xx ][ yy ].b = n * 1;
				map->groundPos[ xx ][ yy ].a = 1;
			} else {
				float n = ( h / ( 6.0f * MUL ) ) * 0.65f + 0.35f;
				if ( Util::dice( 6 ) ) {
					//groundPos[ xx ][ yy ].r = n * 0.55f;
					map->groundPos[ xx ][ yy ].r = n;
					map->groundPos[ xx ][ yy ].g = n;
					//groundPos[ xx ][ yy ].b = n * 0.45f;
					map->groundPos[ xx ][ yy ].b = n;
					map->groundPos[ xx ][ yy ].a = 1;
				} else {
					map->groundPos[ xx ][ yy ].r = n;
					map->groundPos[ xx ][ yy ].g = n;
					//groundPos[ xx ][ yy ].b = n * 0.25f;
					map->groundPos[ xx ][ yy ].b = n;
					map->groundPos[ xx ][ yy ].a = 1;
				}
			}
			//n++;
		}
	}


	// add light
	CVectorTex *p[3];
	for ( int xx = 0; xx < MAP_TILES_X; xx++ ) {
		for ( int yy = 0; yy < MAP_TILES_Y; yy++ ) {
			p[0] = &( map->groundPos[ xx ][ yy ] );
			p[1] = &( map->groundPos[ xx + 1 ][ yy ] );
			p[2] = &( map->groundPos[ xx ][ yy + 1 ] );
			addLight( p[0], p[1], p[2] );
			addLight( p[1], p[0], p[2] );
			addLight( p[2], p[0], p[1] );
		}
	}
}

/// Adds a light source.

void Outdoor::addLight( CVectorTex *pt, CVectorTex *a, CVectorTex *b ) {
	float v[3], u[3], normal[3];

	v[0] = pt->x - a->x;
	v[1] = pt->y - a->y;
	v[2] = pt->z - a->z;
	Util::normalize( v );

	u[0] = pt->x - b->x;
	u[1] = pt->y - b->y;
	u[2] = pt->z - b->z;
	Util::normalize( u );

	Util::cross_product( u, v, normal );
	float light = Util::getLight( normal );
	pt->r *= light;
	pt->g *= light;
	pt->b *= light;
}
