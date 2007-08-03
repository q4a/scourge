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
 **/
struct FValueNodeComparitor
{
 bool operator()(const CPathNode* a, const CPathNode* b) const
  {
      return a->f > b->f;
  }
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
    GetCloseGoal(Creature* searcher, Creature* target);
    virtual ~GetCloseGoal();

    virtual bool fulfilledBy( CPathNode * node);
};

//////////////////////////////////////////////////////
//              The search method(s)
//////////////////////////////////////////////////////
class AStar {
  
public: 
	AStar();
	~AStar();

  static void findPath( Sint16 sx, Sint16 sy, Sint16 sz,
                        Sint16 dx, Sint16 dy, Sint16 dz,
                        std::vector<Location> *pVector,
                        Map *map,
                        Creature *creature,
                        Creature *player,
                        int maxNodes,
                        bool ignoreParty,
                        bool ignoreEndShape );

  static void findPath( Sint16 sx, Sint16 sy, Sint16 sz,
                        Sint16 dx, Sint16 dy, Sint16 dz,
                        std::vector<Location> *pVector,
                        Map *map,
                        Creature *creature,
                        Creature *player,
                        int maxNodes,
                        bool ignoreParty,
                        bool ignoreEndShape,
                        GoalFunction * goal );

  static bool isOutOfTheWay( Creature *a, std::vector<Location> *aPath, int aStart,
                             Creature *b, std::vector<Location> *bPath, int bStart );

protected:
  static bool isBlocked( Sint16 x, Sint16 y, Sint16 shapeX, Sint16 shapeY, Sint16 dx, Sint16 dy,
                         Creature *creature, Creature *player, Map *map, 
                         bool ignoreCreatures=false, bool ignoreEndShape=false );

};

#endif
