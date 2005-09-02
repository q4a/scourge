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
#include "tradedialog.h"
#include "scourge.h"
#include "creature.h"
#include "item.h"
#include "gui/window.h"
#include "rpg/rpglib.h"

#define AVAILABLE_COINS "Available Coins:"
#define TOTAL "Selected Total:"
#define SELECTED_COINS "Selected Coins:"

TradeDialog::TradeDialog( Scourge *scourge ) {
  this->scourge = scourge;
  this->creature = NULL;
  
  win = 
    scourge->createWindow( 50, 50, 
                           605, 320, 
                           Constants::getMessage( Constants::TRADE_DIALOG_TITLE ) );
  //win->setModal( true );
  labelA = win->createLabel( 5, 15, "" );
  totalA = win->createLabel( 5, 28, TOTAL );
  listA = new ItemList( scourge, win, 75, 35, 225, 210, this );
  win->addWidget( listA );
  sellButton = win->createButton( 5, 35, 70, 55, "Sell" );
  infoButtonA = win->createButton( 5, 60, 70, 80, "Info" );

  
  labelB = win->createLabel( 305, 15, "" );
  totalB = win->createLabel( 305, 28, TOTAL );
  listB = new ItemList( scourge, win, 305, 35, 225, 210, this );
  win->addWidget( listB );
  tradeButton = win->createButton( 535, 35, 600, 55, "Buy" );
  stealButton = win->createButton( 535, 60, 600, 80, "Steal" );  
  infoButtonB = win->createButton( 535, 85, 600, 105, "Info" );

  closeButton = win->createButton( 535, 270, 600, 290, "Close" );
  
  coinAvailA = win->createLabel( 5, 260, AVAILABLE_COINS );
  coinTradeA = win->createLabel( 5, 280, "$0" );
  coinReset = win->createButton( 135, 270, 160, 290, "Clr" );
  coinPlusA = win->createButton( 165, 270, 195, 290, "+1" );
  coinMinusA = win->createButton( 200, 270, 230, 290, "-1" );
  coinRest = win->createButton( 235, 270, 265, 290, "Diff" );
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
  labelA->setText( scourge->getParty()->getPlayer()->getName() );
  listA->setCreature( scourge->getParty()->getPlayer() );
  labelB->setText( creature->getName() );
  listB->setCreature( creature );
  tradeA = 0;
  updateLabels();
}

void TradeDialog::updateLabels() {
  char tmp[120];
  sprintf( tmp, "%s $%d", AVAILABLE_COINS, scourge->getParty()->getPlayer()->getMoney() );
  coinAvailA->setText( tmp );
  sprintf( tmp, "%s $%d", SELECTED_COINS, tradeA );
  coinTradeA->setText( tmp );
  sprintf( tmp, "%s $%d", TOTAL, ( getSelectedTotal( listA ) + tradeA ) );
  totalA->setText( tmp );
  sprintf( tmp, "%s $%d", TOTAL, getSelectedTotal( listB ) );
  totalB->setText( tmp );
}

int TradeDialog::getSelectedTotal( ItemList *list ) {
  int total = 0;
  for( int i = 0; i < list->getSelectedLineCount(); i++ ) {
    Item *item = list->getSelectedItem( i );
    total += prices[ item ];
  }
  return total;
}

void TradeDialog::handleEvent( Widget *widget, SDL_Event *event ) {
  if( widget == win->closeButton || widget == closeButton ) {
    win->setVisible( false );
  } else if( widget == infoButtonA &&listA->getSelectedLineCount() ) {
    scourge->getInfoGui()->
    setItem( listA->getSelectedItem( 0 ), 
             scourge->getParty()->getPlayer()->getSkill( Constants::IDENTIFY_ITEM_SKILL ) );
    if( !scourge->getInfoGui()->getWindow()->isVisible() ) 
      scourge->getInfoGui()->getWindow()->setVisible( true );
  } else if( widget == infoButtonB && listB->getSelectedLineCount() ) {
    scourge->getInfoGui()->
    setItem( listB->getSelectedItem( 0 ), 
             scourge->getParty()->getPlayer()->getSkill( Constants::IDENTIFY_ITEM_SKILL ) );
    if( !scourge->getInfoGui()->getWindow()->isVisible() ) 
      scourge->getInfoGui()->getWindow()->setVisible( true );
  } else if( widget == listA || widget == listB ) {
    updateLabels();
  } else if( widget == coinPlusA && tradeA + 1 < scourge->getParty()->getPlayer()->getMoney() ) {
    tradeA++;
    updateLabels(); 
  } else if( widget == coinMinusA && tradeA ) {
    tradeA--;
    updateLabels();
  } else if( widget == coinReset ) {
    tradeA = 0;
    updateLabels();
  } else if( widget == coinRest ) {
    int total = getSelectedTotal( listB ) - getSelectedTotal( listA );
    tradeA = ( total < scourge->getParty()->getPlayer()->getMoney() ? 
               total : 
               scourge->getParty()->getPlayer()->getMoney() );
    if( tradeA < 0 ) tradeA = 0;
    updateLabels();
  } else if( widget == tradeButton ) {
    trade();
  } else if( widget == sellButton ) {
    sell();
  } else if( widget == stealButton ) {
    steal();
  }
}

void TradeDialog::render( const Widget *widget, const Item *item, int bufferSize, const char *buffer ) {
  char s[ 120 ];
  ((Item*)item)->getDetailedDescription( s );
  float skill = (float)( scourge->getParty()->getPlayer()->getSkill( Constants::LEADERSHIP ) );
  // level-based mark-up is already included
  int price = ((Item*)item)->getPrice();
  int percentage = (int)( (float)price * ( ( 100.0f - skill ) * random() / RAND_MAX ) / 100.0f * 0.5f );
  int total = price + ( widget == listA ? ( -1 * percentage ) : percentage );
  prices[ (Item*)item ] = total;
  sprintf( (char*)buffer, "$%d %s", total, s );
}

void TradeDialog::trade() {
  if( !validateInventory() ) {
    scourge->showMessageDialog( "Inventories changed." );
    return;
  }
  
  int totalA = getSelectedTotal( listA ) + tradeA;
  int totalB = getSelectedTotal( listB );
  if( !totalB ) {
    scourge->showMessageDialog( "Select items to buy." );
    return;
  } else if( totalA < totalB ) {
    scourge->showMessageDialog( "You are not offering enough to trade." );
    return;
  } else if( totalA > totalB ) {
    // FIXME: show are you sure? dialog.
  }
  
  // move items
  for( int i = 0; i < listA->getSelectedLineCount(); i++ ) {
    Item *item = listA->getSelectedItem( i );
    scourge->getParty()->getPlayer()->removeInventory( scourge->getParty()->getPlayer()->findInInventory( item ) );
    creature->addInventory( item, true );
  }
  for( int i = 0; i < listB->getSelectedLineCount(); i++ ) {
    Item *item = listB->getSelectedItem( i );
    creature->removeInventory( creature->findInInventory( item ) );
    scourge->getParty()->getPlayer()->addInventory( item, true );
  }
  
  // move money
  scourge->getParty()->getPlayer()->setMoney( scourge->getParty()->getPlayer()->getMoney() - tradeA );
  if( totalA > totalB ) 
    scourge->getParty()->getPlayer()->setMoney( scourge->getParty()->getPlayer()->getMoney() + 
                                    ( totalA - totalB ) );
  
  updateUI();
  scourge->refreshInventoryUI();
  scourge->showMessageDialog( "Selected items traded." );
}

void TradeDialog::sell() {
  if( !validateInventory() ) {
    scourge->showMessageDialog( "Inventories changed." );
    return;
  }
  
  int totalA = getSelectedTotal( listA );
  if( !totalA ) {
    scourge->showMessageDialog( "Select items to sell." );
    return;
  }
  
  // move items
  for( int i = 0; i < listA->getSelectedLineCount(); i++ ) {
    Item *item = listA->getSelectedItem( i );
    scourge->getParty()->getPlayer()->removeInventory( scourge->getParty()->getPlayer()->findInInventory( item ) );
    creature->addInventory( item, true );
  }
  
  // move money
  scourge->getParty()->getPlayer()->setMoney( scourge->getParty()->getPlayer()->getMoney() + totalA );
  
  updateUI();
  scourge->refreshInventoryUI();
  scourge->showMessageDialog( "Selected items sold." );
}

void TradeDialog::steal() {
  if( !validateInventory() ) {
    scourge->showMessageDialog( "Inventories changed." );
    return;
  }

  int totalB = getSelectedTotal( listB );
  if( !totalB ) {
    scourge->showMessageDialog( "Select items to steal." );
    return;
  }
  
  bool success = true;
  for( int i = 0; i < listB->getSelectedLineCount(); i++ ) {
    //Item *item = listB->getSelectedItem( i );
    float steal = (float)( scourge->getParty()->getPlayer()->getSkill( Constants::STEALING ) );
    float luck = (float)( scourge->getParty()->getPlayer()->getSkill( Constants::LUCK ) );
    success = ( ( ( steal + ( luck / 2.0f ) ) * rand() / RAND_MAX ) > 50.0f ? true : false );
    if( !success ) break;
  }
  
  // FIXME:
  scourge->showMessageDialog( success ? (char*)"FIXME: Success" : (char*)"FIXME: Failed" );
}

bool TradeDialog::validateInventory() {
  for( int i = 0; i < listA->getSelectedLineCount(); i++ ) {
    Item *item = listA->getSelectedItem( i );
    cerr << "item=" << item->getRpgItem()->getName() << " index=" << scourge->getParty()->getPlayer()->findInInventory( item ) << endl;
    if( scourge->getParty()->getPlayer()->findInInventory( item ) == -1 ) return false;
  }
  for( int i = 0; i < listB->getSelectedLineCount(); i++ ) {
    Item *item = listB->getSelectedItem( i );
    if( creature->findInInventory( item ) == -1 ) return false;
  }
  return true;
}
