/***************************************************************************
                glshape.h  -  Class representing any 3D shape.
                             -------------------
    begin                : Thu Jul 10 2003
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

#ifndef GLSHAPE_H
#define GLSHAPE_H
#pragma once

#include "render.h"
#include "shape.h"
#include <vector>
#include "shapes.h"

#define MUL 16.0f

class WallTheme;
class Effect;

/**
  *@author Gabor Torok
  */

/// Nearly every 3D object in the game is a GLShape.

class GLShape : public Shape  {
protected:
	Texture* tex; // Represents an array of 3 Textures. 
	Uint8 shapePalIndex;
	int skipside;
	bool useTexture;
	bool lightBlocking;
	float xIconRot, yIconRot, zIconRot;
	GLuint displayListStart;
	bool initialized;
	bool wallShape;
	int iconRotX;
	int iconRotY;
	int iconRotZ;
	Texture icon;
	int iconWidth, iconHeight;
	std::string ambient;
	float alpha;
	std::vector<GLShape*> virtualShapes;
	bool ignoreHeightMap;
	bool notFlatten;
	bool noFloor;

	Surface *surfaces[5];
	enum {
		LEFT_SURFACE = 0,
		RIGHT_SURFACE,
		FRONT_SURFACE,
		TOP_SURFACE,
		BOTTOM_SURFACE
	};

#define LIGHTMAP_SIZE 16

	int effectType;
	int effectWidth, effectDepth, effectHeight;
	int effectX, effectY, effectZ;

	Occurs occurs;

protected:
	Uint32 color;

public:
	static const int FRONT_SIDE = 0;
	static const int TOP_SIDE = 1;
	static const int LEFT_RIGHT_SIDE = 2;

	enum {
		NORMAL_LIGHTING = 0,
		OUTDOOR_LIGHTING
	};
	
	inline void setNoFloor( bool b ) { noFloor = b; }
	inline bool getNoFloor() { return noFloor; }
	
	virtual void getSurfaces( std::set<Surface*> *shape_surfaces, bool skip_top );

	inline void setAlpha( float f ) {
		this->alpha = f;
	}
	inline float getAlpha() {
		return this->alpha;
	}

	inline void setUseTexture( bool b ) {
		useTexture = b;
	}
	inline bool getUseTexture() {
		return useTexture;
	}

	bool useShadow;

	inline bool isWallShape() {
		return wallShape;
	}
	inline void setIsWallShape( bool b ) {
		wallShape = b;
	}

	void setOccurs( Occurs *o );
	inline Occurs *getOccurs() {
		return &occurs;
	}

	inline void setIgnoreHeightMap( bool b ) {
		this->ignoreHeightMap = b;
	}
	inline bool getIgnoreHeightMap() {
		return this->ignoreHeightMap;
	}
	
	inline void setNotFlatten( bool b ) {
		this->notFlatten = b;
	}
	inline bool getNotFlatten() {
		return this->notFlatten;
	}

public:

	/**
	   Passing 0 for texture disables the creation of
	   shapes. (eg. torch, md2 shape)
	*/
	GLShape( Texture texture[], int width, int depth, int height, char const* name, int descriptionGroup,
	         Uint32 color, Uint8 shapePalIndex = 0 );

	virtual ~GLShape();

	virtual void initialize();
	virtual inline void cleanup() {}

	static void createDarkTexture( WallTheme *theme );
	void setTexture( Texture* textureGroup );

	inline void setSkipSide( int n ) {
		skipside = n;
	}
	//bool fitsInside( GLShape *smaller, bool relaxedRules = false );
	bool isLightBlocking();
	void setLightBlocking( bool b );
	void draw();
	void outline( float r, float g, float b );

	inline Uint8 getShapePalIndex() {
		return shapePalIndex;
	}

	inline void setCameraRot( GLfloat xrot, GLfloat yrot, GLfloat zrot ) {
		this->xrot = xrot; this->yrot = yrot; this->zrot = zrot;
	}
	inline void setCameraPos( GLfloat xpos, GLfloat ypos, GLfloat zpos, GLfloat xpos2, GLfloat ypos2, GLfloat zpos2 ) {
		this->xpos = xpos; this->ypos = ypos; this->zpos = zpos; this->xpos2 = xpos2; this->ypos2 = ypos2; this->zpos2 = zpos2;
	}
	inline void setLocked( bool locked ) {
		this->locked = locked;
	}
	inline GLfloat getYRot() {
		return yrot;
	}
	inline GLfloat getZRot() {
		return zrot;
	}

	// if true, the next two functions are called
	inline bool isBlended() {
		return false;
	}
	//inline void setupBlending() { glBlendFunc(GL_ONE_MINUS_SRC_COLOR, GL_SRC_ALPHA); }
	void setupBlending();
	inline void endBlending() { }
	inline void setIconRotation( float x, float y, float z ) {
		xIconRot = x; yIconRot = y; zIconRot = z;
	}
	inline void applyIconRotation() {
		glRotatef( xIconRot, 1.0f, 0.0f, 0.0f ); glRotatef( yIconRot, 0.0f, 1.0f, 0.0f ); glRotatef( zIconRot, 0.0f, 0.0f, 1.0f );
	}
	inline void setIcon( Texture n, int w, int h ) {
		icon = n; iconWidth = w; iconHeight = h;
	}
	inline Texture getIcon() {
		return icon;
	}
	inline void setAmbientName( std::string const& s ) {
		this->ambient = s;
	}
	inline std::string& getAmbientName() {
		return ambient;
	}
	inline int getIconWidth() {
		return iconWidth;
	}
	inline int getIconHeight() {
		return iconHeight;
	}

	virtual void setCurrentAnimation ( int numAnim, bool force = false );
	virtual void setPauseAnimation( bool pause );

	virtual inline int getEffectType() {
		return effectType;
	}
	virtual inline int getEffectWidth() {
		return effectWidth;
	}
	virtual inline int getEffectDepth() {
		return effectDepth;
	}
	virtual inline int getEffectHeight() {
		return effectHeight;
	}
	virtual inline int getEffectX() {
		return effectX;
	}
	virtual inline int getEffectY() {
		return effectY;
	}
	virtual inline int getEffectZ() {
		return effectZ;
	}
	virtual inline void setEffectType( int n, int w, int d, int h, int x, int y, int z ) {
		effectType = n;
		effectWidth = w;
		effectDepth = d;
		effectHeight = h;
		effectX = x;
		effectY = y;
		effectZ = z;
	}
	inline void setIconRotation( int iconRotX, int iconRotY, int iconRotZ ) {
		this->iconRotX = iconRotX;
		this->iconRotY = iconRotY;
		this->iconRotZ = iconRotZ;
	}
	inline void rotateIcon() {
		glRotatef( iconRotX, 1.f, 0.f, 0.f );
		glRotatef( iconRotY, 0.f, 1.f, 0.f );
		glRotatef( iconRotZ, 0.f, 0.f, 1.f );
	}

	virtual inline bool isShownInMapEditor() {
		return true;
	}

	void addVirtualShape( int x, int y, int z, int w, int d, int h, bool draws );
	inline bool hasVirtualShapes() {
		return virtualShapes.size() > 0;
	}
	inline std::vector<GLShape*> *getVirtualShapes() {
		return &virtualShapes;
	}
	void clearVirtualShapes( bool freeMemory );
protected:
	float getLight( float *normal );
	bool locked;
	GLfloat xrot, yrot, zrot;
	GLfloat xpos, ypos, zpos, xpos2, ypos2, zpos2;
	void commonInit( Texture tex[], Uint32 color, Uint8 shapePalIndex );
	static Surface *new_surface( float vertices[4][3] );
	void initSurfaces();
	void drawShadow();
	void createShadowList( GLuint listName );
	void createBodyList( int side, GLuint listName );
	void createTopList( GLuint listName );
	void resetMultiTexture(); 
	DECLARE_NOISY_OPENGL_SUPPORT();
};

#endif
