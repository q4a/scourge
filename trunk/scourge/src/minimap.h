/***************************************************************************
                      minimap.h  -  The ingame minimap
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

#define MINI_MAP_X_SCALE 3
#define MINI_MAP_Y_SCALE 3

#define MINI_MAP_PARTY_VIEW 4

#define MINI_MAP_WIDTH MAP_WIDTH/MINI_MAP_X_SCALE
#define MINI_MAP_DEPTH MAP_DEPTH/MINI_MAP_Y_SCALE

#include "common/constants.h"
#include "gui/widgetview.h"
#include "gui/window.h"

// forward decl.
class Shape;
class Scourge;
class Location;
class Canvas;

/// Unused.
struct MiniMapPoint {
    GLfloat r, g, b;
    bool visible;
};

/**
 *@author Daroth-U
 */

/// The minimap in the upper left of the ingame view.
class MiniMap {
private:
    
  Scourge *scourge;  
  
  bool showMiniMap;     // true : draw it, false : don't draw the minimap
  // Texture that will hold the minimap
  int textureSizeH, textureSizeW;   
    
 public:
  MiniMap();
  ~MiniMap();
  MiniMap( Scourge *scourge );

  void drawMap();

  inline void setShowMiniMap( bool b ) { showMiniMap = b; }
  inline bool isMiniMapShown() { return showMiniMap; }
  
protected:
	void drawPointers( std::set<Location*> *p, Color color );
  
};

#endif
