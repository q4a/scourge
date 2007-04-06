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
#include "pcui.h"

using namespace std;

/**
  *@author Gabor Torok
  */

Equip::Equip( PcUi *pcUi, int x, int y, int w, int h ) {
	this->pcUi = pcUi;
	this->creature = NULL;
	this->backgroundTexture = pcUi->getScourge()->getShapePalette()->getNamedTexture( "equip" );
	this->scrollTexture = pcUi->getScourge()->getShapePalette()->getNamedTexture( "scroll" );
  this->currentHole = -1;
	this->x = x;
	this->y = y;
	this->w = w;
	this->h = h;
	this->lastItem = NULL;
	this->mode = EQUIP_MODE;

	canvas = new Canvas( x, y, x + w, y + h, this, this);
	canvas->setDrawBorders( false );
}

Equip::~Equip() {
}

bool Equip::handleEvent(Widget *widget, SDL_Event *event) {
	if( pcUi->getScourge()->getSDLHandler()->isDoubleClick ) {
		if( mode == EQUIP_MODE ) {
			Item *item = getItemAtPos( pcUi->getScourge()->getSDLHandler()->mouseX - pcUi->getWindow()->getX() - x, 
																 pcUi->getScourge()->getSDLHandler()->mouseY - pcUi->getWindow()->getY() - y - TITLE_HEIGHT );
			if( item && item->getRpgItem()->isContainer() ) {
				pcUi->getScourge()->openContainerGui(item);
			}
		}
	} else if( pcUi->getScourge()->getSDLHandler()->mouseButton == SDL_BUTTON_RIGHT ) {
		if( mode == EQUIP_MODE ) {
			Item *item = getItemAtPos( pcUi->getScourge()->getSDLHandler()->mouseX - pcUi->getWindow()->getX() - x, 
																 pcUi->getScourge()->getSDLHandler()->mouseY - pcUi->getWindow()->getY() - y - TITLE_HEIGHT );
			if( item ) {
				pcUi->getScourge()->getInfoGui()->setItem( item );
				if( !pcUi->getScourge()->getInfoGui()->getWindow()->isVisible() ) 
					pcUi->getScourge()->getInfoGui()->getWindow()->setVisible( true );
			}
		}
	}
  return false;
}

bool Equip::handleEvent(SDL_Event *event) {
	Item *item;
	char tooltip[ 500 ];
  switch(event->type) {
	case SDL_MOUSEMOTION:
		if( mode == EQUIP_MODE ) {
			currentHole = getHoleAtPos( event->motion.x - pcUi->getWindow()->getX() - x, 
																	event->motion.y - pcUi->getWindow()->getY() - TITLE_HEIGHT );
	
			item = getItemInHole( currentHole );
			if( item != lastItem ) {
				lastItem = item;
				if( item ) {
					item->getTooltip( tooltip );
					canvas->setTooltip( tooltip );
				} else {
					canvas->setTooltip( NULL );
				}
			}
		}

    break;
  case SDL_MOUSEBUTTONUP:
    break;     
	default: break;
	}
  return false;
}

Item *Equip::getItemInHole( int hole ) {
	return( hole > -1 && creature ? creature->getEquippedInventory( hole ) : NULL );
}

Item *Equip::getItemAtPos( int x, int y ) {
  int hole = getHoleAtPos( x, y );
  return getItemInHole( hole );
}

int Equip::getHoleAtPos( int x, int y ) {
  SDL_Rect point;
  point.x = x;
  point.y = y;
  point.w = point.h = 1;
  for( int i = 0; i < Constants::INVENTORY_COUNT; i++ ) {
    SDL_Rect *rect = pcUi->getScourge()->getShapePalette()->getInventoryHole( i );
    if( SDLHandler::intersects( rect, &point ) ) {
      return i;
    }
  }
  return -1;
}

void Equip::receive( Widget *widget ) {
	if( mode == EQUIP_MODE && creature ) {
		Item *item = pcUi->getScourge()->getMovingItem();
		if( item ) {
			char *err = creature->canEquipItem( item );
			if( err ) {
				pcUi->getScourge()->showMessageDialog( err );
				pcUi->getScourge()->getSDLHandler()->getSound()->playSound( Window::DROP_FAILED );
			} else {
				if( creature->addInventory( item ) ) {
					// message: the player accepted the item
					char message[120];
					snprintf( message, 119, _( "%s picks up %s." ), 
									 creature->getName(),
									 item->getItemName() );
					pcUi->getScourge()->getMap()->addDescription( message );
					pcUi->getScourge()->endItemDrag();
					int index = creature->findInInventory( item );
					creature->equipInventory( index, currentHole );
					pcUi->getScourge()->getSDLHandler()->getSound()->playSound( Window::DROP_SUCCESS );
				} else {
					// message: the player's inventory is full
					pcUi->getScourge()->getSDLHandler()->getSound()->playSound( Window::DROP_FAILED );
					pcUi->getScourge()->showMessageDialog( _( "You can't fit the item!" ) );
				}
			}
		}
	}
}

bool Equip::startDrag( Widget *widget, int x, int y ) {
	if( mode == EQUIP_MODE && creature ) {
	
		if( pcUi->getScourge()->getTradeDialog()->getWindow()->isVisible() ) {
			pcUi->getScourge()->showMessageDialog( _( "Can't change inventory while trading." ) );
			return false;
		}
		
		// what's equiped at this inventory slot?
		currentHole = getHoleAtPos( x, y );
		Item *item = getItemAtPos( x, y );
		if( item ) {
			if( item->isCursed() ) {
				pcUi->getScourge()->showMessageDialog( _( "Can't remove cursed item!" ) );
				return false;
			} else {
	
				creature->removeInventory( creature->findInInventory( item ) );
				pcUi->getScourge()->startItemDragFromGui( item );
				char message[120];
				snprintf(message, 119, _( "%s drops %s." ), 
								creature->getName(),
								item->getItemName() );
				pcUi->getScourge()->getMap()->addDescription( message );
	
				return true;
			}
		}
	}
  return false;
}

void Equip::setCreature( Creature *creature ) {
	this->creature = creature;
}

void Equip::drawWidgetContents( Widget *widget ) {
	glEnable( GL_TEXTURE_2D );
	glEnable( GL_ALPHA_TEST );
	glAlphaFunc( GL_NOTEQUAL, 0 );
	if( mode == EQUIP_MODE ) {
		glBindTexture( GL_TEXTURE_2D, backgroundTexture );
	} else {
		glBindTexture( GL_TEXTURE_2D, scrollTexture );
	}
	glColor4f( 1, 1, 1, 1 );
	glBegin( GL_QUADS );
	glTexCoord2d( 0, 1 );
	glVertex2d( 0, h );
	glTexCoord2d( 0, 0 );
	glVertex2d( 0, 0 );
	glTexCoord2d( 1, 0 );
	glVertex2d( w, 0 );
	glTexCoord2d( 1, 1 );
	glVertex2d( w, h );
	glEnd();
	glDisable( GL_ALPHA_TEST );
	if( creature ) {
		if( mode == EQUIP_MODE ) {
			for( int i = 0; i < Constants::INVENTORY_COUNT; i++ ) {
				Item *item = creature->getEquippedInventory( i );
				if( item ) {
					SDL_Rect *rect = pcUi->getScourge()->getShapePalette()->getInventoryHole( i );
					if( rect && rect->w && rect->h ) {
						item->renderIcon( pcUi->getScourge(), rect->x, rect->y, rect->w, rect->h );
					}
				}
			}
		}
	}

  if( mode == EQUIP_MODE && currentHole > -1 ) {
    SDL_Rect *rect = pcUi->getScourge()->getShapePalette()->getInventoryHole( currentHole );
    glDisable( GL_TEXTURE_2D );
    pcUi->getWindow()->setTopWindowBorderColor();
    glBegin( GL_LINE_LOOP );
    glVertex2d( rect->x, rect->y + rect->h );
    glVertex2d( rect->x, rect->y );
    glVertex2d( rect->x + rect->w, rect->y );
    glVertex2d( rect->x + rect->w, rect->y + rect->h );
    glEnd();
  }
}

