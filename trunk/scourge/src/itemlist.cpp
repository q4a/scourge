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

ItemList::ItemList( Scourge *scourge, Window *win, int x, int y, int width, int height, ItemRenderer *itemRenderer ) : 
ScrollingList( x, y, width, height, scourge->getShapePalette()->getHighlightTexture(), NULL, 30 ) {
  this->scourge = scourge;
  this->win = win;
  this->itemRenderer = itemRenderer;
  this->creature = creature;
  this->container = NULL;

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

void ItemList::setCreature( Creature *creature ) {
  this->creature = creature;
  this->container = NULL;
  commonInit();
}

void ItemList::setContainer( Item *container ) {
  this->container = container;
  this->creature = NULL;
  commonInit();
}

void ItemList::commonInit() {
  char s[120];
  for(int t = 0; t < getItemCount(); t++) {
    Item *item = getItem( t );
    if( itemRenderer ) {
      itemRenderer->render( this, item, 120, (const char *)name[ t ] );
    } else {
      item->getDetailedDescription( s );
      //sprintf( itemA[t], "%s %s", ( creature->getEquippedIndex(t) > -1 ? "(E)" : "" ), s );
      sprintf( name[ t ], "%s", s );
    }
    if( !item->isMagicItem() ) {
      if( win->getTheme()->getWindowText() ) {
        color[t].r = win->getTheme()->getWindowText()->r;
        color[t].g = win->getTheme()->getWindowText()->g;
        color[t].b = win->getTheme()->getWindowText()->b;
      } else {
        color[t].r = 0;
        color[t].g = 0;
        color[t].b = 0;
      }
    } else {
      color[t].r = Constants::MAGIC_ITEM_COLOR[ item->getMagicLevel() ]->r;
      color[t].g = Constants::MAGIC_ITEM_COLOR[ item->getMagicLevel() ]->g;
      color[t].b = Constants::MAGIC_ITEM_COLOR[ item->getMagicLevel() ]->b;
    }
    color[t].a = 1;
    icon[t] = scourge->getShapePalette()->tilesTex[ item->getRpgItem()->getIconTileX() ][ item->getRpgItem()->getIconTileY() ];
  }    
  for(int t = getItemCount(); t < MAX_INVENTORY_SIZE; t++) {
    strcpy( name[t], "" );
  }  
  setLines( creature->getInventoryCount(), 
                  (const char **)name,
                  color, icon );
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
             scourge->getParty()->getPlayer()->getSkill( Constants::IDENTIFY_ITEM_SKILL ) );
    if( !scourge->getInfoGui()->getWindow()->isVisible() ) 
      scourge->getInfoGui()->getWindow()->setVisible( true );
  }
  return ret;
}

