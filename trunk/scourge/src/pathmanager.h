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
#include "render/renderedcreature.h"

class Map;
class Location;
class Creature;
class GLShape;

/**
 * A PathManager keeps a path, the current location on the path and the speed
 * to move down the path.
 * This class should manage all calls to search algorithms, requests for creatures to
 * get out of the way and formations, so they are abstracted away from creatures and parties.
 **/
class PathManager{
  private:
    Creature* owner;
    std::vector<Location> path;
    unsigned int positionOnPath; //how far down the path this creature is
  public:
    PathManager(Creature* owner);
    virtual ~PathManager();

    inline std::vector<Location>* getPath(){return &path;}
    inline int getPositionOnPath(){return positionOnPath;}

    virtual bool findPath(int x, int y, Creature* player, Map* map, bool ignoreParty=false, int maxNodes=100);
    virtual bool findPathToCreature(Creature* target, Creature* player, Map* map, float distance=MIN_DISTANCE, bool ignoreParty=false, int maxNodes=100);
    virtual void findPathAway(int awayX, int awayY, Creature* player, Map* map, float distance, bool ignoreParty=false, int maxNodes=100);
    virtual bool findPathAway(Creature* other, Creature* player, Map* map, bool tryHard, float distance, bool ignoreParty=false, int maxNodes=100);
    virtual int getSpeed();

    void incrementPositionOnPath();
    Location getNextStepOnPath();
    bool atEndOfPath();
    bool atStartOfPath();
    void clearPath();
    int getPathRemainingSize();

    Location getEndOfPath();

    bool isPathToTargetCreature();
    bool isPathTowardTargetCreature(float range);

};

/**
 * This PathManager includes a direction faced at each path location. This can be used
 * to calculate where followers should stand so as to be in formation with this leader.
 **
class FormationLeaderPathManager : PathManager{
  private:
    std::vector<int> directions; //the direction faced at each location on the path
    FormationFollowerPathManager* followers; //so we can update our followers when we make a new path
    int followersSize; //number of followers we have
    int formation;
  public:
    FormationLeaderPathManager(Creature* owner, FormationFollowerPathManager* followers, int followersSize);
    ~FormationLeaderPathManager();

    virtual bool findPath(int x, int y, Creature* player, Map map, bool ignoreParty=false, int maxNodes=100);
    virtual bool findPathToCreature(Creature* target, Creature* player, Map map, float distance=MIN_DISTANCE, bool ignoreParty=false, int maxNodes=100);

    **
     * Get the position of this creature in the formation.
     * returns -1,-1 if the position cannot be set (if the person followed is not moving)
     *
    void getFormationPosition( Sint16 *px, Sint16 *py, Sint16 *pz, int x=-1, int y=-1 );

    void setOwner(Creature* owner); // so that formations can change the leader
};
**
 *
 *
class FormationFollowerPathManager : PathManager{
  private:
    FormationLeaderPathManager* leader;
  public:
    FormationFollowerPathManager(FormationLeaderPathManager* leader);
    ~FormationFollowerPathManager();
    virtual int getSpeed(); //speed changes depending on how far behind or ahead of the leader we are
};*/
#endif
