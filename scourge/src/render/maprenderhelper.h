/***************************************************************************
                          maprenderhelper.h  -  description
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

#ifndef MAP_RENDER_HELPER_H
#define MAP_RENDER_HELPER_H

#include "render.h"

class Fog;
class Shape;
class Map;
class  RenderedCreature;

/**
 * Used to handle map-type-specific rendering, for example light.
 */
class MapRenderHelper {
private:
  Map *map;

public:
  
  enum {
    CAVE_HELPER=0,
    ROOM_HELPER,
		OUTDOOR_HELPER,
		DEBUG_OUTDOOR_HELPER,

    HELPER_COUNT
  };

  static MapRenderHelper *helpers[];

  MapRenderHelper();
  virtual ~MapRenderHelper();

  virtual inline void setMap( Map *map ) { this->map = map; }
  inline Map *getMap() { return map; }

  inline virtual void reset() {}
  inline virtual void draw( int x, int y, int w, int h ) {}
  inline virtual bool isVisible( int x, int y, Shape *shape ) { return true; }
  inline virtual void visit( RenderedCreature *creature ) {}
  inline virtual void hideDeadParty() {}
  inline virtual bool isLightMapEnabled() { return true; }
	inline virtual void loadHelper( FogInfo *fogInfo ) {}
	inline virtual void saveHelper( FogInfo *fogInfo ) {}
	inline virtual bool drawShadow() { return true; }
};

class CaveRenderHelper : public MapRenderHelper {
private:
  Fog *fog;

public:
  CaveRenderHelper();
  virtual ~CaveRenderHelper();
  virtual void setMap( Map *map );
  virtual void reset();
  virtual void draw( int x, int y, int w, int h );
  virtual bool isVisible( int x, int y, Shape *shape );
  virtual void visit( RenderedCreature *creature );
  virtual void hideDeadParty();
  inline virtual bool isLightMapEnabled() { return false; }
	virtual void loadHelper( FogInfo *fogInfo );
	virtual void saveHelper( FogInfo *fogInfo );
};

class OutdoorRenderHelper : public CaveRenderHelper {
public:
  OutdoorRenderHelper();
  virtual ~OutdoorRenderHelper();
	inline virtual bool drawShadow() { return false; }
};

class DebugOutdoorRenderHelper : public MapRenderHelper {
public:
  DebugOutdoorRenderHelper();
  virtual ~DebugOutdoorRenderHelper();
	inline virtual bool drawShadow() { return false; }
};

#define OVERLAY_SIZE 16

class RoomRenderHelper : public MapRenderHelper {
private:  
  GLuint overlay_tex;
  unsigned char overlay_data[OVERLAY_SIZE * OVERLAY_SIZE * 3];

public:
  RoomRenderHelper();
  virtual ~RoomRenderHelper();
  virtual void setMap( Map *map );
  virtual void draw( int x, int y, int w, int h );

protected:
  void createOverlayTexture();

};

#endif
