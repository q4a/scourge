/***************************************************************************
                  indoor.h  -  Manages and renders the level map
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

#ifndef INDOOR_H
#define INDOOR_H
#pragma once

class Map;
class Surface;
class Location;
#include "render.h"

class Indoor {
private:
	Map *map;

public:
	Indoor( Map *map ) { this->map = map; }
	virtual ~Indoor() {}
	void draw();

private:		
	void drawRoofsIndoor();
	void drawFrontWallsAndWater();
	void drawWaterIndoor();
	void drawIndoorShadows();
	void drawObjectsAndCreatures();
	void drawLightsFloor();
	void drawLightsWalls();
	bool isFacingLight( Surface *surface, Location *p, Location *lightPos );
	
	
	
	DECLARE_NOISY_OPENGL_SUPPORT();
};

#endif
