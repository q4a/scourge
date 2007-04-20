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
#define INVEN_WIDTH 605
#define INVEN_HEIGHT 160
#define PORTRAIT_WIDTH 290
#define PORTRAIT_HEIGHT 300
#define WIN_WIDTH EQUIP_WIDTH + 320 + 40 + 35
#define WIN_HEIGHT EQUIP_HEIGHT + 240
#define DEFAULT_STATUS "Right click for info, double-click to open."

#define EQUIP_CARD 0
#define MISSION_CARD 0

PcUi::PcUi( Scourge *scourge ) {
	this->scourge = scourge;
	this->creature = NULL;

	mainWin = scourge->createWindow( ( scourge->getScreenWidth() - ( WIN_WIDTH ) ) / 2, 
																	 ( scourge->getScreenHeight() - ( WIN_HEIGHT ) ) / 2,
																	 WIN_WIDTH, WIN_HEIGHT,
																	 _( "Character Information" ) );
	mainWin->addWindowListener( this );
	int x = 10;
	int y = 5;
	equipButton = mainWin->createButton( x, y, x + 32, y + 32, NULL, true, scourge->getShapePalette()->getNamedTexture( "equipButton" ) );
	equipButton->setSelected( true );
	equipButton->setTooltip( _( "Show paperdoll inventory" ) );
	y += 33;
	spellsButton = mainWin->createButton( x, y, x + 32, y + 32, NULL, true, scourge->getShapePalette()->getNamedTexture( "spellsButton" ) );
	spellsButton->setSelected( false );
	spellsButton->setTooltip( _( "Show spellbook" ) );
	y += 33;
	capabilitiesButton = mainWin->createButton( x, y, x + 32, y + 32, NULL, true, scourge->getShapePalette()->getNamedTexture( "capabilitiesButton" ) );
	capabilitiesButton->setSelected( false );
	capabilitiesButton->setTooltip( _( "Show special capabilities" ) );
	y += 33;
	missionButton = mainWin->createButton( x, y, x + 32, y + 32, NULL, true, scourge->getShapePalette()->getNamedTexture( "missionButton" ) );
	missionButton->setSelected( false );
	missionButton->setTooltip( _( "Show info about the current mission" ) );

	x += 40;
	//CardContainer *cc = new CardContainer( mainWin );
	equip = new Equip( this, x, 5, EQUIP_WIDTH, EQUIP_HEIGHT );
	//cc->addWidget( equip->getWidget(), EQUIP_CARD );
	mainWin->addWidget( equip->getWidget() );
	missionInfo = new MissionInfoUI( this, x, 5, EQUIP_WIDTH, EQUIP_HEIGHT );
	missionInfo->hide();
	//cc->addWidget( missionInfo, MISSION_CARD );

	inven = new Inven( this, 10, EQUIP_HEIGHT + 10, INVEN_WIDTH, INVEN_HEIGHT );
	mainWin->addWidget( inven->getWidget() );
	portrait = new Portrait( this, x + 5 + EQUIP_WIDTH, 5, PORTRAIT_WIDTH, PORTRAIT_HEIGHT );
	mainWin->addWidget( portrait->getWidget() );
	x = 12;
	y = EQUIP_HEIGHT + 15 + INVEN_HEIGHT;
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
	
	x = WIN_WIDTH - 12 - 33 - 33;
	prev = mainWin->createButton( x, y, x + 32, y + 32, NULL, false, scourge->getShapePalette()->getNamedTexture( "prev" ) );
	prev->setTooltip( _( "Switch to previous party member" ) );
	x += 33;
	next = mainWin->createButton( x, y, x + 32, y + 32, NULL, false, scourge->getShapePalette()->getNamedTexture( "next" ) );
	next->setTooltip( _( "Switch to next party member" ) );
	
	x = WIN_WIDTH - 12 - 33;
	y = 5;
	stats = mainWin->createButton( x, y, x + 32, y + 32, NULL, true, scourge->getShapePalette()->getNamedTexture( "stats" ) );
	stats->setTooltip( _( "Show character stats" ) );
	stats->setSelected( true );
	y += 33;
	skills = mainWin->createButton( x, y, x + 32, y + 32, NULL, true, scourge->getShapePalette()->getNamedTexture( "skills" ) );
	skills->setTooltip( _( "Show character skills" ) );
	y += 33;
	statemods = mainWin->createButton( x, y, x + 32, y + 32, NULL, true, scourge->getShapePalette()->getNamedTexture( "stateMods" ) );
	statemods->setTooltip( _( "Show additional info about the character" ) );
	
	y = PORTRAIT_HEIGHT - 33 - 33 - 33 - 33 + 5;
	poolMoney = mainWin->createButton( x, y, x + 32, y + 32, NULL, false, scourge->getShapePalette()->getNamedTexture( "pool" ) );
	poolMoney->setTooltip( _( "Give all of the party's coins to this character" ) );
	y += 33;
	up = mainWin->createButton( x, y, x + 32, y + 32, NULL, false, scourge->getShapePalette()->getNamedTexture( "up" ) );
	up->setTooltip( _( "Page skills up" ) );
	y += 33;
	down = mainWin->createButton( x, y, x + 32, y + 32, NULL, false, scourge->getShapePalette()->getNamedTexture( "down" ) );
	down->setTooltip( _( "Page skills down" ) );
	cast = mainWin->createButton( 10, y, 10 + 32, y + 32, NULL, true, scourge->getShapePalette()->getNamedTexture( "cast" ) );
	cast->setTooltip( _( "Cast spell or use capability" ) );
	y += 33;
	applyMods = mainWin->createButton( x, y, x + 32, y + 32, NULL, false, scourge->getShapePalette()->getNamedTexture( "applySkillMods" ) );
	applyMods->setTooltip( _( "Apply Skill Modifications" ) );
	storeSpell = mainWin->createButton( 10, y, 10 + 32, y + 32, NULL, true, scourge->getShapePalette()->getNamedTexture( "store" ) );
	storeSpell->setTooltip( _( "Store spell or capability in a quickspell slot" ) );
	
	up->setEnabled( false );
	down->setEnabled( false );
	poolMoney->setEnabled( true );
	applyMods->setEnabled( false );
	cast->setEnabled( false );
	storeSpell->setEnabled( false );
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
	} else if( widget == prev ) {
		scourge->getParty()->previousPartyMember();
	} else if( widget == next ) {
		scourge->getParty()->nextPartyMember();
	} else if( widget == stats ) {
		stats->setSelected( true );
		skills->setSelected( false );
		statemods->setSelected( false );
		up->setEnabled( false );
		down->setEnabled( false );
		applyMods->setEnabled( false );		
		portrait->setMode( Portrait::STATS_MODE );
	} else if( widget == skills ) {
		stats->setSelected( false );
		skills->setSelected( true );
		statemods->setSelected( false );	
		up->setEnabled( true );
		down->setEnabled( true );
		applyMods->setEnabled( true );
		portrait->setMode( Portrait::SKILLS_MODE );
	} else if( widget == statemods ) {
		stats->setSelected( false );
		skills->setSelected( false );
		statemods->setSelected( true );
		up->setEnabled( false );
		down->setEnabled( false );
		applyMods->setEnabled( false );		
		portrait->setMode( Portrait::STATE_MODS );		
	} else if( widget == poolMoney ) {
		if( creature ) {
			for( int i = 0; i < scourge->getSession()->getParty()->getPartySize(); i++ ) {
				Creature *c = scourge->getSession()->getParty()->getParty(i);
				if( c != creature ) {
					creature->setMoney( creature->getMoney() + c->getMoney() );
					c->setMoney( 0 );
				}
			}
			char msg[120];
			sprintf( msg, _( "Party members give all their money to %s." ), creature->getName() );
			scourge->showMessageDialog( msg );
		}
	} else if( widget == up ) {
		portrait->scrollSkillsUp();
	} else if( widget == down ) {
		portrait->scrollSkillsDown();
	} else if( widget == applyMods && creature && creature->getHasAvailableSkillPoints() ) {
		if( creature->getAvailableSkillMod() > 0 ) {
				scourge->showMessageDialog( _( "You still have skill points to distribute." ) );
		} else {
			creature->applySkillMods();
			scourge->showMessageDialog( _( "All available skill points have been applied." ) );
		}
	} else if( widget == equipButton ) {
		toggleLeftButtons( equipButton );
		equip->setMode( Equip::EQUIP_MODE );
		//cc->setActiveCard( EQUIP_CARD );
	} else if( widget == spellsButton ) {
		toggleLeftButtons( spellsButton );
		equip->setMode( Equip::SPELLS_MODE );
		//cc->setActiveCard( EQUIP_CARD );
	} else if( widget == capabilitiesButton ) {
		toggleLeftButtons( capabilitiesButton );
		equip->setMode( Equip::CAPABILITIES_MODE );
		//cc->setActiveCard( EQUIP_CARD );
	} else if( widget == missionButton ) {
		toggleLeftButtons( missionButton );
		//cc->setActiveCard( MISSION_CARD );
	}	else if( widget == cast || widget == storeSpell ) {
		toggleSpellButtons( (Button*)widget );
	} else if( widget == missionInfo->getConsoleButton() ) {
		scourge->getSquirrelConsole()->setVisible( true );
	}
  return false;
}

void PcUi::toggleLeftButtons( Button *button ) {
	if( button != equipButton ) equipButton->setSelected( false );
	if( button != spellsButton ) spellsButton->setSelected( false );
	if( button != capabilitiesButton ) capabilitiesButton->setSelected( false );
	if( button != missionButton ) missionButton->setSelected( false );
	bool b = ( button == spellsButton || button == capabilitiesButton );
	cast->setEnabled( b );
	storeSpell->setEnabled( b );
	if( missionButton->isSelected() ) {
		missionInfo->show();
		equip->getWidget()->setVisible( false );
	} else {
		missionInfo->hide();
		equip->getWidget()->setVisible( true );
	}
}

void PcUi::toggleButtons( Button *button ) {
	if( button != info ) info->setSelected( false );
	if( button != use ) use->setSelected( false );
	if( button != transcribe ) transcribe->setSelected( false );
	if( button != enchant ) enchant->setSelected( false );
	if( button != store ) store->setSelected( false );
	if( info->isSelected() || use->isSelected() || transcribe->isSelected() || enchant->isSelected() || store->isSelected() ) {
		status->setText( _( "Click on an item to select it..." ) );
		scourge->setCursorMode( Constants::CURSOR_CROSSHAIR );
	} else {
		status->setText( _( DEFAULT_STATUS ) );
		scourge->setCursorMode( Constants::CURSOR_NORMAL );
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

bool PcUi::isCastSelected() {
	return cast->isSelected();
}

bool PcUi::isStoreSpellSelected() {
	return storeSpell->isSelected();
}

void PcUi::unselectSpellButtons() {
	toggleSpellButtons( NULL );
}

void PcUi::toggleSpellButtons( Button *button ) {
	if( button != cast ) cast->setSelected( false );
	if( button != storeSpell ) storeSpell->setSelected( false );
	if( cast->isSelected() || storeSpell->isSelected() ) {
		status->setText( _( "Click on a spell or capability to select it..." ) );
		scourge->setCursorMode( Constants::CURSOR_CROSSHAIR );
	} else {
		status->setText( _( DEFAULT_STATUS ) );
		scourge->setCursorMode( Constants::CURSOR_NORMAL );
	}
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

void PcUi::windowClosing() {
	toggleButtons( NULL );
	toggleSpellButtons( NULL );
}

void PcUi::setCreature( Creature *creature ) {
	this->creature = creature;
	equip->setCreature( creature );
	inven->setCreature( creature );
	portrait->setCreature( creature );
	missionInfo->refresh();
	refresh();
}

void PcUi::receiveInventory() {
  inven->receive( inven->getWidget() );
}

bool PcUi::receiveInventory(Item *item) {
  return inven->receive(item, false);
}


void PcUi::show() {
	refresh();
	mainWin->setVisible( true );
}

void PcUi::hide() {
	mainWin->setVisible( false );
}

void PcUi::refresh() {
	next->setEnabled( !scourge->inTurnBasedCombat() );
	prev->setEnabled( !scourge->inTurnBasedCombat() );
}

Storable *PcUi::getStorable() { 
	return( equip->getStorable() ? equip->getStorable() : inven->getStorable() );
}

void PcUi::clearStorable() {
	equip->clearStorable();
	inven->clearStorable();
}

