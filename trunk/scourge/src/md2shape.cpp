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
#include "md2shape.h"


MD2Shape::MD2Shape(char *file_name, char *texture_name, float div,
                   GLuint texture[],
                   int width, int depth, int height,
                   char *name,
                   Uint32 color, GLuint display_list, Uint8 shapePalIndex) :
  // passing 0 for texture causes glshape to not init
#ifdef DEBUG_MD2
  GLShape(texture, width, depth, height, name, color, display_list, shapePalIndex) {
#else
  GLShape(0, width, depth, height, name, color, display_list, shapePalIndex) {
#endif    
  commonInit(file_name, texture_name, div);    
}

MD2Shape::MD2Shape(char *file_name, char *texture_name, float div,
                   GLuint texture[],
                   int width, int depth, int height,
                   char *name, char **description, int descriptionCount,
                   Uint32 color, GLuint display_list, Uint8 shapePalIndex) :
  // passing 0 for texture causes glshape to not init
#ifdef DEBUG_MD2
  GLShape(texture, width, depth, height, name, description, descriptionCount, color, display_list, shapePalIndex) {
#else
  GLShape(0, width, depth, height, name, description, descriptionCount, color, display_list, shapePalIndex) {
#endif
  commonInit(file_name, texture_name, div);    
}

MD2Shape::~MD2Shape() {
  delete [] vect;
}

void MD2Shape::commonInit(char *file_name, char *texture_name, float div) {  
  g_Texture[0] = 0;
  g_ViewMode = GL_TRIANGLES;
  this->attackEffect = false;
  this->div = div;

  char fn[300], texfn[300];
  strcpy(fn, rootDir);
  strcat(fn, file_name);

  strcpy(texfn, rootDir);
  strcat(texfn, texture_name);   
  
  // Animation stuff
  elapsedTime = 0.0f;
  lastTime = 0.0f;  
  pauseAnimation = false;          
  
  // Loads the model with the given texture  
  // There is no color information for these models, and only one texture.    
  g_LoadMd2.ImportMD2(&g_3DModel, fn, texfn);
    
  // Go through all the materials and see if there is a 
  // file name to load for each material
  for(int i = 0; i < g_3DModel.numOfMaterials; i++) {    
    if(strlen(g_3DModel.pMaterials[i].strFile) > 0) {      
      cout << "CreateTexture " << endl;
      CreateTexture((GLuint*)g_Texture, g_3DModel.pMaterials[i].strFile, i);
      cout << g_3DModel.pMaterials[i].strFile << endl;
    }    
    g_3DModel.pMaterials[i].texureId = i;
  }    

  vect = new vect3d [g_3DModel.numVertices]; 
  for(int i = 0; i < g_3DModel.numVertices; i++){
    for(int j = 0; j < 3; j++){
        vect[i][j] = 0.0;
    }
  }
    
  // Find the lowest point
  float minx, miny, minz;  
  float maxx, maxy, maxz;
  minx = miny = minz = maxx = maxy = maxz = 0;
  for(int i = 0; i < g_3DModel.numVertices ; i++){
    if(g_3DModel.vertices[i][0] > maxx) maxx = g_3DModel.vertices[i][0];
    if(g_3DModel.vertices[i][1] > maxy) maxy = g_3DModel.vertices[i][1];
    if(g_3DModel.vertices[i][2] > maxz) maxz = g_3DModel.vertices[i][2];
    if(g_3DModel.vertices[i][0] < minx) minx = g_3DModel.vertices[i][0];
    if(g_3DModel.vertices[i][1] < miny) miny = g_3DModel.vertices[i][1];
    if(g_3DModel.vertices[i][2] < minz) minz = g_3DModel.vertices[i][2];     
  }
  movex = maxx - minx;
  movey = maxy;
  movez = maxz - minz;    
}

void MD2Shape::setCurrentAnimation(int numAnim){
    
    if(numAnim >= 0 && numAnim <= MD2_CREATURE_ACTION_COUNT){                
        if(numAnim != g_3DModel.currentAnim) {            
            g_3DModel.currentAnim = numAnim;                
            g_3DModel.currentFrame = g_3DModel.pAnimations[g_3DModel.currentAnim].startFrame;                      
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
  glTranslatef(-movex / 2.0f * div, 0.0f, 0.0f);
  glTranslatef(0.0f, movey * div, 0.0f);
  glTranslatef(0.0f, 0.0f, -movez / 2.0f * div);   

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
   
  AnimateMD2Model(&g_3DModel);
  /*string s;
  s = g_3DModel.pMaterials[0].strFile;
  s = s.substr(s.size()-6, 6);
  int numAnim = g_3DModel.currentAnim;
  cout << s << " " << g_3DModel.pAnimations[numAnim].strName << " frame " << g_3DModel.currentFrame <<       
        "/" << g_3DModel.pAnimations[numAnim].endFrame << endl; 
  */
 
  glDisable(GL_CULL_FACE);
  glPopMatrix();    
}



// This returns time t for the interpolation between the current and next key frame
float MD2Shape::ReturnCurrentTime(t3DModel *pModel, int nextFrame)
{       
    float time = SDL_GetTicks();   
    elapsedTime = time - lastTime;    
    float t = elapsedTime / (1000.0f / ANIMATION_SPEED);
    
    // If elapsed time goes over a 5th of a second, we go to the next key frame
    if (elapsedTime >= (1000.0f / ANIMATION_SPEED) )
    {
        // Set current frame to the next key frame (which could be the start of the anim)
        if(!pauseAnimation){
            pModel->currentFrame = nextFrame;
        }               
        lastTime = time;
    }
    return t;
}


// This draws and animates the .md2 model by interpoloated key frame animation
void MD2Shape::AnimateMD2Model(t3DModel *pModel)
{
    int *ptricmds ;
    int nb;               
        
    tAnimationInfo *pAnim = &(pModel->pAnimations[pModel->currentAnim]);
    int nextFrame = (pModel->currentFrame + 1) % pAnim->endFrame;
    
    // If next frame == 0, we need to start the animation over
    if(nextFrame == 0){        
        nextFrame =  pAnim->startFrame;
		if(g_3DModel.currentAnim == MD2_ATTACK) {
		  setCurrentAnimation(MD2_STAND);
		  setAttackEffect(false);
		}
        /*else if(g_3DModel.currentAnim == MD2_TAUNT) {
		  setCurrentAnimation(MD2_STAND);
		}*/		        
    }

    // t = [0, 1] => 0 : beginning of the animation, 1 : end of the animation    
    float t = ReturnCurrentTime(pModel, nextFrame);
            
    if(getUseTexture()) glBindTexture(GL_TEXTURE_2D, g_Texture[0]);           
    
    // Compute interpolated vertices        
    vect3d * currVertices, * nextVertices;    
    currVertices = &pModel->vertices[ pModel->numVertices * pModel->currentFrame ];
    nextVertices = &pModel->vertices[ pModel->numVertices * nextFrame ];
    if(!pauseAnimation){    
        for(int i = 0; i < pModel->numVertices ; i++){
            vect[i][0] = (currVertices[i][0] + t * (nextVertices[i][0] - currVertices[i][0])) * div;
            vect[i][1] = (currVertices[i][1] + t * (nextVertices[i][1] - currVertices[i][1])) * div;
            vect[i][2] = (currVertices[i][2] + t * (nextVertices[i][2] - currVertices[i][2])) * div;    
        }
    }
    else{
        for(int i = 0; i < pModel->numVertices ; i++){
            vect[i][0] = currVertices[i][0]  * div;
            vect[i][1] = currVertices[i][1]  * div;
            vect[i][2] = currVertices[i][2]  * div;    
        }    
    }
            
    ptricmds = pModel->pGlCommands;
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
