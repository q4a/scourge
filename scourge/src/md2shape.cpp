/**
 * Credit for this code is mainly due to:
 * DigiBen     digiben@gametutorials.com
 * Look up his other great tutorials at:
 * http://www.gametutorials.com
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
}

void MD2Shape::commonInit(char *file_name, char *texture_name, float div) {
  g_Texture[0] = 0;
  g_ViewMode = GL_TRIANGLES;
  this->div = div;

  char fn[300], texfn[300];
  strcpy(fn, rootDir);
  strcat(fn, file_name);

  strcpy(texfn, rootDir);
  strcat(texfn, texture_name);
          
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

void MD2Shape::draw() {

#ifdef DEBUG_MD2
  // draw the outline for debugging
  GLShape::draw();
#endif  

  glPushMatrix();

  // To make our model render somewhat faster, we do some front back culling.
  // It seems that Quake2 orders their polygons clock-wise.  
  glEnable( GL_CULL_FACE );
  glCullFace( GL_FRONT );
  
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
  

  // Make sure we have valid objects just in case. (size() is in the vector class)  
  if(g_3DModel.pObject.size() > 0) {
    // Get the current object that we are displaying
    t3DObject *pObject = &g_3DModel.pObject[0];

    // Bind the texture
    if(getUseTexture()) glBindTexture(GL_TEXTURE_2D, g_Texture[0]);
    // set the color so it's not random...
    //glColor4f(1.0f, 1.0f, 1.0f, 1.0f);

    // Render lines or normal triangles mode, depending on the global variable
    glBegin(g_ViewMode);    

      // Go through all of the faces (polygons) of the object and draw them
      for(int j = 0; j < pObject->numOfFaces; j++) {
        // Go through each corner of the triangle and draw it.
        for(int whichVertex = 0; whichVertex < 3; whichVertex++) {
          // Get the index for each point in the face
          int index = pObject->pFaces[j].vertIndex[whichVertex];

          // Get the index for each texture coord in the face
          int index2 = pObject->pFaces[j].coordIndex[whichVertex];

          // Give OpenGL the normal for this vertex.  Notice that we put a
          // - sign in front.  It appears that because of the ordering of Quake2's
          // polygons, we need to invert the normal
          glNormal3f(-pObject->pNormals[ index ].x, -pObject->pNormals[ index ].y, pObject->pNormals[ index ].z);          

          // Make sure there was a UVW map applied to the object or else it won't have tex coords.
          if(getUseTexture() && pObject->pTexVerts) {
              glTexCoord2f(pObject->pTexVerts[ index2 ].x, pObject->pTexVerts[ index2 ].y);             
          }          

          // Pass in the current vertex of the object (Corner of current face)
          glVertex3f(pObject->pVerts[ index ].x * div, pObject->pVerts[ index ].y * div, -pObject->pVerts[ index ].z * div);
        }
      }
    glEnd();  
  }
  glDisable(GL_CULL_FACE);
  glPopMatrix();    
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
