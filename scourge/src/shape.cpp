/***************************************************************************
                          shape.cpp  -  description
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

#include "shape.h"

Shape::Shape(int width, int depth, int height, char *name, char **description, int descriptionCount){
  this->width = width;
  this->depth = depth;
  this->height = height;
  this->name = name;
  this->description = description;
  this->descriptionCount = descriptionCount;
  this->stencil = false;
}

Shape::Shape(Shape *shape) {
	this->width = shape->getWidth();
	this->depth = shape->getDepth();
	this->height = shape->getHeight();
  this->name = shape->getName();
  this->description = shape->getDescription();
  this->descriptionCount = shape->getDescriptionCount();  
}

Shape::~Shape(){
}

char *Shape::getRandomDescription() {
  if(descriptionCount == 0) return NULL;
  else if(descriptionCount == 1) return description[0];
  else {
    int n = (int) ((float)(descriptionCount)*rand()/(RAND_MAX+1.0));
    return description[n];
  }
}


