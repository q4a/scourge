/***************************************************************************
                          pathmanager.cpp  -  description
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
// how close to stay to the player
#define CLOSE_DISTANCE 8

#define MOVE_STATE_NORMAL 0
#define MOVE_STATE_CLEARING_PATH 1
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

PathManager::PathManager(Creature* owner){
  this->owner = owner;
  positionOnPath = 0;
  moveState = MOVE_STATE_NORMAL;
}
PathManager::~PathManager(){

}

bool PathManager::findPath(int x, int y, Creature* player, Map* map, bool ignoreParty, int maxNodes){
  GoalFunction * goal = new SingleNodeGoal(x,y);
  Heuristic * heuristic = new DiagonalDistanceHeuristic(x,y);
  AStar::findPath(toint(owner->getX()),toint(owner->getY()),&path,map,owner,player,maxNodes,ignoreParty,goal,heuristic);
  delete heuristic;
  delete goal;
  
  positionOnPath = 0;
  moveState = MOVE_STATE_NORMAL;

  calculateAllPathLocations();
  moveNPCsOffPath(player,map);
  if(path.size() == 0) return x == toint(owner->getX()) && y == toint(owner->getY());
  Location last = path[path.size()-1];
  return last.x == x && last.y == y;
}

/**
 * Move within a certain distance of the target creature
 **/
bool PathManager::findPathToCreature(Creature* target, Creature* player, Map* map, float distance, bool ignoreParty, int maxNodes){
  GoalFunction * goal = new GetCloseGoal(owner,target,distance);
  Heuristic * heuristic = new DiagonalDistanceHeuristic(toint(target->getX() + target->getShape()->getWidth()/2.0f),toint(target->getY()+ target->getShape()->getDepth()/2.0f));
  AStar::findPath(toint(owner->getX()),toint(owner->getY()),&path,map,owner,player,maxNodes,ignoreParty,goal,heuristic);
  delete heuristic;
  delete goal;

  positionOnPath = 0;
  moveState = MOVE_STATE_NORMAL;

  calculateAllPathLocations();
  moveNPCsOffPath(player,map);

  return isPathTowardTargetCreature(distance);
}

/**
 * Get a path away from a given location.
 **/
void PathManager::findPathAway(int awayX, int awayY, Creature* player, Map* map, float distance, bool ignoreParty, int maxNodes){
  GoalFunction * goal = new GetAwayGoal(awayX,awayY,distance);
  Heuristic * heuristic = new DistanceAwayHeuristic(awayX,awayY);
  AStar::findPath(toint(owner->getX()),toint(owner->getY()),&path,map,owner,player,maxNodes,ignoreParty,goal,heuristic);
  delete heuristic;
  delete goal;
 
  positionOnPath = 0;
  moveState = MOVE_STATE_NORMAL;
  
  calculateAllPathLocations();
}

/**
 * Picks a random direction and heads towards it. Will occasionally deviate (fixed to 10% chance for now), and turns when blocked.
**/
void PathManager::findWanderingPath(int maxPathLength, Creature* player, Map* map){

}

/**
 * Get a path away from given locations.
 * This assumes some external force has taken over the creature. So their path will be overwritten and the selected
 * x and y location will be changed.
 **/
void PathManager::findPathOffLocations(set<Location,LocationComparitor>* locations, Creature* player, Map* map, int maxNodes){
  GoalFunction * goal = new ClearLocationsGoal(owner,locations);
  Heuristic * heuristic = new NoHeuristic();
  AStar::findPath(toint(owner->getX()),toint(owner->getY()),&path,map,owner,player,maxNodes,true,goal,heuristic);
  delete heuristic;
  delete goal;
 
  positionOnPath = 0;
  moveState = MOVE_STATE_CLEARING_PATH;
  if(path.size() > 0) owner->setSelXYNoPath(path[path.size()-1].x,path[path.size()-1].y);
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
          loc.y = y+m;
          allPathLocations.insert(loc);
      }
  }
}

void PathManager::moveNPCsOffPath(Creature* player, Map* map){
  calculateAllPathLocations();
  //check every location we will occupy
  set<Location>::iterator setItr = allPathLocations.begin();
  while(setItr != allPathLocations.end()){
    Location loc = *setItr;
    Location* mapLoc = map->getLocation(loc.x,loc.y,0);
    if(mapLoc && mapLoc->creature && mapLoc->creature != owner && 
       mapLoc->creature != player &&
       (!mapLoc->creature->isMonster() || mapLoc->creature->isNpc())){
      Creature* blocker = (Creature*)mapLoc->creature;
     // cout << blocker->getName() << " is in the way.\n";
      blocker->setMotion(Constants::MOTION_MOVE_AWAY); //make sure monsters do the move too

      bool inTheWay = blocker->getPathManager()->atEndOfPath();
      if(!inTheWay){
        //now check to make sure that their path does actually get out of the way
        Location end = blocker->getPathManager()->getEndOfPath();
        inTheWay = isBlockingPath(blocker,end.x,end.y);
      }
      //it's some other NPC who isn't moving! Get OUT of the WAY!
      if(inTheWay)
        blocker->getPathManager()->findPathOffLocations(&allPathLocations,player,map,100);
      else{
      //  cout << "blocker is getting out of the way, current: " << toint(blocker->getX()) << "," << toint(blocker->getY()) << " target: " << blocker->getPathManager()->getEndOfPath().x << "," << blocker->getPathManager()->getEndOfPath().y << "\n";  
      }
    }
    setItr++;
  }
}

bool PathManager::isBlockingPath(Creature* blocker){
  return isBlockingPath(blocker,toint(blocker->getX()),toint(blocker->getY()));
}

bool PathManager::isBlockingPath(Creature* blocker, int x, int y){
  set<Location>::iterator containsItr;
  Location loc;
  loc.z = 0;
  for(int i = 0; i < blocker->getShape()->getWidth(); i++)
    for(int j = 0; j < blocker->getShape()->getDepth(); j++){
      loc.x = x+i;
      loc.y = y+j;
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
  return pos.x >= tx && pos.y >= ty && 
         pos.x < tx + toint(target->getShape()->getWidth()) &&
         pos.y < ty + toint(target->getShape()->getDepth());
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

int PathManager::getSpeed(){
  if(this->moveState == MOVE_STATE_CLEARING_PATH){
    return FAST_SPEED;
  }
  return owner->getSpeed(); //the creature should walk as fast as it is walking.
}

void PathManager::incrementPositionOnPath(){
  positionOnPath++;
  if(atEndOfPath()){
    moveState = MOVE_STATE_NORMAL;
  }
}
bool PathManager::atEndOfPath(){
  return path.size() == 0 || positionOnPath >= path.size()-1;
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
  if(path.size() > 0) 
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

bool FormationLeaderPathManager::findPath(int x, int y, Creature* player, Map* map, bool ignoreParty, int maxNodes){
  return PathManager::findPath(x,y,player,map,ignoreParty,maxNodes);
}

bool FormationLeaderPathManager::findPathToCreature(Creature* target, Creature* player, Map* map, float distance, bool ignoreParty, int maxNodes){
  return PathManager::findPathToCreature(target,player,map,distance,ignoreParty,maxNodes);
}

void FormationLeaderPathManager::findPathAway(int awayX, int awayY, Creature* player, Map* map, float distance, bool ignoreParty, int maxNodes){
  return PathManager::findPathAway(awayX,awayY,player,map,distance,ignoreParty,maxNodes);
}

FormationFollowerPathManager::FormationFollowerPathManager(Creature* owner,FormationLeaderPathManager* leader) : PathManager(owner){
  this->leader = leader;
}

FormationFollowerPathManager::~FormationFollowerPathManager(){}

int FormationFollowerPathManager::getSpeed(){ return PathManager::getSpeed();}

