/***************************************************************************
                          debugshape.h  -  description
                             -------------------
    begin                : Tue Sep 23 2003
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

#ifndef DEBUGSHAPE_H
#define DEBUGSHAPE_H

#include "glshape.h"

/**
  *@author Gabor Torok
  */

class DebugShape : public GLShape  {
private:
  GLfloat zz, yy, xx;
  GLShape *inner;
public: 
  DebugShape::DebugShape(GLuint texture[],
          int width, int depth, int height,
          char *name,
          Uint32 color, GLuint display_list, Uint8 shapePalIndex);
          
  DebugShape::DebugShape(GLuint texture[],
          int width, int depth, int height,
          char *name, char **description, int descriptionCount,
          Uint32 color, GLuint display_list, Uint8 shapePalIndex);
	~DebugShape();

  void draw();
};

#endif
