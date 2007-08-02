/***************************************************************************
                          astar.cpp  -  description
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

#define MAX_CLOSED_NODES 180
#define FAST_MAX_CLOSED_NODES 80

AStar::AStar(){
}

AStar::~AStar(){
}

// An A* implementation by Jonathan Teutenberg
//
// An A* search is a form of best-first search, where the "goodness" of a node is determined
// by f = g + h.  
// Where g is the length of the path taken so far, and h, the heuristic, is an
// estimate of the length of the rest of the path to the goal.
// For A* to guarantee a shortest path to the goal and to remain n.log(n) complexity
// the heuristic must never overestimate the remaining distance to the goal. 
//
// Data-structures:
// The priority queue for the open nodes (yet to be checked).
// The set for the closed list - nodes we have already checked and so don't need to see again.
// Membership checks are logarithmic for the set, but we would prefer constant, as in a hashtable. 
//
// Also included is a (non-standard for A*) set for checking whether nodes are already on the open list.
// Because we have to pop all the nodes off open at the end when we need to delete them, we expect
// 2.n.log(n) operations (pushing and popping every child node we come across). 
// The set now gets checked at every child insert at log(n) cost, and worst case (where no nodes are ever
// checked twice) we can now expect 4.n.log(n) operations. 
// However, in most cases there are 3+ children already in the open list, in which case the extra membership
// checks cost no more. Also, since open list is smaller, we are dealing with a smaller "n" and overall it 
// makes a speed up. Phew.
//
// Because these STL containers copy on insert, we cannot store CPathNodes directly,
// otherwise the parent pointer links between nodes would become invalid.
//
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

  if(dx < 0 || sy < 0 || sx > MAP_WIDTH || sy > MAP_DEPTH){ //we'll waste heaps of time trying to find these
    if(pVector->size()) pVector->erase(pVector->begin(), pVector->end()); // just clear the old path
    return; 
  }
  priority_queue<CPathNode*, vector<CPathNode*>, FValueNodeComparitor> open;
  set<CPathNode*,XYNodeComparitor> closed;   
  vector<CPathNode> path;

  set<CPathNode*,XYNodeComparitor> openContents; //to check for membership of the open queue
  set<CPathNode*>::iterator setItr; //iterator to be used with closed when we want to delete nodes from memory
  int closedSize = 0; //STL Set can take O(n) to determine size, so we keep a record ourselves
  
  CPathNode * start = new CPathNode();     // Has to be persistent, so we put it on the heap.
  (*start).x = sx;                         // Create the start node
  (*start).y = sy;
  (*start).gone = 0;
  (*start).heuristic = max(abs(dx-sx),abs(dy-sy));
  (*start).f = (*start).gone + (*start).heuristic;    // The classic formula f = g + h
  (*start).parent = NULL;  // no parent for the start location
  open.push(start);            // Populate the OPEN container with the first location.
  openContents.insert(start);

  CPathNode* bestNode;             //best node = lowest f-value
  CPathNode* closestNode = start;  //closest node = lowest heuristic value

  //cout << "starting search from " << sx << "," << sy << " to " << dx << "," << dy << "\n";

  while (!open.empty()) {
    bestNode = open.top();     // Set the Node with lowest f value to BESTNODE
    open.pop();    

    // If at destination, break and create path below
    if (((*bestNode).x == dx) && ((*bestNode).y == dy)) {
      closestNode = bestNode; 
      closed.insert(bestNode); //add the goal to closed to ensure it gets deleted later
      break;//build the path from closestNode back to the start
    }

    // Check to see if already in the closed set
    if((setItr = closed.find(bestNode)) != closed.end()){
      continue;
    }

    if((*bestNode).heuristic < (*closestNode).heuristic)
      closestNode = bestNode;
    closed.insert(bestNode);         // Push the BestNode onto CLOSED
    closedSize++;
    // Set limit to break if looking too long
    if ( closedSize > maxNodes ){
	break;
    }
    int x = -1;
    int y = -1;
    double f,g,h;
    
    for (int i=1; i<9; i++) {
      switch(i) {
      case 1:
        x = (*bestNode).x;
        y = (*bestNode).y - 1;
        break;
      case 2:
        x = (*bestNode).x + 1;
        y = (*bestNode).y - 1;
        break;
      case 3:
        x = (*bestNode).x + 1;
        y = (*bestNode).y;
        break;
      case 4:
        x = (*bestNode).x + 1;
        y = (*bestNode).y + 1;
        break;
      case 5:
        x = (*bestNode).x;
        y = (*bestNode).y + 1;
        break;
      case 6:
        x = (*bestNode).x - 1;
        y = (*bestNode).y + 1;
        break;
      case 7:
        x = (*bestNode).x - 1;
        y = (*bestNode).y;
        break;
      case 8:
        x = (*bestNode).x - 1;
        y = (*bestNode).y - 1;
        break;
      }

      if ((x >= 0) && (x < MAP_WIDTH) &&
          (y >= 0) && (y < MAP_DEPTH)) {
        
        // Determine cost of distance travelled
        if( isBlocked( x, y, sx, sy, dx, dy, creature, player, map, ignoreParty, ignoreEndShape ) )
          g = 1000;
        else 
          g = (*bestNode).gone + 1;
        
        
        //heuristic calculation
        int DX = abs(dx - x);
        int DY = abs(dy - y);

	if(i % 2 == 0) //a non-diagonal move
		h = max(DX,DY)+0.5; //we discourage diagonals to remove zig-zagging
	else
		h = max(DX,DY);

        // The A* formula
        f = g+h;
        
	CPathNode * next = new CPathNode();
	(*next).x = x;
	(*next).y = y;
  	(*next).heuristic = h;
	(*next).gone = g;
	(*next).f = f;
	(*next).parent = bestNode;

        // Check to see if already in the open queue by checking openContents
        if((setItr = openContents.find(next)) != openContents.end()){
          delete next;
          continue;
        }

        open.push(next);           // Insert into the open queue
        openContents.insert(next);
        
      }
    }
  }

  if (!closed.empty()) {
    // Create the path from elements of the CLOSED container
    if(path.size()) path.erase(path.begin(), path.end());

    CPathNode nextNode = *closestNode;
    path.push_back(nextNode);
    CPathNode * parent = nextNode.parent;
    while(parent != NULL){
      nextNode = *parent;
      parent = nextNode.parent;
      path.push_back(nextNode);
    }

    for (setItr = closed.begin(); setItr != closed.end(); ++setItr){
      bestNode = *setItr;
      delete bestNode;
    }
    closed.erase(closed.begin(), closed.end());
    
    //and delete any left over nodes in OPEN
    while(!open.empty()){
        CPathNode* next = open.top();
	open.pop();         
        delete next;
    }
    //I'm unsure whether this is required to free up the memory. Better safe than sorry though.
    openContents.erase(openContents.begin(), openContents.end()); 
	
    // Populate the vector that was passed in by reference
    Location Fix;
    if(pVector->size()) pVector->erase(pVector->begin(), pVector->end());
    for (int i=(path.size()-1); i>=0; i--) {
      //for (container::iterator i=PATH.begin(); i!= PATH.end(); ++i)
      Fix.x = path[i].x;
      Fix.y = path[i].y;
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

