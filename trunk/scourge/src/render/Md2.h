#ifndef _MD2_H
#define _MD2_H

#include "render.h"

/**
 * Credit for this code is mainly due to:
 * DigiBen     digiben@gametutorials.com
 * Look up his other great tutorials at:
 * http://www.gametutorials.com
 *
 * glCommands (and thus simplification of this file) is implemented 
 * thanks to David Henry tutorial : 
 *   http://tfc.duke.free.fr/us/tutorials/models/md2.htm 
 */ 

/**
  *@author Gabor Torok
  */
// These are the needed defines for the max values when loading .MD2 files
#define MD2_MAX_TRIANGLES       4096
#define MD2_MAX_VERTICES        2048
#define MD2_MAX_TEXCOORDS       2048
#define MD2_MAX_FRAMES          512
#define MD2_MAX_SKINS           32
#define MD2_MAX_FRAMESIZE       (MD2_MAX_VERTICES * 4 + 128)

// TODO :  Load different skins

// Md2 header information 
struct tMd2Header
{ 
   int magic;                   // This is used to identify the file
   int version;                 // The version number of the file (Must be 8)
   int skinWidth;               // The skin width in pixels
   int skinHeight;              // The skin height in pixels
   int frameSize;               // The size in bytes of a frame (constant for each)
   int numSkins;                // The number of skins associated with the model
   int numVertices;             // The number of vertices (constant for each frame)
   int numTexCoords;            // The number of texture coordinates
   int numTriangles;            // The number of faces (polygons)
   int numGlCommands;           // The number of gl commands
   int numFrames;               // The number of animation frames
   int offsetSkins;             // The offset in the file for the skin data
   int offsetTexCoords;         // The offset in the file for the texture data
   int offsetTriangles;         // The offset in the file for the face data
   int offsetFrames;            // The offset in the file for the frames data
   int offsetGlCommands;        // The offset in the file for the gl commands data
   int offsetEnd;               // The end of the file offset
};


// This is used to store the vertices that are read in for the current frame
struct tMd2AliasTriangle
{
   byte vertex[3];
   byte lightNormalIndex;
};

// This stores the animation scale, translation and name information for a frame, plus verts
struct tMd2AliasFrame
{
   float scale[3];
   float translate[3];
   char name[16];
   tMd2AliasTriangle aliasVertices[1];
};


// This stores a skin or a frame name 
typedef char tMd2String[64];

// different actions possible in a md2 file
enum md2_action{
    MD2_STAND,
    MD2_RUN,
    MD2_ATTACK,
    MD2_PAIN1,
    MD2_PAIN2,
    MD2_PAIN3,
    MD2_JUMP,
    MD2_FLIP,
    MD2_SALUTE,    
    MD2_TAUNT,
    MD2_WAVE,
    MD2_POINT,
    MD2_CRSTAND,
    MD2_CRWALK,
    MD2_CRATTACK,
    MD2_CRPAIN,
    MD2_CRDEATH,    
    MD2_DEATH1,
    MD2_DEATH2,
    MD2_DEATH3,
    
    // Must be last   
    MD2_CREATURE_ACTION_COUNT
};


class CLoadMD2
{
 
public:
    CLoadMD2();                                
    bool ImportMD2(t3DModel *pModel, char *strFileName);    
	void DeleteMD2( t3DModel *pModel );

private:        
        
    void ReadMD2Data(t3DModel *pModel);        
    void ParseAnimations(t3DModel *pModel);    
    void ComputeMinMaxValues(t3DModel *pModel);
    void CleanUp();
        
    FILE *m_FilePointer;   

    tMd2Header              m_Header;           // The header data
    tMd2String              *m_pSkins;          // The skin data        
    tMd2String              *m_pFrames;         // The name of the frames
};


#endif


/////////////////////////////////////////////////////////////////////////////////
// 
// Ben Humphrey (DigiBen)
// Game Programmer
// DigiBen@GameTutorials.com
// Co-Web Host of www.GameTutorials.com
//
// The Quake2 .Md2 file format is owned by ID Software.  This tutorial is being used 
// as a teaching tool to help understand model loading and animation.  This should
// not be sold or used under any way for commercial use with out written conset
// from ID Software.
//
// Quake and Quake2 are trademarks of id Software.
// All trademarks used are properties of their respective owners. 
//
//

