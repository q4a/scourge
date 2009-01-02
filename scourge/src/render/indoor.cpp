/***************************************************************************
                indoor.cpp  -  Manages and renders the level map
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

#include "indoor.h"
#include "map.h"
#include "mapsettings.h"
#include "mapadapter.h"
#include "maprenderhelper.h"
#include "renderedprojectile.h"
#include "renderedcreature.h"
#include "rendereditem.h"
#include "glshape.h"


using namespace std;

// ###### MS Visual C++ specific ###### 
#if defined(_MSC_VER) && defined(_DEBUG)
# define new DEBUG_NEW
# undef THIS_FILE
  static char THIS_FILE[] = __FILE__;
#endif 

/// Renders the 3D view for indoor levels.
void Indoor::drawMap() {
	if ( map->preferences->getStencilbuf() && map->preferences->getStencilBufInitialized() ) {
		drawIndoorShadows();
	} else {
		// draw the ground
		map->setupShapes( true, false );
	}
	
	if ( map->preferences->isLightingEnabled() ) {
		drawLightsFloor();
	}
	
	// draw lava flows
	for ( int i = 0; i < map->otherCount; i++ ) {
		if ( map->other[i].pos->shape->isFlatCaveshape() ) {
			map->other[i].draw();
		}
	}

	// find the player
	RenderedLocation *playerDrawLater = NULL;
	for ( int i = 0; i < map->otherCount; i++ ) {
		if ( map->other[i].pos->shape->isFlatCaveshape() ) continue;
		if ( map->settings->isPlayerEnabled() ) {
			if ( map->other[i].pos->creature && map->other[i].pos->creature == map->adapter->getPlayer() )
				playerDrawLater = &( map->other[i] );
		}
	}

	// draw the walls: walls in front of the player will be transparent
	if ( playerDrawLater ) {

		if ( map->floorTexWidth == 0 && map->resortShapes ) {
			if ( map->helper->isShapeSortingEnabled() ) {
				map->sortShapes( playerDrawLater, map->stencil, map->stencilCount );
			}
			map->resortShapes = false;
		}

		// draw walls behind the player
		for ( int i = 0; i < map->stencilCount; i++ ) {
			if ( !( map->stencil[i].inFront ) ) map->stencil[i].draw();
		}
		
		// lights on walls
		if ( map->preferences->isLightingEnabled() ) {
			drawLightsWalls();
		}	
		
		// draw the creatures/objects/doors/etc.
		drawObjectsAndCreatures();		

		// draw walls in front of the player and water effects
		drawFrontWallsAndWater();

	} else {
		// no player; just draw the damn walls
		for ( int i = 0; i < map->stencilCount; i++ ) {
			map->stencil[i].draw();
		}
		
		// lights on walls
		if ( map->preferences->isLightingEnabled() ) {
			drawLightsWalls();
		}	
		
		drawObjectsAndCreatures();

		// draw water (has to come after walls to look good)
		if ( map->hasWater ) {
			glEnable( GL_BLEND );
			glDepthMask( GL_FALSE );
			drawWaterIndoor();
			glDepthMask( GL_TRUE );
		}
	}

	// draw the effects
	glEnable( GL_TEXTURE_2D );
	glEnable( GL_BLEND );
	glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );

	glDepthMask( GL_FALSE );
	for ( int i = 0; i < map->laterCount; i++ ) {
		map->later[i].pos->shape->setupBlending();
		map->later[i].draw();
		map->later[i].pos->shape->endBlending();
	}
	glBlendFunc( GL_SRC_ALPHA, GL_ONE );
	for ( int i = 0; i < map->damageCount; i++ ) {
		map->damage[i].draw();
	}	
			
	// draw the fog of war or shading
#ifdef USE_LIGHTING
#if DEBUG_MOUSE_POS == 0
	if ( map->helper && !map->adapter->isInMovieMode() && !( isCurrentlyUnderRoof && !groundVisible ) ) map->helper->draw( getX(), getY(), MVW, MVD );
#endif
#endif

	glDisable( GL_BLEND );
	glDepthMask( GL_TRUE );
}

void Indoor::drawFrontWallsAndWater() {
	glEnable( GL_DEPTH_TEST );
	glEnable( GL_BLEND );
	glDepthMask( GL_FALSE );
	if ( map->hasWater && map->preferences->getStencilbuf() && map->preferences->getStencilBufInitialized() ) {
		// map->stencil out the transparent walls (and draw them)
		//glDisable(GL_DEPTH_TEST);
		//glColorMask(0,0,0,0);
		glClear( GL_STENCIL_BUFFER_BIT );
		glEnable( GL_STENCIL_TEST );
		glStencilOp( GL_REPLACE, GL_REPLACE, GL_REPLACE );
		glStencilFunc( GL_ALWAYS, 1, 0xffffffff );
		// draw walls blended in front of the player
		// 6,2 6,4 work well
		// FIXME: blending walls have some artifacts that depth-sorting
		// is supposed to get rid of but that didn't work for me.
		//glBlendFunc( GL_SRC_ALPHA, GL_SRC_COLOR );
		glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );
		for ( int i = 0; i < map->stencilCount; i++ ) {
			if ( map->stencil[i].inFront ) {
				setupBlendedWallColor();
				RenderedLocation::colorAlreadySet = true;
				map->stencil[i].draw();
			}
		}

		// draw the water (except where the transp. walls are)
		glStencilFunc( GL_NOTEQUAL, 1, 0xffffffff );
		glStencilOp( GL_KEEP, GL_KEEP, GL_KEEP );
		drawWaterIndoor();

		glDisable( GL_STENCIL_TEST );
	} else {
		// draw transp. walls and water w/o map->stencil buffer
		//        glBlendFunc( GL_SRC_ALPHA, GL_SRC_COLOR );
		glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );
		for ( int i = 0; i < map->stencilCount; i++ ) {
			if ( map->stencil[i].inFront ) {
				setupBlendedWallColor();
				RenderedLocation::colorAlreadySet = true;
				map->stencil[i].draw();
			}
		}
		if ( map->hasWater ) {
			drawWaterIndoor();
		}	
	}
	glDepthMask( GL_TRUE );
}

void Indoor::drawWaterIndoor() {
	glDisable( GL_TEXTURE_2D );
	glBlendFunc( GL_ONE, GL_SRC_COLOR );
	map->setupShapes( false, true );
	glEnable( GL_TEXTURE_2D );	
}

void Indoor::drawIndoorShadows() {
	// map->stencil and draw the floor
	//glDisable(GL_DEPTH_TEST);
	//glColorMask(0,0,0,0);
	glEnable( GL_STENCIL_TEST );
	glStencilOp( GL_REPLACE, GL_REPLACE, GL_REPLACE );
	glStencilFunc( GL_ALWAYS, 1, 0xffffffff );

	// cave floor and map editor bottom (so cursor shows)
	if ( map->settings->isGridShowing() || map->floorTexWidth > 0 || map->isHeightMapEnabled() ) {
		renderFloor();
	} else {
		map->setupShapes( true, false );
	}

	// shadows
	if ( map->preferences->getShadows() >= Constants::OBJECT_SHADOWS && map->helper->drawShadow() ) {
		glStencilFunc( GL_EQUAL, 1, 0xffffffff );
		glStencilOp( GL_KEEP, GL_KEEP, GL_INCR );
		glDisable( GL_TEXTURE_2D );
		glDepthMask( GL_FALSE );
		glEnable( GL_BLEND );
		RenderedLocation::useShadow = true;
		glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );
		for ( int i = 0; i < map->otherCount; i++ ) {
			map->other[i].draw();
		}
		if ( map->preferences->getShadows() == Constants::ALL_SHADOWS ) {
			for ( int i = 0; i < map->stencilCount; i++ ) {
				map->stencil[i].draw();
			}
		}
		RenderedLocation::useShadow = false;
		glDisable( GL_BLEND );
		glEnable( GL_TEXTURE_2D );
		glDepthMask( GL_TRUE );
	}

	//glEnable(GL_DEPTH_TEST);
	glDisable( GL_STENCIL_TEST );	
}

void Indoor::drawObjectsAndCreatures() {
	for ( int i = 0; i < map->otherCount; i++ ) {
		if ( map->other[i].pos->shape->isFlatCaveshape() ) continue;
		if ( map->selectedDropTarget && ( ( map->selectedDropTarget->creature && map->selectedDropTarget->creature == map->other[i].pos->creature ) ||
		                             ( map->selectedDropTarget->item && map->selectedDropTarget->item == map->other[i].pos->item ) ) ) {
			RenderedLocation::colorAlreadySet = true;
			setupDropLocationColor();
		}
		map->other[i].draw();
	}	
}

void Indoor::drawLightsFloor() {
	// map->stencil and draw the floor
	//glDisable(GL_DEPTH_TEST);
	glColorMask( 0, 0, 0, 0 );
	glClear( GL_STENCIL_BUFFER_BIT );
	glEnable( GL_STENCIL_TEST );
	glStencilOp( GL_REPLACE, GL_REPLACE, GL_REPLACE );
	glStencilFunc( GL_ALWAYS, 1, 0xffffffff );
	
	// draw the floors second
	// cave floor and map editor bottom (so cursor shows)
	if ( map->settings->isGridShowing() || map->floorTexWidth > 0 || map->isHeightMapEnabled() ) {
		renderFloor();
	} else {
		map->setupShapes( true, false );
	}
	
	// draw all the lights
	glStencilFunc( GL_NOTEQUAL, 0, 0xffffffff );
	glStencilOp( GL_KEEP, GL_KEEP, GL_KEEP );
	glDisable(GL_DEPTH_TEST);
	glColorMask( 1, 1, 1, 1 );
	glDepthMask( GL_FALSE );
	glEnable( GL_TEXTURE_2D );
	glEnable( GL_BLEND );
	setupLightBlending();
	for( int i = 0; i < map->lightCount; i++ ) {
		map->lights[i].draw();
	}
	glEnable(GL_DEPTH_TEST);
	//glColorMask(0,0,0,0);
	glDepthMask( GL_TRUE );
	glDisable( GL_BLEND );
	
	//glEnable(GL_DEPTH_TEST);
	glDisable( GL_STENCIL_TEST );
}

void Indoor::drawLightsWalls() {
	//glDepthMask( GL_FALSE );
	glDepthMask( GL_TRUE );
	for( int t = 0; t < map->lightCount; t++ ) {
		// map->stencil and draw the wall
		//glDisable(GL_DEPTH_TEST);
		glColorMask( 0, 0, 0, 0 );
		glClear( GL_STENCIL_BUFFER_BIT );
		glEnable( GL_STENCIL_TEST );
		glStencilOp( GL_REPLACE, GL_REPLACE, GL_REPLACE );
		glStencilFunc( GL_ALWAYS, 1, 0xffffffff );

		// for each shape, map->stencil out the visible surfaces
		for ( int i = 0; i < map->stencilCount; i++ ) {
			if( map->stencil[i].inFront || map->isWallBetweenLocations( map->lights[t].pos, map->stencil[i].pos ) ) {
				continue;
			}
			set<Surface*> surfaces;
			map->stencil[i].pos->shape->getSurfaces( &surfaces, true );
			map->stencil[i].pos->lightFacingSurfaces.clear();
			for( set<Surface*>::iterator e = surfaces.begin(); e != surfaces.end(); ++e ) {
				Surface *surface = *e;
				if( 16 > map->distance( map->stencil[i].pos, map->lights[t].pos ) && isFacingLight( surface, map->stencil[i].pos, map->lights[t].pos ) ) {
					map->stencil[i].pos->lightFacingSurfaces.insert( surface );
				}
			}
			
			if( map->stencil[i].pos->lightFacingSurfaces.empty() ) {
				continue;
			}
			
			// draw the visible surfaces
			map->stencil[i].draw();			
		}
		
		// minus the blocking surfaces
		glStencilFunc( GL_ALWAYS, 0, 0xffffffff );
		glStencilOp( GL_KEEP, GL_KEEP, GL_REPLACE );
		for ( int i = 0; i < map->stencilCount; i++ ) {
			if( !map->stencil[i].inFront && 16 > distance( map->stencil[i].pos, map->lights[t].pos ) ) {
				if( !map->stencil[i].pos->lightFacingSurfaces.empty() ) {
					set<Surface*> surfaces;
					map->stencil[i].pos->shape->getSurfaces( &surfaces, false );
					map->stencil[i].pos->lightFacingSurfaces.clear();
					for( set<Surface*>::iterator e = surfaces.begin(); e != surfaces.end(); ++e ) {
						Surface *surface = *e;
						if( !isFacingLight( surface, map->stencil[i].pos, map->lights[t].pos ) ) {
							map->stencil[i].pos->lightFacingSurfaces.insert( surface );
						}
					}
				}
				map->stencil[i].draw();
			}
		}
		
		// draw the current light
		glStencilFunc( GL_NOTEQUAL, 0, 0xffffffff );
		glStencilOp( GL_KEEP, GL_KEEP, GL_KEEP );
		glColorMask( 1, 1, 1, 1 );
		glEnable( GL_TEXTURE_2D );
		glDisable( GL_DEPTH_TEST );		
		glEnable( GL_BLEND );
		setupLightBlending();
		map->lights[t].draw();
		glEnable( GL_DEPTH_TEST );
		glDisable( GL_BLEND );
		
		//glEnable(GL_DEPTH_TEST);
		glDisable( GL_STENCIL_TEST );
	}
	
	// reset light info
	for ( int i = 0; i < map->stencilCount; i++ ) {
		map->stencil[i].pos->lightFacingSurfaces.clear();
	}
	//glDepthMask( GL_TRUE );
	glClear( GL_STENCIL_BUFFER_BIT );
}

bool Indoor::isFacingLight( Surface *surface, Location *p, Location *lightPos ) {
	// is surface's normal pointing towards or away from lightPos?
	float pos[3];
	pos[0] = surface->matrix[6];
	pos[1] = surface->matrix[7];
	pos[2] = surface->matrix[8];
	Util::normalize( pos );
	
	// plane's distance from the origin
	float d = 0 - ( pos[0] * p->x + pos[1] * p->y + pos[2] * p->z );
	
	// the plane equation
	float s = pos[0] * lightPos->x + pos[1] * lightPos->y + pos[2] * lightPos->z + d;
	bool b = ( s > 0 ? true : false );
	
	// todo: also check that the face is visible (ie. plug in the camera's position)
	
//	cerr << "isFacingLight: normal:" << pos[0] << "," << pos[1] << "," << pos[2] << " d=" << d << " s=" << s << " " << ( b ? "LIT" : "" ) << endl;
	return b;
}

/// Draws a shape placed on an indoor water tile.

void Indoor::drawWaterPosition( int posX, int posY, float xpos2, float ypos2, Shape *shape ) {
	GLuint name;
	// encode this shape's map location in its name
	name = posX + ( MAP_WIDTH * posY );
	glTranslatef( xpos2, ypos2, 0.0f );

	// draw water
	Uint32 key = map->createPairKey( posX, posY );
	if ( map->water.find( key ) != map->water.end() ) {
		glDisable( GL_CULL_FACE );

		float sx = ( static_cast<float>( MAP_UNIT ) / static_cast<float>( WATER_TILE_X ) ) * MUL;
		float sy = ( static_cast<float>( MAP_UNIT ) / static_cast<float>( WATER_TILE_Y ) ) * MUL;

		int xp = 0;
		int yp = 0;
		while ( true ) {
			int stx = xp;
			int sty = yp;
			glBegin( GL_TRIANGLE_STRIP );
			for ( int i = 0; i < 4; i++ ) {
				int wx, wy;
				if ( xp == WATER_TILE_X && yp == WATER_TILE_Y ) {
					wx = ( posX + MAP_UNIT ) * WATER_TILE_X;
					wy = ( posY + MAP_UNIT ) * WATER_TILE_Y;
				} else if ( xp == WATER_TILE_X ) {
					wx = ( posX + MAP_UNIT ) * WATER_TILE_X;
					wy = posY * WATER_TILE_Y + yp;
				} else if ( yp == WATER_TILE_Y ) {
					wx = posX * WATER_TILE_X + xp;
					wy = ( posY + MAP_UNIT ) * WATER_TILE_Y;
				} else {
					wx = posX * WATER_TILE_X + xp;
					wy = posY * WATER_TILE_Y + yp;
				}

				int xx = wx % WATER_TILE_X;
				int yy = wy % WATER_TILE_Y;
				WaterTile *w = NULL;
				Uint32 key = map->createPairKey( wx / WATER_TILE_X, wy / WATER_TILE_Y );
				if ( map->water.find( key ) != map->water.end() ) {
					w = map->water[key];

					Uint32 time = SDL_GetTicks();
					Uint32 elapsedTime = time - w->lastTime[xx][yy];
					if ( elapsedTime >= ( Uint32 )( 1000.0f / WATER_ANIM_SPEED ) ) {

						w->z[xx][yy] += w->step[xx][yy];
						if ( w->z[xx][yy] > WATER_AMP ||
						        w->z[xx][yy] < -WATER_AMP ) w->step[xx][yy] *= -1.0f;

						w->lastTime[xx][yy] = time;
					}
				}

				float zz = ( w ? w->z[xx][yy] : 0.0f );
				float sz = ( WATER_HEIGHT + zz ) * MUL;
				glColor4f( 0.3f + ( zz / 30.0f ),
				           0.25f + ( zz / 10.0f ),
				           0.17f + ( zz / 15.0f ),
				           0.5f );


				glVertex3f( static_cast<float>( xp ) * sx, static_cast<float>( yp ) * sy, sz );

				switch ( i ) {
				case 0: xp++; break;
				case 1: yp++; xp--; break;
				case 2: xp++; break;
				case 3: yp--; xp--; break;
				}
				if ( xp > WATER_TILE_X || yp > WATER_TILE_Y ) {
					break;
				}
			}
			glEnd();
			xp = stx + 1;
			yp = sty;
			if ( xp >= WATER_TILE_X ) {
				xp = 0;
				yp++;
				if ( yp >= WATER_TILE_Y ) break;
			}
		}


		//glDepthMask( GL_TRUE );
		//glDisable( GL_BLEND );
	}

	glTranslatef( -xpos2, -ypos2, 0.0f );
}

/// Draws a shape sitting on the ground at the specified map coordinates.

void Indoor::drawGroundPosition( int posX, int posY, float xpos2, float ypos2, Shape *shape ) {
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

void Indoor::doRenderFloor() {
	if ( map->settings->isGridShowing() ) {
		glDisable( GL_TEXTURE_2D );
		glColor4f( 0.0f, 0.0f, 0.0f, 0.0f );
	}
	drawFlatFloor();
}

/// Draws the indoors floor as a single quad.

void Indoor::drawFlatFloor() {
	glDisable( GL_CULL_FACE );
	GLfloat ratio = MAP_UNIT / CAVE_CHUNK_SIZE;
	float w = static_cast<float>( map->mapViewWidth ) * MUL;
	float d = static_cast<float>( map->mapViewDepth ) * MUL;
	//glTranslatef( xpos2, ypos2, 0.0f);
// glNormal3f( 0.0f, 0.0f, 1.0f );
	glBegin( GL_TRIANGLE_STRIP );
	glTexCoord2f( map->getX() / MUL * ratio, map->getY() / MUL * ratio );
	glVertex3f( 0.0f, 0.0f, 0.0f );
	glTexCoord2f( ( map->getX() + map->mapViewWidth ) / MUL * ratio, map->getY() / MUL * ratio );
	glVertex3f( w, 0.0f, 0.0f );
	glTexCoord2f( map->getX() / MUL * ratio, ( map->getY() + map->mapViewDepth ) / MUL * ratio );
	glVertex3f( 0.0f, d, 0.0f );
	glTexCoord2f( ( map->getX() + map->mapViewWidth ) / MUL * ratio, ( map->getY() + map->mapViewDepth ) / MUL * ratio );
	glVertex3f( w, d, 0.0f );
	glEnd();
}
