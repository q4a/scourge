/***************************************************************************
                          item.h  -  description
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

#ifndef ITEM_H
#define ITEM_H

#include "constants.h"
#include "glshape.h"
#include "shapepalette.h"
#include "rpg/rpgitem.h"

class Scourge;

/**
  *@author Gabor Torok

  This class is both the UI representation (shape) of the rpgItem and it's state (for example, wear).
  All instances of the RpgItem point to the same RpgItem, but a new Item is created for each.

  */

class Item {
private:
  RpgItem *rpgItem;
  
public:
  Item(RpgItem *rpgItem);
  ~Item();
  
  inline GLShape *getShape() { return ShapePalette::getInstance()->getItemShape(this->rpgItem->getShapeIndex()); }
  inline RpgItem *getRpgItem() { return rpgItem; }
};

#endif
