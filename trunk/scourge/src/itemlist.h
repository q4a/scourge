/***************************************************************************
itemlist.h  -  description
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

#ifndef ITEM_LIST_H
#define ITEM_LIST_H

#include "common/constants.h"
#include "gui/scrollinglist.h"
#include <set>

class Scourge;
class Creature;
class Item;
class Window;
class Label;

/**
Add arbitrary rendering to displaying items in the list. For example, the trade 
 dialog displays the item's price next to the name.
 */
class ItemRenderer {
public:
  ItemRenderer() {
  }
  
  virtual ~ItemRenderer() {
  }
  
  virtual void render( const Widget *widget, const Item *item, int bufferSize, const char *buffer ) = 0;
};

/**
A special scrolling list that shows a list of items. It can be used to show a creature's
 inventory or the contents of a container. The class shows items with the right color and
 icon. It also supports showing the item info dialog when right-clicking an item.
 */
class ItemList : public ScrollingList {
private:
  Scourge *scourge;
  Creature *creature;
  Item *container;
  Window *win;
  ItemRenderer *itemRenderer;
  std::set<int> *filter;
  std::vector<Item*> items;
	bool allowCursed;
  
	char **name;
	Color *color;
  GLuint *icon;
  
public:
  ItemList( Scourge *scourge, Window *win, int x, int y, int width, int height, ItemRenderer *itemRenderer = NULL );
  ~ItemList();
  
  void setCreature( Creature *creature, std::set<int> *filter = NULL );
  inline Creature *getCreature() { return creature; }
  void setContainer( Item *container, std::set<int> *filter = NULL );
  inline Item *getContainer() { return container; }

	inline void setAllowCursed( bool b ) { allowCursed = b; }
  
  char *getName();
  int getItemCount();
  Item *getItem( int index );
  
  virtual bool handleEvent(Widget *parent, SDL_Event *event, int x, int y);

  inline Item *getSelectedItem( int index ) { 
    if( index < 0 || index >= getSelectedLineCount() ) return NULL;
    return items[ getSelectedLine( index ) ];
  }
  
protected:
  void commonInit();
};

#endif