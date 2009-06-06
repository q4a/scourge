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

#include "render.h"
#include "maprender.h"

class Map;
class Surface;
class Location;
struct DrawLater;
class Shape;
class Rug;

class Indoor : public MapRender {
private:

public:
	Indoor( Map *map ) : MapRender( map ) {}
	virtual ~Indoor() {}

protected:
	virtual void drawMap();
	virtual void doRenderFloor();
	
	virtual void drawGroundPosition( int posX, int posY, float xpos2, float ypos2, Shape *shape );
	virtual void drawWaterPosition( int posX, int posY, float xpos2, float ypos2, Shape *shape );
	
private:		
	void drawFrontWallsAndWater();
	void drawWaterIndoor();
	void drawIndoorShadows();
	void drawObjectsAndCreatures();
	void drawLightsFloor();
	void drawLightsWalls();
	bool isFacingLight( Surface *surface, Location *p, Location *lightPos );
	void drawFlatFloor();	
	void sortShapes( RenderedLocation *playerDrawLater, RenderedLocation *shapes, int shapeCount );
	bool isShapeInFront( GLdouble playerWinY, GLdouble objX, GLdouble objY, std::map< std::string, bool > *cache, GLdouble *mm, GLdouble *pm, GLint *vp );
	
	
	DECLARE_NOISY_OPENGL_SUPPORT();
};

#endif
