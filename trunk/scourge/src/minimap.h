/***************************************************************************
                          map.h  -  description
                             -------------------
    begin                : Thu Jan 29 2004
    copyright            : (C) 2004 by Daroth-U
    email                : daroth-u@ifrance.com
***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef MINIMAP_H
#define MINIMAP_H


#define MINI_MAP_WIDTH MAP_WIDTH/3
#define MINI_MAP_DEPTH MAP_DEPTH/3

// set to non-zero for debugging
#define DEBUG_MINIMAP 0

#include "constants.h"
#include "location.h"
#include "sdlhandler.h"
#include "shape.h"
#include "map.h"
#include "dungeongenerator.h"
#include "scourge.h"

using namespace std;

// forward decl.
class Location;
class Shape;
class DungeonGenerator;
class Map;
class Scourge;


typedef struct _MiniMapPoint {
    GLfloat r, g, b;
    bool visible;
}MiniMapPoint;

/**
 *@author Daroth-U
 */
class MiniMap {
 private:
  
  // Scale factors for transforming Map (x, y) to MiniMap (x, y)
  float Sx;
  float Sy;
  
  // Basic Idea : minimap is already filled but a point becomes visible only
  // if the party has reached it.
  MiniMapPoint pos[MINI_MAP_WIDTH][MINI_MAP_DEPTH];
  Scourge *scourge;  
  
  // Transform Map coordinates to MiniMap coordinates
  void toMiniMapCoord(int &x, int &y);   
  
  int mode;  // 0 : show structures : wall, doors and floor , 1: add monsters ... 
  
 public:
  MiniMap::MiniMap();
  MiniMap::~MiniMap();
  MiniMap(Scourge *scourge);
  
  // x, y are in *Map* coordinates
  void colorMiniMapPoint(int x, int y, Shape *shape);
  void eraseMiniMapPoint(int x, int y);
  
  void draw(int xCoord, int yCoord);
  
  //void handleMouseClick(Uint16 mapx, Uint16 mapy, Uint16 mapz, Uint8 button);    
  
 protected:
  
};

#endif
