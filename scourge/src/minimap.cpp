/***************************************************************************
                     minimap.cpp  -  The ingame minimap
                             -------------------
    begin                : Thu Jan 29 2004
    copyright            : (C) 2004 by Daroth-U
    email                : daroth-u@ifrance.com
***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "common/constants.h"
#include "minimap.h"
#include "render/renderlib.h"
#include "sdlhandler.h"
#include "dungeongenerator.h"
#include "scourge.h"
#include "math.h"
#include "gui/canvas.h"
#include "item.h"
#include "creature.h"
#include "shapepalette.h"
#include "rpg/rpglib.h"

using namespace std;

#define DEBUG_MINIMAP 0
#define MINI_MAP_OFFSET_X 30
#define MINI_MAP_OFFSET_Y 50
#define MINI_MAP_SIZE 60
#define MINI_MAP_BLOCK 4

MiniMap :: MiniMap( Scourge *scourge ) {
	this->scourge = scourge;
	showMiniMap = true;
	textureSizeH = textureSizeW = 512;
	currentTravelMapAlpha = targetTravelMapAlpha = 1.0f;
	lastAlphaCheck = SDL_GetTicks();
}

MiniMap :: ~MiniMap() {
}

void MiniMap::drawMap() {
	if ( !showMiniMap ) return;

	bool useStencil =
	  ( scourge->getPreferences()->getStencilbuf() &&
	    scourge->getPreferences()->getStencilBufInitialized() );

	Map *map = scourge->getMap();

	bool outdoors = map->isHeightMapEnabled();

	// Onscreen pixel size of minimap
	int mmsize = MINI_MAP_SIZE * MINI_MAP_BLOCK;

	// Center coordinate of minimap
	int mmcenter = mmsize / 2;

	int sx = map->getX() + 75 - 30 - ( MINI_MAP_SIZE / 2 );
	if ( sx < 0 ) sx = 0;
	int sy = map->getY() + 75 - 30 - ( MINI_MAP_SIZE / 2 );
	if ( sy < 0 ) sy = 0;
	int ex = sx + MINI_MAP_SIZE;
	if ( ex > textureSizeW ) ex = textureSizeW;
	int ey = sy + MINI_MAP_SIZE;
	if ( ey > textureSizeH ) ey = textureSizeW;

	Creature *player;
	Location *playerPos;

	int monstersClose = false;
	int creatureCount = scourge->getSession()->getCreatureCount();

	// Check whether the party is on the map
	if ( scourge->getParty() && scourge->getParty()->getPartySize() ) {
		player = scourge->getSession()->getParty()->getPlayer();
		playerPos = map->getLocation( player->getX(), player->getY(), player->getZ() );

		if ( playerPos ) {

			// Check whether monsters are close (visible or not)
			for ( int i = 0; i < creatureCount ; i++ ) {
				Creature *creature = scourge->getSession()->getCreature( i ) ;

				if ( !creature->getStateMod( StateMod::dead ) && !creature->getStateMod( StateMod::possessed ) && creature->isMonster() ) {
					float dist = Constants::distance( player->getX() , player->getY(), player->getShape()->getWidth(), player->getShape()->getDepth(), creature->getX(), creature->getY(), creature->getShape()->getWidth(), creature->getShape()->getDepth() );

					if ( dist <= CREATURE_SIGHT_RADIUS ) {
						monstersClose = true;
						break;
					}

				}

			}

		}

	}

	glsDisable( GLS_CULL_FACE | GLS_DEPTH_TEST );
	glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );

	// Set rotation/translation for all following drawing operations.
	glPushMatrix();
	glLoadIdentity();
	glTranslatef( MINI_MAP_OFFSET_X, MINI_MAP_OFFSET_Y, 0.0f );
	glTranslatef( mmcenter, mmcenter, 0.0f );
	glRotatef( map->getZRot(), 0.0f, 0.0f, 1.0f );
	glTranslatef( -mmcenter, -mmcenter, 0.0f );

	// Create the stencil from the minimap mask texture.
	if ( useStencil ) {
		glClear( GL_STENCIL_BUFFER_BIT );
		glColorMask( 0, 0, 0, 0 );
		glsEnable( GLS_STENCIL_TEST | GLS_ALPHA_TEST | GLS_TEXTURE_2D );
		glStencilOp( GL_REPLACE, GL_REPLACE, GL_REPLACE );
		glStencilFunc( GL_ALWAYS, 1, 0xffffffff );
		glAlphaFunc( GL_NOTEQUAL, 0 );

		glPushMatrix();
		scourge->getShapePalette()->getMinimapMaskTexture().glBind();
		glColor4f( 1.0f, 1.0f, 1.0f, 1.0f );
		glBegin( GL_TRIANGLE_STRIP );
		glTexCoord2i( 0, 0 );
		glVertex2i( 0, 0 );
		glTexCoord2i( 1, 0 );
		glVertex2i( mmsize, 0 );
		glTexCoord2i( 0, 1 );
		glVertex2i( 0, mmsize );
		glTexCoord2i( 1, 1 );
		glVertex2i( mmsize, mmsize );
		glEnd();
		glsDisable( GLS_TEXTURE_2D | GLS_ALPHA_TEST );
		glPopMatrix();

		glColorMask( 1, 1, 1, 1 );
		glStencilFunc( GL_EQUAL, 1, 0xffffffff );
		glStencilOp( GL_KEEP, GL_KEEP, GL_KEEP );
		glsDisable( GLS_DEPTH_MASK );

		// Draw the transparent background.
		glPushMatrix();
		glsEnable( GLS_TEXTURE_2D | GLS_BLEND );
		scourge->getShapePalette()->getMinimapMaskTexture().glBind();
		glColor4f( 0.0f, 0.0f, 0.0f, 0.5f );
		glBegin( GL_TRIANGLE_STRIP );
		glTexCoord2i( 0, 0 );
		glVertex2i( 0, 0 );
		glTexCoord2i( 1, 0 );
		glVertex2i( mmsize, 0 );
		glTexCoord2i( 0, 1 );
		glVertex2i( 0, mmsize );
		glTexCoord2i( 1, 1 );
		glVertex2i( mmsize, mmsize );
		glEnd();
		glsDisable( GLS_TEXTURE_2D );
		glPopMatrix();
	} else {
		// Draw north marker and outline for the "simple" non-stencil version.
		glsEnable( GLS_BLEND );

		glColor4f( 0.5f, 0.5f, 0.5f, 0.5f );
		glBegin( GL_TRIANGLES );
		glVertex2i( 0, 0 );
		glVertex2i( 30, 0 );
		glVertex2i( 0, 30 );
		glEnd();
		glPushMatrix();
		glRotatef( -45.0f, 0.0f, 0.0f, 1.0f );
		glTranslatef( -7.0f, 20.0f, 0.0f );
		glScalef( 1.5f, 1.5f, 1.5f );
		glColor4f( 1.0f, 1.0f, 1.0f, 0.5f );
		scourge->getSDLHandler()->texPrint( 0, 0, "N" );
		glScalef( 1.0f, 1.0f, 1.0f );
		glPopMatrix();

		// outline
		glColor4f( 0.5f, 0.5f, 0.5f, 0.5f );
		glBegin( GL_LINE_LOOP );
		glVertex2i( 0, 0 );
		glVertex2i( 0, mmsize );
		glVertex2i( mmsize, mmsize );
		glVertex2i( mmsize, 0 );
		glEnd();
	}

	float targetAlpha = 1.0f;

	// Draw the surrounding objects into the map. Naive method: draw each block.
	for ( int x = sx; x < ex; x++ ) {
		if ( x < 0 || x >= MAP_WIDTH ) continue;

		for ( int y = sy; y < ey; y++ ) {
			if ( y < 0 || y >= MAP_DEPTH ) continue;

			Location *pos = map->getLocation( x, y, 0 );
			Location *floorPos = map->getItemLocation( x, y );

			int xp = ( x - sx ) * MINI_MAP_BLOCK;
			int yp = ( y - sy ) * MINI_MAP_BLOCK;

			bool visible = scourge->getSession()->getMap()->isLocationInLight( x, y, NULL );

			// Draw the floor (indoors) resp. roads (outdoors)
			// FIXME: The big non-path roads are not displayed correctly.
			if ( ( ( !outdoors && map->isOnFloorTile( x, y ) ) || ( outdoors && map->isRoad( x, y ) ) ) && visible ) {
				glColor4f( 1.0f, 1.0f, 1.0f, 0.2f );

				glBegin( GL_TRIANGLE_STRIP );
				glVertex2i( xp, yp );
				glVertex2i( xp + MINI_MAP_BLOCK, yp );
				glVertex2i( xp, yp + MINI_MAP_BLOCK );
				glVertex2i( xp + MINI_MAP_BLOCK, yp + MINI_MAP_BLOCK );
				glEnd();
			}

			// Draw items lying on the ground
			if ( floorPos && visible) {
				RenderedItem *item = floorPos->item;

				if ( item ) {
					if ( playerPos && ( map->distance( playerPos, floorPos ) <= CREATURE_SIGHT_RADIUS ) ) if ( targetAlpha > 0.4f ) targetAlpha = 0.4f;

					if ( item->isSpecial() ) {
						glColor4f( 1.0f, 0.0f, 0.5f, 0.8f );
					} else if ( item->isMagicItem() ) {
						glColor4f( 0.5f, 0.0f, 1.0f, 0.8f );
					} else {
						glColor4f( 0.5f, 0.0f, 1.0f, 0.5f );
					}

					glBegin( GL_TRIANGLE_STRIP );
					glVertex2i( xp, yp );
					glVertex2i( xp + MINI_MAP_BLOCK, yp );
					glVertex2i( xp, yp + MINI_MAP_BLOCK );
					glVertex2i( xp + MINI_MAP_BLOCK, yp + MINI_MAP_BLOCK );
					glEnd();

				}

			// Draw other objects
			} else if ( pos && visible ) {

			RenderedCreature *creature = pos->creature;
			Shape *shape = pos->shape;
			RenderedItem *item = pos->item;
			
				// Don't draw trees and scenery objects in the outdoors
				if ( shape && ( shape->getOutdoorWeight() == 0 ) ) {

					if ( !creature ) {

						// Draw items (mostly containers)
						if ( item ) {
							if ( playerPos && ( map->distance( playerPos, pos ) <= CREATURE_SIGHT_RADIUS ) ) if ( targetAlpha > 0.5f ) targetAlpha = 0.5f;

							if ( item->getContainsMagicItem() ) {
								glColor4f( 0.0f, 0.0f, 1.0f, 0.8f );
							} else {
								glColor4f( 0.0f, 0.0f, 1.0f, 0.5f );
							}

						// Draw shapes
						} else {

							// A wall or something
							if ( !shape->isInteractive() ) {
								if ( playerPos && ( map->distance( playerPos, pos ) <= CREATURE_SIGHT_RADIUS ) ) if ( targetAlpha > 0.6f ) targetAlpha = 0.6f;
								glColor4f( 1.0f, 1.0f, 1.0f, 0.5f );
							// A door or stairway
							} else {
								if ( playerPos && ( map->distance( playerPos, pos ) <= CREATURE_SIGHT_RADIUS ) ) if ( targetAlpha > 0.4f ) targetAlpha = 0.4f;
								glColor4f( 1.0f, 0.7f, 0.0f, 0.5f );
							}

						}

						glBegin( GL_TRIANGLE_STRIP );
						glVertex2i( xp, yp );
						glVertex2i( xp + MINI_MAP_BLOCK, yp );
						glVertex2i( xp, yp + MINI_MAP_BLOCK );
						glVertex2i( xp + MINI_MAP_BLOCK, yp + MINI_MAP_BLOCK );
						glEnd();

					// Draw creatures
					} else {

						// Monsters
						if ( creature->isMonster() ) {
							if ( playerPos && ( map->distance( playerPos, pos ) <= CREATURE_SIGHT_RADIUS ) ) if ( targetAlpha > 0.0f ) targetAlpha = 0.0f;

							if ( creature->isBoss() ) {
								glColor4f( 1.0f, 0.0f, 0.0f, 0.8f );
							} else {
								glColor4f( 1.0f, 0.0f, 0.0f, 0.5f );
							}

						// NPCs
						} else if ( creature->isNpc() ) {
							if ( playerPos && ( map->distance( playerPos, pos ) <= CREATURE_SIGHT_RADIUS ) ) if ( targetAlpha > 0.5f ) targetAlpha = 0.5f;
							glColor4f( 0.8f, 0.8f, 0.0f, 0.5f );

						// Characters
						} else {

							// Active PC
							if ( creature == scourge->getSession()->getParty()->getPlayer() ) {
								glColor4f( 0.0f, 1.0f, 0.0f, 0.5f );

							} else {

								// Wandering heroes
								glColor4f( 0.0f, 0.8f, 0.8f, 0.5f );
								// Other party members
								for ( int c = 0; c < scourge->getParty()->getPartySize(); c++ ) {

									if ( creature == scourge->getSession()->getParty()->getParty( c ) ) {
										glColor4f( 0.0f, 0.8f, 0.0f, 0.5f );
										break;
									}

								}

							}

						}

						int width = creature->getShape()->getWidth() / 2.0f * MINI_MAP_BLOCK;
						float cx =  ( creature->getX() - sx ) * MINI_MAP_BLOCK + width;
						float cy = ( creature->getY() - sy ) * MINI_MAP_BLOCK - width;

						glPushMatrix();
						glTranslatef( cx, cy, 0.0f );
						glRotatef( ( ( AnimatedShape* )creature->getShape() )->getAngle(), 0.0f, 0.0f, 1.0f );
						glBegin( GL_TRIANGLES );
						glVertex2i( width, width );
						glVertex2i( -width, width );
						glVertex2i( 0, -width );
						glEnd();
						glPopMatrix();

					}

				}

			}

		}

	}

	glsEnable( GLS_TEXTURE_2D );

	// Draw the travel map

	if ( outdoors ) {

		// Calculate a few coordinates first

		// The region we are in
		int rx = map->getRegionX();
		int ry = map->getRegionY();

		// The center bitmap we need
		int bx = rx / REGIONS_PER_BITMAP;
		int by = ry / REGIONS_PER_BITMAP;

		// Region at top left of bitmap
		int rleft = bx * REGIONS_PER_BITMAP;
		int rtop = by * REGIONS_PER_BITMAP;

		// Offset of our region into the center bitmap
		int roffsx = ( rx - rleft ) * REGION_SIZE;
		int roffsy = ( ry - rtop ) * REGION_SIZE;

		// Our position within the region
		int px = ( map->getMapX() / (float)MAP_WIDTH ) * (float)REGION_SIZE;
		int py = ( map->getMapY() / (float)MAP_DEPTH ) * (float)REGION_SIZE;

		// Where to draw the center bitmap
		int x = mmcenter - roffsx - px;
		int y = mmcenter - roffsy - py;
		
		// Variables for the bitmaps to draw
		int bitmapX, bitmapY;

		// Calculate alpha for the superimposed travel map

		#define CHANGE_PER_SECOND 0.1f

		targetTravelMapAlpha = targetAlpha;

		Uint32 now = SDL_GetTicks();
		Uint32 elapsed = now - lastAlphaCheck;
		lastAlphaCheck = now;

		float oldAlpha = currentTravelMapAlpha;
		currentTravelMapAlpha += signum( (float)(targetTravelMapAlpha - currentTravelMapAlpha) ) * ( (float)elapsed / 1000 * CHANGE_PER_SECOND );

		// Do some clipping so the alpha doesn't oscillate around the target value
		if ( ( ( oldAlpha < targetTravelMapAlpha ) && ( currentTravelMapAlpha > targetTravelMapAlpha ) ) || ( ( oldAlpha > targetTravelMapAlpha ) && ( currentTravelMapAlpha < targetTravelMapAlpha ) ) ) currentTravelMapAlpha = targetTravelMapAlpha;

		// Start drawing
		glColor4f( 1.0f, 1.0f, 1.0f, currentTravelMapAlpha );
		
		for ( int ty = -1; ty < 2; ty++ ) {
			for ( int tx = -1; tx < 2; tx++ ) {

				if ( ( ( bx + tx ) > -1 ) && ( ( bx + tx ) < BITMAPS_PER_ROW ) && ( ( by + ty ) > -1 ) && ( ( by + ty ) < BITMAPS_PER_COL ) ) {
					bitmapX = bx + tx;
					bitmapY = by + ty;
				} else {
					// Map edge? Just use the water tile from 0,0 :-)
					bitmapX = bitmapY = 0;
				}

				glPushMatrix();
				scourge->getShapePalette()->travelMap[bitmapX][bitmapY].glBind();
				glTranslatef( x + ( tx * BITMAP_SIZE ), y + ( ty * BITMAP_SIZE ), 0 );
				glBegin( GL_TRIANGLE_STRIP );
				glTexCoord2i( 0, 0 );
				glVertex2i( 0, 0 );
				glTexCoord2i( 1, 0 );
				glVertex2i( BITMAP_SIZE, 0 );
				glTexCoord2i( 0, 1 );
				glVertex2i( 0, BITMAP_SIZE );
				glTexCoord2i( 1, 1 );
				glVertex2i( BITMAP_SIZE, BITMAP_SIZE );
				glEnd();
				glPopMatrix();

			}

		}

	}

	// Draw the minimap frame.
	if ( useStencil ) {
		glsDisable( GLS_STENCIL_TEST );
		glsEnable( GLS_DEPTH_MASK | GLS_ALPHA_TEST );
		glAlphaFunc( GL_ALWAYS, 0 );

		glPushMatrix();
		!monstersClose ? scourge->getShapePalette()->getMinimapTexture().glBind() : scourge->getShapePalette()->getMinimap2Texture().glBind();
		glColor4f( 1.0f, 1.0f, 1.0f, 1.0f );
		glBegin( GL_TRIANGLE_STRIP );
		glTexCoord2i( 0, 0 );
		glVertex2i( 0, 0 );
		glTexCoord2i( 1, 0 );
		glVertex2i( mmsize, 0 );
		glTexCoord2i( 0, 1 );
		glVertex2i( 0, mmsize );
		glTexCoord2i( 1, 1 );
		glVertex2i( mmsize, mmsize );
		glEnd();
		glsDisable( GLS_TEXTURE_2D | GLS_ALPHA_TEST );
		glPopMatrix();

		// draw pointers for gates and teleporters
		if ( scourge->getParty() && scourge->getParty()->getPartySize() ) {
			drawPointers( map->getGates(), Color( 1.0f, 0.0f, 0.0f, 1.0f ) );
			drawPointers( map->getTeleporters(), Color( 0.0f, 0.0f, 1.0f, 1.0f ) );
		}
	}

	// Draw the crosshair marking player's position

	if ( outdoors && player ) {
		// Offset of the crosshair from minimap center in regard to player vs. camera location
		int ox = ( ( player->getX() - map->getMapX() ) / (float)MAP_WIDTH ) * (float)REGION_SIZE;
		int oy = ( ( player->getY() - map->getMapY() ) / (float)MAP_DEPTH ) * (float)REGION_SIZE;

		glColor4f( 1.0f, 0.0f, 0.0f, currentTravelMapAlpha );
		glTranslatef( mmcenter + ox, mmcenter + oy, 0.0f );
		glRotatef( -map->getZRot(), 0.0f, 0.0f, 1.0f );

		glBegin( GL_LINES );
		glVertex2i( -3, -3);
		glVertex2i( 3, 3 );
		glVertex2i( -3, 3 );
		glVertex2i( 3, -3 );
		glEnd();

		glColor4f( 1.0f, 1.0f, 1.0f, 1.0f );
	}

	glPopMatrix();

	glsDisable( GLS_BLEND );
	glsEnable( GLS_CULL_FACE | GLS_DEPTH_TEST | GLS_TEXTURE_2D );
}

void MiniMap::drawPointers( std::set<Location*> *p, Color color ) {
	// player's pos
	float px = scourge->getParty()->getPlayer()->getX();
	float py = scourge->getParty()->getPlayer()->getY();

	// center coord. of minimap
	float r = MINI_MAP_SIZE * MINI_MAP_BLOCK / 2;

	for ( set<Location*>::iterator e = p->begin(); e != p->end(); ++e ) {
		Location *pos = *e;
		float angle = Util::getAngle( px, py, 0.0f, 0.0f, ( float )pos->x, ( float )pos->y, 0.0f, 0.0f );
		float nx = r + ( r - 10 ) * Constants::cosFromAngle( angle ) - 5;
		float ny = r + ( r - 10 ) * Constants::sinFromAngle( angle );

		glColor4f( color.r, color.g, color.b, color.a );

		glBegin( GL_TRIANGLE_STRIP );
		glVertex2f( nx, ny );
		glVertex2f( nx + 4.0f, ny );
		glVertex2f( nx, ny + 4.0f );
		glVertex2f( nx + 4.0f, ny + 4.0f );
		glEnd();

		glColor4f( 0.0f, 0.0f, 0.0f, 1.0f );

		glBegin( GL_LINE_LOOP );
		glVertex2f( nx - 1.0f, ny + 6.0f );
		glVertex2f( nx + 6.0f, ny + 6.0f );
		glVertex2f( nx + 6.0f, ny - 1.0f );
		glVertex2f( nx - 1.0f, ny - 1.0f );
		glEnd();
	}

	glColor4f( 1.0f, 1.0f, 1.0f, 1.0f );
}
