/***************************************************************************
tradedialog.cpp  -  description
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
#include "rechargedialog.h"
#include "scourge.h"
#include "creature.h"
#include "item.h"
#include "gui/window.h"
#include "rpg/rpglib.h"
#include "render/renderlib.h"

using namespace std;

RechargeDialog::RechargeDialog( Scourge *scourge ) {
  this->scourge = scourge;
  this->creature = NULL;

  int w = 400;
  int h = 328;
  
  win = 
    scourge->createWindow( 50, 50, 
                           w, h, 
                           Constants::getMessage( Constants::RECHARGE_DIALOG_TITLE ) );

  int xStart = 13;
  
  labelA = win->createLabel( xStart, 15, "" );
  totalA = win->createLabel( xStart, 28, _( "Selected Total:" ) );
  listA = new ItemList( scourge, win, xStart, 35, w - ( xStart * 2 ), 210, this );
	listA->setNeedsRechargeOnly( true );
  win->addWidget( listA );
  rechargeButton = win->createButton( xStart, 274, 110, 294, _( "Recharge" ) );
  infoButtonA = win->createButton( 115, 274, 187, 294, _( "Info" ) );

  closeButton = win->createButton( 315, 274, 387, 294, _( "Close" ) );
  
  coinAvailA = win->createLabel( xStart, 260, _( "Available Coins:" ) );
}

RechargeDialog::~RechargeDialog() {
  delete win;
}

void RechargeDialog::setCreature( Creature *creature ) {
  this->creature = creature;
  win->setVisible( true );
  updateUI();
}

void RechargeDialog::updateUI() {
  prices.clear();  
  labelA->setText( scourge->getParty()->getPlayer()->getName() );
  listA->setCreature( scourge->getParty()->getPlayer(), creature->getNpcInfo()->getSubtype() );
  tradeA = 0;
  updateLabels();
}

void RechargeDialog::updateLabels() {
	enum { TMP_SIZE = 120 };
  char tmp[ TMP_SIZE ];
  snprintf( tmp, TMP_SIZE, _( "%s $%d" ), _( "Available Coins:" ), scourge->getParty()->getPlayer()->getMoney() );
  coinAvailA->setText( tmp );
  snprintf( tmp, TMP_SIZE, _( "%s $%d" ), _( "Selected Total:" ), ( getSelectedTotal( listA ) + tradeA ) );
  totalA->setText( tmp );
}

int RechargeDialog::getSelectedTotal( ItemList *list ) {
  int total = 0;
  for( int i = 0; i < list->getSelectedLineCount(); i++ ) {
    Item *item = list->getSelectedItem( i );
    total += prices[ item ];
  }
  return total;
}

void RechargeDialog::handleEvent( Widget *widget, SDL_Event *event ) {
  if( widget == win->closeButton || widget == closeButton ) {
    win->setVisible( false );
  } else if( widget == infoButtonA &&listA->getSelectedLineCount() ) {
    scourge->getInfoGui()->
    setItem( listA->getSelectedItem( 0 ) );
    if( !scourge->getInfoGui()->getWindow()->isVisible() ) 
      scourge->getInfoGui()->getWindow()->setVisible( true );
  } else if( widget == listA ) {
    updateLabels();
  } else if( widget == rechargeButton ) {
    recharge();
  }
}

void RechargeDialog::render( const Widget *widget, const Item *item, std::string& buffer ) {
	std::string s;
  ((Item*)item)->getDetailedDescription( s );
  float skill = (float)( scourge->getParty()->getPlayer()->getSkill( Skill::LEADERSHIP ) );
  // level-based mark-up is already included and price is randomized
  int reqCharges = ((Item*)item)->getRpgItem()->getMaxCharges() - ((Item*)item)->getCurrentCharges();
  int pricePerCharge = ((Item*)item)->getPrice() / ((Item*)item)->getRpgItem()->getMaxCharges() / 2;
  int price = reqCharges * pricePerCharge;
  // 25% variance based on leadership skill.
  int percentage = (int)( (float)price * ( 100.0f - skill ) / 100.0f * 0.25f );
  int total = price + percentage;
  prices[ (Item*)item ] = total;

  char priceStr[20];
	snprintf( priceStr, 20, _( "$%d " ), total );
	buffer = priceStr + s;
}

void RechargeDialog::recharge() {
  if( !validateInventory() ) {
    scourge->showMessageDialog( _( "Inventories changed." ) );
    return;
  }
  
  int totalA = getSelectedTotal( listA );
  if( !totalA ) {
    scourge->showMessageDialog( _( "Select items to recharge." ) );
    return;
  } else if( scourge->getParty()->getPlayer()->getMoney() < totalA ) {
    scourge->showMessageDialog( _( "You can't afford the recharging." ) );
    return;
  }
  
  // recharge items
  for( int i = 0; i < listA->getSelectedLineCount(); i++ ) {
    Item *item = listA->getSelectedItem( i );
    item->setCurrentCharges(item->getRpgItem()->getMaxCharges());
  }
  
  // move money
  scourge->getParty()->getPlayer()->setMoney( scourge->getParty()->getPlayer()->getMoney() - totalA );
  
  updateUI();
  scourge->refreshInventoryUI();
  scourge->showMessageDialog( _( "Selected items recharged." ) );
}

bool RechargeDialog::validateInventory() {
  for( int i = 0; i < listA->getSelectedLineCount(); i++ ) {
    Item *item = listA->getSelectedItem( i );
    cerr << "item=" << item->getRpgItem()->getDisplayName() << " index=" << scourge->getParty()->getPlayer()->findInInventory( item ) << endl;
    if( scourge->getParty()->getPlayer()->findInInventory( item ) == -1 ) return false;
  }
  return true;
}
