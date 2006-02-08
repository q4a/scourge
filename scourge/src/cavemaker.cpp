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
  int dungeonLevel = level / 8;
  this->w = 25 + dungeonLevel * 4;
  this->h = 25 + dungeonLevel * 4;
  int maxw = ( MAP_WIDTH - ( 2 * MAP_OFFSET ) ) / CAVE_CHUNK_SIZE;
  int maxh = ( MAP_DEPTH - ( 2 * MAP_OFFSET ) ) / CAVE_CHUNK_SIZE;
  if( this->w > maxw ) this->w = maxw;
  if( this->h > maxh ) this->h = maxh;
  cerr << "CaveMaker: dungeonLevel=" << dungeonLevel << " size=" << w << "x" << h << endl;
  this->roomCounter = 0;
  this->biggestRoom = 0;
  node = (NodePoint**)malloc( w * sizeof( NodePoint* ) );
  for( int x = 0; x < w; x++ ) {
    node[ x ] = (NodePoint*)malloc( h * sizeof( NodePoint ) );
  }

  // reasonable defaults
  TerrainGenerator::doorCount = 0;
  TerrainGenerator::roomCount = 1;
  TerrainGenerator::room[0].x = room[0].y = 0;
  TerrainGenerator::room[0].w = ( this->w * CAVE_CHUNK_SIZE ) / MAP_UNIT;
  TerrainGenerator::room[0].h = ( this->h * CAVE_CHUNK_SIZE ) / MAP_UNIT;
  TerrainGenerator::room[0].valueBonus = 0;
  TerrainGenerator::roomMaxWidth = 0;
  TerrainGenerator::roomMaxHeight = 0;
  TerrainGenerator::objectCount = 7 + dungeonLevel * 5;
  TerrainGenerator::monsters = true;

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

  addIslands();

  growCellsIsland();

  addIslandLand();

  print();
}


#define isWall(x,y) ( x < 0 || y < 0 || x >= w || y >= h || node[x][y].wall )
#define isIsland(x,y) ( x < 0 || y < 0 || x >= w || y >= h || node[x][y].island )
#define setCaveShape(map,x,y,index) ( map->setPosition( MAP_OFFSET + (x * CAVE_CHUNK_SIZE), MAP_OFFSET + ( (y + 1) * CAVE_CHUNK_SIZE ), 0, GLCaveShape::getShape(index) ) )
#define setCaveFloorShape(map,x,y,index) ( map->setFloorPosition( MAP_OFFSET + (x * CAVE_CHUNK_SIZE), MAP_OFFSET + ( (y + 1) * CAVE_CHUNK_SIZE ), GLCaveShape::getShape(index) ) )

bool CaveMaker::drawNodes( Map *map, ShapePalette *shapePal ) {

  shapePal->loadRandomCaveTheme();

  string ref = WallTheme::themeRefName[ WallTheme::THEME_REF_PASSAGE_FLOOR ];
  GLuint *floorTextureGroup = shapePal->getCurrentTheme()->getTextureGroup( ref );
  map->setFloor( CAVE_CHUNK_SIZE, CAVE_CHUNK_SIZE, floorTextureGroup[ GLShape::TOP_SIDE ] );

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
      if( isIsland( x, y ) ) {
        int lavaIndex = GLCaveShape::LAVA_NONE;

        if( isIsland( x - 1, y ) &&
            isIsland( x + 1, y ) &&
            isIsland( x, y - 1 ) &&
            isIsland( x, y + 1 ) ) {
          lavaIndex = GLCaveShape::LAVA_NONE;
        } else if( !isIsland( x - 1, y ) &&
                   isIsland( x + 1, y ) &&
                   isIsland( x, y - 1 ) &&
                   isIsland( x, y + 1 ) ) {
          lavaIndex = GLCaveShape::LAVA_SIDE_W;
        } else if( isIsland( x - 1, y ) &&
                   !isIsland( x + 1, y ) &&
                   isIsland( x, y - 1 ) &&
                   isIsland( x, y + 1 ) ) {
          lavaIndex = GLCaveShape::LAVA_SIDE_E;
        } else if( isIsland( x - 1, y ) &&
                   isIsland( x + 1, y ) &&
                   !isIsland( x, y - 1 ) &&
                   isIsland( x, y + 1 ) ) {
          lavaIndex = GLCaveShape::LAVA_SIDE_N;
        } else if( isIsland( x - 1, y ) &&
                   isIsland( x + 1, y ) &&
                   isIsland( x, y - 1 ) &&
                   !isIsland( x, y + 1 ) ) {
          lavaIndex = GLCaveShape::LAVA_SIDE_S;
        } else if( isIsland( x - 1, y ) &&
                   isIsland( x, y - 1 ) ) {
          lavaIndex = GLCaveShape::LAVA_OUTSIDE_TURN_SE;
        } else if( isIsland( x + 1, y ) &&
                   isIsland( x, y - 1 ) ) {
          lavaIndex = GLCaveShape::LAVA_OUTSIDE_TURN_SW;
        } else if( isIsland( x - 1, y ) &&
                   isIsland( x, y + 1 ) ) {
          lavaIndex = GLCaveShape::LAVA_OUTSIDE_TURN_NE;
        } else if( isIsland( x + 1, y ) &&
                   isIsland( x, y + 1 ) ) {
          lavaIndex = GLCaveShape::LAVA_OUTSIDE_TURN_NW;
        } else if( isIsland( x - 1, y ) ) {
          lavaIndex = GLCaveShape::LAVA_U_E;
        } else if( isIsland( x + 1, y ) ) {
          lavaIndex = GLCaveShape::LAVA_U_W;
        } else if( isIsland( x, y - 1 ) ) {
          lavaIndex = GLCaveShape::LAVA_U_S;
        } else if( isIsland( x, y + 1 ) ) {
          lavaIndex = GLCaveShape::LAVA_U_N;
        } else {
          lavaIndex = GLCaveShape::LAVA_ALL;
        }



        setCaveShape( map, x, y, lavaIndex ); 
      } else if( node[x][y].wall ) {
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
              setCaveFloorShape( map, x, y, GLCaveShape::CAVE_INDEX_FLOOR );
            } else {
              setCaveShape( map, x, y, GLCaveShape::CAVE_INDEX_NW );
              setCaveFloorShape( map, x, y, GLCaveShape::CAVE_INDEX_FLOOR );
            }
          } else if( !isWall( x + 1, y ) ) {
            if( isWall( x, y - 1 ) &&
                isWall( x, y + 1 ) ) {
              setCaveShape( map, x, y, GLCaveShape::CAVE_INDEX_E );
            } else if( isWall( x, y - 1 ) ) {
              setCaveShape( map, x, y, GLCaveShape::CAVE_INDEX_SE );
              setCaveFloorShape( map, x, y, GLCaveShape::CAVE_INDEX_FLOOR );
            } else {
              setCaveShape( map, x, y, GLCaveShape::CAVE_INDEX_NE );
              setCaveFloorShape( map, x, y, GLCaveShape::CAVE_INDEX_FLOOR );
            }
          } else if( !isWall( x, y - 1 ) ) {
            if( isWall( x - 1, y ) &&
                isWall( x + 1, y ) ) {
              setCaveShape( map, x, y, GLCaveShape::CAVE_INDEX_N );
            } else if( isWall( x - 1, y ) ) {
              setCaveShape( map, x, y, GLCaveShape::CAVE_INDEX_NE );
              setCaveFloorShape( map, x, y, GLCaveShape::CAVE_INDEX_FLOOR );
            } else {
              setCaveShape( map, x, y, GLCaveShape::CAVE_INDEX_NW );
              setCaveFloorShape( map, x, y, GLCaveShape::CAVE_INDEX_FLOOR );
            }
          } else if( !isWall( x, y + 1 ) ) {
            if( isWall( x - 1, y ) &&
                isWall( x + 1, y ) ) {
              setCaveShape( map, x, y, GLCaveShape::CAVE_INDEX_S );
            } else if( isWall( x - 1, y ) ) {
              setCaveShape( map, x, y, GLCaveShape::CAVE_INDEX_SE );
              setCaveFloorShape( map, x, y, GLCaveShape::CAVE_INDEX_FLOOR );
            } else {
              setCaveShape( map, x, y, GLCaveShape::CAVE_INDEX_SW );
              setCaveFloorShape( map, x, y, GLCaveShape::CAVE_INDEX_FLOOR );
            }
          }
        }
      } else {
        setCaveFloorShape( map, x, y, GLCaveShape::CAVE_INDEX_FLOOR );
      }
    }
  }

  return true;
}

#define DIST 4

void CaveMaker::addIslands() {
  for( int x = 0; x < w; x++ ) {
    for( int y = 0; y < h; y++ ) {
      // does it qualify?
      if( !node[x][y].wall ) {
        bool test = true;
        for( int i = 0; i < DIST; i++ ) {
          if( isWall( x + i, y ) ||
              isWall( x - i, y ) ||
              isWall( x, y + i ) ||
              isWall( x, y - i ) ||
              isWall( x + i, y + i ) ||
              isWall( x - i, y + i ) ||
              isWall( x + i, y - i ) ||
              isWall( x - i, y - i ) ) {
            test = false;          
          }
        }
        if( test ) node[x][y].island = true;        
      }
    }
  }
  for( int x = 0; x < w; x++ ) {
    for( int y = 0; y < h; y++ ) {
      if( node[x][y].island && 0 == (int)( 5.0f * rand() / RAND_MAX ) ) {
        node[x][y].island = false;
      }
    }
  }
}

void CaveMaker::growCellsIsland() {
  for( int x = 1; x < w - 1; x++ ) {
    for( int y = 1; y < h - 1; y++ ) {

      // count the neighbors
      int count = 0;
      if( node[x-1][y-1].island ) count++;
      if( node[x  ][y-1].island ) count++;
      if( node[x+1][y-1].island ) count++;
      if( node[x-1][y  ].island ) count++;
      if( node[x+1][y  ].island ) count++;
      if( node[x-1][y+1].island ) count++;
      if( node[x  ][y+1].island ) count++;
      if( node[x+1][y+1].island ) count++;

      // 4-5 rule (<4 starves, >5 lives)
      if( count < 4 ) {
        node[x][y].island = false;
      }
      if( count > 5 ) {
        node[x][y].island = true;
      }
    }
  }
}

#define RIVER 3

/**
 * Create "rivers". This code uses a hack:
 * it uses node.wall to track where the land will be.
 */
void CaveMaker::addIslandLand() {
  for( int x = 0; x < w; x++ ) {
    for( int y = 0; y < h; y++ ) {
      // does it qualify?
      if( node[x][y].island ) {
        bool test = true;
        for( int i = 0; i < RIVER; i++ ) {
          if( !isIsland( x + i, y ) ||
              !isIsland( x - i, y ) ||
              !isIsland( x, y + i ) ||
              !isIsland( x, y - i ) ||
              !isIsland( x + i, y + i ) ||
              !isIsland( x - i, y + i ) ||
              !isIsland( x + i, y - i ) ||
              !isIsland( x - i, y - i ) ) {
            test = false;          
          }
        }
        if( test ) node[x][y].wall = true;
      }
    }
  }
  
  growCellsIsland();

  // now make sure that every land within an island is accesible


  // convert to land
  for( int x = 0; x < w; x++ ) {
    for( int y = 0; y < h; y++ ) {
      if( node[x][y].island && node[x][y].wall ) {
        node[x][y].island = node[x][y].wall = false;
      }
    }
  }
  
}

void CaveMaker::randomize() {
  for( int x = 0; x < w; x++ ) {
    for( int y = 0; y < h; y++ ) {
      node[x][y].wall = true;
      node[x][y].island = false;
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
  room[0].size = 0;
  room[0].x = room[0].y = 0;
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
    room[roomCounter].size = 0;
    room[roomCounter].x = room[roomCounter].y = 0;
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

// Remove sharp edges because these don't render well. 
// Another option is to draw stalagmites instead of wall.
void CaveMaker::removeSingles() {
  bool hasSingles = true;
  while( hasSingles ) {
    hasSingles = false;
    for( int x = 1; x < w - 1; x++ ) {
      for( int y = 1; y < h - 1; y++ ) {
        if( node[x][y].wall && 
            ( ( !(node[x + 1][y].wall) && !(node[x - 1][y].wall) ) ||
              ( !(node[x][y + 1].wall) && !(node[x][y - 1].wall) ) ||
              ( !(node[x + 1][y - 1].wall) && !(node[x - 1][y + 1].wall) ) ||
              ( !(node[x - 1][y - 1].wall) && !(node[x + 1][y + 1].wall) ) ) ) {
          node[x][y].wall = false;
          hasSingles = true;
        }
      }
    }
  }
}

void CaveMaker::print() {
  for( int x = 0; x < w; x++ ) {
    for( int y = 0; y < h; y++ ) {
      cerr << ( node[x][y].wall ? "X" : ( node[x][y].island ? "+" : " " ) );
    }
    cerr << endl;
  }
  cerr << endl << "Rooms:" << endl;
  for( int i = 0; i < roomCounter; i++ ) {
    cerr << "\tsize=" << room[i].size << ( i == biggestRoom ? " (biggest)" : "" ) << endl;
  }
}

void CaveMaker::addFurniture(Map *map, ShapePalette *shapePal) {
  // add tables, chairs, etc.
  addItemsInEveryRoom( RpgItem::getItemByName("Table"), 5 );
  addItemsInEveryRoom( RpgItem::getItemByName("Chair"), 15 );  

  // add some magic pools
  DisplayInfo di;
  for( int i = 0; i < 8; i++ ) {
    if( 0 == (int)( 0.0f * rand() / RAND_MAX ) ) {
      MagicSchool *ms = MagicSchool::getRandomSchool();
      di.red = ms->getDeityRed();
      di.green = ms->getDeityGreen();
      di.blue = ms->getDeityBlue();

      Location *pos = addShapeInRoom( scourge->getShapePalette()->findShapeByName("POOL"), 0, &di );
      if( pos ) {
        // store pos->deity in scourge
        scourge->addDeityLocation( pos, ms );
      }
    }
  }
}

MapRenderHelper *CaveMaker::getMapRenderHelper() {
  return MapRenderHelper::helpers[ MapRenderHelper::CAVE_HELPER ];
}

