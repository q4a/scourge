/**
 * Credit for this code is mainly due to:
 * DigiBen     digiben@gametutorials.com
 * Look up his other great tutorials at:
 * http://www.gametutorials.com
 */


/***************************************************************************
                          md2shape.h  -  description
                             -------------------
    begin                : Fri Oct 3 2003
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

#ifndef MD2SHAPE_H
#define MD2SHAPE_H

#include <iostream>
#include <stdio.h>
#include <stdlib.h>

# include <string>
# include <vector>

#include "constants.h"
#include "glshape.h"
#include "Md2.h"

class MD2Shape : public GLShape  {

// uncomment to show debug shapes
//#define DEBUG_MD2 1

private:
  bool attackEffect;
  float div;
  float movex, movey, movez;
  unsigned int g_Texture[MAX_TEXTURES]; // This holds the texture info, referenced by an ID
  CLoadMD2 g_LoadMd2;                   // This is MD2 class.  This should go in a good model class.
  t3DModel g_3DModel;                   // This holds the 3D Model info that we load in
  int g_ViewMode;                       // make this GL_LINE_STRIP for outline
  int dir;
  
  // Animation stuff
  float elapsedTime;
  float lastTime;
  int numAnimationWaiting;  // The "animation buffer" stores only one animation 
  bool animationPlayed;     // Has current animation been played at least once ?
   
  // This draws and animates the .md2 model by interpoloated key frame animation
  void AnimateMD2Model(t3DModel *pModel);
  
  // This returns time t for the interpolation between the current and next key frame
  float ReturnCurrentTime(t3DModel *pModel, int nextFrame);

public: 
	MD2Shape(char *file_name, char *texture_name, float div,
          GLuint texture[], int width, int depth, int height,
          char *name,
          Uint32 color, GLuint display_list, Uint8 shapePalIndex=0);

	MD2Shape(char *file_name, char *texture_name, float div,
          GLuint texture[], int width, int depth, int height,
          char *name, char **description, int descriptionCount,
          Uint32 color, GLuint display_list, Uint8 shapePalIndex=0);
          
	inline bool getAttackEffect() { return attackEffect; }
	inline void setAttackEffect(bool b) { attackEffect = b; }

    void setCurrentAnimation(int numAnim);
    
	~MD2Shape();

  void draw();

  inline void setDir(int dir) { this->dir = dir; }
  
  bool drawFirst();
  // if true, the next two functions are called
  bool drawLater();
  void setupBlending();
  void endBlending();  

protected:
  void commonInit(char *file_name, char *texture_name, float div);
};

#endif
