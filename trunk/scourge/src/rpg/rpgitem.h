/***************************************************************************
                          rpgitem.cpp  -  description
                             -------------------
    begin                : Sun Sep 28 2003
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
#ifndef RPG_ITEM_H
#define RPG_ITEM_H

#include "../constants.h"

class RpgItem {
 private:
  char *name, *desc, *shortDesc;
  int weight, price, quality;
  int action; // damage, defence, potion str.
  int shape_index;

 public:
  enum itemNames {
    SHORT_SWORD=0,
    DAGGER,
    BASTARD_SWORD,
	CHEST,
	BOOKSHELF,
	CHEST2,
	BOOKSHELF2,
	
	// must be the last ones
	ITEM_COUNT
  };

  static RpgItem *items[];
  
  RpgItem(char *name, int weight, int price, int quality, 
		  int action, char *desc, char *shortDesc, int shape_index);
  ~RpgItem();

  inline int getShapeIndex() { return shape_index; }
  inline char *getShortDesc() { return shortDesc; }
};

#endif
