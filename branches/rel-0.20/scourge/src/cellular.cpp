/***************************************************************************
                          cellular.cpp  -  description
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
#include "cellular.h"
#include "util.h"

using namespace std;
  
CellularAutomaton::CellularAutomaton( int w, int h ) {
  this->w = w;
  this->h = h;
  this->roomCounter = 0;
  this->biggestRoom = 0;
  node = (NodePoint**)malloc( w * sizeof( NodePoint* ) );
  for( int x = 0; x < w; x++ ) {
    node[ x ] = (NodePoint*)malloc( h * sizeof( NodePoint ) );
  }
  phase = 1;
}

CellularAutomaton::~CellularAutomaton() {
  for( int x = 0; x < w; x++ ) {
    free( node[ x ] );
  }
  free( node );
}

void CellularAutomaton::generate( bool islandsEnabled, 
                                  bool removeSinglesEnabled,
																	int pathWidth ) {
  randomize();

  for( int i = 0; i < CELL_GROWTH_CYCLES; i++ ) {
    growCells();
  }

  phase = 1;
  findRooms();

  connectRooms();

  if( removeSinglesEnabled ) removeSingles();



  if( islandsEnabled ) {
    
    // add lava/rivers
    phase = 2;
    for( int x = 0; x < w; x++ ) {
      for( int y = 0; y < h; y++ ) {
        node[x][y].seen = false;
        node[x][y].room = -1;
      }
    }
  
    addIslands();
  
    growCellsIsland();
  
    addIslandLand();
  
		findRooms();
		connectRooms( pathWidth );
  
    //if( removeSinglesEnabled ) removeSingles();
  
    //print();
  }
}


#define isWall(x,y) ( x < 0 || y < 0 || x >= w || y >= h || node[x][y].wall )
#define isIsland(x,y) ( x < 0 || y < 0 || x >= w || y >= h || node[x][y].island )

#define DIST 4

void CellularAutomaton::addIslands() {
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
        if( test ) {
          node[x][y].island = true;        
        }
      }
    }
  }
  for( int x = 0; x < w; x++ ) {
    for( int y = 0; y < h; y++ ) {
      if( node[x][y].island && 0 == Util::dice( 5 ) ) {
        node[x][y].island = false;
      }
    }
  }
}

void CellularAutomaton::growCellsIsland() {
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
        node[x][y].room = -1;
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
void CellularAutomaton::addIslandLand() {
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

  // convert to land
  for( int x = 0; x < w; x++ ) {
    for( int y = 0; y < h; y++ ) {
      if( node[x][y].island && node[x][y].wall ) {
        node[x][y].island = node[x][y].wall = false;
        node[x][y].room = -1;
      }
    }
  }
  
}

void CellularAutomaton::randomize() {
  for( int x = 0; x < w; x++ ) {
    for( int y = 0; y < h; y++ ) {
      node[x][y].wall = true;
      node[x][y].island = false;
      node[x][y].room = -1;
    }
  }

  for( int x = 1; x < w - 1; x++ ) {
    for( int y = 1; y < h - 1; y++ ) {
      if( Util::roll( 0.0f, 1.0f ) < CLEAR_WALL_RATIO ) {
        node[ x ][ y ].wall = false;
      }
    }
  }
}

void CellularAutomaton::growCells() {
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

void CellularAutomaton::setSeen( bool b ) {
  for( int x = 0; x < w; x++ ) {
    for( int y = 0; y < h; y++ ) {
      node[x][y].seen = false;
    }
  }
}

bool CellularAutomaton::canReach( int sx, int sy, int ex, int ey ) {
  if( sx == ex && sy == ey ) return true;
  if( sx > w - 1 || sx < 1 || sy > h - 1 || sy < 1 ||
      node[sx][sy].seen || 
      node[sx][sy].wall || 
      node[sx][sy].island ) return false;
  node[sx][sy].seen = true;
  return( canReach( sx + 1, sy, ex, ey ) || 
          canReach( sx - 1, sy, ex, ey ) ||
          canReach( sx, sy + 1, ex, ey ) ||
          canReach( sx, sy - 1, ex, ey ) ? true : false );
}

void CellularAutomaton::findRooms() {  

  biggestRoom = roomCounter = 0;
  room[0].size = 0;
  room[0].x = room[0].y = 0;

  while( true ) {
    // find the first empty space of an unclaimed room
    int sx, sy;
    sx = sy = -1;
    for( int x = 1; x < w - 1; x++ ) {
      for( int y = 1; y < h - 1; y++ ) {
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
    for( int x = 1; x < w - 1; x++ ) {
      for( int y = 1; y < h - 1; y++ ) {
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

void CellularAutomaton::connectPoints( int sx, int sy, int ex, int ey, bool isBiggestRoom ) {
  /**
   * Reach the center point, or the biggest room (if not in the biggest room.
   * this check is needed to ensure that the rare case of the biggest room not
   * touching the center will not happen.)
   */
  while( !( canReach( sx, sy, ex, ey ) ||
            ( !isBiggestRoom && node[sx][sy].room == biggestRoom ) ) ) {
    bool toTarget = true;
    if( phase == 1 && Util::roll( 0.0f, 1.0f ) < 0.3f ) {
      // meander
      int ox = sx;
      int oy = sy;
      if( Util::roll( 0.0f, 1.0f ) < 0.5f ) sx++;
      else sx--;
      if( Util::roll( 0.0f, 1.0f ) < 0.5f ) sy++;
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

void CellularAutomaton::connectRooms( int pathWidth ) {
	if( pathWidth <= 1 ) {
		// connect each room to the center of the map (except the room at the center)
		int cx = w / 2;
		int cy = h / 2;
		for( int i = 0; i < roomCounter; i++ ) {
			connectPoints( room[i].x, room[i].y, cx, cy, 
										 ( i == biggestRoom ? true : false ) );
		}
	} else {
		// create a wide path off the island's inner land
		for( int i = 0; i < roomCounter; i++ ) {
			bool success = false;
			for( int dir = 0; dir < 4; dir++ ) {
				int rx = room[i].x;
				int ry = room[i].y;
				
				success = false;
				while( rx >= 1 && rx < w - pathWidth &&
							 ry >= 1 && ry < h - pathWidth ) {

					// stomp
					for( int n = 0; n < pathWidth; n++ ) {
						for( int m = 0; m < pathWidth; m++ ) {
							if( node[rx + n][ry + m].island ) {
								node[rx + n][ry + m].island = false;
								node[rx + n][ry + m].room = i;
							}
						}
					}

					// success?
					if( node[rx][ry].island == false && node[rx][ry].wall == false && node[rx][ry].room != i ) {
						success = true;
						break;
					}

					// move
					switch( dir ) {
					case 0: rx -= pathWidth; break;
					case 1: ry -= pathWidth; break;
					case 2: rx += pathWidth; break;
					case 3: ry += pathWidth; break;
					}
				}

				if( success ) break;
			}

			//if( !success ) {
				//cerr << "*** Warning: unable to create exit from this room: " << room[i].x << "," << room[i].y << endl;
			//}
		}
	}
}


void CellularAutomaton::makeAccessible( int x, int y ) {
	node[x][y].wall = false;
	int cx = w / 2;
	int cy = h / 2;
	connectPoints( x, y, cx, cy, false );
}

void CellularAutomaton::makeMinSpace( int size ) {
	// fix horizontal spaces
	for( int y = 0; y < h; y++ ) {
		for( int x = 0; x < w - size; x++ ) {		
			if( ( x == 0 || node[x - 1][y].wall ) &&
					!node[x][y].wall ) {
				for( int i = 0; i < size - 1; i++ ) {
					if( x + 1 < w - 1 ) {
						node[x + i][y].wall = false;
						for( int t = 0; t < size - 1; t++ ) {
							if( y + t < h && !node[x][y + t].wall ) node[x + i][y + t].wall = false;
						}
					}
				}
				x += size;
			}
		}		
	}

	// fix vertical spaces
	for( int x = 0; x < w; x++ ) {
		for( int y = 0; y < h - 2; y++ ) {
			if( ( y == 0 || node[x][y - 1].wall ) &&
					!node[x][y].wall ) {
				for( int i = 0; i < size - 1; i++ ) {
					if( y + 1 < h - 1 ) {
						node[x][y + i].wall = false;
						for( int t = 0; t < size - 1; t++ ) {
							if( x + t < w && !node[x + t][y].wall ) node[x + t][y + i].wall = false;
						}
					}
				}
				y += size;
			}
		}
	}

	removeSingles();
}

// Remove sharp edges because these don't render well. 
// Another option is to draw stalagmites instead of wall.
void CellularAutomaton::removeSingles() {
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
        /*
        if( node[x][y].island && 
            ( ( !(node[x + 1][y].island) && !(node[x - 1][y].island) ) ||
              ( !(node[x][y + 1].island) && !(node[x][y - 1].island) ) ||
              ( !(node[x + 1][y - 1].island) && !(node[x - 1][y + 1].island) ) ||
              ( !(node[x - 1][y - 1].island) && !(node[x + 1][y + 1].island) ) ) ) {
          node[x][y].island = false;
          hasSingles = true;
        }
        */
      }
    }
  }
}

void CellularAutomaton::print() {
  for( int x = 0; x < w; x++ ) {
    for( int y = 0; y < h; y++ ) {
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

