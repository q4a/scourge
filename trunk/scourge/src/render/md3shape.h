/***************************************************************************
                          md3shape.h  -  description
                             -------------------
    begin                : Thu Aug 31 2006
    copyright            : (C) 2006 by Gabor Torok
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

#ifndef MD3SHAPE_H
#define MD3SHAPE_H

#include <iostream>
#include <stdio.h>
#include <stdlib.h>
# include <string>
# include <vector>
# include <map>

#include "render.h"
#include "animatedshape.h"
#include "Md3.h"

class MD3Shape : public AnimatedShape  {

// uncomment to show debug shapes
//#define DEBUG_MD3 1

private:
  float div;
	CModelMD3 *md3;

public:     
  MD3Shape( CModelMD3 *md3, float div,
            GLuint texture[], int width, int depth, int height,
            char *name, int descriptionGroup,
            Uint32 color, Uint8 shapePalIndex=0 );

	virtual ~MD3Shape();

  void setCurrentAnimation(int numAnim, bool force=false);
  void draw();
  void outline( float r, float g, float b );
  void setupToDraw();

protected:
	void setCurrentAnimation( int numAnim, bool force, int whichPart );

};

#endif
