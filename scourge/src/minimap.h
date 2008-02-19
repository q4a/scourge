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

struct MiniMapPoint {
    GLfloat r, g, b;
    bool visible;
};

/**
 *@author Daroth-U
 */
class MiniMap {
private:
    
  Scourge *scourge;  
  
  bool showMiniMap;     // true : draw it, false : don't draw the minimap
  // Texture that will hold the minimap
  int textureSizeH, textureSizeW;   
  GLuint texture[1];
  unsigned char * textureInMemory;
  bool mustBuildTexture;
  bool directMode;
    
 public:
  MiniMap();
  ~MiniMap();
  MiniMap( Scourge *scourge, bool directMode = false );

  void reset();

  void prepare();
  void drawMap();

  inline void setShowMiniMap( bool b ) { showMiniMap = b; }
  inline bool isMiniMapShown() { return showMiniMap; }
  
};

#endif
