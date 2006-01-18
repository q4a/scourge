/***************************************************************************
                          cavemaker.cpp  -  description
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
#include "cavemaker.h"
#include "render/renderlib.h"
#include "rpg/rpglib.h"
#include "scourge.h"
#include "shapepalette.h"
#include "board.h"
#include "gui/progress.h"
#include "item.h"
#include "creature.h"

using namespace std;
  
CaveMaker::CaveMaker( Scourge *scourge, int level, int depth, 
                      bool stairsDown, bool stairsUp, 
                      Mission *mission) :
TerrainGenerator( scourge, level, depth, stairsDown, stairsUp, mission, 10 ) {
  this->w = ( MAP_WIDTH - ( 2 * MAP_OFFSET ) ) / CAVE_CHUNK_SIZE;
  this->h = ( MAP_DEPTH - ( 2 * MAP_OFFSET ) ) / CAVE_CHUNK_SIZE;
  cerr << "CaveMaker: size=" << w << "x" << h << endl;
  this->roomCounter = 0;
  this->biggestRoom = 0;
  node = (NodePoint**)malloc( w * sizeof( NodePoint* ) );
  for( int x = 0; x < w; x++ ) {
    node[ x ] = (NodePoint*)malloc( h * sizeof( NodePoint ) );
  }
}

CaveMaker::~CaveMaker() {
  for( int x = 0; x < w; x++ ) {
    free( node[ x ] );
  }
  free( node );
}

void CaveMaker::generate( Map *map, ShapePalette *shapePal ) {
  randomize();

  for( int i = 0; i < CELL_GROWTH_CYCLES; i++ ) {
    growCells();
  }

  findRooms();

  connectRooms();

  removeSingles();

  print();

  drawOnMap( map, shapePal );
}


#define isWall(x,y) ( x < 0 || y < 0 || x >= w || y >= h || node[x][y].wall )
#define setCaveShape(map,x,y,index) ( map->setPosition( MAP_OFFSET + (x * CAVE_CHUNK_SIZE), MAP_OFFSET + ( (y + 1) * CAVE_CHUNK_SIZE ), 0, GLCaveShape::getShape(index) ) )
#define setCaveFloorShape(map,x,y,index) ( map->setFloorPosition( MAP_OFFSET + (x * CAVE_CHUNK_SIZE), MAP_OFFSET + ( (y + 1) * CAVE_CHUNK_SIZE ), GLCaveShape::getShape(index) ) )

void CaveMaker::drawOnMap( Map *map, ShapePalette *shapePal ) {
  for( int x = 0; x < MAP_WIDTH - CAVE_CHUNK_SIZE; x+=CAVE_CHUNK_SIZE ) {
    for( int y = CAVE_CHUNK_SIZE; y < MAP_DEPTH - CAVE_CHUNK_SIZE; y+=CAVE_CHUNK_SIZE ) {
      if( x < MAP_OFFSET || y < CAVE_CHUNK_SIZE || 
          x >= MAP_WIDTH - MAP_OFFSET || y >= MAP_DEPTH - MAP_OFFSET ) {
        map->setPosition( x, y, 0, GLCaveShape::getShape( GLCaveShape::CAVE_INDEX_BLOCK ) );
      }
    }
  }

  for( int x = 0; x < w; x++ ) {
    for( int y = 0; y < h; y++ ) {
      if( node[x][y].wall ) {
        if( isWall( x - 1, y ) &&
            isWall( x + 1, y ) &&
            isWall( x, y - 1 ) &&
            isWall( x, y + 1 ) ) {
          if( !isWall( x - 1, y - 1 ) &&
              !isWall( x + 1, y + 1 ) ) {
            setCaveShape( map, x, y, GLCaveShape::CAVE_INDEX_CROSS_NW );
          } else if( !isWall( x + 1, y - 1 ) &&
                     !isWall( x - 1, y + 1 ) ) {
            setCaveShape( map, x, y, GLCaveShape::CAVE_INDEX_CROSS_NE );
          } else if( !isWall( x - 1, y - 1 ) ) {
            setCaveShape( map, x, y, GLCaveShape::CAVE_INDEX_INV_NW );
          } else if( !isWall( x + 1, y - 1 ) ) { 
            setCaveShape( map, x, y, GLCaveShape::CAVE_INDEX_INV_NE );
          } else if( !isWall( x - 1, y + 1 ) ) {
            setCaveShape( map, x, y, GLCaveShape::CAVE_INDEX_INV_SW );
          } else if( !isWall( x + 1, y + 1 ) ) {
            setCaveShape( map, x, y, GLCaveShape::CAVE_INDEX_INV_SE );
          } else {
            setCaveShape( map, x, y, GLCaveShape::CAVE_INDEX_BLOCK );
          }
        } else {
          if( !isWall( x - 1, y ) ) {
            if( isWall( x, y - 1 ) &&
                isWall( x, y + 1 ) ) {
              setCaveShape( map, x, y, GLCaveShape::CAVE_INDEX_W );
            } else if( isWall( x, y - 1 ) ) {
              setCaveShape( map, x, y, GLCaveShape::CAVE_INDEX_SW );
            } else {
              setCaveShape( map, x, y, GLCaveShape::CAVE_INDEX_NW );
            }
          } else if( !isWall( x + 1, y ) ) {
            if( isWall( x, y - 1 ) &&
                isWall( x, y + 1 ) ) {
              setCaveShape( map, x, y, GLCaveShape::CAVE_INDEX_E );
            } else if( isWall( x, y - 1 ) ) {
              setCaveShape( map, x, y, GLCaveShape::CAVE_INDEX_SE );
            } else {
              setCaveShape( map, x, y, GLCaveShape::CAVE_INDEX_NE );
            }
          } else if( !isWall( x, y - 1 ) ) {
            if( isWall( x - 1, y ) &&
                isWall( x + 1, y ) ) {
              setCaveShape( map, x, y, GLCaveShape::CAVE_INDEX_N );
            } else if( isWall( x - 1, y ) ) {
              setCaveShape( map, x, y, GLCaveShape::CAVE_INDEX_NE );
            } else {
              setCaveShape( map, x, y, GLCaveShape::CAVE_INDEX_NW );
            }
          } else if( !isWall( x, y + 1 ) ) {
            if( isWall( x - 1, y ) &&
                isWall( x + 1, y ) ) {
              setCaveShape( map, x, y, GLCaveShape::CAVE_INDEX_S );
            } else if( isWall( x - 1, y ) ) {
              setCaveShape( map, x, y, GLCaveShape::CAVE_INDEX_SE );
            } else {
              setCaveShape( map, x, y, GLCaveShape::CAVE_INDEX_SW );
            }
          }
        }
      } else {
        setCaveFloorShape( map, x, y, GLCaveShape::CAVE_INDEX_FLOOR );
      }
    }
  }
}

void CaveMaker::randomize() {
  for( int x = 0; x < w; x++ ) {
    for( int y = 0; y < h; y++ ) {
      node[x][y].wall = true;
      node[x][y].room = -1;
    }
  }

  for( int x = 1; x < w - 1; x++ ) {
    for( int y = 1; y < h - 1; y++ ) {
      if( ( 1.0f * rand() / RAND_MAX ) < CLEAR_WALL_RATIO ) {
        node[ x ][ y ].wall = false;
      }
    }
  }
}

void CaveMaker::growCells() {
  for( int x = 1; x < w - 1; x++ ) {
    for( int y = 1; y < h - 1; y++ ) {
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

void CaveMaker::setSeen( bool b ) {
  for( int x = 0; x < w; x++ ) {
    for( int y = 0; y < h; y++ ) {
      node[x][y].seen = false;
    }
  }
}

bool CaveMaker::canReach( int sx, int sy, int ex, int ey ) {
  if( sx == ex && sy == ey ) return true;
  if( sx > w - 1 || sx < 1 || sy > h - 1 || sy < 1 ||
      node[sx][sy].seen || node[sx][sy].wall ) return false;
  node[sx][sy].seen = true;
  return( canReach( sx + 1, sy, ex, ey ) || 
          canReach( sx - 1, sy, ex, ey ) ||
          canReach( sx, sy + 1, ex, ey ) ||
          canReach( sx, sy - 1, ex, ey ) ? true : false );
}

void CaveMaker::findRooms() {
  biggestRoom = roomCounter = 0;
  while( true ) {
    // find the first empty space of an unclaimed room
    int sx, sy;
    sx = sy = -1;
    for( int x = 1; x < w - 1; x++ ) {
      for( int y = 1; y < h - 1; y++ ) {
        if( !(node[x][y].wall) &&
            node[x][y].room == -1 ) {
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
    for( int x = 1; x < w - 1; x++ ) {
      for( int y = 1; y < h - 1; y++ ) {
        if( !(node[x][y].wall) &&
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

    roomCounter++;      
  }
}

void CaveMaker::connectPoints( int sx, int sy, int ex, int ey, bool isBiggestRoom ) {
  /**
   * Reach the center point, or the biggest room (if not in the biggest room.
   * this check is needed to ensure that the rare case of the biggest room not
   * touching the center will not happen.)
   */
  while( !( canReach( sx, sy, ex, ey ) ||
            ( !isBiggestRoom && node[sx][sy].room == biggestRoom ) ) ) {
    bool toTarget = true;
    if( 1.0f * rand() / RAND_MAX < 0.3f ) {
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
    node[sx][sy].wall = false;
  }
}

void CaveMaker::connectRooms() {
  // connect each room to the center of the map (except the room at the center)
  int cx = w / 2;
  int cy = h / 2;
  for( int i = 0; i < roomCounter; i++ ) {
    connectPoints( room[i].x, room[i].y, cx, cy, 
                   ( i == biggestRoom ? true : false ) );
  }
}

// remove sharp edges
void CaveMaker::removeSingles() {
  for( int x = 1; x < w - 1; x++ ) {
    for( int y = 1; y < h - 1; y++ ) {
      if( node[x][y].wall && 
          ( ( !(node[x + 1][y].wall) && !(node[x - 1][y].wall) ) ||
            ( !(node[x][y + 1].wall) && !(node[x][y - 1].wall) ) ) ) {
        node[x][y].wall = false;
      }
    }
  }
}

void CaveMaker::print() {
  for( int x = 0; x < w; x++ ) {
    for( int y = 0; y < h; y++ ) {
      cerr << ( node[x][y].wall ? "X" : " " );
    }
    cerr << endl;
  }
  cerr << endl << "Rooms:" << endl;
  for( int i = 0; i < roomCounter; i++ ) {
    cerr << "\tsize=" << room[i].size << ( i == biggestRoom ? " (biggest)" : "" ) << endl;
  }
}

