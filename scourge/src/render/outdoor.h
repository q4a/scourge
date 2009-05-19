/***************************************************************************
                  outdoor.h  -  Manages and renders the level map
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

#ifndef OUTDOOR_H_
#define OUTDOOR_H_
#pragma once

#include "render.h"
#include "maprender.h"

class Map;
class Surface;
class Location;
struct DrawLater;
class Shape;
struct Rug;
class CVectorTex;

class Outdoor : public MapRender {
private:
	
public:
	Outdoor( Map *map ) : MapRender( map ) { useDisplayList = false; hasDisplayList = false; }
	virtual ~Outdoor() {}
	
	virtual void drawGroundPosition( int posX, int posY, float xpos2, float ypos2, Shape *shape );
	virtual void createGroundMap();
	
protected:
	virtual void drawMap();
	virtual void doRenderFloor();
	
	virtual inline void setupShapeColor() {
		glColor4f( 1.0f, 1.0f, 1.0f, 0.9f );
	}	
	
	void drawWalls();

	/// Draws creature effects and damage counters.
	void drawEffects();
	
	/// Draws the roofs on outdoor levels, including the fading.
	void drawRoofs();
	
private:
	GLuint floorDisplayList;
	bool useDisplayList;
	bool hasDisplayList;

	void drawObjects( std::vector<RenderedLocation*> *shades );
	void drawOutdoorTex( Texture tex, float tx, float ty, float tw, float th, float angle = 0 );	
	bool drawHeightMapFloor();
	void drawWaterLevel();
//	void applyGrassEdges( int x, int y, bool w, bool e, bool s, bool n );
//	Texture getThemeTex( int ref );
//	void addHighVariation( int ref, int z );
//	bool isRockTexture( int x, int y );
//	bool isLakebedTexture( int x, int y );
//	bool isAllHigh( int x, int y, int w, int h );
	void addLight( CVectorTex *pt, CVectorTex *a, CVectorTex *b );
};

#endif /*OUTDOOR_H_*/
