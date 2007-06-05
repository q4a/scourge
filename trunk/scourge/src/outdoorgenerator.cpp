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
  this->cellular = new CellularAutomaton( WIDTH_IN_NODES, DEPTH_IN_NODES );

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
  delete cellular;
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
			if( cellular->getNode( x, y )->wall ) {
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
  cellular->generate();
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

