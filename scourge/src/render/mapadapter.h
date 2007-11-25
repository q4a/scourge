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

class MapAdapter {
public:
  MapAdapter();
  virtual ~MapAdapter();

	virtual void setUpdate( char *message, int n, int total ) = 0;
	virtual void setCursorVisible( bool b ) = 0;
  virtual int getScreenWidth() = 0;
  virtual int getScreenHeight() = 0;
  virtual Uint16 getMouseX() = 0;
  virtual Uint16 getMouseY() = 0;
  virtual bool isMouseIsMovingOverMap() = 0;
  virtual RenderedCreature *getPlayer() = 0;
  virtual void setDebugStr(char *s) = 0;
  virtual bool isMissionCreature( RenderedCreature *creature ) = 0;
  virtual bool hasParty() = 0;
  virtual int getPartySize() = 0;
  virtual RenderedCreature *getParty( int index ) = 0;
  virtual RenderedItem *createItem( char *item_name, int level, int depth ) = 0;
  virtual RenderedCreature *createMonster( char *monster_name ) = 0;
  virtual RenderedItem *createItem( ItemInfo *info ) = 0;
  virtual RenderedCreature *createMonster( CreatureInfo *info ) = 0;
  virtual void loadMapData( const std::string& name ) = 0;
  virtual void saveMapData( const std::string& name ) = 0;
	virtual char *getMagicSchoolIndexForLocation( Location *pos ) = 0;
	virtual void setMagicSchoolIndexForLocation( Location *pos, char *magicSchoolName ) = 0;

  /**
   * What color to outline this location with? If NULL is returned
   * the location is not outlined.
   */
  virtual Color *getOutlineColor( Location *pos ) = 0;

  /**
   * Set up the opengl view.
   */
  virtual void setView() = 0;

  /**
   * Is the oval cutout shown over the map?
   */
  virtual bool isLevelShaded() = 0;

  /**
   * Rectangle intersection test.
   */
  virtual bool intersects( int x, int y, int w, int h,
                           int x2, int y2, int w2, int h2 ) = 0;

  virtual void addDescription(char *description, float r=1.0f, float g=1.0f, float b=0.4f) = 0;

	virtual GLuint getNamedTexture( char *name ) = 0;
};

#endif
