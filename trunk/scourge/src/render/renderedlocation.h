/***************************************************************************
                    shape.h  -  Class for static 3D shapes
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

#ifndef RENDEREDLOCATION_H_
#define RENDEREDLOCATION_H_
#pragma once

#include "render.h"
#include "texture.h"
class Map;
class Location;
class EffectLocation;

/// Stores the state and contents of a level map location for later drawing.

class RenderedLocation {
public:
	
	// todo: make these private w. accessors
	static Texture lightTex;
	Map *map;
	float xpos, ypos, zpos;
	EffectLocation *effect;
	GLuint name;
	Location *pos;
	bool inFront;
	int x, y;
	bool light;
	bool effectMode;
	
	static bool useShadow;
	static bool colorAlreadySet;
	
	RenderedLocation();
	~RenderedLocation();
	
	void reset();
	void set( Map *map,
	          float xpos2, float ypos2, float zpos2,
            EffectLocation *effect,
            Location *pos,
            GLuint name,
            int posX, int posY,
            bool inFront,
            bool light, 
            bool effectMode );
	
	void draw();
	
protected:
	void doDrawEffect();
	void findOccludedSides( bool *sides );
	void drawLight();
	void drawCreature();
	void drawItem();
	void drawShape();
	void drawEffect();
	void drawMousePosition();
	void outlineVirtuals();
};

#endif /*RENDEREDLOCATION_H_*/
