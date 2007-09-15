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
 * Get a path away from given locations.
 * This assumes some external force has taken over the creature. So their path will be overwritten and the selected
 * x and y location will be changed.
 **/
void PathManager::findPathOffLocations(set<Location,LocationComparitor>* locations, Creature* player, Map* map, int maxNodes){
  GoalFunction * goal = new ClearLocationsGoal(owner,locations);
  Heuristic * heuristic = new NoHeuristic();
  AStar::findPath(toint(owner->getX()),toint(owner->getY()),&path,map,owner,player,maxNodes,false,goal,heuristic);
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
  //first add our current location. We need this incase of 0 length path.
  int x = toint(owner->getX());
  int y = toint(owner->getY());
  Location loc;
  loc.z = 0;
  for(int k = 0; k < owner->getShape()->getWidth(); k++)
    for(int m = 0; m < owner->getShape()->getDepth(); m++){
      loc.x = x+k;
      loc.y = y+m;
      allPathLocations.insert(loc);
    }

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
  //check every location we will occupy
  set<Location>::iterator setItr = allPathLocations.begin();
  while(setItr != allPathLocations.end()){
    Location loc = *setItr;
    Location* mapLoc = map->getLocation(loc.x,loc.y,0);
    if(mapLoc && mapLoc->creature && mapLoc->creature != owner && 
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
  if(this->moveState == MOVE_STATE_CLEARING_PATH)
    return FAST_SPEED;
  return owner->getSpeed(); //the creature should walk as fast as it is walking.
}

void PathManager::incrementPositionOnPath(){
  positionOnPath++;
  if(atEndOfPath()) moveState = MOVE_STATE_NORMAL;
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
  positionOnPath = 0;
}
int PathManager::getPathRemainingSize(){
  return path.size() - positionOnPath -1;
}

/*
void FormationLeaderPathManager::getFormationPosition( Sint16 *px, Sint16 *py, Sint16 *pz, int x, int y ) {
  Sint16 dx = layout[formation][index][0];
  Sint16 dy = -layout[formation][index][1];

  // get the angle
  float angle = 0;
  if(next->getDir() == Constants::MOVE_RIGHT) angle = 270.0;
  else if(next->getDir() == Constants::MOVE_DOWN) angle = 180.0;
  else if(next->getDir() == Constants::MOVE_LEFT) angle = 90.0;

  // rotate points
  if(angle != 0) { 
    Util::rotate(dx, dy, px, py, angle);
  } else {
    *px = dx;
    *py = dy;
  }

  // translate
  if( x > -1 ) {
    *px = (*(px) * getShape()->getWidth()) + x;
    *py = (-(*(py)) * getShape()->getDepth()) + y;
  } else if(next->getSelX() > -1) {
    *px = (*(px) * getShape()->getWidth()) + next->getSelX();
    *py = (-(*(py)) * getShape()->getDepth()) + next->getSelY();
  } else {
    *px = (*(px) * getShape()->getWidth()) + toint(next->getX());
    *py = (-(*(py)) * getShape()->getDepth()) + toint(next->getY());
  }
  *pz = toint(next->getZ());
}*/
