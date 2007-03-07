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
#include "render/renderlib.h"

using namespace std;

TradeDialog::TradeDialog( Scourge *scourge ) {
  this->scourge = scourge;
  this->creature = NULL;
  
  win = 
    scourge->createWindow( 50, 50, 
                           605, 340, 
                           Constants::getMessage( Constants::TRADE_DIALOG_TITLE ) );
  //win->setModal( true );
	int xStart = 8;
  labelA = win->createLabel( xStart, 15, "" );
  totalA = win->createLabel( xStart, 28, _( "Selected Total:" ) );
  listA = new ItemList( scourge, win, 75, 35, 220, 210, this );
	listA->setAllowCursed( false );
  win->addWidget( listA );
  sellButton = win->createButton( xStart, 35, 70, 55, _( "Sell" ) );
  infoButtonA = win->createButton( xStart, 60, 70, 80, _( "Info" ) );

  
  labelB = win->createLabel( 305, 15, "" );
  totalB = win->createLabel( 305, 28, _( "Selected Total:" ) );
  listB = new ItemList( scourge, win, 300, 35, 220, 210, this );
	listB->setAllowCursed( false );
  win->addWidget( listB );
  tradeButton = win->createButton( 530, 35, 595, 55, _( "Buy" ) );
  stealButton = win->createButton( 530, 60, 595, 80, _( "Steal" ) );  
  infoButtonB = win->createButton( 530, 85, 595, 105, _( "Info" ) );

  closeButton = win->createButton( 530, 290, 595, 310, _( "Close" ) );
  
  coinAvailA = win->createLabel( xStart, 260, _( "Available Coins:" ) );
  coinTradeA = win->createLabel( xStart, 280, "$0" );
  coinReset = win->createButton( 180, 270, 220, 290, _( "Clr" ) );
  coinPlusA = win->createButton( 225, 270, 265, 290, "+1" );
  coinMinusA = win->createButton( 270, 270, 310, 290, "-1" );
  coinRest = win->createButton( 315, 270, 355, 290, _( "Diff" ) );

  win->createLabel( xStart, 305, _( "Shift+click to select multiple items, right click to get info." ) );
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
  listA->setCreature( scourge->getParty()->getPlayer(), creature->getNpcInfo()->getSubtype() );
  labelB->setText( creature->getName() );
  listB->setCreature( creature, creature->getNpcInfo()->getSubtype() );
  tradeA = 0;
  updateLabels();
}

void TradeDialog::updateLabels() {
  char tmp[120];
  sprintf( tmp, "%s $%d", _( "Available Coins:" ), scourge->getParty()->getPlayer()->getMoney() );
  coinAvailA->setText( tmp );
  sprintf( tmp, "%s $%d", _( "Selected Coins:" ), tradeA );
  coinTradeA->setText( tmp );
  sprintf( tmp, "%s $%d", _( "Selected Total:" ), ( getSelectedTotal( listA ) + tradeA ) );
  totalA->setText( tmp );
  sprintf( tmp, "%s $%d", _( "Selected Total:" ), getSelectedTotal( listB ) );
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
    setItem( listA->getSelectedItem( 0 ) );
    if( !scourge->getInfoGui()->getWindow()->isVisible() ) 
      scourge->getInfoGui()->getWindow()->setVisible( true );
  } else if( widget == infoButtonB && listB->getSelectedLineCount() ) {
    scourge->getInfoGui()->
    setItem( listB->getSelectedItem( 0 ) );
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
  float skill = (float)( scourge->getParty()->getPlayer()->getSkill( Skill::LEADERSHIP ) );
  // level-based mark-up is already included and price is randomized
  int price = ((Item*)item)->getPrice();
  // 25% variance based on leadership skill.
  int percentage = (int)( (float)price * ( 100.0f - skill ) / 100.0f * 0.25f );
  int total = price + ( widget == listA ? ( -1 * percentage ) : percentage );
  prices[ (Item*)item ] = total;
  sprintf( (char*)buffer, "$%d %s", total, s );
}

void TradeDialog::trade() {
  if( !validateInventory() ) {
    scourge->showMessageDialog( _( "Inventories changed." ) );
    return;
  }
  
  int totalA = getSelectedTotal( listA ) + tradeA;
  int totalB = getSelectedTotal( listB );
  if( !totalB ) {
    scourge->showMessageDialog( _( "Select items to buy." ) );
    return;
  } else if( totalA < totalB ) {
    scourge->showMessageDialog( _( "You are not offering enough to trade." ) );
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
  scourge->showMessageDialog( _( "Selected items traded." ) );
}

void TradeDialog::sell() {
  if( !validateInventory() ) {
    scourge->showMessageDialog( _( "Inventories changed." ) );
    return;
  }
  
  int totalA = getSelectedTotal( listA );
  if( !totalA ) {
    scourge->showMessageDialog( _( "Select items to sell." ) );
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
  scourge->showMessageDialog( _( "Selected items sold." ) );
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
  if( !validateInventory() ) {
    scourge->showMessageDialog( _( "Inventories changed." ) );
    return;
  }

  int totalB = getSelectedTotal( listB );
  if( !totalB ) {
    scourge->showMessageDialog( _( "Select items to steal." ) );
    return;
  }
  
  float steal = (float)( scourge->getParty()->getPlayer()->getSkill( Skill::STEALING ) );
  float luck = (float)( scourge->getParty()->getPlayer()->getSkill( Skill::LUCK ) );

  float stealB = (float)( creature->getSkill( Skill::STEALING ) );
  float coordinationB = (float)( creature->getSkill( Skill::COORDINATION ) );
  float luckB = (float)( creature->getSkill( Skill::LUCK ) );

  int price = 0;
  int exp = 0;
  bool success = true;
  for( int i = 0; i < listB->getSelectedLineCount(); i++ ) {
    float maxA = steal + ( luck / 4.0f );
    float valueA = maxA - ( ( maxA / 4.0f ) * rand() / RAND_MAX );
    float maxB = ( stealB / 2.0f ) + ( coordinationB / 2.0f ) + ( luckB / 4.0f );
    float valueB = maxB - ( ( maxB / 4.0f ) * rand() / RAND_MAX );
    if( success ) {
      success = ( valueA > valueB ? true : false );
    }
    exp += (int)( maxA > maxB ? 
                  maxA - ( maxA - maxB ) : 
                  maxA + maxB );
    price += prices[ listB->getSelectedItem( i ) ];
  }
  
  char *p;
  if( success ) {
    // move items
    for( int i = 0; i < listB->getSelectedLineCount(); i++ ) {
      Item *item = listB->getSelectedItem( i );
      creature->removeInventory( creature->findInInventory( item ) );
      scourge->getParty()->getPlayer()->addInventory( item, true );
    }

    // add experience (gain level, etc.)
    scourge->getParty()->getPlayer()->addExperienceWithMessage( exp );

    p = _( "You succesfully burgled the items!" );
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
    char s[ 255 ];
    sprintf( s, _( "%s looses %d coins!" ), scourge->getParty()->getPlayer()->getName(), money );
    scourge->getMap()->addDescription( s, 1, 0.05f, 0.05f );

    // remove some items
    for( int i = 0; i < scourge->getParty()->getPlayer()->getInventoryCount(); i++ ) {
      Item *item = scourge->getParty()->getPlayer()->getInventory( i );
      if( price - prices[ item ] > 0 ) {
        scourge->getParty()->getPlayer()->removeInventory( i );
        creature->addInventory( item, true );
        price -= prices[ item ];
        sprintf( s, _( "Item confiscated: %s" ), item->getRpgItem()->getName() );
        scourge->getMap()->addDescription( s, 1, 0.05f, 0.05f );
      }
    }

    p = _( "Your burgling attempt failed. Items and ex. were confiscated." );
  }

  updateUI();
  scourge->refreshInventoryUI();
  scourge->showMessageDialog( p );
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

