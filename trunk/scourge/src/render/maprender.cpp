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

const float shadowTransformMatrix[16] = {
	1, 0, 0, 0,
	0, 1, 0, 0,
// 0.5f, -0.5f, 0, 0,
	0.75f, -0.25f, 0, 0,
	0, 0, 0, 1
};

MapRender::MapRender( Map *map ) { 
	this->map = map;
	lightTex = map->shapes->findTextureByName( "light.png", true );
}

MapRender::~MapRender() {
}

void MapRender::draw() {
	drawMap();
	drawProjectiles();
}

/// Draws a shape stored in a DrawLater object.

void MapRender::doDrawShape( DrawLater *later, int effect ) {
	doDrawShape( later->xpos, later->ypos, later->zpos, later->shape, effect, later );
}

void MapRender::doDrawShape( float xpos2, float ypos2, float zpos2, Shape *shape, int effect, DrawLater *later ) {

	// fog test for creatures
	if ( map->helper && later && later->creature && later->pos && !map->adapter->isInMovieMode() && !map->helper->isVisible( later->pos->x, later->pos->y, later->creature->getShape() ) ) {
		return;
	}

	if ( shape ) ( ( GLShape* )shape )->useShadow = map->useShadow;

	// slow on mac os X:
	// glPushAttrib(GL_ENABLE_BIT);

	glPushMatrix();

	float heightPos = 0.0f;
	if ( later && later->pos ) {
		GLShape *s = ( GLShape* )later->pos->shape;
		if ( s->isVirtual() ) {
			s = ( ( VirtualShape* )s )->getRef();
		}

		if ( !s->getIgnoreHeightMap() ) {
			heightPos = later->pos->heightPos * MUL;
		}
	} else if ( later && later->effect ) {
		heightPos = later->effect->heightPos;
	}

	float xdiff = 0;
	float ydiff = 0;
	if ( later && later->creature ) {
		xdiff = ( later->creature->getX() - static_cast<float>( toint( later->creature->getX() ) ) );
		ydiff = ( later->creature->getY() - static_cast<float>( toint( later->creature->getY() ) ) );
	}

	if ( map->useShadow ) {
		// put shadow above the floor a little
		glTranslatef( xpos2 + xdiff * MUL, ypos2 + ydiff * MUL, ( 0.26f * MUL + heightPos ) );
		glMultMatrixf( shadowTransformMatrix );
		// purple shadows
		setupShadowColor();
	} else {
		glTranslatef( xpos2 + xdiff * MUL, ypos2 + ydiff * MUL, zpos2 + heightPos );

		if ( later && later->pos ) {
			glTranslatef( later->pos->moveX, later->pos->moveY, later->pos->moveZ );
			glRotatef( later->pos->angleX, 1.0f, 0.0f, 0.0f );
			glRotatef( later->pos->angleY, 0.0f, 1.0f, 0.0f );
			glRotatef( later->pos->angleZ, 0.0f, 0.0f, 1.0f );
		}

		if ( later && later->creature ) {
			glTranslatef( later->creature->getOffsetX(), later->creature->getOffsetY(), later->creature->getOffsetZ() );
		}

#ifdef DEBUG_SECRET_DOORS
		if ( later && later->pos ) {
			int xp = later->pos->x;
			int yp = later->pos->y;
			int index = xp + MAP_WIDTH * yp;
			if ( secretDoors.find( index ) != secretDoors.end() ) {
				glColor4f( 1.0f, 0.3f, 0.3f, 1.0f );
				colorAlreadySet = true;
			}
		}
#endif

		// show detected secret doors
		if ( later && later->pos ) {
			if ( map->isSecretDoor( later->pos ) && ( map->isSecretDoorDetected( later->pos ) || map->settings->isGridShowing() ) ) {
				setupSecretDoorColor();
				map->colorAlreadySet = true;
			}
		}

		if ( map->colorAlreadySet   ) {
			map->colorAlreadySet = false;
		} else {
			if ( later && later->pos && map->isLocked( later->pos->x, later->pos->y, later->pos->z ) ) {
				setupLockedDoorColor();
			} else {
				setupShapeColor();
			}
		}
	}

	glDisable( GL_CULL_FACE );

	if ( shape ) {
		( ( GLShape* )shape )->setCameraRot( map->xrot, map->yrot, map->zrot );
		( ( GLShape* )shape )->setCameraPos( map->xpos, map->ypos, map->zpos, xpos2, ypos2, zpos2 );
		if ( later && later->pos ) ( ( GLShape* )shape )->setLocked( map->isLocked( later->pos->x, later->pos->y, 0 ) );
		else ( ( GLShape* )shape )->setLocked( false );
	}
		
	
	// ==========================================================================
	// lights
	if( later && later->light ) {

		glPushMatrix();
		glRotatef( -map->zrot, 0.0f, 0.0f, 1.0f );
		glRotatef( -map->yrot, 1.0f, 0.0f, 0.0f );
		lightTex.glBind();
		float r;
		if( later->shape->getLightEmitter() ) {
			r = later->shape->getLightEmitter()->getRadius() * MUL;
			Color color = later->shape->getLightEmitter()->getColor();
			glColor4f( color.r, color.g, color.b, color.a );
		} else {
			r = 8 * MUL;
			setupPlayerLightColor();
		}		
		
		glBegin( GL_QUADS );
		glTexCoord2f( 1.0f, 1.0f );
		glVertex3f( -r, r, 0 );
		glTexCoord2f( 1.0f, 0.0f );
		glVertex3f( -r, -r, 0 );
		glTexCoord2f( 0.0f, 0.0f );
		glVertex3f( r, -r, 0 );
		glTexCoord2f( 0.0f, 1.0f );
		glVertex3f( r, r, 0 );		
		glEnd();
		glPopMatrix();
		
		// ==========================================================================
		// effects
	} else if ( effect && later ) {
		if ( later->creature ) {
			// translate hack for md2 models... see: md2shape::draw()
			//glTranslatef( 0, -1 * MUL, 0 );
			later->creature->getEffect()->draw( later->creature->getEffectType(),
			                                    later->creature->getDamageEffect() );
			//glTranslatef( 0, 1 * MUL, 0 );
		} else if ( later->effect ) {
			later->effect->getEffect()->draw( later->effect->getEffectType(),
			                                  later->effect->getDamageEffect() );
		}
		
		// ==========================================================================
		// creatures		
	} else if ( later && later->creature && !map->useShadow ) {
		// outline mission creatures
		if ( map->adapter->isMissionCreature( later->creature ) ) {
			//if( session->getCurrentMission() &&
			//session->getCurrentMission()->isMissionCreature( later->creature ) ) {
			shape->outline( 0.15f, 0.15f, 0.4f );
		} else if ( later->creature->isBoss() ) {
			shape->outline( 0.4f, 0.15f, 0.4f );
		} else if ( later && later->pos &&
		            later->pos->outlineColor ) {
			shape->outline( later->pos->outlineColor );
		}
		
		if ( later->creature->getStateMod( StateMod::invisible ) ) {
			glColor4f( 0.3f, 0.8f, 1.0f, 0.5f );
			glEnable( GL_BLEND );
			//glDepthMask( GL_FALSE );
			glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );
		} else if ( later->creature->getStateMod( StateMod::possessed ) ) {
			glColor4f( 1.0, 0.3f, 0.8f, 1.0f );
		}
		
		if( later && later->pos && !later->pos->lightFacingSurfaces.empty() ) {
			shape->setLightFacingSurfaces( &later->pos->lightFacingSurfaces );
		}
		shape->draw();

		if ( later->creature->getStateMod( StateMod::invisible ) ) {
			glDisable( GL_BLEND );
			//glDepthMask( GL_TRUE );
		}

		// ==========================================================================
		// items
	} else if ( later && later->item && !map->useShadow ) {

		if ( later->item->isSpecial() ) {
			shape->outline( Constants::SPECIAL_ITEM_COLOR );
		} else if ( later->item->isMagicItem() ) {
			shape->outline( Constants::MAGIC_ITEM_COLOR[ later->item->getMagicLevel() ] );
		} else if ( later->item->getContainsMagicItem() ) {
			shape->outline( 0.8f, 0.8f, 0.3f );
		}

		if ( later && later->pos && later->pos->outlineColor && !map->useShadow ) {
			shape->outline( later->pos->outlineColor );
		}
		
		if( later && later->pos && !later->pos->lightFacingSurfaces.empty() ) {
			shape->setLightFacingSurfaces( &later->pos->lightFacingSurfaces );
		}
		shape->draw();


		// ==========================================================================
		// shapes
	} else {
		if ( later ) {
			bool sides[6];
			findOccludedSides( later, sides );
			shape->setOccludedSides( sides );
		}
		if ( later && later->pos ) {
			if ( later->pos->outlineColor && !map->useShadow ) {
				shape->outline( later->pos->outlineColor );
			}
			if ( shape->getTextureCount() > 3 ) {
				// select which alternate texture to use
				shape->setTextureIndex( later->pos->texIndex );
			}
		}
		
		if( later && later->pos && !later->pos->lightFacingSurfaces.empty() ) {
			shape->setLightFacingSurfaces( &later->pos->lightFacingSurfaces );
		}		
		shape->draw();
	}
	// ==========================================================================

#if DEBUG_MOUSE_POS == 1
	if ( shape && !useShadow && ( ( later && later->item ) || ( later && later->creature ) || shape->isInteractive() ) ) {
		glDisable( GL_DEPTH_TEST );
		glDepthMask( GL_FALSE );
		glDisable( GL_CULL_FACE );
		glDisable( GL_TEXTURE_2D );
		glColor4f( 1, 1, 1, 1 );
		glBegin( GL_LINE_LOOP );
		glVertex3f( 0, 0, 0 );
		glVertex3f( 0, shape->getDepth() * MUL, 0 );
		glVertex3f( shape->getWidth() * MUL, shape->getDepth() * MUL, 0 );
		glVertex3f( shape->getWidth() * MUL, 0, 0 );
		glEnd();
		glBegin( GL_LINE_LOOP );
		glVertex3f( 0, 0, shape->getHeight() * MUL );
		glVertex3f( 0, shape->getDepth() * MUL, shape->getHeight() * MUL );
		glVertex3f( shape->getWidth() * MUL, shape->getDepth() * MUL, shape->getHeight() * MUL );
		glVertex3f( shape->getWidth() * MUL, 0, shape->getHeight() * MUL );
		glEnd();
		glBegin( GL_LINES );
		glVertex3f( 0, 0, 0 );
		glVertex3f( 0, 0, shape->getHeight() * MUL );
		glEnd();
		glBegin( GL_LINES );
		glVertex3f( 0, shape->getDepth() * MUL, 0 );
		glVertex3f( 0, shape->getDepth() * MUL, shape->getHeight() * MUL );
		glEnd();
		glBegin( GL_LINES );
		glVertex3f( shape->getWidth() * MUL, shape->getDepth() * MUL, 0 );
		glVertex3f( shape->getWidth() * MUL, shape->getDepth() * MUL, shape->getHeight() * MUL );
		glEnd();
		glBegin( GL_LINES );
		glVertex3f( shape->getWidth() * MUL, 0, 0 );
		glVertex3f( shape->getWidth() * MUL, 0, shape->getHeight() * MUL );
		glEnd();
		glDepthMask( GL_TRUE );
		glEnable( GL_DEPTH_TEST );
	}
#endif

	// in the map editor outline virtual shapes
	if ( shape->isVirtual() && map->settings->isGridShowing() && map->gridEnabled ) {

		if ( heightPos > 1 ) {
			cerr << "heightPos=" << heightPos << " for virtual shape " << shape->getName() << endl;
		}
		if ( later && later->pos && later->pos->z > 0 ) {
			cerr << "z=" << later->pos->z << " for virtual shape " << shape->getName() << endl;
		}

		glColor4f( 0.75f, 0.75f, 1.0f, 1.0f );

		float z = ( shape->getHeight() + 0.25f ) * MUL;
		float lowZ = 0.25f * MUL;

		float wm = shape->getWidth() * MUL;
		float dm = shape->getDepth() * MUL;

		glPushMatrix();
		glTranslatef( 0.0f, 20.0f, z );
		map->adapter->texPrint( 0, 0, "virtual" );
		glPopMatrix();

		glDisable( GL_TEXTURE_2D );
		glBegin( GL_LINE_LOOP );
		glVertex3f( 0.0f, 0.0f, z );
		glVertex3f( 0.0f, dm, z );
		glVertex3f( wm, dm, z );
		glVertex3f( wm, 0.0f, z );

		glVertex3f( 0.0f, 0.0f, z );
		glVertex3f( 0.0f, 0.0f, lowZ );
		glVertex3f( wm, 0.0f, lowZ );
		glVertex3f( wm, 0.0f, z );

		glVertex3f( 0.0f, dm, z );
		glVertex3f( 0.0f, dm, lowZ );
		glVertex3f( wm, dm, lowZ );
		glVertex3f( wm, dm, z );

		glVertex3f( 0.0f, 0.0f, z );
		glVertex3f( 0.0f, 0.0f, lowZ );
		glVertex3f( 0.0f, dm, lowZ );
		glVertex3f( 0.0f, dm, z );

		glVertex3f( wm, 0.0f, z );
		glVertex3f( wm, 0.0f, lowZ );
		glVertex3f( wm, dm, lowZ );
		glVertex3f( wm, dm, z );

		glEnd();
		glEnable( GL_TEXTURE_2D );
	}

	glPopMatrix();

	// slow on mac os X
	// glPopAttrib();

	if ( shape ) {
		shape->setLightFacingSurfaces( NULL );
		( ( GLShape* )shape )->useShadow = false;
	}	
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


/// Determines which sides of a shape are not visible for various reasons.

void MapRender::findOccludedSides( DrawLater *later, bool *sides ) {
	if ( map->colorAlreadySet || !later || !later->pos || !later->shape || !later->shape->isStencil() || ( later->shape && map->isDoor( later->shape ) ) ) {
		sides[Shape::BOTTOM_SIDE] = sides[Shape::N_SIDE] = sides[Shape::S_SIDE] =
			sides[Shape::E_SIDE] = sides[Shape::W_SIDE] = sides[Shape::TOP_SIDE] = true;
		return;
	}

	sides[Shape::BOTTOM_SIDE] = sides[Shape::N_SIDE] =
		sides[Shape::S_SIDE] = sides[Shape::E_SIDE] =
			sides[Shape::W_SIDE] = sides[Shape::TOP_SIDE] = false;

	int x, y;
	Location *pos;
	for ( x = later->pos->x; x < later->pos->x + later->pos->shape->getWidth(); x++ ) {
		y = later->y - later->pos->shape->getDepth();
		pos = map->getLocation( x, y, later->pos->z );
		if ( !pos || !pos->shape->isStencil() || ( !map->isLocationInLight( x, y, pos->shape ) && !map->isDoorType( pos->shape ) ) ) {
			sides[Shape::N_SIDE] = true;
		}

		y = later->y + 1;
		pos = map->getLocation( x, y, later->pos->z );
		if ( !pos || !pos->shape->isStencil() || ( !map->isLocationInLight( x, y, pos->shape ) && !map->isDoorType( pos->shape ) ) ) {
			sides[Shape::S_SIDE] = true;
		}

		if ( sides[Shape::N_SIDE] && sides[Shape::S_SIDE] ) {
			break;
		}
	}


	for ( y = later->pos->y - later->pos->shape->getDepth() + 1; y <= later->pos->y; y++ ) {
		x = later->x - 1;
		pos = map->getLocation( x, y, later->pos->z );
		if ( !pos || !pos->shape->isStencil() || ( !map->isLocationInLight( x, y, pos->shape ) && !map->isDoorType( pos->shape ) ) ) {
			sides[Shape::W_SIDE] = true;
		}

		x = later->x + later->pos->shape->getWidth();
		pos = map->getLocation( x, y, later->pos->z );
		if ( !pos || !pos->shape->isStencil() || ( !map->isLocationInLight( x, y, pos->shape ) && !map->isDoorType( pos->shape ) ) ) {
			sides[Shape::E_SIDE] = true;
		}

		if ( sides[Shape::W_SIDE] && sides[Shape::E_SIDE] ) {
			break;
		}
	}


	for ( x = later->pos->x; x < later->pos->x + later->pos->shape->getWidth(); x++ ) {
		for ( y = later->pos->y - later->pos->shape->getDepth() + 1; !sides[Shape::TOP_SIDE] && y <= later->pos->y; y++ ) {
			pos = map->getLocation( x, y, later->pos->z + later->pos->shape->getHeight() );
			if ( !pos || !pos->shape->isStencil() || ( !map->isLocationInLight( x, y, pos->shape ) && !map->isDoorType( pos->shape ) ) ) {
				sides[Shape::TOP_SIDE] = true;
				break;
			}
		}
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
	float w, d, h;
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
