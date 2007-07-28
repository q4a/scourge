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

#include "astar.h"
#include "render/renderlib.h"
#include "creature.h"
#include "sdlhandler.h"
#include "debug.h"

using namespace std;

#define MAX_CLOSED_NODES 160
#define FAST_MAX_CLOSED_NODES 80

AStar::AStar(){
}

AStar::~AStar(){
}

// A Sample by D.S.Reynolds
// Fixed by Jonathan Teutenberg
//
// How It Works (original):
//
// The basic formula for the A* algorithm is f = g + h.  g represents the cost of
// distance travelled (or gone).  h represents the estimate (or heuristic) of distance
// from current node to the destination.  
//
// Data-structures:
// The heap used for the OPEN list is fairly standard.
// The vector for the CLOSED list is a poor choice. Membership checks are linear, but we 
// would prefer constant, as in a hashtable. 
// The vector for PATH is fine.
//
// Algorithm:
//  1.  Create OPEN, CLOSED and PATH containers.
//  2.  Start with the OPEN container containing only the starting node.
//      Set that nodes g value to 0, h value to the maximum of the x and y distance.
//  3.  Until the goal node is found, repeat the following:
//        If no nodes found exit with failure.
//        Select Node on OPEN with the lowest f value.  This is the BestNode.
//        If BestNode is the destination, exit.
//	  Check CLOSED to see whether we've processed BestNode before.
//        Push BestNode to Closed (This is the only place CLOSED is populated).
//        Determine the child nodes (adjacents) of BestNode.
//           For each child node do the following:
//           Set each child nodes Parent to BestNode (We'll use this later to
//             get the path.)
//           Calc the cost of g:  Child.g = BestNode.g + 1  (Use other than 1
//             for various terrain)
//           Calculate the h value using max(dx,dy). 
//           If this is a diagonal move, add 0.5 to the h value to avoid unnecessary zig-zagging
//           Add the child to the OPEN queue.
//
// TODO:
// Add a goal function, so we can find a path to a location "within range" of a point.
// Add a cost function to allow path planning over varying terrain (use the Map for this?)
// Fix up the parent references of nodes so we can use a hash table for CLOSED
///////////////////////////////////////////////////////////
void AStar::findPath( Sint16 sx, Sint16 sy, Sint16 sz,
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
  Node.heuristic = max(abs(dx-sx),abs(dy-sy));
  Node.f = Node.gone + Node.heuristic;    // The classic formula f = g + h
  Node.px = 0;                            // No parent for start location
  Node.py = 0;                            // No parent for start location
  OPEN.push_back(Node);                   // Populate the OPEN container with the first location
  make_heap( OPEN.begin(), OPEN.end() );  // Create a heap from OPEN for sorting


  while (!OPEN.empty()) {

    BestNode = OPEN.front();            // Set the Node with lowest f value to BESTNODE
    pop_heap(OPEN.begin(), OPEN.end()); // Pop off the heap.  Actually this moves the
                                        // far left value to the far right.  The node
                                        // is not actually removed since the pop is to
                                        // the heap and not the container.
    OPEN.pop_back();                    // Remove node from right (the value we pop_heap'd)
    
    // If at destination, break and create path below
    if ((BestNode.x == dx) && (BestNode.y == dy)) {
	CLOSED.push_back(BestNode);//add the goal to our path
      break;
    }

    // Check to see if already on CLOSED
    bNodeFound = false;
    for (int t=0; t<(int)CLOSED.size(); t++) {
            if ((BestNode.x == CLOSED[t].x) &&
                (BestNode.y == CLOSED[t].y)) {
              bNodeFound = true;
              break;
            }
     }
    if(bNodeFound){
	continue;
    }

    CLOSED.push_back(BestNode);         // Push the BestNode onto CLOSED

    // Set limit to break if looking too long
    if ( (int)CLOSED.size() > maxNodes ){
	break;
    }

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
        
        //heuristic calculation
        int DX = abs(dx - Node.x);
        int DY = abs(dy - Node.y);

	if(i % 2 == 0) //a non-diagonal move
		Node.heuristic = max(DX,DY)+0.5; //we discourage diagonals to remove zig-zagging
	else
		Node.heuristic = max(DX,DY);
	//Node.heuristic = max(DX,DY)*2 + min(DX,DY); // the old heuristic

        // The A* formula
        Node.f = Node.gone + Node.heuristic;
        Node.px = BestNode.x; // Point parent to last BestNode (pushed onto CLOSED)
        Node.py = BestNode.y; // Point parent to last BestNode (pushed onto CLOSED)

          OPEN.push_back(Node);                  // Push NewNode onto OPEN
          push_heap( OPEN.begin(), OPEN.end() ); // Push NewNode onto heap

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

  if (CLOSED.size() > 0) {
	//cout << "got path after " << CLOSED.size() << "closed nodes.\n";
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
bool AStar::isBlocked( Sint16 x, Sint16 y,
                      Sint16 shapeX, Sint16 shapeY, 
											Sint16 dx, Sint16 dy,
                      Creature *creature, 
											Creature *player,
											Map *map,
                      bool ignoreParty,
											bool ignoreEndShape ) {
  for( int sx = 0; sx < creature->getShape()->getWidth(); sx++ ) {
    for( int sy = 0; sy < creature->getShape()->getDepth(); sy++ ) {

			if( fabs( map->getGroundHeight( ( x + sx ) / OUTDOORS_STEP, ( y - sy ) / OUTDOORS_STEP ) ) > 10.0f ) {
				return true;
			}

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
bool AStar::isOutOfTheWay( Creature *a, vector<Location> *aPath, int aStart,
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

