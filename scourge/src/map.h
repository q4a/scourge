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
#include "gui/scrollinglist.h"

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
  Creature *creature;
  Item *item;
  GLuint name;  
} DrawLater;

#define SWAP(src, dst) { int _t; _t = src; src = dst; dst = _t; }

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
  Scourge *scourge;
  bool debugGridFlag;
  bool drawGridFlag;
  float xrot, yrot, zrot;  
  float xpos, ypos, zpos;
  float xRotating, yRotating, zRotating;
  Uint16 selX, selY, selZ, oldLocatorSelX, oldLocatorSelY, oldLocatorSelZ;
  float fShadowMatrix[16];
  //bool alwaysCenter;
  static const int X_CENTER_TOLERANCE = 8;
  static const int Y_CENTER_TOLERANCE = 8;

  // on screen item descriptions
  static const int MAX_DESCRIPTION_COUNT = 200;
  int descriptionCount;
  bool descriptionsChanged;
  char *descriptions[MAX_DESCRIPTION_COUNT];
  Color descriptionsColor[MAX_DESCRIPTION_COUNT];

  bool lightMapChanged;
  int lightMap[MAP_WIDTH / MAP_UNIT][MAP_DEPTH / MAP_UNIT];
  
  static const float ZOOM_DELTA = 1.02f;

  // FIXME: either make this value adjustable or find a faster way to blast it onscreen?
  static const int SHADE_SIZE = 20;

  bool useShadow;
  // squish on z and shear x,y
  static const float shadowTransformMatrix[16];

  bool colorAlreadySet;
  Location *selectedDropTarget;

#define OVERLAY_SIZE 16
  GLuint overlay_tex;
  unsigned char overlay_data[OVERLAY_SIZE * OVERLAY_SIZE * 3];

  float targetWidth, targetWidthDelta;
  GLint lastTick;

  static int dir_index[];
  
  void drawGrid(SDL_Surface *surface);
  void debugGrid(SDL_Surface *surface);

 public:
  Map(Scourge *scourge);
  ~Map();

  int debugX, debugY, debugZ;  
  
  bool selectMode;
  bool floorOnly;

  inline void setSelectedDropTarget(Location *loc) { selectedDropTarget = loc; }
  inline Location *getSelectedDropTarget() { return selectedDropTarget; }
  
  inline void setZoomIn(bool b) { zoomIn = b; }
  inline void setZoomOut(bool b) { zoomOut = b; }
  inline float getZoom() { return zoom; }

  inline void setXRot(float b) { xRotating = b; }
  inline void setYRot(float b) { yRotating = b; }
  inline void setZRot(float b) { zRotating = b; }

  inline float getXRot() { return xrot; }
  inline float getYRot() { return yrot; }
  inline float getZRot() { return zrot; }  

  inline void addXPos(float f) { xpos += f; }
  inline void addYPos(float f) { ypos += f; }  
  inline void addZPos(float f) { zpos += f; }

  inline float getXPos() { return xpos; }
  inline float getYPos() { return ypos; }
  inline float getZPos() { return zpos; } 

  inline bool isLocationVisible(int x, int y) { 
	return (x >= getX() && x < getX() + MAP_VIEW_WIDTH &&
			y >= getY() && y < getY() + MAP_VIEW_DEPTH);
  }
    
  void draw();
  
  /**
	 The map top left x coordinate
  */
  inline int getX() { return x; }
  
  /**
	 The map top left y coordinate
  */
  inline int getY() { return y; }
  
  inline void setXY(int x, int y) { this->x = x; this->y = y; }
  
  void center(Sint16 x, Sint16 y, bool force=false);
    
  void setPosition(Sint16 x, Sint16 y, Sint16 z, Shape *shape);
  Shape *removePosition(Sint16 x, Sint16 y, Sint16 z);
  
  void Map::setItem(Sint16 x, Sint16 y, Sint16 z, Item *item);
  Item *removeItem(Sint16 x, Sint16 y, Sint16 z);
  
  void Map::setCreature(Sint16 x, Sint16 y, Sint16 z, Creature *creature);
  Creature *removeCreature(Sint16 x, Sint16 y, Sint16 z);
  
  /**
   * if you can't move to this spot (blocked) returns the blocking shape,
   * otherwise returns NULL and moves the shape.
   */
  Location *moveCreature(Sint16 x, Sint16 y, Sint16 z, Uint16 dir, 
						 Creature *newCreature);
  Location *moveCreature(Sint16 x, Sint16 y, Sint16 z, 
						 Sint16 nx, Sint16 ny, Sint16 nz, 
						 Creature *newCreature);
  
  void switchPlaces(Sint16 x1, Sint16 y1, Sint16 z1, 
					Sint16 x2, Sint16 y2, Sint16 z2);
  
  void setFloorPosition(Sint16 x, Sint16 y, Shape *shape);
  Shape *removeFloorPosition(Sint16 x, Sint16 y);
  
  /**
   * Can shape at shapeX, shapeY, shapeZ move to location x, y, z?
   * returns NULL if ok, or the blocking Shape* otherwise.
   * if newz is not null, it will ignore blocking "item"-s and instead stack the new
   * shape on top, returning the new z position in newz.
   */
  Location *isBlocked(Sint16 x, Sint16 y, Sint16 z, 
					  Sint16 shapex, Sint16 shapey, Sint16 shapez,
					  Shape *shape,
					  int *newz = NULL);
  
  /** This one only returns if the shape originates at xyz. */
  Location *getPosition(Sint16 x, Sint16 y, Sint16 z);
  /** This one returns even if shape doesn't originate at xyz */
  inline Location *getLocation(Sint16 x, Sint16 y, Sint16 z) { return pos[x][y][z]; }
  inline Shape *getFloorPosition(Sint16 x, Sint16 y) { if(x < 0 || x >= MAP_WIDTH || y < 0 || y >= MAP_DEPTH) return NULL; else return floorPositions[x][y]; }
  
  void showInfoAtMapPos(Uint16 mapx, Uint16 mapy, Uint16 mapz, char *message);
  void showCreatureInfo(Creature *creature, bool player, bool selected, bool groupMode);
  
  void initMapView(bool ignoreRot = false);
  
  void addDescription(char *description, float r=1.0f, float g=1.0f, float b=0.4f);
  
  void drawDescriptions(ScrollingList *list);
  
  void handleMouseClick(Uint16 mapx, Uint16 mapy, Uint16 mapz, Uint8 button);
  void handleMouseMove(Uint16 mapx, Uint16 mapy, Uint16 mapz);
  
  inline Uint16 getSelX() { return selX; }
  inline Uint16 getSelY() { return selY; }
  inline Uint16 getSelZ() { return selZ; }

  void move(int dir);
  
  bool isWallBetween(int x1, int y1, int z1,
					 int x2, int y2, int z2);
  
  bool shapeFits(Shape *shape, int x, int y, int z);

  // like shapefits, but returns the blocking location or, NULL if there's nothing there
  Location *getBlockingLocation(Shape *shape, int x, int y, int z);

  Location *Map::getDropLocation(Shape *shape, int x, int y, int z);

  inline void updateLightMap() { lightMapChanged = true; }

  inline void refresh() { mapChanged = lightMapChanged = true; }

  // drop items above this one
  void dropItemsAbove(int x, int y, int z, Item *item);

  void drawDraggedItem();

 protected:
  DrawLater later[100], stencil[1000], other[1000], damage[1000];
  int laterCount, stencilCount, otherCount, damageCount;
  
  void doDrawShape(DrawLater *later, int effect=0);
  void doDrawShape(float xpos2, float ypos2, float zpos2, 
				   Shape *shape, GLuint name, int effect=0,
				   DrawLater *later=NULL);

  /**
	 If 'ground' is true, it draws the ground layer.
	 Otherwise the shape arrays (other, stencil, later) are populated.
   */
  void setupShapes(bool ground);
  void setupPosition(int posX, int posY, int posZ,
					 float xpos2, float ypos2, float zpos2,
					 Shape *shape, Item *item, Creature *creature);
  void drawGroundPosition(int posX, int posY,
						  float xpos2, float ypos2,
						  Shape *shape);
  bool isWall(int x, int y, int z);

  void configureLightMap();
  void traceLight(int chunkX, int chunkY);
  bool isLocationBlocked(int x, int y, int z);

  void drawShade();
  
  void drawCube(float x, float y, float z, float r);

  void createOverlayTexture();    
};

#endif
