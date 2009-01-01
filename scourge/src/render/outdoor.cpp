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
 *   (at your option) any map->later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "outdoor.h"
#include "map.h"
#include "mapadapter.h"
#include "maprenderhelper.h"
#include "renderedprojectile.h"
#include "renderedcreature.h"
#include "rendereditem.h"
#include "glshape.h"
#include "../debug.h"

using namespace std;

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

/// Renders the 3D view for outdoor levels.

void Outdoor::drawMap() {
	// draw the ground
	renderFloor();

	// draw the creatures/objects/doors/etc.
	for ( int i = 0; i < map->otherCount; i++ ) {
		if ( map->selectedDropTarget && ( ( map->selectedDropTarget->creature && map->selectedDropTarget->creature == map->other[i].creature ) ||
		                             ( map->selectedDropTarget->item && map->selectedDropTarget->item == map->other[i].item ) ) ) {
			map->colorAlreadySet = true;
			setupDropLocationColor();
		}
		doDrawShape( &map->other[i] );

		// FIXME: if feeling masochistic, try using stencil buffer to remove shadow-on-shadow effect.
		// draw simple shadow in outdoors
		if ( map->other[i].creature ) {
			setupShadowColor();
			drawGroundTex( map->outdoorShadow, map->other[i].creature->getX() + 0.25f, map->other[i].creature->getY() + 0.25f, ( map->other[i].creature->getShape()->getWidth() + 2 ) * 0.7f, map->other[i].creature->getShape()->getDepth() * 0.7f );
		} else if ( map->other[i].pos && map->other[i].shape && map->other[i].shape->isOutdoorShadow() ) {
			setupShadowColor();
			drawGroundTex( map->outdoorShadowTree, static_cast<float>( map->other[i].pos->x ) - ( map->other[i].shape->getWidth() / 2.0f ) + ( map->other[i].shape->getWindValue() / 2.0f ), static_cast<float>( map->other[i].pos->y ) + ( map->other[i].shape->getDepth() / 2.0f ), map->other[i].shape->getWidth() * 1.7f, map->other[i].shape->getDepth() * 1.7f );
		}
	}

	for ( int i = 0; i < map->stencilCount; i++ ) doDrawShape( &( map->stencil[i] ) );

	// draw the effects
	glEnable( GL_TEXTURE_2D );
	glEnable( GL_BLEND );
	drawRoofs();

	glDepthMask( GL_FALSE );
	drawEffects();

	// draw the fog of war or shading
#if DEBUG_MOUSE_POS == 0
	if ( map->helper && !map->adapter->isInMovieMode() && !( map->isCurrentlyUnderRoof && !map->groundVisible ) ) {
		map->helper->draw( map->getX(), map->getY(), map->mapViewWidth, map->mapViewDepth );
	}
#endif

	glDisable( GL_BLEND );
	glDepthMask( GL_TRUE );
}

/// Draws creature effects and damage counters.

void Outdoor::drawEffects() {
	for ( int i = 0; i < map->laterCount; i++ ) {
		map->later[i].shape->setupBlending();
		doDrawShape( &map->later[i] );
		map->later[i].shape->endBlending();
	}
	glBlendFunc( GL_SRC_ALPHA, GL_ONE );
	for ( int i = 0; i < map->damageCount; i++ ) {
		doDrawShape( &map->damage[i], 1 );
	}
}

/// Draws the roofs on outdoor levels, including the fading.

void Outdoor::drawRoofs() {
	// draw the roofs
	Uint32 now = SDL_GetTicks();
	if ( now - map->roofAlphaUpdate > 25 ) {
		map->roofAlphaUpdate = now;
		if ( map->isCurrentlyUnderRoof ) {
			if ( map->roofAlpha > 0 ) {
				map->roofAlpha -= 0.05f;
			} else {
				map->roofAlpha = 0;
			}
		} else {
			if ( map->roofAlpha < 1 ) {
				map->roofAlpha += 0.05f;
			} else {
				map->roofAlpha = 1;
			}
		}
	}
	if ( map->roofAlpha > 0 ) {
//  glEnable( GL_BLEND );
//  glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );
		for ( int i = 0; i < map->roofCount; i++ ) {
			( ( GLShape* )map->roof[i].shape )->setAlpha( map->roofAlpha );
			doDrawShape( &map->roof[i] );
		}
//    glDisable( GL_BLEND );
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
	shape->drawHeightMap( map->ground, posX, posY );
	glPopName();

	glTranslatef( -xpos2, -ypos2, 0.0f );
}

void Outdoor::doRenderFloor() {
	if ( map->groundVisible || map->settings->isGridShowing() ) {
		drawHeightMapFloor();
		drawWaterLevel();
	}
	map->setupShapes( true, false );
}

/// Draws the ground on outdoor maps.

bool Outdoor::drawHeightMapFloor() {
	CVectorTex *p[4];
	float gx, gy;

	bool ret = true;

	glDisable( GL_CULL_FACE );
	glEnable( GL_TEXTURE_2D );

	int startX = ( map->getX() / OUTDOORS_STEP );
	int startY = ( map->getY() / OUTDOORS_STEP );
	int endX = ( ( map->getX() + map->mapViewWidth ) / OUTDOORS_STEP ) - 1;
	int endY = ( ( map->getY() + map->mapViewDepth ) / OUTDOORS_STEP ) - 1;

	for ( int yy = startY; yy < endY; yy++ ) {
		for ( int xx = startX; xx < endX; xx++ ) {

			//int chunkX = ( ( xx * OUTDOORS_STEP ) - MAP_OFFSET ) / MAP_UNIT;
			//int chunkY = ( ( ( yy + 1 ) * OUTDOORS_STEP ) - ( MAP_OFFSET + 1 ) ) / MAP_UNIT;
			//if( lightMap[chunkX][chunkY] ) {
			map->groundPos[ xx ][ yy ].tex.glBind();
			//} else {
			//glDisable( GL_TEXTURE_2D );
			//}

			p[0] = &( map->groundPos[ xx ][ yy ] );
			p[1] = &( map->groundPos[ xx + 1 ][ yy ] );
			p[2] = &( map->groundPos[ xx ][ yy + 1 ] );
			p[3] = &( map->groundPos[ xx + 1 ][ yy + 1 ] );
			glBegin( GL_TRIANGLE_STRIP );
			for ( int i = 0; i < 4; i++ ) {
				//if( lightMap[chunkX][chunkY] ) {
				glTexCoord2f( p[i]->u, p[i]->v );
				glColor4f( p[i]->r, p[i]->g, p[i]->b, p[i]->a );
				//} else {
				//glColor4f( 0, 0, 0, 0 );
				//}
				gx = p[i]->x - map->getX() * MUL;
				gy = p[i]->y - map->getY() * MUL;
				glVertex3f( gx, gy, p[i]->z );
			}
			glEnd();
		}
	}

	//glEnable( GL_DEPTH_TEST );
	glDepthMask( GL_FALSE );
	glEnable( GL_BLEND );
	glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );

	// draw outdoor textures
	//cerr << "from: " << getX() << "," << getY() << " to: " << ( getX() + mapViewWidth ) << "," << ( getY() + mapViewDepth ) << endl;
	for ( int z = 0; z < MAX_OUTDOOR_LAYER; z++ ) {
		for ( int yy = startY; yy < endY; yy++ ) {
			for ( int xx = startX; xx < endX; xx++ ) {
				if ( map->outdoorTex[xx][yy][z].texture.isSpecified() ) {
					drawOutdoorTex( map->outdoorTex[xx][yy][z].texture, 
					                xx + map->outdoorTex[xx][yy][z].offsetX, 
					                yy + map->outdoorTex[xx][yy][z].offsetY, 
					                map->outdoorTex[xx][yy][z].width, 
					                map->outdoorTex[xx][yy][z].height, 
					                map->outdoorTex[xx][yy][z].angle );
				}
			}
		}
	}

	//glDisable( GL_DEPTH_TEST );
	glDepthMask( GL_TRUE );
	glDisable( GL_BLEND );

	if ( DEBUG_MOUSE_POS || ( map->settings->isGridShowing() && map->gridEnabled ) ) {
		//glDisable( GL_DEPTH_TEST );
		glEnable( GL_BLEND );
		glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );
		glDisable( GL_TEXTURE_2D );
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
		glEnable( GL_TEXTURE_2D );
		//glEnable( GL_DEPTH_TEST );
		glDisable( GL_BLEND );
	}

	return ret;
}

/// Draws a ground texture on outdoor maps. Uses OUTDOORS_STEP coordinates.

void Outdoor::drawOutdoorTex( Texture tex, float tx, float ty, float tw, float th, float angle ) {
	tex.glBind();

	glMatrixMode( GL_TEXTURE );
	glPushMatrix();
	glLoadIdentity();

	glTranslatef( 0.5f, 0.5f, 0.0f );
	glRotatef( angle, 0.0f, 0.0f, 1.0f );
	glTranslatef( -0.5f, -0.5f, 0.0f );

	//glTranslatef( offSX, offSY, 0 );
	glMatrixMode( GL_MODELVIEW );

	int sx = tx;
	int sy = ty;
	int ex = tx + tw;
	if ( ex == sx ) ex++;
	int ey = ty + th;
	if ( ey == sy ) ey++;

	int DIFF_Z = 0.01f * MUL;
	CVectorTex *p;
	for ( int xx = sx; xx < ex; xx++ ) {
		for ( int yy = sy; yy < ey; yy++ ) {

			float texSx = ( ( xx - sx ) ) / ( float )( ex - sx );
			float texEx = ( ( xx + 1 - sx ) ) / ( float )( ex - sx );
			float texSy = ( ( yy - sy ) ) / ( float )( ey - sy );
			float texEy = ( ( yy + 1 - sy ) ) / ( float )( ey - sy );

			//glBegin( GL_LINE_LOOP );
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

	glEnable( GL_TEXTURE_2D );
	map->getShapes()->getCurrentTheme()->getOutdoorTextureGroup( WallTheme::OUTDOOR_THEME_REF_WATER )[0].glBind();
	glEnable( GL_BLEND );
	glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );
	GLfloat ratio = MAP_UNIT / CAVE_CHUNK_SIZE;
	float w = static_cast<float>( map->mapViewWidth ) * MUL;
	float d = static_cast<float>( map->mapViewDepth ) * MUL;
	//float z = -4 * MUL;
	//glTranslatef( xpos2, ypos2, 0.0f);
	glColor4f( 1.0f, 1.0f, 1.0f, 0.35f );
// glNormal3f( 0.0f, 0.0f, 1.0f );
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
	//glDisable( GL_BLEND );
}
