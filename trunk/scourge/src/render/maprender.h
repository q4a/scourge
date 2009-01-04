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

class Map;
class Surface;
class Location;
struct DrawLater;
class Shape;
struct Rug;
class RenderedLocation;

class MapRender {
	
protected:
	Map *map;	
	
public:
	MapRender( Map *map );
	virtual ~MapRender();
	
	void draw();
	
	void willDrawGrid();
	void drawTraps();
	void drawRug( Rug *rug, float xpos2, float ypos2, int xchunk, int ychunk );
	
	virtual inline void drawGroundPosition( int posX, int posY,
	                                        float xpos2, float ypos2,
	                                        Shape *shape ) {}
	virtual inline void drawWaterPosition( int posX, int posY,
	                                       float xpos2, float ypos2,
	                                       Shape *shape ) {}
	virtual inline void initOutdoorsGroundTexture() {}
	virtual void createGroundMap();

	virtual inline void setupLightBlending() {
		//Scourge::setBlendFuncStatic();
		glBlendFunc( GL_SRC_ALPHA, GL_ONE );
	}

	virtual inline void setupPlayerLightColor() {
		//glColor4f( 0.3f, 0.3f, 0.3f, 1.0f );	
		glColor4f( 0.5f, 0.45f, 0.2f, 0.5f );
	}

	virtual inline void setupShapeColor() {
		glColor4f( 0.5f, 0.5f, 0.5f, 0.5f );
	}

	virtual inline void setupBlendedWallColor( float alpha=0.45f ) {
		glColor4f( 1.0f, 1.0f, 1.0f, alpha );
	}

	virtual inline void setupDropLocationColor() {
		glColor4f( 0.0f, 1.0f, 1.0f, 1.0f );
	}

	virtual inline void setupShadowColor() {
		glColor4f( 0.04f, 0.0f, 0.07f, 0.3f );
	}

	virtual inline void setupSecretDoorColor() {
		glColor4f( 0.3f, 0.7f, 0.3f, 1.0f );	
	}

	virtual inline void setupLockedDoorColor() {
		glColor4f( 1.0f, 0.3f, 0.3f, 1.0f );
	}
	
	void drawGroundTex( Texture tex, float tx, float ty, float tw, float th, float angle = 0 );
	
protected:
	void renderFloor();
	virtual void doRenderFloor() = 0;
	virtual void drawMap() = 0;
	void drawProjectiles();
	void debugGround( int sx, int sy, int ex, int ey );
};

#endif /*MAPRENDER_H_*/
