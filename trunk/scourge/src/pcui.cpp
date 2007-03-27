/***************************************************************************
                          pcui.cpp  -  description
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

#include "pcui.h"
#include "rpg/rpglib.h"
#include "sdlhandler.h"
#include "sdleventhandler.h"
#include "sdlscreenview.h"
#include "scourge.h"
#include "item.h"
#include "creature.h"
#include "equip.h"
#include "inven.h"
#include "portrait.h"
#include "shapepalette.h"

using namespace std;

/**
  *@author Gabor Torok
  */

#define EQUIP_WIDTH 230
#define EQUIP_HEIGHT 300
#define INVEN_WIDTH 530
#define INVEN_HEIGHT 160
#define PORTRAIT_WIDTH 290
#define PORTRAIT_HEIGHT 300
#define WIN_WIDTH EQUIP_WIDTH + 320
#define WIN_HEIGHT EQUIP_HEIGHT + 245
#define DEFAULT_STATUS "Right click for info, double-click to open."

PcUi::PcUi( Scourge *scourge ) {
	this->scourge = scourge;
	this->creature = NULL;

	mainWin = scourge->createWindow( ( scourge->getSDLHandler()->getScreen()->w - WIN_WIDTH ) / 2, 
																	 ( scourge->getSDLHandler()->getScreen()->h - WIN_HEIGHT ) / 2,
																	 WIN_WIDTH, WIN_HEIGHT,
																	 _( "Character Information" ) );
 equip = new Equip( this, 10, 5, EQUIP_WIDTH, EQUIP_HEIGHT );
 mainWin->addWidget( equip->getWidget() );
 inven = new Inven( this, 10, EQUIP_HEIGHT + 15, INVEN_WIDTH, INVEN_HEIGHT );
 mainWin->addWidget( inven->getWidget() );
 portrait = new Portrait( this, 20 + EQUIP_WIDTH, 5, PORTRAIT_WIDTH, PORTRAIT_HEIGHT );
 mainWin->addWidget( portrait->getWidget() );
 int x = 12;
 int y = EQUIP_HEIGHT + 20 + INVEN_HEIGHT;
 use = mainWin->createButton( x, y, x + 32, y + 32, NULL, true, scourge->getShapePalette()->getNamedTexture( "use" ) );
 use->setTooltip( _( "Use item" ) );
 x += 33;
 transcribe = mainWin->createButton( x, y, x + 32, y + 32, NULL, true, scourge->getShapePalette()->getNamedTexture( "transcribe" ) );
 transcribe->setTooltip( _( "Transcribe scroll into the spellbook" ) );
 x += 33;
 enchant = mainWin->createButton( x, y, x + 32, y + 32, NULL, true, scourge->getShapePalette()->getNamedTexture( "enchant" ) );
 enchant->setTooltip( _( "Enchant item" ) );
 x += 33;
 info = mainWin->createButton( x, y, x + 32, y + 32, NULL, true, scourge->getShapePalette()->getNamedTexture( "info" ) );
 info->setTooltip( _( "Show item information" ) );
 x += 33;
 store = mainWin->createButton( x, y, x + 32, y + 32, NULL, true, scourge->getShapePalette()->getNamedTexture( "store" ) );
 store->setTooltip( _( "Store item in quick-spell slot" ) );
 x += 33;
 status = new Label( x + 5, y + 22, _( DEFAULT_STATUS ) );
 mainWin->addWidget( status );
}

PcUi::~PcUi() {
	delete equip;
	delete inven;
	delete portrait;
}

bool PcUi::handleEvent(Widget *widget, SDL_Event *event) {
	equip->handleEvent( widget, event );
	inven->handleEvent( widget, event );
	portrait->handleEvent( widget, event );

  if( widget == mainWin->closeButton ) {
    scourge->toggleInventoryWindow();
	} else if( widget == info ) {
		toggleButtons( info );
	} else if( widget == use ) {
		toggleButtons( use );
	} else if( widget == transcribe ) {
		toggleButtons( transcribe );
	} else if( widget == enchant ) {
		toggleButtons( enchant );
	} else if( widget == store ) {
		toggleButtons( store );
	}
  return false;
}

void PcUi::toggleButtons( Button *button ) {
	if( button != info ) info->setSelected( false );
	if( button != use ) use->setSelected( false );
	if( button != transcribe ) transcribe->setSelected( false );
	if( button != enchant ) enchant->setSelected( false );
	if( button != store ) store->setSelected( false );
	if( info->isSelected() || use->isSelected() || transcribe->isSelected() || enchant->isSelected() || store->isSelected() ) {
		status->setText( _( "Click on an item to select it..." ) );
	} else {
		status->setText( _( DEFAULT_STATUS ) );
	}
}

bool PcUi::isUseSelected() {
	return use->isSelected();
}

bool PcUi::isEnchantSelected() {
	return enchant->isSelected();
}

bool PcUi::isTranscribeSelected() {
	return transcribe->isSelected();
}

bool PcUi::isInfoSelected() {
	return info->isSelected();
}

bool PcUi::isStoreSelected() {
	return store->isSelected();
}

void PcUi::unselectButtons() {
	toggleButtons( NULL );
}

bool PcUi::handleEvent(SDL_Event *event) {
	equip->handleEvent( event );
	inven->handleEvent( event );
	portrait->handleEvent( event );

  switch(event->type) {
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

void PcUi::setCreature( Creature *creature ) {
	this->creature = creature;
	equip->setCreature( creature );
	inven->setCreature( creature );
	portrait->setCreature( creature );
}


