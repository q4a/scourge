/***************************************************************************
                          astar.h  -  description
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

#ifndef A_STAR_H
#define A_STAR_H

#include <math.h>
#include <stdlib.h>
#include <iostream>
#include <vector>    // STL for Vector
#include <queue> // STL for Priority Queue
#include <set> //STL for Set, Hash Set
#include <string>
#include "common/constants.h"
#include "gui/guitheme.h"
#include "render/location.h"

class Map;
class Location;
class Shape;
class Creature;

/**
  *@author Gabor Torok
  */

/**
  A path class used by A* algorithm
*/
class CPathNode{
public:
  double f, gone, heuristic; // f = gone + heuristic
  int x, y;               // Location of node
  CPathNode * parent;   //a pointer to the parent of this node  

};

//////////////////////////////////////////////////////
//    Comparison functions for sorting CPathNodes
//////////////////////////////////////////////////////

/**
 * Used to sort nodes by x and y location. Good for searching for equivalence.
 **/
struct XYNodeComparitor
{
  bool operator()(const CPathNode* a, const CPathNode* b) const
  {
    int ax = a->x;
    int bx = b->x;
    return ax < bx || (ax == bx && a->y < b->y);
  }
};

/**
 * Used to sort nodes by f value.
 * If f values are equal, then sort by h value (so we'll expand good looking
 * paths that rely on less of a guess first).
 **/
struct FValueNodeComparitor
{
 bool operator()(const CPathNode* a, const CPathNode* b) const
  {
      return a->f > b->f || (a->f == b->f && a->heuristic > b->heuristic);
  }
};

/**
 * Used to sort locations by x and y.
 **/
struct LocationComparitor
{
  bool operator()(const Location a, const Location b) const
  {
    int ax = a.x;
    int bx = b.x;
    return ax < bx || (ax == bx && a.y < b.y);
  }
};

//////////////////////////////////////////////////////
//         Heuristics for guiding the search
//////////////////////////////////////////////////////

/**
 * A class to represent the heuristic used in a search.
 **/
class Heuristic {
  public:
    Heuristic();
    virtual ~Heuristic();
    virtual double heuristic( CPathNode* node) = 0;
};

/**
 * Standard "moving toward a single location" heuristic.
 * This one assumes that diagonal moves are allowed.
 **/
class DiagonalDistanceHeuristic : public Heuristic
{
  private:
    int x,y;
  public:
    DiagonalDistanceHeuristic(int x, int y);
    virtual ~DiagonalDistanceHeuristic();

    virtual double heuristic( CPathNode * node);
};

/**
 * Negative distance from the location, so that it expects the goal to be anywhere 
 * *away* from there.
 * This is still a valid heuristic as it always underestimates the distance left and
 * is consistent.
 **/
class DistanceAwayHeuristic : public Heuristic
{
  private:
    int x,y;
  public:
    DistanceAwayHeuristic(int x, int y);
    virtual ~DistanceAwayHeuristic();

    virtual double heuristic( CPathNode * node);
};

/**
 * A "no distance left to goal" heuristic.
 * This results in a Dijkstra (for us, essentially breadth-first) search.
 **/
class NoHeuristic : public Heuristic
{
  public:
    NoHeuristic();
    virtual ~NoHeuristic();

    virtual double heuristic( CPathNode * node);
};

//////////////////////////////////////////////////////
//    Standard goal functions for search
//////////////////////////////////////////////////////

/**
 * A class used to wrap goal functions to be checked in a search.
 * Goal functions must implement the fulfills goal method.
 **/
class GoalFunction {
  public:
    GoalFunction();
    virtual ~GoalFunction();
    virtual bool fulfilledBy( CPathNode* node) = 0;
};

/**
 * The basic "get to this node" goal.
 **/
class SingleNodeGoal : public GoalFunction {
  private:
    int x,y;
  public:
    SingleNodeGoal(int x, int y);
    virtual ~SingleNodeGoal();

    virtual bool fulfilledBy( CPathNode * node);
};

/**
 * Get within a given range of the target creature.
 **/
class GetCloseGoal : public GoalFunction {
  private:
    Creature* searcher; //the one who is looking for a path. We want their width and depth.
    Creature* target;   //the creature they want to get close to

    float distance;
  public:
    GetCloseGoal(Creature* searcher, Creature* target, float distance);
    virtual ~GetCloseGoal();

    virtual bool fulfilledBy( CPathNode * node);
};

/**
 * Get a certain distance from a location.
 **/
class GetAwayGoal : public GoalFunction {
  private:
    int x,y;
    float distance;
  public:
    GetAwayGoal(int x, int y, float distance);
    virtual ~GetAwayGoal();

    virtual bool fulfilledBy( CPathNode * node);
};

/**
 * Ensures that the goal leaves every provided location clear.
 **/
class ClearLocationsGoal : public GoalFunction {
  private:
    Creature* searcher; //the creature that is finding a path
    std::set<Location,LocationComparitor>* locations; //the locations to get away from
  public:
    ClearLocationsGoal(Creature* searcher, std::set<Location,LocationComparitor>* locations);
    virtual ~ClearLocationsGoal();

    virtual bool fulfilledBy( CPathNode * node);
};

/**
 * Aim for the x,y coordinates of the creature, but any
 * node that it occupies can be the goal.
 **/
class SingleCreatureGoal : public GoalFunction {
  private:
    Creature* target;
  public:
    SingleCreatureGoal(Creature* target);
    virtual ~SingleCreatureGoal();

    virtual bool fulfilledBy( CPathNode * node);
};

//////////////////////////////////////////////////////
//              The search method(s)
//////////////////////////////////////////////////////
class AStar {
  
public: 
	AStar();
	~AStar();

  static void findPath( Sint16 sx, Sint16 sy,
                        std::vector<Location> *pVector,
                        Map *map,
                        Creature *creature,
                        Creature *player,
                        long maxNodes,
                        bool ignoreParty,
                        GoalFunction * goal,
                        Heuristic * heuristic );

  static void findPathToNearest( Sint16 sx, Sint16 sy,
                        std::vector<Location> *pVector,
                        Map *map,
                        Creature *creature,
                        Creature *player,
                        long maxNodes,
                        bool ignoreParty,
                        GoalFunction * goal ); //TODO: pass in a set of targets, not a goal


  static bool isOutOfTheWay( Creature *a, std::vector<Location> *aPath, int aStart,
                             Creature *b, std::vector<Location> *bPath, int bStart );

  static bool isBlocked( Sint16 x, Sint16 y,
                         Creature *creature, Creature *player, Map *map, 
                         bool ignoreCreatures=false);

};

#endif
