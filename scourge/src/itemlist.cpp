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

  color = (Color*)malloc( MAX_INVENTORY_SIZE * sizeof( Color ) );
  name = (char**)malloc( MAX_INVENTORY_SIZE * sizeof( char* ) );
  icon = (GLuint*)malloc( MAX_INVENTORY_SIZE * sizeof( GLuint ) );
  for( int i = 0; i < MAX_INVENTORY_SIZE; i++ ) {
    name[i] = (char*)malloc( 120 * sizeof( char ) );
  }
  
  setAllowMultipleSelection( true );
}

ItemList::~ItemList() {
  for( int i = 0; i < MAX_INVENTORY_SIZE; i++ ) free( name[i] );
  free( name );
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
  char s[120];
  int count = 0;
  for(int t = 0; t < getItemCount(); t++) {
    Item *item = getItem( t );

    // if there is a non-empty filter, it should contain this type of item
    if( filter && filter->size() && 
        filter->find( item->getRpgItem()->getType() ) == filter->end() ) {
      continue;
    }

    items.push_back( item );

    if( itemRenderer ) {
      itemRenderer->render( this, item, 120, (const char *)name[ count ] );
    } else {
      item->getDetailedDescription( s );
      //sprintf( itemA[t], "%s %s", ( creature->getEquippedIndex(t) > -1 ? "(E)" : "" ), s );
      sprintf( name[ count ], "%s", s );
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
    strcpy( name[t], "" );
  }  
  setLines( count, (const char **)name, color, icon );
  //setSelectedLine( 0 );
}

char *ItemList::getName() {
  return( creature ? creature->getName() : container->getRpgItem()->getName() );
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
    setItem( getSelectedItem( 0 ), 
             scourge->getParty()->getPlayer()->getSkill( Skill::IDENTIFY_ITEM ) );
    if( !scourge->getInfoGui()->getWindow()->isVisible() ) 
      scourge->getInfoGui()->getWindow()->setVisible( true );
  }
  return ret;
}

