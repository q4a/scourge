/***************************************************************************
itemlist.cpp  -  description
-------------------
    begin                : 8/27/2005
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
#include "itemlist.h"
#include "scourge.h"
#include "creature.h"
#include "item.h"
#include "rpg/rpglib.h"
#include "gui/window.h"
#include "gui/scrollinglist.h"
#include "gui/label.h"
#include "shapepalette.h"

using namespace std;

ItemList::ItemList( Scourge *scourge, Window *win, int x, int y, int width, int height, ItemRenderer *itemRenderer ) : 
ScrollingList( x, y, width, height, scourge->getShapePalette()->getHighlightTexture(), NULL, 30 ) {
  this->scourge = scourge;
  this->win = win;
  this->itemRenderer = itemRenderer;
  this->creature = creature;
  this->container = NULL;
  this->filter = NULL;
	this->unidentifiedOnly = false;
	this->needsRechargeOnly = false;
	this->cursedOnly = false;
	this->allowCursed = true;
	this->allowEquipped = true;

  color = (Color*)malloc( MAX_INVENTORY_SIZE * sizeof( Color ) );
  icon = (GLuint*)malloc( MAX_INVENTORY_SIZE * sizeof( GLuint ) );
  
  setAllowMultipleSelection( true );
}

ItemList::~ItemList() {
  free( color );
  free( icon );
}

void ItemList::setCreature( Creature *creature, set<int> *filter ) {
  this->creature = creature;
  this->container = NULL;
  this->filter = filter;
  this->items.clear();
  commonInit();
}

void ItemList::setContainer( Item *container, set<int> *filter ) {
  this->container = container;
  this->creature = NULL;
  this->filter = filter;
  this->items.clear();
  commonInit();
}

void ItemList::commonInit() {
  int count = 0;
  for(int t = 0; t < getItemCount(); t++) {
    Item *item = getItem( t );

    // Skip equipped items if they are not to be shown
    if( !allowEquipped && creature->isEquipped( item ) ) continue;

    // if there is a non-empty filter, it should contain this type of item
    if( filter && !filter->empty() && 
        filter->find( item->getRpgItem()->getType() ) == filter->end() ) {
      continue;
    }

    // show only cursed items?
    if ( cursedOnly && !item->isCursed() ) {
      continue;
    // show only unidentified items?
    } else if ( unidentifiedOnly && item->isFullyIdentified() ) {
      continue;
    // show only items that need recharge?
    } else if ( needsRechargeOnly && ( ( item->getCurrentCharges() >= item->getRpgItem()->getMaxCharges() ) || !item->isMagicItem() ) ) {
      continue;
    // cursed items?
    } else if ( !allowCursed && item->isCursed() ) {
      continue;
    }

    items.push_back( item );

    if( itemRenderer ) {
      itemRenderer->render( this, item, name[ count ] );
    } else {
      item->getDetailedDescription( name[ count ] );
    }
    if( !item->isMagicItem() ) {
      if( win->getTheme()->getWindowText() ) {
        color[count].r = win->getTheme()->getWindowText()->r;
        color[count].g = win->getTheme()->getWindowText()->g;
        color[count].b = win->getTheme()->getWindowText()->b;
      } else {
        color[count].r = 0;
        color[count].g = 0;
        color[count].b = 0;
      }
    } else {
      color[count].r = Constants::MAGIC_ITEM_COLOR[ item->getMagicLevel() ]->r;
      color[count].g = Constants::MAGIC_ITEM_COLOR[ item->getMagicLevel() ]->g;
      color[count].b = Constants::MAGIC_ITEM_COLOR[ item->getMagicLevel() ]->b;
    }
    color[count].a = 1;
    icon[count] = scourge->getShapePalette()->tilesTex[ item->getRpgItem()->getIconTileX() ][ item->getRpgItem()->getIconTileY() ];
    count++;
  }    
  for(int t = count; t < MAX_INVENTORY_SIZE; t++) {
    name[t].clear();
  }  
  setLines( count, name, color, icon );
}

char *ItemList::getName() {
  return( creature ? creature->getName() : container->getRpgItem()->getDisplayName() );	
}

int ItemList::getItemCount() {
  return( creature ? creature->getInventoryCount() : container->getContainedItemCount() );
}

Item *ItemList::getItem( int index ) {
  return( creature ? creature->getInventory( index ) : container->getContainedItem( index ) );
}

bool ItemList::handleEvent( Widget *parent, SDL_Event *event, int x, int y ) {
  bool ret = ScrollingList::handleEvent( parent, event, x, y );
  if( isInside( x, y ) && getSelectedLineCount() && event->button.button == SDL_BUTTON_RIGHT ) {
    scourge->getInfoGui()->
    setItem( getSelectedItem( 0 ) );
    if( !scourge->getInfoGui()->getWindow()->isVisible() ) 
      scourge->getInfoGui()->getWindow()->setVisible( true );
  }
  return ret;
}
