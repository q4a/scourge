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

//#define DEBUG_MD2 1

MD2Shape::MD2Shape(t3DModel * g_3DModel, GLuint textureId, float div,
                   GLuint texture[],
                   int width, int depth, int height,
                   char *name, int descriptionGroup,
                   Uint32 color, Uint8 shapePalIndex) :
  GLShape(0, width, depth, height, name, descriptionGroup, color, shapePalIndex) {
  commonInit(g_3DModel, textureId, div);    
  debugShape = new GLShape(0, this->width, this->depth, 1, name, descriptionGroup, color, shapePalIndex);
  debugShape->initialize();
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
  this->debug = false;
  
  // Animation stuff
  elapsedTime = 0.0f;
  lastTime = 0.0f;  
  pauseAnimation = false;  
  currentAnim = MD2_STAND;
  currentFrame = 1;
  playedOnce = true;
  animationWaiting = -1;      

  attackEffect = false;
  setDir(Constants::MOVE_UP);
  
  vect = new vect3d [g_3DModel->numVertices]; 
  for(int i = 0; i < g_3DModel->numVertices; i++){
    for(int j = 0; j < 3; j++){
        vect[i][j] = 0.0;
    }
  }         
}

void MD2Shape::draw() {

#ifdef DEBUG_MD2
  if( glIsEnabled( GL_TEXTURE_2D ) ) {
    glPushMatrix();
    debugShape->draw();
    glPopMatrix();
  }
#endif

  glPushMatrix();

  // rotate to upright
  glRotatef( 90.0f, 1.0f, 0.0f, 0.0f );

  // move to the middle of the space
  glTranslatef( ((float)width / DIV) / 2.0f, 0.25f / DIV, -((float)depth / DIV) / 2.0f );

  // rotate to movement angle
  glRotatef(angle - 90, 0.0f, 1.0f, 0.0f);

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

void MD2Shape::setDir(int dir) { 
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

void MD2Shape::setCurrentAnimation(int numAnim, bool force){    
  if(numAnim != currentAnim && numAnim >= 0 && numAnim <= MD2_CREATURE_ACTION_COUNT){
    if(force || playedOnce){
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

void MD2Shape::setupToDraw() {

}

// This returns time t for the interpolation between the current and next key frame
float MD2Shape::ReturnCurrentTime(int nextFrame)
{       
    float time = SDL_GetTicks();   
    elapsedTime = time - lastTime;    
    //float speed = ANIMATION_SPEED;
    
    float speed = (7.0f - (7.0f * (creatureSpeed / 10.0f))) + 3.0f;
    if( speed < 2.0f ) speed = 2.0f;
    if( speed > 10.0f ) speed = 10.0f;

    float t = elapsedTime / (1000.0f / speed);
    
    // If elapsed time goes over a 5th of a second, we go to the next key frame
    if (elapsedTime >= (1000.0f / speed) )
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
    if(!(currentAnim == MD2_STAND || currentAnim == MD2_RUN)) {
      if(animationWaiting == - 1){
        setCurrentAnimation(MD2_STAND);
      } else{
        setCurrentAnimation(animationWaiting);
        animationWaiting = -1;
      }
      setAttackEffect(false);
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

// factory method to create shape
MD2Shape *MD2Shape::createShape(t3DModel *g_3DModel, GLuint textureId, float div,
                                GLuint texture[], char *name, int descriptionGroup,
                                Uint32 color, Uint8 shapePalIndex) {
  vect3d min;
  vect3d max;
  int width, depth, height;

  // bogus initial value
  width = depth = height = 1;

  // mins/max-s
  vect3d *point = &g_3DModel->vertices[ g_3DModel->numVertices * g_3DModel->pAnimations[MD2_STAND].startFrame ];
  min[0] = min[1] = min[2] = 100000.0f; // BAD!!
  max[0] = max[1] = max[2] = 0.0f;
  for(int i = 0; i < g_3DModel->numVertices; i++) {
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
    
  // set the shape's dimensions
  width = (int)(fw + 0.5f);
  depth = (int)(fd + 0.5f);
  height = (int)(fh + 0.5f);
  
  // normalize and center points
  map<int, int> seenFrames;
  for(int r = 0; r < MD2_CREATURE_ACTION_COUNT; r++) {

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
    
    for(int a = g_3DModel->pAnimations[r].startFrame; a < g_3DModel->pAnimations[r].endFrame; a++) {
      if(seenFrames.find(a) == seenFrames.end()) {
        point = &g_3DModel->vertices[ g_3DModel->numVertices * a ];
        for(int i = 0; i < g_3DModel->numVertices; i++) {
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

  return new MD2Shape(g_3DModel,textureId,div,texture,width,depth,height,
                      name,descriptionGroup,color,shapePalIndex);
}

