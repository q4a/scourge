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
 
//#define DEBUG_3DS 1

#define LIGHT_ANGLE 135.0f
  
#include "3dsshape.h"

C3DSShape::C3DSShape(char *file_name, float div, ShapePalette *shapePal, 
					 GLuint texture[],
					 char *name, int descriptionGroup,
					 Uint32 color, GLuint display_list, Uint8 shapePalIndex,
					 int offsetx, int offsety) :
	GLShape(0, 1, 1, 1, name, descriptionGroup, color, display_list, shapePalIndex) {
	commonInit(file_name, div, shapePal, offsetx, offsety);
  debugShape = new GLShape(0, this->width, this->depth, 1, name, descriptionGroup, color, display_list, shapePalIndex);
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
  fprintf(stderr, "Loading 3ds file: %s\n", path);
  g_Load3ds.Import3DS(&g_3DModel, path);         // Load our .3DS file into our model structure

  resolveTextures();

  normalizeModel();

  preRenderLight();

}

void C3DSShape::normalizeModel() {

  if (g_3DModel.pObject.size() <= 0) return;

  // Find the lowest point
  float minx, miny, minz;  
  float maxx, maxy, maxz;

  // Bad!
  minx = miny = minz = 100000; 
  maxx = maxy = maxz = -100000;

  for (int i = 0; i < g_3DModel.numOfObjects; i++) {
    for (int j = 0; j < g_3DModel.pObject[i].numOfFaces; j++) {
      for (int whichVertex = 0; whichVertex < 3; whichVertex++) {
        int index = g_3DModel.pObject[i].pFaces[j].vertIndex[whichVertex];

        if (g_3DModel.pObject[i].pVerts[ index ].x < minx) minx = g_3DModel.pObject[i].pVerts[ index ].x;
        if (g_3DModel.pObject[i].pVerts[ index ].y < miny) miny = g_3DModel.pObject[i].pVerts[ index ].y;
        if (g_3DModel.pObject[i].pVerts[ index ].z < minz) minz = g_3DModel.pObject[i].pVerts[ index ].z;

        if (g_3DModel.pObject[i].pVerts[ index ].x >= maxx) maxx = g_3DModel.pObject[i].pVerts[ index ].x;
        if (g_3DModel.pObject[i].pVerts[ index ].y >= maxy) maxy = g_3DModel.pObject[i].pVerts[ index ].y;
        if (g_3DModel.pObject[i].pVerts[ index ].z >= maxz) maxz = g_3DModel.pObject[i].pVerts[ index ].z;
        
      }
    }
  }
  cerr << "min=(" << minx << "," << miny << "," << minz << ") max=(" << maxx << "," << maxy << "," << maxz << ")" << endl;

  // normalize vertecies
  char tmp[80];
  map<string, int> seenIndexes;
  for (int i = 0; i < g_3DModel.numOfObjects; i++) {
    for (int j = 0; j < g_3DModel.pObject[i].numOfFaces; j++) {
      for (int whichVertex = 0; whichVertex < 3; whichVertex++) {
        int index = g_3DModel.pObject[i].pFaces[j].vertIndex[whichVertex];
        sprintf(tmp, "%d,%d", i, index);
        string key = tmp;
        if(seenIndexes.find(key) == seenIndexes.end()) {
          g_3DModel.pObject[i].pVerts[ index ].x -= minx;
          g_3DModel.pObject[i].pVerts[ index ].y -= miny;
          g_3DModel.pObject[i].pVerts[ index ].z -= minz;
          seenIndexes[key] = 1;
        }
      }
    }
  }
  maxx -= minx;
  maxy -= miny;
  maxz -= minz;
  
  movex = 0;
  movey = maxy;
  float n = 0.25 / DIV;
  movez = n;

  // calculate dimensions
  float fw = maxx * div * DIV;
  float fd = maxy * div * DIV;
  float fh = maxz * div * DIV;

  // set the shape's dimensions
  this->width = (int)(fw + 0.5f);
  if(this->width < 1) this->width = 1;
  this->depth = (int)(fd + 0.5f);
  if(this->depth < 1) this->depth = 1;
  this->height = (int)(fh + 0.5f);
  if(this->height < 1) this->height = 1;

  cerr << this->getName() << " width=" << width << " depth=" << depth << " height=" << height << endl;
}

void C3DSShape::resolveTextures() {
  // Depending on how many textures we found, load each one (Assuming .BMP)
  // If you want to load other files than bitmaps, you will need to adjust CreateTexture().
  // Below, we go through all of the materials and check if they have a texture map to load.
  // Otherwise, the material just holds the color information and we don't need to load a texture.

  // Go through all the materials
  for (int i = 0; i < g_3DModel.numOfMaterials; i++) {
    // Check to see if there is a file name to load in this material
    if (strlen(g_3DModel.pMaterials[i].strFile) > 0) {


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
}

void C3DSShape::preRenderLight() {
  // pre-render the light
  for (int i = 0; i < g_3DModel.numOfObjects; i++) {
    if (g_3DModel.pObject.size() <= 0) break;
    t3DObject *pObject = &g_3DModel.pObject[i]; 
    for (int j = 0; j < pObject->numOfFaces; j++) {
      for (int whichVertex = 0; whichVertex < 3; whichVertex++) {
        int index = pObject->pFaces[j].vertIndex[whichVertex];

        // Simple light rendering:
        // need the normal as mapped on the xy plane
        // it's degree is the intensity of light it gets
        float x = (pObject->pNormals[ index ].x == 0 ? 0.01f : pObject->pNormals[ index ].x);
        float y = pObject->pNormals[ index ].y;
        float rad = atan(y / x);
        float angle = (180.0f * rad) / 3.14159;

        // read about the arctan problem: 
        // http://hyperphysics.phy-astr.gsu.edu/hbase/ttrig.html#c3
        int q = 1;
        if (x < 0) {     // Quadrant 2 & 3
          q = ( y >= 0 ? 2 : 3);
          angle += 180;
        } else if (y < 0) { // Quadrant 4
          q = 4;
          angle += 360;
        }

        // assertion
        if (angle < 0 || angle > 360) {
          cerr << "Warning: angle=" << angle << " quadrant=" << q << endl;
        }

        // these are not used
        //if(angle < 0) angle += 360.0f;
        //if(angle > 360) angle -= 360.0f;

        // calculate the angle distance from the light
        float delta = 0;
        if (angle > LIGHT_ANGLE && angle < LIGHT_ANGLE + 180.0f) {
          delta = angle - LIGHT_ANGLE;
        } else {
          if (angle < LIGHT_ANGLE) angle += 360.0f;
          delta = (360 + LIGHT_ANGLE) - angle;
        }

        // assertion
        if (delta < 0 || delta > 180.0f) {
          cerr << "WARNING: angle=" << angle << " delta=" << delta << endl;
        }

        // reverse and convert to value between 0 and 1
        delta = 1.0f - (delta / 180.0f);

        // store the value
        pObject->shadingColorDelta[ index ] = delta;
      }
    }
  }
}

void C3DSShape::draw() {

#ifdef DEBUG_3DS
  if ( glIsEnabled( GL_TEXTURE_2D ) ) {
    glPushMatrix();
    debugShape->draw();
    glPopMatrix();
  }
#endif

  GLfloat currentColor[4];
  glGetFloatv( GL_CURRENT_COLOR, currentColor );

  glPushMatrix();

  glDisable( GL_CULL_FACE );
  //  glEnable( GL_CULL_FACE );
  // glCullFace( GL_FRONT );

  //  glScalef( div, div, div );
  //  glTranslatef( -movex, -movey, -movez );
  glTranslatef(-movex * div, 0.0f, 0.0f);
  glTranslatef(0.0f, (getDepth() / DIV) - (movey * div), 0.0f);
//  glTranslatef(0.0f, 0.0f, -movez / 2.0f);
  glTranslatef(0.0f, 0.0f, movez);

  /*
  glTranslatef((float)offsetx / DIV, (float)offsety / DIV, 0.0f);
  */

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
  for (int i = 0; i < g_3DModel.numOfObjects; i++) {
    // Make sure we have valid objects just in case. (size() is in the vector class)
    if (g_3DModel.pObject.size() <= 0) break;

    // Get the current object that we are displaying
    t3DObject *pObject = &g_3DModel.pObject[i];

    // Check to see if this object has a texture map, if so bind the texture to it.
    if (pObject->bHasTexture && !useShadow) {
      // Bind the texture map to the object by it's materialID
      glEnable(GL_TEXTURE_2D);
      glBindTexture(GL_TEXTURE_2D, g_Texture[pObject->materialID]);
      //if (!useShadow) glColor3ub(255, 255, 255);
    } else {
      // Turn off texture mapping and turn on color
      glDisable(GL_TEXTURE_2D);   
      // Reset the color to normal again
      //if (!useShadow) glColor3ub(255, 255, 255);
    } 

    float c[3];

    // This determines if we are in wireframe or normal mode
    glBegin(g_ViewMode);                    // Begin drawing with our selected mode (triangles or lines)

    // Go through all of the faces (polygons) of the object and draw them
    for (int j = 0; j < pObject->numOfFaces; j++) {
      // Go through each corner of the triangle and draw it.
      for (int whichVertex = 0; whichVertex < 3; whichVertex++) {
        // Get the index for each point of the face
        int index = pObject->pFaces[j].vertIndex[whichVertex];

        // Give OpenGL the normal for this vertex.
        glNormal3f(pObject->pNormals[ index ].x, pObject->pNormals[ index ].y, pObject->pNormals[ index ].z);

        // If the object has a texture associated with it, give it a texture coordinate.
        if (!useShadow) {
          if (pObject->bHasTexture) {

            // texture color is white
            c[0] = c[1] = c[2] = 1.0f;

            // Make sure there was a UVW map applied to the object or else it won't have tex coords.
            if (pObject->pTexVerts) {
              glTexCoord2f(pObject->pTexVerts[ index ].x, pObject->pTexVerts[ index ].y);
            }
          } else {

            // Make sure there is a valid material/color assigned to this object.
            // You should always at least assign a material color to an object, 
            // but just in case we want to check the size of the material list.
            // if the size is at least one, and the material ID != -1,
            // then we have a valid material.
            if (g_3DModel.pMaterials.size() && pObject->materialID >= 0) {
              // Get and set the color that the object is, since it must not have a texture
              BYTE *pColor = g_3DModel.pMaterials[pObject->materialID].color;

              // Assign the current color to this model
              //glColor3ub(pColor[0], pColor[1], pColor[2]);
              c[0] = (float)pColor[0] / 255.0f;
              c[1] = (float)pColor[1] / 255.0f;
              c[2] = (float)pColor[2] / 255.0f;
            }
          }

          // apply the current color
          c[0] *= currentColor[0];
          c[1] *= currentColor[1];
          c[2] *= currentColor[2];

          // apply the precomputed shading to the current color
          c[0] *= pObject->shadingColorDelta[ index ];
          c[1] *= pObject->shadingColorDelta[ index ];
          c[2] *= pObject->shadingColorDelta[ index ];
          glColor3f(c[0], c[1], c[2]);
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
  if (!useShadow) glEnable(GL_TEXTURE_2D);
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
