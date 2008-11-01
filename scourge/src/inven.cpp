/***************************************************************************
                       inven.cpp  -  Inventory widget
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

	view = new ContainerView( pcUi->getScourge(), pcUi->getScourge()->getSession()->getParty()->getPlayer()->getBackpack(), pcUi->getWindow(), x, y );
	view->setItem( pcUi->getScourge()->getSession()->getParty()->getPlayer()->getBackpack(), pcUi->getScourge()->getSession()->getParty()->getPlayer() );
}

Inven::~Inven() {
}

bool Inven::handleEvent( SDL_Event *event ) {
	return view->handleEvent( event );
}

bool Inven::handleEvent( Widget *widget, SDL_Event *event ) {
	if ( pcUi->getScourge()->getSDLHandler()->mouseButton == SDL_BUTTON_LEFT && view->getSelectedItem() ) {
		Item *item = view->getSelectedItem();
		if ( creature && item ) {
			if ( pcUi->isEnchantSelected() ) {
				pcUi->getScourge()->enchantItem( creature, item );
				view->cancelDrag();
			} else if ( pcUi->isInfoSelected() ) {
				view->showInfo( item );
				view->cancelDrag();
			} else if ( pcUi->isStoreSelected() ) {
				storeItem( item );
				view->cancelDrag();
			} else if ( pcUi->isTranscribeSelected() ) {
				pcUi->getScourge()->transcribeItem( creature, item );
				view->cancelDrag();
			} else if ( pcUi->isUseSelected() ) {
				pcUi->getScourge()->useItem( creature, item );
				view->cancelDrag();
			}
			pcUi->unselectButtons();
		}
	}
	return view->handleEvent( widget, event );	
}

/// Which creature's inventory will we display?

void Inven::setCreature( Creature *creature ) {
	this->creature = creature;
	view->setItem( creature->getBackpack(), creature );
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

/// Receive an item other than drag + drop

bool Inven::receive( Item *item, bool atCursor ) {
	return view->receiveItem( item, atCursor );
}

void Inven::receive( Widget *widget ) {
	view->receive( widget );
}
