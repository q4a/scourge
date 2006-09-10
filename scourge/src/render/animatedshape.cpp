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
#include "Md2.h"

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
	this->animationWaiting = -1;      
	this->pauseAnimation = false;
	this->playedOnce = true;  
	
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

void AnimatedShape::animationFinished() {
	//cerr << "animationFinished for " << getName() << " anim=" << currentAnim << endl;
	playedOnce = true;            
	// some animations only run once
	if( !(currentAnim == MD2_STAND || currentAnim == MD2_RUN) ) {
		if( animationWaiting == - 1 ){
			setCurrentAnimation( MD2_STAND );
		} else{
			setCurrentAnimation( animationWaiting );
			animationWaiting = -1;
		}
		setAttackEffect( false );
	}
}

void AnimatedShape::setCurrentAnimation( int numAnim, bool force ){    
  if( numAnim != currentAnim && numAnim >= 0 && numAnim <= MD2_CREATURE_ACTION_COUNT ){
    if( ( force && currentAnim == MD2_RUN ) || playedOnce ){
      currentAnim = numAnim;                
      //currentFrame = g_3DModel->pAnimations[currentAnim].startFrame; 
			setModelAnimation();
      
      // MD2_STAND animation is too long, so we make it "interruptible"
      if( currentAnim != MD2_STAND ){
        playedOnce = false;                    
      }
    } else {
      // if animationWaiting != -1 there is already an animation waiting
      // and we store only one at a time
      if( animationWaiting == -1 ){
        animationWaiting = currentAnim;
      }
    }
  }
}
