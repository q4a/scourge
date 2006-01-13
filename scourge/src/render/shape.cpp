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

Shape::Shape(int width, int depth, int height, char *name, int descriptionGroup){
  this->width = width;
  this->depth = depth;
  this->height = height;
  this->name = name;
  this->descriptionGroup = descriptionGroup;
  this->stencil = false;
  this->outlineColor = NULL;
  interactive = false;
}

Shape::Shape(Shape *shape) {
  this->width = shape->getWidth();
  this->depth = shape->getDepth();
  this->height = shape->getHeight();
  this->name = shape->getName();
  this->descriptionGroup = shape->getDescriptionGroup();
  this->outlineColor = NULL;
  interactive = false;
}

Shape::~Shape(){
}


