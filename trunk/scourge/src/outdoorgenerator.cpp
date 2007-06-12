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
#include "cellular.h"
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
  this->cellular[0][0] = new CellularAutomaton( WIDTH_IN_NODES, DEPTH_IN_NODES );
	this->cellular[1][0] = new CellularAutomaton( WIDTH_IN_NODES, DEPTH_IN_NODES );
	this->cellular[0][1] = new CellularAutomaton( WIDTH_IN_NODES, DEPTH_IN_NODES );
	this->cellular[1][1] = new CellularAutomaton( WIDTH_IN_NODES, DEPTH_IN_NODES );
}

OutdoorGenerator::~OutdoorGenerator() {
  delete cellular[0][0];
	delete cellular[0][1];
	delete cellular[1][0];
	delete cellular[1][1];
}

bool OutdoorGenerator::drawNodes( Map *map, ShapePalette *shapePal ) {

  updateStatus( _( "Loading theme" ) );
  if( map->getPreferences()->isDebugTheme() ) shapePal->loadDebugTheme();
	else shapePal->loadRandomTheme();

	map->setHeightMapEnabled( true );

  // set rooms
  doorCount = 0;
	roomCount = 0;
	for( int cx = 0; cx < 2; cx++ ) {
		for( int cy = 0; cy < 2; cy++ ) {
			//CellularAutomaton *c = cellular[cx][cy];
			room[ roomCount ].x = ( cx * WIDTH_IN_NODES * OUTDOORS_STEP ) / MAP_UNIT;
			room[ roomCount ].y = ( cy * DEPTH_IN_NODES * OUTDOORS_STEP ) / MAP_UNIT;
			room[ roomCount ].w = WIDTH_IN_NODES * OUTDOORS_STEP / MAP_UNIT;
			room[ roomCount ].h = DEPTH_IN_NODES * OUTDOORS_STEP / MAP_UNIT;
			room[ roomCount ].valueBonus = 0;
			roomCount++;
		}
	}
  roomMaxWidth = 0;
  roomMaxHeight = 0;
  objectCount = 7 + ( level / 8 ) * 5;
  monsters = true;


	// add mountains
	int offs = MAP_OFFSET / OUTDOORS_STEP;
	for( int x = 0; x < MAP_WIDTH / OUTDOORS_STEP; x++ ) {
		for( int y = 0; y < MAP_DEPTH / OUTDOORS_STEP; y++ ) {
			map->setGroundHeight( x, y, ground[x][y] );
			if( x >= offs && x < 2 * WIDTH_IN_NODES + offs &&
					y >= offs && y < 2 * DEPTH_IN_NODES + offs ) {
				int cx = ( x - offs ) / WIDTH_IN_NODES;
				int cy = ( y - offs ) / DEPTH_IN_NODES;
				int mx = ( x - offs ) % WIDTH_IN_NODES;
				int my = ( y - offs ) % DEPTH_IN_NODES;
				if( cellular[ cx ][ cy ]->getNode( mx, my )->wall ) {
					map->setGroundHeight( x, y, 10.0f + ( 6.0f * rand() / RAND_MAX ) );
				}
			} else {
				map->setGroundHeight( x, y, 10.0f + ( 6.0f * rand() / RAND_MAX ) );
			}
		}
	}

	// add trees
	for( int x = 0; x < MAP_WIDTH / OUTDOORS_STEP; x++ ) {
		for( int y = 0; y < MAP_DEPTH / OUTDOORS_STEP; y++ ) {
			if( x >= offs && x < 2 * WIDTH_IN_NODES + offs &&
					y >= offs && y < 2 * DEPTH_IN_NODES + offs ) {
				int cx = ( x - offs ) / WIDTH_IN_NODES;
				int cy = ( y - offs ) / DEPTH_IN_NODES;
				int mx = ( x - offs ) % WIDTH_IN_NODES;
				int my = ( y - offs ) % DEPTH_IN_NODES;
				if( cellular[ cx ][ cy ]->getNode( mx, my )->island ) {
					GLShape *shape = getRandomTreeShape( shapePal );
					int xx = x * OUTDOORS_STEP;
					int yy = y * OUTDOORS_STEP + shape->getHeight();
					if( !map->isBlocked( xx, yy, 0, 0, 0, 0, shape ) ) {
						map->setPosition( xx, yy, 0, shape );
					}
				}
			}
		}
	}
	
	// set ground texture
	int ex = MAP_WIDTH / OUTDOORS_STEP;
	int ey = MAP_DEPTH / OUTDOORS_STEP;
	for( int x = 0; x < ex; x += OUTDOOR_FLOOR_TEX_SIZE ) {
		for( int y = 0; y < ey; y += OUTDOOR_FLOOR_TEX_SIZE ) {
			GLuint tex = 0;
			int n = (int)( 3.0f * rand() / RAND_MAX );
			switch( n ) {
			case 0:
				tex = shapePal->getNamedTexture( "grass1" ); break;
			case 1:
				tex = shapePal->getNamedTexture( "grass2" ); break;
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

	return true;
}

typedef struct _ShapeLimit {
  GLShape *shape;
  float start, end;
} ShapeLimit;
vector<ShapeLimit> trees;

GLShape *OutdoorGenerator::getRandomTreeShape( ShapePalette *shapePal ) {
  if( trees.empty() ) {
    float offs = 0;
    for( int i = 1; i < shapePal->getShapeCount(); i++ ) {
      Shape *shape = shapePal->getShape( i );
      if( shape->getOutdoorWeight() > 0 ) {
        ShapeLimit limit;
        limit.start = offs;
        offs += shape->getOutdoorWeight();
        limit.end = offs;
        limit.shape = (GLShape*)shape;
        trees.push_back( limit );
      }
    }
  }
  assert( !trees.empty() );
  
  float roll = trees[ trees.size() - 1 ].end * rand() / RAND_MAX;

  // FIXME: implement binary search here
  for( unsigned int i = 0; i < trees.size(); i++ ) {
    if( trees[i].start <= roll && roll < trees[i].end ) {
      return trees[i].shape;
    }
  }
  cerr << "Unable to find tree shape! roll=" << roll << " max=" << trees[ trees.size() - 1 ].end << endl;
  cerr << "--------------------" << endl;
  for( unsigned int i = 0; i < trees.size(); i++ ) {
    cerr << "\t" << trees[i].shape->getName() << " " << trees[i].start << "-" << trees[i].end << endl;
  }
  cerr << "--------------------" << endl;
  return NULL;
  
  /*
	//return shapePal->findShapeByName( "DEBUG_TREE" );
	if( trees.size() == 0 ) {
		// this should be in config file
		trees.push_back( shapePal->findShapeByName( "PINE_TREE" ) );
		trees.push_back( shapePal->findShapeByName( "WILLOW_TREE" ) );
		trees.push_back( shapePal->findShapeByName( "OAK_TREE" ) );
		trees.push_back( shapePal->findShapeByName( "OAK2_TREE" ) );
		trees.push_back( shapePal->findShapeByName( "BUSH" ) );
		trees.push_back( shapePal->findShapeByName( "BUSH2" ) );
	}
	return trees[ (int)( (float)( trees.size() ) * rand() / RAND_MAX ) ];
  */
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
  cellular[0][0]->generate( true, true );
	cellular[0][0]->makeAccessible( WIDTH_IN_NODES - 1, DEPTH_IN_NODES / 2 );
	cellular[0][0]->makeAccessible( WIDTH_IN_NODES / 2, DEPTH_IN_NODES - 1 );
	cellular[0][0]->makeMinSpace( 4 );
	cellular[0][0]->print();
	
	cellular[1][0]->generate( true, true );
	cellular[1][0]->makeAccessible( 0, DEPTH_IN_NODES / 2 );
	cellular[1][0]->makeAccessible( WIDTH_IN_NODES / 2, DEPTH_IN_NODES - 1 );
	cellular[1][0]->makeMinSpace( 4 );
	cellular[1][0]->print();

	cellular[0][1]->generate( true, true );
	cellular[0][1]->makeAccessible( WIDTH_IN_NODES - 1, DEPTH_IN_NODES / 2 );
	cellular[0][1]->makeAccessible( WIDTH_IN_NODES / 2, 0 );
	cellular[0][1]->makeMinSpace( 4 );
	cellular[0][1]->print();

	cellular[1][1]->generate( true, true );
	cellular[1][1]->makeAccessible( 0, DEPTH_IN_NODES / 2 );
	cellular[1][1]->makeAccessible( WIDTH_IN_NODES / 2, 0 );
	cellular[1][1]->makeMinSpace( 4 );
	cellular[1][1]->print();
	createGround();
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


