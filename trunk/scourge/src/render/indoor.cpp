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
#include "mapadapter.h"
#include "maprenderhelper.h"
#include "renderedprojectile.h"
#include "renderedcreature.h"
#include "rendereditem.h"


using namespace std;

// ###### MS Visual C++ specific ###### 
#if defined(_MSC_VER) && defined(_DEBUG)
# define new DEBUG_NEW
# undef THIS_FILE
  static char THIS_FILE[] = __FILE__;
#endif 

/// Renders the 3D view for indoor levels.
void Indoor::draw() {
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
		if ( map->other[i].shape->isFlatCaveshape() ) {
			map->doDrawShape( &map->other[i] );
		}
	}

	// find the player
	DrawLater *playerDrawLater = NULL;
	for ( int i = 0; i < map->otherCount; i++ ) {
		if ( map->other[i].shape->isFlatCaveshape() ) continue;
		if ( map->settings->isPlayerEnabled() ) {
			if ( map->other[i].creature && map->other[i].creature == map->adapter->getPlayer() )
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
			if ( !( map->stencil[i].inFront ) ) map->doDrawShape( &( map->stencil[i] ) );
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
			map->doDrawShape( &( map->stencil[i] ) );
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

	// draw the roofs (is this needed indoors?!)
	drawRoofsIndoor();
	
	glDepthMask( GL_FALSE );
	for ( int i = 0; i < map->laterCount; i++ ) {
		map->later[i].shape->setupBlending();
		map->doDrawShape( &map->later[i] );
		map->later[i].shape->endBlending();
	}
	glBlendFunc( GL_SRC_ALPHA, GL_ONE );
	for ( int i = 0; i < map->damageCount; i++ ) {
		map->doDrawShape( &map->damage[i], 1 );
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

void Indoor::drawRoofsIndoor() {
//	Uint32 now = SDL_GetTicks();
//	if ( now - roofAlphaUpdate > 25 ) {
//		roofAlphaUpdate = now;
//		if ( isCurrentlyUnderRoof ) {
//			if ( roofAlpha > 0 ) {
//				roofAlpha -= 0.05f;
//			} else {
//				roofAlpha = 0;
//			}
//		} else {
//			if ( roofAlpha < 1 ) {
//				roofAlpha += 0.05f;
//			} else {
//				roofAlpha = 1;
//			}
//		}
//	}
//	if ( roofAlpha > 0 ) {
//		for ( int i = 0; i < roofCount; i++ ) {
//			( ( GLShape* )roof[i].shape )->setAlpha( roofAlpha );
//			map->doDrawShape( &roof[i] );
//		}
//	}	
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
				map->setupBlendedWallColor();
				map->colorAlreadySet = true;
				map->doDrawShape( &( map->stencil[i] ) );
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
				map->setupBlendedWallColor();
				map->colorAlreadySet = true;
				map->doDrawShape( &( map->stencil[i] ) );
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
		map->renderFloor();
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
		map->useShadow = true;
		glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );
		for ( int i = 0; i < map->otherCount; i++ ) {
			map->doDrawShape( &map->other[i] );
		}
		if ( map->preferences->getShadows() == Constants::ALL_SHADOWS ) {
			for ( int i = 0; i < map->stencilCount; i++ ) {
				map->doDrawShape( &map->stencil[i] );
			}
		}
		map->useShadow = false;
		glDisable( GL_BLEND );
		glEnable( GL_TEXTURE_2D );
		glDepthMask( GL_TRUE );
	}

	//glEnable(GL_DEPTH_TEST);
	glDisable( GL_STENCIL_TEST );	
}

void Indoor::drawObjectsAndCreatures() {
	for ( int i = 0; i < map->otherCount; i++ ) {
		if ( map->other[i].shape->isFlatCaveshape() ) continue;
		if ( map->selectedDropTarget && ( ( map->selectedDropTarget->creature && map->selectedDropTarget->creature == map->other[i].creature ) ||
		                             ( map->selectedDropTarget->item && map->selectedDropTarget->item == map->other[i].item ) ) ) {
			map->colorAlreadySet = true;
			map->setupDropLocationColor();
		}
		map->doDrawShape( &map->other[i] );
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
		map->renderFloor();
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
	map->setupLightBlending();
	for( int i = 0; i < map->lightCount; i++ ) {
		map->doDrawShape( &map->lights[i] );
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
			map->stencil[i].shape->getSurfaces( &surfaces, true );
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
			map->doDrawShape( &map->stencil[i] );			
		}
		
		// minus the blocking surfaces
		glStencilFunc( GL_ALWAYS, 0, 0xffffffff );
		glStencilOp( GL_KEEP, GL_KEEP, GL_REPLACE );
		for ( int i = 0; i < map->stencilCount; i++ ) {
			if( !map->stencil[i].inFront && 16 > distance( map->stencil[i].pos, map->lights[t].pos ) ) {
				if( !map->stencil[i].pos->lightFacingSurfaces.empty() ) {
					set<Surface*> surfaces;
					map->stencil[i].shape->getSurfaces( &surfaces, false );
					map->stencil[i].pos->lightFacingSurfaces.clear();
					for( set<Surface*>::iterator e = surfaces.begin(); e != surfaces.end(); ++e ) {
						Surface *surface = *e;
						if( !isFacingLight( surface, map->stencil[i].pos, map->lights[t].pos ) ) {
							map->stencil[i].pos->lightFacingSurfaces.insert( surface );
						}
					}
				}
				map->doDrawShape( &map->stencil[i] );
			}
		}
		
		// draw the current light
		glStencilFunc( GL_NOTEQUAL, 0, 0xffffffff );
		glStencilOp( GL_KEEP, GL_KEEP, GL_KEEP );
		glColorMask( 1, 1, 1, 1 );
		glEnable( GL_TEXTURE_2D );
		glDisable( GL_DEPTH_TEST );		
		glEnable( GL_BLEND );
		map->setupLightBlending();
		map->doDrawShape( &map->lights[t] );
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
