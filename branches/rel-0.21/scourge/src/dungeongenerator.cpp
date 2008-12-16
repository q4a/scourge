/***************************************************************************
              dungeongenerator.cpp  -  Generates multilevel mazes
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

#include "common/constants.h"
#include "dungeongenerator.h"
#include "render/renderlib.h"
#include "rpg/rpglib.h"
#include "scourge.h"
#include "shapepalette.h"
#include "board.h"
#include "gui/progress.h"
#include "item.h"
#include "creature.h"
#include "debug.h"

using namespace std;

#define FORCE_WATER 0

// 1 out of SECRET_DOOR_CHANCE doors are secret doors
#ifdef DEBUG_SECRET_DOORS
#define SECRET_DOOR_CHANCE 2.0f
#else
#define SECRET_DOOR_CHANCE 10.0f
#endif

/*
width - max 31
height - max 31

curvyness - in %, the lower the more twisty maze
sparseness - (1-5) the higher the more sparse (more empty space)
loopyness - in %, the higher the more loops in the maze

roomcount
room max width
room max height

object count
*/
#define MAX_DUNGEON_LEVEL 7

const int DungeonGenerator::levels[][9] = {
	{ 10, 10,  90, 5,  1,    2,  4,  4,      5 },
	{ 15, 15,  90, 5,  1,    3,  6,  4,     10 },
	{ 15, 15,  50, 4, 20,    4,  5,  5,     15 },
	{ 20, 20,  50, 6, 20,    5,  6,  5,     20 },
	{ 25, 25,  40, 6, 20,    5,  6,  5,     25 },
	{ 30, 25,   5, 6, 25,    6,  6,  6,     30 },
	{ 31, 31,   3, 5, 25,    6,  7,  7,     35 }
};

DungeonGenerator::DungeonGenerator( Scourge *scourge, int level, int depth, int maxDepth,
    bool stairsDown, bool stairsUp, Mission *mission ) :
		TerrainGenerator( scourge, level, depth, maxDepth, stairsDown, stairsUp, mission, 18 ) {


	initByLevel();

	nodes = new Uint16*[ width ];
	for ( int i = 0; i < width; i++ ) {
		nodes[i] = new Uint16[ height ];
	}

	for ( int x = 0; x < width; x++ ) {
		for ( int y = 0; y < height; y++ ) {
			nodes[x][y] = UNVISITED;
		}
	}

	notVisitedCount = width * height;
	notVisited = new int[notVisitedCount];
	for ( int i = 0; i < notVisitedCount; i++ ) {
		notVisited[i] = i;
	}
	visitedCount = 0;
	visited = new int[notVisitedCount];

	doorCount = 0;
}

DungeonGenerator::~DungeonGenerator() {
	for ( int i = 0; i < width; i++ ) {
		delete [] nodes[i];
	}
	delete [] nodes;
	delete[] notVisited;
	delete[] visited;
}

void DungeonGenerator::initByLevel() {
	int dungeonLevel = level / 8;
	if ( dungeonLevel < 0 ) dungeonLevel = 0;

	if ( dungeonLevel >= MAX_DUNGEON_LEVEL ) {
		cerr << "*** Warning: attempted to create dungeon level " << dungeonLevel <<
		". Max is " << MAX_DUNGEON_LEVEL << endl;
		dungeonLevel = MAX_DUNGEON_LEVEL - 1;
	}

//  cerr << "*** Creating dungeon level: " << dungeonLevel << " depth=" << depth << endl;

	this->width = levels[dungeonLevel][dgWIDTH];
	this->height = levels[dungeonLevel][dgHEIGHT];
	this->curvyness = levels[dungeonLevel][dgCURVYNESS];
	this->sparseness = levels[dungeonLevel][dgSPARSENESS];
	this->loopyness = levels[dungeonLevel][dgLOOPYNESS];
	this->roomCount = levels[dungeonLevel][dgROOMCOUNT];
	this->roomMaxWidth = levels[dungeonLevel][dgROOMMAXWIDTH];
	this->roomMaxHeight = levels[dungeonLevel][dgROOMMAXHEIGHT];
	this->objectCount = levels[dungeonLevel][dgOBJECTCOUNT];
	this->monsters = true;
}

void DungeonGenerator::makeRooms() {
	int rw, rh, px, py;
	int best, score;

	for ( int i = 0; i < roomCount; i++ ) {
		// create a room
		rw = Util::pickOne( roomMaxWidth / 2, roomMaxWidth );
		rh = Util::pickOne( roomMaxHeight / 2, roomMaxHeight );
		best = -1;
		px = py = -1;
		// find best place for this room
		if ( i % 2 ) {
			for ( int x = 0; x < width - rw; x++ ) {
				for ( int y = 0; y < height - rh; y++ ) {
					score = getScore( x, y, rw, rh );
					if ( score > 0 && ( best == -1 || score < best ) ) {
						best = score;
						px = x;
						py = y;
					}
				}
			}
		} else {
			for ( int x = width - rw - 1; x >= 0; x-- ) {
				for ( int y = height - rh - 1; y >= 0; y-- ) {
					score = getScore( x, y, rw, rh );
					if ( score > 0 && ( best == -1 || score < best ) ) {
						best = score;
						px = x;
						py = y;
					}
				}
			}
		}
		// set the room
		if ( px > -1 && py > -1 ) {

			// save the room info
			room[i].x = offset + px * unitSide;
			room[i].y = offset + py * unitSide;
			room[i].w = rw * unitSide;
			room[i].h = rh * unitSide;
			room[i].valueBonus = depth / 2;

			for ( int x = px; x < px + rw; x++ ) {
				for ( int y = py; y < py + rh; y++ ) {
					nodes[x][y] = ROOM + N_PASS + S_PASS + E_PASS + W_PASS;

					// 1. connect the room to the passage
					// 2. put in some doors: after each door, the chance of there being
					//    another door decreases.
					// 3. put in the walls
					if ( x == px ) {
						if ( x > 0 && nodes[x - 1][y] != UNVISITED ) {
							nodes[x - 1][y] |= E_PASS;
						} else {
							nodes[x][y] -= W_PASS;
						}
					}
					if ( x == px + rw - 1 ) {
						if ( x < width - 1 && nodes[x + 1][y] != UNVISITED ) {
							nodes[x + 1][y] |= W_PASS;
						} else {
							nodes[x][y] -= E_PASS;
						}
					}
					if ( y == py ) {
						if ( y > 0 && nodes[x][y - 1] != UNVISITED ) {
							nodes[x][y - 1] |= S_PASS;
						} else {
							nodes[x][y] -= N_PASS;
						}
					}
					if ( y == py + rh - 1 ) {
						if ( y < height - 1 && nodes[x][y + 1] != UNVISITED ) {
							nodes[x][y + 1] |= N_PASS;
						} else {
							nodes[x][y] -= S_PASS;
						}
					}
				}
			}

			// add doors
			for ( int x = px; x < px + rw; x++ ) {
				for ( int y = py; y < py + rh; y++ ) {
					if ( x == px ) {
						if ( x > 0 && nodes[x - 1][y] != UNVISITED && nodes[x - 1][y] < ROOM ) {
							nodes[x][y] |= W_DOOR;
						}
					}
					if ( x == px + rw - 1 ) {
						if ( x < width - 1 && nodes[x + 1][y] != UNVISITED && nodes[x + 1][y] < ROOM ) {
							nodes[x][y] |= E_DOOR;
						}
					}
					if ( y == py ) {
						if ( y > 0 && nodes[x][y - 1] != UNVISITED && nodes[x][y - 1] < ROOM ) {
							nodes[x][y] |= N_DOOR;
						}
					}
					if ( y == py + rh - 1 ) {
						if ( y < height - 1 && nodes[x][y + 1] != UNVISITED && nodes[x][y + 1] < ROOM ) {
							nodes[x][y] |= S_DOOR;
						}
					}

				}
			}
		}

	}
}

int DungeonGenerator::getScore( int px, int py, int rw, int rh ) {
	int score = 0;
	for ( int x = px; x < px + rw; x++ ) {
		for ( int y = py; y < py + rh; y++ ) {
			if ( nodes[x][y] == UNVISITED ) {
				if ( x < width - 1 && nodes[x + 1][y] != UNVISITED ) score++;
				if ( x > 0 && nodes[x - 1][y] != UNVISITED ) score++;
				if ( y < height - 1 && nodes[x][y + 1] != UNVISITED ) score++;
				if ( y > 0 && nodes[x][y - 1] != UNVISITED ) score++;

				if ( x < width - 1 && nodes[x + 1][y] >= ROOM ) score += 100;
				if ( x > 0 && nodes[x - 1][y] >= ROOM ) score += 100;
				if ( y < height - 1 && nodes[x][y + 1] >= ROOM ) score += 100;
				if ( y > 0 && nodes[x][y - 1] >= ROOM ) score += 100;
			} else if ( nodes[x][y] >= ROOM ) {
				score += 100;
			} else {
				score += 3;
			}
		}
	}
	return score;
}

void DungeonGenerator::makeLoops() {
	for ( int i = 0; i < sparseness; i++ ) {
		for ( int x = 0; x < width; x++ ) {
			for ( int y = 0; y < height; y++ ) {
				switch ( nodes[x][y] ) {
				case N_PASS:
				case S_PASS:
				case E_PASS:
				case W_PASS:
					if ( Util::dice( 100 ) <= loopyness ) {
						generatePassage( x, y, false );
					}
					break;
				default:
					break;
				}
			}
		}
	}
}

void DungeonGenerator::makeSparse() {
	for ( int i = 0; i < sparseness; i++ ) {
		for ( int x = 0; x < width; x++ ) {
			for ( int y = 0; y < height; y++ ) {
				switch ( nodes[x][y] ) {
				case N_PASS:
					nodes[x][y] = UNVISITED;
					if ( y > 0 ) {
						nodes[x][y - 1] -= S_PASS;
					}
					break;
				case S_PASS:
					nodes[x][y] = UNVISITED;
					if ( y < height - 1 ) {
						nodes[x][y + 1] -= N_PASS;
					}
					break;
				case E_PASS:
					nodes[x][y] = UNVISITED;
					if ( x < width - 1 ) {
						nodes[x + 1][y] -= W_PASS;
					}
					break;
				case W_PASS:
					nodes[x][y] = UNVISITED;
					if ( x > 0 ) {
						nodes[x - 1][y] -= E_PASS;
					}
					break;
				default:
					break;
				}
			}
		}
	}
}

void DungeonGenerator::generateMaze() {
	int x, y;

	//fprintf(stderr, "Starting maze w=%d h=%d\n", width, height);
	nextNotVisited( &x, &y );
	while ( notVisitedCount > 0 ) {

		// draw the passage
		generatePassage( x, y, true );

		// select a starting point
		nextVisited( &x, &y );
		if ( x == -1 ) break;
	}
}

void DungeonGenerator::generatePassage( const int sx, const int sy, const bool stopAtVisited ) {
	//char line[80];

	int x = sx;
	int y = sy;

	//fprintf(stderr, "\tnotVisitedCount=%d x=%d y=%d\n", notVisitedCount, x, y);
	int nx = x;
	int ny = y;

	int dir = initDirections();
	// keep going while you can
	int stepCount = 0;
	bool reachedVisited = false;
	bool inMap = false;
	while ( true ) {
		// take a step in the selected direction
		switch ( dir ) {
		case DIR_N: ny--; break;
		case DIR_S: ny++; break;
		case DIR_W: nx--; break;
		case DIR_E: nx++; break;
		default: fprintf( stderr, "ERROR: unknown direction selected: %d\n", dir );
		}

		// off the map or location already visited
		inMap = ( nx >= 0 && nx < width && ny >= 0 && ny < height );

		if ( !inMap ||
		        nodes[nx][ny] != UNVISITED ||
		        stepCount++ >= width / 2 ||
		        ( curvyness > 1 && curvyness < 100 &&
		          Util::dice( curvyness ) == 0 ) ) {

			if ( !stopAtVisited && inMap && nodes[nx][ny] != UNVISITED ) {
				reachedVisited = true;
			} else {
				// step back
				nx = x;
				ny = y;
				// pick another direction
				dir = nextDirection();
				if ( dir == -1 ) {
					break;
				}
				stepCount = 0;
				continue;
			}
		}

		// connect to the previous cell
		switch ( dir ) {
		case DIR_N:
			nodes[x][y] |= N_PASS;
			if ( inMap ) nodes[nx][ny] |= S_PASS;
			break;
		case DIR_S:
			nodes[x][y] |= S_PASS;
			if ( inMap ) nodes[nx][ny] |= N_PASS;
			break;
		case DIR_W:
			nodes[x][y] |= W_PASS;
			if ( inMap ) nodes[nx][ny] |= E_PASS;
			break;
		case DIR_E:
			nodes[x][y] |= E_PASS;
			if ( inMap ) nodes[nx][ny] |= W_PASS;
			break;
		}

		if ( reachedVisited ) {
			reachedVisited = false;
			// step back
			nx = x;
			ny = y;
			// pick another direction
			dir = nextDirection();
			if ( dir == -1 ) {
				break;
			}
			stepCount = 0;
			continue;
		}


		// mark the cell visited
		markVisited( nx, ny );

		// save last position
		x = nx;
		y = ny;

		// debug
		//printMaze();
		//gets(line);
	}
}

void DungeonGenerator::markVisited( int x, int y ) {
	// add to visited
	int n = ( y * width ) + x;
	// has it already been visited?
	for ( int i = 0; i < visitedCount; i++ ) {
		if ( visited[i] == n ) return;
	}
	visited[visitedCount++] = n;
	// remove from not visited
	for ( int i = 0; i < notVisitedCount; i++ ) {
		if ( notVisited[i] == n ) {
			notVisitedCount--;
			for ( int t = i; t < notVisitedCount; t++ ) {
				notVisited[t] = notVisited[t + 1];
			}
			return;
		}
	}
}

bool DungeonGenerator::isVisited( int x, int y ) {
	int n = ( y * width ) + x;
	for ( int i = 0; i < visitedCount; i++ ) {
		if ( visited[i] == n ) return true;
	}
	return false;
}

void DungeonGenerator::nextNotVisited( int *x, int *y ) {
	// are there no more unvisited locations?
	if ( notVisitedCount <= 0 ) {
		*x = *y = -1;
		return;
	}
	// get a random location
	int index = Util::dice( notVisitedCount );
	int n = notVisited[index];
	// remove from visited areas
	notVisitedCount--;
	for ( int i = index; i < notVisitedCount; i++ ) {
		notVisited[i] = notVisited[i + 1];
	}
	// break up into x,y coordinates
	*y = n / width;
	*x = n % width;
	// add it to visited
	visited[visitedCount++] = n;
}

void DungeonGenerator::nextVisited( int *x, int *y ) {
	// are there no visited locations?
	if ( visitedCount <= 0 ) {
		*x = *y = -1;
		return;
	}
	// get a random location
	int index = Util::dice( visitedCount );
	int n = visited[index];
	// break up into x,y coordinates
	*y = n / width;
	*x = n % width;
}

int DungeonGenerator::initDirections() {
	// init all available directions
	dirCount = DIR_COUNT;
	for ( int i = 0; i < dirCount; i++ ) {
		dirs[i] = i;
	}
	return dirs[ Util::dice( dirCount ) ];
}

int DungeonGenerator::nextDirection() {
	if ( dirCount <= 0 ) return -1;
	int index = Util::dice( dirCount );
	int dir = dirs[index];
	dirCount--;
	for ( int i = index; i < dirCount; i++ ) {
		dirs[i] = dirs[i + 1];
	}
	return dir;
}

void DungeonGenerator::printMaze() {
	printf( "---------------------------------------\n" );
	int c = 0;
	for ( int y = 0; y < height; y++ ) {
		for ( int i = 0; i < 3; i++ ) {
			for ( int x = 0; x < width; x++ ) {

				switch ( i ) {
				case 0: // top row
					if ( ( nodes[x][y] & N_PASS ) ) {
						printf( " | " );
					} else {
						printf( "   " );
					}
					break;
				case 1:
					if ( ( nodes[x][y] & W_PASS ) ) {
						printf( "-" );
					} else {
						printf( " " );
					}
					if ( nodes[x][y] == UNVISITED )
						printf( " " );
					else if ( nodes[x][y] & ROOM )
						printf( "*" );
					else
						printf( "O" );
					if ( ( nodes[x][y] & E_PASS ) ) {
						printf( "-" );
					} else {
						printf( " " );
					}
					break;
				case 2: // bottom row
					if ( ( nodes[x][y] & S_PASS ) ) {
						printf( " | " );
					} else {
						printf( "   " );
					}
					break;
				}
				c++;
			}
			printf( "\n" );
		}
		c++;
	}
	printf( "---------------------------------------\n" );
}

void DungeonGenerator::generate( Map *map, ShapePalette *shapePal ) {
	//cerr << "DUNGEON" << endl;

	updateStatus( _( "Assembling Dungeon Level" ) );

	generateMaze();
	//  printMaze();

	makeSparse();
	//  printMaze();

	makeLoops();
	//  printMaze();

	makeRooms();
	//  printMaze();
}

bool DungeonGenerator::drawNodes( Map *map, ShapePalette *shapePal ) {
	// flooded map?
	map->setHasWater( FORCE_WATER ||
	                  0 == Util::dice( 5 ) );

	updateStatus( _( "Loading theme" ) );
	if ( map->getPreferences()->isDebugTheme() ) shapePal->loadDebugTheme();
	else shapePal->loadRandomTheme();

	updateStatus( _( "Drawing walls" ) );
	drawBasics( map, shapePal );

	updateStatus( _( "Fixing rooms" ) );
	removeColumns( map, shapePal );
	addRugs( map, shapePal );

	return true;
}

void DungeonGenerator::drawBasics( Map *map, ShapePalette *shapePal ) {
	// add shapes to map
	Sint16 mapx, mapy;
	for ( Sint16 x = 0; x < width; x++ ) {
		for ( Sint16 y = 0; y < height; y++ ) {

			mapx = x * unitSide + offset;
			mapy = y * unitSide + offset;
			if ( nodes[x][y] != UNVISITED ) {

				if ( nodes[x][y] >= ROOM ) {
					map->setFloorPosition( mapx, mapy + unitSide,
					                       shapePal->findShapeByName( "ROOM_FLOOR_TILE" ) );
				} else {
					map->setFloorPosition( mapx, mapy + unitSide,
					                       shapePal->findShapeByName( "FLOOR_TILE" ) );
				}

				// init the free space
				int secretDoor = 0;
				if ( nodes[x][y] & E_DOOR ) {
					if ( 0 == Util::dice( static_cast<int>( SECRET_DOOR_CHANCE ) ) ) {
						nodes[x][y] -= E_DOOR;
						secretDoor = E_DOOR;
						nodes[x][y] -= E_PASS;
					} else {
						drawDoor( map, shapePal, mapx, mapy, E_DOOR );
					}
				} else if ( nodes[x][y] & W_DOOR ) {
					if ( 0 == Util::dice( static_cast<int>( SECRET_DOOR_CHANCE ) ) ) {
						nodes[x][y] -= W_DOOR;
						secretDoor = W_DOOR;
						nodes[x][y] -= W_PASS;
					} else {
						drawDoor( map, shapePal, mapx, mapy, W_DOOR );
					}
				} else if ( nodes[x][y] & N_DOOR ) {
					if ( 0 == Util::dice( static_cast<int>( SECRET_DOOR_CHANCE ) ) ) {
						nodes[x][y] -= N_DOOR;
						secretDoor = N_DOOR;
						nodes[x][y] -= N_PASS;
					} else {
						drawDoor( map, shapePal, mapx, mapy, N_DOOR );
					}
				} else if ( nodes[x][y] & S_DOOR ) {
					if ( 0 == Util::dice( static_cast<int>( SECRET_DOOR_CHANCE ) ) ) {
						nodes[x][y] -= S_DOOR;
						secretDoor = S_DOOR;
						nodes[x][y] -= S_PASS;
					} else {
						drawDoor( map, shapePal, mapx, mapy, S_DOOR );
					}
				}

				// random doors
				if ( !secretDoor ) {
					if ( ( nodes[x][y] & W_PASS ) &&
					        !( nodes[x][y] & N_PASS ) &&
					        !( nodes[x][y] & S_PASS ) ) {
						if ( Util::dice( 100 ) <= randomDoors ) {
							if ( 0 == Util::dice( static_cast<int>( SECRET_DOOR_CHANCE ) ) ) {
								nodes[x][y] -= W_DOOR;
								secretDoor = W_DOOR;
								nodes[x][y] -= W_PASS;
							} else {
								drawDoor( map, shapePal, mapx, mapy, W_DOOR );
							}
						}
					}
					if ( ( nodes[x][y] & E_PASS ) &&
					        !( nodes[x][y] & N_PASS ) &&
					        !( nodes[x][y] & S_PASS ) ) {
						if ( Util::dice( 100 ) <= randomDoors ) {
							if ( 0 == Util::dice( static_cast<int>( SECRET_DOOR_CHANCE ) ) ) {
								nodes[x][y] -= E_DOOR;
								secretDoor = E_DOOR;
								nodes[x][y] -= E_PASS;
							} else {
								drawDoor( map, shapePal, mapx, mapy, E_DOOR );
							}
						}
					}
					if ( ( nodes[x][y] & S_PASS ) &&
					        !( nodes[x][y] & W_PASS ) &&
					        !( nodes[x][y] & E_PASS ) ) {
						if ( Util::dice( 100 ) <= randomDoors ) {
							if ( 0 == Util::dice( static_cast<int>( SECRET_DOOR_CHANCE ) ) ) {
								nodes[x][y] -= S_DOOR;
								secretDoor = S_DOOR;
								nodes[x][y] -= S_PASS;
							} else {
								drawDoor( map, shapePal, mapx, mapy, S_DOOR );
							}
						}
					}
					if ( ( nodes[x][y] & N_PASS ) &&
					        !( nodes[x][y] & W_PASS ) &&
					        !( nodes[x][y] & E_PASS ) ) {
						if ( Util::dice( 100 ) <= randomDoors ) {
							if ( 0 == Util::dice( static_cast<int>( SECRET_DOOR_CHANCE ) ) ) {
								nodes[x][y] -= N_DOOR;
								secretDoor = N_DOOR;
								nodes[x][y] -= N_PASS;
							} else {
								drawDoor( map, shapePal, mapx, mapy, N_DOOR );
							}
						}
					}
				}

				int wallX, wallY;
				Shape *wall = NULL;
				if ( !( nodes[x][y] & W_PASS ) ) {
					if ( nodes[x][y] & N_PASS && nodes[x][y] & S_PASS ) {
						wallX = mapx;
						wallY = mapy + unitSide;
						wall = shapePal->findShapeByName( "EW_WALL_TWO_EXTRAS" );
					} else if ( nodes[x][y] & N_PASS ) {
						wallX = mapx;
						wallY = mapy + unitSide - unitOffset;
						wall = shapePal->findShapeByName( "EW_WALL_EXTRA" );
					} else if ( nodes[x][y] & S_PASS ) {
						wallX = mapx;
						wallY = mapy + unitSide;
						wall = shapePal->findShapeByName( "EW_WALL_EXTRA" );
					} else {
						wallX = mapx;
						wallY = mapy + unitSide - unitOffset;
						wall = shapePal->findShapeByName( "EW_WALL" );
					}
					if ( wall ) {
						map->setPosition( wallX, wallY, 0, wall );
						if ( secretDoor == W_DOOR ) {
							map->addSecretDoor( wallX, wallY );
						} else {
							if ( Util::dice( 100 ) <= torches ) {
								map->setPosition( mapx + unitOffset, mapy + unitSide - 4,
								                  6, shapePal->findShapeByName( "LAMP_WEST" ) );
								map->setPosition( mapx + unitOffset, mapy + unitSide - 4,
								                  4, shapePal->findShapeByName( "LAMP_BASE" ) );
							}
						}
					}
				}
				if ( !( nodes[x][y] & E_PASS ) ) {
					if ( nodes[x][y] & N_PASS && nodes[x][y] & S_PASS ) {
						wallX = mapx + unitSide - unitOffset;
						wallY = mapy + unitSide;
						wall = shapePal->findShapeByName( "EW_WALL_TWO_EXTRAS" );
					} else if ( nodes[x][y] & N_PASS ) {
						wallX = mapx + unitSide - unitOffset;
						wallY = mapy + unitSide - unitOffset;
						wall = shapePal->findShapeByName( "EW_WALL_EXTRA" );
					} else if ( nodes[x][y] & S_PASS ) {
						wallX = mapx + unitSide - unitOffset;
						wallY = mapy + unitSide;
						wall = shapePal->findShapeByName( "EW_WALL_EXTRA" );
					} else {
						wallX = mapx + unitSide - unitOffset;
						wallY = mapy + unitSide - unitOffset;
						wall = shapePal->findShapeByName( "EW_WALL" );
					}
					if ( wall ) {
						map->setPosition( wallX, wallY, 0, wall );
						if ( secretDoor == E_DOOR ) {
							map->addSecretDoor( wallX, wallY );
						} else {
							if ( Util::dice( 100 ) <= torches ) {
								map->setPosition( mapx + unitSide - ( unitOffset + 1 ), mapy + unitSide - 4,
								                  6, shapePal->findShapeByName( "LAMP_EAST" ) );
								map->setPosition( mapx + unitSide - ( unitOffset + 1 ), mapy + unitSide - 4,
								                  4, shapePal->findShapeByName( "LAMP_BASE" ) );
							}
						}
					}
				}
				if ( !( nodes[x][y] & N_PASS ) ) {
					if ( nodes[x][y] & W_PASS && nodes[x][y] & E_PASS ) {
						wallX = mapx;
						wallY = mapy + unitOffset;
						wall = shapePal->findShapeByName( "NS_WALL_TWO_EXTRAS" );
					} else if ( nodes[x][y] & W_PASS ) {
						wallX = mapx;
						wallY = mapy + unitOffset;
						wall = shapePal->findShapeByName( "NS_WALL_EXTRA" );
					} else if ( nodes[x][y] & E_PASS ) {
						wallX = mapx + unitOffset;
						wallY = mapy + unitOffset;
						wall = shapePal->findShapeByName( "NS_WALL_EXTRA" );
					} else {
						wallX = mapx + unitOffset;
						wallY = mapy + unitOffset;
						wall = shapePal->findShapeByName( "NS_WALL" );
					}
					if ( wall ) {
						map->setPosition( wallX, wallY, 0, wall );
						if ( secretDoor == N_DOOR ) {
							map->addSecretDoor( wallX, wallY );
						} else {
							if ( Util::dice( 100 ) <= torches ) {
								map->setPosition( mapx + 4, mapy + unitOffset + 1, 6,
								                  shapePal->findShapeByName( "LAMP_NORTH" ) );
								map->setPosition( mapx + 4, mapy + unitOffset + 1, 4,
								                  shapePal->findShapeByName( "LAMP_BASE" ) );
							}
						}
					}
				}
				if ( !( nodes[x][y] & S_PASS ) ) {
					if ( nodes[x][y] & W_PASS && nodes[x][y] & E_PASS ) {
						wallX = mapx;
						wallY = mapy + unitSide;
						wall = shapePal->findShapeByName( "NS_WALL_TWO_EXTRAS" );
					} else if ( nodes[x][y] & W_PASS ) {
						wallX = mapx;
						wallY = mapy + unitSide;
						wall = shapePal->findShapeByName( "NS_WALL_EXTRA" );
					} else if ( nodes[x][y] & E_PASS ) {
						wallX = mapx + unitOffset;
						wallY = mapy + unitSide;
						wall = shapePal->findShapeByName( "NS_WALL_EXTRA" );
					} else {
						wallX = mapx + unitOffset;
						wallY = mapy + unitSide;
						wall = shapePal->findShapeByName( "NS_WALL" );
					}
					if ( wall ) {
						map->setPosition( wallX, wallY, 0, wall );
						if ( secretDoor == S_DOOR ) {
							map->addSecretDoor( wallX, wallY );
						}
					}
				}


				if ( nodes[x][y] & N_PASS && nodes[x][y] & W_PASS ) {
					map->setPosition( mapx, mapy + unitOffset, 0,
					                  shapePal->findShapeByName( "CORNER" ) );
				}
				if ( nodes[x][y] & N_PASS && nodes[x][y] & E_PASS ) {
					map->setPosition( mapx + unitSide - unitOffset, mapy + unitOffset, 0,
					                  shapePal->findShapeByName( "CORNER" ) );
				}
				if ( nodes[x][y] & S_PASS && nodes[x][y] & W_PASS ) {
					map->setPosition( mapx, mapy + unitSide, 0,
					                  shapePal->findShapeByName( "CORNER" ) );
				}
				if ( nodes[x][y] & S_PASS && nodes[x][y] & E_PASS ) {
					map->setPosition( mapx + unitSide - unitOffset, mapy + unitSide, 0,
					                  shapePal->findShapeByName( "CORNER" ) );
				}
				if ( !( nodes[x][y] & N_PASS ) && !( nodes[x][y] & W_PASS ) ) {
					map->setPosition( mapx, mapy + unitOffset, 0,
					                  shapePal->findShapeByName( "CORNER" ) );
				}
				if ( !( nodes[x][y] & N_PASS ) && !( nodes[x][y] & E_PASS ) ) {
					map->setPosition( mapx + unitSide - unitOffset, mapy + unitOffset, 0,
					                  shapePal->findShapeByName( "CORNER" ) );
				}
				if ( !( nodes[x][y] & S_PASS ) && !( nodes[x][y] & W_PASS ) ) {
					map->setPosition( mapx, mapy + unitSide, 0,
					                  shapePal->findShapeByName( "CORNER" ) );
				}
				if ( !( nodes[x][y] & S_PASS ) && !( nodes[x][y] & E_PASS ) ) {
					map->setPosition( mapx + unitSide - unitOffset, mapy + unitSide, 0,
					                  shapePal->findShapeByName( "CORNER" ) );
				}
			}
		}
	}
}

void DungeonGenerator::removeColumns( Map *map, ShapePalette *shapePal ) {
	// Remove 'columns' from rooms
	for ( int roomIndex = 0; roomIndex < roomCount; roomIndex++ ) {
		int startx = ( room[roomIndex].x ) + unitOffset;
		int endx = ( room[roomIndex].x + room[roomIndex].w ) - ( unitOffset * 2 );
		int starty = ( room[roomIndex].y ) + ( unitOffset * 2 );
		int endy = ( room[roomIndex].y + room[roomIndex].h ) - unitOffset;
		for ( int x = startx; x < endx; x++ ) {
			for ( int y = starty; y < endy; y++ ) {
				map->removePosition( x, y, 0 );
			}
		}
	}
}

void DungeonGenerator::drawEastDoor( Map *map, ShapePalette *shapePal, Sint16 mapx, Sint16 mapy, bool secret ) {
	if ( !map->coversDoor( shapePal->findShapeByName( "EW_DOOR" ),
	                       mapx + unitSide - unitOffset + 1, mapy + unitSide - unitOffset - 2 ) ) {
		if ( secret ) {
		} else {
			map->setPosition( mapx + unitSide - unitOffset, mapy + unitSide - unitOffset,
			                  wallHeight - 2, shapePal->findShapeByName( "EW_DOOR_TOP" ) );
			map->setPosition( mapx + unitSide - unitOffset, mapy + unitOffset +  2,
			                  0, shapePal->findShapeByName( "DOOR_SIDE" ) );
			map->setPosition( mapx + unitSide - unitOffset, mapy + unitOffset * 2 +  2,
			                  0, shapePal->findShapeByName( "DOOR_SIDE" ) );
			map->setPosition( mapx + unitSide - unitOffset + 1, mapy + unitSide - unitOffset - 2,
			                  0, shapePal->findShapeByName( "EW_DOOR" ) );
			if ( doorCount < MAX_DOOR_COUNT ) {
				door[doorCount][0] = mapx + unitSide - unitOffset + 1;
				door[doorCount][1] = mapy + unitSide - unitOffset - 2;
				doorCount++;
			}
			map->setPosition( mapx + unitSide - unitOffset, mapy + unitSide - unitOffset,
			                  0, shapePal->findShapeByName( "DOOR_SIDE" ) );
		}
	}
}

void DungeonGenerator::drawWestDoor( Map *map, ShapePalette *shapePal, Sint16 mapx, Sint16 mapy, bool secret ) {
	if ( !map->coversDoor( shapePal->findShapeByName( "EW_DOOR" ),
	                       mapx + 1, mapy + unitSide - unitOffset - 2 ) ) {
		map->setPosition( mapx, mapy + unitSide - unitOffset,
		                  wallHeight - 2, shapePal->findShapeByName( "EW_DOOR_TOP" ) );
		map->setPosition( mapx, mapy + unitOffset +  2,
		                  0, shapePal->findShapeByName( "DOOR_SIDE" ) );
		map->setPosition( mapx, mapy + unitOffset * 2 +  2,
		                  0, shapePal->findShapeByName( "DOOR_SIDE" ) );
		map->setPosition( mapx + 1, mapy + unitSide - unitOffset - 2,
		                  0, shapePal->findShapeByName( "EW_DOOR" ) );
		if ( doorCount < MAX_DOOR_COUNT ) {
			door[doorCount][0] = mapx + 1;
			door[doorCount][1] = mapy + unitSide - unitOffset - 2;
			doorCount++;
		}
		map->setPosition( mapx, mapy + unitSide - unitOffset,
		                  0, shapePal->findShapeByName( "DOOR_SIDE" ) );
	}
}

void DungeonGenerator::drawSouthDoor( Map *map, ShapePalette *shapePal, Sint16 mapx, Sint16 mapy, bool secret ) {
	if ( !map->coversDoor( shapePal->findShapeByName( "NS_DOOR" ),
	                       mapx + unitOffset * 2, mapy + unitSide - 1 ) ) {
		map->setPosition( mapx + unitOffset, mapy + unitSide,
		                  wallHeight - 2, shapePal->findShapeByName( "NS_DOOR_TOP" ) );
		map->setPosition( mapx + unitOffset, mapy + unitSide,
		                  0, shapePal->findShapeByName( "DOOR_SIDE" ) );
		map->setPosition( mapx + unitOffset * 2, mapy + unitSide - 1,
		                  0, shapePal->findShapeByName( "NS_DOOR" ) );
		if ( doorCount < MAX_DOOR_COUNT ) {
			door[doorCount][0] = mapx + unitOffset * 2;
			door[doorCount][1] = mapy + unitSide - 1;
			doorCount++;
		}
		map->setPosition( mapx + unitSide - unitOffset * 2, mapy + unitSide,
		                  0, shapePal->findShapeByName( "DOOR_SIDE" ) );
		map->setPosition( mapx + unitSide - unitOffset * 3, mapy + unitSide,
		                  0, shapePal->findShapeByName( "DOOR_SIDE" ) );
	}
}

void DungeonGenerator::drawNorthDoor( Map *map, ShapePalette *shapePal, Sint16 mapx, Sint16 mapy, bool secret ) {
	if ( !map->coversDoor( shapePal->findShapeByName( "NS_DOOR" ),
	                       mapx + unitOffset * 2, mapy + unitOffset - 1 ) ) {
		map->setPosition( mapx + unitOffset, mapy + unitOffset,
		                  wallHeight - 2, shapePal->findShapeByName( "NS_DOOR_TOP" ) );
		map->setPosition( mapx + unitOffset, mapy + unitOffset,
		                  0, shapePal->findShapeByName( "DOOR_SIDE" ) );
		map->setPosition( mapx + unitOffset * 2, mapy + unitOffset - 1,
		                  0, shapePal->findShapeByName( "NS_DOOR" ) );
		if ( doorCount < MAX_DOOR_COUNT ) {
			door[doorCount][0] = mapx + unitOffset * 2;
			door[doorCount][1] = mapy + unitOffset - 1;
			doorCount++;
		}
		map->setPosition( mapx + unitSide - unitOffset * 2, mapy + unitOffset,
		                  0, shapePal->findShapeByName( "DOOR_SIDE" ) );
		map->setPosition( mapx + unitSide - unitOffset * 3, mapy + unitOffset,
		                  0, shapePal->findShapeByName( "DOOR_SIDE" ) );
	}
}

void DungeonGenerator::drawDoor( Map *map, ShapePalette *shapePal,
    Sint16 mapx, Sint16 mapy, int doorType, bool secret ) {
	switch ( doorType ) {
	case E_DOOR: drawEastDoor( map, shapePal, mapx, mapy, secret ); break;
	case W_DOOR: drawWestDoor( map, shapePal, mapx, mapy, secret ); break;
	case N_DOOR: drawNorthDoor( map, shapePal, mapx, mapy, secret ); break;
	case S_DOOR: drawSouthDoor( map, shapePal, mapx, mapy, secret ); break;
	default: cerr << "*** Error: Unknown door type: " << doorType << endl;
	}
}

void DungeonGenerator::addFurniture( Map *map, ShapePalette *shapePal ) {
	// add tables, chairs, etc.
	addItemsInEveryRoom( RpgItem::getItemByName( "Table" ), 1 );
	addItemsInEveryRoom( RpgItem::getItemByName( "Chair" ), 2 );

	// add some magic pools
	DisplayInfo di;
	for ( int i = 0; i < roomCount; i++ ) {
		MagicSchool *ms = MagicSchool::getRandomSchool();
		di.red = ms->getDeityRed();
		di.green = ms->getDeityGreen();
		di.blue = ms->getDeityBlue();
		Location *pos = addShapeInRoom( scourge->getShapePalette()->findShapeByName( "POOL" ), i, &di );
		if ( pos ) {
			// store pos->deity in scourge
			scourge->addDeityLocation( pos, ms );
		}
	}
}

void DungeonGenerator::addContainers( Map *map, ShapePalette *shapePal ) {
	addContainersInRooms( map, shapePal );
}

MapRenderHelper *DungeonGenerator::getMapRenderHelper() {
	return MapRenderHelper::helpers[ MapRenderHelper::ROOM_HELPER ];
}

