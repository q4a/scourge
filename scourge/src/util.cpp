/***************************************************************************
                          util.cpp  -  description
                             -------------------
    begin                : Sun Jun 22 2003
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

#include "util.h"
#include "render/renderlib.h"
#include "creature.h"
#include "sdlhandler.h"
#include "debug.h"

using namespace std;

#define MAX_CLOSED_NODES 120
#define FAST_MAX_CLOSED_NODES 50

Util::Util(){
}
Util::~Util(){
}

void Util::rotate(Sint16 x, Sint16 y, Sint16 *px, Sint16 *py, float angle) {
    // convert to radians
    angle = degreesToRadians(angle);
    // rotate
    float oldx = (float)(x);
    float oldy = (float)(y);
    *px = (Sint16)rint((oldx * cos(angle)) - (oldy * sin(angle)));
    *py = (Sint16)rint((oldx * sin(angle)) + (oldy * cos(angle)));
}

// A Sample by D.S.Reynolds
//
// How It Works:
//
// The basic formula for the A* algorithm is f = g + h.  g represents the cost of
// distance travelled (or gone).  h represents the estimate (or heuristic) of distance
// from current node to the destination.  Some sort of contianers are needed.  Some
// chose stacks, I chose STL vectors and a heap.  I believe the most critical part
// of the formula is the calculation for the heuristic.  Also, Breadth-First and Depth-First
// algorithms are simplified subsets of the A*.
//
//  1.  Create OPEN, CLOSED and PATH containers.
//  2.  Start with the OPEN container containing only the starting node.
//      Set that nodes g value to 0, it's h value to the euclidian difference
//      ((dx - sx)*(dx - sx)) + ((dy - sy)*(dy - sy)), calc the f value and
//      set the parent node to NULL.
//  3.  Until the goal node is found, repeat the following:
//        If no nodes found exit with failure.
//        Select Node on OPEN with the lowest f value.  This is the BestNode.
//        Push BestNode to Closed (This is the only place CLOSED is populated).
//        If BestNode is the destination, exit.  Otherwise, determine the
//        child nodes (adjacents) of BestNode.
//           For each child node do the following:
//           Set each child nodes Parent to BestNode (We'll use this later to
//             get the path.)
//           Calc the cost of g:  Child.g = BestNode.g + 1  (Use other than 1
//             for various terrain)
//           See if child node is already on OPEN.  If so, determine which has
//             the lower g value and use it's other corresponding values.
//           See if child node is already on CLOSED.  If so, determine which
//             has the lower g value and use it's other corresponding values.
//           If the child node was NOT on OPEN or CLOSED, add it to OPEN.
//
///////////////////////////////////////////////////////////
void Util::findPath( Sint16 sx, Sint16 sy, Sint16 sz,
                     Sint16 dx, Sint16 dy, Sint16 dz,
                     vector<Location> *pVector,
                     Map *map,
                     Creature *creature,
										 Creature *player,
                     int maxNodes,
                     bool ignoreParty,
										 bool ignoreEndShape ) {

	if( PATH_DEBUG ) 
		cerr << "Util::findPath for " << creature->getName() << 
		" maxNodes=" << maxNodes << " ignoreParty=" << ignoreParty << 
		" ignoreEndShape=" << ignoreEndShape <<
		endl;

  vector<CPathNode> OPEN;                 // STL Vectors chosen because of rapid
  vector<CPathNode> CLOSED;               // insertions/deletions at back,
  vector<CPathNode> PATH;                 // and Direct access to any element.
  CPathNode Node, BestNode;               // Temporary Node and BestNode
  bool bNodeFound = false;                // Flag if node is found in container


  Node.x = sx;                            // Create the start node
  Node.y = sy;
  Node.gone = 0;
  Node.heuristic = ((dx - sx)*(dx - sx)) + ((dy - sy)*(dy - sy));
  Node.f = Node.gone + Node.heuristic;    // The classic formula f = g + h
  Node.px = 0;                            // No parent for start location
  Node.py = 0;                            // No parent for start location
  OPEN.push_back(Node);                   // Populate the OPEN container with the first location
  make_heap( OPEN.begin(), OPEN.end() );  // Create a heap from OPEN for sorting


  while (!OPEN.empty()) {
    sort_heap(OPEN.begin(), OPEN.end());// Ascending sort based on overloaded operators below
    BestNode = OPEN.front();            // Set the Node with lowest f value to BESTNODE
    pop_heap(OPEN.begin(), OPEN.end()); // Pop off the heap.  Actually this moves the
                                        // far left value to the far right.  The node
                                        // is not actually removed since the pop is to
                                        // the heap and not the container.
    OPEN.pop_back();                    // Remove node from right (the value we pop_heap'd)
    CLOSED.push_back(BestNode);         // Push the BestNode onto CLOSED

    // If at destination, break and create path below
    if ((BestNode.x == dx) && (BestNode.y == dy)) {
      //bPathFound = true; // arrived at destination...
      break;
    }

    // Set limit to break if looking too long
    if ( (int)CLOSED.size() > maxNodes )
      break;

    // Check adjacent locations (This is done in a clockwise order to lessen jaggies)
    for (int i=1; i<9; i++) {
      switch(i) {
      case 1:
        Node.x = BestNode.x;
        Node.y = BestNode.y - 1;
        break;
      case 2:
        Node.x = BestNode.x + 1;
        Node.y = BestNode.y - 1;
        break;
      case 3:
        Node.x = BestNode.x + 1;
        Node.y = BestNode.y;
        break;
      case 4:
        Node.x = BestNode.x + 1;
        Node.y = BestNode.y + 1;
        break;
      case 5:
        Node.x = BestNode.x;
        Node.y = BestNode.y + 1;
        break;
      case 6:
        Node.x = BestNode.x - 1;
        Node.y = BestNode.y + 1;
        break;
      case 7:
        Node.x = BestNode.x - 1;
        Node.y = BestNode.y;
        break;
      case 8:
        Node.x = BestNode.x - 1;
        Node.y = BestNode.y - 1;
        break;
      }

      if ((Node.x >= 0) && (Node.x < MAP_WIDTH) &&
          (Node.y >= 0) && (Node.y < MAP_DEPTH)) {
        
        // Determine cost of distance travelled
        if( isBlocked( Node.x, Node.y, sx, sy, dx, dy, creature, player, map, ignoreParty, ignoreEndShape ) ) {
          Node.gone = 1000;
        } else {
          Node.gone = BestNode.gone + 1;
        }
        
        // Determine the Heuristic.  Probably the most crucial aspect
        // Heuristic by Simple Euclidian method
        // Node.heuristic = ((dx - Node.x)*(dx - Node.x)) + ((dy - Node.y)*(dy - Node.y));
        // Heuristic by my own Orthogonal/Diagonal + Euclidian modifier
        int DX = abs(dx - Node.x);
        int DY = abs(dy - Node.y);
        int Orthogonal = abs(DX - DY);
        int Diagonal = abs(((DX + DY) - Orthogonal)/2);
        Node.heuristic = Diagonal + Orthogonal + DX + DY;
        //Node.heuristic = (int)( sqrt( DX * DX + DY * DY ) );

        // The A* formula
        Node.f = Node.gone + Node.heuristic;
        Node.px = BestNode.x; // Point parent to last BestNode (pushed onto CLOSED)
        Node.py = BestNode.y; // Point parent to last BestNode (pushed onto CLOSED)


        bNodeFound = false;

        // Check to see if already on OPEN
        for (int t=0; t<(int)OPEN.size(); t++) {
          if ((Node.x == OPEN[t].x) &&
              (Node.y == OPEN[t].y)) {   // If already on OPEN
            if (Node.gone < OPEN[t].gone) {
              OPEN[t].gone = Node.gone;
              OPEN[t].f = Node.gone + OPEN[t].heuristic;
              OPEN[t].px = Node.px;
              OPEN[t].py = Node.py;
            }
            bNodeFound = true;
            break;
          }
        }
        if (!bNodeFound ) { // If Node NOT found on OPEN
          // Check to see if already on CLOSED
          for (int t=0; t<(int)CLOSED.size(); t++) {
            if ((Node.x == CLOSED[t].x) &&
                (Node.y == CLOSED[t].y)) {   // If on CLOSED, Which has lower gone?
              if (Node.gone < CLOSED[t].gone) {
                CLOSED[t].gone = Node.gone;
                CLOSED[t].f = Node.gone + CLOSED[t].heuristic;
                CLOSED[t].px = Node.px;
                CLOSED[t].py = Node.py;
              }
              bNodeFound = true;
              break;
            }
          }
        }
        if (!bNodeFound ) { // If Node NOT found on OPEN or CLOSED
          OPEN.push_back(Node);                  // Push NewNode onto OPEN
          push_heap( OPEN.begin(), OPEN.end() ); // Push NewNode onto heap
          make_heap( OPEN.begin(), OPEN.end() ); // Re-Assert heap, or will be short by one

                   /*
                   // Display OPEN and CLOSED containers (For Debugging)
                   int i;
                   cout << "OPEN:   ";
                   for (i=0; i<OPEN.size(); i++)
                   {
                       cout << OPEN[i].x << "," << OPEN[i].y << ",";
                       cout << OPEN[i].gone << "," << OPEN[i].heuristic << "  ";
                   }
                   cout << endl;
                   cout << "CLOSED:   ";
                   for (i=0; i<CLOSED.size(); i++)
                   {
                       cout << CLOSED[i].x << "," << CLOSED[i].y << ",";
                       cout << CLOSED[i].gone << "," << CLOSED[i].heuristic << "  ";
                   }
                   cout << endl << endl;
                   int ch = _getch();
                   //*/
        }
      }
    }
  }

  if (CLOSED.size() > 0) {
    // Create the path from elements of the CLOSED container
    if(PATH.size()) PATH.erase(PATH.begin(), PATH.end());
    PATH.push_back(CLOSED.back());
    CLOSED.pop_back();

    while (!CLOSED.empty()) {
      if ((CLOSED.back().x == PATH.back().px) &&
          (CLOSED.back().y == PATH.back().py))
        PATH.push_back(CLOSED.back());

      CLOSED.pop_back();
    }

    // Populate the vector that was passed in by reference
    Location Fix;
    if(pVector->size()) pVector->erase(pVector->begin(), pVector->end());
    for (int i=(PATH.size()-1); i>=0; i--) {
      //for (container::iterator i=PATH.begin(); i!= PATH.end(); ++i)
      Fix.x = PATH[i].x;
      Fix.y = PATH[i].y;
      Fix.z = 0;
      pVector->push_back(Fix);
    }
  }
}

// a simpler/quicker 2D version of Map::isBlocked()
bool Util::isBlocked( Sint16 x, Sint16 y,
                      Sint16 shapeX, Sint16 shapeY, 
											Sint16 dx, Sint16 dy,
                      Creature *creature, 
											Creature *player,
											Map *map,
                      bool ignoreParty,
											bool ignoreEndShape ) {
  for( int sx = 0; sx < creature->getShape()->getWidth(); sx++ ) {
    for( int sy = 0; sy < creature->getShape()->getDepth(); sy++ ) {
      Location *loc = map->getLocation( x + sx, y - sy, 0 );
			if( loc ) {
				if( ignoreEndShape && 
						loc->shape &&
						loc->x <= dx && loc->x + loc->shape->getWidth() >= dx &&
						loc->y >= dy && loc->y - loc->shape->getDepth() <= dy ) {
					if( PATH_DEBUG ) {
						cerr << "*** ignoreEndShape allowed." << endl;
					}
					//continue;
					return false;
				}
				if( ignoreParty && 
						loc->creature && 
						loc->creature != player &&
						( !loc->creature->isMonster() ||
							loc->creature->isNpc() ) ) continue;
				if( !( ( loc->creature && 
								 ( loc->creature == creature || 
									 loc->creature == creature->getTargetCreature() ) ) ||
							 ( loc->shape && loc->x == shapeX && loc->y == shapeY ) ) ) {
					return true;
				}
			}
    }
  }
  return false;
}

// is the a's final position out of the way of b's current location?
bool Util::isOutOfTheWay( Creature *a, vector<Location> *aPath, int aStart,
														 Creature *b, vector<Location> *bPath, int bStart ) {
	if( !( aPath && aStart < (int)aPath->size() ) ||
			!( bPath && bStart < (int)bPath->size() ) ) return false;
	Location aLast = (*aPath)[ aPath->size() - 1 ];
	Location bCurrent = (*bPath)[ bStart ];
	return 
		SDLHandler::intersects( aLast.x - a->getShape()->getWidth() / 2, 
														aLast.y - a->getShape()->getDepth() / 2, 
														a->getShape()->getWidth(), 
														a->getShape()->getDepth(),
														bCurrent.x - b->getShape()->getWidth() / 2, 
														bCurrent.y - b->getShape()->getDepth() / 2, 
														b->getShape()->getWidth(), 
														b->getShape()->getDepth() );
}

///////////////////////////////////////////////////////////
// Needed because the template doesn't know what a PathNode is
//bool operator<(const CPathNode &a, const CPathNode &b) {
//  return a.f < b.f;
//}


///////////////////////////////////////////////////////////
// Needed because the template doesn't know what a PathNode is
//bool operator>(const CPathNode &a, const CPathNode &b) {
//  return a.f > b.f;
//}

float Util::dot_product(float v1[3], float v2[3]) {
  return (v1[0] * v2[0] + v1[1] * v2[1] + v1[2] * v2[2]);
}

void Util::normalize(float v[3]) {
  float f = 1.0f / sqrt(dot_product(v, v));
  
  v[0] *= f;
  v[1] *= f;
  v[2] *= f;
}

void Util::cross_product(const float *v1, const float *v2, float *out) {
  out[0] = v1[1] * v2[2] - v1[2] * v2[1];
  out[1] = v1[2] * v2[0] - v1[0] * v2[2];
  out[2] = v1[0] * v2[1] - v1[1] * v2[0];
}

void Util::multiply_vector_by_matrix(const float m[9], float v[3]) {
  float tmp[3];
  
  tmp[0] = v[0] * m[0] + v[1] * m[3] + v[2] * m[6];
  tmp[1] = v[0] * m[1] + v[1] * m[4] + v[2] * m[7];
  tmp[2] = v[0] * m[2] + v[1] * m[5] + v[2] * m[8];
  
  v[0] = tmp[0];
  v[1] = tmp[1];
  v[2] = tmp[2];
}

void Util::multiply_vector_by_matrix2(const float m[16], float v[4]) {
  float tmp[4];
  
  tmp[0] = v[0] * m[0] + v[1] * m[4] + v[2] * m[8] + v[3] * m[12];
  tmp[1] = v[0] * m[1] + v[1] * m[5] + v[2] * m[9] + v[3] * m[13];
  tmp[2] = v[0] * m[2] + v[1] * m[6] + v[2] * m[10] + v[3] * m[14];
  tmp[3] = v[0] * m[3] + v[1] * m[7] + v[2] * m[11] + v[3] * m[15];

  
  v[0] = tmp[0];
  v[1] = tmp[1];
  v[2] = tmp[2];
  v[3] = tmp[3];
}


// Return a string containing the last OpenGL error.
// Useful to debug strange OpenGL behaviors
char * Util :: getOpenGLError(){
    int error;
    error = glGetError();
    
    // All openGl errors possible
    switch(error){
        case GL_NO_ERROR : return "GL_NO_ERROR";break;
        case GL_INVALID_ENUM : return "GL_INVALID_ENUM"; break;
        case GL_INVALID_VALUE : return "GL_INVALID_VALUE"; break;
        case GL_INVALID_OPERATION : return "GL_INVALID_OPERATION"; break;
        case GL_STACK_OVERFLOW : return "GL_STACK_OVERFLOW"; break;
        case GL_OUT_OF_MEMORY : return "GL_OUT_OF_MEMORY"; break;
        default : return "Unknown error"; break;
    }
}

// Returns next word from the given position. If there is not a space at the given
// position, the function suppose it is the first letter of the word wanted. 
string Util::getNextWord(const string theInput, int fromPos, int &endWord){
    int firstChar, lastStringChar;
    string sub;
	if(sub.size()) sub.erase(sub.begin(), sub.end());
    
    if (theInput.empty() || fromPos==-1) {return sub;}

    lastStringChar = theInput.find_last_not_of(' ');    
    if(theInput[fromPos] == ' '){
        firstChar = theInput.find_first_not_of(' ', fromPos);
    }
    else{
        firstChar = fromPos;
    }    
    endWord = theInput.find_first_of(' ', firstChar);
   // cout << "line :" << theInput << endl;
   // cout << "\t\tpos = " << fromPos << " firstChar = " << firstChar << " endWord = " << endWord << " lastStringChar =" << lastStringChar << endl; 
    if(endWord == -1){
        if( (lastStringChar >= firstChar)&&(firstChar!=-1)){
            sub = theInput.substr(firstChar, lastStringChar - firstChar + 1);
        }                    
    } 
    else{        
        sub = theInput.substr(firstChar, endWord - firstChar);
    }
    return sub;

}

float Util::getAngle(float fx, float fy, float fw, float fd,
					 float tx, float ty, float tw, float td) {
  // figure out targetCreatureAngle
  float sx = fx + (fw / 2);
  float sy = fy - (fd / 2);
  float ex = tx + (tw / 2);
  float ey = ty - (td / 2);

  float x = ex - sx;
  float y = ey - sy;
  if( x == 0.0f ) x = 0.001f;
  float angle = Constants::toAngle(atan(y / x));

  // read about the arctan problem: 
  // http://hyperphysics.phy-astr.gsu.edu/hbase/ttrig.html#c3
  //  q = 1;
  if(x < 0) { 		// Quadrant 2 & 3
	//	q = ( y >= 0 ? 2 : 3);
	angle += 180;
  } else if(y < 0) { // Quadrant 4
	//	q = 4;
	angle += 360;
  }

  // normalize
  if( angle < 0.0f ) angle = 360.0f + angle;
  if( angle >= 360.0f ) angle -= 360.0f;

  return angle;
}

#define FOV_ANGLE 60

/**
 * Is px,py in the field of vision defined by x,y,angle?
 */
bool Util::isInFOV( float x, float y, float angle, float px, float py ) {
	float angleToP = getAngle( x, y, 1, 1, 
														 px, py, 1, 1 );
	//cerr << "fov: angle=" << angle << " toP=" << angleToP << endl;
	float diff = fabs( diffAngle( angle, angleToP ) );
	bool b = ( diff < FOV_ANGLE );
	//cerr << "\tb=" << b << " diff=" << diff << endl;
	return b;
}

float Util::diffAngle(float a, float b) {
//  a -= (((int)a / 360) * 360);
//  b -= (((int)b / 360) * 360);
  float diff = a - b;
  if( diff > 180.0f ) {
    diff = -(360.0f - diff);
  } else if( diff < -180.0f ) {
    diff = 360 + diff;
  }
  return diff;
}

void Util::drawBar( int x, int y, float barLength, float value, float maxValue,
                    float red, float green, float blue, float gradient, GuiTheme *theme,
                    int layout ) {
  float percent = (maxValue == 0 ? 0 : (value >= maxValue ? 100.0f : value / (maxValue / 100.0f)));
  float length = barLength * (percent / 100.0f);
  if(length < 0) {
    length = percent = 0;
  }

  glPushMatrix();
  glTranslatef( x, y, 0 );

  /*
  if( theme && theme->getInputBackground() ) {
    glColor4f( theme->getInputBackground()->color.r,
               theme->getInputBackground()->color.g,
               theme->getInputBackground()->color.b,
               theme->getInputBackground()->color.a );
  } else {
    glColor3f( 0.8f, 0.5f, 0.2f );
  }
  glBegin( GL_QUADS );
  if( layout == HORIZONTAL_LAYOUT ) {
    glVertex3f( barLength + 1, -4, 0 );
    glVertex3f( -1, -4, 0 );
    glVertex3f( -1, 4, 0 );
    glVertex3f( barLength + 1, 4, 0 );
  } else {
    glVertex3f( -4, barLength + 1, 0 );
    glVertex3f( -4, -1, 0 );
    glVertex3f( 4, -1, 0 );
    glVertex3f( 4, barLength + 1, 0 );
  }
  glEnd();
  */

  glLineWidth(6.0f);

  //  glColor3f( 0.2f, 0.2f, 0.2f );
  if( theme && theme->getWindowBorder() ) {
    glColor4f( theme->getWindowBorder()->color.r,
               theme->getWindowBorder()->color.g,
               theme->getWindowBorder()->color.b,
               theme->getWindowBorder()->color.a );
  } else {
    //glColor3f( 0.8f, 0.5f, 0.2f );
    glColor3f( 0, 0, 0 );
  }
  //glColor3f( 1, 0.75f, 0.45f );
  glBegin( GL_LINES );
  if( layout == HORIZONTAL_LAYOUT ) {
    glVertex3f( 0, 0, 0 );
    glVertex3f( barLength, 0, 0 );
  } else {
    glVertex3f( 0, 0, 0 );
    glVertex3f( 0, barLength, 0 );
  }
  glEnd();

  // default args so I don't have to recompile .h file
  if(red == -1) {
    red = 0.5f;
    green = 1.0f;
    blue = 0.5f;
  }
  if(!gradient || percent > 40.0f) {  
    glColor3f( red, green, blue );
  } else if(percent > 25.0f) {
    glColor3f( 1.0f, 1.0f, 0.5f );
  } else {
    glColor3f( 1.0f, 0.5f, 0.5f );
  }
  glBegin( GL_LINES );
  if( layout == HORIZONTAL_LAYOUT ) {
    glVertex3f( 0, 0, 0 );
    glVertex3f( length, 0, 0 );
  } else {
    glVertex3f( 0, barLength - length, 0 );
    glVertex3f( 0, barLength, 0 );
  }
  glEnd();

  glLineWidth(1.0f);
  /*
  if(percent > 0.0f && percent < 100.0f) {
    if( theme && theme->getWindowBorder() ) {
      glColor4f( theme->getWindowBorder()->color.r,
                 theme->getWindowBorder()->color.g,
                 theme->getWindowBorder()->color.b,
                 theme->getWindowBorder()->color.a );
    } else {
      glColor3f( 0.8f, 0.5f, 0.2f );
    }
    glBegin( GL_LINES );
    if( layout == HORIZONTAL_LAYOUT ) {
      glVertex3f( length, -4, 0 );
      glVertex3f( length, 4, 0 );
    } else {
      glVertex3f( -4, length, 0 );
      glVertex3f( 4, length, 0 );
    }
    glEnd();
  }
  */
  glPopMatrix();
}

float Util::getRandomSum( float base, int count, float div ) {
  float sum = 0;
  float third = base / div;
  for( int i = 0; i < ( count < 1 ? 1 : count ); i++ ) {
    sum += ( ( third * rand()/RAND_MAX ) + ( base - third ) );
  }
  return sum;
}

char *Util::toLowerCase( char *s ) {
  char *p = s;
  while( *p ) {
    if( *p >= 'A' && *p <= 'Z' ) *p = *p - 'A' + 'a';
    p++;
  }
  return s;
}

// FIXME: take into account, existing |-s in text
char *Util::addLineBreaks( const char *in, char *out, int lineLength ) {
	strcpy( out, "" );
	char tmp[3000];
	strcpy( tmp, in );
	char *token = strtok( tmp, " \r\n\t" );
	int count = 0;
	while( token ) {
		if( count + strlen( token ) >= lineLength ) {
			strcat( out, "|" );
			count = 0;
		}
		strcat( out, token );
		strcat( out, " " );
		count += strlen( token ) + 1;
		token = strtok( NULL, " \r\n\t" );
	}
	return out;
}
