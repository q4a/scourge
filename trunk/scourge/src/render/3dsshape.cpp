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
#include "shapes.h"
#include "location.h"

using namespace std;

C3DSShape::C3DSShape(const string& file_name, float div, Shapes *shapePal, 
										 GLuint texture[],
										 char *name, int descriptionGroup,
										 Uint32 color, Uint8 shapePalIndex,
										 float size_x, float size_y, float size_z,
										 float offs_x, float offs_y, float offs_z,
										 float xrot3d, float yrot3d, float zrot3d ) :
	GLShape(0, 1, 1, 1, name, descriptionGroup, color, shapePalIndex) {
	commonInit(file_name, div, shapePal, size_x, size_y, size_z, offs_x, offs_y, offs_z, xrot3d, yrot3d, zrot3d );
#ifdef DEBUG_3DS
  debugShape = new GLShape(0, this->width, this->depth, 1, name, descriptionGroup, color, shapePalIndex);
  debugShape->initialize();
#endif
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

  glDeleteLists( displayListStart, 2 );
}

void C3DSShape::commonInit(const string& file_name, float div, Shapes *shapePal, float size_x, float size_y, float size_z, float offs_x, float offs_y, float offs_z, float xrot3d, float yrot3d, float zrot3d ) {

  g_Texture[0] = 0;
  g_ViewMode = GL_TRIANGLES;
  this->divx = this->divy = this->divz = div;
  this->shapePal = shapePal;
  this->size_x = size_x;
  this->size_y = size_y;
  this->size_z = size_z;  
  this->offs_x = offs_x; 
  this->offs_y = offs_y;
  this->offs_z = offs_z;
	this->xrot3d = xrot3d;
	this->yrot3d = yrot3d;
	this->zrot3d = zrot3d;

  // First we need to actually load the .3DS file.  We just pass in an address to
  // our t3DModel structure and the file name string we want to load ("face.3ds").
  string path = rootDir + file_name;
//  fprintf(stderr, "Loading 3ds file: %s\n", path);
	if( !shapePal->isHeadless() ) {
	  g_Load3ds.Import3DS(&g_3DModel, path);         // Load our .3DS file into our model structure

	  resolveTextures();

	  normalizeModel();

	  preRenderLight();
	}
}

void C3DSShape::normalizeModel() {

	if ( g_3DModel.pObject.empty() ) return;

	// "rotate" the model
	float n;
	map<string, int> seenIndexes;
	if( getZRot3d() != 0 || getXRot3d() != 0 || getYRot3d() != 0 ) {
		for (int i = 0; i < g_3DModel.numOfObjects; i++) {
			for (int j = 0; j < g_3DModel.pObject[i].numOfFaces; j++) {
				for (int whichVertex = 0; whichVertex < 3; whichVertex++) {
					int index = g_3DModel.pObject[i].pFaces[j].vertIndex[whichVertex];
					char tmp[80];
					snprintf(tmp, 80, "%d,%d", i, index);
					string key = tmp;
					if(seenIndexes.find(key) == seenIndexes.end()) {
						if( getZRot3d() != 0 ) {
							n = g_3DModel.pObject[i].pVerts[ index ].x;
							g_3DModel.pObject[i].pVerts[ index ].x = g_3DModel.pObject[i].pVerts[ index ].y;
							g_3DModel.pObject[i].pVerts[ index ].y = n;

							n = g_3DModel.pObject[i].pNormals[ index ].x;
							g_3DModel.pObject[i].pNormals[ index ].x = g_3DModel.pObject[i].pNormals[ index ].y;
							g_3DModel.pObject[i].pNormals[ index ].y = n;
						} else if( getYRot3d() != 0 ) {
							n = g_3DModel.pObject[i].pVerts[ index ].x;
							g_3DModel.pObject[i].pVerts[ index ].x = g_3DModel.pObject[i].pVerts[ index ].z;
							g_3DModel.pObject[i].pVerts[ index ].z = n;

							n = g_3DModel.pObject[i].pNormals[ index ].x;
							g_3DModel.pObject[i].pNormals[ index ].x = g_3DModel.pObject[i].pNormals[ index ].z;
							g_3DModel.pObject[i].pNormals[ index ].z = n;
						} else if( getXRot3d() != 0 ) {
							n = g_3DModel.pObject[i].pVerts[ index ].y;
							g_3DModel.pObject[i].pVerts[ index ].y = g_3DModel.pObject[i].pVerts[ index ].z;
							g_3DModel.pObject[i].pVerts[ index ].z = n;

							n = g_3DModel.pObject[i].pNormals[ index ].y;
							g_3DModel.pObject[i].pNormals[ index ].y = g_3DModel.pObject[i].pNormals[ index ].z;
							g_3DModel.pObject[i].pNormals[ index ].z = n;
						}
						seenIndexes[key] = 1;
					}
				}
			}
		}
	}

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
  //cerr << "min=(" << minx << "," << miny << "," << minz << ") max=(" << maxx << "," << maxy << "," << maxz << ")" << endl;

  // normalize vertecies
  seenIndexes.clear();
  for (int i = 0; i < g_3DModel.numOfObjects; i++) {
    for (int j = 0; j < g_3DModel.pObject[i].numOfFaces; j++) {
      for (int whichVertex = 0; whichVertex < 3; whichVertex++) {
        int index = g_3DModel.pObject[i].pFaces[j].vertIndex[whichVertex];
        char tmp[80];
		snprintf(tmp, 80, "%d,%d", i, index);
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
  n = 0.25f / DIV;
  movez = n;

	if( divx > 0 ) {
	  // calculate dimensions where 'div' is given
	  float fw = maxx * divx * DIV;
	  float fd = maxy * divy * DIV;
	  float fh = maxz * divz * DIV;

	  // set the shape's dimensions
	  this->width = (int)(fw + 0.5f);
	  if(this->width < 1) this->width = 1;
	  this->depth = (int)(fd + 0.5f);
	  if(this->depth < 1) this->depth = 1;
	  this->height = (int)(fh + 0.5f);
	  if(this->height < 1) this->height = 1;

		cerr << this->getName() << " size=" << fw << "," << fd << "," << fh << endl;
	} else {
		// calculate 'div' where dimensions are given
	  this->width = toint( size_x );
	  if(this->width < 1) this->width = 1;
	  this->depth = toint( size_y );
	  if(this->depth < 1) this->depth = 1;
	  this->height = toint( size_z );
	  if(this->height < 1) this->height = 1;		
	
		divx = size_x / ( maxx * DIV );
		divy = size_y / ( maxy * DIV );
		divz = size_z / ( maxz * DIV );

		/*
		cerr << "div=(" << divx << "," << divy << "," << divz << ") " <<
			"max=(" << maxx << "," << maxy << "," << maxz << ") " <<
			"size=(" << size_x << "," << size_y << "," << size_z << ") " << endl;
		*/
	}  
}

void C3DSShape::resolveTextures() {
  // Depending on how many textures we found, load each one (Assuming .BMP)
  // If you want to load other files than bitmaps, you will need to adjust CreateTexture().
  // Below, we go through all of the materials and check if they have a texture map to load.
  // Otherwise, the material just holds the color information and we don't need to load a texture.

  // Go through all the materials
  //cerr << "3ds model: " << this->getName() << endl;
  for (int i = 0; i < g_3DModel.numOfMaterials; i++) {
    // Check to see if there is a file name to load in this material
    if (g_3DModel.pMaterials[i].strFile.length() > 0) {


      // Use the name of the texture file to load the bitmap, with a texture ID (i).
      // We pass in our global texture array, the name of the texture, and an ID to reference it. 
//      fprintf(stderr, "Loading texture: %s\n", g_3DModel.pMaterials[i].strFile);
      //CreateTexture((GLuint*)g_Texture, g_3DModel.pMaterials[i].strFile, i);           


      // instead of loading the texture, get one of the already loaded textures
      g_Texture[i] = shapePal->findTextureByName( g_3DModel.pMaterials[i].strFile, true );
			if( !g_Texture[i] ) cerr << "*** error: can't find 3ds texture reference: " << g_3DModel.pMaterials[i].strFile << endl;
      //cerr << "\tTexture: " << g_3DModel.pMaterials[i].strFile << " found? " << g_Texture[i] << endl;
    }
    // Set the texture ID for this material
    g_3DModel.pMaterials[i].texureId = i;
  }
}

void C3DSShape::preRenderLight() {
  // pre-render the light
  for (int i = 0; i < g_3DModel.numOfObjects; i++) {
    if ( g_3DModel.pObject.empty() ) break;
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
#ifdef DEBUG_3DS
          cerr << "Warning: object: " << getName() << " angle=" << angle << " quadrant=" << q << endl;
#endif
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
#ifdef DEBUG_3DS
          cerr << "WARNING: object: " << getName() << " angle=" << angle << " delta=" << delta << endl;
#endif
        }

        // reverse and convert to value between 0 and 1
        delta = 1.0f - (delta / 180.0f);

        // store the value
        pObject->shadingColorDelta[ index ] = delta;
      }
    }
  }
}

void C3DSShape::initialize() {
  displayListStart = glGenLists( 2 );
  if( !displayListStart ) {
    cerr << "*** Error: couldn't generate display lists for shape: " << getName() << endl;
    exit(1);
  }

  createDisplayList( displayListStart, false );
  createDisplayList( displayListStart + 1, true );

  initialized = true;
}

void C3DSShape::createDisplayList( GLuint listName, bool isShadow ) {
  glNewList( listName, GL_COMPILE );
	drawShape( isShadow );
  glEndList();
}

void C3DSShape::drawShape( bool isShadow ) {
  // Since we know how many objects our model has, go through each of them.
  for (int i = 0; i < g_3DModel.numOfObjects; i++) {
    // Make sure we have valid objects just in case. (size() is in the vector class)
    if ( g_3DModel.pObject.empty() ) break;

    // Get the current object that we are displaying
    t3DObject *pObject = &g_3DModel.pObject[i];

    // Check to see if this object has a texture map, if so bind the texture to it.
    if (pObject->bHasTexture && !isShadow) {
      // Bind the texture map to the object by it's materialID
      glEnable(GL_TEXTURE_2D);
      glBindTexture(GL_TEXTURE_2D, g_Texture[pObject->materialID]);
      //if (!isShadow) glColor3ub(255, 255, 255);
    } else {
      // Turn off texture mapping and turn on color
      glDisable(GL_TEXTURE_2D);   
      // Reset the color to normal again
      //if (!isShadow) glColor3ub(255, 255, 255);
    } 

    float c[3] = { 0.0f, 0.0f, 0.0f };

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
        if (!isShadow) {
          if (pObject->bHasTexture) {

            // texture color is white
            c[0] = c[1] = c[2] = 1.0f;

            // Make sure there was a UVW map applied to the object or else it won't have tex coords.
            if (pObject->pTexVerts) {
              glTexCoord2f(pObject->pTexVerts[ index ].x, -pObject->pTexVerts[ index ].y);
            }
          } //else {

            // Make sure there is a valid material/color assigned to this object.
            // You should always at least assign a material color to an object, 
            // but just in case we want to check the size of the material list.
            // if the size is at least one, and the material ID != -1,
            // then we have a valid material.
            if ( !g_3DModel.pMaterials.empty() && pObject->materialID >= 0) {
              // Get and set the color that the object is, since it must not have a texture
              BYTE *pColor = g_3DModel.pMaterials[pObject->materialID].color;

              // Assign the current color to this model
              //glColor3ub(pColor[0], pColor[1], pColor[2]);
              c[0] = (float)pColor[0] / 255.0f;
              c[1] = (float)pColor[1] / 255.0f;
              c[2] = (float)pColor[2] / 255.0f;
            }
          //}

          // FIXME: will need to figure out some way to do this
          /*
          // apply the current color
          c[0] *= currentColor[0];
          c[1] *= currentColor[1];
          c[2] *= currentColor[2];
          */

          // apply the precomputed shading to the current color
          c[0] *= pObject->shadingColorDelta[ index ];
          c[1] *= pObject->shadingColorDelta[ index ];
          c[2] *= pObject->shadingColorDelta[ index ];
          glColor3f(c[0], c[1], c[2]);
        }

        // Pass in the current vertex of the object (Corner of current face)
				if( isWind() ) {
					//float n = sin( windAngle ) * 0.5f * ( ( pObject->pVerts[ index ].z * divz ) / (float)getHeight() );
					float nx = windInfo.getValue() * ( ( pObject->pVerts[ index ].z * divz ) / (float)getHeight() );
					//float ny = getWindInfo()->getValue() * getWindInfo()->getYMod() * ( ( pObject->pVerts[ index ].z * divz ) / (float)getHeight() );
          float ny = 0;
					glVertex3f(pObject->pVerts[ index ].x * divx + nx, 
										 pObject->pVerts[ index ].y * divy + ny, 
										 pObject->pVerts[ index ].z * divz);
				} else {
					glVertex3f(pObject->pVerts[ index ].x * divx, 
										 pObject->pVerts[ index ].y * divy, 
										 pObject->pVerts[ index ].z * divz);
				}
			}
		}

    glEnd();                                // End the drawing
  }
}

void C3DSShape::draw() {
  if(!initialized) {
    cerr << "*** Warning: shape not intialized. name=" << getName() << endl;
  }

  bool textureEnabled = glIsEnabled( GL_TEXTURE_2D );

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
	glEnable(GL_ALPHA_TEST);
	glAlphaFunc( GL_NOTEQUAL, 0 );
  glTranslatef( offs_x / DIV, offs_y / DIV, offs_z / DIV );
  glTranslatef(-movex * divx, 0.0f, 0.0f);
  glTranslatef(0.0f, (getDepth() / DIV) - (movey * divy), 0.0f);
  glTranslatef(0.0f, 0.0f, movez);

	// update the wind
	if( isWind() && !useShadow ) {
		if( windInfo.update() ) createDisplayList( displayListStart, false );
	}

	glCallList( displayListStart + (useShadow ? 1 : 0) );

	glDisable(GL_ALPHA_TEST);
  glPopMatrix();

  //  if( !useShadow ) 
  //	glEnable(GL_TEXTURE_2D);

  if( textureEnabled ) glEnable( GL_TEXTURE_2D );
  else glDisable( GL_TEXTURE_2D );
}

void C3DSShape::outline( float r, float g, float b ) {
  useShadow = true;
  GLboolean blend;
  glGetBooleanv( GL_BLEND, &blend );
  glEnable( GL_BLEND );
  glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );
	GLboolean texture = glIsEnabled( GL_TEXTURE_2D );
  glDisable( GL_TEXTURE_2D );
  glPolygonMode( GL_FRONT, GL_LINE );
  glLineWidth( 4 );
  glEnable( GL_CULL_FACE );
  glCullFace( GL_BACK );
  //glEnable( GL_DEPTH_TEST );
  //GLint df;
  //glGetIntegerv( GL_DEPTH_FUNC, &df );
  //glDepthFunc( GL_GEQUAL );
  glColor3f( r, g, b );  
  draw();
  glLineWidth( 1 );
  //glDepthFunc( df );
  //glCullFace( GL_BACK );
  glDisable( GL_CULL_FACE );
  glPolygonMode( GL_FRONT, GL_FILL );
  if( !blend ) glDisable( GL_BLEND );
	if( texture ) glEnable( GL_TEXTURE_2D );
  useShadow = false;
  glColor4f(1, 1, 1, 0.9f);
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
