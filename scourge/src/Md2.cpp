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

#include "md2shape.h"
#include "Md2.h"
#include <math.h>
#include <string>
 
// Initializes the md2 structures
CLoadMD2::CLoadMD2()
{
    memset(&m_Header, 0, sizeof(tMd2Header));  
    m_pSkins=NULL;        
    m_pFrames=NULL;    
}

//   Called by the client to open the .Md2 file, read it, then clean up
bool CLoadMD2::ImportMD2(t3DModel *pModel, char *strFileName)
{
    char strMessage[255] = {0};

    // Open the MD2 file in binary
    m_FilePointer = fopen(strFileName, "rb");    
    if(!m_FilePointer) 
    {        
        sprintf(strMessage, "Unable to find the file: %s!", strFileName);
        cerr << strMessage << endl;
        exit(1);
    }
    
    // Read the header data
    fread(&m_Header, 1, sizeof(tMd2Header), m_FilePointer);
    if(SDL_BYTEORDER == SDL_BIG_ENDIAN) {
	  unsigned int *p = (unsigned int *)&m_Header;
	  for(unsigned int n = 0; n < sizeof(tMd2Header) / sizeof(unsigned int); n++) {
		*p = SDL_SwapLE32(*p);
		p++;
	  }
	}
    
    // Make sure version is '8'
    if(m_Header.version != 8)
    {    
        sprintf(strMessage, "Invalid file format (Version not 8): %s!", strFileName);
        cerr << strMessage << endl;
        exit(1);
    }

    // Read in the model and animation data    
    memset(pModel, 0, sizeof(t3DModel));
    ReadMD2Data(pModel);
    
    // Stores animation info    
    ParseAnimations(pModel);  
    
    // Computes min/max vertices values      
    ComputeMinMaxValues(pModel);
    
    CleanUp();
    return true;
}


// Reads in all of the model's data, except the animation frames
void CLoadMD2::ReadMD2Data(t3DModel *pModel)
{
    // Create a larger buffer for the frames of animation (not fully used yet)
    unsigned char buffer[MD2_MAX_FRAMESIZE];
    int k;    

    // Allocate all of our memory from the header's information    
    m_pSkins     = new tMd2String [m_Header.numSkins];             
    m_pFrames    = new tMd2String [m_Header.numFrames];    

    // Reads skin data
    fseek(m_FilePointer, m_Header.offsetSkins, SEEK_SET);        
    fread(m_pSkins, sizeof(tMd2String), m_Header.numSkins, m_FilePointer);            
	
	// pObjects will hold the key frames
    pModel->numOfObjects = m_Header.numFrames;    
    
	// Read the glCommands
    fseek(m_FilePointer, m_Header.offsetGlCommands, SEEK_SET);
    pModel->pGlCommands = new int [m_Header.numGlCommands];       
    fread(pModel->pGlCommands, sizeof(int), m_Header.numGlCommands, m_FilePointer);    
    if(SDL_BYTEORDER == SDL_BIG_ENDIAN) {
	  for(int i = 0; i < m_Header.numGlCommands; i++) {
		pModel->pGlCommands[i] = SDL_SwapLE32(pModel->pGlCommands[i]);
	  }
	}	             
    
    // set number of vertices of the model
    pModel->numVertices = m_Header.numVertices;
    
    // Extract and compute vertices for each frame of the model
    fseek(m_FilePointer, m_Header.offsetFrames, SEEK_SET); 
    pModel->vertices = new vect3d[ m_Header.numVertices * m_Header.numFrames ];       
    for (int i=0; i < m_Header.numFrames; i++)
    {        
        tMd2AliasFrame *pFrame = (tMd2AliasFrame *) buffer;                      
        fread(pFrame, 1, m_Header.frameSize, m_FilePointer);
        if(SDL_BYTEORDER == SDL_BIG_ENDIAN) {            
        // Note: byteswapping floats works on my powerbook, but I think
        //	 this may cause problems in the future. Floats and doubles aren't
        //	 as easy to change from LE to BE as ints.
	       float f;
	       for(int t = 0; t < 3; t++) {
		      BSWAPF(pFrame->scale[t], f);
		      pFrame->scale[t] = f;
    		  BSWAPF(pFrame->translate[t], f);
    		  pFrame->translate[t] = f;
    	   }
	   }
        
        strcpy(m_pFrames[i], pFrame->name);                            
        k = m_Header.numVertices * i;   

        // Translate and scale each vertex        
        for (int j=0; j < m_Header.numVertices; j++)
        {
            pModel->vertices[k + j][0] = pFrame->aliasVertices[j].vertex[0] * pFrame->scale[0] + pFrame->translate[0];            
            pModel->vertices[k + j][2] = -1 * (pFrame->aliasVertices[j].vertex[1] * pFrame->scale[1] + pFrame->translate[1]);
            pModel->vertices[k + j][1] = pFrame->aliasVertices[j].vertex[2] * pFrame->scale[2] + pFrame->translate[2];
        }
    }                        
}


//  Fills in the animation list for each animation by name and frame
void CLoadMD2::ParseAnimations(t3DModel *pModel)
{
    tAnimationInfo animation;
    string strLastName = "";
    
    // A hack because sometimes there is only 1 pain animation and/or 
    // 1 death animation.
    int nbPain = 0;    
    int nbDeath = 0;
    bool painDone = false;
    bool deathDone = false;
    tAnimationInfo painAnimation;
    tAnimationInfo deathAnimation;
    memset(&painAnimation, 0, sizeof(tAnimationInfo)); 
    memset(&deathAnimation, 0, sizeof(tAnimationInfo)); 
        
    // Go through all of the frames of animation and parse each animation
    // There is a standard frame number for each animations of md2 files    
    // TODO : implement it (no need to look for the last frame of each animation,
    // no need to store the number of frames for each animations ...), 
    // I(Daroth) will do it, someday...
    for(int i = 0; i < pModel->numOfObjects; i++)
    {        
        string strName  = m_pFrames[i];        
        int frameNum = 0;
        
        // Erase the frame numbers from the name
        for(int j = 0; j < strName.length(); j++)
        {
            // If the current index is a number and it's one of the last 2 characters 
            if( isdigit(strName[j]) && j >= strName.length() - 2)
            {                                
                frameNum = atoi(&strName[j]);               
                strName.erase(j, strName.length() - j);
                break;
            }
        }

        // Check if this animation name is not the same as the last frame,
        // or if we are on the last frame of animation for this model
        if(strName != strLastName || i == pModel->numOfObjects - 1)
        {
            // If this animation frame is NOT the first frame
            if(strLastName != "")
            {                                
                strcpy(animation.strName, strLastName.c_str());                             
                animation.endFrame = i;
                
                // ARRG! Sometimes there is only 1 pain/death animation instead of 3
                if(strLastName.substr(0, 4) == "pain"){                    
                    painAnimation.startFrame = animation.startFrame;
                    painAnimation.endFrame = animation.endFrame;
                    strcpy(painAnimation.strName, animation.strName);
                    nbPain++;
                }
                else if(nbPain >= 1 && nbPain < 3 && !painDone){
                    // insert 2 pain animation to keep 
                    // md2 action <-> pModel->pAnimation bijection                    
                    pModel->pAnimations.push_back(painAnimation);
                    pModel->pAnimations.push_back(painAnimation);
                    pModel->numOfAnimations+= 2;                    
                    painDone = true;
                }
                if(strLastName.substr(0, 5) == "death"){                    
                    deathAnimation.startFrame = animation.startFrame;
                    deathAnimation.endFrame = animation.endFrame;
                    strcpy(deathAnimation.strName, animation.strName);
                    nbDeath++;
                }
                else if(nbDeath >= 1 && nbDeath < 3 && !deathDone){
                    // insert 2 death animation to keep 
                    // md2 action <-> pModel->pAnimation bijection                     
                    pModel->pAnimations.push_back(deathAnimation);
                    pModel->pAnimations.push_back(deathAnimation); 
                    pModel->numOfAnimations+= 2;                                       
                    deathDone = true;
                }                                                
                
                pModel->pAnimations.push_back(animation);
                memset(&animation, 0, sizeof(tAnimationInfo));                                  
                pModel->numOfAnimations++;
            }

            // Set the starting frame number to the current frame number we just found,
            // minus 1 (since 0 is the first frame) and add 'i'.
            animation.startFrame = frameNum - 1 + i;
        }
        
        strLastName = strName;
    }
    
    // Death animations are at the end of the file
    if(nbDeath >= 1 && nbDeath < 3 && !deathDone){
        // insert 2 death animation to keep 
        // md2 action <-> pModel->pAnimation bijection                            
        pModel->pAnimations.push_back(deathAnimation);
        pModel->pAnimations.push_back(deathAnimation); 
        pModel->numOfAnimations+= 2;                           
    }         
}

void CLoadMD2::ComputeMinMaxValues(t3DModel *pModel){
    
    // Find the lowest point
    float minx, miny, minz;  
    float maxx, maxy, maxz;
    minx = miny = minz = maxx = maxy = maxz = 0;
    for(int i = 0; i < pModel->numVertices ; i++){
        if(pModel->vertices[i][0] > maxx) maxx = pModel->vertices[i][0];
        if(pModel->vertices[i][1] > maxy) maxy = pModel->vertices[i][1];
        if(pModel->vertices[i][2] > maxz) maxz = pModel->vertices[i][2];
        if(pModel->vertices[i][0] < minx) minx = pModel->vertices[i][0];
        if(pModel->vertices[i][1] < miny) miny = pModel->vertices[i][1];
        if(pModel->vertices[i][2] < minz) minz = pModel->vertices[i][2];     
    }
    pModel->movex = maxx - minx;
    pModel->movey = maxy;
    pModel->movez = maxz - minz; 

}

// Cleans up our allocated memory and closes the file
void CLoadMD2::CleanUp()
{    
    fclose(m_FilePointer);                      
    if(m_pSkins)     delete [] m_pSkins;        // Free the skins data        
    if(m_pFrames)    delete [] m_pFrames;       // Free the frames of animation    
}
