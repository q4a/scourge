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


#include "constants.h"
#include "location.h"
#include "sdlhandler.h"
#include "shape.h"
#include "map.h"
#include "dungeongenerator.h"
#include "scourge.h"
#include "math.h"
#include "gui/window.h"
#include "gui/canvas.h"
#include "gui/widgetview.h"

using namespace std;

// forward decl.
class Shape;
//class Map;
class Scourge;
class Util;


typedef struct _MiniMapPoint {
    GLfloat r, g, b;
    bool visible;
}MiniMapPoint;

/**
 *@author Daroth-U
 */
class MiniMap : public WidgetView {
 private:
    
  // Basic Idea : minimap is already filled but a point becomes visible only
  // if the party has reached it.
  MiniMapPoint pos[MINI_MAP_WIDTH][MINI_MAP_DEPTH];
  Scourge *scourge;  
  
	Window *win;
  Canvas *canvas;
  
  GLfloat zoomFactor;   // Determines the zoom factor of the minimap   
  int mode;             // 0 : show structures : wall, doors and floor , 1: add monsters ... 
  bool showMiniMap;     // true : draw it, false : don't draw the minimap
  int screenHeight;     // Needed for glScissor used in MiniMap::Draw()
  
  // Texture that will hold the minimap
  int textureSizeH, textureSizeW;   
  GLuint texture[1];
  unsigned char * textureInMemory;
  bool mustBuildTexture;   
  
  // Real width and height of minimap in pixels (i.e. : without insignificant
  // pixels at the bottom or at the right side) including a little marge.
  int effectiveWidth, effectiveHeight; 
  int maxX, maxY;
  int minX, minY;
  float midX, midY;  
  
  // Transform Map coordinates to MiniMap coordinates
  void toMiniMapCoord(int &x, int &y);       
  
 public:
  MiniMap::MiniMap();
  MiniMap::~MiniMap();
  MiniMap(Scourge *scourge);

  void reset();

  inline void show() { win->setVisible(true); }
  inline void hide() { win->setVisible(false); }
  inline Window *getWindow() { return win; }
  inline void resize(int w, int h) { win->resize(w, h); canvas->resize(w, h - 25); }
  
  // x, y are in *global Map* coordinates (see Map.h to know its size) 
  void colorMiniMapPoint(int x, int y, Shape *shape, Location *pos);
  void eraseMiniMapPoint(int x, int y);
  void zoomIn();
  void zoomOut();
  
  bool checkInside(int a, int b); 
  inline void toggle(){showMiniMap = !showMiniMap;} 
  void computeDrawValues(); // needed before drawing the minimap
  void updateFog(int a, int b); // gradually set the minimap visible for the player
  void buildTexture(int xCoord, int yCoord);

  void drawWidget(Widget *w);
  
  //void handleMouseClick(Uint16 mapx, Uint16 mapy, Uint16 mapz, Uint8 button);    
  
 protected:
  
};

#endif
