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

#ifndef SHAPE_H
#define SHAPE_H
#pragma once

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "render.h"

class WindInfo;

/// A flat surface making up one side of a wall etc.

class Surface {
public:	
	float vertices[4][3];
	float matrix[9];
	float s_dist, t_dist;
};

class LightEmitter {
public:
	LightEmitter() {}
	virtual ~LightEmitter() {}
	virtual float getRadius() = 0;	
	virtual Color const& getColor() = 0;
};

/// Contains a static 3D shape.
class Shape {

private:
	Uint8 index;
	std::string name;
	int descriptionGroup;
	bool stencil;
	Color *outlineColor;
	bool interactive;
	int groundSX, groundEX, groundSY, groundEY;
	float outdoorWeight;
	bool outdoorShadow;
	bool wind;
	WindInfo *windInfo;
	bool occludedSides[6];
	int textureCount, textureIndex;
	bool roof;
	bool iAmGround;

protected:
	int width, height, depth;
	float offsetX, offsetY, offsetZ;
	std::set<Surface*> *lightFacingSurfaces;	

public:
	enum {
		TOP_SIDE = 0,
		BOTTOM_SIDE,
		E_SIDE,
		S_SIDE,
		W_SIDE,
		N_SIDE
	};

	Shape( int width, int depth, int height, char const* name, int descriptionGroup );
	Shape( Shape *shape );
	virtual ~Shape();
	
	virtual inline void getSurfaces( std::set<Surface*> *shape_surfaces, bool skip_top ) {}
	virtual inline void setLightFacingSurfaces( std::set<Surface*> *lightFacingSurfaces ) { this->lightFacingSurfaces = lightFacingSurfaces; }
	
	inline void setGround( bool b ) { iAmGround = b; }
	inline bool isGround() { return iAmGround; }

	void setOccludedSides( bool *sides );
	inline bool *getOccludedSides() {
		return occludedSides;
	}

	inline void setDebugGroundPos( int sx, int sy, int ex, int ey ) {
		this->groundSX = sx;
		this->groundSY = sy;
		this->groundEX = ex;
		this->groundEY = ey;
	}

	inline int getGroundSX() {
		return groundSX;
	}
	inline int getGroundSY() {
		return groundSY;
	}
	inline int getGroundEX() {
		return groundEX;
	}
	inline int getGroundEY() {
		return groundEY;
	}

	inline float getOffsX() {
		return offsetX;
	}
	inline float getOffsY() {
		return offsetY;
	}
	inline float getOffsZ() {
		return offsetZ;
	}
	inline void setOffset( float x, float y, float z ) {
		offsetX = x; offsetY = y; offsetZ = z;
	}

	/**
	  Call this once before the shape is to be displayed.
	*/
	virtual inline void intialize() { }

	/**
	  The widht (x) of the shape-block's base
	*/
	inline int getWidth() {
		return width;
	}
	/**
	  The depth (y) of the shape-block's base
	*/
	inline int getDepth() {
		return depth;
	}
	/**
	  The height (z) of the shape-block
	*/
	inline int getHeight() {
		return height;
	}

	/// check if position is cowered by shape 
	/// Shape has rectangular base by default
	virtual bool isInside( int x, int y ) {
		assert( x >= 0 && x < width && y >= 0 && y < depth );
		return true;
	}

	inline char const* getName() {
		return name.c_str();
	}
	inline int getDescriptionGroup() {
		return descriptionGroup;
	}

	virtual LightEmitter *getLightEmitter() { return NULL; }
	virtual void draw() = 0;
	void drawHeightMap( float ground[][MAP_DEPTH], int groundX, int groundY ) {
		draw();
	}
	virtual void outline( const Color *color ) {
		outline( color->r, color->g, color->b );
	};
	virtual void outline( float r, float g, float b ) {};

	inline void setIndex( Uint8 index ) {
		this->index = index;
	}

	inline Uint8 getIndex() {
		return index;
	}

	// if true, the next two functions are called
	virtual bool isBlended() = 0;
	virtual void setupBlending() = 0;
	virtual void endBlending() = 0;


	inline void setStencil( bool b ) {
		stencil = b;
	}
	inline bool isStencil() {
		return stencil;
	}

	inline void setOutlineColor( Color *color ) {
		this->outlineColor = color;
	}
	inline Color *getOutlineColor() {
		return this->outlineColor;
	}

	void setInteractive( bool b ) {
		interactive = b;
	}
	bool isInteractive() {
		return interactive;
	}

	void setOutdoorWeight( float f ) {
		outdoorWeight = f;
	}
	float getOutdoorWeight() {
		return outdoorWeight;
	}

	void setOutdoorShadow( bool b ) {
		outdoorShadow = b;
	}
	bool isOutdoorShadow() {
		return outdoorShadow;
	}

	void setWind( bool b ) {
		wind = b;
	}
	bool isWind() {
		return wind;
	}
	virtual inline float getWindValue() {
		return 0;
	}

	virtual inline bool isFlatCaveshape() {
		return false;
	}

	inline void setTextureCount( int count ) {
		this->textureCount = count;
	}
	inline int getTextureCount() {
		return textureCount;
	}
	inline void setTextureIndex( int index ) {
		this->textureIndex = index;
	}
	inline int getTextureIndex() {
		return textureIndex;
	}

	virtual inline bool isVirtual() {
		return false;
	}

	inline void setRoof( bool b ) {
		this->roof = b;
	}
	inline bool isRoof() {
		return roof;
	}
};

#endif
