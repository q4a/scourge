/***************************************************************************
                          gltorch.h  -  description
                             -------------------
    begin                : Sat Sep 20 2003
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

#ifndef GLTORCH_H
#define GLTORCH_H

#include "glshape.h"

class GLShape;

/**
  *@author Gabor Torok
  */

class GLTorch : public GLShape  {
private:
  GLuint flameTex;

  typedef struct _ParticleStruct {
    GLfloat x, y, z;
    GLint height;
  } ParticleStruct;

  static const int PARTICLE_COUNT = 30;
  ParticleStruct *particle[PARTICLE_COUNT];
  
  GLuint torchback;
  int torch_dir;
  
public:
	GLTorch(GLuint texture[], GLuint flameTex,
			int width, int depth, int height,
			char *name,
			Uint32 color, GLuint display_list, Uint8 shapePalIndex=0, 
			GLuint torchback=0, int torch_dir=Constants::NORTH);

	GLTorch(GLuint texture[], GLuint flameTex,
			int width, int depth, int height,
			char *name, char **description, int descriptionCount,
			Uint32 color, GLuint display_list, Uint8 shapePalIndex=0, 
			GLuint torchback=0, int torch_dir=Constants::NORTH);
	
  ~GLTorch();
  
  void draw();

  inline bool drawFirst() { return false; }
  // if true, the next two functions are called
  inline bool drawLater() { return true; }
  inline void setupBlending() { glBlendFunc(GL_SRC_ALPHA, GL_ONE); }

          
protected:
  void initParticles();
};

#endif
