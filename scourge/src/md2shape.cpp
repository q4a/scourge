/**
 * Credit for this code is mainly due to:
 * DigiBen     digiben@gametutorials.com
 * Look up his other great tutorials at:
 * http://www.gametutorials.com
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
  // Go through all the objects in the scene
  for(int i = 0; i < g_3DModel.numOfObjects; i++) {
    // Free the faces, normals, vertices, and texture coordinates.
    delete [] g_3DModel.pObject[i].pFaces;
    delete [] g_3DModel.pObject[i].pNormals;
    delete [] g_3DModel.pObject[i].pVerts;
    delete [] g_3DModel.pObject[i].pTexVerts;
  } 
    
  for(int i = 0; i < g_3DModel.pObject[0].numOfVerts; i++){
    if(vect[i]) delete [] vect[i];  
  } 
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
  numAnimationWaiting = -1;
  animationPlayed = false;
  pauseAnimation = false;
          
  // First we need to actually load the .MD2 file.  We just pass in an address to
  // our t3DModel structure and the file name string we want to load ("tris.md2").
  // We also need to give the texture name we will be using.  This is because there
  // are usually a lot of textures with each character.  You choose the best one.
  // It seems that all of the Quake2 characters .md2 files are called: "tris.md2"

  g_LoadMd2.ImportMD2(&g_3DModel, fn, texfn);
  
  // There is no color information for these models, as well as only one
  // texture.  If everything goes well, it should load the TEXTURE_NAME file.

  // Go through all the materials
  for(int i = 0; i < g_3DModel.numOfMaterials; i++) {
    // Check to see if there is a file name to load in this material
    if(strlen(g_3DModel.pMaterials[i].strFile) > 0) {
      // Use the name of the texture file to load the bitmap, with a texture ID (i).
      // We pass in our global texture array, the name of the texture, and an ID to reference it.
      cout << "CreateTexture " << endl;
      CreateTexture((GLuint*)g_Texture, g_3DModel.pMaterials[i].strFile, i);
      cout << g_3DModel.pMaterials[i].strFile << endl;
    }
    // Set the texture ID for this material
    g_3DModel.pMaterials[i].texureId = i;
  }    

  // Allocate memory for interpolated vertices and initializes it
  vect = new (float *) [g_3DModel.pObject[0].numOfVerts];
  for(int i = 0; i < g_3DModel.pObject[0].numOfVerts; i++){
    vect[i] = new (float) [3];  
  }  
  for(int i = 0; i < g_3DModel.pObject[0].numOfVerts; i++){
    for(int j = 0; j < 3; j++){
        vect[i][j] = 0.0;
    }
  }  
    
  // Find the lowest point
  float minx, miny, minz;  
  float maxx, maxy, maxz;
  minx = miny = minz = maxx = maxy = maxz = 0;
  if(g_3DModel.pObject.size() > 0) {
    // Get the current object that we are displaying
    t3DObject *pObject = &g_3DModel.pObject[0];  
    for(int j = 0; j < pObject->numOfFaces; j++) {
      for(int whichVertex = 0; whichVertex < 3; whichVertex++) {
        int index = pObject->pFaces[j].vertIndex[whichVertex];
        if(pObject->pVerts[ index ].x > maxx) maxx = pObject->pVerts[ index ].x;
        if(pObject->pVerts[ index ].y > maxy) maxy = pObject->pVerts[ index ].y;
        if(pObject->pVerts[ index ].z > maxz) maxz = pObject->pVerts[ index ].z;
        if(pObject->pVerts[ index ].x < minx) minx = pObject->pVerts[ index ].x;
        if(pObject->pVerts[ index ].y < miny) miny = pObject->pVerts[ index ].y;
        if(pObject->pVerts[ index ].z < minz) minz = pObject->pVerts[ index ].z; 
                     
      }
    }
  }
  movex = maxx - minx;
  movey = maxy;
  movez = maxz - minz;    
}

 void MD2Shape::setCurrentAnimation(int numAnim){
    
    if(numAnim >= 0 && numAnim <= MD2_CREATURE_ACTION_COUNT){        
        //if(animationPlayed){
            if(numAnim != g_3DModel.currentAnim) {
                // Set the current animation to the new one wanted
                g_3DModel.currentAnim = numAnim;
    
                // Set the current frame to be the starting frame of the new animation
                g_3DModel.currentFrame = g_3DModel.pAnimations[g_3DModel.currentAnim].startFrame;
 
                animationPlayed = false;
                /*cout << g_3DModel.pMaterials[0].strFile << " animation : " << 
                g_3DModel.pAnimations[numAnim].strName << 
                " frames " << g_3DModel.pAnimations[numAnim].startFrame << " to " <<
                g_3DModel.pAnimations[numAnim].endFrame << endl; */
            }
            
        /*}
        else{ 
            // stores the new animation until the current one has been totally played       
            numAnimationWaiting = numAnim;
        }*/
        
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
 
  // This is where we call our animation function to draw and animate our character.
  // You can pass in any model into here and it will draw and animate it.  Of course,
  // it would be a good idea to stick this function in your model class.

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

///////////////////////////////// RETURN CURRENT TIME \\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\*
/////
/////   This returns time t for the interpolation between the current and next key frame
/////
///////////////////////////////// RETURN CURRENT TIME \\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\*

float MD2Shape::ReturnCurrentTime(t3DModel *pModel, int nextFrame)
{
    // This function is very similar to finding the frames per second.
    // Instead of checking when we reach a second, we check if we reach
    // 1 second / our animation speed. (1000 ms / kAnimationSpeed).
    // That's how we know when we need to switch to the next key frame.
    // In the process, we get the t value for how we are at to going to the
    // next animation key frame.  We use time to do the interpolation, that way
    // it runs the same speed on any persons computer, regardless of their specs.
    // It might look chopier on a junky computer, but the key frames still be
    // changing the same time as the other persons, it will just be not as smooth
    // of a transition between each frame.  The more frames per second we get, the
    // smoother the animation will be.

    // Get the current time in milliseconds
    float time = SDL_GetTicks();

    // Find the time that has elapsed since the last time that was stored
    elapsedTime = time - lastTime;

    // To find the current t we divide the elapsed time by the ratio of 1 second / our anim speed.
    // Since we aren't using 1 second as our t = 1, we need to divide the speed by 1000
    // milliseconds to get our new ratio, which is a 5th of a second.
    float t = elapsedTime / (1000.0f / ANIMATION_SPEED);
    
    // If our elapsed time goes over a 5th of a second, we start over and go to the next key frame
    if (elapsedTime >= (1000.0f / ANIMATION_SPEED) )
    {
        // Set our current frame to the next key frame (which could be the start of the anim)
        if(!pauseAnimation){
            pModel->currentFrame = nextFrame;
        }        

        // Set our last time to the current time just like we would when getting our FPS.
        lastTime = time;
    }

    // Return the time t so we can plug this into our interpolation.
    return t;
}


///////////////////////////////// ANIMATE MD2 MODEL \\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\*
/////
/////   This draws and animates the .md2 model by interpoloated key frame animation
/////
///////////////////////////////// ANIMATE MD2 MODEL \\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\*

void MD2Shape::AnimateMD2Model(t3DModel *pModel)
{

    int *ptricmds ;
    int nb;     
    
    // Now comes the juice of our tutorial.  Fear not, this is actually very intuitive
    // if you drool over it for a while (stay away from the keyboard though...).
    // What's going on here is, we are getting our current animation that we are
    // on, finding the current frame of that animation that we are on, then interpolating
    // between that frame and the next frame.  To make a smooth constant animation when
    // we get to the end frame, we interpolate between the last frame of the animation 
    // and the first frame of the animation.  That way, if we are doing the running 
    // animation let's say, when the last frame of the running animation is hit, we don't
    // have a huge jerk when going back to the first frame of that animation.  Remember,
    // because we have the texture and face information stored in the first frame of our
    // animation, we need to reference back to this frame every time when drawing the
    // model.  The only thing the other frames store is the vertices, but no information
    // about them.
    
    // Make sure we have valid objects just in case. (size() is in the vector class)
    if(pModel->pObject.size() <= 0) return; 
    
    // Here we grab the current animation that we are on from our model's animation list
    tAnimationInfo *pAnim = &(pModel->pAnimations[pModel->currentAnim]);

    // This gives us the current frame we are on.  We mod the current frame plus
    // 1 by the current animations end frame to make sure the next frame is valid.
    // If the next frame is past our end frame, then we go back to zero.  We check this next.    
    int nextFrame = (pModel->currentFrame + 1) % pAnim->endFrame;
    
    // If the next frame is zero, that means that we need to start the animation over.
    // To do this, we set nextFrame to the starting frame of this animation.
    // If a new animation is waiting to be played, play it.
    if(nextFrame == 0){        
        nextFrame =  pAnim->startFrame;

		if(g_3DModel.currentAnim == MD2_ATTACK) {
		  setCurrentAnimation(MD2_STAND);
		  setAttackEffect(false);
		}		

        /*animationPlayed = true;
        if(numAnimationWaiting == -1){
            nextFrame =  pAnim->startFrame;
        }    
        else{        
            setCurrentAnimation(numAnimationWaiting);
            numAnimationWaiting = -1; 
            pAnim = &(pModel->pAnimations[pModel->currentAnim]);
            nextFrame = (pModel->currentFrame + 1) % pAnim->endFrame;   
        }*/
    }

    // Get the current key frame we are on
    t3DObject *pFrame =      &pModel->pObject[pModel->currentFrame];
    
    // Get the next key frame we are interpolating too
    t3DObject *pNextFrame =  &pModel->pObject[nextFrame];

    // Get the first key frame so we have an address to the texture and face information
    t3DObject *pFirstFrame = &pModel->pObject[0];

    // Next, we want to get the current time that we are interpolating by.  Remember,
    // if t = 0 then we are at the beginning of the animation, where if t = 1 we are at the end.
    // Anyhing from 0 to 1 can be thought of as a percentage from 0 to 100 percent complete.    
    float t = ReturnCurrentTime(pModel, nextFrame);
        
    // Bind the appropriate texture
    if(getUseTexture()) glBindTexture(GL_TEXTURE_2D, g_Texture[0]);           
    
    // Stores interpolated vertices    
    for(int j = 0; j < pFrame->numOfFaces; j++)        
    {
        // Go through each corner of the triangle and draw it.
        for(int whichVertex = 0; whichVertex < 3; whichVertex++)
        {
            // Get the index for each point of the face
            int vertIndex = pFirstFrame->pFaces[j].vertIndex[whichVertex];                                              
                
            // Store the current and next frame's vertex
            CVector3 vPoint1 = pFrame->pVerts[ vertIndex ];                
            CVector3 vPoint2 = pNextFrame->pVerts[ vertIndex ]; 

            // By using the equation: p(t) = p0 + t(p1 - p0), with a time t
            // passed in, we create a new vertex that is closer to the next key frame.
            if(!pauseAnimation){
                vect[vertIndex][0] = (vPoint1.x + t * (vPoint2.x - vPoint1.x))*div;
                vect[vertIndex][1] = (vPoint1.y + t * (vPoint2.y - vPoint1.y))*div;
                vect[vertIndex][2] = (vPoint1.z + t * (vPoint2.z - vPoint1.z))*div;                
            }
            else{
                vect[vertIndex][0] = vPoint1.x  *div;
                vect[vertIndex][1] = vPoint1.y  *div;
                vect[vertIndex][2] = vPoint1.z  *div;                            
            }            
        }
    }
    
    ptricmds = pFirstFrame->pGlCommands;
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
