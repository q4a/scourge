/**
 * Credit for this code is mainly due to:
 * DigiBen     digiben@gametutorials.com
 * Look up his other great tutorials at:
 * http://www.gametutorials.com
 *
 * And to David Henry (http://tfc.duke.free.fr/us/tutorials/models/md2.htm)
 * for glCommands.
 */

/***************************************************************************
                          md2shape.cpp  -  description
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

#include <string>  
#include "Md2.h"
#include "md2shape.h"

MD2Shape::MD2Shape(t3DModel * g_3DModel, GLuint textureId, float div,
                   GLuint texture[],
                   int width, int depth, int height,
                   char *name,
                   Uint32 color, Uint8 shapePalIndex) :
  // passing 0 for texture causes glshape to not init, 
  // 0 for display_list means : no display list available
#ifdef DEBUG_MD2
  GLShape(texture, width, depth, height, name, color, display_list, shapePalIndex) {
#else 
  //GLShape(0, width, depth, height, name, color, display_list, shapePalIndex) {
  GLShape(0, width, depth, height, name, color, 0, shapePalIndex) {
#endif    
  commonInit(g_3DModel, textureId, div);    
}

MD2Shape::MD2Shape(t3DModel * g_3DModel, GLuint textureId, float div,
                   GLuint texture[],
                   int width, int depth, int height,
                   char *name, char **description, int descriptionCount,
                   Uint32 color, Uint8 shapePalIndex) :
  // passing 0 for texture causes glshape to not init
  // 0 for display_list means : no display list available
#ifdef DEBUG_MD2
  GLShape(texture, width, depth, height, name, description, descriptionCount, color, display_list, shapePalIndex) {
#else
  //GLShape(0, width, depth, height, name, description, descriptionCount, color, display_list, shapePalIndex) {
  GLShape(0, width, depth, height, name, description, descriptionCount, color, 0, shapePalIndex) {
#endif
  commonInit(g_3DModel, textureId, div);    
}

MD2Shape::~MD2Shape() {
  delete [] vect;
}

void MD2Shape::commonInit(t3DModel * g_3DModel, GLuint textureId,  float div) {
  this->g_3DModel = g_3DModel;    
  g_ViewMode = GL_TRIANGLES;
  this->attackEffect = false;
  this->div = div; 
  this->textureId = textureId;
  
  // Animation stuff
  elapsedTime = 0.0f;
  lastTime = 0.0f;  
  pauseAnimation = false;  
  currentAnim = MD2_STAND;
  currentFrame = 1;
  playedOnce = true;
  animationWaiting = -1;                
  

  vect = new vect3d [g_3DModel->numVertices]; 
  for(int i = 0; i < g_3DModel->numVertices; i++){
    for(int j = 0; j < 3; j++){
        vect[i][j] = 0.0;
    }
  }         
}



void MD2Shape::setCurrentAnimation(int numAnim){    
    if(numAnim != currentAnim && numAnim >= 0 && numAnim <= MD2_CREATURE_ACTION_COUNT){
        if(playedOnce){
            currentAnim = numAnim;                
            currentFrame = g_3DModel->pAnimations[currentAnim].startFrame; 
                                     
            // MD2_STAND animation is too long, so we make it "interruptible"
            if(currentAnim != MD2_STAND){
                playedOnce = false;                    
            }                
        }
        else{
            // if animationWaiting != -1 there is already an animation waiting
            // and we store only one at a time
            if(animationWaiting == -1){
                animationWaiting = currentAnim;
            }
        }
    }
}

void MD2Shape::draw() {
#ifdef DEBUG_MD2
  // draw the outline for debugging
  GLShape::draw();
#endif  

  glPushMatrix();

  // To make our model render somewhat faster, we do some front back culling.
  // It seems that Quake2 orders their polygons clock-wise.  
  glEnable( GL_CULL_FACE );
  glCullFace( GL_BACK );
  
  glRotatef(-90.0f, 0.0f, 1.0f, 0.0f);
  glRotatef(-90.0f, 0.0f, 0.0f, 1.0f);
  glTranslatef(-g_3DModel->movex / 2.0f * div, 0.0f, 0.0f);
  glTranslatef(0.0f, g_3DModel->movey * div, 0.0f);
  glTranslatef(0.0f, 0.0f, -g_3DModel->movez / 2.0f * div);   

//    glRotatef(180.0f, 0.0f, 1.0f, 0.0f);
  switch(dir) {
    case Constants::MOVE_UP:
      glRotatef(0.0f, 0.0f, 1.0f, 0.0f); break;
    case Constants::MOVE_LEFT:
      glRotatef(-90.0f, 0.0f, 1.0f, 0.0f); break;
    case Constants::MOVE_RIGHT:
      glRotatef(90.0f, 0.0f, 1.0f, 0.0f); break;
    default:
      glRotatef(180.0f, 0.0f, 1.0f, 0.0f);
  }  
   
  AnimateMD2Model();      
 //cout << textureId << " " << g_3DModel->pAnimations[currentAnim].strName << " frame " << currentFrame <<       
  //      "/" << g_3DModel->pAnimations[currentAnim].endFrame << endl; 
 
  glDisable(GL_CULL_FACE);
  glPopMatrix();    
}



// This returns time t for the interpolation between the current and next key frame
float MD2Shape::ReturnCurrentTime(int nextFrame)
{       
    float time = SDL_GetTicks();   
    elapsedTime = time - lastTime;    
    float t = elapsedTime / (1000.0f / ANIMATION_SPEED);
    
    // If elapsed time goes over a 5th of a second, we go to the next key frame
    if (elapsedTime >= (1000.0f / ANIMATION_SPEED) )
    {
        // Set current frame to the next key frame (which could be the start of the anim)
        if(!pauseAnimation){
            currentFrame = nextFrame;
        }               
        lastTime = time;
    }
    return t;
}


// This draws and animates the .md2 model by interpoloated key frame animation
void MD2Shape::AnimateMD2Model()
{
    int *ptricmds ;
    int nb;               
        
    tAnimationInfo *pAnim = &(g_3DModel->pAnimations[currentAnim]);
    int nextFrame = (currentFrame + 1) % pAnim->endFrame;
    
    // MD2_STAND and MD2_TAUNT animations must be played only once 
    if(nextFrame == 0){        
        nextFrame =  pAnim->startFrame;
        playedOnce = true;        
		if(currentAnim == MD2_ATTACK) {
		  if(animationWaiting == - 1){
    		  setCurrentAnimation(MD2_STAND);
		  }
		  else{
		      setCurrentAnimation(animationWaiting);
		      animationWaiting = -1;
          }
		  setAttackEffect(false);
		}
        else if(currentAnim == MD2_TAUNT) {
          if(animationWaiting == - 1){
    		  setCurrentAnimation(MD2_STAND);
		  }
		  else{
		      setCurrentAnimation(animationWaiting);
		      animationWaiting = -1;
          }
		}
    } 

    // t = [0, 1] => 0 : beginning of the animation, 1 : end of the animation    
    float t = ReturnCurrentTime(nextFrame);
                        
    glBindTexture(GL_TEXTURE_2D, textureId);
    
    // Compute interpolated vertices        
    vect3d * currVertices, * nextVertices;    
    currVertices = &g_3DModel->vertices[ g_3DModel->numVertices * currentFrame ];
    nextVertices = &g_3DModel->vertices[ g_3DModel->numVertices * nextFrame ];
    if(!pauseAnimation){    
        for(int i = 0; i < g_3DModel->numVertices ; i++){
            vect[i][0] = (currVertices[i][0] + t * (nextVertices[i][0] - currVertices[i][0])) * div;
            vect[i][1] = (currVertices[i][1] + t * (nextVertices[i][1] - currVertices[i][1])) * div;
            vect[i][2] = (currVertices[i][2] + t * (nextVertices[i][2] - currVertices[i][2])) * div;    
        }
    }
    else{
        for(int i = 0; i < g_3DModel->numVertices ; i++){
            vect[i][0] = currVertices[i][0]  * div;
            vect[i][1] = currVertices[i][1]  * div;
            vect[i][2] = currVertices[i][2]  * div;    
        }    
    }
            
    ptricmds = g_3DModel->pGlCommands;
    nb = *(ptricmds);    
    ptricmds++;
    while(nb != 0){        
    
        if( nb < 0 ){
            glBegin( GL_TRIANGLE_FAN );
            nb = -nb;
        }
        else{
            glBegin( GL_TRIANGLE_STRIP );
        }
        
        while(nb > 0){
            // ptricmds[0]  :   s texture coordinate
            // ptricmds[1]  :   t texture coordinate
            // ptricmds[2]  :   vertex index to draw            
            glTexCoord2f(((float *)ptricmds)[0], 1.0f - ((float *)ptricmds)[1]);            
            glVertex3fv( vect[ ptricmds[2] ] );
            nb--;
            ptricmds+=3;            
        }
			
        glEnd();        
        nb = *(ptricmds);
        ptricmds++;
    }  
}


void MD2Shape::setupBlending() { 
    glBlendFunc(GL_ONE, GL_ONE); 
}

void MD2Shape::endBlending() { 
}

bool MD2Shape::drawFirst() { 
    return true; 
}
  // if true, the next two functions are called
bool MD2Shape::drawLater() { 
    return false; 
}
