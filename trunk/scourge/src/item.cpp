/***************************************************************************
                          item.cpp  -  description
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

#include "item.h"

Item::Item(RpgItem *rpgItem) {
  this->rpgItem = rpgItem;
  this->shapeIndex = this->rpgItem->getShapeIndex();
  this->color = NULL;
  this->shape = ShapePalette::getInstance()->getItemShape(shapeIndex);
  // for now objects larger than 1 height will block (we can change this later)
  // this is so the player is not blocked by swords and axes on the ground
  this->blocking = (shape->getHeight() > 1);
}

Item::~Item(){
}
