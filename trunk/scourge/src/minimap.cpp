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
}

MiniMap :: ~MiniMap() {
}

void MiniMap::drawMap() {
	if ( !showMiniMap ) return;

	bool useStencil =
	  ( scourge->getPreferences()->getStencilbuf() &&
	    scourge->getPreferences()->getStencilBufInitialized() );

	int sx = scourge->getSession()->getMap()->getX() + 75 - 30 - ( MINI_MAP_SIZE / 2 );
	if ( sx < 0 ) sx = 0;
	int sy = scourge->getSession()->getMap()->getY() + 75 - 30 - ( MINI_MAP_SIZE / 2 );
	if ( sy < 0 ) sy = 0;
	int ex = sx + MINI_MAP_SIZE;
	if ( ex > textureSizeW ) ex = textureSizeW;
	int ey = sy + MINI_MAP_SIZE;
	if ( ey > textureSizeH ) ey = textureSizeW;

	glDisable( GL_CULL_FACE );
	glDisable( GL_DEPTH_TEST );
	glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );

	// Set rotation/translation for all following drawing operations.
	glPushMatrix();
	glLoadIdentity();
	glTranslatef( MINI_MAP_OFFSET_X, MINI_MAP_OFFSET_Y, 0 );
	glTranslatef( MINI_MAP_SIZE * MINI_MAP_BLOCK / 2, MINI_MAP_SIZE * MINI_MAP_BLOCK / 2, 0 );
	glRotatef( scourge->getSession()->getMap()->getZRot(), 0, 0, 1 );
	glTranslatef( -MINI_MAP_SIZE * MINI_MAP_BLOCK / 2, -MINI_MAP_SIZE * MINI_MAP_BLOCK / 2, 0 );

	// Create the stencil from the minimap mask texture.
	if ( useStencil ) {
		glClear( GL_STENCIL_BUFFER_BIT );
		glColorMask( 0, 0, 0, 0 );
		glEnable( GL_STENCIL_TEST );
		glStencilOp( GL_REPLACE, GL_REPLACE, GL_REPLACE );
		glStencilFunc( GL_ALWAYS, 1, 0xffffffff );

		glPushMatrix();
		glDisable( GL_BLEND );
		glEnable( GL_ALPHA_TEST );
		//glAlphaFunc( GL_EQUAL, 0xff );
		glAlphaFunc( GL_NOTEQUAL, 0 );
		glEnable( GL_TEXTURE_2D );
		scourge->getShapePalette()->getMinimapMaskTexture().glBind();
		glColor4f( 1.0f, 1.0f, 1.0f, 1.0f );
		glBegin( GL_TRIANGLE_STRIP );
		glTexCoord2i( 0, 0 );
		glVertex2i( 0, 0 );
		glTexCoord2i( 1, 0 );
		glVertex2i( MINI_MAP_SIZE * MINI_MAP_BLOCK, 0 );
		glTexCoord2i( 0, 1 );
		glVertex2i( 0, MINI_MAP_SIZE * MINI_MAP_BLOCK );
		glTexCoord2i( 1, 1 );
		glVertex2i( MINI_MAP_SIZE * MINI_MAP_BLOCK, MINI_MAP_SIZE * MINI_MAP_BLOCK );
		glEnd();
		glDisable( GL_TEXTURE_2D );
		glDisable( GL_ALPHA_TEST );
		glPopMatrix();

		glColorMask( 1, 1, 1, 1 );
		glStencilFunc( GL_EQUAL, 1, 0xffffffff );
		glStencilOp( GL_KEEP, GL_KEEP, GL_KEEP );
		glDepthMask( GL_FALSE );

		// Draw the transparent background.
		glPushMatrix();
		glEnable( GL_BLEND );
		glEnable( GL_TEXTURE_2D );
		scourge->getShapePalette()->getMinimapMaskTexture().glBind();
		glColor4f( 0.0f, 0.0f, 0.0f, 0.5f );
		glBegin( GL_TRIANGLE_STRIP );
		glTexCoord2i( 0, 0 );
		glVertex2i( 0, 0 );
		glTexCoord2i( 1, 0 );
		glVertex2i( MINI_MAP_SIZE * MINI_MAP_BLOCK, 0 );
		glTexCoord2i( 0, 1 );
		glVertex2i( 0, MINI_MAP_SIZE * MINI_MAP_BLOCK );
		glTexCoord2i( 1, 1 );
		glVertex2i( MINI_MAP_SIZE * MINI_MAP_BLOCK, MINI_MAP_SIZE * MINI_MAP_BLOCK );
		glEnd();
		glDisable( GL_TEXTURE_2D );
		glPopMatrix();
	} else {
		// Draw north marker and outline for the "simple" non-stencil version.
		glEnable( GL_BLEND );

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
		glVertex2i( 0, MINI_MAP_SIZE * MINI_MAP_BLOCK );
		glVertex2i( MINI_MAP_SIZE * MINI_MAP_BLOCK, MINI_MAP_SIZE * MINI_MAP_BLOCK );
		glVertex2i( MINI_MAP_SIZE * MINI_MAP_BLOCK, 0 );
		glEnd();
	}

	// Draw the surrounding objects into the map. Naive method: draw each block.
	for ( int x = sx; x < ex; x++ ) {
		if ( x < 0 || x >= MAP_WIDTH ) continue;

		for ( int y = sy; y < ey; y++ ) {
			if ( y < 0 || y >= MAP_DEPTH ) continue;

			Location *pos = scourge->getSession()->getMap()->getLocation( x, y, 0 );
			Location *floorPos = scourge->getSession()->getMap()->getItemLocation( x, y );

			int xp = ( x - sx ) * MINI_MAP_BLOCK;
			int yp = ( y - sy ) * MINI_MAP_BLOCK;

			// Draw the floor (indoors)
			if ( !scourge->getSession()->getMap()->isHeightMapEnabled() && scourge->getSession()->getMap()->isOnFloorTile( x, y ) ) {
				glColor4f( 1.0f, 1.0f, 1.0f, 0.2f );

				glBegin( GL_TRIANGLE_STRIP );
				glVertex2i( xp, yp );
				glVertex2i( xp + MINI_MAP_BLOCK, yp );
				glVertex2i( xp, yp + MINI_MAP_BLOCK );
				glVertex2i( xp + MINI_MAP_BLOCK, yp + MINI_MAP_BLOCK );
				glEnd();
			}

			// Draw items laying on the ground
			if ( floorPos ) {
				RenderedItem *item = floorPos->item;

				if ( item ) {

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

			}

			// Draw other objects
			if ( pos ) {

			RenderedCreature *creature = pos->creature;
			Shape *shape = pos->shape;
			RenderedItem *item = pos->item;
			
				// Don't draw trees and scenery objects in the outdoors
				if ( shape && ( shape->getOutdoorWeight() == 0 ) ) {

					if ( !creature ) {

						// Draw items
						if ( item ) {

							if ( item->getContainsMagicItem() ) {
								glColor4f( 0.0f, 0.0f, 1.0f, 0.8f );
							} else {
								glColor4f( 0.0f, 0.0f, 1.0f, 0.5f );
							}

						// Draw shapes
						} else {

							// A wall or something
							if ( !shape->isInteractive() ) {
								glColor4f( 1.0f, 1.0f, 1.0f, 0.5f );
							// A door or stairway
							} else {
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

							if ( creature->isBoss() ) {
								glColor4f( 1.0f, 0.0f, 0.0f, 0.8f );
							} else {
								glColor4f( 1.0f, 0.0f, 0.0f, 0.5f );
							}

						// NPCs
						} else if ( creature->isNpc() ) {
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

	// Draw the minimap frame.
	if ( useStencil ) {
		glDepthMask( GL_TRUE );
		glDisable( GL_STENCIL_TEST );

		glPushMatrix();
		glEnable( GL_ALPHA_TEST );
		glAlphaFunc( GL_ALWAYS, 0 );
		glEnable( GL_TEXTURE_2D );
		scourge->getShapePalette()->getMinimapTexture().glBind();
		glColor4f( 1, 1, 1, 1 );
		glBegin( GL_TRIANGLE_STRIP );
		glTexCoord2i( 0, 0 );
		glVertex2i( 0, 0 );
		glTexCoord2i( 1, 0 );
		glVertex2i( MINI_MAP_SIZE * MINI_MAP_BLOCK, 0 );
		glTexCoord2i( 0, 1 );
		glVertex2i( 0, MINI_MAP_SIZE * MINI_MAP_BLOCK );
		glTexCoord2i( 1, 1 );
		glVertex2i( MINI_MAP_SIZE * MINI_MAP_BLOCK, MINI_MAP_SIZE * MINI_MAP_BLOCK );
		glEnd();
		glDisable( GL_TEXTURE_2D );
		glDisable( GL_ALPHA_TEST );
		glPopMatrix();

		// draw pointers for gates and teleporters
		if ( scourge->getParty() && scourge->getParty()->getPartySize() ) {
			drawPointers( scourge->getSession()->getMap()->getGates(), Color( 1, 0, 0, 1 ) );
			drawPointers( scourge->getSession()->getMap()->getTeleporters(), Color( 0, 0, 1, 1 ) );
		}
	}

	glPopMatrix();
	glDisable( GL_BLEND );
	glEnable( GL_CULL_FACE );
	glEnable( GL_DEPTH_TEST );
	glEnable( GL_TEXTURE_2D );
}

void MiniMap::drawPointers( std::set<Location*> *p, Color color ) {
	// player's pos
	float px = scourge->getParty()->getPlayer()->getX();
	float py = scourge->getParty()->getPlayer()->getY();

	// center coord. of minimap
	float r = MINI_MAP_SIZE * MINI_MAP_BLOCK / 2;

	for ( set<Location*>::iterator e = p->begin(); e != p->end(); ++e ) {
		Location *pos = *e;
		float angle = Util::getAngle( px, py, 0, 0, ( float )pos->x, ( float )pos->y, 0, 0 );
		float nx = r + ( r - 10 ) * Constants::cosFromAngle( angle ) - 5;
		float ny = r + ( r - 10 ) * Constants::sinFromAngle( angle );
		glColor4f( color.r, color.g, color.b, color.a );
		glBegin( GL_TRIANGLE_STRIP );
		glVertex2f( nx, ny );
		glVertex2f( nx + 4, ny );
		glVertex2f( nx, ny + 4 );
		glVertex2f( nx + 4, ny + 4 );
		glEnd();
		glColor4f( 0, 0, 0, 1 );
		glBegin( GL_LINE_LOOP );
		glVertex2f( nx - 1, ny + 6 );
		glVertex2f( nx + 6, ny + 6 );
		glVertex2f( nx + 6, ny - 1 );
		glVertex2f( nx - 1, ny - 1 );
		glEnd();
	}
	glColor4f( 1, 1, 1, 1 );
}
