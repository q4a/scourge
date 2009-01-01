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

class Outdoor : public MapRender {
private:
	
public:
	Outdoor( Map *map ) : MapRender( map ) {}
	virtual ~Outdoor() {}
	
	virtual void drawGroundPosition( int posX, int posY, float xpos2, float ypos2, Shape *shape );
	
protected:
	virtual void drawMap();
	virtual void doRenderFloor();
	
	virtual inline void setupShapeColor() {
		glColor4f( 1.0f, 1.0f, 1.0f, 0.9f );
	}	

	/// Draws creature effects and damage counters.
	void drawEffects();
	
	/// Draws the roofs on outdoor levels, including the fading.
	void drawRoofs();
	
private:
	void drawOutdoorTex( Texture tex, float tx, float ty, float tw, float th, float angle = 0 );	
	bool drawHeightMapFloor();
	void drawWaterLevel();
};

#endif /*OUTDOOR_H_*/
