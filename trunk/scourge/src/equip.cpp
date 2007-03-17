/***************************************************************************
                          inventory.cpp  -  description
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

#include "equip.h"
#include "render/renderlib.h"
#include "rpg/rpglib.h"
#include "sound.h"
#include "sdlhandler.h"
#include "sdleventhandler.h"
#include "sdlscreenview.h"
#include "scourge.h"
#include "item.h"
#include "creature.h"
#include "tradedialog.h"
#include "sqbinding/sqbinding.h"
#include "characterinfo.h"
#include "shapepalette.h"
#include "skillsview.h"
#include "gui/confirmdialog.h"

using namespace std;

/**
  *@author Gabor Torok
  */

Equip::Equip( Scourge *scourge, Window *window ) {
	this->scourge = scourge;
	this->creature = NULL;
	this->backgroundTexture = scourge->getShapePalette()->getNamedTexture( "equip" );
  this->currentHole = -1;
	this->window = window;

	canvas = new Canvas( CANVAS_OFFSET_X, 0, EQUIP_WIDTH + CANVAS_OFFSET_X + 2, EQUIP_HEIGHT + 2, this, this);
	canvas->setDrawBorders( false );
}

Equip::~Equip() {
}

bool Equip::handleEvent(Widget *widget, SDL_Event *event) {
  return false;
}

bool Equip::handleEvent(SDL_Event *event) {
  switch(event->type) {
  case SDL_MOUSEMOTION:
    currentHole = getHoleAtPos( event->motion.x - window->getX() - CANVAS_OFFSET_X, 
                                event->motion.y - window->getY() - CANVAS_OFFSET_Y );
    break;
  case SDL_MOUSEBUTTONUP:
    break;     
	default: break;
	}
  return false;
}

Item *Equip::getItemAtPos( int x, int y ) {
  int hole = getHoleAtPos( x, y );
  return( hole > -1 && creature ? creature->getEquippedInventory( hole ) : NULL );
}

int Equip::getHoleAtPos( int x, int y ) {
  SDL_Rect point;
  point.x = x;
  point.y = y;
  point.w = point.h = 1;
  for( int i = 0; i < Constants::INVENTORY_COUNT; i++ ) {
    SDL_Rect *rect = scourge->getShapePalette()->getInventoryHole( i );
    if( SDLHandler::intersects( rect, &point ) ) {
      return i;
    }
  }
  return -1;
}

void Equip::receive( Widget *widget ) {
	if( creature ) {
		Item *item = scourge->getMovingItem();
		if( item ) {
			char *err = creature->canEquipItem( item );
			if( err ) {
				scourge->showMessageDialog( err );
				scourge->getSDLHandler()->getSound()->playSound( Window::DROP_FAILED );
			} else {
				if( creature->addInventory( item ) ) {
					// message: the player accepted the item
					char message[120];
					snprintf( message, 119, _( "%s picks up %s." ), 
									 creature->getName(),
									 item->getItemName() );
					scourge->getMap()->addDescription( message );
					scourge->endItemDrag();
					int index = creature->findInInventory( item );
					creature->equipInventory( index, currentHole );
					scourge->getSDLHandler()->getSound()->playSound( Window::DROP_SUCCESS );
				} else {
					// message: the player's inventory is full
					scourge->getSDLHandler()->getSound()->playSound( Window::DROP_FAILED );
					scourge->showMessageDialog( _( "You can't fit the item!" ) );
				}
			}
		}
	}
}

bool Equip::startDrag( Widget *widget, int x, int y ) {
	if( creature ) {
	
		if( scourge->getTradeDialog()->getWindow()->isVisible() ) {
			scourge->showMessageDialog( _( "Can't change inventory while trading." ) );
			return false;
		}
		
		// what's equiped at this inventory slot?
		currentHole = getHoleAtPos( x, y );
		Item *item = getItemAtPos( x, y );
		if( item ) {
			if( item->isCursed() ) {
				scourge->showMessageDialog( _( "Can't remove cursed item!" ) );
				return false;
			} else {
	
				creature->removeInventory( creature->findInInventory( item ) );
				scourge->startItemDragFromGui( item );
				char message[120];
				snprintf(message, 119, _( "%s drops %s." ), 
								creature->getName(),
								item->getItemName() );
				scourge->getMap()->addDescription( message );
	
				return true;
			}
		}
	}
  return false;
}

void Equip::setCreature( Creature *creature ) {
	this->creature = creature;
}

void Equip::drawWidgetContents( Widget *w ) {
	glEnable( GL_TEXTURE_2D );
	glBindTexture( GL_TEXTURE_2D, backgroundTexture );
  glPushMatrix();
  glTranslatef( 1, 1, 0 );
	glBegin( GL_QUADS );
	glTexCoord2d( 0, 1 );
	glVertex2d( 0, EQUIP_HEIGHT );
	glTexCoord2d( 0, 0 );
	glVertex2d( 0, 0 );
	glTexCoord2d( 1, 0 );
	glVertex2d( EQUIP_WIDTH, 0 );
	glTexCoord2d( 1, 1 );
	glVertex2d( EQUIP_WIDTH, EQUIP_HEIGHT );
	glEnd();
  glPopMatrix();
	if( creature ) {
		for( int i = 0; i < Constants::INVENTORY_COUNT; i++ ) {
			Item *item = creature->getEquippedInventory( i );
      if( item ) {
				SDL_Rect *rect = scourge->getShapePalette()->getInventoryHole( i );
				if( rect && rect->w && rect->h ) {
					GLuint tex = scourge->getShapePalette()->
						tilesTex[ item->getRpgItem()->getIconTileX() ][ item->getRpgItem()->getIconTileY() ];
					glEnable( GL_ALPHA_TEST );
					glAlphaFunc( GL_NOTEQUAL, 0 );
					glBindTexture( GL_TEXTURE_2D, tex );
					glBegin( GL_QUADS );
					glTexCoord2d( 0, 1 );
					glVertex2d( rect->x, rect->y + rect->h );
					glTexCoord2d( 0, 0 );
					glVertex2d( rect->x, rect->y );
					glTexCoord2d( 1, 0 );
					glVertex2d( rect->x + rect->w, rect->y );
					glTexCoord2d( 1, 1 );
					glVertex2d( rect->x + rect->w, rect->y + rect->h );
					glEnd();
				}
			}
		}
	}

  if( currentHole > -1 ) {
    SDL_Rect *rect = scourge->getShapePalette()->getInventoryHole( currentHole );
    glDisable( GL_TEXTURE_2D );
    window->setTopWindowBorderColor();
    glBegin( GL_LINE_LOOP );
    glVertex2d( rect->x, rect->y + rect->h );
    glVertex2d( rect->x, rect->y );
    glVertex2d( rect->x + rect->w, rect->y );
    glVertex2d( rect->x + rect->w, rect->y + rect->h );
    glEnd();
  }
}

