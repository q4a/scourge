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

///////////////////
//     Goals
///////////////////
GoalFunction::GoalFunction(){}
GoalFunction::~GoalFunction(){}

/**
 * The basic "get to this node" goal.
 **/
SingleNodeGoal::SingleNodeGoal(int x, int y){
  this->x = x;
  this->y = y;
}
SingleNodeGoal::~SingleNodeGoal(){}

bool SingleNodeGoal::fulfilledBy( CPathNode * node){
  return (node->x == x) && (node->y == y);
}

/**
 * Get within a given range of the target creature.
 **/
GetCloseGoal::GetCloseGoal(Creature* searcher, Creature* target, float distance){
  this->searcher = searcher; 
  this->target = target;
  this->distance = distance;
}
GetCloseGoal::~GetCloseGoal(){}

bool GetCloseGoal::fulfilledBy( CPathNode * node){
  return Constants::distance(node->x,node->y,
                             searcher->getShape()->getWidth(),searcher->getShape()->getDepth(),
                             target->getX(),target->getY(),
                             target->getShape()->getWidth(),target->getShape()->getDepth())  < distance;
}

/**
 * Get a certain distance from a location.
 **/
GetAwayGoal::GetAwayGoal(int x, int y, float distance){
  this->x = x; 
  this->y = y;
  this->distance = distance;
}
GetAwayGoal::~GetAwayGoal(){}

bool GetAwayGoal::fulfilledBy( CPathNode * node){
  int dx = node->x - x;
  int dy = node->y - y;
  return distance < sqrt((float)(dx*dx + dy*dy));
}

/**
 * Get within a given range of the target creature.
 **/
ClearLocationsGoal::ClearLocationsGoal(Creature* searcher, set<Location,LocationComparitor>* locations){
  this->searcher = searcher; 
  this->locations = locations;
}
ClearLocationsGoal::~ClearLocationsGoal(){}

bool ClearLocationsGoal::fulfilledBy( CPathNode * node){
  int x = node->x;
  int y = node->y;
  int w = toint(searcher->getShape()->getWidth());
  int d = toint(searcher->getShape()->getDepth());
  Location loc;
  loc.z = 0;
  set<Location,LocationComparitor>::iterator setItr;
  //check every occupied location against the set of locations
  for(int i = 0; i < w; i++)
    for(int j = 0; j < d; j++){
      loc.x = x+i;
      loc.y = y-j;
      if((setItr = locations->find(loc)) != locations->end())
        return false;
    }
  return true;
}


/**
 * Aim for the x,y coordinates of the creature, but any
 * node that it occupies can be the goal.
 **/
SingleCreatureGoal::SingleCreatureGoal(Creature* target){
  this->target = target;
}
SingleCreatureGoal::~SingleCreatureGoal(){}

bool SingleCreatureGoal::fulfilledBy( CPathNode * node){
  int cx = toint(target->getX());
  int cy = toint(target->getY());
  return node->x >= cx && node->y <= cy &&
         node->x < cx + target->getShape()->getWidth() &&
         node->y > cy - target->getShape()->getDepth();
}

///////////////////
//   Heuristics
///////////////////

Heuristic::Heuristic(){}
Heuristic::~Heuristic(){}

/**
 * Standard "moving toward a single location" heuristic.
 * This one assumes that diagonal moves are allowed.
 **/
DiagonalDistanceHeuristic::DiagonalDistanceHeuristic(int x, int y){
  this->x = x;
  this->y = y;
}
DiagonalDistanceHeuristic::~DiagonalDistanceHeuristic(){}

double DiagonalDistanceHeuristic::heuristic( CPathNode * node){
  return max(abs(node->x - x),abs(node->y - y));
}

/**
 * Negative distance from the location, so that it expects the goal to be anywhere 
 * *away* from there.
 * This is still a valid heuristic as it always underestimates the distance left.
 **/
DistanceAwayHeuristic::DistanceAwayHeuristic(int x, int y){
  this->x = x;
  this->y = y;
}
DistanceAwayHeuristic::~DistanceAwayHeuristic(){}

double DistanceAwayHeuristic::heuristic( CPathNode * node){
  int dx = node->x - x;
  int dy = node->y - y;
  return -sqrt((float)(dx*dx + dy*dy))*1.001;  //the extra 0.001 is a tweak to make farther away paths get checked first
}


/**
 * A "no distance left to goal" heuristic.
 * This results in a Dijkstra (for us, essentially breadth-first) search.
 **/
NoHeuristic::NoHeuristic(){}
NoHeuristic::~NoHeuristic(){}

double NoHeuristic::heuristic( CPathNode * node){
  return 0;
}

///////////////////
//     Search
///////////////////

AStar::AStar(){
}

AStar::~AStar(){
}

/**
 * Dijkstra's search to find the shortest path to the goal.
 * This is applicable when there are multiple goals that cannot be
 * assumed to lie near a single target location.
 *
 * For example: moving away from a creature, getting off another path,
 *  moving to the nearest opponent etc.
 *
 * Dijkstra's algorithm is essentially a breadth-first search for weighted
 * graphs. In this case we use this because blocked nodes cost more than 
 * unblocked nodes. It will also allow terrain costs to be added in the
 * future.
 **/
void AStar::findPathToNearest( Sint16 sx, Sint16 sy, 
                      vector<Location> *pVector,
                      Map *map,
                      Creature *creature,
                      Creature *player,
                      int maxNodes,
                      bool ignoreParty,
                      GoalFunction * goal ) {
  Heuristic * heuristic = new NoHeuristic();
  AStar::findPath(sx,sy,pVector,map,creature,player,maxNodes,ignoreParty,goal,heuristic);
  delete heuristic;
  
}

/**
 * An A* implementation by Jonathan Teutenberg
 *
 * An A* search is a form of best-first search, where the "goodness" of a node is determined
 * by f = g + h.  
 * Here g is the length of the path taken so far, and h, the heuristic, is an
 * estimate of the length of the rest of the path to the goal.
 * For A* to guarantee a shortest path to the goal and to remain n.log(n) complexity
 * the heuristic must never overestimate the remaining distance to the goal. 
 *
 * Data-structures:
 * The priority queue for the open nodes (yet to be checked).
 * The set for the closed list - nodes we have already checked and so don't need to see again.
 * Membership checks are logarithmic for the set, but we would prefer constant, as in a hashtable. 
 *
 * Also included is a (non-standard for A*) set for checking whether nodes are already on the open list.
 * Because we have to pop all the nodes off open at the end when we need to delete them, we expect
 * 2.n.log(n) operations (pushing and popping every child node we come across). 
 * The set now gets checked at every child insert at log(n) cost, and worst case (where no child node ever
 * appears twice) we can now expect 4.n.log(n) operations. 
 * However, in most cases there are 3+ children that are already in the open list, in which case the extra 
 * membership checks pay for themselves. Also, since open list is smaller, we are dealing with a smaller "n"  
 * and overall it should make a speed up. Whew.
 *
 * The goal function:
 * The goal function is used to specify when the result is good enough and should be returned. The standard 
 * goal is to reach the target location, but other possibilities are to get within a certain distance of
 * the location or to find a place that has a clear shot to the target.
 **/
void AStar::findPath( Sint16 sx, Sint16 sy,
                      vector<Location> *pVector,
                      Map *map,
                      Creature *creature,
                      Creature *player,
                      int maxNodes,
                      bool ignoreParty,
                      GoalFunction * goal,
                      Heuristic * heuristic ) {

  //if( PATH_DEBUG ) 
//		cerr << "Util::findPath for " << creature->getName() << 
//		" maxNodes=" << maxNodes << " ignoreParty=" << ignoreParty << 
//		endl;

  if(sx < 0 || sy < 0 || sx > MAP_WIDTH || sy > MAP_DEPTH){ //we'll waste heaps of time trying to find these
    if(pVector->size()) pVector->clear(); // just clear the old path
    return; 
  }
  priority_queue<CPathNode*, vector<CPathNode*>, FValueNodeComparitor> open;
  set<CPathNode*,XYNodeComparitor> closed;   
  vector<CPathNode> path;

  set<CPathNode*,XYNodeComparitor> openContents; //to check for membership of the open queue
  set<CPathNode*,XYNodeComparitor>::iterator setItr; //iterator to be used with closed when we want to delete nodes from memory
  int closedSize = 0; //STL Set can take O(n) to determine size, so we keep a record ourselves
  
  CPathNode * start = new CPathNode();     // Has to be persistent, so we put it on the heap.
  start->x = sx;                         // Create the start node
  start->y = sy;
  start->gone = 0;
  start->heuristic = heuristic->heuristic(start);
  start->f = start->gone + start->heuristic;    // The classic formula f = g + h
  start->parent = NULL;  // no parent for the start location
  open.push(start);            // Populate the OPEN container with the first location.
  openContents.insert(start);

  CPathNode* bestNode;             //best node = lowest f-value
  CPathNode* closestNode = start;  //closest node = lowest heuristic value

 //cout << "starting search from " << sx << "," << sy << " to " << dx << "," << dy << "\n";

  while (!open.empty()) {
    bestNode = open.top();     // Set the Node with lowest f value to BESTNODE
    open.pop();    

    // Check to see if already in the closed set
    if((setItr = closed.find(bestNode)) != closed.end()){
      continue;
    }

    // If at destination, break and create path below
    if (goal->fulfilledBy(bestNode)) {
      //cout << "finished by finding goal\n";
      closestNode = bestNode; 
      closed.insert(bestNode); //add the goal to closed to ensure it gets deleted later
      break;//build the path from closestNode back to the start
    }

    if(bestNode->heuristic < closestNode->heuristic)
      closestNode = bestNode;
    closed.insert(bestNode);         // Push the BestNode onto CLOSED
    closedSize++;
    // Set limit to break if looking too long
    if ( closedSize > maxNodes ){
      if( PATH_DEBUG ) 
        cout << "maxed out on nodes by going past " << maxNodes << "\n";
      break;
    }
    
    int x = -1;
    int y = -1;
    
    for (int i=-1; i < 2; i++){
      for(int j = -1; j < 2; j++){
        if(i == 0 && j == 0) continue;

        x = bestNode->x + i;
        y = bestNode->y + j;

        if ((x < 0) || (x >= MAP_WIDTH) || (y < 0) || (y >= MAP_DEPTH)) continue;
        
        CPathNode * next = new CPathNode();
        next->x = x;
        next->y = y;

        // Check to see if already in the open queue by checking openContents
        if((setItr = openContents.find(next)) != openContents.end()){
          delete next;
          continue;
        }

        // Determine cost of distance travelled
        if( isBlocked( x, y, creature, player, map, ignoreParty ) )
          next->gone = 1000;
        else 
          next->gone = bestNode->gone + 1;
        
        
        //heuristic calculation
        double h = heuristic->heuristic(next);

        if(abs(i+j) == 1) //a non-diagonal move
          next->heuristic = h;
        else
          next->heuristic = h+0.5; //we discourage diagonals to remove zig-zagging

        next->f = next->gone + next->heuristic;
        next->parent = bestNode;
        open.push(next);           // Insert into the open queue
        openContents.insert(next);
        
      }
    }
  }

  if (!closed.empty()) {
    // Create the path from elements of the CLOSED container
    if(path.size()) path.erase(path.begin(), path.end());
    //if( PATH_DEBUG ) 
     // cout << "Final node " << closestNode->x << "," << closestNode->y << " fulfils goal? " << goal->fulfilledBy(closestNode) <<"\n";
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
      Fix.x = path[i].x;
      Fix.y = path[i].y;
      Fix.z = 0;
      pVector->push_back(Fix);
    }
  }
}

// a simpler/quicker 2D version of Map::isBlocked()
bool AStar::isBlocked( Sint16 x, Sint16 y,
                      Creature *creature, 
                      Creature *player,
                      Map *map,
                      bool ignoreParty) {
  for( int sx = 0; sx < creature->getShape()->getWidth(); sx++ ) {
    for( int sy = 0; sy < creature->getShape()->getDepth(); sy++ ) {

      if( fabs( map->getGroundHeight( ( x + sx ) / OUTDOORS_STEP, ( y - sy ) / OUTDOORS_STEP ) ) > 10.0f ) {
        return true;
      }

      Location *loc = map->getLocation( x + sx, y - sy, 0 );
      if( loc ) {
        //if we are ignoring our party and there is a party member there, no problem.
        if( ignoreParty && 
          loc->creature && 
          loc->creature != player &&
          ( !loc->creature->isMonster() ||
          loc->creature->isNpc() ) ) continue;
        //if there is a creature there and it is us or our target creature then we are ok
        if(loc->creature && 
             ( loc->creature == creature || 
               loc->creature == creature->getTargetCreature() )) continue;

        //if there is some other shape there then we are blocked by it
        if( loc->shape ) {
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

