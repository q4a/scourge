/***************************************************************************
                          constants.cpp  -  description
                             -------------------
    begin                : Sun Oct 12 2003
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

#include "constants.h"
 
char rootDir[300];

//sprintf(s, "Welcome to Scourge version %7.2f", SCOURGE_VERSION);
char *Constants::messages[][80] = {
  { 
	"Infamy awaits in the dungeons of Scourge!", 
	"Another day, another sewer! Welcome to Scourge!", 
	"Happy hunting; welcome to Scourge!" },
  { "That item is out of your reach", 
	"You can't touch that", 
	"You have to be closer to get that", 
	"You are too far to reach it" },
  { "The door is blocked",
	"Something is blocking that door",
	"You can't use that door; something is in the way" },
  { "You are now in single-step mode" },
  { "You are now in group mode" },
  { "Paused: you have entered turn-based mode" },
  { "Un-paused: you are in real-time mode" },
  { "Close" },
  { "Drop Item" },
  { "Open Item" },
  { "Drag items to/from the list" },
  { "Play Mission" },
  { "Do you really want to exit this mission?" },
  { "Exit mission and teleport back to base?" },
  { "OK" },
  { "Cancel" },
  { "Yes" },
  { "No" },
  { "Select a character who has leveled up." },
  { "No skill points available." },
  { "Select a skill first." },
  { "S.C.O.U.R.G.E. dialog" }
};

int Constants::messageCount[] = {
  3, 4, 3, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1
};

// opengl extension routines
PFNGLACTIVETEXTUREARBPROC glSDLActiveTextureARB = NULL;
PFNGLMULTITEXCOORD2FARBPROC glSDLMultiTexCoord2fARB = NULL;
PFNGLMULTITEXCOORD2IARBPROC glSDLMultiTexCoord2iARB = NULL;

const char *Constants::SKILL_NAMES[] = {
  "SWORD_WEAPON",
  "AXE_WEAPON",
  "BOW_WEAPON",
  "HAND_TO_HAND_COMBAT",

  "SPEED",
  "COORDINATION",
  "POWER",
  "IQ",
  "LEADERSHIP",
  "LUCK",
  "PIETY",
  "LORE",

  "SHIELD_DEFEND",
  "ARMOR_DEFEND",
  "WEAPON_DEFEND",
  "HAND_DEFEND",

  "MATERIAL_SPELL",
  "ILLUSION_SPELL",
  "NATURE_SPELL",
  "DIVINE_SPELL",

  "OPEN_LOCK",
  "FIND_TRAP",
  "MOVE_UNDETECTED",

  "SKILL_0", "SKILL_1", "SKILL_2", "SKILL_3", "SKILL_4", "SKILL_5", "SKILL_6", "SKILL_7", "SKILL_8", "SKILL_9"
};

const char *Constants::STATE_NAMES[] = {
  "blessed", "empowered", "enraged", "ac_protected", "magic_protected",
  "drunk", "poisoned", "cursed", "possessed", "blinded", "charmed", "changed", 
  "overloaded", "dead", "leveled"
}; 
/*
float Constants::textColor[][4]={{0.8f, 0.2f, 0.0f, 0.0f},
                           {0.0f, 0.2f, 0.0f, 0.0f}
                           };*/

bool Constants::multitexture = true;	

Constants::Constants(){
}

Constants::~Constants(){
}

char *Constants::getMessage(int index) {
  int n = (int)((float)messageCount[index] * rand() / RAND_MAX);
  return messages[index][n];
}

int Constants::getSkillByName(char *p) {
  if(!p || !strlen(p)) return -1;
  for(int i = 0; i < SKILL_COUNT; i++) {
	if(!strcmp(p, SKILL_NAMES[i])) return i;
  }
  return -1;
}

/*
  Read until the EOL (or EOF whichever comes first)
  Put line chars into 'line', excluding EOL chars.
  Return first char after EOL.
 */
int Constants::readLine(char *line, FILE *fp) {
  bool reachedEOL = false;
  int lc = 0;
  int n;
  while((n = fgetc(fp)) != EOF) {
	bool isEOLchar = (n == '\n' || n == '\r');
	if(reachedEOL) {
	  if(!isEOLchar) {
		line[lc++] = '\0';
		return n;
	  }
	} else {
	  if(!isEOLchar) line[lc++] = n;
	  else reachedEOL = true;
	}
  }
  line[lc++] = '\0';
  return EOF;
}

// *Note* 
//
// Below are some math functions for calculating vertex normals.  We want vertex normals
// because it makes the lighting look really smooth and life like.  You probably already
// have these functions in the rest of your engine, so you can delete these and call
// your own.  I wanted to add them so I could show how to calculate vertex normals.

//////////////////////////////  Math Functions  ////////////////////////////////*

// This computes the magnitude of a normal.   (magnitude = sqrt(x^2 + y^2 + z^2)
#define Mag(Normal) (sqrt(Normal.x*Normal.x + Normal.y*Normal.y + Normal.z*Normal.z))

// This calculates a vector between 2 points and returns the result
CVector3 Vector(CVector3 vPoint1, CVector3 vPoint2)
{
    CVector3 vVector;                           // The variable to hold the resultant vector

    vVector.x = vPoint1.x - vPoint2.x;          // Subtract point1 and point2 x's
    vVector.y = vPoint1.y - vPoint2.y;          // Subtract point1 and point2 y's
    vVector.z = vPoint1.z - vPoint2.z;          // Subtract point1 and point2 z's

    return vVector;                             // Return the resultant vector
}

// This adds 2 vectors together and returns the result
CVector3 AddVector(CVector3 vVector1, CVector3 vVector2)
{
    CVector3 vResult;                           // The variable to hold the resultant vector
    
    vResult.x = vVector2.x + vVector1.x;        // Add Vector1 and Vector2 x's
    vResult.y = vVector2.y + vVector1.y;        // Add Vector1 and Vector2 y's
    vResult.z = vVector2.z + vVector1.z;        // Add Vector1 and Vector2 z's

    return vResult;                             // Return the resultant vector
}

// This divides a vector by a single number (scalar) and returns the result
CVector3 DivideVectorByScaler(CVector3 vVector1, float Scaler)
{
    CVector3 vResult;                           // The variable to hold the resultant vector
    
    vResult.x = vVector1.x / Scaler;            // Divide Vector1's x value by the scaler
    vResult.y = vVector1.y / Scaler;            // Divide Vector1's y value by the scaler
    vResult.z = vVector1.z / Scaler;            // Divide Vector1's z value by the scaler

    return vResult;                             // Return the resultant vector
}

// This returns the cross product between 2 vectors
CVector3 Cross(CVector3 vVector1, CVector3 vVector2)
{
    CVector3 vCross;                                // The vector to hold the cross product
                                                // Get the X value
    vCross.x = ((vVector1.y * vVector2.z) - (vVector1.z * vVector2.y));
                                                // Get the Y value
    vCross.y = ((vVector1.z * vVector2.x) - (vVector1.x * vVector2.z));
                                                // Get the Z value
    vCross.z = ((vVector1.x * vVector2.y) - (vVector1.y * vVector2.x));

    return vCross;                              // Return the cross product
}

// This returns the normal of a vector
CVector3 Normalize(CVector3 vNormal)
{
    double Magnitude;                           // This holds the magitude          

    Magnitude = Mag(vNormal);                   // Get the magnitude

    vNormal.x /= (float)Magnitude;              // Divide the vector's X by the magnitude
    vNormal.y /= (float)Magnitude;              // Divide the vector's Y by the magnitude
    vNormal.z /= (float)Magnitude;              // Divide the vector's Z by the magnitude

    return vNormal;                             // Return the normal
}

///////////////////////////////// COMPUTER NORMALS \\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\*
/////
/////   This function computes the normals and vertex normals of the objects
/////
///////////////////////////////// COMPUTER NORMALS \\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\*

void ComputeNormals(t3DModel *pModel)
{
    CVector3 vVector1, vVector2, vNormal, vPoly[3];

    // If there are no objects, we can skip this part
    if(pModel->numOfObjects <= 0)
        return;

    // What are vertex normals?  And how are they different from other normals?
    // Well, if you find the normal to a triangle, you are finding a "Face Normal".
    // If you give OpenGL a face normal for lighting, it will make your object look
    // really flat and not very round.  If we find the normal for each vertex, it makes
    // the smooth lighting look.  This also covers up blocky looking objects and they appear
    // to have more polygons than they do.    Basically, what you do is first
    // calculate the face normals, then you take the average of all the normals around each
    // vertex.  It's just averaging.  That way you get a better approximation for that vertex.

    // Go through each of the objects to calculate their normals
    for(int index = 0; index < pModel->numOfObjects; index++)
    {
        // Get the current object
        t3DObject *pObject = &(pModel->pObject[index]);

        // Here we allocate all the memory we need to calculate the normals
        CVector3 *pNormals      = new CVector3 [pObject->numOfFaces];
        CVector3 *pTempNormals  = new CVector3 [pObject->numOfFaces];
        pObject->pNormals       = new CVector3 [pObject->numOfVerts];

        // Go though all of the faces of this object
        for(int i=0; i < pObject->numOfFaces; i++)
        {                                               
            // To cut down LARGE code, we extract the 3 points of this face
            vPoly[0] = pObject->pVerts[pObject->pFaces[i].vertIndex[0]];
            vPoly[1] = pObject->pVerts[pObject->pFaces[i].vertIndex[1]];
            vPoly[2] = pObject->pVerts[pObject->pFaces[i].vertIndex[2]];

            // Now let's calculate the face normals (Get 2 vectors and find the cross product of those 2)

            vVector1 = Vector(vPoly[0], vPoly[2]);      // Get the vector of the polygon (we just need 2 sides for the normal)
            vVector2 = Vector(vPoly[2], vPoly[1]);      // Get a second vector of the polygon

            vNormal  = Cross(vVector1, vVector2);       // Return the cross product of the 2 vectors (normalize vector, but not a unit vector)
            pTempNormals[i] = vNormal;                  // Save the un-normalized normal for the vertex normals
            vNormal  = Normalize(vNormal);              // Normalize the cross product to give us the polygons normal

            pNormals[i] = vNormal;                      // Assign the normal to the list of normals
        }

        //////////////// Now Get The Vertex Normals /////////////////

        CVector3 vSum = {0.0, 0.0, 0.0};
        CVector3 vZero = vSum;
        int shared=0;

        for (int i = 0; i < pObject->numOfVerts; i++)           // Go through all of the vertices
        {
            for (int j = 0; j < pObject->numOfFaces; j++)   // Go through all of the triangles
            {                                               // Check if the vertex is shared by another face
                if (pObject->pFaces[j].vertIndex[0] == i || 
                    pObject->pFaces[j].vertIndex[1] == i || 
                    pObject->pFaces[j].vertIndex[2] == i)
                {
                    vSum = AddVector(vSum, pTempNormals[j]);// Add the un-normalized normal of the shared face
                    shared++;                               // Increase the number of shared triangles
                }
            }      
            
            // Get the normal by dividing the sum by the shared.  We negate the shared so it has the normals pointing out.
            pObject->pNormals[i] = DivideVectorByScaler(vSum, float(-shared));

            // Normalize the normal for the final vertex normal
            pObject->pNormals[i] = Normalize(pObject->pNormals[i]); 

            vSum = vZero;                                   // Reset the sum
            shared = 0;                                     // Reset the shared
        }
    
        // Free our memory and start over on the next object
        delete [] pTempNormals;
        delete [] pNormals;
    }
}

///////////////////////////////////////      SWAP      \\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\*
/////
/////  This swaps 2 elements we pass to it (swaps bytes)
/////
//////////////////////////////////////       SWAP      \\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\*
void swap(unsigned char & a, unsigned char & b) {
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

void CreateTexture(GLuint textureArray[],char *strFileName,int textureID) {
    SDL_Surface *pBitmap[1];

    if( strFileName == NULL )                           // Return from the function if no file name was passed in
        return ;

    // We need to load the texture data, so we use a cool function that SDL offers.

    pBitmap[0] = SDL_LoadBMP(strFileName);              // Load the bitmap and store the data

    if(pBitmap[0] == NULL)                                // If we can't load the file, quit!
    {
	  textureArray[textureID] = 0;	  
	  cerr << " Failed loading " << strFileName << " : " << SDL_GetError() << endl;
	  //exit(0);
	  return;
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
