/***************************************************************************
                          pathmanager.h  -  description
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

#ifndef PATHMANAGER_H
#define PATHMANAGER_H

#include <stdio.h>
#include <string.h>
#include <vector> 
#include <set> 
#include "render/renderedcreature.h"
#include "render/location.h"
#include "astar.h"

class Map;
class Location;
class Creature;
class GLShape;
class FormationFollowerPathManager;

/**
 * A PathManager keeps a path, the current location on the path and the speed
 * to move down the path.
 * This class should manage all calls to search algorithms, requests for creatures to
 * get out of the way and formations, so they are abstracted away from creatures and parties.
 **/
class PathManager{
  protected:
    Creature* owner;
    std::vector<Location> path;
    std::set<Location,LocationComparitor> allPathLocations;

    unsigned int positionOnPath; //how far down the path this creature is
    int moveState;

    void calculateAllPathLocations();
  public:
    PathManager(Creature* owner);
    virtual ~PathManager();

    inline std::vector<Location>* getPath(){return &path;}
    inline int getPositionOnPath(){return positionOnPath;}

    virtual bool findPath(int x, int y, Creature* player, Map* map, bool ignoreParty=false, int maxNodes=100);
    virtual bool findPathToCreature(Creature* target, Creature* player, Map* map, float distance=MIN_DISTANCE, bool ignoreParty=false, int maxNodes=100);
    virtual void findPathAway(int awayX, int awayY, Creature* player, Map* map, float distance, bool ignoreParty=false, int maxNodes=100);
    void findWanderingPath(int maxPathLength, Creature* player, Map* map);
    void findPathOffLocations(std::set<Location,LocationComparitor>* locations, Creature* player, Map* map, int maxNodes);
    
    void moveNPCsOffPath(Creature* player, Map* map); //runs up the path, asking any stationary NPCs to clear off
    virtual int getSpeed();

    void incrementPositionOnPath();
    Location getNextStepOnPath();
    bool atEndOfPath();
    bool atStartOfPath();
    void clearPath();
    int getPathRemainingSize();

    Location getEndOfPath();

    bool isBlockingPath(Creature* blocker);
    bool isBlockingPath(Creature* blocker, int x, int y);
    bool isPathToTargetCreature();
    bool isPathTowardTargetCreature(float range);
   
    

};

/**
 * This PathManager includes a direction faced at each path location. This can be used
 * to calculate where followers should stand so as to be in formation with this leader.
 **/
class FormationLeaderPathManager : PathManager{
  protected:
    std::vector<float> directions; //the direction faced at each location on the path (in degrees)
    FormationFollowerPathManager* followers; //so we can update our followers when we make a new path
    int followersSize; //number of followers we have
    int formation;

    void calculateDirections();
  public:
    FormationLeaderPathManager(Creature* owner, FormationFollowerPathManager* followers, int followersSize);
    virtual ~FormationLeaderPathManager();

    virtual bool findPath(int x, int y, Creature* player, Map* map, bool ignoreParty=false, int maxNodes=100);
    virtual bool findPathToCreature(Creature* target, Creature* player, Map* map, float distance=MIN_DISTANCE, bool ignoreParty=false, int maxNodes=100);
    virtual void findPathAway(int awayX, int awayY, Creature* player, Map* map, float distance, bool ignoreParty=false, int maxNodes=100);

    /**
     * Get the position of this creature in the formation, given the leaders location and facing.
     */
    void getFormationPosition( Sint16 *px, Sint16 *py, int formation, int formationIndex, int x, int y, float angle );

    inline void setOwner(Creature* owner){this->owner = owner;} // so that formations can change the leader
    inline void setFollowersSize(int size){followersSize = size;}
};

/**
 *
 */
class FormationFollowerPathManager : PathManager{
  protected:
    FormationLeaderPathManager* leader;
  public:
    FormationFollowerPathManager(Creature* owner,FormationLeaderPathManager* leader);
    ~FormationFollowerPathManager();
    virtual int getSpeed(); //speed changes depending on how far behind or ahead of the leader we are
  
    inline void setOwner(Creature* owner){this->owner = owner;}
};
#endif
