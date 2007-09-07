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
#include "render/glshape.h"

using namespace std;

#define FAST_SPEED 1
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

PathManager::PathManager(Creature* owner){
  this->owner = owner;
  this->positionOnPath = 0;
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
}
/**
 * Get a path away from a creature.
 **/
bool PathManager::findPathAway(Creature* other, Creature* player, Map* map, bool tryHard, float distance, bool ignoreParty, int maxNodes){
  findPathAway(toint(other->getX() + other->getShape()->getWidth()/2.0f),toint(other->getY() + other->getShape()->getDepth()/2.0f),
               player,map,distance,ignoreParty,maxNodes);
 
  bool pathFound = false;
  int minIntersect = 0;

  if( !path.empty() && 
      !AStar::isOutOfTheWay( owner, &path, 0, player, 
                             player->getPathManager()->getPath(), player->getPathManager()->getPositionOnPath() + 1 ) ) {
    int intersectCount = 0;
    for( int i = 0; i < owner->getSession()->getCreatureCount(); i++ ) {
         Creature *c = owner->getSession()->getCreature( i ); 
      if( c != owner && c->isNpc() && 
          AStar::isOutOfTheWay( owner, &path, 0, 
                                c, c->getPathManager()->getPath(), c->getPathManager()->getPositionOnPath() + 1 ) ) {
        intersectCount++;
      }
    }
								
    for( int i = 0; i < owner->getSession()->getParty()->getPartySize(); i++ ) {
         Creature *c = owner->getSession()->getParty()->getParty( i );
      if( tryHard ) {
        if( c != owner && c != player && 
            !AStar::isOutOfTheWay( owner, &path, 0, 
                                   c, c->getPathManager()->getPath(), c->getPathManager()->getPositionOnPath() + 1 ) ) {
          c->moveAway( other );
        }
      }
      else {
        if( c != owner && 
            AStar::isOutOfTheWay( owner, &path, 0, 
                                  c, c->getPathManager()->getPath(), c->getPathManager()->getPositionOnPath() + 1 ) ) {
          intersectCount++;
        }
      }
    }		
    // if this puts a pc out of range; that's bad
    if( !owner->isMonster() ) {
      float dist = owner->getDistance( owner->getSession()->getParty()->getPlayer() );
      if( dist >= CLOSE_DISTANCE ) {
          intersectCount += 2;
      }
    }

    if( minIntersect == 0 || intersectCount < minIntersect ) {
      //owner->setSpeed(FAST_SPEED);
     // owner->selX = path[ path.size() - 1 ].x;
     // owner->selY = path[ path.size() - 1 ].y;
     owner->setSelXY( path[ path.size() - 1 ].x, path[ path.size() - 1 ].y); //TODO: this plans another path which is unnecessary
     //clearPath();
     // for( int i = 0; i < (int)path.size(); i++ ) path.push_back( path[ i ] );
      //pathSelected = true;
      minIntersect = intersectCount;
    //  if( minIntersect == 0 ) pathSelected = true;
      pathFound = true;

    }
  }
  return pathFound;
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
  return owner->getSpeed(); //the creature should walk as fast as it is walking.
}

void PathManager::incrementPositionOnPath(){
  positionOnPath++;
}
bool PathManager::atEndOfPath(){
  return positionOnPath >= path.size()-1;
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
