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


using namespace std;

// ###### MS Visual C++ specific ###### 
#if defined(_MSC_VER) && defined(_DEBUG)
# define new DEBUG_NEW
# undef THIS_FILE
  static char THIS_FILE[] = __FILE__;
#endif 

/// Renders the 3D view for outdoor levels.

void Outdoor::draw() {
	// draw the ground
	map->renderFloor();

	// draw the creatures/objects/doors/etc.
	for ( int i = 0; i < map->otherCount; i++ ) {
		if ( map->selectedDropTarget && ( ( map->selectedDropTarget->creature && map->selectedDropTarget->creature == map->other[i].creature ) ||
		                             ( map->selectedDropTarget->item && map->selectedDropTarget->item == map->other[i].item ) ) ) {
			map->colorAlreadySet = true;
			map->setupDropLocationColor();
		}
		map->doDrawShape( &map->other[i] );

		// FIXME: if feeling masochistic, try using stencil buffer to remove shadow-on-shadow effect.
		// draw simple shadow in outdoors
		if ( map->other[i].creature ) {
			map->setupShadowColor();
			map->drawGroundTex( map->outdoorShadow, map->other[i].creature->getX() + 0.25f, map->other[i].creature->getY() + 0.25f, ( map->other[i].creature->getShape()->getWidth() + 2 ) * 0.7f, map->other[i].creature->getShape()->getDepth() * 0.7f );
		} else if ( map->other[i].pos && map->other[i].shape && map->other[i].shape->isOutdoorShadow() ) {
			map->setupShadowColor();
			map->drawGroundTex( map->outdoorShadowTree, static_cast<float>( map->other[i].pos->x ) - ( map->other[i].shape->getWidth() / 2.0f ) + ( map->other[i].shape->getWindValue() / 2.0f ), static_cast<float>( map->other[i].pos->y ) + ( map->other[i].shape->getDepth() / 2.0f ), map->other[i].shape->getWidth() * 1.7f, map->other[i].shape->getDepth() * 1.7f );
		}
	}

	for ( int i = 0; i < map->stencilCount; i++ ) map->doDrawShape( &( map->stencil[i] ) );

	// draw the effects
	glEnable( GL_TEXTURE_2D );
	glEnable( GL_BLEND );
	drawRoofs();

	glDepthMask( GL_FALSE );
	drawEffects();

	// draw the fog of war or shading
#ifdef USE_LIGHTING
#if DEBUG_MOUSE_POS == 0
	if ( map->helper && !map->adapter->isInMovieMode() && !( map->isCurrentlyUnderRoof && !map->groundVisible ) ) {
		map->helper->draw( getX(), getY(), MVW, MVD );
	}
#endif
#endif

	glDisable( GL_BLEND );
	glDepthMask( GL_TRUE );
}

/// Draws creature effects and damage counters.

void Outdoor::drawEffects() {
	for ( int i = 0; i < map->laterCount; i++ ) {
		map->later[i].shape->setupBlending();
		map->doDrawShape( &map->later[i] );
		map->later[i].shape->endBlending();
	}
	glBlendFunc( GL_SRC_ALPHA, GL_ONE );
	for ( int i = 0; i < map->damageCount; i++ ) {
		map->doDrawShape( &map->damage[i], 1 );
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
			map->doDrawShape( &map->roof[i] );
		}
//    glDisable( GL_BLEND );
	}
}
