/***************************************************************************
                          creature.h  -  description
                             -------------------
    begin                : Sat May 3 2003
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

#ifndef CREATURE_H
#define CREATURE_H

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <iostream>
//#include <conio.h>   // for getch
#include <vector>    // STL for Vector
#include <algorithm> // STL for Heap
#include "glshape.h"
#include "md2shape.h"
#include "map.h"
#include "scourge.h"
#include "util.h"
#include "rpg/pc.h"
#include "constants.h"

using namespace std;

// if this is defined, party members will exchange places with each other
// can't get stuck, but can move-off screen.
// fix this to keep player centered
//#define ENABLE_PARTY_SWAP 1

#define MAX_CLOSED_NODES 50

class Map;
class Scourge;

/**
  *@author Gabor Torok
  */

class Creature {

private:
    Sint16 x, y, z;
	Creature *next;
	GLShape *shape;
	Uint16 dir;
	Scourge *scourge;
    PlayerChar *pc;
	GLUquadric *quadric;
    int motion;

	int formation;
	int index;

    int tx, ty;
    int selX, selY;
    int bestPathPos;
    vector<Location> bestPath;

	
  
public:
	static const int DIAMOND_FORMATION = 0;
    static const int STAGGERED_FORMATION = 1;
	static const int SQUARE_FORMATION = 2;
	static const int ROW_FORMATION = 3;
	static const int SCOUT_FORMATION = 4;
	static const int CROSS_FORMATION = 5;
	static const int FORMATION_COUNT = 6;
    
	Creature(Scourge *scourge, GLShape *shape, PlayerChar *pc);
	~Creature();

	inline PlayerChar *getPC() { return pc; }

	inline GLUquadric *getQuadric() { return quadric; }

    inline void setMotion(int motion) { this->motion = motion; }
    inline int getMotion() { return this->motion; }
										
	//inline void setShapeIndex(Uint8 shapePosition) { this->shapePosition = shapePosition; }
	//inline Uint8 getShapeIndex() { return shapePosition; }
	bool move(Uint16 dir, Map *map);
	void follow(Map *map);
    void gotoPosition(Map *map, Sint16 px, Sint16 py, Sint16 pz);
    void moveToLocator(Map *map);
  
	inline void moveTo(Sint16 x, Sint16 y, Sint16 z) { this->x = x; this->y = y; this->z = z; }
	inline Sint16 getX() { return x; }
	inline Sint16 getY() { return y; }
	inline Sint16 getZ() { return z; }
	inline GLShape *getShape() { return shape; }
	inline void setFormation(int formation) { this->formation = formation; }
	inline int getFormation() { return formation; }
	void setNext(Creature *next, int index);
	void setNextDontMove(Creature *next, int index);
    inline Uint16 getDir() { return dir; }

	inline void draw() { getShape()->draw(); }  

	/**
	 * Get the position of this creature in the formation.
	 */
	void getFormationPosition(Sint16 *px, Sint16 *py, Sint16 *pz);

    /**
        Used to move away from the player. Find the nearest corner of the map.
    */
    void findCorner(Sint16 *px, Sint16 *py, Sint16 *pz);
  
    inline void setSelXY(int x, int y) { selX = x; selY = y; }
};


#endif

