/***************************************************************************
             cellular.h  -  Cellular automaton for map generation
                             -------------------
    begin                : Thu May 15 2003
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
#ifndef CELLULAR_AUTOMATON_H
#define CELLULAR_AUTOMATON_H
#pragma once

#include <stdio.h>
#include <stdlib.h>
#include <map>

#define CELL_GROWTH_CYCLES 3

#define CLEAR_WALL_RATIO 0.45f

#define MAX_ROOM_COUNT 100

/// A generated node.
struct NodePoint {
	bool wall, island, water;
	int room;
	bool seen;
};

/// A generated room.
struct Room {
	int size;
	int x, y;
};


/// A cellular automaton, used in map generation

class CellularAutomaton {
private:
	int w, h;
	int phase;
	NodePoint **node;
	int roomCounter, biggestRoom;
	Room room[ MAX_ROOM_COUNT ];
	bool initialized;

public:
	CellularAutomaton( int w, int h );
	virtual ~CellularAutomaton();
	
	void initialize( int dw, int dh, int *data );

	void generate( bool islandsEnabled = false,
	               bool removeSinglesEnabled = false,
	               int pathWidth = 1,
	               bool isWaterEnabled = false,
	               bool findRoomsEnabled = true );

	inline int getWidth() {
		return this->w;
	}
	inline int getHeight() {
		return this->h;
	}
	inline NodePoint *getNode( int x, int y ) {
		return &node[ x ][ y ];
	}
	inline int getRoomCount() {
		return roomCounter;
	}
	inline int getBiggestRoom() {
		return biggestRoom;
	}
	inline Room* getRoom( int n ) {
		return &room[ n ];
	}

	void print();
	void makeAccessible( int x, int y );
	void makeMinSpace( int width );

protected:
	void randomize();
	void growCells();
	void growWaterCells();
	void setSeen( bool b );
	bool canReach( int sx, int sy, int ex, int ey );
	void findRooms();
	void connectPoints( int sx, int sy, int ex, int ey, bool isBiggestRoom );
	void connectRooms( int pathWidth = 1 );
	void removeSingles();
	void addIslands();
	void growCellsIsland();
	void addIslandLand();
};

#endif

