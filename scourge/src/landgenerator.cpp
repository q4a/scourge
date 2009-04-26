/***************************************************************************
                landgenerator.cpp  -  Generates outdoor maps
                             -------------------
    begin                : Sat March 28, 2009
    copyright            : (C) 2009 by Gabor Torok
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
#include "landgenerator.h"
#include "render/map.h"
#include "shapepalette.h"
#include "scourge.h"
#include "board.h"
#include "render/maprender.h"
#include "render/maprenderhelper.h"
#include "render/glshape.h"
#include "render/virtualshape.h"
#include "cellular.h"
#include "sqbinding/sqbinding.h"
#include "rpg/rpglib.h"
#include "creature.h"
#include <vector>

using namespace std;


/**
 * This generator generates a 296x296 (quarter) map. Before calling generate(), be sure to call setRegion().
 * 
 * A region is a 32x32 pixel section of the map bitmap. The map bitmap is broken into 128x128 pixel section.
 * (See: scourge_data/mapgrid/world/) There are 4x4 regions per bitmap grid (128/32). So for example, calling
 * setRegion( 6, 39 ) would load map_100.png ( (int)( 39 / 4 ) * 11 + (int)( 6 / 4 ) ). Then inside map_100.png, 
 * the region shown is 2, 1.
 * 
 * A Map object can thus contain 4 296x296 map section (since it's 592x592 units large) which is the max number
 * of sections seen by the player at one time. (If the player reaches the corner of one 296x296 section.) 
 * 
 * When the player reaches the corner, existing sections will be passivated (saved) and new ones will be loaded or 
 * generated by this class. The section(s) that remain in memory (drawn on the Map class) will be shuffled around. 
 * For example, when traveling north, the southern section will be passivated, the northern section becomes the southern
 * one and a new section is loaded/generated into the north part of the in-memory Map class.
 * 
 * Also remember to call setMapPosition() before calling generate to denote where in the map the new quarter
 * should get rendered to. The arguments to setMapPosition() are in OUTDOORS_STEP units (so, 75, 75 is the fourth quarter.)
 */
LandGenerator::LandGenerator( Scourge *scourge, int level, int depth, int maxDepth,
                              bool stairsDown, bool stairsUp,
                              Mission *mission ) :
		TerrainGenerator( scourge, level, depth, maxDepth, stairsDown, stairsUp, mission, 13 ) {
	// init the ground
	for ( int x = 0; x < QUARTER_WIDTH_IN_NODES; x++ ) {
		for ( int y = 0; y < QUARTER_DEPTH_IN_NODES; y++ ) {
			ground[x][y] = 0;
		}
	}
	this->cellular = new CellularAutomaton( QUARTER_WIDTH_IN_NODES, QUARTER_DEPTH_IN_NODES );
	
	this->regionX = this->regionY = 0;
	this->mapPosX = this->mapPosY = 0;
	this->willAddParty = false;
	this->bitmapIndex = -1;
	this->bitmapSurface = NULL;
}

LandGenerator::~LandGenerator() {
	delete cellular;
}


bool LandGenerator::drawNodes( Map *map, ShapePalette *shapePal ) {
	updateStatus( _( "Loading theme" ) );
	if ( map->getPreferences()->isDebugTheme() ) shapePal->loadDebugTheme();
	else shapePal->loadTheme( "outdoor" );

	map->setHeightMapEnabled( true );

	// add mountains
	for ( int x = 0; x < QUARTER_WIDTH_IN_NODES; x++ ) {
		for ( int y = 0; y < QUARTER_DEPTH_IN_NODES; y++ ) {
			if ( cellular->getNode( x, y )->wall ) {
				map->setGroundHeight( mapPosX + x, mapPosY + y, Util::roll( 14.0f, 20.0f ) );
			} else if ( cellular->getNode( x, y )->water ) {
				map->setGroundHeight( mapPosX + x, mapPosY + y, -Util::roll( 14.0f, 20.0f )	 );
			} else {
				map->setGroundHeight( mapPosX + x, mapPosY + y, ground[x][y] );
			}
		}
	}
	
	// event handler for custom map processing
	int params[4];
	params[0] = regionX;
	params[1] = regionY;
	params[2] = mapPosX * OUTDOORS_STEP;
	params[3] = mapPosY * OUTDOORS_STEP;
	bool ret = shapePal->getSession()->getSquirrel()->callIntArgMethod( "generate_land", 4, params );
	
	if( !ret ) {
		// add trees
		for ( int x = 0; x < QUARTER_WIDTH_IN_NODES; x++ ) {
			for ( int y = 0; y < QUARTER_DEPTH_IN_NODES; y++ ) {
				if ( cellular->getNode( x, y )->island && !cellular->getNode( x, y )->water ) {
					GLShape *shape = shapePal->getRandomTreeShape( shapePal );
					int xx = ( mapPosX + x ) * OUTDOORS_STEP;
					int yy = ( mapPosY + y ) * OUTDOORS_STEP + shape->getHeight();
	
					// don't put them on roads and in houses
					if ( map->shapeFitsOutdoors( shape, xx, yy, 0 ) ) {
						map->setPosition( xx, yy, 0, shape );
					}
				}
			}
		}
		
		// create a set of rooms for outdoor items
		doorCount = 0;
		roomCount = 0;
		room[ roomCount ].x = mapPosX * OUTDOORS_STEP;
		room[ roomCount ].y = mapPosY * OUTDOORS_STEP;
		room[ roomCount ].w = QUARTER_WIDTH_IN_NODES * OUTDOORS_STEP;
		room[ roomCount ].h = QUARTER_DEPTH_IN_NODES * OUTDOORS_STEP;
		room[ roomCount ].valueBonus = 0;
		roomCount++;
		roomMaxWidth = 0;
		roomMaxHeight = 0;
		objectCount = 7 + ( level / 8 ) * 5;
		monsters = true;
	}

	return true;
}

MapRenderHelper* LandGenerator::getMapRenderHelper() {
	// we need fog
	return MapRenderHelper::helpers[ MapRenderHelper::OUTDOOR_HELPER ];
	//return MapRenderHelper::helpers[ MapRenderHelper::DEBUG_OUTDOOR_HELPER ];
}

// =====================================================
// =====================================================
// generation
//

// a 16x16 block of data (in four 8x8 sections, one per cellular section)
int data[ REGION_SIZE * REGION_SIZE ];

void printData() {
	cerr << "-----------------------------------" << endl;
	for( int y = 0; y < REGION_SIZE; y++ ) {
		for( int x = 0; x < REGION_SIZE; x++ ) {
			fprintf( stderr, " %2d", data[ y * REGION_SIZE + x ] );
		}
		fprintf( stderr, "\n" );
	}
}

void printRGB( vector<GLubyte> *image ) {
	for( int y = 0; y < REGION_SIZE; y++ ) {
		for( int x = 0; x < REGION_SIZE; x++ ) {
			int offs = y * REGION_SIZE * BYTES_PER_PIXEL + x * BYTES_PER_PIXEL;
			fprintf( stderr, " %02x%02x%02x", image->at( offs + 0 ), image->at( offs + 1 ), image->at( offs + 2 ) );
		}
		cerr << "\n";
	}
}	

void LandGenerator::loadMapGridBitmap() {
	// load the correct bitmap (if not already in memory)
	int bitmapX = regionX / REGIONS_PER_BITMAP;
	int bitmapY = regionY / REGIONS_PER_BITMAP;
	int bitmapIndex = bitmapY * BITMAPS_PER_ROW + bitmapX;
	cerr << "Needs bitmap index=" << bitmapIndex << 
		" region: " << regionX << "," << regionY << 
		" bitmap: " << bitmapX << "," << bitmapY << endl;
	
	if( this->bitmapIndex != bitmapIndex ) {
		char bitmapName[3000];
		sprintf( bitmapName, "/mapgrid/world/map_%02d.png", bitmapIndex );
		cerr << "Needs bitmap " << bitmapName << endl;
		
		if( bitmapSurface ) {
			SDL_FreeSurface( bitmapSurface );
		}

		string fn = rootDir + bitmapName;
		cerr << "Loading bitmap: " << fn << endl;
		if ( !( bitmapSurface = IMG_Load( fn.c_str() ) ) ) {
			cerr << "*** Error loading map chunk (" << fn << "): " << IMG_GetError() << endl;
		}
	}
	
	loadMapGridBitmapRegion();
}

void LandGenerator::loadMapGridBitmapRegion() {
	// select the correct section of the image
	// The raw data of the source image.
	unsigned char * data = ( unsigned char * ) ( bitmapSurface->pixels );
//		for( int i = 0; i < 128 * 128 * BYTES_PER_PIXEL; i+=BYTES_PER_PIXEL ) {
//			fprintf( stderr, "%02x%02x%02x%02x,", data[i], data[i+1], data[i+2], data[i+3]);
//			if( i > 0 && i % ( 128 * BYTES_PER_PIXEL ) == 0 ) cerr << endl;
//		}
//	cerr << "bytesPerPixel: " << bitmapSurface->format->BytesPerPixel <<
//		" bitsPerPixel: " << bitmapSurface->format->BitsPerPixel <<
//		" pitch=" << bitmapSurface->pitch << endl;

	// The destination image (a single tile)
	std::vector<GLubyte> image( REGION_SIZE * REGION_SIZE * BYTES_PER_PIXEL );

	int rx = regionX % REGIONS_PER_BITMAP;
	int ry = regionY % REGIONS_PER_BITMAP;
	int count = 0;
	// where the tile starts in a line
	int offs = rx * REGION_SIZE * BYTES_PER_PIXEL;
	// where the tile ends in a line
	int rest = ( rx + 1 ) * REGION_SIZE * BYTES_PER_PIXEL;
	// Current position in the source data
	int c = offs + ( ry * REGION_SIZE * bitmapSurface->pitch );
	// the following lines extract R,G and B values from any bitmap

//	cerr << " c:" << c << " ";
	for ( int i = 0; i < REGION_SIZE * REGION_SIZE; ++i ) {

		if ( i > 0 && i % REGION_SIZE == 0 ) {
			// skip the rest of the line
			c += ( bitmapSurface->pitch - rest );
			// skip the offset (go to where the tile starts)
			c += offs;
//			cerr << endl;
//			cerr << " c:" << c << " ";
		}

		for ( int p = 0; p < BYTES_PER_PIXEL; p++ ) {
//			fprintf( stderr, "%02x", data[c] );
			image[count++] = data[c++];
		}
//		cerr << ",";
	}
//	cerr << endl << "found " << image.size() << " pixels." << endl;
	//printRGB( &image );
	
	packMapData( image );
}

void LandGenerator::packMapData( std::vector<GLubyte> &image ) {
	for( int i = 0; i < (int)image.size(); i += BYTES_PER_PIXEL ) {
		data[ i / BYTES_PER_PIXEL ] = image[ i + 2 ] > 0 ? -1 : ( image[ i ] > 0 ? 1 : 0 );
	}
	//printData();
	cellular->initialize( REGION_SIZE, REGION_SIZE, data );
}

void LandGenerator::generate( Map *map, ShapePalette *shapePal ) {
	loadMapGridBitmap();
	
	cellular->generate( true, true, 4, true, false );
	cellular->makeMinSpace( 4 );
	cellular->print();
	
	createGround();
}

void LandGenerator::createGround() {
	// create the undulating ground
	float amp = 1.0f;
	float freq = 40.0f;
	for ( int x = 0; x < QUARTER_WIDTH_IN_NODES; x++ ) {
		for ( int y = 0; y < QUARTER_DEPTH_IN_NODES; y++ ) {
			// fixme: use a more sinoid function here
			// ground[x][y] = ( 1.0f * rand() / RAND_MAX );
			float a = Util::roll( 0.25f, amp );
			float f = freq / 2 + Util::roll( 0.25f, freq / 2 );
			ground[x][y] = a +
			               ( a *
			                 sin( PI / ( 180.0f / static_cast<float>( x * OUTDOORS_STEP * f ) ) ) *
			                 cos( PI / ( 180.0f / static_cast<float>( y * OUTDOORS_STEP  * f ) ) ) );
			if ( ground[x][y] < 0 ) ground[x][y] = 0;
		}
	}
}

void LandGenerator::printMaze() {
	cellular->print(); 
}

bool LandGenerator::addParty( Map *map, ShapePalette *shapePal, bool goingUp, bool goingDown ) {
	if( willAddParty ) {
		TerrainGenerator::addParty( map, shapePal, goingUp, goingDown );
	}
	return true;
}
