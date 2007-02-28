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

using namespace std;

const char *Constants::localhost = "localhost";
const char *Constants::adminUserName = "admin";

// assign the data dir
//char rootDir[300] = DATA_DIR;
char rootDir[300];
char localeDir[300];
char configDir[300] = CONFIG_DIR;

int get_config_dir_name( char *buff, int len )
{
#if defined( WIN32 )
    if ( (int)strlen( CONFIG_DIR ) +1 > len ) {
        return 1;
    }
    strcpy( buff, CONFIG_DIR );
    return 0;
#else
    struct passwd *pwent;

    pwent = getpwuid( getuid() );
    if ( pwent == NULL ) {
        perror( "getpwuid" );
        return 1;
    }

    if ( (int)(strlen( pwent->pw_dir ) + strlen( CONFIG_DIR) + 2) > len ) {
        return 1;
    }

    sprintf( buff, "%s/%s", pwent->pw_dir, CONFIG_DIR );
    return 0;
#endif /* defined( WIN32 ) */
}

int get_config_file_name( char *buff, int len ) {
  return get_file_name( buff, len, CONFIG_FILE );
}

int get_file_name( char *buff, int len, char *fileName ) {
  if (get_config_dir_name( buff, len ) != 0) {
    return 1;
  }
  if ( (int)(strlen( buff ) + strlen( fileName ) +2) > len ) {
    return 1;
  }

#if defined( WIN32 )
  strcat( buff, "\\" );
#else
  strcat( buff, "/" );
#endif /* defined( WIN32 ) */

  strcat( buff, fileName );
  return 0;
}

//sprintf(s, "Welcome to Scourge version %7.2f", SCOURGE_VERSION);
char *Constants::messages[][80] = {
	{
		N_("Infamy awaits in the dungeons of Scourge!"),
		N_("Another day, another sewer! Welcome to Scourge!"),
		N_("Happy hunting; welcome to Scourge!")},
	{ N_("That item is out of your reach"),
		N_("You can't touch that"),
		N_("You have to be closer to get that"),
		N_("You are too far to reach it")},
	{ N_("The door is blocked"),
		N_("Something is blocking that door"),
		N_("You can't use that door; something is in the way")},
	{ N_("You are now in single-step mode")},
	{ N_("You are now in group mode")},
	{ N_("Paused: you have entered turn-based mode")},
	{ N_("Un-paused: you are in real-time mode")},
	{ N_("Close")},
	{ N_("Drop Item")},
	{ N_("Open Item")},
	{ N_("Drag items to/from the list, Right click for info")},
	{ N_("Play Mission")},
	{ N_("Do you really want to exit this mission?")},
	{ N_("Teleport back to base?")},
	{ N_("OK")},
	{ N_("Cancel")},
	{ N_("Yes")},
	{ N_("No")},
	{ N_("Select a character who is alive and has leveled up.")},
	{ N_("No skill points available.")},
	{ N_("Select a skill first.")},
	{ N_("S.C.O.U.R.G.E. dialog")},
	{ N_("Use gate to descend a level?" ), 
		N_( "Use gate to ascend a level?")},
	{ N_("A dead character cannot perform this action.")},
	{ N_("hp:")},
	{ N_("ac:")},
	{ N_("Your magic fizzles and dies."),
		N_("Only the roaches are impressed by your mumbled words."),
		N_("The silence is broken only by some crickets nearby."),
		N_("Bazzoomm! A small cloud of smoke rises to the ceiling.")},
	{ N_("Your character cannot equip that item.")},
	{ N_("Fill out the server details, first.")},
	{ N_("Unable to connect to server.")},
	{ N_("You hear a very loud, metallic sound nearby."),
		N_("Something clicks."),
		N_("The loud twang doesn't help; this is freaky enough already.")},
	{ N_("A muffled, metalic, grating noise echoes from the dark."),
		N_("You've done something... you're not sure what but you hope it's had dinner.")},
	{ N_("You feel the resonance of an almost sub-tonal bass note."),
		N_("An omnious sound floats from a distant corner.")},
	{ N_("Suck! It is locked shut!"),
		N_("Try as you might, you can't open it."),
		N_("Perhaps it's locked.")},
	{ N_("This teleporter is off-line."),
		N_("Nothing happens."),
		N_("The teleporter blinks unimpressively.")},
	{ N_("Information")},
	{ N_("Delete old saved game?")},
	{ N_("You're not experienced enough to equip it yet.")},
	{ N_("Change key")},
	{ N_("Waiting for new key (Esc to cancel)")},
	{ N_("Conversation")},
	{ N_("Trade")},
	{ N_("Train")},
	{ N_("Healing Services")},
	{ N_("Donate to Temple")},
	{ N_("You don't meet the prerequisites for this capability.")},
	{ N_("You need cannot activate an automatic capability.")},
	{ N_("A two handed item requires two free hands.")},
	{ N_("TRAIN")},
	{ N_("SKILL")},
	{ N_("A magical force turns the lock's tumbles to open."),
		N_("Some kind of summoned energy opens the lock.")},
	{ N_("Killed by" ), 
		N_("Annihilated by"), 
		N_("Slain by"), 
		N_("Brought low by"), 
		N_("Dropped by"), 
		N_("Ruined by"), 
		N_("Extinguished by"), 
		N_("Laid low by")}
};

int Constants::messageCount[] = {
  3, 4, 3, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 4, 1, 1, 1,
  3, 2, 2, 3, 3, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 2, 8
};

// opengl extension routines
PFNGLACTIVETEXTUREARBPROC glSDLActiveTextureARB = NULL;
PFNGLMULTITEXCOORD2FARBPROC glSDLMultiTexCoord2fARB = NULL;
PFNGLMULTITEXCOORD2IARBPROC glSDLMultiTexCoord2iARB = NULL;

const char *Constants::POTION_SKILL_NAMES[] = {
  "HP", 
	"MP", 
	"AC"
};

// FIXME: these need display names
const char *Constants::STATE_NAMES[] = {
  "blessed", 
	"empowered", 
	"enraged", 
	"ac_protected", 
	"magic_protected",
  "drunk", 
	"poisoned", 
	"cursed", 
	"possessed", 
	"blinded", 
	"paralysed", 
	"invisible",
  "overloaded", 
	"dead", 
	"asleep"
};

const char *Constants::STATE_DISPLAY_NAMES[] = {
  N_( "blessed" ), 
	N_( "empowered" ), 
	N_( "enraged" ), 
	N_( "ac_protected" ), 
	N_( "magic_protected" ),
  N_( "drunk" ), 
	N_( "poisoned" ), 
	N_( "cursed" ), 
	N_( "possessed" ), 
	N_( "blinded" ), 
	N_( "paralysed" ), 
	N_( "invisible" ),
  N_( "overloaded" ), 
	N_( "dead" ), 
	N_( "asleep" )
};

const char *Constants::STATE_SYMBOLS[] = {
  N_("Air"), 
	N_("Earth"), 
	N_("Fire"), 
	N_("Stone"), 
	N_("Water"),
  N_("Jelly"), 
	N_("Ice"), 
	N_("Planar"), 
	N_("Astral"), 
	N_("Dire"), 
	N_("Cave"), 
	N_("Acid"),
  N_("Spirit"), 
	N_("Mist"), 
	N_("Quiet")
};

vector<int> Constants::goodStateMod;
vector<int> Constants::badStateMod;

const char *Constants::MAGIC_ITEM_NAMES[] = {
  N_("Lesser"), 
	N_("Greater"), 
	N_("Champion"), 
	N_("Divine")
};

const Color *Constants::MAGIC_ITEM_COLOR[] = {
  new Color( 0.6f, 1, 0.7f, 1 ),
  new Color( 0.6f, 0.7f, 1, 1 ),
  new Color( 1, 0.6f, 0.7f, 1 ),
  new Color( 1, 0.6f, 1, 1 )
};

const Color *Constants::SPECIAL_ITEM_COLOR = new Color( 1, 1, 0.5, 1 );

const char *Constants::EFFECT_NAMES[] = {
  "EFFECT_FLAMES", 
	"EFFECT_GLOW", 
	"EFFECT_TELEPORT", 
	"EFFECT_GREEN", 
	"EFFECT_EXPLOSION",
  "EFFECT_SWIRL", 
	"EFFECT_CAST_SPELL", 
	"EFFECT_RING", 
	"EFFECT_RIPPLE", 
	"EFFECT_DUST",
  "EFFECT_HAIL", 
	"EFFECT_TOWER", 
	"EFFECT_BLAST"
};

const char *Constants::npcTypeName[] = {
  "commoner", 
	"merchant", 
	"healer", 
	"sage", 
	"trainer"
};

const char *Constants::npcTypeDisplayName[] = {
  N_( "commoner" ), 
	N_( "merchant" ), 
	N_( "healer" ), 
	N_( "sage" ), 
	N_( "trainer" )
};

const char *Constants::cursorTextureName[] = {
	"cursor.bmp",
	"crosshair.bmp", 
	"attack.bmp", 
	"talk.bmp", 
	"use.bmp", 
	"forbidden.bmp", 
	"ranged.bmp", 
	"move.bmp"
};

bool Constants::multitexture = true;

Constants::Constants(){
}

Constants::~Constants(){
}

char *Constants::getMessage(int index) {
  int n = (int)((float)messageCount[index] * rand() / RAND_MAX);
  return _( messages[index][n] );
}

// return -1 on failure, or (-2 - i) on success
int Constants::getPotionSkillByName(char *p) {
  if(!p || !strlen(p)) return -1;
  for(int i = 0; i < POTION_SKILL_COUNT; i++) {
    if(!strcmp(p, POTION_SKILL_NAMES[i])) return (-2 - i);
  }
  return -1;
}

// return -1 on failure, 0+ on success
int Constants::getStateModByName( const char *p ) {
  if(!p || !strlen(p)) return -1;
  for(int i = 0; i < STATE_MOD_COUNT; i++) {
    if(!strcmp(p, STATE_NAMES[i])) return i;
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
  int ret = EOF;
  // read until the end of line
  while((n = fgetc(fp)) != EOF) {
    bool isEOLchar = (n == '\n' || n == '\r');
    if(reachedEOL) {
      if(!isEOLchar) {
        //line[lc++] = '\0';
        ret = n;
        break;
      }
    } else {
      if(!isEOLchar) line[lc++] = n;
      else reachedEOL = true;
    }
  }
  line[lc++] = '\0';
  // exclude same-line comment
  for( int i = 0; i < lc; i++ ) {
    if( line[i] == '#' || line[i] == '%' ) {
	  line[i] = '\0';
	  break;
	}
  }
  return ret;
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

void findNormal( CVector3 *p1, CVector3 *p2, CVector3 *p3, CVector3 *normal ) {
  CVector3 vVector1, vVector2, vNormal;

  // Now let's calculate the face normals (Get 2 vectors and find the cross product of those 2)

  vVector1 = Vector( *p1, *p3 );      // Get the vector of the polygon (we just need 2 sides for the normal)
  vVector2 = Vector( *p3, *p2 );      // Get a second vector of the polygon

  vNormal  = Cross( vVector1, vVector2 );       // Return the cross product of the 2 vectors (normalize vector, but not a unit vector)
  //pTempNormals[i] = vNormal;                  // Save the un-normalized normal for the vertex normals
  vNormal  = Normalize(vNormal);              // Normalize the cross product to give us the polygons normal

  normal->x = vNormal.x;
  normal->y = vNormal.y;
  normal->z = vNormal.z;
}

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
		pObject->shadingColorDelta = new float [pObject->numOfVerts];

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

float Constants::distance(float x1, float y1, float w1, float h1,
					 float x2, float y2, float w2, float h2) {
  // the distance between their centers
  float rx1 = x1 + (w1 / 2.0f);
  float rx2 = x2 + (w2 / 2.0f);
  float ry1 = y1 - (h1 / 2.0f);
  float ry2 = y2 - (h2 / 2.0f);

  float d = sqrt(((rx2 - rx1) * (rx2 - rx1)) +
				 ((ry2 - ry1) * (ry2 - ry1)));

  // remove the shapes' radius from the distance
  float d1 = sqrt(((w1 / 2.0f) * (w1 / 2.0f)) +
				  ((h1 / 2.0f) * (h1 / 2.0f)));
  float d2 = sqrt(((w2 / 2.0f) * (w2 / 2.0f)) +
				  ((h2 / 2.0f) * (h2 / 2.0f)));

  float ret = d - (d1 + d2);
  return (ret < 0 ? 0 : ret);
}

void Constants::checkTexture(char *message, int w, int h) {
  GLint maxTextureSize;
  glGetIntegerv( GL_MAX_TEXTURE_SIZE, &maxTextureSize );
  if( w > maxTextureSize || h > maxTextureSize) {
    cerr << "*****************************" << endl;
    cerr << "*****************************" << endl;
    cerr << "***&&&*** " << message <<
          " size=" << w << "x" << h <<
          " max texture size=" << maxTextureSize << endl;
    cerr << "Error: texture too big." << endl;
    cerr << "*****************************" << endl;
    cerr << "*****************************" << endl;
  }
}

bool Constants::isStateModTransitionWanted(int mod, bool setting) {
  bool effectFound = false;
  for(int i = 0; i < (int)goodStateMod.size(); i++) {
    if(goodStateMod[i] == mod) {
      effectFound = true;
      break;
    }
  }
  if(effectFound && setting) return true;

  effectFound = false;
  for(int i = 0; i < (int)badStateMod.size(); i++) {
    if(badStateMod[i] == mod) {
      effectFound = true;
      break;
    }
  }
  if((effectFound || mod == Constants::dead) && !setting) return true;
  return false;
}

int Constants::getRandomGoodStateMod() {
  return goodStateMod[(int)((float)goodStateMod.size()*rand()/RAND_MAX)];
}

int Constants::getRandomBadStateMod() {
  return badStateMod[(int)((float)badStateMod.size()*rand()/RAND_MAX)];
}

void Constants::initConstants() {
  Constants::goodStateMod.push_back( Constants::blessed );
  Constants::goodStateMod.push_back( Constants::empowered );
  Constants::goodStateMod.push_back( Constants::enraged );
  Constants::goodStateMod.push_back( Constants::ac_protected );
  Constants::goodStateMod.push_back( Constants::magic_protected );
  Constants::goodStateMod.push_back( Constants::invisible );

  Constants::badStateMod.push_back( Constants::poisoned );
  Constants::badStateMod.push_back( Constants::cursed );
  Constants::badStateMod.push_back( Constants::possessed );
  Constants::badStateMod.push_back( Constants::blinded );
  Constants::badStateMod.push_back( Constants::paralysed );
}

#define TEST_FILE "scourge.mo"
int Constants::findLocaleDir() {
	// Set the working locale. 
	// To change it, change the env var LANGUAGE or LANG (order of precedence.)
	setlocale( LC_ALL, "C" );
	setlocale( LC_MESSAGES, "" );
	
	// assume translations dir in rootDir
	sprintf( localeDir, "%s/translations", rootDir );
	cerr << "Looking for localized resources in: " << bindtextdomain( "scourge", localeDir ) << endl;
	bind_textdomain_codeset( "scourge", "UTF8" );

	textdomain( "scourge" ); 

	return 1;
}

int Constants::initRootDir( int argc, char *argv[] ) {

  // init the rootdir via binreloc
  cerr << "Constructing root path:" << endl;
#ifdef WIN32
  cerr << "\tWindows detected..." << endl;
  // for windows (binreloc doesn't compile in windows)
  strcpy( rootDir, DATA_DIR_NAME );
#else
#ifdef ENABLE_BINRELOC
  cerr << "\tusing binreloc..." << endl;
  char *p = br_find_data_dir( DATA_DIR_NAME );
  char tmp[255];
  sprintf( tmp, "%s/%s", br_find_data_dir( "" ), DATA_DIR_NAME );
  strcpy( rootDir, tmp );
  free( p );
#else
  cerr << "\tnot using binreloc..." << endl;
  strcpy( rootDir, DATA_DIR );
#endif
#endif

  cerr << "\ttemp rootDir=" << rootDir << endl;

  // FIXME: for windows, if this doesn't work, try using DATA_DIR
  // which is made by autoconf

  // Check to see if there's a local version of the data dir
  // (ie. we're running in the build folder and not in a distribution)
  char dir[300];
  dir[0] = '\0';

#ifdef WIN32
  findLocalResources(argv[0], dir);
#else
  char cwd[300];
  if( !getcwd( cwd, 300 ) ) {
    cerr << "Can't determine current working directory." << endl;
    exit( 1 );
  }
  findLocalResources( cwd, dir );
#endif

  if(strlen(dir)) {
    cerr << "*** Using local data dir. Not running a distribution. dir=" << dir << endl;
    sprintf( rootDir, "%s%s", dir, DATA_DIR_NAME );
    cerr << "\trootDir=" << rootDir << endl;
  }

  // config check
  if(argc >= 2 && !strcmp(argv[1], "--test-config")) {
    cerr << "Configuration:" << endl;
    char dir[300];
    char file[500];
    int dir_res = get_config_dir_name( dir, 300 );
    int file_res = get_config_file_name( file, 500 );
    cerr << "starting app: " << argv[0] << endl;
    cerr << "rootDir=" << rootDir <<
      "\nconfigDir=" << configDir <<
      "\nconfigFile=" << CONFIG_FILE <<
      "\ndir=" << dir << " dir_res=" << dir_res <<
      "\nfile=" << file << " file_res=" << file_res <<	endl;
    return 0;
  }

  // do a final sanity check before running the game
  if( !checkFile( rootDir, "/textures/test.txt" ) ) {
    cerr << "ERROR: check for files failed in data dir: " << rootDir << endl;
    cerr << "Either install the data files at the above location, or rebuild with ./configure --with-data-dir=<new location> or run the game from the source distribution's main directory (the one that contains src,data,etc.)" << endl;
    return 1;
  }

	findLocaleDir();

  cerr << "Starting session. Final rootDir=" << rootDir << endl;

	return 0;
}

bool Constants::checkFile(const char *dir, const char *file) {
  char path[300];
  strcpy(path, dir);
  strcat(path, file);
  //fprintf(stderr, "\tchecking path: %s\n", path);
  bool ret = true;
  FILE *fp = fopen(path, "rb");
  if(!fp || ferror(fp)) ret = false;
  if(fp) fclose(fp);
  return ret;
}

// this function is used to be able to run scourge while developing
void Constants::findLocalResources(const char *appPath, char *dir) {

  //cerr << "&^&^&^&^ appPath=" << appPath << endl;

  char testFile[80];
  sprintf( testFile, "%s/textures/test.txt", DATA_DIR_NAME );
  // Where are we running from?
  strcpy(dir, appPath);

	// append an ending / so the current dir is also considered
	int n = strlen( dir );
	if( !( dir[ n - 1 ] == '/' || dir[ n - 1 ] == SEPARATOR ) ) {
		strcat( dir, "/" );
	}

  // Look in this and the parent dir for a 'data' folder
  // ('i' has to count to at least 4 for OS X)
  for(int i = 0; i < 10; i++) {
    char *pp = strrchr(dir, '/');
    char *p = strrchr(dir, SEPARATOR);
    if(!p && !pp) {
      dir[0] = '\0';
      cerr << "*** Can't find local version of data dir. You're running a distribution." << endl;
      return;
    }
    // Take whichever comes first. This is to solve a problem when running in
    // mingw or cygwin. It may cause problems if the actual path has a \ or / in it.
    if(pp > p) p = pp;
    *(p + 1) = 0;
    //cerr << "*** Looking at: dir=" << dir << endl;
    if( checkFile( dir, testFile ) ) return;
    // remove the last separator
    *(p) = 0;
  }
  dir[0] = '\0';
}

char* GetDataPath(char *file)
{
	static char path[300];
	sprintf(path, file, rootDir);
	return path;
}

void Constants::getQuadrantAndAngle( float nx, float ny, int *q, float *angle ) {
  if( nx == 0 ) *angle = ( ny <= 0 ? ( 90.0f + 180.0f ) : 90.0f );
  else *angle = Constants::toAngle( atan( ny / nx ) );
  //cerr << "x=" << nx << " y=" << ny << " angle=" << (*angle) << endl;

  // read about the arctan problem: 
  // http://hyperphysics.phy-astr.gsu.edu/hbase/ttrig.html#c3
  *q = 1;
  if( nx < 0 ) {     // Quadrant 2 & 3
    *q = ( ny >= 0 ? 2 : 3 );
    (*angle) += 180;
  } else if( ny < 0 ) { // Quadrant 4
    *q = 4;
    (*angle) += 360;
  }
	//cerr << "\tfinal angle=" << (*angle) << " quadrant=" << (*q) << endl;
}
