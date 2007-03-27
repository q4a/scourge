/***************************************************************************
                          inven.cpp  -  description
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
#include "inven.h"
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

/**
  *@author Gabor Torok
  */

using namespace std;

#define GRID_SIZE 30
#define OFFSET_X 5
#define OFFSET_Y 5

Inven::Inven( PcUi *pcUi, int x, int y, int w, int h ) {
	this->pcUi = pcUi;
	this->creature = NULL;
	this->backgroundTexture = pcUi->getScourge()->getShapePalette()->getNamedTexture( "inven" );
  this->currentHole = -1;
	this->x = x;
	this->y = y;
	this->w = w;
	this->h = h;
	this->lastItem = NULL;

	canvas = new Canvas( x, y, x + w, y + h, this, this);
	canvas->setDrawBorders( false );
}

Inven::~Inven() {
}

bool Inven::handleEvent( SDL_Event *event ) {
	if( event->type == SDL_MOUSEMOTION ) {
		Item *item = getItemAtPos( event->motion.x - pcUi->getWindow()->getX() - x, 
															 event->motion.y - pcUi->getWindow()->getY() - y - TITLE_HEIGHT );
		if( item != lastItem ) {
			lastItem = item;
			if( item ) {
				char tooltip[ 500 ];
				item->getTooltip( tooltip );
				canvas->setTooltip( tooltip );
			} else {
				canvas->setTooltip( NULL );
			}
		}
	}
	return false;
}

bool Inven::handleEvent( Widget *widget, SDL_Event *event ) {
	if( pcUi->getScourge()->getSDLHandler()->isDoubleClick ) {
		Item *item = getItemAtPos( pcUi->getScourge()->getSDLHandler()->mouseX - pcUi->getWindow()->getX() - x, 
															 pcUi->getScourge()->getSDLHandler()->mouseY - pcUi->getWindow()->getY() - y - TITLE_HEIGHT );
		if( item && item->getRpgItem()->isContainer() ) {
			pcUi->getScourge()->openContainerGui(item);
		}
	} else if( pcUi->getScourge()->getSDLHandler()->mouseButton == SDL_BUTTON_RIGHT || ( pcUi->getScourge()->getSDLHandler()->mouseButton == SDL_BUTTON_LEFT && pcUi->isInfoSelected() ) ) {
		Item *item = getItemAtPos( pcUi->getScourge()->getSDLHandler()->mouseX - pcUi->getWindow()->getX() - x, 
															 pcUi->getScourge()->getSDLHandler()->mouseY - pcUi->getWindow()->getY() - y - TITLE_HEIGHT );
		if( item ) {
			showInfo( item );
			pcUi->unselectButtons();
		}
	}
  return false;
}

void Inven::showInfo( Item *item ) {
	pcUi->getScourge()->getInfoGui()->setItem( item );
	if( !pcUi->getScourge()->getInfoGui()->getWindow()->isVisible() ) 
		pcUi->getScourge()->getInfoGui()->getWindow()->setVisible( true );
}

void Inven::receive( Widget *widget ) {
	if( creature ) {
		Item *item = pcUi->getScourge()->getMovingItem();
		if( item ) {

			// try to fit it
			if( !findInventoryPosition( item, 
																	pcUi->getScourge()->getSDLHandler()->mouseX - pcUi->getWindow()->getX() - x, 
																	pcUi->getScourge()->getSDLHandler()->mouseY - pcUi->getWindow()->getY() - y - TITLE_HEIGHT ) ) {
				pcUi->getScourge()->showMessageDialog( _( "Can't fit item in inventory." ) );
			} else {
				if( creature->addInventory( item ) ) {
					// message: the player accepted the item
					char message[120];
					snprintf( message, 119, _( "%s picks up %s." ), 
									 creature->getName(),
									 item->getItemName() );
					pcUi->getScourge()->getMap()->addDescription( message );
					pcUi->getScourge()->endItemDrag();
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

bool Inven::startDrag( Widget *widget, int x, int y ) {
	if( creature && !pcUi->getScourge()->getMovingItem() ) {
	
		if( pcUi->getScourge()->getTradeDialog()->getWindow()->isVisible() ) {
			pcUi->getScourge()->showMessageDialog( _( "Can't change inventory while trading." ) );
			return false;
		}
		
		// what's equiped at this inventory slot?
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

void Inven::convertMousePos( int x, int y, int *invX, int *invY ) {
	*invX = ( x - OFFSET_X ) / GRID_SIZE;
	*invY = ( y - OFFSET_Y ) / GRID_SIZE;
}

// Note: this is O(n)
Item *Inven::getItemAtPos( int x, int y ) {
	for( int i = 0; creature && i < creature->getInventoryCount(); i++ ) {
		if( !creature->isEquipped( i ) ) {
			Item *item = creature->getInventory( i );
			int posX, posY;
			convertMousePos( x, y, &posX, &posY );
			if( posX >= item->getInventoryX() && 
					posX < item->getInventoryX() + item->getInventoryWidth() &&
					posY >= item->getInventoryY() && 
					posY < item->getInventoryY() + item->getInventoryHeight() ) {
				return item;
			}
		}
	}
	return NULL;
}

// note: optimize this, current O(n^2)
bool Inven::findInventoryPosition( Item *item, int x, int y, bool useExistingLocationForSameItem ) {
	if( creature ) {
		int colCount = canvas->getWidth() / GRID_SIZE;
		int rowCount = canvas->getHeight() / GRID_SIZE;

		int selX = -1;
		int selY = -1;

		int posX, posY;
		convertMousePos( x, y, &posX, &posY );

		for( int xx = 0; xx < colCount; xx++ ) {
			for( int yy = 0; yy < rowCount; yy++ ) {
				if( xx + item->getInventoryWidth() <= colCount &&
						yy + item->getInventoryHeight() <= rowCount && 
						checkInventoryLocation( item, useExistingLocationForSameItem, xx, yy ) ) {
					if( posX == xx && posY == yy ) {
						selX = xx;
						selY = yy;
						break;
					} else if( selX == -1 ) {
						selX = xx;
						selY = yy;
					}
				}
			}
		}
		
		if( selX > -1 ) {
			item->setInventoryLocation( selX, selY );
			return true;
		}
	}
	return false;
}

bool Inven::checkInventoryLocation( Item *item, bool useExistingLocationForSameItem, int xx, int yy ) {
	SDL_Rect itemRect;
	itemRect.x = xx;
	itemRect.y = yy;
	itemRect.w = item->getInventoryWidth();
	itemRect.h = item->getInventoryHeight();
	for( int t = 0; creature && t < creature->getInventoryCount(); t++ ) {
		if( !creature->isEquipped( t ) ) {
			Item *i = creature->getInventory( t );
			if( i == item ) {
				if( useExistingLocationForSameItem ) {
					return true;
				} else {
					continue;
				}
			}
				
			SDL_Rect iRect;
			iRect.x = i->getInventoryX();
			iRect.y = i->getInventoryY();
			iRect.w = i->getInventoryWidth();
			iRect.h = i->getInventoryHeight();
	
			if( SDLHandler::intersects( &itemRect, &iRect ) ) return false;
		}
	}
	return true;
}

void Inven::drawWidgetContents( Widget *widget ) {
	glEnable( GL_TEXTURE_2D );
	glBindTexture( GL_TEXTURE_2D, backgroundTexture );
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

	glPushMatrix();
	glTranslatef( OFFSET_X, OFFSET_Y, 0 );

	glDisable( GL_TEXTURE_2D );
	pcUi->getWindow()->setTopWindowBorderColor();

	glEnable( GL_BLEND );
	//glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );
  glBlendFunc( GL_SRC_COLOR, GL_DST_COLOR );

	int colCount = canvas->getWidth() / GRID_SIZE;
	int rowCount = canvas->getHeight() / GRID_SIZE;

	if( pcUi->getScourge()->getMovingItem() ) {
		int currentX, currentY;
		convertMousePos( pcUi->getScourge()->getSDLHandler()->mouseX - pcUi->getWindow()->getX() - x, 
										 pcUi->getScourge()->getSDLHandler()->mouseY - pcUi->getWindow()->getY() - y - TITLE_HEIGHT, 
										 &currentX, &currentY );
		int px = currentX;
		int py = currentY;
		if( px >= 0 && px + pcUi->getScourge()->getMovingItem()->getInventoryWidth() <= colCount &&
				py >= 0 && py + pcUi->getScourge()->getMovingItem()->getInventoryHeight() <= rowCount ) {
			px *= GRID_SIZE;
			py *= GRID_SIZE;
			int pw = pcUi->getScourge()->getMovingItem()->getInventoryWidth() * GRID_SIZE;
			int ph = pcUi->getScourge()->getMovingItem()->getInventoryHeight() * GRID_SIZE;
			//cerr << "pw=" << pw << " ph=" << ph << endl;
			glBegin( GL_QUADS );
			glVertex2d( px, py + ph );
			glVertex2d( px, py );
			glVertex2d( px + pw, py );
			glVertex2d( px + pw, py + ph );
			glEnd();
		}
	}

	for( int yy = 0; yy <= rowCount; yy++ ) {
		glBegin( GL_LINE_LOOP );
		glVertex2d( 0, yy * GRID_SIZE );
		glVertex2d( colCount * GRID_SIZE, yy * GRID_SIZE );
		glEnd();
	}
	for( int xx = 0; xx <= colCount; xx++ ) {
		glBegin( GL_LINE_LOOP );
		glVertex2d( xx * GRID_SIZE, 0 );
		glVertex2d( xx * GRID_SIZE, rowCount * GRID_SIZE );
		glEnd();
	}
	glDisable( GL_BLEND );

	glEnable( GL_TEXTURE_2D );
	glColor4f( 1, 1, 1, 1 );

	if( creature ) {
		for( int i = 0; i < creature->getInventoryCount(); i++ ) {
			if( !creature->isEquipped( i ) ) {
				Item *item = creature->getInventory( i );
	
				int ix = item->getInventoryX() * GRID_SIZE;
				int iy = item->getInventoryY() * GRID_SIZE;
				int iw = item->getInventoryWidth() * GRID_SIZE;
				int ih = item->getInventoryHeight() * GRID_SIZE;

				item->renderIcon( pcUi->getScourge(), ix, iy, iw, ih );
			}
		}
	}

	glPopMatrix();
	glDisable( GL_TEXTURE_2D );
}

void Inven::setCreature( Creature *creature ) { 
	this->creature = creature; 
	if( !creature->isInventoryArranged() ) {
		for( int t = 0; creature && t < creature->getInventoryCount(); t++ ) {
			if( !creature->isEquipped( t ) ) {
				Item *item = creature->getInventory( t );
				findInventoryPosition( item, -1, -1, false );
			}
		}
		creature->setInventoryArranged( true );
	}
}

