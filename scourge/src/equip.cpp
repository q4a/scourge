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

#define EQUIP_WIDTH 260
#define EQUIP_HEIGHT 360
#define WIN_WIDTH EQUIP_WIDTH + 20
#define WIN_HEIGHT EQUIP_HEIGHT + 24

Equip::Equip(Scourge *scourge) {
	this->scourge = scourge;
	this->creature = NULL;
	this->backgroundTexture = scourge->getShapePalette()->getNamedTexture( "equip" );

	mainWin = new Window( scourge->getSDLHandler(),
                        ( scourge->getSDLHandler()->getScreen()->w - WIN_WIDTH ) / 2, 
                        ( scourge->getSDLHandler()->getScreen()->h - WIN_HEIGHT ) / 2,
                        WIN_WIDTH, WIN_HEIGHT,
                        "", false, Window::SIMPLE_WINDOW, "default" );
	canvas = new Canvas( 8, 0, EQUIP_WIDTH + 8, EQUIP_HEIGHT, this, this);
  mainWin->addWidget( canvas );
}

Equip::~Equip() {
}

bool Equip::handleEvent(Widget *widget, SDL_Event *event) {
  if( widget == mainWin->closeButton ) {
    scourge->toggleInventoryWindow();
	}
  return false;
}

bool Equip::handleEvent(SDL_Event *event) {
  switch(event->type) {
  case SDL_MOUSEBUTTONUP:
    break;     
  case SDL_KEYUP:
    switch(event->key.keysym.sym) {
    case SDLK_ESCAPE: 
    scourge->toggleInventoryWindow();
    return true;
    default: break;
    }
  default: break;
  }
  return false;
}

void Equip::receive(Widget *widget) {
	if( putItem() != -1 ) equipItem();
}

bool Equip::startDrag(Widget *widget, int x, int y) {
  return false;
}

int Equip::putItem() {
	int index = -1;
	/*
  Item *item = scourge->getMovingItem();
  if(item) {
    if(scourge->getParty()->getParty(selected)->addInventory(item)) {
      // message: the player accepted the item
      char message[120];
      sprintf(message, _( "%s picks up %s." ), 
              scourge->getParty()->getParty(selected)->getName(),
              item->getItemName());
      scourge->getMap()->addDescription(message);
      scourge->endItemDrag();
      index = scourge->getParty()->getParty(selected)->findInInventory(item);
      //setSelectedPlayerAndMode(selected, INVENTORY);
      //invList->setSelectedLine(index);
      scourge->getSDLHandler()->getSound()->playSound(Window::DROP_SUCCESS);
      return index;
    } else {
      // message: the player's inventory is full
      scourge->getSDLHandler()->getSound()->playSound(Window::DROP_FAILED);
      scourge->showMessageDialog( _( "You can't fit the item!" ) );
    }
  }
	*/
  return index;
}

void Equip::equipItem() {
	/*
  int itemIndex = invList->getSelectedLine();  
  if(itemIndex > -1 && 
     scourge->getParty()->getParty(selected)->getInventoryCount() > itemIndex) {
    Item *item = scourge->getParty()->getParty(selected)->getInventory(itemIndex);

    char *err = scourge->getParty()->getParty(selected)->canEquipItem( item );
    if( err ) {
      scourge->showMessageDialog( err );
      scourge->getSDLHandler()->getSound()->playSound( Window::DROP_FAILED );
    } else {
      scourge->getParty()->getParty(selected)->equipInventory(itemIndex);
      // recreate list strings
      //refresh();
      scourge->getSDLHandler()->getSound()->playSound(Window::DROP_SUCCESS);
    }
  }
	*/
}

void Equip::dropItem() {
	/*
  int itemIndex = invList->getSelectedLine();  
  if(itemIndex > -1 && 
     scourge->getParty()->getParty(selected)->getInventoryCount() > itemIndex) {
    Item *item = scourge->getParty()->getParty(selected)->removeInventory(itemIndex);
    scourge->startItemDragFromGui(item);
    char message[120];
    sprintf(message, _( "%s drops %s." ), 
            scourge->getParty()->getParty(selected)->getName(),
            item->getItemName());
    scourge->getMap()->addDescription(message);
    setSelectedPlayerAndMode(selected, INVENTORY);
  }
	*/
}

void Equip::setCreature( Creature *creature ) {
	this->creature = creature;
}

void Equip::drawWidgetContents( Widget *w ) {
	glEnable( GL_TEXTURE_2D );
	glBindTexture( GL_TEXTURE_2D, backgroundTexture );
	glBegin( GL_QUADS );
	glTexCoord2d( 0, 1 );
	glVertex2d( 0, w->getHeight() );
	glTexCoord2d( 0, 0 );
	glVertex2d( 0, 0 );
	glTexCoord2d( 1, 0 );
	glVertex2d( w->getWidth(), 0 );
	glTexCoord2d( 1, 1 );
	glVertex2d( w->getWidth(), w->getHeight() );
	glEnd();
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
}

