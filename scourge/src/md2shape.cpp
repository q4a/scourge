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
    glBindTexture(GL_TEXTURE_2D, g_Texture[0]);
    // set the color so it's not random...
    glColor4f(1.0f, 1.0f, 1.0f, 1.0f);

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
          if(pObject->pTexVerts) {
              glTexCoord2f(pObject->pTexVerts[ index2 ].x, pObject->pTexVerts[ index2 ].y);             
          }          

          // Pass in the current vertex of the object (Corner of current face)
          glVertex3f(pObject->pVerts[ index ].x * div, pObject->pVerts[ index ].y * div, -pObject->pVerts[ index ].z * div);
        }
      }
    glEnd();  
  }
  glPopMatrix();    
  glDisable(GL_CULL_FACE);
}






void MD2Shape::swap(unsigned char & a, unsigned char & b) {
    unsigned char temp;

    temp = a;
    a    = b;
    b    = temp;

    return;
}


/////////////////////////////////// CREATE TEXTURE \\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\*
/////
/////  This creates a texture in OpenGL that we can texture map
/////
/////////////////////////////////// CREATE TEXTURE \\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\*

void MD2Shape::CreateTexture(GLuint textureArray[],char *strFileName,int textureID) {
    SDL_Surface *pBitmap[1];

    if( strFileName == NULL )                           // Return from the function if no file name was passed in
        return ;

    // We need to load the texture data, so we use a cool function that SDL offers.

    pBitmap[0] = SDL_LoadBMP(strFileName);              // Load the bitmap and store the data

    if(pBitmap[0] == NULL)                                // If we can't load the file, quit!
    {
        cerr << " Failed loading " << strFileName << " : " << SDL_GetError() << endl;
        exit(0);
    }

    // Generate a texture with the associative texture ID stored in the array
    glGenTextures(1, &textureArray[textureID]);

    // Bind the texture to the texture arrays index and init the texture
    glBindTexture(GL_TEXTURE_2D, textureArray[textureID]);

    // WARNING   : GO THROUGH THESE FEW LINES FOR SWAPPING ROWS ONLY IF YOU REALLY NEED TO, OR ELSE SKIP
    // THE LINES MARKED BELOW. Just take it for granted that these lines of code swap rows in the texture
    // as required by us.

    // <SKIP> <SKIP> <SKIP> <SKIP>   (FROM HERE)        -------------------
    //
    // IMPORTANT : SDL loads Bitmaps differently when compared to the default windows loader. The row 0
    // corresponds to the top row and NOT the bottom row. Therefore if we don't invert the order of the rows,
    // then the textures will appear (vertically) inverted.
    // Therefore we must reverse the ordering of the rows in the data of the loaded surface ( the member
    //  'pixels' of the structure)

    // Rearrange the pixelData

    int width  = pBitmap[0] -> w;
    int height = pBitmap[0] -> h;
    unsigned char * data = (unsigned char *) (pBitmap[0] -> pixels);         // the pixel data

    int BytesPerPixel = pBitmap[0] -> format -> BytesPerPixel;

    //////////// This is how we swap the rows :
    // For half the rows, we swap row 'i' with row 'height - i -1'. (if we swap all the rows
    // like this and not the first half or the last half, then we get the same texture again !
    //
    // Now these rows are not stored as 2D arrays, instead they are stored as a long 1D array.
    // So each row is concatenated after the previous one to make this long array. Our swap
    // function swaps one byte at a time and therefore we swap BytesPerPixel (= total bits per pixel)
    // bytes succesively.
    //
    // so the three loops below are :
    // for the first half of the rows
    //   for all the width (which is width of image * BytesPerPixel, where BytesPerPixel = total bits per pixel).
    //   (Here, for each pixel, we have to increment j by total bits per pixel (to get to next pixel to swap))
    //      for(each byte in this pixel i.e k = 0 to BytesPerPixel - 1, i.e BytesPerPixel bytes.
    //        swap the byte with the corresponding byte in the 'height -i -1'th row ('i'th row from bottom)
    for( int i = 0 ; i < (height / 2) ; ++i )
        for( int j = 0 ; j < width * BytesPerPixel; j += BytesPerPixel )
            for(int k = 0; k < BytesPerPixel; ++k)
                swap( data[ (i * width * BytesPerPixel) + j + k], data[ ( (height - i - 1) * width * BytesPerPixel ) + j + k]);

    unsigned char *pixels = new unsigned char[width * height * 3];

    int count = 0;

    // the following lines extract R,G and B values from any bitmap

    for(int i = 0; i < (width * height); ++i)
    {
        byte r,g,b;                                                // R,G and B that we will put into pImage

        Uint32 pixel_value = 0;                                    // 32 bit unsigned int (as dictated by SDL)

        // the following loop extracts the pixel (however wide it is 8,16,24 or 32) and
        // creates a long with all these bytes taken together.

        for(int j = BytesPerPixel - 1 ; j >=0; --j)                // for each byte in the pixel (from the right)
        {
            pixel_value = pixel_value << 8;                        // left shift pixel value by 8 bits
            pixel_value = pixel_value | data[ (i * BytesPerPixel) + j ];  // then make the last 8 bits of pixel value  =
        }                                                                 // the byte that we extract from pBitmap's data

        SDL_GetRGB(pixel_value, pBitmap[0] -> format, (Uint8 *)&r, (Uint8 *)&g, (Uint8 *)&b);     // here we get r,g,b from pixel_value which is stored in the form specified by pBitmap->format

        pixels[count++] = r;          // in our tImage classes we store r first
        pixels[count++] = g;          // then g
        pixels[count++] = b;          // and finally b (for bmps - three channels only)
        /*pixels[(i * BytesPerPixel) + 0] = r;          // in our tImage classes we store r first
        pixels[(i * BytesPerPixel) + 1] = g;          // then g
        pixels[(i * BytesPerPixel) + 2] = b;          // and finally b (for bmps - three channels only)*/

        pixel_value = 0;                                           // reset pixel , else we get incorrect values of r,g,b
    }


    // </SKIP> </SKIP> </SKIP> </SKIP>    (TO HERE)   -------------------

    // Build Mipmaps (builds different versions of the picture for distances - looks better)
    gluBuild2DMipmaps(GL_TEXTURE_2D, 3, pBitmap[0]->w, pBitmap[0]->h, GL_RGB, GL_UNSIGNED_BYTE, pixels);

    // Lastly, we need to tell OpenGL the quality of our texture map.  GL_LINEAR is the smoothest.
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR_MIPMAP_NEAREST);
    glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);

    // Now we need to free the bitmap data that we loaded since openGL stored it as a texture

    SDL_FreeSurface(pBitmap[0]);                        // Free the texture data we dont need it anymore
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
