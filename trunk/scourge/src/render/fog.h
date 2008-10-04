/***************************************************************************
                            fog.h  -  Fog of war
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
#pragma once

#include "render.h"
#include <set>

class CFrustum;
class Shape;
class Map;
class RenderedCreature;

/// The "unvisited" fog and fog of war
class Fog {
private:
	float lampRadiusSquared;
	int fog[MAP_WIDTH][MAP_DEPTH];
	std::set<RenderedCreature *> *players;
	Map *map;

#define OVERLAY_SIZE 16
	GLuint overlay_tex;
	unsigned char overlay_data[OVERLAY_SIZE * OVERLAY_SIZE * 4];

	GLuint shade_tex;
	unsigned char shade_data[OVERLAY_SIZE * OVERLAY_SIZE * 3];

public:

	enum {
		FOG_UNVISITED = 0,
		FOG_VISITED,
		FOG_CLEAR
	};

	Fog( Map *map, float lampRadiusSquared = 36.0f );
	~Fog();

	int getValue( int mapx, int mapy );
	void visit( RenderedCreature *player );
	void hideDeadParty();
	void reset();
	void draw( int sx, int sy, int w, int h, CFrustum *frustum );
	int getVisibility( int xp, int yp, Shape *shape );
	void load( FogInfo *fogInfo );
	void save( FogInfo *fogInfo );
protected:
	void getScreenXY( GLdouble mapx, GLdouble mapy, GLdouble mapz,
	                  GLdouble *screenx, GLdouble *screeny );
	int getHighestZ( int sx, int sy, int w, int h );
	void createOverlayTexture();
	void createShadeTexture();
};

#endif

