/***************************************************************************
                          outdoorgenerator.cpp  -  description
                             -------------------
    begin                : Sat May 12 2007
    copyright            : (C) 2007 by Gabor Torok
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
#include "outdoorgenerator.h"
#include "render/map.h"
#include "shapepalette.h"
#include "scourge.h"
#include "board.h"
#include "render/maprenderhelper.h"
#include "render/glshape.h"
#include <vector>

using namespace std;

OutdoorGenerator::OutdoorGenerator( Scourge *scourge, int level, int depth, int maxDepth,
																		bool stairsDown, bool stairsUp, 
																		Mission *mission) : 
TerrainGenerator( scourge, level, depth, maxDepth, stairsDown, stairsUp, mission, 12 ) {
  // init the ground
	for( int x = 0; x < MAP_WIDTH / OUTDOORS_STEP; x++ ) {
		for( int y = 0; y < MAP_DEPTH / OUTDOORS_STEP; y++ ) {
			ground[x][y] = 0;
		}
	}

  this->roomCounter = 0;
  this->biggestRoom = 0;
  this->phase = 1;

  // reasonable defaults
  TerrainGenerator::doorCount = 0;
  TerrainGenerator::roomCount = 1;
  TerrainGenerator::room[0].x = room[0].y = 0;
  TerrainGenerator::room[0].w = ( MAP_WIDTH - 2 * MAP_OFFSET ) / MAP_UNIT;
  TerrainGenerator::room[0].h = ( MAP_DEPTH - 2 * MAP_OFFSET ) / MAP_UNIT;
  TerrainGenerator::room[0].valueBonus = 0;
  TerrainGenerator::roomMaxWidth = 0;
  TerrainGenerator::roomMaxHeight = 0;
  TerrainGenerator::objectCount = 7 + ( level / 8 ) * 5;
  TerrainGenerator::monsters = true;

}

OutdoorGenerator::~OutdoorGenerator() {
}

bool OutdoorGenerator::drawNodes( Map *map, ShapePalette *shapePal ) {

  updateStatus( _( "Loading theme" ) );
  if( map->getPreferences()->isDebugTheme() ) shapePal->loadDebugTheme();
	else shapePal->loadRandomTheme();

	map->setHeightMapEnabled( true );

	// do this first, before adding shapes
	// FIXME: elevate shapes if needed, in Map::setGroundHeight, so this method can be called anytime
	for( int x = 0; x < MAP_WIDTH / OUTDOORS_STEP; x++ ) {
		for( int y = 0; y < MAP_DEPTH / OUTDOORS_STEP; y++ ) {
			map->setGroundHeight( x, y, ground[x][y] );
		}
	}

	
	for( int x = 0; x < MAP_WIDTH / OUTDOORS_STEP; x += OUTDOOR_FLOOR_TEX_SIZE ) {
		for( int y = 0; y < MAP_DEPTH / OUTDOORS_STEP; y += OUTDOOR_FLOOR_TEX_SIZE ) {
			GLuint tex = 0;
			int n = (int)( 5.0f * rand() / RAND_MAX );
			switch( n ) {
			case 0:
				tex = shapePal->getNamedTexture( "grass1" ); break;
			case 1:
				tex = shapePal->getNamedTexture( "grass2" ); break;
			case 2:
				tex = shapePal->getNamedTexture( "grass3" ); break;
			case 3:
				tex = shapePal->getNamedTexture( "grass4" ); break;
			default:
				tex = shapePal->getNamedTexture( "grass" );
			}
			for( int xx = 0; xx < OUTDOOR_FLOOR_TEX_SIZE; xx++ ){
				for( int yy = 0; yy < OUTDOOR_FLOOR_TEX_SIZE; yy++ ) {
					map->setGroundTex( x + xx, y + yy, tex );				
				}
			}
		}
	}
	
	// is this needed?
	//map->setFloor( CAVE_CHUNK_SIZE, CAVE_CHUNK_SIZE, shapePal->getNamedTexture( "grass" ) );

	// set the floor, so random positioning works in terrain generator
	for( int x = MAP_OFFSET; x < MAP_WIDTH - MAP_OFFSET; x += MAP_UNIT ) {
		for( int y = MAP_OFFSET; y < MAP_DEPTH - MAP_OFFSET; y += MAP_UNIT ) {
			map->setFloorPosition( (Sint16)x, (Sint16)y + MAP_UNIT, (Shape*)shapePal->findShapeByName( "FLOOR_TILE", true ) );
		}
	}


	for( int x = 0; x < WIDTH_IN_NODES; x++ ) {
    for( int y = 0; y < DEPTH_IN_NODES; y++ ) {
			if( node[ x ][ y ].wall ) {
				GLShape *shape = getRandomTreeShape( shapePal );
				int mapx = MAP_OFFSET + x * OUTDOOR_NODE_SIZE;
				int mapy = MAP_OFFSET + y * OUTDOOR_NODE_SIZE + shape->getDepth();
				if( !map->isBlocked( mapx, mapy, 0, 0, 0, 0, shape ) ) {
					map->setPosition( mapx, mapy, 0, shape );
				} else {
					// should not happen here
					//cerr << "shape blocked at: " << mapx << "," << mapy << endl;
					cerr << ".";
				}
			}
    }
  }

	/*
	// add some groups of trees
	for( int i = 0; i < 20; i++ ) {
		int w = (int)( 40.0f * rand() / RAND_MAX ) + 20;
		int h = (int)( 40.0f * rand() / RAND_MAX ) + 20;
		int x = MAP_OFFSET + (int)( (float)( MAP_WIDTH - w - MAP_OFFSET * 2 ) * rand() / RAND_MAX );
		int y = MAP_OFFSET + (int)( (float)( MAP_DEPTH - h - MAP_OFFSET * 2 ) * rand() / RAND_MAX );
		int count = 0;
		while( count < 10 ) {
			GLShape *shape = getRandomTreeShape( shapePal );
			int xx = x + (int)( (float)( w - shape->getWidth() ) * rand() / RAND_MAX );
			int yy = y + (int)( (float)( h - shape->getDepth() ) * rand() / RAND_MAX );
			if( !map->isBlocked( xx, yy, 0, 0, 0, 0, shape ) ) {
				map->setPosition( xx, yy, 0, shape );
			} else {
				count++;
			}
		}
	}
	*/

	return true;
}

vector<GLShape*> trees;
GLShape *OutdoorGenerator::getRandomTreeShape( ShapePalette *shapePal ) {
	//return shapePal->findShapeByName( "DEBUG_TREE" );
	if( trees.size() == 0 ) {
		// this should be in config file
		trees.push_back( shapePal->findShapeByName( "PINE_TREE" ) );
		trees.push_back( shapePal->findShapeByName( "WILLOW_TREE" ) );
		trees.push_back( shapePal->findShapeByName( "OAK_TREE" ) );
		trees.push_back( shapePal->findShapeByName( "OAK2_TREE" ) );

		//trees.push_back( shapePal->findShapeByName( "BUSH" ) );
		//trees.push_back( shapePal->findShapeByName( "BUSH2" ) );
	}
	return trees[ (int)( (float)( trees.size() ) * rand() / RAND_MAX ) ];
}

MapRenderHelper* OutdoorGenerator::getMapRenderHelper() {
	// we need fog
	return MapRenderHelper::helpers[ MapRenderHelper::DEBUG_OUTDOOR_HELPER ];
}

// =====================================================
// =====================================================
// generation
//
void OutdoorGenerator::generate( Map *map, ShapePalette *shapePal ) {
	cerr << "generate 1" << endl;
  randomize();

	cerr << "generate 2" << endl;
  for( int i = 0; i < CELL_GROWTH_CYCLES; i++ ) {
		cerr << "generate 2, cycle " << i << endl;
    growCells();
  }

	cerr << "generate 3" << endl;
  phase = 1;
  findRooms();

	cerr << "generate 4" << endl;
  connectRooms();

	cerr << "generate 5" << endl;
	print();

	cerr << "generate 6" << endl;
	createGround();

	cerr << "generate Done" << endl;
}

void OutdoorGenerator::createGround() {
	// create the undulating ground
	float amp = 3.0f;
	float freq = 10.0f;
	for( int x = 0; x < MAP_WIDTH / OUTDOORS_STEP; x++ ) {
		for( int y = 0; y < MAP_DEPTH / OUTDOORS_STEP; y++ ) {
			// fixme: use a more sinoid function here
			// ground[x][y] = ( 1.0f * rand() / RAND_MAX );
			ground[x][y] = amp + 
				( amp * 
					sin( PI / ( 180.0f / (float)( x * OUTDOORS_STEP * freq ) ) ) * 
					cos( PI / ( 180.0f / (float)( y * OUTDOORS_STEP  * freq )) ) );
			if( ground[x][y] < 0 ) ground[x][y] = 0;
		}
	}
}

void OutdoorGenerator::randomize() {
  for( int x = 0; x < WIDTH_IN_NODES; x++ ) {
    for( int y = 0; y < DEPTH_IN_NODES; y++ ) {
      node[x][y].wall = true;
      node[x][y].island = false;
      node[x][y].room = -1;
    }
  }

  for( int x = 1; x < WIDTH_IN_NODES - 1; x++ ) {
    for( int y = 1; y < DEPTH_IN_NODES - 1; y++ ) {
      if( ( 1.0f * rand() / RAND_MAX ) < CLEAR_WALL_RATIO ) {
        node[ x ][ y ].wall = false;
      }
    }
  }
}

void OutdoorGenerator::growCells() {
  for( int x = 1; x < WIDTH_IN_NODES - 1; x++ ) {
    for( int y = 1; y < DEPTH_IN_NODES - 1; y++ ) {
      // count the neighbors
      int count = 0;
      if( node[x-1][y-1].wall ) count++;
      if( node[x  ][y-1].wall ) count++;
      if( node[x+1][y-1].wall ) count++;
      if( node[x-1][y  ].wall ) count++;
      if( node[x+1][y  ].wall ) count++;
      if( node[x-1][y+1].wall ) count++;
      if( node[x  ][y+1].wall ) count++;
      if( node[x+1][y+1].wall ) count++;

      // 4-5 rule (<4 starves, >5 lives)
      if( count < 4 ) {
        node[x][y].wall = false;
      }
      if( count > 5 ) {
        node[x][y].wall = true;
      }
    }
  }
}

void OutdoorGenerator::setSeen( bool b ) {
  for( int x = 0; x < WIDTH_IN_NODES; x++ ) {
    for( int y = 0; y < DEPTH_IN_NODES; y++ ) {
      node[x][y].seen = false;
    }
  }
}

bool OutdoorGenerator::canReach( int sx, int sy, int ex, int ey ) {
  if( sx == ex && sy == ey ) return true;
  if( sx > WIDTH_IN_NODES - 1 || sx < 1 || sy > DEPTH_IN_NODES - 1 || sy < 1 ||
      node[sx][sy].seen || 
      node[sx][sy].wall || 
      node[sx][sy].island ) return false;
  node[sx][sy].seen = true;
  return( canReach( sx + 1, sy, ex, ey ) || 
          canReach( sx - 1, sy, ex, ey ) ||
          canReach( sx, sy + 1, ex, ey ) ||
          canReach( sx, sy - 1, ex, ey ) ? true : false );
}

void OutdoorGenerator::findRooms() {  

  biggestRoom = roomCounter = 0;
  room[0].size = 0;
  room[0].x = room[0].y = 0;

  while( true ) {
		cerr << "\troomCounter=" << roomCounter << endl;

    // find the first empty space of an unclaimed room
    int sx, sy;
    sx = sy = -1;
    for( int x = 1; x < WIDTH_IN_NODES - 1; x++ ) {
      for( int y = 1; y < DEPTH_IN_NODES - 1; y++ ) {
        if( node[x][y].room == -1 &&
            !node[x][y].wall &&
            !node[x][y].island ) {
          sx = x;
          sy = y;
          break;
        }
      }
    }
    
    // if no more free space, we're done
    if( sx == -1 ) break;
    assert( roomCounter < MAX_ROOM_COUNT );
    
    // mark this spot
    node[sx][sy].room = roomCounter;

    // now find all other points that can reach this point
    for( int x = 1; x < WIDTH_IN_NODES - 1; x++ ) {
      for( int y = 1; y < DEPTH_IN_NODES - 1; y++ ) {
        if( !( node[x][y].wall ) &&
            !( node[x][y].island ) &&
            node[x][y].room == -1 ) {
          setSeen( false );
          if( canReach( x, y, sx, sy ) ) {
            node[x][y].room = roomCounter;
            room[roomCounter].size++;
            if( room[biggestRoom].size < room[roomCounter].size ) biggestRoom = roomCounter;
            room[roomCounter].x = x;
            room[roomCounter].y = y;
          }
        }
      }
    }

    if( room[roomCounter].x > 0 && room[roomCounter].y > 0 ) {
      roomCounter++;      
      room[roomCounter].size = 0;
      room[roomCounter].x = room[roomCounter].y = 0;
    }
  }
}

void OutdoorGenerator::connectPoints( int sx, int sy, int ex, int ey, bool isBiggestRoom ) {
  /**
   * Reach the center point, or the biggest room (if not in the biggest room.
   * this check is needed to ensure that the rare case of the biggest room not
   * touching the center will not happen.)
   */
  while( !( canReach( sx, sy, ex, ey ) ||
            ( !isBiggestRoom && node[sx][sy].room == biggestRoom ) ) ) {
    bool toTarget = true;
    if( phase == 1 && 1.0f * rand() / RAND_MAX < 0.3f ) {
      // meander
      int ox = sx;
      int oy = sy;
      if( 1.0f * rand() / RAND_MAX < 0.5f ) sx++;
      else sx--;
      if( 1.0f * rand() / RAND_MAX < 0.5f ) sy++;
      else sy--;

      if( sx > 0 && sy > 0 && sx < w - 1 && sy < h - 1 ) {
        toTarget = false;
      } else {
        sx = ox;
        sy = oy;
      }
    } 
    
    if( toTarget) {
      // to target!
      if( sx < ex ) sx++;
      else if( sx > ex ) sx--;
      else if( sy < ey ) sy++;
      else if( sy > ey ) sy--;
    }
    node[sx][sy].wall = node[sx][sy].island = false;
  }
}

void OutdoorGenerator::connectRooms() {
  // connect each room to the center of the map (except the room at the center)
  int cx = WIDTH_IN_NODES / 2;
  int cy = DEPTH_IN_NODES / 2;
  for( int i = 0; i < roomCounter; i++ ) {
    connectPoints( room[i].x, room[i].y, cx, cy, 
                   ( i == biggestRoom ? true : false ) );
  }
}

void OutdoorGenerator::print() {
  for( int x = 0; x < WIDTH_IN_NODES; x++ ) {
    for( int y = 0; y < DEPTH_IN_NODES; y++ ) {
      cerr << ( node[x][y].wall ? 'X' : 
                ( node[x][y].island ? '+' : 
                  //(char)( '0' + node[x][y].room ) ) );
                  ' ' ) );
    }
    cerr << endl;
  }
  cerr << endl << "Rooms:" << endl;
  for( int i = 0; i < roomCounter; i++ ) {
    cerr << "\tsize=" << room[i].size << ( i == biggestRoom ? " (biggest)" : "" ) << endl;
  }
}
