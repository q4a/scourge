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
#include "cellular.h"

using namespace std;
  
CaveMaker::CaveMaker( Scourge *scourge, int level, int depth, int maxDepth,
                      bool stairsDown, bool stairsUp, 
                      Mission *mission) :
TerrainGenerator( scourge, level, depth, maxDepth, stairsDown, stairsUp, mission, 13 ) {
  int dungeonLevel = level / 8;
  this->w = 25 + dungeonLevel * 4;
  this->h = 25 + dungeonLevel * 4;
  int maxw = ( MAP_WIDTH - ( 2 * MAP_OFFSET ) ) / CAVE_CHUNK_SIZE;
  int maxh = ( MAP_DEPTH - ( 2 * MAP_OFFSET ) ) / CAVE_CHUNK_SIZE;
  if( this->w > maxw ) this->w = maxw;
  if( this->h > maxh ) this->h = maxh;

  cellular = new CellularAutomaton( w, h );

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
  delete cellular;
}

void CaveMaker::generate( Map *map, ShapePalette *shapePal ) {
  cellular->generate( true, true );
}


#define isWall(x,y) ( x < 0 || y < 0 || x >= w || y >= h || cellular->getNode( x, y )->wall )
#define isIsland(x,y) ( x < 0 || y < 0 || x >= w || y >= h || cellular->getNode( x, y )->island )
#define setCaveShape(map,x,y,index) ( map->setPosition( MAP_OFFSET + (x * CAVE_CHUNK_SIZE), MAP_OFFSET + ( (y + 1) * CAVE_CHUNK_SIZE ), 0, GLCaveShape::getShape(index) ) )
#define setCaveFloorShape(map,x,y,index) ( map->setFloorPosition( MAP_OFFSET + (x * CAVE_CHUNK_SIZE), MAP_OFFSET + ( (y + 1) * CAVE_CHUNK_SIZE ), GLCaveShape::getShape(index) ) )

bool CaveMaker::drawNodes( Map *map, ShapePalette *shapePal ) {

	map->initForCave();

	for( int y = CAVE_CHUNK_SIZE; y < MAP_DEPTH; y += CAVE_CHUNK_SIZE ) {
    for( int x = 0; x < MAP_WIDTH - CAVE_CHUNK_SIZE; x += CAVE_CHUNK_SIZE ) {
      if( x < MAP_OFFSET || x >= MAP_OFFSET + w * CAVE_CHUNK_SIZE ||
          y < MAP_OFFSET + CAVE_CHUNK_SIZE || y >= MAP_OFFSET + ( h + 1 ) * CAVE_CHUNK_SIZE) {
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
        } else if( isIsland( x + 1, y ) &&
                   isIsland( x - 1, y ) ) {
          lavaIndex = GLCaveShape::LAVA_SIDES_NS;
        } else if( isIsland( x, y + 1 ) &&
                   isIsland( x, y - 1 ) ) {
          lavaIndex = GLCaveShape::LAVA_SIDES_EW;
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
      } else if( cellular->getNode( x, y )->wall ) {
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

void CaveMaker::printMaze() {
	cellular->print();
}
