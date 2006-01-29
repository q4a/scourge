/***************************************************************************
                          fog.h  -  description
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

#ifndef FOG_H
#define FOG_H

#include "../constants.h"

class CFrustum;                
class Shape;
class Map;

class Fog {
private:
  int fog[MAP_WIDTH][MAP_DEPTH];
  Map *map;
  GLuint texture;

public:

  enum {
    FOG_UNVISITED=0,
    FOG_VISITED,
    FOG_CLEAR
  };

  Fog( Map *map, GLuint texture );
  ~Fog();

  int getValue( int mapx, int mapy );
  void visit( int mapx, int mapy );
  void reset();
  void draw( int sx, int sy, int w, int h, CFrustum *frustum );
  int getVisibility( int xp, int yp, Shape *shape );
protected:
  void getScreenXY( GLdouble mapx, GLdouble mapy, GLdouble mapz,
                    GLdouble *screenx, GLdouble *screeny );
  int getHighestZ( int sx, int sy, int w, int h );
};

#endif

