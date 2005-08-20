/***************************************************************************
                          rendereditem.h  -  description
                             -------------------
    begin                : Sat May 3 2003
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

#ifndef RENDERED_ITEM_H
#define RENDERED_ITEM_H

#include "../constants.h"
#include "../persist.h"

class GLShape;

/**
 * @author Gabor Torok
 * 
 * A creature rendered on the map.
 */

class RenderedItem {
public:
  RenderedItem() {}
  virtual ~RenderedItem() {}

  virtual GLShape *getShape() = 0;
  virtual bool isMagicItem() = 0;
  virtual int getMagicLevel() = 0;
  virtual bool getContainsMagicItem() = 0;
  virtual bool isBlocking() = 0;
  virtual char *getItemName() = 0;
  virtual ItemInfo *save() = 0;
  virtual int getDistance() = 0;
};


#endif

