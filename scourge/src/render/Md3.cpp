//***********************************************************************//
//																		 //
//		- "Talk to me like I'm a 3 year old!" Programming Lessons -		 //
//                                                                       //
//		$Author:		DigiBen		digiben@gametutorials.com			 //
//																		 //
//		$Program:		MD3 Loader										 //
//																		 //
//		$Description:	Demonstrates how to loader a Quake3 MD3 Model	 //
//																		 //
//		$Date:			3/10/02											 //
//																		 //
//***********************************************************************//



#include "Md3.h"
#include <string>
#include <iostream>
#include <fstream>
#include <math.h>
#include "modelwrapper.h"
#include "md3shape.h"

using namespace std;																				 

//////////// *** NEW *** ////////// *** NEW *** ///////////// *** NEW *** ////////////////////

///////////////////////////////////////////////////////////////////////////////////
//
// This file includes all of the class definitions for CLoadMD3 and CModelMD3.
// Get ready for some intense code.  There is a lot to look through that might
// discourage you from loading MD3 models, but It's not so bad once you get
// started.  Hopefully I can explain this well enough that you won't be
// intimidated by it.
//
// Let me describe what these classes are for.  CModelMD3 is the class the 
// client (you) uses to load and draw the character.  CLoadMD3 is what the
// CModelMD3 class uses to load the character mesh information.  It is nearly
// identical to the CLoadMD2 class that we implemented in the MD2 Loader tutorial.
//
// Also, like in that tutorial, we convert the Quake3 structures to our own t3DModel
// format that we use in the other loader tutorials, give or take a few variables.
// Once again, I want to keep all the loader tutorials using the same structure so
// there isn't more code to get lost in, which you are unfamiliar with.  Another
// reason for this is that this is a perfect opportunity to create an abstract
// base class for a C3DModel (or perhaps CModel3D) object.  This will allow each
// file loader to have their own inherited class.  This is a great design which will
// enable us to pass in any type of 3D object into functions that deal with objects,
// no matter what file format they are loaded from.  If you don't understand this
// concept, don't worry, when we make our sweet engine from all these tutorials combined,
// you will see what I mean.
//
// You'll notice that we have a stray function for checking sub-strings, IsInString().
// We use this when we load the skin.  Don't worry about it.  It just returns true
// if the strSubString is inside of the original string, strString.  That way we don't
// have to parse the whole skin texture file line to look for the tag.
//
// Let's get to the code.  Be sure to look down at the Quick Notes near the bottom
// of this file for a overview of what this code all does.  It is assumed that you have
// read the introduction to .md3 loading at the top of Main.cpp
//
//

///////////////////////////////////////////////////////////////////////////////////
//
// This version of the tutorial incorporates the animation data stored in the MD3
// character files.  We will be reading in the .cfg file that stores the animation
// data.  The rotations and translations of the models will be done using a matrix.
// There will be no more calls to glTranslatef().  To create the rotation and
// translation matrix, quaternions will be used.  This is because quaternions
// are excellent for interpolating between 2 rotations, as well as not overriding
// another translation causing "gimbal lock".
// 
// So, why do we need to interpolate?  Well, the animations for the character are
// stored in key frames.  Instead of saving each frame of an animation, key frames
// are stored to cut down on memory and disk space.  The files would be huge if every
// frame was saved for every animation, as well as creating a huge memory footprint.  
// Can you imagine having 10+ models in memory with all of that animation data?  
// 
// The animation key frames are stored in 2 ways.  The torso and legs mesh have vertices
// stored for each of the key frames, along with separate rotations and translations
// for the basic bone animation.  Remember, that each .md3 represents a bone, that needs
// to be connected at a joint.  For instance, the torso is connected to the legs, and the
// head is connected to the torso.  So, that makes 3 bones and 2 joints.  If you add the
// weapon, the weapon is connected to the hand joint, which gives us 4 bones and 3 joints.
// Unlike conventional skeletal animation systems, the main animations of the character's
// movement, such as a gesture or swimming animation, are done not with bones, but with 
// vertex key frames, like in the .md2 format. Since the lower, upper, head and weapon models
// are totally different models, which aren't seamlessly connected to each other, then parent
// node needs to end a message (a translation and rotation) down to all it's child nodes to
// tell them where they need to be in order for the animation to look right.  A good example
// of this is when the legs has the DEATH3 animation set,  The legs might kick back into a back
// flip that lands the character on their face, dead.  Well, since the main models are separate,
// if the legs didn't tell the torso where to go, then the model's torso would stay in the same
// place and the body would detach itself from the legs.  The exporter calculates all this stuff
// for you of course.
// 
// But getting back to the interpolation, since we use key frames, if we didn't interpolate
// between them, the animation would look very jumping and unnatural.  It would also go too
// fast.  By interpolating, we create a smooth transition between each key frame.
//
// As seen in the .md2 tutorials, interpolating between vertices is easy if we use the
// linear interpolation function:  p(t) = p0 + t(p1 - p0).  The same goes for translations,
// since it's just 2 3D points.  This is not so for the rotations.  The Quake3 character
// stores the rotations for each key frame in a 3x3 matrix.  This isn't a simple linear
// interpolation that needs to be performed.  If we convert the matrices to a quaternion,  
// then use spherical linear interpolation (SLERP) between the current frame's quaternion 
// and the next key frame's quaternion, we will have a new interpolated quaternion that
// can be converted into a 4x4 matrix to be applied to the current model view matrix in OpenGL.
// After finding the interpolated translation to be applied, we can slip that into the rotation
// matrix before it's applied to the current matrix, which will require only one matrix command.
//
// You'll notice that in the CreateFromMatrix() function in our quaternion class, I allow a
// row and column count to be passed in.  This is just a dirty way to allow a 3x3 or 4x4 matrix
// to be passed in.  Instead of creating a whole new function and copy and pasting the main 
// code, it seemed fitting for a tutorial.  It's obvious that the quaternion class is missing
// a tremendous amount of functions, but I chose to only keep the functions that we would use.
// 
// For those of you who don't know what interpolation us, here is a section abstracted 
// from the MD2 Animation tutorial:
//
// -------------------------------------------------------------------------------------
// Interpolation: Gamedev.net's Game Dictionary say interpolation is "using a ratio 
// to step gradually a variable from one value to another."  In our case, this
// means that we gradually move our vertices from one key frame to another key frame.
// There are many types of interpolation, but we are just going to use linear.
// The equation for linear interpolation is this:
//
//				p(t) = p0 + t(p1 - p0)
//
//				t - The current time with 0 being the start and 1 being the end
//				p(t) - The result of the equation with time t
//				p0 - The starting position
//				p1 - The ending position
//
// Let's throw in an example with numbers to test this equation.  If we have
// a vertex stored at 0 along the X axis and we wanted to move the point to
// 10 with 5 steps, see if you can fill in the equation without a time just yet.
//
// Finished?  You should have come up with:
//
//				p(t) = 0 + t(10 - 0)
//				p(t) = 0 + 10t
//				p(t) = 10t
//
// Now, all we need it a time from 0 to 1 to pass in, which will allow us to find any
// point from 0 to 10, depending on the time.  Since we wanted to find out the distance
// we need to travel each frame if we want to reach the end point in 5 steps, we just
// divide 1 by 5: 1/5 = 0.2
//
// We can then pass this into our equation:
//
//				p(0.2) = 10 * 0.2
//				p(0.2) = 2
//
// What does that tell us?  It tells us we need to move the vertex along the x
// axis each frame by a distance of 2 to reach 10 in 5 steps.  Yah yah, this isn't
// rocket science, but it's important to know that what your mind would have done
// immediately without thinking about it, is linear interpolation.  
//
// Are you starting to see how this applies to our model?  If we only read in key
// frames, then we need to interpolate every vertex between the current and next
// key frame for animation.  To get a perfect idea of what is going on, try
// taking out the interpolation and just render the key frames.  You will notice
// that you can still see what is kinda going on, but it moves at an incredible pace!
// There is not smoothness, just a really fast jumpy animation.
// ------------------------------------------------------------------------------------
//
// Let's jump into the code (hold your breath!)
//
//


////////////////////////////// CREATE MATRIX \\\\\\\\\\\\\\\\\\\\\\\\\\\\\*
/////
/////	This function converts a quaternion to a rotation matrix
/////
////////////////////////////// CREATE MATRIX \\\\\\\\\\\\\\\\\\\\\\\\\\\\\*	

void CQuaternion::CreateMatrix(float *pMatrix)
{
	// Make sure the matrix has allocated memory to store the rotation data
	if(!pMatrix) return;
	
	// Fill in the rows of the 4x4 matrix, according to the quaternion to matrix equations

	// First row
	pMatrix[ 0] = 1.0f - 2.0f * ( y * y + z * z );  
	pMatrix[ 1] = 2.0f * ( x * y - w * z );  
	pMatrix[ 2] = 2.0f * ( x * z + w * y );  
	pMatrix[ 3] = 0.0f;  

	// Second row
	pMatrix[ 4] = 2.0f * ( x * y + w * z );  
	pMatrix[ 5] = 1.0f - 2.0f * ( x * x + z * z );  
	pMatrix[ 6] = 2.0f * ( y * z - w * x );  
	pMatrix[ 7] = 0.0f;  

	// Third row
	pMatrix[ 8] = 2.0f * ( x * z - w * y );  
	pMatrix[ 9] = 2.0f * ( y * z + w * x );  
	pMatrix[10] = 1.0f - 2.0f * ( x * x + y * y );  
	pMatrix[11] = 0.0f;  

	// Fourth row
	pMatrix[12] = 0;  
	pMatrix[13] = 0;  
	pMatrix[14] = 0;  
	pMatrix[15] = 1.0f;

	// Now pMatrix[] is a 4x4 homogeneous matrix that can be applied to an OpenGL Matrix
}


///////////////////////////////// CREATE FROM MATRIX \\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\*
/////
/////	This creates a quaternion from a 3x3 or a 4x4 matrix, depending on rowColumnCount
/////
///////////////////////////////// CREATE FROM MATRIX \\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\*

void CQuaternion::CreateFromMatrix(float *pTheMatrix, int rowColumnCount)
{
	// Make sure the matrix has valid memory and it's not expected that we allocate it.
	// Also, we do a check to make sure the matrix is a 3x3 or a 4x4 (must be 3 or 4).
	if(!pTheMatrix || ((rowColumnCount != 3) && (rowColumnCount != 4))) return;

	// This function is used to take in a 3x3 or 4x4 matrix and convert the matrix
	// to a quaternion.  If rowColumnCount is a 3, then we need to convert the 3x3
	// matrix passed in to a 4x4 matrix, otherwise we just leave the matrix how it is.
	// Since we want to apply a matrix to an OpenGL matrix, we need it to be 4x4.

	// Point the matrix pointer to the matrix passed in, assuming it's a 4x4 matrix
	float *pMatrix = pTheMatrix;

	// Create a 4x4 matrix to convert a 3x3 matrix to a 4x4 matrix (If rowColumnCount == 3)
	float m4x4[16] = {0};

	// If the matrix is a 3x3 matrix (which it is for Quake3), then convert it to a 4x4
	if(rowColumnCount == 3)
	{
		// Set the 9 top left indices of the 4x4 matrix to the 9 indices in the 3x3 matrix.
		// It would be a good idea to actually draw this out so you can visualize it.
		m4x4[0]  = pTheMatrix[0];	m4x4[1]  = pTheMatrix[1];	m4x4[2]  = pTheMatrix[2];
		m4x4[4]  = pTheMatrix[3];	m4x4[5]  = pTheMatrix[4];	m4x4[6]  = pTheMatrix[5];
		m4x4[8]  = pTheMatrix[6];	m4x4[9]  = pTheMatrix[7];	m4x4[10] = pTheMatrix[8];

		// Since the bottom and far right indices are zero, set the bottom right corner to 1.
		// This is so that it follows the standard diagonal line of 1's in the identity matrix.
		m4x4[15] = 1;

		// Set the matrix pointer to the first index in the newly converted matrix
		pMatrix = &m4x4[0];
	}

	// The next step, once we made sure we are dealing with a 4x4 matrix, is to check the
	// diagonal of the matrix.  This means that we add up all of the indices that comprise
	// the standard 1's in the identity matrix.  If you draw out the identity matrix of a
	// 4x4 matrix, you will see that they 1's form a diagonal line.  Notice we just assume
	// that the last index (15) is 1 because it is not effected in the 3x3 rotation matrix.

	// Find the diagonal of the matrix by adding up it's diagonal indices.
	// This is also known as the "trace", but I will call the variable diagonal.
	float diagonal = pMatrix[0] + pMatrix[5] + pMatrix[10] + 1;
	float scale = 0.0f;

	// Below we check if the diagonal is greater than zero.  To avoid accidents with
	// floating point numbers, we substitute 0 with 0.00000001.  If the diagonal is
	// great than zero, we can perform an "instant" calculation, otherwise we will need
	// to identify which diagonal element has the greatest value.  Note, that it appears
	// that %99 of the time, the diagonal IS greater than 0 so the rest is rarely used.

	// If the diagonal is greater than zero
	if(diagonal > 0.00000001)
	{
		// Calculate the scale of the diagonal
		scale = float(sqrt(diagonal ) * 2);

		// Calculate the x, y, x and w of the quaternion through the respective equation
		x = ( pMatrix[9] - pMatrix[6] ) / scale;
		y = ( pMatrix[2] - pMatrix[8] ) / scale;
		z = ( pMatrix[4] - pMatrix[1] ) / scale;
		w = 0.25f * scale;
	}
	else 
	{
		// If the first element of the diagonal is the greatest value
		if ( pMatrix[0] > pMatrix[5] && pMatrix[0] > pMatrix[10] )  
		{	
			// Find the scale according to the first element, and double that value
			scale  = (float)sqrt( 1.0f + pMatrix[0] - pMatrix[5] - pMatrix[10] ) * 2.0f;

			// Calculate the x, y, x and w of the quaternion through the respective equation
			x = 0.25f * scale;
			y = (pMatrix[4] + pMatrix[1] ) / scale;
			z = (pMatrix[2] + pMatrix[8] ) / scale;
			w = (pMatrix[9] - pMatrix[6] ) / scale;	
		} 
		// Else if the second element of the diagonal is the greatest value
		else if ( pMatrix[5] > pMatrix[10] ) 
		{
			// Find the scale according to the second element, and double that value
			scale  = (float)sqrt( 1.0f + pMatrix[5] - pMatrix[0] - pMatrix[10] ) * 2.0f;
			
			// Calculate the x, y, x and w of the quaternion through the respective equation
			x = (pMatrix[4] + pMatrix[1] ) / scale;
			y = 0.25f * scale;
			z = (pMatrix[9] + pMatrix[6] ) / scale;
			w = (pMatrix[2] - pMatrix[8] ) / scale;
		} 
		// Else the third element of the diagonal is the greatest value
		else 
		{	
			// Find the scale according to the third element, and double that value
			scale  = (float)sqrt( 1.0f + pMatrix[10] - pMatrix[0] - pMatrix[5] ) * 2.0f;

			// Calculate the x, y, x and w of the quaternion through the respective equation
			x = (pMatrix[2] + pMatrix[8] ) / scale;
			y = (pMatrix[9] + pMatrix[6] ) / scale;
			z = 0.25f * scale;
			w = (pMatrix[4] - pMatrix[1] ) / scale;
		}
	}
}


/////////////////////////////////////// SLERP \\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\*
/////
/////	Returns a spherical linear interpolated quaternion between q1 and q2, with respect to t
/////
/////////////////////////////////////// SLERP \\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\*

CQuaternion CQuaternion::Slerp(CQuaternion &q1, CQuaternion &q2, float t)
{
	// Create a local quaternion to store the interpolated quaternion
	CQuaternion qInterpolated;

	// This function is the milk and honey of our quaternion code, the rest of
	// the functions are an appendage to what is done here.  Everyone understands
	// the terms, "matrix to quaternion", "quaternion to matrix", "create quaternion matrix",
	// "quaternion multiplication", etc.. but "SLERP" is the stumbling block, even a little 
	// bit after hearing what it stands for, "Spherical Linear Interpolation".  What that
	// means is that we have 2 quaternions (or rotations) and we want to interpolate between 
	// them.  The reason what it's called "spherical" is that quaternions deal with a sphere.  
	// Linear interpolation just deals with 2 points primarily, where when dealing with angles
	// and rotations, we need to use sin() and cos() for interpolation.  If we wanted to use
	// quaternions for camera rotations, which have much more instant and jerky changes in 
	// rotations, we would use Spherical-Cubic Interpolation.  The equation for SLERP is this:
	//
	// q = (((b.a)^-1)^t)a
	//
	// Go here for an a detailed explanation and proofs of this equation:
	//
	// http://www.magic-software.com/Documentation/quat.pdf
	//
	// Now, Let's code it

	// Here we do a check to make sure the 2 quaternions aren't the same, return q1 if they are
	if(q1.x == q2.x && q1.y == q2.y && q1.z == q2.z && q1.w == q2.w) 
		return q1;

	// Following the (b.a) part of the equation, we do a dot product between q1 and q2.
	// We can do a dot product because the same math applied for a 3D vector as a 4D vector.
	float result = (q1.x * q2.x) + (q1.y * q2.y) + (q1.z * q2.z) + (q1.w * q2.w);

	// If the dot product is less than 0, the angle is greater than 90 degrees
	if(result < 0.0f)
	{
		// Negate the second quaternion and the result of the dot product
		q2 = CQuaternion(-q2.x, -q2.y, -q2.z, -q2.w);
		result = -result;
	}

	// Set the first and second scale for the interpolation
	float scale0 = 1 - t, scale1 = t;

	// Next, we want to actually calculate the spherical interpolation.  Since this
	// calculation is quite computationally expensive, we want to only perform it
	// if the angle between the 2 quaternions is large enough to warrant it.  If the
	// angle is fairly small, we can actually just do a simpler linear interpolation
	// of the 2 quaternions, and skip all the complex math.  We create a "delta" value
	// of 0.1 to say that if the cosine of the angle (result of the dot product) between
	// the 2 quaternions is smaller than 0.1, then we do NOT want to perform the full on 
	// interpolation using.  This is because you won't really notice the difference.

	// Check if the angle between the 2 quaternions was big enough to warrant such calculations
	if(1 - result > 0.1f)
	{
		// Get the angle between the 2 quaternions, and then store the sin() of that angle
		float theta = (float)acos(result);
		float sinTheta = (float)sin(theta);

		// Calculate the scale for q1 and q2, according to the angle and it's sine value
		scale0 = (float)sin( ( 1 - t ) * theta) / sinTheta;
		scale1 = (float)sin( ( t * theta) ) / sinTheta;
	}	

	// Calculate the x, y, z and w values for the quaternion by using a special
	// form of linear interpolation for quaternions.
	qInterpolated.x = (scale0 * q1.x) + (scale1 * q2.x);
	qInterpolated.y = (scale0 * q1.y) + (scale1 * q2.y);
	qInterpolated.z = (scale0 * q1.z) + (scale1 * q2.z);
	qInterpolated.w = (scale0 * q1.w) + (scale1 * q2.w);

	// Return the interpolated quaternion
	return qInterpolated;
}

///////////////////////////////// IS IN STRING \\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\*
/////
/////	This returns true if the string strSubString is inside of strString
/////
///////////////////////////////// IS IN STRING \\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\*

bool IsInString(string strString, string strSubString)
{
	// Make sure both of these strings are valid, return false if any are empty
	if(strString.length() <= 0 || strSubString.length() <= 0) return false;

	// grab the starting index where the sub string is in the original string
	unsigned int index = strString.find(strSubString);

	// Make sure the index returned was valid
	if(index >= 0 && index < strString.length())
		return true;

	// The sub string does not exist in strString.
	return false;
}


///////////////////////////////// CMODEL MD3 \\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\*
/////
/////	This is our CModelMD3 constructor
/////
///////////////////////////////// CMODEL MD3 \\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\*

CModelMD3::CModelMD3( ModelLoader *loader )
{
	for( int i = 0; i < 3; i++ ) {
		this->min[i] = this->max[i] = 0;
	}
	paused = false;
	this->loader = loader;
	// Here we initialize all our mesh structures for the character
	memset(&m_Head,  0, sizeof(t3DModel));
	memset(&m_Upper, 0, sizeof(t3DModel));
	memset(&m_Lower, 0, sizeof(t3DModel));
	memset(&m_Weapon, 0, sizeof(t3DModel));
}

///////////////////////////////// ~CMODEL MD3 \\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\*
/////
/////	This is our CModelMD3 deconstructor
/////
///////////////////////////////// ~CMODEL MD3 \\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\*

CModelMD3::~CModelMD3()
{
	unloadSkinTextures();

	// Here we free all of the meshes in our model
	DestroyModel(&m_Head);
	DestroyModel(&m_Upper);
	DestroyModel(&m_Lower);
	DestroyModel(&m_Weapon);
}	

void CModelMD3::unloadSkinTextures() {
	for( unsigned int j = 0; j < strTextures.size(); j++ ) {
//		cerr << "*** Deleting MD3 texture:" << strTextures[j] << endl;
		loader->unloadSkinTexture( (char*)strTextures[j].c_str() );
	}
}

///////////////////////////////// DESTROY MODEL \\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\*
/////
/////	This frees our Quake3 model and all it's associated data
/////
///////////////////////////////// DESTROY MODEL \\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\*

void CModelMD3::DestroyModel(t3DModel *pModel)
{
	// To free a model, we need to go through every sub-object and delete
	// their model data.  Since there is only one array of tags and links stored
	// for the model and all of it's objects, we need to only free the model's 
	// tags and links once.

	// Go through all the objects in the model
	for(int i = 0; i < pModel->numOfObjects; i++)
	{
		// Free the faces, normals, vertices, and texture coordinates.
		if(pModel->pObject[i].pFaces)		delete [] pModel->pObject[i].pFaces;
		if(pModel->pObject[i].pNormals)		delete [] pModel->pObject[i].pNormals;
		if(pModel->pObject[i].pVerts)		delete [] pModel->pObject[i].pVerts;
		if(pModel->pObject[i].pTexVerts)	delete [] pModel->pObject[i].pTexVerts;
	}

	// Free the tags associated with this model
	if(pModel->pTags)		delete [] pModel->pTags;

	// Free the links associated with this model (We use free because we used malloc())
	if(pModel->pLinks)		free(pModel->pLinks);

	pModel->pAnimationMap.clear();
	pModel->pAnimations.clear();
}
	

///////////////////////////////// GET BODY PART \\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\*
/////
/////	This returns a specific model from the character (kLower, kUpper, kHead, kWeapon)
/////
///////////////////////////////// GET BODY PART \\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\*

t3DModel *CModelMD3::GetModel(int whichPart)
{
	// Return the legs model if desired
	if(whichPart == kLower) 
		return &m_Lower;

	// Return the torso model if desired
	if(whichPart == kUpper) 
		return &m_Upper;

	// Return the head model if desired
	if(whichPart == kHead) 
		return &m_Head;

	// Return the weapon model
	return &m_Weapon;
}

///////////////////////////////// LOAD MODEL \\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\*
/////
/////	This loads our Quake3 model from the given path and character name
/////
///////////////////////////////// LOAD MODEL \\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\*

bool CModelMD3::LoadModel(char *strPath, char *strModel )
{
	char strLowerModel[255] = {0};	// This stores the file name for the lower.md3 model
	char strUpperModel[255] = {0};	// This stores the file name for the upper.md3 model
	char strHeadModel[255]  = {0};	// This stores the file name for the head.md3 model
	char strLowerSkin[255]  = {0};	// This stores the file name for the lower.md3 skin
	char strUpperSkin[255]  = {0};	// This stores the file name for the upper.md3 skin
	char strHeadSkin[255]   = {0};	// This stores the file name for the head.md3 skin
	CLoadMD3 loadMd3;				// This object allows us to load each .md3 and .skin file

	// This function is where all the character loading is taken care of.  We use
	// our CLoadMD3 class to load the 3 mesh and skins for the character. Since we
	// just have 1 name for the model, we add that to _lower.md3, _upper.md3 and _head.md3
	// to load the correct mesh files.

	// Make sure valid path and model names were passed in
	if(!strPath || !strModel) return false;

	// Store the correct files names for the .md3 and .skin file for each body part.
	// We concatinate this on top of the path name to be loaded from.
//	if( strlen( strModel ) ) {
//		sprintf(strLowerModel, "%s/lower_%s.md3", strPath, strModel);
//		sprintf(strUpperModel, "%s/upper_%s.md3", strPath, strModel);
//		sprintf(strHeadModel,  "%s/head_%s.md3",  strPath, strModel);
//	} else {
		sprintf(strLowerModel, "%s/lower.md3", strPath );
		sprintf(strUpperModel, "%s/upper.md3", strPath );
		sprintf(strHeadModel,  "%s/head.md3",  strPath );
//	}
	
	// Get the skin file names with their path
	if( strlen( strModel ) ) {
		sprintf(strLowerSkin, "%s/lower_%s.skin", strPath, strModel);
		sprintf(strUpperSkin, "%s/upper_%s.skin", strPath, strModel);
		sprintf(strHeadSkin,  "%s/head_%s.skin",  strPath, strModel);
	} else {
		sprintf(strLowerSkin, "%s/lower.skin", strPath);
		sprintf(strUpperSkin, "%s/upper.skin", strPath);
		sprintf(strHeadSkin,  "%s/head.skin",  strPath);
	}
	
	// Next we want to load the character meshes.  The CModelMD3 class has member
	// variables for the head, upper and lower body parts.  These are of type t3DModel.
	// Depending on which model we are loading, we pass in those structures to ImportMD3.
	// This returns a true of false to let us know that the file was loaded okay.  The
	// appropriate file name to load is passed in for the last parameter.

	// Load the head mesh (*_head.md3) and make sure it loaded properly
	if(!loadMd3.ImportMD3(&m_Head,  strHeadModel))
	{
		// Display an error message telling us the file could not be found
		cerr << "Unable to load the HEAD model!" << endl;
		return false;
	}

	// Load the upper mesh (*_head.md3) and make sure it loaded properly
	if(!loadMd3.ImportMD3(&m_Upper, strUpperModel))		
	{
		// Display an error message telling us the file could not be found
		cerr <<"Unable to load the UPPER model!" << endl;
		return false;
	}

	// Load the lower mesh (*_lower.md3) and make sure it loaded properly
	if(!loadMd3.ImportMD3(&m_Lower, strLowerModel))
	{
		// Display an error message telling us the file could not be found
		cerr <<"Unable to load the LOWER model!" << endl;
		return false;
	}

	// Load the lower skin (*_upper.skin) and make sure it loaded properly
	if(!loadMd3.LoadSkin(&m_Lower, strLowerSkin))
	{
		// Display an error message telling us the file could not be found
		cerr <<"Unable to load the LOWER skin!" << endl;
		return false;
	}

	// Load the upper skin (*_upper.skin) and make sure it loaded properly
	if(!loadMd3.LoadSkin(&m_Upper, strUpperSkin))
	{
		// Display an error message telling us the file could not be found
		cerr <<"Unable to load the UPPER skin!" << endl;
		return false;
	}

	// Load the head skin (*_head.skin) and make sure it loaded properly
	if(!loadMd3.LoadSkin(&m_Head,  strHeadSkin))
	{
		// Display an error message telling us the file could not be found
		cerr <<"Unable to load the HEAD skin!" << endl;
		return false;
	}

	// Once the models and skins were loaded, we need to load then textures.
	// We don't do error checking for this because we call CreateTexture() and 
	// it already does error checking.  Most of the time there is only
	// one or two textures that need to be loaded for each character.  There are
	// different skins though for each character.  For instance, you could have a
	// army looking Lara Croft, or the normal look.  You can have multiple types of
	// looks for each model.  Usually it is just color changes though.

	// Load the lower, upper and head textures.  
	LoadModelTextures(&m_Lower, strPath );
	LoadModelTextures(&m_Upper, strPath );
	LoadModelTextures(&m_Head,  strPath );

	// We added to this function the code that loads the animation config file

	// This stores the file name for the .cfg animation file
	char strConfigFile[255] = {0};	

	// Add the path and file name prefix to the animation.cfg file
//	if( strlen( strModel ) ) {
//		sprintf(strConfigFile,  "%s/animation_%s.cfg",  strPath, strModel);
//	} else {
		sprintf(strConfigFile,  "%s/animation.cfg",  strPath );
//	}

	// Load the animation config file (*_animation.config) and make sure it loaded properly
	if(!LoadAnimations(strConfigFile))
	{
		// Display an error message telling us the file could not be found
		cerr << "Unable to load the Animation Config File!" << endl;
		return false;
	}

	// The character data should all be loaded when we get here (except the weapon).
	// Now comes the linking of the body parts.  This makes it so that the legs (lower.md3)
	// are the parent node, then the torso (upper.md3) is a child node of the legs.  Finally,
	// the head is a child node of the upper body.  What I mean by node, is that you can
	// think of the model having 3 bones and 2 joints.  When you translate the legs you want
	// the whole body to follow because they are inseparable (unless a magic trick goes wrong).
	// The same goes for the head, it should go wherever the body goes.  When we draw the
	// lower body, we then recursively draw all of it's children, which happen to be just the
	// upper body.  Then we draw the upper body's children, which is just the head.  So, to
	// sum this all up, to set each body part's children, we need to link them together.
	// For more information on tags, refer to the Quick Notes and the functions below.

	// Link the lower body to the upper body when the tag "tag_torso" is found in our tag array
	LinkModel(&m_Lower, &m_Upper, "tag_torso");

	// Link the upper body to the head when the tag "tag_head" is found in our tag array
	LinkModel(&m_Upper, &m_Head, "tag_head");


	//SetTorsoAnimation( "TORSO_STAND", true );
	//SetLegsAnimation( "LEGS_IDLE", true );
		
	// The character was loaded correctly so return true
	return true;
}


///////////////////////////////// LOAD WEAPON \\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\*
/////
/////	This loads a Quake3 weapon model from the given path and weapon name
/////
///////////////////////////////// LOAD WEAPON \\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\*

bool CModelMD3::LoadWeapon(char *strPath, char *strModel)
{
	char strWeaponModel[255]  = {0};	// This stores the file name for the weapon model
	char strWeaponShader[255] = {0};	// This stores the file name for the weapon shader.
	CLoadMD3 loadMd3;					// This object allows us to load the.md3 and .shader file

	// Make sure the path and model were valid, otherwise return false
	if(!strPath || !strModel) return false;

	// Concatenate the path and model name together
	sprintf(strWeaponModel, "%s/%s.md3", strPath, strModel);
	
	// Next we want to load the weapon mesh.  The CModelMD3 class has member
	// variables for the weapon model and all it's sub-objects.  This is of type t3DModel.
	// We pass in a reference to this model in to ImportMD3 to save the data read.
	// This returns a true of false to let us know that the weapon was loaded okay.  The
	// appropriate file name to load is passed in for the last parameter.

	// Load the weapon mesh (*.md3) and make sure it loaded properly
	if(!loadMd3.ImportMD3(&m_Weapon,  strWeaponModel))
	{
		// Display the error message that we couldn't find the weapon MD3 file and return false
		cerr <<"Unable to load the WEAPON model!" << endl;
		return false;
	}

	// Unlike the other .MD3 files, a weapon has a .shader file attached with it, not a
	// .skin file.  The shader file has it's own scripting language to describe behaviors
	// of the weapon.  All we care about for this tutorial is it's normal texture maps.
	// There are other texture maps in the shader file that mention the ammo and sphere maps,
	// but we don't care about them for our purposes.  I gutted the shader file to just store
	// the texture maps.  The order these are in the file is very important.  The first
	// texture refers to the first object in the weapon mesh, the second texture refers
	// to the second object in the weapon mesh, and so on.  I didn't want to write a complex
	// .shader loader because there is a TON of things to keep track of.  It's a whole
	// scripting language for goodness sakes! :)  Keep this in mind when downloading new guns.

	// Add the path, file name and .shader extension together to get the file name and path
	sprintf(strWeaponShader, "%s/%s.shader", strPath, strModel);

	// Load our textures associated with the gun from the weapon shader file
	if(!loadMd3.LoadShader(&m_Weapon, strWeaponShader))
	{
		// Display the error message that we couldn't find the shader file and return false
		cerr <<"Unable to load the SHADER file!" << endl;
		return false;
	}

	// We should have the textures needed for each weapon part loaded from the weapon's
	// shader, so let's load them in the given path.
	LoadModelTextures(&m_Weapon, strPath );

	// Just like when we loaded the character mesh files, we need to link the weapon to
	// our character.  The upper body mesh (upper.md3) holds a tag for the weapon.
	// This way, where ever the weapon hand moves, the gun will move with it.

	// Link the weapon to the model's hand that has the weapon tag
	LinkModel(&m_Upper, &m_Weapon, "tag_weapon");
		
	// The weapon loaded okay, so let's return true to reflect this
	return true;
}


///////////////////////////////// LOAD MODEL TEXTURES \\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\*
/////
/////	This loads the textures for the current model passed in with a directory
/////
///////////////////////////////// LOAD WEAPON \\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\*

void CModelMD3::LoadModelTextures(t3DModel *pModel, char *strPath )
{
	// This function loads the textures that are assigned to each mesh and it's
	// sub-objects.  For instance, the Lara Croft character has a texture for the body
	// and the face/head, and since she has the head as a sub-object in the lara_upper.md3 model, 
	// the MD3 file needs to contain texture information for each separate object in the mesh.
	// There is another thing to note too...  Some meshes use the same texture map as another 
	// one. We don't want to load 2 of the same texture maps, so we need a way to keep track of
	// which texture is already loaded so that we don't double our texture memory for no reason.
	// This is controlled with a STL vector list of "strings".  Every time we load a texture
	// we add the name of the texture to our list of strings.  Before we load another one,
	// we make sure that the texture map isn't already in our list.  If it is, we assign
	// that texture index to our current models material texture ID.  If it's a new texture,
	// then the new texture is loaded and added to our characters texture array: m_Textures[].

	// fix the texture file names
	for(int i = 0; i < pModel->numOfMaterials; i++) {
		if(strlen(pModel->pMaterials[i].strFile) > 0) {
			char strFullPath[255] = {0};
			char *p = strchr( pModel->pMaterials[i].strFile, '\r' );
			if( p ) *p = 0;
			sprintf( strFullPath, "%s/%s", strPath, pModel->pMaterials[i].strFile );
			strcpy( pModel->pMaterials[i].strFile, strFullPath );
		}
	}

	// Go through all the materials that are assigned to this model
	for(int i = 0; i < pModel->numOfMaterials; i++)
	{
		// Check to see if there is a file name to load in this material
		if(strlen(pModel->pMaterials[i].strFile) > 0)
		{
			// Create a boolean to tell us if we have a new texture to load
			bool bNewTexture = true;

			// Go through all the textures in our string list to see if it's already loaded
			for(unsigned int j = 0; j < strTextures.size(); j++)
			{
				// If the texture name is already in our list of texture, don't load it again.
				if(!strcmp(pModel->pMaterials[i].strFile, strTextures[j].c_str()) )
				{
					// We don't need to load this texture since it's already loaded
					bNewTexture = false;

					// Assign the texture index to our current material textureID.
					// This ID will them be used as an index into m_Textures[].
					pModel->pMaterials[i].texureId = j;
				}
			}

			// Make sure before going any further that this is a new texture to be loaded
			if(bNewTexture == false) continue;
			
			//m_Textures[strTextures.size()] = wrapper->loadSkinTexture( strPath, pModel->pMaterials[i].strFile );
//			cerr << "*** Loading MD3 texture: count=" << strTextures.size() << " file=" << pModel->pMaterials[i].strFile << endl;
			m_Textures[strTextures.size()] = loader->loadSkinTexture( pModel->pMaterials[i].strFile );

			// We pass in a reference to an index into our texture array member variable.
			// The size() function returns the current loaded texture count.  Initially
			// it will be 0 because we haven't added any texture names to our strTextures list.
			//CreateTexture(&(m_Textures[strTextures.size()]), strFullPath);

			// Set the texture ID for this material by getting the current loaded texture count
			pModel->pMaterials[i].texureId = strTextures.size();

			// Now we increase the loaded texture count by adding the texture name to our
			// list of texture names.  Remember, this is used so we can check if a texture
			// is already loaded before we load 2 of the same textures.  Make sure you
			// understand what an STL vector list is.  We have a tutorial on it if you don't.
			strTextures.push_back(pModel->pMaterials[i].strFile);
		}
	}
}


///////////////////////////////// LOAD ANIMATIONS \\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\*
/////
/////	This loads the .cfg file that stores all the animation information
/////
///////////////////////////////// LOAD ANIMATIONS \\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\*

bool CModelMD3::LoadAnimations(char *strConfigFile)
{
	// This function is given a path and name to an animation config file to load.
	// The implementation of this function is arbitrary, so if you have a better way
	// to parse the animation file, that is just as good.  Whatever works.
	// Basically, what is happening here, is that we are grabbing an animation line:
	//
	// "0	31	0	25		// BOTH_DEATH1"
	//
	// Then parsing it's values.  The first number is the starting frame, the next
	// is the frame count for that animation (endFrame would equal startFrame + frameCount),
	// the next is the looping frames (ignored), and finally the frames per second that
	// the animation should run at.  The end of this line is the name of the animation.
	// Once we get that data, we store the information in our tAnimationInfo object, then
	// after we finish parsing the file, the animations are assigned to each model.  
	// Remember, that only the torso and the legs objects have animation.  It is important
	// to note also that the animation prefixed with BOTH_* are assigned to both the legs
	// and the torso animation list, hence the name "BOTH" :)

	// Create an animation object for every valid animation in the Quake3 Character
	tAnimationInfo animations[MAX_ANIMATIONS] = {0};

	// Open the config file
	ifstream fin(strConfigFile);

	// Here we make sure that the file was found and could be opened
	if( fin.fail() )
	{
		// Return an unsuccessful retrieval
		return false;
	}

	string strWord = "";				// This stores the current word we are reading in
	string strLine = "";				// This stores the current line we read in
	int currentAnim = 0;				// This stores the current animation count
	int torsoOffset = 0;				// The offset between the first torso and leg animation

	// Here we go through every word in the file until a numeric number if found.
	// This is how we know that we are on the animation lines, and past header info.
	// This of course isn't the most solid way, but it works fine.  It wouldn't hurt
	// to put in some more checks to make sure no numbers are in the header info.
	while( fin >> strWord)
	{
		// If the first character of the word is NOT a number, we haven't hit an animation line
		if(!isdigit( strWord[0] ))
		{
			// Store the rest of this line and go to the next one
			getline(fin, strLine);
			continue;
		}

		// If we get here, we must be on an animation line, so let's parse the data.
		// We should already have the starting frame stored in strWord, so let's extract it.

		// Get the number stored in the strWord string and create some variables for the rest
		int startFrame = atoi(strWord.c_str());
		int numOfFrames = 0, loopingFrames = 0, framesPerSecond = 0;
		
		// Read in the number of frames, the looping frames, then the frames per second
		// for this current animation we are on.
		fin >> numOfFrames >> loopingFrames >> framesPerSecond;

		// Initialize the current animation structure with the data just read in
		animations[currentAnim].startFrame		= startFrame;
		animations[currentAnim].endFrame		= startFrame + numOfFrames;
		animations[currentAnim].loopingFrames	= loopingFrames;
		animations[currentAnim].framesPerSecond = framesPerSecond;

		// Read past the "//" and read in the animation name (I.E. "BOTH_DEATH1").
		// This might not be how every config file is set up, so make sure.
		fin >> strLine >> strLine;

		// Copy the name of the animation to our animation structure
		strcpy(animations[currentAnim].strName, strLine.c_str());

		// If the animation is for both the legs and the torso, add it to their animation list
		if(IsInString(strLine, "BOTH"))
		{
			// Add the animation to each of the upper and lower mesh lists
			m_Upper.pAnimations.push_back(animations[currentAnim]);
			m_Lower.pAnimations.push_back(animations[currentAnim]);
		}
		// If the animation is for the torso, add it to the torso's list
		else if(IsInString(strLine, "TORSO"))
		{
			m_Upper.pAnimations.push_back(animations[currentAnim]);
		}
		// If the animation is for the legs, add it to the legs's list
		else if(IsInString(strLine, "LEGS"))
		{	
			// Because I found that some config files have the starting frame for the
			// torso and the legs a different number, we need to account for this by finding
			// the starting frame of the first legs animation, then subtracting the starting
			// frame of the first torso animation from it.  For some reason, some exporters
			// might keep counting up, instead of going back down to the next frame after the
			// end frame of the BOTH_DEAD3 anim.  This will make your program crash if so.
			
			// If the torso offset hasn't been set, set it
			if(!torsoOffset)
				torsoOffset = animations[LEGS_WALKCR].startFrame - animations[TORSO_GESTURE].startFrame;

			// Minus the offset from the legs animation start and end frame.
			animations[currentAnim].startFrame -= torsoOffset;
			animations[currentAnim].endFrame -= torsoOffset;

			// Add the animation to the list of leg animations
			m_Lower.pAnimations.push_back(animations[currentAnim]);
		}
	
		// Increase the current animation count
		currentAnim++;
	}	

	// Store the number if animations for each list by the STL vector size() function
	m_Lower.numOfAnimations = m_Lower.pAnimations.size();
	m_Upper.numOfAnimations = m_Upper.pAnimations.size();
	m_Head.numOfAnimations = m_Head.pAnimations.size();
	m_Weapon.numOfAnimations = m_Head.pAnimations.size();

	hashAnimations( &m_Lower );
	hashAnimations( &m_Upper );
	hashAnimations( &m_Head );
	hashAnimations( &m_Weapon );


	// Return a success
	return true;
}

void CModelMD3::hashAnimations( t3DModel *pModel ) {
	pModel->pAnimationMap.clear();
	for( unsigned int i = 0; i < pModel->pAnimations.size(); i++ ) {
		string s = pModel->pAnimations[ i ].strName;
		pModel->pAnimationMap[ s ] = i;
	}
}

///////////////////////////////// LINK MODEL \\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\*
/////
/////	This links the body part models to each other, along with the weapon
/////
///////////////////////////////// LINK MODEL \\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\*

void  CModelMD3::LinkModel(t3DModel *pModel, t3DModel *pLink, char *strTagName)
{
	// Make sure we have a valid model, link and tag name, otherwise quit this function
	if(!pModel || !pLink || !strTagName) return;

	// This function is used to link 2 models together at a psuedo joint.  For instance,
	// if we were animating an arm, we would link the top part of the arm to the shoulder,
	// then the forearm to would be linked to the top part of the arm, then the hand to
	// the forearm.  That is what is meant by linking.  That way, when we rotate the
	// arm at the shoulder, the rest of the arm will move with it because they are attached
	// to the same matrix that moves the top of the arm.  You can think of the shoulder
	// as the arm's parent node, and the rest are children that are subject to move to where
	// ever the top part of the arm goes.  That is how bone/skeletal animation works.
	//
	// So, we have an array of tags that have a position, rotation and name.  If we want
	// to link the lower body to the upper body, we would pass in the lower body mesh first,
	// then the upper body mesh, then the tag "tag_torso".  This is a tag that quake set as
	// as a standard name for the joint between the legs and the upper body.  This tag was
	// saved with the lower.md3 model.  We just need to loop through the lower body's tags,
	// and when we find "tag_torso", we link the upper.md3 mesh too that tag index in our
	// pLinks array.  This is an array of pointers to hold the address of the linked model.
	// Quake3 models are set up in a weird way, but usually you would just forget about a
	// separate array for links, you would just have a pointer to a t3DModel in the tag
	// structure, which in retrospect, you wouldn't have a tag array, you would have
	// a bone/joint array.  Stayed tuned for a bone animation tutorial from scratch.  This
	// will show you exactly what I mean if you are confused.

	// Go through all of our tags and find which tag contains the strTagName, then link'em
	for(int i = 0; i < pModel->numOfTags; i++)
	{
		// If this current tag index has the tag name we are looking for
		if( !strcasecmp(pModel->pTags[i].strName, strTagName) )
		{
			// Link the model's link index to the link (or model/mesh) and return
			pModel->pLinks[i] = pLink;
			return;
		}
	}
}


///////////////////////////////// UPDATE MODEL \\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\*
/////
/////	This sets the current frame of animation, depending on it's fps and t
/////
///////////////////////////////// UPDATE MODEL \\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\*	

void CModelMD3::UpdateModel( t3DModel *pModel, MD3Shape *shape )
{

	if( paused ) return;

	AnimInfo *ai = shape->getAnimInfo( pModel );

	// Initialize a start and end frame, for models with no animation
	int startFrame = 0;
	int endFrame   = 1;

	// This function is used to keep track of the current and next frames of animation
	// for each model, depending on the current animation.  Some models down have animations,
	// so there won't be any change.

	// Here we grab the current animation that we are on from our model's animation list
	tAnimationInfo *pAnim = &(pModel->pAnimations[ai->currentAnim]);

	// If there is any animations for this model
	if(pModel->numOfAnimations)
	{
		// Set the starting and end frame from for the current animation
		startFrame = pAnim->startFrame;
		endFrame   = pAnim->endFrame;
	}
	
	// This gives us the next frame we are going to.  We mod the current frame plus
	// 1 by the current animations end frame to make sure the next frame is valid.
	ai->nextFrame = (ai->currentFrame + 1) % endFrame;

	//if( pModel == &( m_Upper ) ) {
		//cerr << "Md3 anim=" << shape->getCurrentAnimation() << " start=" << startFrame << 
			//" end=" << endFrame << " current=" << pModel->currentFrame << " next=" << pModel->nextFrame << endl;
	//}

	// If the next frame is zero, that means that we need to start the animation over.
	// To do this, we set nextFrame to the starting frame of this animation.
	if(ai->nextFrame == 0) {		
		ai->nextFrame =  startFrame;
		if( shape && pModel == &( m_Upper ) ) {
			//cerr << "MD3 upper animation finished: annim=" << shape->getCurrentAnimation() << endl;
			shape->animationFinished();
		}
	}
		

	//cerr << "MD3: " << ( pModel == &(m_Upper) ? "UPPER" : "LOWER" ) << 
//		" anim=" << pModel->currentAnim <<
		//" start=" << startFrame << " end=" << endFrame << 
		//" next=" << pModel->nextFrame << " current=" << pModel->currentFrame << endl;

	// Next, we want to get the current time that we are interpolating by.  Remember,
	// if t = 0 then we are at the beginning of the animation, where if t = 1 we are at the end.
	// Anything from 0 to 1 can be thought of as a percentage from 0 to 100 percent complete.
	SetCurrentTime( pModel, shape );
}

///////////////////////////////// DRAW MODEL \\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\*
/////
/////	This recursively draws all the character nodes, starting with the legs
/////
///////////////////////////////// DRAW MODEL \\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\*

void CModelMD3::DrawModel( MD3Shape *shape )
{
	// This is the function that is called by the client (you) when using the 
	// CModelMD3 class object.  You will notice that we rotate the model by
	// -90 degrees along the x-axis.  This is because most modelers have z up
	// so we need to compensate for this.  Usually I would just switch the
	// z and y values when loading in the vertices, but the rotations that
	// are stored in the tags (joint info) are a matrix, which makes it hard
	// to change those to reflect Y up.  I didn't want to mess with that so
	// this 1 rotate will fix this problem.

	// Rotate the model to compensate for the z up orientation that the model was saved
	glRotatef(-90, 1, 0, 0);

	// Since we have animation now, when we draw the model the animation frames need
	// to be updated.  To do that, we pass in our lower and upper models to UpdateModel().
	// There is no need to pass in the head of weapon, since they don't have any animation.

	// Update the leg and torso animations
	UpdateModel( &m_Lower, shape );
	UpdateModel( &m_Upper, shape );

	// You might be thinking to draw the model we would just call RenderModel()
	// 4 times for each body part and the gun right?  That sounds logical, but since
	// we are doing a bone/joint animation... and the models need to be linked together,
	// we can't do that.  It totally would ignore the tags.  Instead, we start at the
	// root model, which is the legs.  The legs drawn first, then we go through each of
	// the legs linked tags (just the upper body) and then it with the tag's rotation
	// and translation values.  I ignored the rotation in this tutorial since we aren't
	// doing any animation.  I didn't want to overwhelm you with quaternions just yet :)
	// Normally in skeletal animation, the root body part is the hip area.  Then the legs
	// bones are created as children to the torso.  The upper body is also a child to
	// the torso.  Since the legs are one whole mesh, this works out somewhat the same way.  
	// This wouldn't work if the feet and legs weren't connected in the same mesh because
	// the feet rotations and positioning don't directly effect the position and rotation
	// of the upper body, the hips do.  If that makes sense...  That is why the root starts
	// at the hips and moves down the legs, and also branches out to the upper body and
	// out to the arms.

	// Draw the first link, which is the lower body.  This will then recursively go
	// through the models attached to this model and drawn them.
	glTranslatef( 0, 0, -min[1] ); // translate to floor.
	DrawLink( &m_Lower, shape );
}


///////////////////////////////// DRAW LINK \\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\*
/////
/////	This draws the current mesh with an effected matrix stack from the last mesh
/////
///////////////////////////////// DRAW LINK \\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\*

void CModelMD3::DrawLink( t3DModel *pModel, MD3Shape *shape )
{
	// Draw the current model passed in (Initially the legs)
	RenderModel( pModel, shape );

	AnimInfo *ai = shape->getAnimInfo( pModel );

//////////// *** NEW *** ////////// *** NEW *** ///////////// *** NEW *** ////////////////////

	// Though the changes to this function from the previous tutorial aren't huge, they
	// are pretty powerful.  Since animation is in effect, we need to create a rotational
	// matrix for each key frame, at each joint, to be applied to the child nodes of that 
	// object.  We can also slip in the interpolated translation into that same matrix.
	// The big thing in this function is interpolating between the 2 rotations.  The process
	// involves creating 2 quaternions from the current and next key frame, then using
	// slerp (spherical linear interpolation) to find the interpolated quaternion, then
	// converting that quaternion to a 4x4 matrix, adding the interpolated translation
	// to that matrix, then finally applying it to the current model view matrix in OpenGL.
	// This will then effect the next objects that are somehow explicitly or inexplicitly
	// connected and drawn from that joint.

	// Create some local variables to store all this crazy interpolation data
	CQuaternion qQuat, qNextQuat, qInterpolatedQuat;
	float *pMatrix, *pNextMatrix;
	float finalMatrix[16] = {0};

//////////// *** NEW *** ////////// *** NEW *** ///////////// *** NEW *** ////////////////////


	// Now we need to go through all of this models tags and draw them.
	for(int i = 0; i < pModel->numOfTags; i++)
	{
		// Get the current link from the models array of links (Pointers to models)
		t3DModel *pLink = pModel->pLinks[i];

		// If this link has a valid address, let's draw it!
		if(pLink)
		{			

//////////// *** NEW *** ////////// *** NEW *** ///////////// *** NEW *** ////////////////////

			// To find the current translation position for this frame of animation, we times
			// the currentFrame by the number of tags, then add i.  This is similar to how
			// the vertex key frames are interpolated.
			CVector3 vPosition = pModel->pTags[ai->currentFrame * pModel->numOfTags + i].vPosition;

			// Grab the next key frame translation position
			CVector3 vNextPosition = pModel->pTags[ai->nextFrame * pModel->numOfTags + i].vPosition;
		
			// By using the equation: p(t) = p0 + t(p1 - p0), with a time t,
			// we create a new translation position that is closer to the next key frame.
			vPosition.x = vPosition.x + ai->t * (vNextPosition.x - vPosition.x),
			vPosition.y	= vPosition.y + ai->t * (vNextPosition.y - vPosition.y),
			vPosition.z	= vPosition.z + ai->t * (vNextPosition.z - vPosition.z);			

			// Now comes the more complex interpolation.  Just like the translation, we
			// want to store the current and next key frame rotation matrix, then interpolate
			// between the 2.

			// Get a pointer to the start of the 3x3 rotation matrix for the current frame
			pMatrix = &pModel->pTags[ai->currentFrame * pModel->numOfTags + i].rotation[0][0];

			// Get a pointer to the start of the 3x3 rotation matrix for the next frame
			pNextMatrix = &pModel->pTags[ai->nextFrame * pModel->numOfTags + i].rotation[0][0];

			// Now that we have 2 1D arrays that store the matrices, let's interpolate them

			// Convert the current and next key frame 3x3 matrix into a quaternion
			qQuat.CreateFromMatrix( pMatrix, 3);
			qNextQuat.CreateFromMatrix( pNextMatrix, 3 );

			// Using spherical linear interpolation, we find the interpolated quaternion
			qInterpolatedQuat = qQuat.Slerp(qQuat, qNextQuat, ai->t);

			// Here we convert the interpolated quaternion into a 4x4 matrix
			qInterpolatedQuat.CreateMatrix( finalMatrix );
			
			// To cut out the need for 2 matrix calls, we can just slip the translation
			// into the same matrix that holds the rotation.  That is what index 12-14 holds.
			finalMatrix[12] = vPosition.x;
			finalMatrix[13] = vPosition.y;
			finalMatrix[14] = vPosition.z;
//////////// *** NEW *** ////////// *** NEW *** ///////////// *** NEW *** ////////////////////


			// Start a new matrix scope
			glPushMatrix();


//////////// *** NEW *** ////////// *** NEW *** ///////////// *** NEW *** ////////////////////

				// Finally, apply the rotation and translation matrix to the current matrix
				glMultMatrixf( finalMatrix );

//////////// *** NEW *** ////////// *** NEW *** ///////////// *** NEW *** ////////////////////

				
				// Recursively draw the next model that is linked to the current one.
				// This could either be a body part or a gun that is attached to
				// the hand of the upper body model.
				DrawLink( pLink, shape );

			// End the current matrix scope
			glPopMatrix();
		}
	}
}


///////////////////////////////// SET CURRENT TIME \\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\*
/////
/////	This sets time t for the interpolation between the current and next key frame
/////
///////////////////////////////// SET CURRENT TIME \\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\*

void CModelMD3::SetCurrentTime( t3DModel *pModel, MD3Shape *shape )
{
	float elapsedTime   = 0.0f;

	AnimInfo *ai = shape->getAnimInfo( pModel );

	// This function is very similar to finding the frames per second.
	// Instead of checking when we reach a second, we check if we reach
	// 1 second / our animation speed. (1000 ms / animationSpeed).
	// That's how we know when we need to switch to the next key frame.
	// In the process, we get the t value for how far we are at to going to the
	// next animation key frame.  We use time to do the interpolation, that way
	// it runs the same speed on any persons computer, regardless of their specs.
	// It might look choppier on a junky computer, but the key frames still be
	// changing the same time as the other persons, it will just be not as smooth
	// of a transition between each frame.  The more frames per second we get, the
	// smoother the animation will be.  Since we are working with multiple models 
	// we don't want to create static variables, so the t and elapsedTime data are 
	// stored in the model's structure.
	
	// Return if there is no animations in this model
	if(!pModel->pAnimations.size()) return;

	// Get the current time in milliseconds
	float time = (float)SDL_GetTicks();

	// Find the time that has elapsed since the last time that was stored
	elapsedTime = time - ai->lastTime;

	// Store the animation speed for this animation in a local variable
	int animationSpeed = pModel->pAnimations[ai->currentAnim].framesPerSecond;

	// To find the current t we divide the elapsed time by the ratio of:
	//
	// (1_second / the_animation_frames_per_second)
	//
	// Since we are dealing with milliseconds, we need to use 1000
	// milliseconds instead of 1 because we are using GetTickCount(), which is in 
	// milliseconds. 1 second == 1000 milliseconds.  The t value is a value between 
	// 0 to 1.  It is used to tell us how far we are from the current key frame to 
	// the next key frame.
	float t = elapsedTime / (1000.0f / animationSpeed);
	
	// If our elapsed time goes over the desired time segment, start over and go 
	// to the next key frame.
	if (elapsedTime >= (1000.0f / animationSpeed) )
	{
		// Set our current frame to the next key frame (which could be the start of the anim)
		ai->currentFrame = ai->nextFrame;

		// Set our last time for the model to the current time
		ai->lastTime = time;
	}

	// Set the t for the model to be used in interpolation
	ai->t = t;
}

///////////////////////////////// RENDER MODEL \\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\*
/////
/////	This renders the model data to the screen
/////
///////////////////////////////// RENDER MODEL \\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\*

void CModelMD3::RenderModel( t3DModel *pModel, MD3Shape *shape )
{

		// Make sure we have valid objects just in case. (size() is in the STL vector class)
	if(pModel->pObject.size() <= 0) return;

	AnimInfo *ai = shape->getAnimInfo( pModel );

	// Make sure we have valid objects just in case. (size() is in the STL vector class)
	if(pModel->pObject.size() <= 0) return;

	// Go through all of the objects stored in this model
	for(int i = 0; i < pModel->numOfObjects; i++)
	{
		// Get the current object that we are displaying
		t3DObject *pObject = &pModel->pObject[i];


//////////// *** NEW *** ////////// *** NEW *** ///////////// *** NEW *** ////////////////////

		// Now that we have animation for our model, we need to interpolate between
		// the vertex key frames.  The .md3 file format stores all of the vertex 
		// key frames in a 1D array.  This means that in order to go to the next key frame,
		// we need to follow this equation:  currentFrame * numberOfVertices
		// That will give us the index of the beginning of that key frame.  We just
		// add that index to the initial face index, when indexing into the vertex array.

		// Find the current starting index for the current key frame we are on
		int currentIndex = ai->currentFrame * pObject->numOfVerts; 

		// Since we are interpolating, we also need the index for the next key frame
		int nextIndex = ai->nextFrame * pObject->numOfVerts; 

//////////// *** NEW *** ////////// *** NEW *** ///////////// *** NEW *** ////////////////////


		// If the object has a texture assigned to it, let's bind it to the model.
		// This isn't really necessary since all models have textures, but I left this
		// in here to keep to the same standard as the rest of the model loaders.
		if(pObject->bHasTexture)
		{
			// Turn on texture mapping
			glEnable(GL_TEXTURE_2D);

			// Grab the texture index from the materialID index into our material list
			int textureID = pModel->pMaterials[pObject->materialID].texureId;

			// Bind the texture index that we got from the material textureID
			glBindTexture(GL_TEXTURE_2D, m_Textures[textureID]);
		}
		else
		{
			// Turn off texture mapping
			glDisable(GL_TEXTURE_2D);
		}

		// Start drawing our model triangles
		glBegin(GL_TRIANGLES);

			// Go through all of the faces (polygons) of the object and draw them
			for(int j = 0; j < pObject->numOfFaces; j++)
			{
				// Go through each vertex of the triangle and draw it.
				for(int whichVertex = 0; whichVertex < 3; whichVertex++)
				{
					// Get the index for the current point in the face list
					int index = pObject->pFaces[j].vertIndex[whichVertex];
								
					// Make sure there is texture coordinates for this (%99.9 likelyhood)
					if(pObject->pTexVerts) 
					{
						// Assign the texture coordinate to this vertex
						glTexCoord2f(pObject->pTexVerts[ index ].x, 
									 pObject->pTexVerts[ index ].y);
					}

//////////// *** NEW *** ////////// *** NEW *** ///////////// *** NEW *** ////////////////////
					
					// Like in the MD2 Animation tutorial, we use linear interpolation
					// between the current and next point to find the point in between,
					// depending on the model's "t" (0.0 to 1.0).

					// Store the current and next frame's vertex by adding the current
					// and next index to the initial index given from the face data.
					CVector3 vPoint1 = pObject->pVerts[ currentIndex + index ];
					CVector3 vPoint2 = pObject->pVerts[ nextIndex + index];

					//vPoint1.z -= min[1];
					//vPoint2.z -= min[1];

					// By using the equation: p(t) = p0 + t(p1 - p0), with a time t,
					// we create a new vertex that is closer to the next key frame.
					glVertex3f(vPoint1.x + ai->t * (vPoint2.x - vPoint1.x),
							   vPoint1.y + ai->t * (vPoint2.y - vPoint1.y),
							   vPoint1.z + ai->t * (vPoint2.z - vPoint1.z));

//////////// *** NEW *** ////////// *** NEW *** ///////////// *** NEW *** ////////////////////

				}
			}

		// Stop drawing polygons
		glEnd();
	}
}

void CModelMD3::findBounds( vect3d min, vect3d max ) {
	findModelBounds( &m_Lower, min, max );
	//cerr << "LOWER: min=" << min[1] << endl;
	findModelBounds( &m_Upper, min, max );
	//cerr << "UPPER: min=" << min[1] << endl;
	findModelBounds( &m_Head, min, max );
	//cerr << "HEAD: min=" << min[1] << endl;
}

void CModelMD3::findModelBounds( t3DModel *pModel, vect3d min, vect3d max ) {

	if( pModel->pAnimations.size() == 0 ) return;
	
	for(int i = 0; i < pModel->numOfObjects; i++) {
		t3DObject *pObject = &pModel->pObject[i];
		for( int frame = pModel->pAnimations[ 0 ].startFrame;
				 frame < pModel->pAnimations[ 0 ].endFrame;
				 frame++ ) {
			int currentIndex = frame * pObject->numOfVerts; 
			for(int j = 0; j < pObject->numOfFaces; j++) {
				for(int whichVertex = 0; whichVertex < 3; whichVertex++) {
					int index = pObject->pFaces[j].vertIndex[whichVertex];
					CVector3 vPoint = pObject->pVerts[ currentIndex + index ];

/*
	for(int o = 0; o < pModel->numOfObjects; o++) {
		t3DObject *pObject = &pModel->pObject[o];
		for(int j = 0; j < pObject->numOfVerts; j++) {
			CVector3 vPoint = pObject->pVerts[ j ];
*/			
			if (vPoint.x < min[2])	min[2] = vPoint.x;
			if (vPoint.y < min[0])	min[0] = vPoint.y;
			if (vPoint.z < min[1])	min[1] = vPoint.z;
			if (vPoint.x >= max[2])	max[2] = vPoint.x;
			if (vPoint.y >= max[0])	max[0] = vPoint.y;
			if (vPoint.z >= max[1])	max[1] = vPoint.z;
				}
			}
		}
	}
}

void CModelMD3::normalize( vect3d pmin, vect3d pmax ) {
	for( int i = 0; i < 3; i++ ) {
		this->min[i] = pmin[i];
		this->max[i] = pmax[i];
	}
}

void CModelMD3::normalizeModel( t3DModel *pModel, vect3d min, vect3d max ) {
	for(int o = 0; o < pModel->numOfObjects; o++) {
		t3DObject *pObject = &pModel->pObject[o];
		for(int j = 0; j < pObject->numOfVerts; j++) {
			CVector3 *vPoint = &(pObject->pVerts[ j ]);

			vPoint->x -= min[2];
			vPoint->y -= min[0];
			vPoint->z -= min[1];
		}
	}
}


//////////// *** NEW *** ////////// *** NEW *** ///////////// *** NEW *** ////////////////////

///////////////////////////////// SET TORSO ANIMATION \\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\*
/////
/////	This sets the current animation that the upper body will be performing
/////
///////////////////////////////// SET TORSO ANIMATION \\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\*

void CModelMD3::SetTorsoAnimation(char *strAnimation, bool force, MD3Shape *shape ) {
	string s = strAnimation;
	if( m_Upper.pAnimationMap.find( s ) != m_Upper.pAnimationMap.end() ) {
		int i = m_Upper.pAnimationMap[ s ];
		AnimInfo *ai = shape->getAnimInfo( &m_Upper );
		// Set the legs animation to the current animation we just found and return
		if( ai->currentAnim == i ) return;
		ai->currentAnim = i;
		if( force ) {
			ai->currentFrame = m_Upper.pAnimations[ai->currentAnim].startFrame;
		} else {
			ai->nextFrame = m_Upper.pAnimations[ai->currentAnim].startFrame;
		}
	} else {
		cerr << "*** WARNING: can't find animation for upper: " << strAnimation << endl;
	}
}


///////////////////////////////// SET LEGS ANIMATION \\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\*
/////
/////	This sets the current animation that the lower body will be performing
/////
///////////////////////////////// SET LEGS ANIMATION \\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\*

void CModelMD3::SetLegsAnimation(char *strAnimation, bool force, MD3Shape *shape ) {
	string s = strAnimation;
	if( m_Lower.pAnimationMap.find( s ) != m_Lower.pAnimationMap.end() ) {
		int i = m_Lower.pAnimationMap[ s ];
		// Set the legs animation to the current animation we just found and return
		AnimInfo *ai = shape->getAnimInfo( &m_Lower );
		if( ai->currentAnim == i ) return;
		ai->currentAnim = i;
		//if( force ) {
		ai->currentFrame = m_Lower.pAnimations[ai->currentAnim].startFrame;
		//} else {
		//				ai->nextFrame = m_Lower.pAnimations[ai->currentAnim].startFrame;
		//}
	} else {
		cerr << "*** WARNING: can't find animation for legs: " << strAnimation << endl;
	}
}


//////////////////////////  BELOW IS THE LOADER CLASS //////////////////////////////



///////////////////////////////// CLOAD MD3 \\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\*
/////
/////	This constructor initializes the md3 structures
/////
///////////////////////////////// CLOAD MD3 \\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\*

CLoadMD3::CLoadMD3()
{
	// Here we initialize our structures to 0
	memset(&m_Header, 0, sizeof(tMd3Header));

	// Set the pointers to null
	m_pSkins=NULL;
	m_pTexCoords=NULL;
	m_pTriangles=NULL;
	m_pBones=NULL;
}


///////////////////////////////// IMPORT MD3 \\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\*
/////
/////	This is called by the client to open the .Md3 file, read it, then clean up
/////
///////////////////////////////// IMPORT MD3 \\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\*

bool CLoadMD3::ImportMD3(t3DModel *pModel, char *strFileName)
{
	char strMessage[255] = {0};

	// This function handles the entire model loading for the .md3 models.
	// What happens is we load the file header, make sure it's a valid MD3 model,
	// then load the rest of the data, then call our CleanUp() function.

	// Open the MD3 file in binary
	m_FilePointer = fopen(strFileName, "rb");

	// Make sure we have a valid file pointer (we found the file)
	if(!m_FilePointer) 
	{
		// Display an error message and don't load anything if no file was found
		sprintf(strMessage, "Unable to find the file: %s!", strFileName);
		cerr << strMessage << endl;
		return false;
	}
	
	// Now that we know the file was found and it's all cool, let's read in
	// the header of the file.  If it has the correct 4 character ID and version number,
	// we can continue to load the rest of the data, otherwise we need to print an error.

	// Read the header data and store it in our m_Header member variable
	fread(&m_Header, 1, sizeof(tMd3Header), m_FilePointer);

	// Get the 4 character ID
	char *ID = m_Header.fileID;

	// The ID MUST equal "IDP3" and the version MUST be 15, or else it isn't a valid
	// .MD3 file.  This is just the numbers ID Software chose.

	// Make sure the ID == IDP3 and the version is this crazy number '15' or else it's a bad egg
	if((ID[0] != 'I' || ID[1] != 'D' || ID[2] != 'P' || ID[3] != '3') || m_Header.version != 15)
	{
		// Display an error message for bad file format, then stop loading
		sprintf(strMessage, "Invalid file format (Version not 15): %s!", strFileName);
		cerr << strMessage << endl;
		return false;
	}
	
	// Read in the model and animation data
	ReadMD3Data(pModel);

	// Clean up after everything
	CleanUp();

	// Return a success
	return true;
}


///////////////////////////////// READ MD3 DATA \\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\*
/////
/////	This function reads in all of the model's data, except the animation frames
/////
///////////////////////////////// READ MD3 DATA \\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\*

void CLoadMD3::ReadMD3Data(t3DModel *pModel)
{
	int i = 0;

	// This member function is the BEEF of our whole file.  This is where the
	// main data is loaded.  The frustrating part is that once the data is loaded,
	// you need to do a billion little things just to get the model loaded to the screen
	// in a correct manner.

	// Here we allocate memory for the bone information and read the bones in.
	m_pBones = new tMd3Bone [m_Header.numFrames];
	fread(m_pBones, sizeof(tMd3Bone), m_Header.numFrames, m_FilePointer);

	// Since we don't care about the bone positions, we just free it immediately.
	// It might be cool to display them so you could get a visual of them with the model.

	// Free the unused bones
	delete [] m_pBones;

	// Next, after the bones are read in, we need to read in the tags.  Below we allocate
	// memory for the tags and then read them in.  For every frame of animation there is
	// an array of tags.
	pModel->pTags = new tMd3Tag [m_Header.numFrames * m_Header.numTags];
	fread(pModel->pTags, sizeof(tMd3Tag), m_Header.numFrames * m_Header.numTags, m_FilePointer);

	// Assign the number of tags to our model
	pModel->numOfTags = m_Header.numTags;
	
	// Now we want to initialize our links.  Links are not read in from the .MD3 file, so
	// we need to create them all ourselves.  We use a double array so that we can have an
	// array of pointers.  We don't want to store any information, just pointers to t3DModels.
	pModel->pLinks = (t3DModel **) malloc(sizeof(t3DModel) * m_Header.numTags);
	
	// Initilialize our link pointers to NULL
	for (i = 0; i < m_Header.numTags; i++)
		pModel->pLinks[i] = NULL;

	// Now comes the loading of the mesh data.  We want to use ftell() to get the current
	// position in the file.  This is then used to seek to the starting position of each of
	// the mesh data arrays.

	// Get the current offset into the file
	long meshOffset = ftell(m_FilePointer);

	// Create a local meshHeader that stores the info about the mesh
	tMd3MeshInfo meshHeader;

	// Go through all of the sub-objects in this mesh
	for (i = 0; i < m_Header.numMeshes; i++)
	{
		// Seek to the start of this mesh and read in it's header
		fseek(m_FilePointer, meshOffset, SEEK_SET);
		fread(&meshHeader, sizeof(tMd3MeshInfo), 1, m_FilePointer);

		// Here we allocate all of our memory from the header's information
		m_pSkins     = new tMd3Skin [meshHeader.numSkins];
		m_pTexCoords = new tMd3TexCoord [meshHeader.numVertices];
		m_pTriangles = new tMd3Face [meshHeader.numTriangles];
		m_pVertices  = new tMd3Triangle [meshHeader.numVertices * meshHeader.numMeshFrames];

		// Read in the skin information
		fread(m_pSkins, sizeof(tMd3Skin), meshHeader.numSkins, m_FilePointer);
		
		// Seek to the start of the triangle/face data, then read it in
		fseek(m_FilePointer, meshOffset + meshHeader.triStart, SEEK_SET);
		fread(m_pTriangles, sizeof(tMd3Face), meshHeader.numTriangles, m_FilePointer);

		// Seek to the start of the UV coordinate data, then read it in
		fseek(m_FilePointer, meshOffset + meshHeader.uvStart, SEEK_SET);
		fread(m_pTexCoords, sizeof(tMd3TexCoord), meshHeader.numVertices, m_FilePointer);

		// Seek to the start of the vertex/face index information, then read it in.
		fseek(m_FilePointer, meshOffset + meshHeader.vertexStart, SEEK_SET);
		fread(m_pVertices, sizeof(tMd3Triangle), meshHeader.numMeshFrames * meshHeader.numVertices, m_FilePointer);

		// Now that we have the data loaded into the Quake3 structures, let's convert them to
		// our data types like t3DModel and t3DObject.  That way the rest of our model loading
		// code will be mostly the same as the other model loading tutorials.
		ConvertDataStructures(pModel, meshHeader);

		// Free all the memory for this mesh since we just converted it to our structures
		delete [] m_pSkins;    
		delete [] m_pTexCoords;
		delete [] m_pTriangles;
		delete [] m_pVertices;   

		// Increase the offset into the file
		meshOffset += meshHeader.meshSize;
	}
}


///////////////////////////////// CONVERT DATA STRUCTURES \\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\*
/////
/////	This function converts the .md3 structures to our own model and object structures
/////
///////////////////////////////// CONVERT DATA STRUCTURES \\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\*

void CLoadMD3::ConvertDataStructures(t3DModel *pModel, tMd3MeshInfo meshHeader)
{
	int i = 0;

	// This is function takes care of converting all of the Quake3 structures to our
	// structures that we have been using in all of our mode loading tutorials.  You
	// do not need this function if you are going to be using the Quake3 structures.
	// I just wanted to make it modular with the rest of the tutorials so you (me really) 
	// can make a engine out of them with an abstract base class.  Of course, each model
	// has some different data variables inside of the, depending on each format, but that
	// is perfect for some cool inheritance.  Just like in the .MD2 tutorials, we only
	// need to load in the texture coordinates and face information for one frame
	// of the animation (eventually in the next tutorial).  Where, the vertex information
	// needs to be loaded for every new frame, since it's vertex key frame animation 
	// used in .MD3 models.  Half-life models do NOT do this I believe.  It's just
	// pure bone/skeletal animation.  That will be a cool tutorial if the time ever comes.

	// Increase the number of objects (sub-objects) in our model since we are loading a new one
	pModel->numOfObjects++;
		
	// Create a empty object structure to store the object's info before we add it to our list
	t3DObject currentMesh = {0};

	// Copy the name of the object to our object structure
	strcpy(currentMesh.strName, meshHeader.strName);

	// Assign the vertex, texture coord and face count to our new structure
	currentMesh.numOfVerts   = meshHeader.numVertices;
	currentMesh.numTexVertex = meshHeader.numVertices;
	currentMesh.numOfFaces   = meshHeader.numTriangles;

	// Allocate memory for the vertices, texture coordinates and face data.
	// Notice that we multiply the number of vertices to be allocated by the
	// number of frames in the mesh.  This is because each frame of animation has a 
	// totally new set of vertices.  This will be used in the next animation tutorial.
	currentMesh.pVerts    = new CVector3 [currentMesh.numOfVerts * meshHeader.numMeshFrames];
	currentMesh.pTexVerts = new CVector2 [currentMesh.numOfVerts];
	currentMesh.pFaces    = new tFace [currentMesh.numOfFaces];

	// Go through all of the vertices and assign them over to our structure
	for (i=0; i < currentMesh.numOfVerts * meshHeader.numMeshFrames; i++)
	{
		// For some reason, the ratio 64 is what we need to divide the vertices by,
		// otherwise the model is gargantuanly huge!  If you use another ratio, it
		// screws up the model's body part position.  I found this out by just
		// testing different numbers, and I came up with 65.  I looked at someone
		// else's code and noticed they had 64, so I changed it to that.  I have never
		// read any documentation on the model format that justifies this number, but
		// I can't get it to work without it.  Who knows....  Maybe it's different for
		// 3D Studio Max files verses other software?  You be the judge.  I just work here.. :)
		currentMesh.pVerts[i].x =  m_pVertices[i].vertex[0] / 64.0f;
		currentMesh.pVerts[i].y =  m_pVertices[i].vertex[1] / 64.0f;
		currentMesh.pVerts[i].z =  m_pVertices[i].vertex[2] / 64.0f;
	}

	// Go through all of the uv coords and assign them over to our structure
	for (i=0; i < currentMesh.numTexVertex; i++)
	{
		// Since I changed the images to bitmaps, we need to negate the V ( or y) value.
		// This is because I believe that TARGA (.tga) files, which were originally used
		// with this model, have the pixels flipped horizontally.  If you use other image
		// files and your texture mapping is crazy looking, try deleting this negative.
		currentMesh.pTexVerts[i].x =  m_pTexCoords[i].textureCoord[0];
		currentMesh.pTexVerts[i].y = -m_pTexCoords[i].textureCoord[1];
	}

	// Go through all of the face data and assign it over to OUR structure
	for(i=0; i < currentMesh.numOfFaces; i++)
	{
		// Assign the vertex indices to our face data
		currentMesh.pFaces[i].vertIndex[0] = m_pTriangles[i].vertexIndices[0];
		currentMesh.pFaces[i].vertIndex[1] = m_pTriangles[i].vertexIndices[1];
		currentMesh.pFaces[i].vertIndex[2] = m_pTriangles[i].vertexIndices[2];

		// Assign the texture coord indices to our face data (same as the vertex indices)
		currentMesh.pFaces[i].coordIndex[0] = m_pTriangles[i].vertexIndices[0];
		currentMesh.pFaces[i].coordIndex[1] = m_pTriangles[i].vertexIndices[1];
		currentMesh.pFaces[i].coordIndex[2] = m_pTriangles[i].vertexIndices[2];
	}

	// Here we add the current object to our list object list
	pModel->pObject.push_back(currentMesh);
}


///////////////////////////////// LOAD SKIN \\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\*
/////
/////	This loads the texture information for the model from the *.skin file
/////
///////////////////////////////// LOAD SKIN \\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\*

bool CLoadMD3::LoadSkin(t3DModel *pModel, char *strSkin)
{
	// Make sure valid data was passed in
	if(!pModel || !strSkin) return false;

	// This function is used to load a .skin file for the .md3 model associated
	// with it.  The .skin file stores the textures that need to go with each
	// object and subject in the .md3 files.  For instance, in our Lara Croft model,
	// her upper body model links to 2 texture; one for her body and the other for
	// her face/head.  The .skin file for the lara_upper.md3 model has 2 textures:
	//
	// u_torso,models/players/laracroft/default.bmp
	// u_head,models/players/laracroft/default_h.bmp
	//
	// Notice the first word, then a comma.  This word is the name of the object
	// in the .md3 file.  Remember, each .md3 file can have many sub-objects.
	// The next bit of text is the Quake3 path into the .pk3 file where the 
	// texture for that model is stored  Since we don't use the Quake3 path
	// because we aren't making Quake, I just grab the texture name at the
	// end of the string and disregard the rest.  of course, later this is
	// concatenated to the original MODEL_PATH that we passed into load our character.
	// So, for the torso object it's clear that default.bmp is assigned to it, where
	// as the head model with the pony tail, is assigned to default_h.bmp.  Simple enough.
	// What this function does is go through all the lines of the .skin file, and then
	// goes through all of the sub-objects in the .md3 file to see if their name is
	// in that line as a sub string.  We use our cool IsInString() function for that.
	// If it IS in that line, then we know that we need to grab it's texture file at
	// the end of the line.  I just parse backwards until I find the last '/' character,
	// then copy all the characters from that index + 1 on (I.E. "default.bmp").
	// Remember, it's important to note that I changed the texture files from .tga
	// files to .bmp files because that is what all of our tutorials use.  That way
	// you don't have to sift through tons of image loading code.  You can write or
	// get your own if you really want to use the .tga format.

	// Open the skin file
	ifstream fin(strSkin);

	// Make sure the file was opened
	if(fin.fail())
	{
		// Display the error message and return false
		cerr << "Unable to load skin!" << endl;
		return false;
	}

	// These 2 variables are for reading in each line from the file, then storing
	// the index of where the bitmap name starts after the last '/' character.
	string strLine = "";
	int textureNameStart = 0;

	// Go through every line in the .skin file
	while(getline(fin, strLine))
	{
		// Loop through all of our objects to test if their name is in this line
		for(int i = 0; i < pModel->numOfObjects; i++)
		{
			// Check if the name of this object appears in this line from the skin file
			if( IsInString(strLine, pModel->pObject[i].strName) )			
			{			
				// To abstract the texture name, we loop through the string, starting
				// at the end of it until we find a '/' character, then save that index + 1.
				for(int j = strLine.length() - 1; j > 0; j--)
				{
					// If this character is a '/', save the index + 1
					if(strLine[j] == '/')
					{
						// Save the index + 1 (the start of the texture name) and break
						textureNameStart = j + 1;
						break;
					}	
				}

				// Create a local material info structure
				tMaterialInfo texture;

				// Copy the name of the file into our texture file name variable.
				// Notice that with string we can pass in the address of an index
				// and it will only pass in the characters from that point on. Cool huh?
				// So now the strFile name should hold something like ("bitmap_name.bmp")
				strcpy(texture.strFile, &strLine[textureNameStart]);
				
				// The tile or scale for the UV's is 1 to 1 
				texture.uTile = texture.vTile = 1;

				// Store the material ID for this object and set the texture boolean to true
				pModel->pObject[i].materialID = pModel->numOfMaterials;
				pModel->pObject[i].bHasTexture = true;

				// Here we increase the number of materials for the model
				pModel->numOfMaterials++;

				// Add the local material info structure to our model's material list
				pModel->pMaterials.push_back(texture);
			}
		}
	}

	// Close the file and return a success
	fin.close();
	return true;
}


///////////////////////////////// LOAD SHADER \\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\*
/////
/////	This loads the basic shader texture info associated with the weapon model
/////
///////////////////////////////// LOAD SHADER \\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\*

bool CLoadMD3::LoadShader(t3DModel *pModel, char *strShader)
{
	// Make sure valid data was passed in
	if(!pModel || !strShader) return false;

	// This function is used to load the .shader file that is associated with
	// the weapon model.  Instead of having a .skin file, weapons use a .shader file
	// because it has it's own scripting language to describe the behavior of the
	// weapon.  There is also many other factors like environment map and sphere map
	// textures, among other things.  Since I am not trying to replicate Quake, I
	// just care about the weapon's texture.  I went through each of the blocks
	// in the shader file and deleted everything except the texture name (of course
	// I changed the .tga files to .bmp for our purposes).  All this file now includes
	// is a texture name on each line.  No parsing needs to be done.  It is important
	// to keep in mind that the order of which these texture are stored in the file
	// is in the same order each sub-object is loaded in the .md3 file.  For instance,
	// the first texture name on the first line of the shader is the texture for
	// the main gun object that is loaded, the second texture is for the second sub-object
	// loaded, and so on. I just want to make sure that you understand that I hacked
	// up the .shader file so I didn't have to parse through a whole language.  This is
	// not a normal .shader file that we are loading.  I only kept the relevant parts.

	// Open the shader file
	ifstream fin(strShader);

	// Make sure the file was opened
	if(fin.fail())
	{
		// Display the error message and return false
		cerr << "Unable to load shader!" << endl;
		return false;
	}

	// These variables are used to read in a line at a time from the file, and also
	// to store the current line being read so that we can use that as an index for the 
	// textures, in relation to the index of the sub-object loaded in from the weapon model.
	string strLine = "";
	int currentIndex = 0;
	
	// Go through and read in every line of text from the file
	while(getline(fin, strLine))
	{
		// Create a local material info structure
		tMaterialInfo texture;

		// Copy the name of the file into our texture file name variable
		strcpy(texture.strFile, strLine.c_str());
				
		// The tile or scale for the UV's is 1 to 1 
		texture.uTile = texture.uTile = 1;

		// Store the material ID for this object and set the texture boolean to true
		pModel->pObject[currentIndex].materialID = pModel->numOfMaterials;
		pModel->pObject[currentIndex].bHasTexture = true;

		// Here we increase the number of materials for the model
		pModel->numOfMaterials++;

		// Add the local material info structure to our model's material list
		pModel->pMaterials.push_back(texture);

		// Here we increase the material index for the next texture (if any)
		currentIndex++;
	}

	// Close the file and return a success
	fin.close();
	return true;
}


///////////////////////////////// CLEAN UP \\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\*
/////
/////	This function cleans up our allocated memory and closes the file
/////
///////////////////////////////// CLEAN UP \\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\*

void CLoadMD3::CleanUp()
{
	// Since we free all of the member variable arrays in the same function as
	// we allocate them, we don't need to do any other clean up other than
	// closing the file pointer, which could probably also be done in the same
	// function.  I left it here so you can add more of your cleanup if you add
	// to this class. 

	// Close the current file pointer
	fclose(m_FilePointer);						
}

/////////////////////////////////////////////////////////////////////////////////
//
// * QUICK NOTES * 
//
// I hope you didn't throw up when you saw all the code :)  It is a beast to start out,
// but you should try writing it from scratch! :)  And besides, when doing character
// animation, you should expect it to be huge because this is some pretty advanced stuff.
// Since this is proprietary, all we have to work on is the file format.  If you find 
// anything wrong or just plain stupid, let me know and I will make the fix some others 
// don't have to suffer in ignorance like myself :)  Let's go over the whole shebang:
// 
// The first thing you have to understand about characters in Quake3, is that the model
// is split into multiple files.  There is a upper.md3, a head.md3 and a lower.md3.
// These hold the upper body, the head and legs models.  Only the upper body and the
// lower body have animations assigned to them.  This is why you will see a little
// head in the middle of the body in wire frame mode because the Lara Croft model
// needed animation in her head, so the artist cut off the head and put it in a secret
// spot (her tummy... oooooow), then made her head and pony tail part of the upper body.
// A cool trick eh? (just wanted the Canadians to feel like I support them).  
// 
// Speaking of animation, so how does this stuff all work?  Is it bone animation or isn't it?
// Well, yes and now.  First, yes, there is bone animation because the legs are the parent
// node, and then the upper body attaches to the legs, then the head attaches to the upper 
// body (or in Lara's model, the torso).  Wherever the legs move, the upper body is then
// translated and rotated to the correct position that it should be, and then the head
// is in the same matrix scope so it goes where the upper body goes.  Once again, we do not
// handle rotating in this tutorial.  That requires a ton of matrix code that will be used
// in the next animation tutorial with quaternions (The room goes silent.....).
//
// And as for the second point, no the animation isn't exactly bone animation.  Each
// animation is stored in vertex key frames, just like the .MD2 files were.  The only
// difference is that we have 3 body parts to keep track of.  That way it is more realistic
// in the way of, you can have Lara Croft doing something totally different with her legs,
// than what the top part of her is doing.  Such as, when she is doing the jumping animation,
// her upper body could be doing a shooting, picking up, or dropping animation.  Make sense?
// So to the question of if this is bone animation, I can say a firm!!!..... Yes/No :)
//
// The .md3 files has skins and shader files associated with them.  It has a skin if it's
// a body part and a shader file if it's a weapon model.  These files store, for the most
// part, the textures associated with each model.  Sometimes the model has multiple textures,
// one for each sub-object.  I left the skin files the same, but I deleted a lot of the
// garbage that we didn't need in the .shader file.  Look at LoaderShader() for more info.
//
// There is also a config (.cfg) file for the animations of the model.  This stores the
// first frame, the amount of frames for that animation, the looping frame count (not used),
// and the frames per second that that animation should run.  We don't do anything with this
// file in this tutorial, but in the next tutorial we will.
//
// Finally, this brings us to the most confusing part in the .md3 model format, the tags.
// What the heck are tags?  Tags can be thought of as joints.  If you look in the .skin files
// you will see the tags on a separate line, right before the texture info.  These are basically
// joints that we need to connect other models too.  These are the ones I have seen:
//
// tag_torso  - The joint that connects the hips and the upper body together
// tag_head   - The joing that connects the neck and the head model too
// tag_weapon - The tag that connects the hand of the upper body and the weapon too.
//
// Now, that's the easy part to understand... it's like the Velcro that each body part
// sticks too right?  Well, the next part is where the confusion comes in.  Besides being
// a joint it stores the bone rotations and translations that need to be applied to the
// model that is connected to that joint.  For instance, we never rotate or translate the
// lower.md3 model.  That is all taken care of in the vertex key frame animation, but, the
// model stores tags for the "tag_torso" joint, which tells the upper body where to move and
// rotate, depending on what the legs are doing.
//
// A good example of this is in the death animations.  One of Lara's death animations makes
// her do a back flip and land on her face.  Well, the legs animation has key frames that
// it interpolates to, to perform this back flip, but the rest of the model doesn't.  To
// compensate for that, each frame, the lower body model sends a message to the upper body
// model to move and rotate to a certain degree and position, along a certain axis.  In
// programming terms, this means that we push on a new matrix and then apply some translations
// and rotations to the rest of the body parts (upper, head and weapon parts).  You don't
// directly apply it to each one, you just push on a matrix, apply the rotations and 
// translations, then anything drawn after that will be apart of that new matrix.  Simple huh?
// The rotation is stored in a 3x3 rotation matrix.  That is why we didn't bother with it
// in this tutorial because there is no animation so no need to add more code to scare you away.
// The translation is a simple (x, y, z) position that can be easily passed in to glTranslatef().
// This can all be seen in DrawLink().  In the next tutorial, we will not use glTranslatef(),
// but we will just include the translation in our matrix that we create to rotate the model.
// Then we can just do a simple call to glMultMatrix() to do both the rotation and translation
// at the same time.
//
// As a final explanation about the tags, let me address the model linking.  This just
// attaches 2 models together at the desired tag (or joint). For example, we attach the
// the upper.md3 model to the lower.md3 model at the "tag_torso" joint.  Now, when ever
// the legs do their animation, it sends the translation and rotation messages to all of
// it's children, which in this case, is just the upper body model, which effects the
// upper body's children (The weapon and head models).  The link array has nothing to do
// with the file format being loaded in.  That is just the way that I chose to handle it.
// I found some source code at www.planetquake.com by Lonerunner.  This guy/gal is awesome!
// He has a ton of formats that he did, which can be found at Romka's site too.  
//
// That is pretty much the crux of the .MD3 loading.  There is some bone animation to be
// loaded, but I never really figured it out.  The position was always (0, 0, 0) so I don't
// know what those are for.  I was going to display the bones, but it doesn't seem to save
// the position.  Who knows... if you know how, let me know.  Maybe I am ignoring something.
// It has min/max values, but I didn't bother to try and figure it out.
// 
// I would like to point out, this is just the solution I came up with.  There is probably
// a lot more intelligent ways to handle this, but it works great for me.  When the Quake3
// source code comes out, maybe we will all learn a few things from the mast'ehr :)
//
// I would like The author of this fabulous model who allowed me to use it, his handle is:
//
//			- Pornstar (nickelbag@nyc.com).  Tasteless name, but he sure does a cool model :)
//
//
// Let me know if this helps you out!
// 
// 
// Ben Humphrey (DigiBen)
// Game Programmer
// DigiBen@GameTutorials.com
// Co-Web Host of www.GameTutorials.com
//
// The Quake3 .Md3 file format is owned by ID Software.  This tutorial is being used 
// as a teaching tool to help understand model loading and animation.  This should
// not be sold or used under any way for commercial use with out written consent
// from ID Software.
//
// Quake, Quake2 and Quake3 are trademarks of ID Software.
// Lara Croft is a trademark of Eidos and should not be used for any commercial gain.
// All trademarks used are properties of their respective owners. 
//
//
//

