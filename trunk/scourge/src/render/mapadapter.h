/***************************************************************************
                          mapadapter.h  -  description
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

#ifndef MAP_ADAPTER_H
#define MAP_ADAPTER_H

#include "render.h"

class RenderedCreature;
class Shape;
class Location;
class RenderedItem;

using namespace std;

class MapAdapter {
public:
  MapAdapter();
  virtual ~MapAdapter();

  virtual int getScreenWidth() = 0;
  virtual int getScreenHeight() = 0;
  virtual Uint16 getMouseX() = 0;
  virtual Uint16 getMouseY() = 0;
  virtual bool isMouseIsMovingOverMap() = 0;
  virtual RenderedCreature *getPlayer() = 0;
  virtual void setDebugStr(char *s) = 0;
  virtual bool isMissionCreature( RenderedCreature *creature ) = 0;
  virtual void colorMiniMapPoint(int x, int y, Shape *shape, Location *pos=NULL) = 0;
  virtual void eraseMiniMapPoint(int x, int y) = 0;
  virtual bool hasParty() = 0;
  virtual int getPartySize() = 0;
  virtual RenderedCreature *getParty( int index ) = 0;
  virtual RenderedItem *load( ItemInfo *info ) = 0;
  virtual RenderedCreature *load( CreatureInfo *info ) = 0;
  virtual void loadMapData( const char *name ) = 0;
  virtual void saveMapData( const char *name ) = 0;

  /**
   * Set up the opengl view.
   */
  virtual void setView() = 0;
};

#endif
