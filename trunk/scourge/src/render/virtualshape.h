/***************************************************************************
                          virtualshape.h  -  description
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

#ifndef VIRTUALSHAPE_H_
#define VIRTUALSHAPE_H_

#include "glshape.h"

/**
  *@author Gabor Torok
  */
class VirtualShape : public GLShape  {
private:
	int offsetX, offsetY, offsetZ;
	bool draws;
	GLShape *refShape;
  
public:
  VirtualShape( char *name,
                int width, int height, int depth,
                int offsetX, int offsetY, int offsetZ,
                bool draws, GLShape *refShape,
                int shapePalIndex ); 
	
  ~VirtualShape();
  
  void draw();
  void outline( float r, float g, float b );
  virtual inline bool isVirtual() { return true; }
};

#endif /*VIRTUALSHAPE_H_*/
