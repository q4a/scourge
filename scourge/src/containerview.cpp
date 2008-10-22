/***************************************************************************
              containerview.cpp -  The container contents widget
                             -------------------
    begin                : Mon Oct 20 2008
    copyright            : (C) 2008 by Gabor Torok
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

#include "common/constants.h"
#include "containerview.h"
#include "render/renderlib.h"
#include "rpg/rpglib.h"
#include "sound.h"
#include "item.h"
#include "creature.h"
#include "shapepalette.h"
#include "pcui.h"

using namespace std;

#define OFFSET_X 1
#define OFFSET_Y 0

ContainerView::ContainerView( Scourge *scourge, Item *container, Window *win, int x, int y ) : 
	Canvas( x, y, x + container->getRpgItem()->getContainerWidth() * GRID_SIZE, y + container->getRpgItem()->getContainerHeight() * GRID_SIZE, this, this ) {
	this->scourge = scourge;
	this->container = container;
	this->win = win;
	this->x = x;
	this->y = y;
	this->lastItem = NULL;
	this->selectedItem = NULL;	
	showContents();
}

ContainerView::~ContainerView() {
}

void ContainerView::convertMousePos( int x, int y, int *invX, int *invY ) {
	*invX = ( x - OFFSET_X ) / GRID_SIZE;
	*invY = ( y - OFFSET_Y ) / GRID_SIZE;
}

void ContainerView::drawWidgetContents( Widget *widget ) {
	int w = container->getRpgItem()->getContainerWidth() * GRID_SIZE;
	int h = container->getRpgItem()->getContainerHeight() * GRID_SIZE;
	glEnable( GL_TEXTURE_2D );
	glDisable( GL_BLEND );
	//glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );
	container->getContainerTexture().glBind();
	glColor4f( 1, 1, 1, 1 );
	glBegin( GL_TRIANGLE_STRIP );
	glTexCoord2d( 0, 0 );
	glVertex2d( 0, 0 );
	glTexCoord2d( 1, 0 );
	glVertex2d( w, 0 );
	glTexCoord2d( 0, 1 );
	glVertex2d( 0, h );
	glTexCoord2d( 1, 1 );
	glVertex2d( w, h );
	glEnd();
	glDisable( GL_BLEND );
	
	glPushMatrix();
	glTranslatef( OFFSET_X, OFFSET_Y, 0 );

	glDisable( GL_TEXTURE_2D );
	win->setTopWindowBorderColor();

	glEnable( GL_BLEND );
	//glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );
	glBlendFunc( GL_SRC_COLOR, GL_DST_COLOR );

	int colCount = container->getRpgItem()->getContainerWidth();
	int rowCount = container->getRpgItem()->getContainerHeight();

	if ( scourge->getMovingItem() ) {
		int currentX, currentY;
		convertMousePos( scourge->getSDLHandler()->mouseX - win->getX() - x,
		                 scourge->getSDLHandler()->mouseY - win->getY() - y - TITLE_HEIGHT,
		                 &currentX, &currentY );
		int px = currentX;
		int py = currentY;
		if ( px >= 0 && px + scourge->getMovingItem()->getInventoryWidth() <= colCount &&
		        py >= 0 && py + scourge->getMovingItem()->getInventoryHeight() <= rowCount ) {
			px *= GRID_SIZE;
			py *= GRID_SIZE;
			int pw = scourge->getMovingItem()->getInventoryWidth() * GRID_SIZE;
			int ph = scourge->getMovingItem()->getInventoryHeight() * GRID_SIZE;
			//cerr << "pw=" << pw << " ph=" << ph << endl;
			glBegin( GL_TRIANGLE_STRIP );
			glVertex2d( px, py );
			glVertex2d( px + pw, py );
			glVertex2d( px, py + ph );
			glVertex2d( px + pw, py + ph );
			glEnd();
		}
	} else if( getSelectedItem() ) {
		int px = getSelectedItem()->getInventoryX() * GRID_SIZE - OFFSET_X;
		int py = getSelectedItem()->getInventoryY() * GRID_SIZE - OFFSET_Y;
		int pw = getSelectedItem()->getInventoryWidth() * GRID_SIZE;
		int ph = getSelectedItem()->getInventoryHeight() * GRID_SIZE;
		glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );
		glColor4f( 0, 0, 0, 0.5f );
		glBegin( GL_TRIANGLE_STRIP );
		glVertex2d( px, py );
		glVertex2d( px + pw, py );
		glVertex2d( px, py + ph );
		glVertex2d( px + pw, py + ph );
		glEnd();
		win->setTopWindowBorderColor();
		glBlendFunc( GL_SRC_COLOR, GL_DST_COLOR );
	}

	for ( int yy = 0; yy <= rowCount; yy++ ) {
		glBegin( GL_LINE_LOOP );
		glVertex2d( 0, yy * GRID_SIZE );
		glVertex2d( colCount * GRID_SIZE, yy * GRID_SIZE );
		glEnd();
	}
	for ( int xx = 0; xx <= colCount; xx++ ) {
		glBegin( GL_LINE_LOOP );
		glVertex2d( xx * GRID_SIZE, 0 );
		glVertex2d( xx * GRID_SIZE, rowCount * GRID_SIZE );
		glEnd();
	}
	glBegin( GL_LINE_LOOP );
	glVertex2d( 0, 0 );
	glVertex2d( colCount * GRID_SIZE - 1, 0 );
	glVertex2d( colCount * GRID_SIZE - 1, rowCount * GRID_SIZE - 1 );
	glVertex2d( 0, rowCount * GRID_SIZE - 1 );
	glEnd();
	glDisable( GL_BLEND );

	glEnable( GL_TEXTURE_2D );
	glColor4f( 1, 1, 1, 1 );

	for ( int i = 0; i < container->getContainedItemCount(); i++ ) {
		Item *item = container->getContainedItem( i );

		int ix = item->getInventoryX() * GRID_SIZE;
		int iy = item->getInventoryY() * GRID_SIZE;
		int iw = item->getInventoryWidth() * GRID_SIZE;
		int ih = item->getInventoryHeight() * GRID_SIZE;

		item->renderIcon( scourge, ix, iy, iw, ih );
	}

	glPopMatrix();
	glDisable( GL_TEXTURE_2D );	
}

void ContainerView::showContents() {
	for( int i = 0; container && i < container->getContainedItemCount(); i++ ) {
		Item *item = container->getContainedItem( i );
		if( item->getInventoryX() <= 0 && item->getInventoryY() <= 0 ) {
			findInventoryPosition( item, item->getInventoryX(), item->getInventoryY(), false );			
		}
	}
}

bool ContainerView::handleEvent( SDL_Event *event ) {
	if ( event->type == SDL_MOUSEMOTION ) {
			Item *item = getItemAtPos( event->motion.x - win->getX() - x,
			                           event->motion.y - win->getY() - y - TITLE_HEIGHT );
			if ( item != lastItem ) {
				lastItem = item;
				if ( item ) {
					char tooltip[ 500 ];
					item->getTooltip( tooltip );
					setTooltip( tooltip );
				} else {
					setTooltip( NULL );
				}
			}
		}
		return false;	
}

Item *ContainerView::getItemAtPos( int x, int y ) {
	for ( int i = 0; container && i < container->getContainedItemCount(); i++ ) {
		Item *item = container->getContainedItem( i );
		int posX, posY;
		convertMousePos( x, y, &posX, &posY );
		if ( posX >= item->getInventoryX() &&
				posX < item->getInventoryX() + item->getInventoryWidth() &&
				posY >= item->getInventoryY() &&
				posY < item->getInventoryY() + item->getInventoryHeight() ) {
			return item;
		}
	}
	return NULL;
}

void ContainerView::showInfo( Item *item ) {
	scourge->getInfoGui()->setItem( item );
	if ( !scourge->getInfoGui()->getWindow()->isVisible() )
		scourge->getInfoGui()->getWindow()->setVisible( true );
}

bool ContainerView::handleEvent( Widget *widget, SDL_Event *event ) {
	if ( scourge->getSDLHandler()->isDoubleClick ) {
		Item *item = getItemAtPos( scourge->getSDLHandler()->mouseX - win->getX() - x,
		                           scourge->getSDLHandler()->mouseY - win->getY() - y - TITLE_HEIGHT );		
		if ( item ) {
			if ( scourge->getPcUi()->receiveInventory( item ) ) {
				if( item == getSelectedItem() ) {
					setSelectedItem( NULL );
				}
				container->removeContainedItem( item );
				showContents();				
			} else {
				scourge->showMessageDialog( _( "There is not enough room in your backpack for everything." ) );
			}
		}
	} else 	if ( scourge->getSDLHandler()->isDoubleClick ) {
		Item *item = getItemAtPos( scourge->getSDLHandler()->mouseX - win->getX() - x,
		                           scourge->getSDLHandler()->mouseY - win->getY() - y - TITLE_HEIGHT );
		if ( item && item->getRpgItem()->isContainer() ) {
			scourge->openContainerGui( item );
		}
	} else if ( scourge->getSDLHandler()->mouseButton == SDL_BUTTON_RIGHT ) {
		Item *item = getItemAtPos( scourge->getSDLHandler()->mouseX - win->getX() - x,
		                           scourge->getSDLHandler()->mouseY - win->getY() - y - TITLE_HEIGHT );
		if ( item ) {
			showInfo( item );
		}
	} else if ( scourge->getSDLHandler()->mouseButton == SDL_BUTTON_LEFT ) {
			Item *item = getItemAtPos( scourge->getSDLHandler()->mouseX - win->getX() - x,
			                           scourge->getSDLHandler()->mouseY - win->getY() - y - TITLE_HEIGHT );
			if ( item ) {
				setSelectedItem( item );
			}
	}
	return false;
}

void ContainerView::receive( Widget *widget ) {
	enum { MSG_SIZE = 120 };
	char message[ MSG_SIZE ];
	if ( scourge->getMovingItem() && scourge->getMovingItem() != container ) {
		if ( receive( scourge->getMovingItem(), true ) && 
				container->addContainedItem( scourge->getMovingItem() ) ) {
			// message: the container accepted the item
			snprintf( message, MSG_SIZE, _( "%1$s is placed in %2$s." ),
			          scourge->getMovingItem()->getItemName(),
			          container->getItemName() );
			scourge->writeLogMessage( message );
			scourge->endItemDrag();
			showContents();
			scourge->getSession()->getSound()->playSound( Window::DROP_SUCCESS, 127 );
		} else {
			// message: the container is full
			scourge->getSession()->getSound()->playSound( Window::DROP_FAILED, 127 );
			scourge->showMessageDialog( _( "The item won't fit in that container!" ) );
		}
	}
}

bool ContainerView::receive( Item *item, bool atCursor ) {
	//Put item in the most left/top availabel position
	int xPos = 0;
	int yPos = 0;
	//If dialog visible put item on the mouse position
	if ( atCursor ) {
		xPos = scourge->getSDLHandler()->mouseX - win->getX() - x;
		yPos = scourge->getSDLHandler()->mouseY - win->getY() - y - TITLE_HEIGHT;
	}

	// try to fit it
	return findInventoryPosition( item, xPos, yPos );
}

/// Find an inventory position for an item dropped at screen pos x,y.

/// note: optimize this,
/// current O(n^2)

bool ContainerView::findInventoryPosition( Item *item, int x, int y, bool useExistingLocationForSameItem ) {
	if ( container && item ) {
		int colCount = getWidth() / GRID_SIZE;
		int rowCount = getHeight() / GRID_SIZE;

		int selX = -1;
		int selY = -1;

		int posX, posY;
		convertMousePos( x, y, &posX, &posY );

		for ( int xx = 0; xx < colCount; xx++ ) {
			for ( int yy = 0; yy < rowCount; yy++ ) {
				if ( xx + item->getInventoryWidth() <= colCount &&
				        yy + item->getInventoryHeight() <= rowCount &&
				        checkInventoryLocation( item, useExistingLocationForSameItem, xx, yy ) ) {
					if ( posX == xx && posY == yy ) {
						selX = xx;
						selY = yy;
						break;
					} else if ( selX == -1 ) {
						selX = xx;
						selY = yy;
					}
				}
			}
		}

		if ( selX > -1 ) {
			item->setInventoryLocation( selX, selY );
			return true;
		}
	}
	return false;
}

/// Checks whether an item fits into the inventory at screen pos xx,yy.

bool ContainerView::checkInventoryLocation( Item *item, bool useExistingLocationForSameItem, int xx, int yy ) {
	SDL_Rect itemRect;
	itemRect.x = xx;
	itemRect.y = yy;
	itemRect.w = item->getInventoryWidth();
	itemRect.h = item->getInventoryHeight();
	for ( int t = 0; container && t < container->getContainedItemCount(); t++ ) {
		Item *i = container->getContainedItem( t );
		if ( i == item ) {
			if ( useExistingLocationForSameItem ) {
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

		if ( SDLHandler::intersects( &itemRect, &iRect ) ) return false;
	}
	return true;
}

bool ContainerView::startDrag( Widget *widget, int x, int y ) {
	if( !scourge->getMovingItem() ) {
		dropItem();
		return true;
	} else {
		return false;
	}
}

/// Drops an item from a container.

void ContainerView::dropItem() {
	Item *item = getItemAtPos( scourge->getSDLHandler()->mouseX - win->getX() - x,
	                           scourge->getSDLHandler()->mouseY - win->getY() - y - TITLE_HEIGHT );
	if( item ) {
		if( item == getSelectedItem() ) {
			setSelectedItem( NULL );
		}
		container->removeContainedItem( item );
		scourge->startItemDragFromGui( item );
		showContents();
	}
}

void ContainerView::setSelectedItem( Item *item ) {
	selectedItem = item;
	//infoButton->setEnabled( selectedItem != NULL );
}