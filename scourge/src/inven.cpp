/***************************************************************************
                       inven.cpp  -  Backpack widget
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
#include "common/constants.h"
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
#include "uncursedialog.h"
#include "identifydialog.h"
#include "rechargedialog.h"
#include "sqbinding/sqbinding.h"
#include "characterinfo.h"
#include "shapepalette.h"
#include "skillsview.h"
#include "gui/confirmdialog.h"
#include "pcui.h"
#include "storable.h"

/**
  *@author Gabor Torok
  */

using namespace std;

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
	this->storable = NULL;

	canvas = new Canvas( x, y, x + w, y + h, this, this );
	canvas->setDrawBorders( false );
}

Inven::~Inven() {
}

bool Inven::handleEvent( SDL_Event *event ) {
	if ( event->type == SDL_MOUSEMOTION ) {
		Item *item = getItemAtPos( event->motion.x - pcUi->getWindow()->getX() - x,
		                           event->motion.y - pcUi->getWindow()->getY() - y - TITLE_HEIGHT );
		if ( item != lastItem ) {
			lastItem = item;
			if ( item ) {
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
	if ( pcUi->getScourge()->getSDLHandler()->isDoubleClick ) {
		Item *item = getItemAtPos( pcUi->getScourge()->getSDLHandler()->mouseX - pcUi->getWindow()->getX() - x,
		                           pcUi->getScourge()->getSDLHandler()->mouseY - pcUi->getWindow()->getY() - y - TITLE_HEIGHT );
		if ( item && item->getRpgItem()->isContainer() ) {
			pcUi->getScourge()->openContainerGui( item );
		}
	} else if ( pcUi->getScourge()->getSDLHandler()->mouseButton == SDL_BUTTON_RIGHT ) {
		Item *item = getItemAtPos( pcUi->getScourge()->getSDLHandler()->mouseX - pcUi->getWindow()->getX() - x,
		                           pcUi->getScourge()->getSDLHandler()->mouseY - pcUi->getWindow()->getY() - y - TITLE_HEIGHT );
		if ( item ) {
			showInfo( item );
			pcUi->unselectButtons();
		}
	} else if ( pcUi->getScourge()->getSDLHandler()->mouseButton == SDL_BUTTON_LEFT ) {
		Item *item = getItemAtPos( pcUi->getScourge()->getSDLHandler()->mouseX - pcUi->getWindow()->getX() - x,
		                           pcUi->getScourge()->getSDLHandler()->mouseY - pcUi->getWindow()->getY() - y - TITLE_HEIGHT );
		if ( creature && item ) {
			if ( pcUi->isEnchantSelected() ) {
				pcUi->getScourge()->enchantItem( creature, item );
				canvas->cancelDrag();
			} else if ( pcUi->isInfoSelected() ) {
				showInfo( item );
				canvas->cancelDrag();
			} else if ( pcUi->isStoreSelected() ) {
				storeItem( item );
				canvas->cancelDrag();
			} else if ( pcUi->isTranscribeSelected() ) {
				pcUi->getScourge()->transcribeItem( creature, item );
				canvas->cancelDrag();
			} else if ( pcUi->isUseSelected() ) {
				pcUi->getScourge()->useItem( creature, item );
				canvas->cancelDrag();
			}
			pcUi->unselectButtons();
		}
	}
	return false;
}

/// Shows the info window for an item.

void Inven::showInfo( Item *item ) {
	pcUi->getScourge()->getInfoGui()->setItem( item );
	if ( !pcUi->getScourge()->getInfoGui()->getWindow()->isVisible() )
		pcUi->getScourge()->getInfoGui()->getWindow()->setVisible( true );
}

/// Adds an item that has been dragged here from another GUI widget.

void Inven::receive( Widget *widget ) {
	if ( creature ) {
		Item *item = pcUi->getScourge()->getMovingItem();
		if ( item ) {
			if ( !receive( item, pcUi->getWindow()->isVisible() ) ) {
				pcUi->getScourge()->showMessageDialog( _( "Can't fit item in backpack." ) );
			}
		}
	}
}

/// Adds an item to the backpack.

bool Inven::receive( Item *item, bool atCursor ) {
	//Put item in the most left/top availabel position
	int xPos = 0;
	int yPos = 0;
	//If dialog visible put item on the mouse position
	if ( atCursor ) {
		xPos = pcUi->getScourge()->getSDLHandler()->mouseX - pcUi->getWindow()->getX() - x;
		yPos = pcUi->getScourge()->getSDLHandler()->mouseY - pcUi->getWindow()->getY() - y - TITLE_HEIGHT;
	}

	// try to fit it
	if ( !findBackpackPosition( item,
	                             xPos,
	                             yPos ) ) {
		return false;
	} else {
		if ( creature->addToBackpack( item ) ) {
			// message: the player accepted the item
			char message[120];
			snprintf( message, 119, _( "%s picks up %s." ),
			          creature->getName(),
			          item->getItemName() );
			pcUi->getScourge()->writeLogMessage( message );
			pcUi->getScourge()->endItemDrag();
			pcUi->getScourge()->getSession()->getSound()->playSound( Window::DROP_SUCCESS, 127 );
		} else {
			// message: the player's backpack is full
			pcUi->getScourge()->getSession()->getSound()->playSound( Window::DROP_FAILED, 127 );
			return false;
		}
	}
	return true;
}

/// Initiates mouse drag from screen pos x,y.

bool Inven::startDrag( Widget *widget, int x, int y ) {
	if ( creature && !pcUi->getScourge()->getMovingItem() ) {

		if ( pcUi->getScourge()->getTradeDialog()->getWindow()->isVisible() ) {
			pcUi->getScourge()->showMessageDialog( _( "Can't change backpack while trading." ) );
			return false;
		} else if ( pcUi->getScourge()->getUncurseDialog()->getWindow()->isVisible() ) {
			pcUi->getScourge()->showMessageDialog( _( "Can't change backpack while employing a sage's services." ) );
			return false;
		} else if ( pcUi->getScourge()->getIdentifyDialog()->getWindow()->isVisible() ) {
			pcUi->getScourge()->showMessageDialog( _( "Can't change backpack while employing a sage's services." ) );
			return false;
		} else if ( pcUi->getScourge()->getRechargeDialog()->getWindow()->isVisible() ) {
			pcUi->getScourge()->showMessageDialog( _( "Can't change backpack while employing a sage's services." ) );
			return false;
		}

		// what's equipped at this slot?
		Item *item = getItemAtPos( x, y );
		if ( item ) {
			/*
			if( item->isCursed() ) {
			 pcUi->getScourge()->showMessageDialog( _( "Can't remove cursed item!" ) );
			 return false;
			} else {
			 */
			creature->removeFromBackpack( creature->findInBackpack( item ) );
			pcUi->getScourge()->startItemDragFromGui( item );
			char message[120];
			snprintf( message, 119, _( "%s drops %s." ),
			          creature->getName(),
			          item->getItemName() );
			pcUi->getScourge()->writeLogMessage( message );

			return true;
			//}
		}
	}
	return false;
}

/// Converts a screen pos to an backpack pos.

void Inven::convertMousePos( int x, int y, int *invX, int *invY ) {
	*invX = ( x - OFFSET_X ) / GRID_SIZE;
	*invY = ( y - OFFSET_Y ) / GRID_SIZE;
}

/// Returns the item at screen pos x,y.

/// Note: this
/// is O(n)

Item *Inven::getItemAtPos( int x, int y ) {
	for ( int i = 0; creature && i < creature->getBackpackContentsCount(); i++ ) {
		if ( !creature->isEquipped( i ) ) {
			Item *item = creature->getBackpackItem( i );
			int posX, posY;
			convertMousePos( x, y, &posX, &posY );
			if ( posX >= item->getBackpackX() &&
			        posX < item->getBackpackX() + item->getBackpackWidth() &&
			        posY >= item->getBackpackY() &&
			        posY < item->getBackpackY() + item->getBackpackHeight() ) {
				return item;
			}
		}
	}
	return NULL;
}

/// Find an backpack position for an item dropped at screen pos x,y.

/// note: optimize this,
/// current O(n^2)

bool Inven::findBackpackPosition( Item *item, int x, int y, bool useExistingLocationForSameItem ) {
	if ( creature && item ) {
		int colCount = canvas->getWidth() / GRID_SIZE;
		int rowCount = canvas->getHeight() / GRID_SIZE;

		int selX = -1;
		int selY = -1;

		int posX, posY;
		convertMousePos( x, y, &posX, &posY );

		for ( int xx = 0; xx < colCount; xx++ ) {
			for ( int yy = 0; yy < rowCount; yy++ ) {
				if ( xx + item->getBackpackWidth() <= colCount &&
				        yy + item->getBackpackHeight() <= rowCount &&
				        checkBackpackLocation( item, useExistingLocationForSameItem, xx, yy ) ) {
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
			item->setBackpackLocation( selX, selY );
			return true;
		}
	}
	return false;
}

/// Checks whether an item fits into the backpack at screen pos xx,yy.

bool Inven::checkBackpackLocation( Item *item, bool useExistingLocationForSameItem, int xx, int yy ) {
	SDL_Rect itemRect;
	itemRect.x = xx;
	itemRect.y = yy;
	itemRect.w = item->getBackpackWidth();
	itemRect.h = item->getBackpackHeight();
	for ( int t = 0; creature && t < creature->getBackpackContentsCount(); t++ ) {
		if ( !creature->isEquipped( t ) ) {
			Item *i = creature->getBackpackItem( t );
			if ( i == item ) {
				if ( useExistingLocationForSameItem ) {
					return true;
				} else {
					continue;
				}
			}

			SDL_Rect iRect;
			iRect.x = i->getBackpackX();
			iRect.y = i->getBackpackY();
			iRect.w = i->getBackpackWidth();
			iRect.h = i->getBackpackHeight();

			if ( SDLHandler::intersects( &itemRect, &iRect ) ) return false;
		}
	}
	return true;
}

void Inven::drawWidgetContents( Widget *widget ) {
	glEnable( GL_TEXTURE_2D );
	glEnable( GL_BLEND );
	glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );
	backgroundTexture.glBind();
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
	pcUi->getWindow()->setTopWindowBorderColor();

	glEnable( GL_BLEND );
	//glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );
	glBlendFunc( GL_SRC_COLOR, GL_DST_COLOR );

	int colCount = canvas->getWidth() / GRID_SIZE;
	int rowCount = canvas->getHeight() / GRID_SIZE;

	if ( pcUi->getScourge()->getMovingItem() ) {
		int currentX, currentY;
		convertMousePos( pcUi->getScourge()->getSDLHandler()->mouseX - pcUi->getWindow()->getX() - x,
		                 pcUi->getScourge()->getSDLHandler()->mouseY - pcUi->getWindow()->getY() - y - TITLE_HEIGHT,
		                 &currentX, &currentY );
		int px = currentX;
		int py = currentY;
		if ( px >= 0 && px + pcUi->getScourge()->getMovingItem()->getBackpackWidth() <= colCount &&
		        py >= 0 && py + pcUi->getScourge()->getMovingItem()->getBackpackHeight() <= rowCount ) {
			px *= GRID_SIZE;
			py *= GRID_SIZE;
			int pw = pcUi->getScourge()->getMovingItem()->getBackpackWidth() * GRID_SIZE;
			int ph = pcUi->getScourge()->getMovingItem()->getBackpackHeight() * GRID_SIZE;
			//cerr << "pw=" << pw << " ph=" << ph << endl;
			glBegin( GL_TRIANGLE_STRIP );
			glVertex2d( px, py );
			glVertex2d( px + pw, py );
			glVertex2d( px, py + ph );
			glVertex2d( px + pw, py + ph );
			glEnd();
		}
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
	glDisable( GL_BLEND );

	glEnable( GL_TEXTURE_2D );
	glColor4f( 1, 1, 1, 1 );

	if ( creature ) {
		for ( int i = 0; i < creature->getBackpackContentsCount(); i++ ) {
			if ( !creature->isEquipped( i ) ) {
				Item *item = creature->getBackpackItem( i );

				int ix = item->getBackpackX() * GRID_SIZE;
				int iy = item->getBackpackY() * GRID_SIZE;
				int iw = item->getBackpackWidth() * GRID_SIZE;
				int ih = item->getBackpackHeight() * GRID_SIZE;

				item->renderIcon( pcUi->getScourge(), ix, iy, iw, ih );
			}
		}
	}

	glPopMatrix();
	glDisable( GL_TEXTURE_2D );
}

/// Which creature's backpack will we display?

void Inven::setCreature( Creature *creature ) {
	this->creature = creature;
	if ( !creature->isBackpackSorted() ) {
		for ( int t = 0; creature && t < creature->getBackpackContentsCount(); t++ ) {
			if ( !creature->isEquipped( t ) ) {
				Item *item = creature->getBackpackItem( t );
				findBackpackPosition( item, -1, -1, false );
			}
		}
		creature->setBackpackSorted( true );
	}
}

/// Initiates "store in quickspell slot" mode.

void Inven::storeItem( Item *item ) {
	this->storable = NULL;
	Storable *s = ( Storable* )item;
	const char *p = s->isStorable();
	if ( p ) {
		pcUi->getScourge()->showMessageDialog( p );
	} else {
		this->storable = s;
		pcUi->getScourge()->showMessageDialog( _( "Click a quickspell slot to store this item." ) );
		pcUi->hide();
	}
}

