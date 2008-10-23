/***************************************************************************
tradedialog.cpp  -  The trade dialog
-------------------
    begin                : 8/26/2005
    copyright            : (C) 2005 by Gabor Torok
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
#include "tradedialog.h"
#include "scourge.h"
#include "creature.h"
#include "item.h"
#include "gui/window.h"
#include "rpg/rpglib.h"
#include "render/renderlib.h"

using namespace std;

TradeDialog::TradeDialog( Scourge *scourge ) {
	this->scourge = scourge;
	this->creature = NULL;

	win =
	  scourge->createWindow( 50, 50,
	                         605, 345,
	                         Constants::getMessage( Constants::TRADE_DIALOG_TITLE ) );
	//win->setModal( true );
	int xStart = 8;
	playerName = win->createLabel( 80, 15, "" );
	playerTotal = win->createLabel( 80, 28, _( "Selected Total:" ) );
	playerList = new ItemList( scourge, win, 75, 35, 220, 210, this );
	playerList->setAllowCursed( false );
	// We don't want to sell equipped items
	playerList->setAllowEquipped( false );
	win->addWidget( playerList );
	infoButtonA = win->createButton( xStart, 35, 70, 55, _( "Info" ) );


	creatureName = win->createLabel( 305, 15, "" );
	creatureTotal = win->createLabel( 305, 28, _( "Selected Total:" ) );
	creatureList = new ItemList( scourge, win, 300, 35, 220, 210, this );
	creatureList->setAllowCursed( false );
	creatureList->setAllowEquipped( false );
	win->addWidget( creatureList );
	stealButton = win->createButton( 530, 35, 595, 55, _( "Steal" ) );
	infoButtonB = win->createButton( 530, 60, 595, 80, _( "Info" ) );

	tradeButton = win->createButton( this->getWindow()->getWidth() / 2 - 45, 270, this->getWindow()->getWidth() / 2 + 45, 290, _( "Trade!" ) );
	closeButton = win->createButton( 530, 295, 595, 315, _( "Close" ) );

	tradeInfo = win->createLabel( xStart, 260, _( "Available Coins:" ) );
	tradeInfo->setSpecialColor();

	win->createLabel( xStart, 310, _( "Shift+click to select multiple items, right click to get info." ) );
}

TradeDialog::~TradeDialog() {
	delete win;
}

void TradeDialog::setCreature( Creature *creature ) {
	this->creature = creature;
	win->setVisible( true );
	updateUI();
}

void TradeDialog::updateUI() {
	prices.clear();
	playerList->unselectAllLines();
	playerName->setText( scourge->getParty()->getPlayer()->getName() );
	playerList->setCreature( scourge->getParty()->getPlayer(), creature->getNpcInfo()->getSubtype() );
	creatureList->unselectAllLines();
	creatureName->setText( _( creature->getName() ) );
	creatureList->setCreature( creature, creature->getNpcInfo()->getSubtype() );
	totalAmount = 0;
	updateLabels();
}

void TradeDialog::updateLabels() {
	enum { TMP_SIZE = 120 };
	char tmp[ TMP_SIZE ];
	snprintf( tmp, TMP_SIZE, _( "%s $%d, %s $%d" ), _( "Available Coins:" ), scourge->getParty()->getPlayer()->getMoney(), ( totalAmount < 0 ? _( "you receive:" ) : _( "to pay:" ) ), ( totalAmount < 0 ? -totalAmount : totalAmount ) );
	if ( ( scourge->getParty()->getPlayer()->getMoney() - totalAmount ) < 0 ) {
		tradeInfo->setColor( 1, 0, 0 );
		tradeButton->setEnabled( false );
	} else {
		tradeInfo->setColor( 1, 1, 1 );
		tradeButton->setEnabled( true );
	}
	tradeInfo->move( ( this->getWindow()->getWidth() / 2 ) - ( scourge->getSDLHandler()->textWidth( tmp ) / 2  ), 260 );
	tradeInfo->setText( tmp );
	snprintf( tmp, TMP_SIZE, _( "%s $%d" ), _( "Selected Total:" ), getSelectedTotal( playerList ) );
	playerTotal->setText( tmp );
	snprintf( tmp, TMP_SIZE, _( "%s $%d" ), _( "Selected Total:" ), getSelectedTotal( creatureList ) );
	creatureTotal->setText( tmp );
}

int TradeDialog::getSelectedTotal( ItemList *list ) {
	int total = 0;
	for ( int i = 0; i < list->getSelectedLineCount(); i++ ) {
		Item *item = list->getSelectedItem( i );
		total += prices[ item ];
	}
	return total;
}

void TradeDialog::handleEvent( Widget *widget, SDL_Event *event ) {
	if ( widget == win->closeButton || widget == closeButton ) {
		win->setVisible( false );
	} else if ( widget == infoButtonA && playerList->getSelectedLineCount() ) {
		scourge->getInfoGui()->
		setItem( playerList->getSelectedItem( 0 ) );
		if ( !scourge->getInfoGui()->getWindow()->isVisible() )
			scourge->getInfoGui()->getWindow()->setVisible( true );
	} else if ( widget == infoButtonB && creatureList->getSelectedLineCount() ) {
		scourge->getInfoGui()->
		setItem( creatureList->getSelectedItem( 0 ) );
		if ( !scourge->getInfoGui()->getWindow()->isVisible() )
			scourge->getInfoGui()->getWindow()->setVisible( true );
	} else if ( widget == playerList || widget == creatureList ) {
		totalAmount = getSelectedTotal( creatureList ) - getSelectedTotal( playerList );
		updateLabels();
	} else if ( widget == tradeButton ) {
		trade();
	} else if ( widget == stealButton ) {
		steal();
	}
}

void TradeDialog::render( const Widget *widget, const Item *item, std::string& buffer ) {
	std::string s;
	( ( Item* )item )->getDetailedDescription( s );
	float skill = static_cast<float>( scourge->getParty()->getPlayer()->getSkill( Skill::LEADERSHIP ) );
	// level-based mark-up is already included and price is randomized
	int price = ( ( Item* )item )->getPrice();
	// 25% variance based on leadership skill.
	int percentage = static_cast<int>( static_cast<float>( price ) * ( 100.0f - skill ) / 100.0f * MAX_DISCOUNT );
	int total = price + ( widget == playerList ? ( -1 * percentage ) : percentage );
	prices[ ( Item* )item ] = total;
	char priceStr[20];
	snprintf( priceStr, 20, _( "$%d " ), total );
	buffer = priceStr + s;
}

void TradeDialog::trade() {
	if ( !validateBackpack() ) {
		scourge->showMessageDialog( _( "Inventories changed." ) );
		return;
	}

	int playerTotal = getSelectedTotal( playerList );
	int creatureTotal = getSelectedTotal( creatureList );
	if ( !playerTotal && !creatureTotal ) {
		scourge->showMessageDialog( _( "Select items to trade." ) );
		return;
	}

	// Do we have enough money?
	if ( ( scourge->getParty()->getPlayer()->getMoney() + playerTotal - creatureTotal ) < 0 ) {
		scourge->showMessageDialog( _( "You can't afford this deal!" ) );
		return;
	}

	// move items
	for ( int i = 0; i < playerList->getSelectedLineCount(); i++ ) {
		Item *item = playerList->getSelectedItem( i );
		scourge->getParty()->getPlayer()->removeFromBackpack( scourge->getParty()->getPlayer()->findInBackpack( item ) );
		creature->addToBackpack( item, true );
		scourge->getParty()->getPlayer()->setMoney( scourge->getParty()->getPlayer()->getMoney() + prices[ ( Item* )item ] );
	}

	for ( int i = 0; i < creatureList->getSelectedLineCount(); i++ ) {
		Item *item = creatureList->getSelectedItem( i );
		if ( !scourge->getPcUi()->addToBackpack( item ) ) {
			scourge->showMessageDialog( _( "Can't fit item in backpack." ) );
			return;
		}
		creature->removeFromBackpack( creature->findInBackpack( item ) );
		scourge->getParty()->getPlayer()->setMoney( scourge->getParty()->getPlayer()->getMoney() - prices[ ( Item* )item ] );
	}

	updateUI();
	scourge->refreshBackpackUI();
	scourge->showMessageDialog( _( "Selected items traded." ) );
}

/**
 * Steal the selected items from an npc.
 *
 * To steal successfully, you must roll a higher number on stealing+(luck/4) than
 * your oppoonent's (stealing/2)+(coordination/2)+(luck/4). The roll is made for
 * every item.
 *
 * Experience is awarded as follows:
 * If your max roll is larger, then you get your max roll minus the difference between
 * your max rolls. (So the closer your numbers are the harder the roll so less is taken
 * away.)
 * If your max roll is smaller or equal to your victim's max roll (you took a real risk)
 * you get the sum of the two max rolls.)
 *
 * FIXME: larger items should be harder to steal.
 */
void TradeDialog::steal() {
	if ( !validateBackpack() ) {
		scourge->showMessageDialog( _( "Inventories changed." ) );
		return;
	}

	int creatureTotal = getSelectedTotal( creatureList );
	if ( !creatureTotal ) {
		scourge->showMessageDialog( _( "Select items to steal." ) );
		return;
	}

	float steal = static_cast<float>( scourge->getParty()->getPlayer()->getSkill( Skill::STEALING ) );
	float luck = static_cast<float>( scourge->getParty()->getPlayer()->getSkill( Skill::LUCK ) );

	float stealB = static_cast<float>( creature->getSkill( Skill::STEALING ) );
	float coordinationB = static_cast<float>( creature->getSkill( Skill::COORDINATION ) );
	float luckB = static_cast<float>( creature->getSkill( Skill::LUCK ) );

	int price = 0;
	int exp = 0;
	bool success = true;
	for ( int i = 0; i < creatureList->getSelectedLineCount(); i++ ) {
		float maxA = steal + ( luck / 4.0f );
		float valueA = Util::roll( 0.75f * maxA, maxA );
		float maxB = ( stealB / 2.0f ) + ( coordinationB / 2.0f ) + ( luckB / 4.0f );
		float valueB = Util::roll( 0.75f * maxB, maxB );
		if ( success ) {
			success = valueA > valueB;
		}
		exp += static_cast<int>( maxA > maxB ? maxB : maxA + maxB );
		price += prices[ creatureList->getSelectedItem( i ) ];
	}

	char *p;
	if ( success ) {
		p = _( "You succesfully burgled the items!" );

		// move items
		for ( int i = 0; i < creatureList->getSelectedLineCount(); i++ ) {
			Item *item = creatureList->getSelectedItem( i );
			//scourge->getParty()->getPlayer()->addToBackpack( item, true );
			if ( scourge->getPcUi()->addToBackpack( item ) ) {
				creature->removeFromBackpack( creature->findInBackpack( item ) );
			} else {
				p = _( "You succesfully burgled the items but some did not fit in your backpack!" );
			}
		}

		// add experience (gain level, etc.)
		scourge->getParty()->getPlayer()->addExperienceWithMessage( exp );


	} else {
		// remove experience
		scourge->getParty()->getPlayer()->addExperienceWithMessage( -exp );

		// remove money
		int money =
		  ( scourge->getParty()->getPlayer()->getMoney() >= price ?
		    price :
		    scourge->getParty()->getPlayer()->getMoney() );
		price -= money;
		scourge->getParty()->getPlayer()->setMoney( scourge->getParty()->getPlayer()->getMoney() - money );
		enum { S_SIZE = 255 };
		char s[ S_SIZE ];
		snprintf( s, S_SIZE, _( "%s looses %d coins!" ), scourge->getParty()->getPlayer()->getName(), money );
		scourge->writeLogMessage( s );

		// remove some items
		for ( int i = 0; i < scourge->getParty()->getPlayer()->getBackpackContentsCount(); i++ ) {
			Item *item = scourge->getParty()->getPlayer()->getBackpackItem( i );
			if ( price - prices[ item ] > 0 ) {
				scourge->getParty()->getPlayer()->removeFromBackpack( i );
				creature->addToBackpack( item, true );
				price -= prices[ item ];
				snprintf( s, S_SIZE, _( "Item confiscated: %s" ), item->getRpgItem()->getDisplayName() );
				scourge->writeLogMessage( s );
			}
		}

		p = _( "Your burgling attempt failed. Items and ex. were confiscated." );
	}

	updateUI();
	scourge->refreshBackpackUI();
	scourge->showMessageDialog( p );
}

bool TradeDialog::validateBackpack() {
	for ( int i = 0; i < playerList->getSelectedLineCount(); i++ ) {
		Item *item = playerList->getSelectedItem( i );
		cerr << "item=" << item->getRpgItem()->getDisplayName() << " index=" << scourge->getParty()->getPlayer()->findInBackpack( item ) << endl;
		if ( scourge->getParty()->getPlayer()->findInBackpack( item ) == -1 ) return false;
	}
	for ( int i = 0; i < creatureList->getSelectedLineCount(); i++ ) {
		Item *item = creatureList->getSelectedItem( i );
		if ( creature->findInBackpack( item ) == -1 ) return false;
	}
	return true;
}

