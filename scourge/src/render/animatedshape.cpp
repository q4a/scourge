/***************************************************************************
                          animatedshape.cpp  -  description
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

#include <string>  
#include "modelwrapper.h"
#include "animatedshape.h"
#include "md2shape.h"
#include "md3shape.h"

using namespace std;

AnimatedShape::AnimatedShape( int width, int depth, int height, 
															char *name, int descriptionGroup,
															Uint32 color, Uint8 shapePalIndex ) :
  GLShape( 0, width, depth, height/2, name, descriptionGroup, color, shapePalIndex ) {
  
  this->g_ViewMode = GL_TRIANGLES;
  this->attackEffect = false;
  this->debug = false;  
	this->useShadow = false;
  setDir(Constants::MOVE_UP); // sets dir, angle
	this->creatureSpeed = 5;
	this->currentAnim = 0;
	this->pauseAnimation = false;
	this->skin_name = NULL;
	
#ifdef DEBUG_MD2
  debugShape = new GLShape( 0, this->width, this->depth, this->height, 
                            name, descriptionGroup, color, shapePalIndex );
  debugShape->initialize();
#endif
}

AnimatedShape::~AnimatedShape() {
}

void AnimatedShape::setDir(int dir) { 
  this->dir = dir; 
  switch(dir) {
  case Constants::MOVE_UP:
    angle = 0.0f; 
    break;
  case Constants::MOVE_LEFT:
    angle = -90.0f; break;
  case Constants::MOVE_RIGHT:
    angle = 90.0f;break;
  default:
    angle = 180.0f;
  }
}

//void AnimatedShape::draw() {
//}

//void AnimatedShape::outline( float r, float g, float b ) {
//}

//void AnimatedShape::setupToDraw() {
//}

void AnimatedShape::setupBlending() { 
	glBlendFunc(GL_ONE, GL_ONE); 
}

void AnimatedShape::endBlending() { 
}

bool AnimatedShape::drawFirst() { 
	return true; 
}
  // if true, the next two functions are called
bool AnimatedShape::drawLater() { 
	return false; 
}

// factory method to create shape
AnimatedShape *AnimatedShape::createShape( ModelWrapper *modelWrapper, GLuint textureId, float div,
                                					 GLuint texture[], char *name, int descriptionGroup,
                                					 Uint32 color, Uint8 shapePalIndex) {
  vect3d min;
  vect3d max;
  int width, depth, height;

	// fixme: will not work for md3  
  if( modelWrapper->getMd2() ) {

	  // bogus initial value
	  width = depth = height = 1;

	  // mins/max-s
	//  vect3d *point = &g_3DModel->vertices[ g_3DModel->numVertices * 
	//  																			g_3DModel->pAnimations[MD2_STAND].startFrame ];
	  vect3d *point = &modelWrapper->getMd2()->vertices[ 
	  		modelWrapper->getMd2()->numVertices * 
				modelWrapper->getMd2()->pAnimations[0].startFrame ];  																			
	  min[0] = min[1] = min[2] = 100000.0f; // BAD!!
	  max[0] = max[1] = max[2] = 0.0f;
	  for(int i = 0; i < modelWrapper->getMd2()->numVertices; i++) {
	    for(int t = 0; t < 3; t++) if(point[i][t] < min[t]) min[t] = point[i][t];
	    for(int t = 0; t < 3; t++) if(point[i][t] >= max[t]) max[t] = point[i][t];
	  }  
	  for(int t = 0; t < 3; t++) max[t] -= min[t];
	  
	  // set the dimensions
	  float fw = max[2] * div * DIV;
	  float fd = max[0] * div * DIV;
	  float fh = max[1] * div * DIV;
	  
	  // make it a square
	  if(fw > fd) fd = fw;
	  else fw = fd;

	  // make it a min size (otherwise pathing has issues)
	  if( fw < 3 ) fw = 3;
	  if( fd < 3 ) fd = 3;
	    
	  // set the shape's dimensions
	  width = (int)( fw + ( (float)((int)fw) == fw ? 0 : 1 ) );
	  depth = (int)( fd + ( (float)((int)fd) == fd ? 0 : 1 ) );
	  height = toint(fh);
	  
	  // normalize and center points
	  map<int, int> seenFrames;
	//  for(int r = 0; r < MD2_CREATURE_ACTION_COUNT; r++) {
	  for(int r = 0; r < (int)modelWrapper->getMd2()->pAnimations.size(); r++) {  

	    /*
	    // An attempt to keep creatures inside their cirlces.
	    // not sure it does anything...
	    // (Be sure to not set min/max for z.)
	    // local min/max for x,y (not z!)
	    vect3d *point = &g_3DModel->vertices[ g_3DModel->numVertices * g_3DModel->pAnimations[r].startFrame ];
	    min[0] = min[2] = 100000.0f; // BAD!!
	    max[0] = max[2] = 0.0f;
	    for(int i = 0; i < g_3DModel->numVertices; i++) {
	      if(point[i][0] < min[0]) min[0] = point[i][0];
	      if(point[i][2] < min[2]) min[2] = point[i][2];
	      if(point[i][0] >= max[0]) max[0] = point[i][0];
	      if(point[i][2] >= max[2]) max[2] = point[i][2];
	    }  
	    max[0] -= min[0];
	    max[2] -= min[2];
	    */
	    
	    for(int a = modelWrapper->getMd2()->pAnimations[r].startFrame; 
	    			a < modelWrapper->getMd2()->pAnimations[r].endFrame; a++) {
	      if(seenFrames.find(a) == seenFrames.end()) {
	        point = &modelWrapper->getMd2()->vertices[ modelWrapper->getMd2()->numVertices * a ];
	        for(int i = 0; i < modelWrapper->getMd2()->numVertices; i++) {
	          for(int t = 0; t < 3; t++) point[i][t] -= min[t];
	          for(int t = 0; t < 3; t++) if(t != 1) point[i][t] -= (max[t] / 2.0f);
	        }
	        seenFrames[a] = 1;
	      }
	    }
	  }

#ifdef DEBUG_MD2
  cerr << "Creating MD2 shape for model=" << name << 
    " width=" << width << " depth=" << depth << " height=" << height << endl;
#endif
	} else {
		width = height = depth = 3;
	}
	
	if( modelWrapper->getMd2() ) {
	  return new MD2Shape( modelWrapper->getMd2(),textureId,div,texture,width,depth,height,
  	                     name,descriptionGroup,color,shapePalIndex );
	} else if( modelWrapper->getMd3() ) {
		  return new MD3Shape( modelWrapper->getMd3(),div,texture,width,depth,height,
  	                       name,descriptionGroup,color,shapePalIndex );	
	} else {
		cerr << "*** Error: Can't create animated shape." << endl;
		return NULL;
	}
}

