/***************************************************************************
                  maprender.h  -  Manages and renders the level map
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

#ifndef MAPRENDER_H_
#define MAPRENDER_H_
#pragma once

#include "render.h"
#include "texture.h"

class Map;
class Surface;
class Location;
class DrawLater;
class Shape;

class MapRender {
private:
	Texture lightTex;
	
protected:
	Map *map;	
	
public:
	MapRender( Map *map );
	virtual ~MapRender();
	
	void draw();
	
	void willDrawGrid();
	void drawTraps();
	
	virtual inline void drawGroundPosition( int posX, int posY,
	                                        float xpos2, float ypos2,
	                                        Shape *shape ) {}
	virtual inline void drawWaterPosition( int posX, int posY,
	                                       float xpos2, float ypos2,
	                                       Shape *shape ) {}

	
	
protected:
	void doDrawShape( DrawLater *later, int effect = 0 );
	void doDrawShape( float xpos2, float ypos2, float zpos2,
	                  Shape *shape, int effect = 0, DrawLater *later = NULL );
	void findOccludedSides( DrawLater *later, bool *sides );
	virtual void drawMap() = 0;
	void drawProjectiles();	
};

#endif /*MAPRENDER_H_*/
