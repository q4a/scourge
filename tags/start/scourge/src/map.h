/***************************************************************************
                          map.h  -  description
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

#ifndef MAP_H
#define MAP_H

#include "constants.h"
#include "sdlhandler.h"
#include "shape.h"
#include "location.h"
#include "dungeongenerator.h"
#include "creature.h"
#include "scourge.h"
#include "item.h"

using namespace std;

// forward decl.
class Location;
class DungeonGenerator;
class Scourge;
class Creature;
class Item;

typedef struct _DrawLater {
  float xpos, ypos, zpos;
  Shape *shape;
  GLuint name;  
} DrawLater;

/**
  *@author Gabor Torok
  */
class Map {
private:
    bool mapChanged;
    float zoom;
    bool zoomIn, zoomOut;
  Sint16 x;
  Sint16 y;
  Location *pos[MAP_WIDTH][MAP_DEPTH][MAP_VIEW_HEIGHT];
	Shape *floorPositions[MAP_WIDTH][MAP_DEPTH];
  Uint16 move;
	Scourge *scourge;
  bool debugGridFlag;
  bool drawGridFlag;
  float xrot, yrot, zrot;  
  float xpos, ypos, zpos;
  float xRotating, yRotating, zRotating;
  Uint16 selX, selY, selZ;

  static const int MAX_ITEM_COUNT = 200;
  typedef struct _ItemPos {
    Item *item;
    int x, y, z;
  } ItemPos;
  ItemPos items[MAX_ITEM_COUNT];
  int itemCount;

  // on screen item descriptions
  int descriptionCount;
  char *descriptions[200];

  static const float ZOOM_DELTA = 1.02f;
  
  void drawGrid(SDL_Surface *surface);
  void debugGrid(SDL_Surface *surface);

public:
	Map(Scourge *scourge);
	~Map();

  bool selectMode;
  bool floorOnly;

  inline void setZoomIn(bool b) { zoomIn = b; }
  inline void setZoomOut(bool b) { zoomOut = b; }
  inline float getZoom() { return zoom; }

  inline void setXRot(float b) { xRotating = b; }
  inline void setYRot(float b) { yRotating = b; }
  inline void setZRot(float b) { zRotating = b; }

  inline float getXRot() { return xrot; }
  inline float getYRot() { return yrot; }
  inline float getZRot() { return zrot; }  

  inline void addXPos(float f) { xpos += f; fprintf(stderr, "xpos=%f\n", xpos); }
  inline void addYPos(float f) { ypos += f; fprintf(stderr, "ypos=%f\n", ypos); }  
  inline void addZPos(float f) { zpos += f; fprintf(stderr, "zpos=%f\n", zpos); }

  inline float getXPos() { return xpos; }
  inline float getYPos() { return ypos; }
  inline float getZPos() { return zpos; }  

  void addItem(Item *item, int x, int y, int z);
  Item *getItem(Sint16 mapx, Sint16 mapy, Sint16 mapz); 
  
  void draw(SDL_Surface *surface);

  /**
    The map top left x coordinate
  */
  inline int getX() { return x; }

  /**
    The map top left y coordinate
  */
  inline int getY() { return y; }

  inline void setXY(int x, int y) { this->x = x; this->y = y; }

	void center(Sint16 x, Sint16 y);

  inline void setMove(Uint16 n) { move |= n; };

  inline void removeMove(Uint16 n) { move &= (0xffff - n); }

  inline Uint16 getMove() { return move; }

  void setPosition(Sint16 x, Sint16 y, Sint16 z, Shape *shape);
	Shape *removePosition(Sint16 x, Sint16 y, Sint16 z);

  void Map::setItem(Sint16 x, Sint16 y, Sint16 z, Item *item);
	Item *removeItem(Sint16 x, Sint16 y, Sint16 z);

	/**
	 * if you can't move to this spot (blocked) returns the blocking shape,
	 * otherwise returns NULL and moves the shape.
	 */
	Location *movePosition(Sint16 x, Sint16 y, Sint16 z, Uint16 dir, Shape *newShape);
	Location *movePosition(Sint16 x, Sint16 y, Sint16 z, Sint16 nx, Sint16 ny, Sint16 nz, Shape *newShape);

	void switchPlaces(Sint16 x1, Sint16 y1, Sint16 z1, 
										Sint16 x2, Sint16 y2, Sint16 z2);

	void setFloorPosition(Sint16 x, Sint16 y, Shape *shape);
	Shape *removeFloorPosition(Sint16 x, Sint16 y);

	/**
	 * Can shape at shapeX, shapeY, shapeZ move to location x, y, z?
	 * returns NULL if ok, or the blocking Shape* otherwise.
	 */
	Location *isBlocked(Sint16 x, Sint16 y, Sint16 z, 
											Sint16 shapex, Sint16 shapey, Sint16 shapez,
											Shape *shape);

  Location *getPosition(Sint16 x, Sint16 y, Sint16 z);
  inline Shape *getFloorPosition(Sint16 x, Sint16 y) { if(x < 0 || x >= MAP_WIDTH || y < 0 || y >= MAP_DEPTH) return NULL; else return floorPositions[x][y]; }

  void showInfoAtMapPos(Uint16 mapx, Uint16 mapy, Uint16 mapz, char *message);
  void showCreatureInfo(Creature *creature);

  void initMapView(bool ignoreRot = false);

  void addDescription(char *description);

  void drawDescriptions();

  void handleMouseClick(Uint16 mapx, Uint16 mapy, Uint16 mapz, Uint8 button);
  void handleMouseMove(Uint16 mapx, Uint16 mapy, Uint16 mapz);

  inline Uint16 getSelX() { return selX; }
  inline Uint16 getSelY() { return selY; }
  inline Uint16 getSelZ() { return selZ; }

//  GLfloat getDistance(float xpos, float ypos, float zpos);

protected:
  DrawLater later[100], stencil[1000], other[1000];
  int laterCount, stencilCount, otherCount;

  void doDrawShape(DrawLater *later);
  void doDrawShape(float xpos2, float ypos2, float zpos2, Shape *shape, GLuint name);
  void setupShapes();
  void drawGround();
  void drawLocator();
};

#endif
