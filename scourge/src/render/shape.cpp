/***************************************************************************
                  shape.cpp  -  Class for static 3D shapes
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

#include "../common/constants.h"
#include "shape.h"

Shape::Shape( int width, int depth, int height, char const* name, int descriptionGroup ) {
	this->width = width;
	this->depth = depth;
	this->height = height;
	if ( name != NULL ) this->name = name;
	this->descriptionGroup = descriptionGroup;
	this->stencil = false;
	this->outlineColor = NULL;
	this->textureCount = 0;
	this->textureIndex = -1; // use default texture
	groundSX = groundEX = groundSY = groundEY = 0;
	interactive = false;
	outdoorWeight = 0;
	outdoorShadow = false;
	wind = false;
	windInfo = NULL;
	this->roof = false;
	offsetX = offsetY = offsetZ = 0.0f;
	for ( int i = 0; i < 6; i++ ) occludedSides[i] = true;
}

Shape::Shape( Shape *shape ) {
	this->width = shape->getWidth();
	this->depth = shape->getDepth();
	this->height = shape->getHeight();
	this->name = shape->getName();
	this->descriptionGroup = shape->getDescriptionGroup();
	this->outlineColor = NULL;
	this->textureCount = shape->getTextureCount();
	this->textureIndex = shape->getTextureIndex();
	groundSX = groundEX = groundSY = groundEY = 0;
	interactive = false;
	outdoorWeight = 0;
	outdoorShadow = false;
	wind = false;
	windInfo = NULL;
	this->roof = shape->roof;
	this->offsetX = shape->getOffsX();
	this->offsetY = shape->getOffsY();
	this->offsetZ = shape->getOffsZ();
	for ( int i = 0; i < 6; i++ ) occludedSides[i] = true;
}

Shape::~Shape() {
}

void Shape::setOccludedSides( bool *sides ) {
	memcpy( this->occludedSides, sides, 6 * sizeof( bool ) );
}
