/***************************************************************************
                          3dsshape.cpp  -  description
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
 
//#define DEBUG_3DS 
  
#include "3dsshape.h"

C3DSShape::C3DSShape(char *file_name, float div, ShapePalette *shapePal, 
					 GLuint texture[],
					 int width, int depth, int height,
					 char *name,
					 Uint32 color, GLuint display_list, Uint8 shapePalIndex, 
					 int offsetx, int offsety) :
  // passing 0 for texture causes glshape to not init
#ifdef DEBUG_3DS
  GLShape(texture, width, depth, height, name, color, display_list, shapePalIndex) {
#else
	GLShape(0, width, depth, height, name, color, display_list, shapePalIndex) {
#endif
	commonInit(file_name, div, shapePal, offsetx, offsety);    
}

C3DSShape::C3DSShape(char *file_name, float div, ShapePalette *shapePal, 
					 GLuint texture[],
					 int width, int depth, int height,
					 char *name, char **description, int descriptionCount,
					 Uint32 color, GLuint display_list, Uint8 shapePalIndex,
					 int offsetx, int offsety) :
  // passing 0 for texture causes glshape to not init
#ifdef DEBUG_3DS
  GLShape(texture, width, depth, height, name, description, descriptionCount, color, display_list, shapePalIndex) {
#else
	GLShape(0, width, depth, height, name, description, descriptionCount, color, display_list, shapePalIndex) {
#endif
	commonInit(file_name, div, shapePal, offsetx, offsety);    
}

C3DSShape::~C3DSShape() {
  // When we are done, we need to free all the model data
  // We do this by walking through all the objects and freeing their information

  // Go through all the objects in the scene
  for(int i = 0; i < g_3DModel.numOfObjects; i++) {
	// Free the faces, normals, vertices, and texture coordinates.
	delete [] g_3DModel.pObject[i].pFaces;
	delete [] g_3DModel.pObject[i].pNormals;
	delete [] g_3DModel.pObject[i].pVerts;
	delete [] g_3DModel.pObject[i].pTexVerts;
  }
 }

 void C3DSShape::commonInit(char *file_name, float div, ShapePalette *shapePal, int offsetx, int offsety) {
   fprintf(stderr, "%s\n", file_name);
   g_Texture[0] = 0;
  g_ViewMode = GL_TRIANGLES;
  this->div = div;
  this->shapePal = shapePal;
  this->offsetx = offsetx;
  this->offsety = offsety;

  // First we need to actually load the .3DS file.  We just pass in an address to
  // our t3DModel structure and the file name string we want to load ("face.3ds").
  char path[300];
  strcpy(path, rootDir);
  strcat(path, file_name);
  g_Load3ds.Import3DS(&g_3DModel, path);         // Load our .3DS file into our model structure

  // Depending on how many textures we found, load each one (Assuming .BMP)
  // If you want to load other files than bitmaps, you will need to adjust CreateTexture().
  // Below, we go through all of the materials and check if they have a texture map to load.
  // Otherwise, the material just holds the color information and we don't need to load a texture.

  // Go through all the materials
  for(int i = 0; i < g_3DModel.numOfMaterials; i++) {
	// Check to see if there is a file name to load in this material
	if(strlen(g_3DModel.pMaterials[i].strFile) > 0) {


	  // Use the name of the texture file to load the bitmap, with a texture ID (i).
	  // We pass in our global texture array, the name of the texture, and an ID to reference it. 
	  fprintf(stderr, "Loading texture: %s\n", g_3DModel.pMaterials[i].strFile);
	  //CreateTexture((GLuint*)g_Texture, g_3DModel.pMaterials[i].strFile, i);           


	  // instead of loading the texture, get one of the already loaded textures
	  g_Texture[i] = shapePal->findTextureByName(g_3DModel.pMaterials[i].strFile);
	  fprintf(stderr, "\t%s\n", (g_Texture[i] ? "FOUND IT" : "NOT FOUND"));
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
  fprintf(stderr, "x=(%f,%f) y=(%f,%f) z=(%f,%f)\n",
		  minx, maxx, miny, maxy, minz, maxz);
  movex = minx;
  movey = maxy;
  movez = minz;
}

void C3DSShape::draw() {

#ifdef DEBUG_3DS
  GLShape::draw();
#endif

  glPushMatrix();
  glDisable( GL_CULL_FACE );
  //  glScalef( div, div, div );
  //  glTranslatef( -movex, -movey, -movez );
  glTranslatef(-movex * div, 0.0f, 0.0f);
  glTranslatef(0.0f, (getDepth() / DIV) - (movey * div), 0.0f);
//  glTranslatef(0.0f, 0.0f, -movez / 2.0f);
  glTranslatef(0.0f, 0.0f, -movez);
  glTranslatef((float)offsetx / DIV, (float)offsety / DIV, 0.0f);

  // I am going to attempt to explain what is going on below up here as not to clutter the 
  // code below.  We have a model that has a certain amount of objects and textures.  We want 
  // to go through each object in the model, bind it's texture map to it, then render it.
  // To render the current object, we go through all of it's faces (Polygons).  
  // What is a face you ask?  A face is just (in this case) a triangle of the object.
  // For instance, a cube has 12 faces because each side has 2 triangles.
  // You might be thinking.  Well, if there are 12 faces in a cube, that makes
  // 36 vertices that we needed to read in for that object.  Not really true.  Because
  // a lot of the vertices are the same, since they share sides, they only need to save
  // 8 vertices, and ignore the duplicates.  Then, you have an array of all the
  // unique vertices in that object.  No 2 vertices will be the same.  This cuts down
  // on memory.  Then, another array is saved, which is the index numbers for each face,
  // which index in to that array of vertices.  That might sound silly, but it is better
  // than saving tons of duplicate vertices.  The same thing happens for UV coordinates.
  // You don't save duplicate UV coordinates, you just save the unique ones, then an array
  // that index's into them.  This might be confusing, but most 3D files use this format.
  // This loop below will stay the same for most file formats that you load, so all you need
  // to change is the loading code.  You don't need to change this loop (Except for animation).
  
  // Since we know how many objects our model has, go through each of them.
  for(int i = 0; i < g_3DModel.numOfObjects; i++) {
	// Make sure we have valid objects just in case. (size() is in the vector class)
	if(g_3DModel.pObject.size() <= 0) break;

	// Get the current object that we are displaying
	t3DObject *pObject = &g_3DModel.pObject[i];
	
	// Check to see if this object has a texture map, if so bind the texture to it.
    if(pObject->bHasTexture) {
		// Bind the texture map to the object by it's materialID
		glEnable(GL_TEXTURE_2D);
		glBindTexture(GL_TEXTURE_2D, g_Texture[pObject->materialID]);
	    if(!useShadow) glColor3ub(255, 255, 255);		
	  } else {
		// Turn off texture mapping and turn on color
		glDisable(GL_TEXTURE_2D);		
		// Reset the color to normal again
		if(!useShadow) glColor3ub(255, 255, 255);
	  }	
	
	// This determines if we are in wireframe or normal mode
	glBegin(g_ViewMode);                    // Begin drawing with our selected mode (triangles or lines)
	
	// Go through all of the faces (polygons) of the object and draw them
	for(int j = 0; j < pObject->numOfFaces; j++) {
	  // Go through each corner of the triangle and draw it.
	  for(int whichVertex = 0; whichVertex < 3; whichVertex++) {
		// Get the index for each point of the face
		int index = pObject->pFaces[j].vertIndex[whichVertex];
		
		// Give OpenGL the normal for this vertex.
		glNormal3f(pObject->pNormals[ index ].x, pObject->pNormals[ index ].y, pObject->pNormals[ index ].z);



		/*
		  TODO:
		  Figure out how far the normal is from the shadow angle and apply multitexturing to this 
		  triangle accordingly.
		 */


		
		// If the object has a texture associated with it, give it a texture coordinate.
		if(!useShadow) {
		  if(pObject->bHasTexture) {
			
			// Make sure there was a UVW map applied to the object or else it won't have tex coords.
			if(pObject->pTexVerts) {
			  glTexCoord2f(pObject->pTexVerts[ index ].x, pObject->pTexVerts[ index ].y);
			}
		  } else {
			
			// Make sure there is a valid material/color assigned to this object.
			// You should always at least assign a material color to an object, 
			// but just in case we want to check the size of the material list.
			// if the size is at least one, and the material ID != -1,
			// then we have a valid material.
			if(g_3DModel.pMaterials.size() && pObject->materialID >= 0) {
			  // Get and set the color that the object is, since it must not have a texture
			  BYTE *pColor = g_3DModel.pMaterials[pObject->materialID].color;
			  
			  // Assign the current color to this model
			  glColor3ub(pColor[0], pColor[1], pColor[2]);
			}
		  }
		}
		
		// Pass in the current vertex of the object (Corner of current face)
		glVertex3f(pObject->pVerts[ index ].x * div, 
				   pObject->pVerts[ index ].y * div, 
				   pObject->pVerts[ index ].z * div);
	  }
	}
	
	glEnd();                                // End the drawing
  }
  glPopMatrix();
  if(!useShadow) glEnable(GL_TEXTURE_2D);
  glEnable( GL_CULL_FACE );
}

void C3DSShape::setupBlending() { 
  glBlendFunc(GL_ONE, GL_ONE); 
}

void C3DSShape::endBlending() { 
}

bool C3DSShape::drawFirst() { 
  return true; 
}

bool C3DSShape::drawLater() { 
  return false; 
}
