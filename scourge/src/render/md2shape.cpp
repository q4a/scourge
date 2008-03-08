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

using namespace std;

//#define DEBUG_MD2 1

MD2Shape::MD2Shape( t3DModel * g_3DModel, GLuint textureId, float div, 
										GLuint texture[], int width, int depth, int height,
										char *name, int descriptionGroup, Uint32 color, Uint8 shapePalIndex ) :
  AnimatedShape( width, depth, height, name, descriptionGroup, color, shapePalIndex ) {
  commonInit( g_3DModel, textureId, div );
}

MD2Shape::~MD2Shape() {
  delete [] vect;
}

void MD2Shape::commonInit(t3DModel * g_3DModel, GLuint textureId,  float div) {
  this->g_3DModel = g_3DModel;    
  this->div = div; 
  this->textureId = textureId;
  
  // Animation stuff
  elapsedTime = 0.0f;
  lastTime = 0.0f;  
  // currentAnim = MD2_STAND;
  currentFrame = 1;
  //playedOnce = true;  
  
  vect = new vect3d [g_3DModel->numVertices]; 
  for(int i = 0; i < g_3DModel->numVertices; i++){
    for(int j = 0; j < 3; j++){
        vect[i][j] = 0.0;
    }
  }         
}

void MD2Shape::draw() {

#ifdef DEBUG_MD2
  //if( glIsEnabled( GL_TEXTURE_2D ) ) {
    glPushMatrix();
    debugShape->draw();
    glPopMatrix();
  //}
#endif

  glPushMatrix();

  // rotate to upright
  glRotatef( 90.0f, 1.0f, 0.0f, 0.0f );

  // move to the middle of the space
  //glTranslatef( (static_cast<float>(width) / DIV) / 2.0f, 
                //0.25f / DIV, 
                //-(static_cast<float>(depth) / DIV) / 2.0f );
  glTranslatef( (static_cast<float>(width) / 2.0f) / DIV, 0.25f / DIV, -((static_cast<float>(depth) / 2.0f) / DIV ) );

  // rotate to movement angle
  glRotatef(getAngle() - 90, 0.0f, 1.0f, 0.0f);

  // To make our model render somewhat faster, we do some front back culling.
  // It seems that Quake2 orders their polygons clock-wise.  
  glEnable( GL_CULL_FACE );
  glCullFace( GL_BACK );
  bool textureWasEnabled = glIsEnabled( GL_TEXTURE_2D );
  glEnable( GL_TEXTURE_2D );
	
  AnimateMD2Model();      
 
  if( !textureWasEnabled ) glDisable( GL_TEXTURE_2D );
  glDisable(GL_CULL_FACE);
  glPopMatrix();    
}

void MD2Shape::outline( float r, float g, float b ) {
	useShadow = true;
  GLboolean blend;
  glGetBooleanv( GL_BLEND, &blend );
  //glEnable( GL_BLEND );
  //glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );
	GLboolean texture = glIsEnabled( GL_TEXTURE_2D );
  glDisable( GL_TEXTURE_2D );
	glFrontFace( GL_CCW );
  glPolygonMode( GL_BACK, GL_LINE );
  glLineWidth( 4 );
  glEnable( GL_CULL_FACE );
  glCullFace( GL_FRONT );
  glColor3f( r, g, b );  
  
	glPushMatrix();
  // rotate to upright
  glRotatef( 90.0f, 1.0f, 0.0f, 0.0f );
  glTranslatef( (static_cast<float>(width) / 2.0f) / DIV, 0.25f / DIV, -((static_cast<float>(depth) / 2.0f) / DIV ) );

  // rotate to movement angle
  glRotatef(getAngle() - 90, 0.0f, 1.0f, 0.0f);
  AnimateMD2Model();
  glPopMatrix();

	glLineWidth( 1 );
  glDisable( GL_CULL_FACE );
  glPolygonMode( GL_BACK, GL_FILL );
	if( !blend ) glDisable( GL_BLEND );
	if( texture ) glEnable( GL_TEXTURE_2D );
  useShadow = false;
  glColor4f(1, 1, 1, 0.9f);	
}

void MD2Shape::setModelAnimation() {
	if( currentAnim >= g_3DModel->pAnimations.size() ) {
		currentAnim = g_3DModel->pAnimations.size() - 1;
	}
	currentFrame = g_3DModel->pAnimations[ currentAnim ].startFrame; 
}

void MD2Shape::setupToDraw() {
}

// This returns time t for the interpolation between the current and next key frame
float MD2Shape::ReturnCurrentTime(int nextFrame) {       
    float time = SDL_GetTicks();   
    elapsedTime = time - lastTime;    
    //float speed = ANIMATION_SPEED;
    
    float speed = (7.0f - (7.0f * (getCreatureSpeed() / 10.0f))) + 3.0f;
		//float speed = ( 10.0f - getCreatureSpeed() );
    if( speed < 2.0f ) speed = 2.0f;
    if( speed > 10.0f ) speed = 10.0f;

    float t = elapsedTime / (1000.0f / speed);
    
    // If elapsed time goes over a 5th of a second, we go to the next key frame
    if (elapsedTime >= (1000.0f / speed) )
    {
        // Set current frame to the next key frame (which could be the start of the anim)
        if( !isAnimationPaused() ){
            currentFrame = nextFrame;
        }               
        lastTime = time;
    }
    return t;
}


// This draws and animates the .md2 model by interpoloated key frame animation
void MD2Shape::AnimateMD2Model() {
  int *ptricmds ;
  int nb;               

  tAnimationInfo *pAnim = &(g_3DModel->pAnimations[currentAnim]);
  int nextFrame;
  nextFrame = (currentFrame + 1) % pAnim->endFrame;
  
  // MD2_TAUNT animations must be played only once 
  if(nextFrame == 0){        
    nextFrame =  pAnim->startFrame;
    animationFinished();
  }  

  // t = [0, 1] => 0 : beginning of the animation, 1 : end of the animation    
  float t = ReturnCurrentTime(nextFrame);

  if( !useShadow ) glBindTexture(GL_TEXTURE_2D, textureId);

  // Compute interpolated vertices        
  vect3d * currVertices, * nextVertices;    
  currVertices = &g_3DModel->vertices[ g_3DModel->numVertices * currentFrame ];
  nextVertices = &g_3DModel->vertices[ g_3DModel->numVertices * nextFrame ];
  if( !isAnimationPaused() ){    
    for(int i = 0; i < g_3DModel->numVertices ; i++){
      vect[i][0] = (currVertices[i][0] + t * (nextVertices[i][0] - currVertices[i][0])) * div;
      vect[i][1] = (currVertices[i][1] + t * (nextVertices[i][1] - currVertices[i][1])) * div;
      vect[i][2] = (currVertices[i][2] + t * (nextVertices[i][2] - currVertices[i][2])) * div;    
    }
  } else{
    for(int i = 0; i < g_3DModel->numVertices ; i++){
      vect[i][0] = currVertices[i][0]  * div;
      vect[i][1] = currVertices[i][1]  * div;
      vect[i][2] = currVertices[i][2]  * div;    
    }    
  }

  /*
  if(currentAnim != MD2_RUN) {
    bool inAir = true;
    float min = -1;
    for(int i = 0; i < g_3DModel->numVertices; i++) {
      if(min == -1 || vect[i][1] < min) min = vect[i][1];
      if(vect[i][1] < 1.0f && vect[i][1] > -1.0f) {
        inAir = false;
        break;
      }
    }
    if(inAir) {
      cerr << getName() << " IN AIR. min=" << min << " currentAnim=" << currentAnim << " start=" << pAnim->startFrame << " end=" << pAnim->endFrame << " currentFrame=" << currentFrame << endl;
    }
  }
  */

  ptricmds = g_3DModel->pGlCommands;
  nb = *(ptricmds);    
  ptricmds++;
  while(nb != 0){        
    if( nb < 0 ){
      glBegin( GL_TRIANGLE_FAN );
      nb = -nb;
    } else{
      glBegin( GL_TRIANGLE_STRIP );
    }

    while(nb > 0){
      // ptricmds[0]  :   s texture coordinate
      // ptricmds[1]  :   t texture coordinate
      // ptricmds[2]  :   vertex index to draw            
      if( !useShadow ) glTexCoord2f(((float *)ptricmds)[0], 1.0f - ((float *)ptricmds)[1]);            
      glVertex3fv( vect[ ptricmds[2] ] );
      nb--;
      ptricmds+=3;            
    }

    glEnd();        
    nb = *(ptricmds);
    ptricmds++;
  }  
}       

