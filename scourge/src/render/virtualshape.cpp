/***************************************************************************
                          virtualshape.cpp  -  description
                             -------------------
    begin                : Tue Jul 8 2008
    copyright            : (C) 2008 by Gabor Torok
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

#include "virtualshape.h"

VirtualShape::VirtualShape( char *name,
                            int width, int height, int depth,
                            int offsetX, int offsetY, int offsetZ,
                            bool draws, GLShape *refShape, 
                            int shapePalIndex ) :
  GLShape( NULL, 
           width, depth, height, 
           name, 
           refShape->getDescriptionGroup(), 
           1, 
           shapePalIndex ) {
	this->offsetX = offsetX;
	this->offsetY = offsetY;
	this->offsetZ = offsetZ;
	this->draws = draws;
	this->refShape = refShape;
}

VirtualShape::~VirtualShape() {
	// refShape is cleaned up elsewhere (and it could be pointed to by multiple shapes)
}

void VirtualShape::draw() {
	if( draws ) {
		refShape->draw();
	}
}

void VirtualShape::outline( float r, float g, float b ) {
	if( draws ) {
		refShape->outline( r, g, b );
	}
}
