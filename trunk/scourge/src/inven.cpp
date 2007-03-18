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

/**
  *@author Gabor Torok
  */

using namespace std;

#define GRID_SIZE 30
#define OFFSET_X 5
#define OFFSET_Y 5

Inven::Inven( Scourge *scourge, Window *window, int x, int y, int w, int h ) {
	this->scourge = scourge;
	this->creature = NULL;
	this->backgroundTexture = scourge->getShapePalette()->getNamedTexture( "inven" );
  this->currentHole = -1;
	this->window = window;
	this->x = x;
	this->y = y;
	this->w = w;
	this->h = h;

	canvas = new Canvas( x, y, x + w, y + h, this, this);
	canvas->setDrawBorders( false );
}

Inven::~Inven() {
}

bool Inven::handleEvent( SDL_Event *event ) {
	return false;
}

bool Inven::handleEvent( Widget *widget, SDL_Event *event ) {
	if( scourge->getSDLHandler()->isDoubleClick ) {
		Item *item = getItemAtPos( scourge->getSDLHandler()->mouseX - window->getX() - x, 
															 scourge->getSDLHandler()->mouseY - window->getY() - y - TITLE_HEIGHT );
		if( item && item->getRpgItem()->isContainer() ) {
			scourge->openContainerGui(item);
		}
	} else if( scourge->getSDLHandler()->mouseButton == SDL_BUTTON_RIGHT ) {
		Item *item = getItemAtPos( scourge->getSDLHandler()->mouseX - window->getX() - x, 
															 scourge->getSDLHandler()->mouseY - window->getY() - y - TITLE_HEIGHT );
		if( item ) {
			scourge->getInfoGui()->setItem( item );
			if( !scourge->getInfoGui()->getWindow()->isVisible() ) 
				scourge->getInfoGui()->getWindow()->setVisible( true );
		}
	}
  return false;
}

void Inven::receive( Widget *widget ) {
	if( creature ) {
		Item *item = scourge->getMovingItem();
		if( item ) {

			// try to fit it
			if( !findInventoryPosition( item, 
																	scourge->getSDLHandler()->mouseX - window->getX() - x, 
																	scourge->getSDLHandler()->mouseY - window->getY() - y - TITLE_HEIGHT ) ) {
				scourge->showMessageDialog( _( "Can't fit item in inventory." ) );
			} else {
				if( creature->addInventory( item ) ) {
					// message: the player accepted the item
					char message[120];
					snprintf( message, 119, _( "%s picks up %s." ), 
									 creature->getName(),
									 item->getItemName() );
					scourge->getMap()->addDescription( message );
					scourge->endItemDrag();
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

bool Inven::startDrag( Widget *widget, int x, int y ) {
	if( creature && !scourge->getMovingItem() ) {
	
		if( scourge->getTradeDialog()->getWindow()->isVisible() ) {
			scourge->showMessageDialog( _( "Can't change inventory while trading." ) );
			return false;
		}
		
		// what's equiped at this inventory slot?
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
	window->setTopWindowBorderColor();

	glEnable( GL_BLEND );
	//glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );
  glBlendFunc( GL_SRC_COLOR, GL_DST_COLOR );

	int colCount = canvas->getWidth() / GRID_SIZE;
	int rowCount = canvas->getHeight() / GRID_SIZE;

	if( scourge->getMovingItem() ) {
		int currentX, currentY;
		convertMousePos( scourge->getSDLHandler()->mouseX - window->getX() - x, 
										 scourge->getSDLHandler()->mouseY - window->getY() - y - TITLE_HEIGHT, 
										 &currentX, &currentY );
		int px = currentX;
		int py = currentY;
		if( px >= 0 && px + scourge->getMovingItem()->getInventoryWidth() <= colCount &&
				py >= 0 && py + scourge->getMovingItem()->getInventoryHeight() <= rowCount ) {
			px *= GRID_SIZE;
			py *= GRID_SIZE;
			int pw = scourge->getMovingItem()->getInventoryWidth() * GRID_SIZE;
			int ph = scourge->getMovingItem()->getInventoryHeight() * GRID_SIZE;
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
	
				GLuint tex = scourge->getShapePalette()->
					tilesTex[ item->getRpgItem()->getIconTileX() ][ item->getRpgItem()->getIconTileY() ];
				glEnable( GL_ALPHA_TEST );
				glAlphaFunc( GL_NOTEQUAL, 0 );
				glBindTexture( GL_TEXTURE_2D, tex );
				glBegin( GL_QUADS );
				glTexCoord2d( 0, 1 );
				glVertex2d( ix, iy + ih );
				glTexCoord2d( 0, 0 );
				glVertex2d( ix, iy );
				glTexCoord2d( 1, 0 );
				glVertex2d( ix + iw, iy );
				glTexCoord2d( 1, 1 );
				glVertex2d( ix + iw, iy + ih );
				glEnd();
			}
		}
	}

	glPopMatrix();
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

