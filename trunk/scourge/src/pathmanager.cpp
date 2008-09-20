/***************************************************************************
                   pathmanager.cpp  -  Manages creature paths
                             -------------------
    begin                : Thu Sep 6 2007
    copyright            : (C) 2007 by Jonathan Teutenberg
    email                : jono@cs.auckland.ac.nz
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
#include "pathmanager.h"
#include "creature.h"
#include "session.h"
#include "common/constants.h"
#include "render/location.h"
#include "render/map.h"
#include "render/glshape.h"

using namespace std;

#define FAST_SPEED 1
#define LOITER_SPEED 9.0f
// how close to stay to the player
#define CLOSE_DISTANCE 8

/**
 * Formations are defined by 4 set of coordinates in 2d space.
 * These starting positions assume dir=Constants::MOVE_UP
 */
static const Sint16 layout[][4][2] = {
  { {0, 0}, {-1, 1}, {1, 1}, {0, 2}},  // DIAMOND_FORMATION
  { {0, 0}, {-1, 1}, {0, 1}, {-1, 2}},  // STAGGERED_FORMATION
  { {0, 0}, {1, 0}, {1, 1}, {0, 1}},   // SQUARE_FORMATION
  { {0, 0}, {0, 1}, {0, 2}, {0, 3}},   // ROW_FORMATION
  { {0, 0}, {-2, 2}, {0, 2}, {2, 2}},  // SCOUT_FORMATION
  { {0, 0}, {-1, 1}, {1, 1}, {0, 3}}   // CROSS_FORMATION
};

/**
 * The probabilities of turning in each direction when making a
 * wandering path. Defined clockwise from current direction.
 **/
static const float turnProbabilities[8] = {0.7f,0.1f,0.03f,0.015f,0.01f,0.015f,0.03f,0.1f};

PathManager::PathManager(Creature* owner){
  this->owner = owner;
  positionOnPath = 0;
  loiterSpeed = LOITER_SPEED;
}
PathManager::~PathManager(){

}

bool PathManager::findPath(int x, int y, Creature* player, Map* map, bool ignoreParty){
  long maxNodes = map->getPreferences()->getMaxPathNodes();

  GoalFunction * goal = new SingleNodeGoal(x,y);
  Heuristic * heuristic = new DiagonalDistanceHeuristic(x,y);
  AStar::findPath(toint(owner->getX()),toint(owner->getY()),&path,map,owner,player,maxNodes,ignoreParty,goal,heuristic);
  delete heuristic;
  delete goal;
  
  positionOnPath = 0;

  calculateAllPathLocations();
  moveNPCsOffPath(player,map);
  if( path.empty() ) return x == toint(owner->getX()) && y == toint(owner->getY());
  Location last = path[path.size()-1];
  return last.x == x && last.y == y;
}

/**
 * Move within a certain distance of the target creature
 **/
bool PathManager::findPathToCreature(Creature* target, Creature* player, Map* map, float distance, bool ignoreParty){
  long maxNodes = map->getPreferences()->getMaxPathNodes();

  GoalFunction * goal = new GetCloseGoal(owner,target,distance);
  Heuristic * heuristic = new DiagonalDistanceHeuristic(toint(target->getX() + target->getShape()->getWidth()/2.0f),toint(target->getY() - target->getShape()->getDepth()/2.0f));
  AStar::findPath(toint(owner->getX()),toint(owner->getY()),&path,map,owner,player,maxNodes,ignoreParty,goal,heuristic);
  delete heuristic;
  delete goal;

  positionOnPath = 0;

  calculateAllPathLocations();
  moveNPCsOffPath(player,map);

  return isPathTowardTargetCreature(distance);
}

/**
 * Get a path away from a given location.
 **/
void PathManager::findPathAway(int awayX, int awayY, Creature* player, Map* map, float distance, bool ignoreParty){
  long maxNodes = map->getPreferences()->getMaxPathNodes();

  GoalFunction * goal = new GetAwayGoal(awayX,awayY,distance);
  Heuristic * heuristic = new DistanceAwayHeuristic(awayX,awayY);
  AStar::findPath(toint(owner->getX()),toint(owner->getY()),&path,map,owner,player,maxNodes,ignoreParty,goal,heuristic);
  delete heuristic;
  delete goal;
 
  positionOnPath = 0;
  
  calculateAllPathLocations();
}

/**
 * Generates a somewhat random path using a series of getNextLocation()
 * It will try to make a path that is maxPathLength long, but will stop if it gets stuck.
 **/
void PathManager::findWanderingPath(unsigned int maxPathLength, Creature* player, Map* map){
  clearPath();
  //start with our facing direction
  Uint16 direction = owner->getFacingDirection();
  int x = toint(owner->getX());
  int y = toint(owner->getY());
  while(path.size() < maxPathLength){
    direction = addNextLocation(x,y,direction,player,map);
    if(direction == 0)
      break;
    x = path[path.size()-1].x;
    y = path[path.size()-1].y;
  }
  //we are wandering, so set the loitering speed to between 8.5 and 9.5
  loiterSpeed = LOITER_SPEED + Util::roll( -0.5f, 0.5f );
}

/**
 * Adds a location to path.
 * To be used for building random walks. Tries turning in all directions if blocked.
 * Returns the new facing direction if a location was added, otherwise returns 0
 **/
Uint16 PathManager::addNextLocation(int cx, int cy, Uint16 direction, Creature* player, Map* map){
  
  //first run through each direction seeing which are possible
  Uint16 canTurn = 0;
  Uint16 dir = direction;
  Uint16 mask = 1;
  float totalProbability = 0;
  int x,y;
  for(Uint16 i = 0; i < 8; i++){
    x = nextX(cx,dir);
    y = nextY(cy,dir);
    if(!AStar::isBlocked(x,y,owner,player,map,false)){
      canTurn = canTurn | mask;
      totalProbability += turnProbabilities[i];
    }
    dir = getClockwiseDirection(dir);
    mask *= 2;
  }

  //do we have any to chose from?
  if(canTurn == 0)
    return 0; // we are blocked in

  //now we randomly choose one
  float random = Util::roll( 0.0f, totalProbability );
  mask = 1;
  for(Uint16 i = 0; i < 8; i++){
    if((canTurn & mask) != 0){ //can use this direction, check if we chose it
      random -= turnProbabilities[i];
      if(random <= 0){ //our choice!
        //turn to the direction
        for(int j = 0; j < i; j++)
          direction = getClockwiseDirection(direction);
        //create the new location
        Location next;
        next.x = nextX(cx,direction);
        next.y = nextY(cy,direction);
        //add it to the path and return
        path.push_back(next);
        return direction;
      }
    }
    //either can't turn, or have chosen not to. Continue
    mask *= 2;
  }

  //we shouldn't get here, but assume we are blocked
  //cerr << "Overflowed random direction generation.\n";
  return 0;
}

int PathManager::nextX(int x, Uint16 direction){
  if((direction & Constants::MOVE_RIGHT) > 0) return x+1;
  if((direction & Constants::MOVE_LEFT) > 0) return x-1;
  return x;
}

int PathManager::nextY(int y, Uint16 direction){
  if((direction & Constants::MOVE_UP) > 0) return y-1; //hopefully this is the right direction
  if((direction & Constants::MOVE_DOWN) > 0) return y+1;
  return y;
}

Uint16 PathManager::getClockwiseDirection(Uint16 direction){
  //for reference: the eight directions clockwise from north are 1,9,8,10,2,6,4,5
  if(direction == Constants::MOVE_UP) return Constants::MOVE_UP_RIGHT;
  if(direction == (Constants::MOVE_UP_RIGHT)) return Constants::MOVE_RIGHT;
  if(direction == Constants::MOVE_RIGHT) return Constants::MOVE_DOWN_RIGHT;
  if(direction == (Constants::MOVE_DOWN_RIGHT)) return Constants::MOVE_DOWN;
  if(direction == Constants::MOVE_DOWN) return Constants::MOVE_DOWN_LEFT;
  if(direction == Constants::MOVE_DOWN_LEFT) return Constants::MOVE_LEFT;
  if(direction == Constants::MOVE_LEFT) return Constants::MOVE_UP_LEFT;
  if(direction == Constants::MOVE_UP_LEFT) return Constants::MOVE_UP;
  return 0;
}
Uint16 PathManager::getAntiClockwiseDirection(Uint16 direction){
  if(direction == Constants::MOVE_UP) return Constants::MOVE_UP_LEFT;
  if(direction == (Constants::MOVE_UP_RIGHT)) return Constants::MOVE_UP;
  if(direction == Constants::MOVE_RIGHT) return Constants::MOVE_UP_RIGHT;
  if(direction == (Constants::MOVE_DOWN_RIGHT)) return Constants::MOVE_RIGHT;
  if(direction == Constants::MOVE_DOWN) return Constants::MOVE_DOWN_RIGHT;
  if(direction == Constants::MOVE_DOWN_LEFT) return Constants::MOVE_DOWN;
  if(direction == Constants::MOVE_LEFT) return Constants::MOVE_DOWN_LEFT;
  if(direction == Constants::MOVE_UP_LEFT) return Constants::MOVE_LEFT;
  return 0;
}

/**
 * Get a path away from given locations.
 * This assumes some external force has taken over the creature. So their path will be overwritten and the selected
 * x and y location will be changed.
 * This will also tell the creature to set its motion to MOTION_CLEAR_PATH, which will in turn make it move fast.
 **/
void PathManager::findPathOffLocations(set<Location,LocationComparitor>* locations, Creature* player, Map* map){
  long maxNodes = map->getPreferences()->getMaxPathNodes();

  GoalFunction * goal = new ClearLocationsGoal(owner,locations);
  Heuristic * heuristic = new NoHeuristic();
  AStar::findPath(toint(owner->getX()),toint(owner->getY()),&path,map,owner,player,maxNodes,false,goal,heuristic);
  delete heuristic;
  delete goal;
 
  positionOnPath = 0;
  owner->setMotion(Constants::MOTION_CLEAR_PATH);

  if( !path.empty() ) owner->setSelXYNoPath(path[path.size()-1].x,path[path.size()-1].y);
  calculateAllPathLocations();
  //cout << owner->getName() << " moving out of the way. Path length " << path.size() << "\n";
  //now merge our path with the locations provided and get NPCs off both? Beware infinite loops
}

void PathManager::calculateAllPathLocations(){
  allPathLocations.clear();

  int x;
  int y;
  Location loc;
  loc.z = 0;
  //now do the rest of the path. Duplicates are thrown out by the set
  for(unsigned int j = positionOnPath; j < path.size(); j++){
    x = path[j].x;
    y = path[j].y;
    //add every location our body would fill
    for(int k = 0; k < owner->getShape()->getWidth(); k++)
      for(int m = 0; m < owner->getShape()->getDepth(); m++){
          loc.x = x+k;
          loc.y = y-m;
          allPathLocations.insert(loc);
      }
  }
}

void PathManager::moveNPCsOffPath(Creature* player, Map* map){
  calculateAllPathLocations();
  //check every location we will occupy
  set<Location,LocationComparitor>::iterator setItr = allPathLocations.begin();
  while(setItr != allPathLocations.end()){
    Location loc = *setItr;
    Location* mapLoc = map->getLocation(loc.x,loc.y,0);
    if(mapLoc && mapLoc->creature && mapLoc->creature != owner && 
       mapLoc->creature != player &&
       (!mapLoc->creature->isMonster() || mapLoc->creature->isNpc())){
      Creature* blocker = (Creature*)mapLoc->creature;
#if PATH_DEBUG      
      cout << blocker->getName() << " is in the way.\n";
#endif      
      blocker->setMotion(Constants::MOTION_CLEAR_PATH); //make sure monsters do the move too

      bool inTheWay = blocker->getPathManager()->atEndOfPath();
      if(!inTheWay){
        //now check to make sure that their path does actually get out of the way
        Location end = blocker->getPathManager()->getEndOfPath();
        inTheWay = isBlockingPath(blocker,end.x,end.y);
      }
      //it's some other NPC who isn't moving! Get OUT of the WAY!
      if(inTheWay) {
        blocker->getPathManager()->findPathOffLocations(&allPathLocations,player,map);
#if PATH_DEBUG        
        cout << "blocker is getting out of the way, current: " << toint(blocker->getX()) << "," << toint(blocker->getY()) << " target: " << blocker->getPathManager()->getEndOfPath().x << "," << blocker->getPathManager()->getEndOfPath().y << "\n";  
#endif      
      }
    }
    setItr++;
  }
}

bool PathManager::isBlockingPath(Creature* blocker){
  return isBlockingPath(blocker,toint(blocker->getX()),toint(blocker->getY()));
}

/**
 * Needs to be updated to include an estimated time range that the potential blocker will be at x and y.
 **/
bool PathManager::isBlockingPath(Creature* blocker, int x, int y){
  set<Location,LocationComparitor>::iterator containsItr;
  Location loc;
  loc.z = 0;
  for(int i = 0; i < blocker->getShape()->getWidth(); i++)
    for(int j = 0; j < blocker->getShape()->getDepth(); j++){
      loc.x = x+i;
      loc.y = y-j;
      if((containsItr = allPathLocations.find(loc)) != allPathLocations.end()){
        return true;
      }
    }
  return false;
}

// does the path end in the target creature
bool PathManager::isPathToTargetCreature() {
  if( path.size() <= 0 || !owner->getTargetCreature() ) return false;
  Location pos = path[ path.size() - 1 ];
  Creature* target = owner->getTargetCreature();
  int tx = toint(target->getX());
  int ty = toint(target->getY());
  return pos.x >= tx && pos.y > ty - toint(target->getShape()->getDepth()) && 
         pos.x < tx + toint(target->getShape()->getWidth()) &&
         pos.y <= ty;
}

// does the path get close enough to the target creature
bool PathManager::isPathTowardTargetCreature(float range) {
  if(!owner->getTargetCreature() ){ 
    return false;
  }
  if(path.size() == 0){
    return owner->getDistance(owner->getTargetCreature()) < range;
  }
  Location pos = path[ path.size() - 1 ];
  return Constants::distance(pos.x, pos.y, owner->getShape()->getWidth(),owner->getShape()->getDepth(),
                             owner->getTargetCreature()->getX(),owner->getTargetCreature()->getY(),
                             owner->getTargetCreature()->getShape()->getWidth(),owner->getTargetCreature()->getShape()->getDepth()) < range;
}

float PathManager::getSpeed(){
  if(owner->getMotion() == Constants::MOTION_CLEAR_PATH){
    return FAST_SPEED;
  }
  if(owner->getMotion() == Constants::MOTION_LOITER){
    return loiterSpeed;
  }
  return static_cast<float>(owner->getSpeed()); //the creature should walk as fast as it is walking.
}

void PathManager::incrementPositionOnPath(){
  positionOnPath++;
}

bool PathManager::atEndOfPath(){
  return path.empty() || positionOnPath >= path.size()-1;
}
bool PathManager::atStartOfPath(){
  return positionOnPath == 0;
}
Location PathManager::getNextStepOnPath(){
  return path[positionOnPath+1];
}
Location PathManager::getEndOfPath(){
  return path[path.size()-1];
}
void PathManager::clearPath(){
  path.clear();
  allPathLocations.clear();
  positionOnPath = 0;
}
int PathManager::getPathRemainingSize(){
  return path.size() - positionOnPath -1;
}
/**
 * Estimates the time this creature will block the given location according to
 * its current path and speed.
 * Since creature speed is directly related to (1 - speed/10), we can just use this
 * factor as "time". 
 **/
float PathManager::getEstimatedTimeAt(Location* location){
  int w = owner->getShape()->getWidth();
  int d = owner->getShape()->getHeight();
  float step = 1.0f - static_cast<float>(getSpeed()) / 10.0f;

  int lx = location->x;
  int ly = location->y;

  float time = 0;
  for(unsigned int j = positionOnPath; j < path.size(); j++){
    time += step;
    int x = path[j].x;
    int y = path[j].y;
    if(lx >= x && lx < x+w && ly >= y && ly < y+d)
      return time;
  }
  return -1; //-1 means we're never going to be at that location
}

void PathManager::writePath(){
  for(unsigned int i = 0; i < path.size(); i++)
    cout << path[i].x << "," << path[i].y << "\n";
}

///////////////////////////////////////////////////
// PathManager subclasses for handling formations
///////////////////////////////////////////////////

FormationLeaderPathManager::FormationLeaderPathManager(Creature* owner, FormationFollowerPathManager* followers, int followersSize) : PathManager(owner){;
  this->followers = followers;
  this->followersSize = followersSize;
}
FormationLeaderPathManager::~FormationLeaderPathManager(){}

void FormationLeaderPathManager::calculateDirections(){
  directions.clear();
  float angle;
  for(unsigned int i = 0; i < path.size()-1; i++){
    int dx = path[i+1].x - path[i].x;
    int dy = path[i+1].y - path[i].y;
    
    if(dx < 0) angle = (2-dy)*45; //45, 90, 135
    else if(dx > 0) angle = 360 - (2-dy)*45; //315, 270, 225
    else angle = 90 + dy*90; //0 or 180
    directions.push_back(angle);
  }
  if( !path.empty() ) 
    directions.push_back(angle); //the last direction is the same as the second to last one
}

/**
* Get the position of this creature in the formation, given the leaders location and facing.
*/
void FormationLeaderPathManager::getFormationPosition( Sint16 *px, Sint16 *py, int formation, int formationIndex, int x, int y, float angle ) {
  Sint16 dx = layout[formation][formationIndex][0];
  Sint16 dy = -layout[formation][formationIndex][1];

  // rotate points
  if(angle != 0) { 
    Util::rotate(dx, dy, px, py, angle);
  } else {
    *px = dx;
    *py = dy;
  }

  // translate
  *px = (*(px) * owner->getShape()->getWidth()) + x;
  *py = (-(*(py)) * owner->getShape()->getDepth()) + y;
  
}

bool FormationLeaderPathManager::findPath(int x, int y, Creature* player, Map* map, bool ignoreParty){
  return PathManager::findPath(x,y,player,map,ignoreParty);
}

bool FormationLeaderPathManager::findPathToCreature(Creature* target, Creature* player, Map* map, float distance, bool ignoreParty){
  return PathManager::findPathToCreature(target,player,map,distance,ignoreParty);
}

void FormationLeaderPathManager::findPathAway(int awayX, int awayY, Creature* player, Map* map, float distance, bool ignoreParty){
  return PathManager::findPathAway(awayX,awayY,player,map,distance,ignoreParty);
}

FormationFollowerPathManager::FormationFollowerPathManager(Creature* owner,FormationLeaderPathManager* leader) : PathManager(owner){
  this->leader = leader;
}

FormationFollowerPathManager::~FormationFollowerPathManager(){}

float FormationFollowerPathManager::getSpeed(){ return PathManager::getSpeed();}

