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
 * The outdoors is generated by connecting four cellular-automaton-created maps.
 * If we were to draw one big map, the recursions for finding free space take too
 * long.
 */
LandGenerator::LandGenerator( Scourge *scourge, int level, int depth, int maxDepth,
                              bool stairsDown, bool stairsUp,
                              Mission *mission ) :
		TerrainGenerator( scourge, level, depth, maxDepth, stairsDown, stairsUp, mission, 13 ) {
	// init the ground
	for ( int x = 0; x < MAP_STEP_WIDTH; x++ ) {
		for ( int y = 0; y < MAP_STEP_DEPTH; y++ ) {
			ground[x][y] = 0; //XXX: ?initializes ground[MAP_WIDTH][MAP_DEPTH] only partially
		}
	}
	this->cellular[0][0] = new CellularAutomaton( WIDTH_IN_NODES, DEPTH_IN_NODES );
	this->cellular[1][0] = new CellularAutomaton( WIDTH_IN_NODES, DEPTH_IN_NODES );
	this->cellular[0][1] = new CellularAutomaton( WIDTH_IN_NODES, DEPTH_IN_NODES );
	this->cellular[1][1] = new CellularAutomaton( WIDTH_IN_NODES, DEPTH_IN_NODES );
	
	this->regionX = this->regionY = 0;
	this->bitmapIndex = -1;
	this->bitmapSurface = NULL;
}

LandGenerator::~LandGenerator() {
	delete cellular[0][0];
	delete cellular[0][1];
	delete cellular[1][0];
	delete cellular[1][1];
}


bool LandGenerator::drawNodes( Map *map, ShapePalette *shapePal ) {
	updateStatus( _( "Loading theme" ) );
	if ( map->getPreferences()->isDebugTheme() ) shapePal->loadDebugTheme();
	else shapePal->loadRandomOutdoorTheme();

	map->setHeightMapEnabled( true );

	// add mountains
	//int offs = MAP_OFFSET / OUTDOORS_STEP;
	int offs = 0;
	for ( int x = 0; x < MAP_STEP_WIDTH; x++ ) {
		for ( int y = 0; y < MAP_STEP_DEPTH; y++ ) {
			map->setGroundHeight( x, y, ground[x][y] );
			if ( x >= offs && x < 2 * WIDTH_IN_NODES + offs &&
			        y >= offs && y < 2 * DEPTH_IN_NODES + offs ) {
				int cx = ( x - offs ) / WIDTH_IN_NODES;
				int cy = ( y - offs ) / DEPTH_IN_NODES;
				int mx = ( x - offs ) % WIDTH_IN_NODES;
				int my = ( y - offs ) % DEPTH_IN_NODES;
				if ( cellular[ cx ][ cy ]->getNode( mx, my )->wall ) {
					map->setGroundHeight( x, y, Util::roll( 14.0f, 20.0f ) );
				} else if ( cellular[ cx ][ cy ]->getNode( mx, my )->water ) {
					map->setGroundHeight( x, y, -Util::roll( 14.0f, 20.0f )	 );
				}
			//} else {
				//map->setGroundHeight( x, y, Util::roll( 14.0f, 20.0f ) );
			}
		}
	}

	// add trees
	for ( int x = 0; x < MAP_STEP_WIDTH; x++ ) {
		for ( int y = 0; y < MAP_STEP_DEPTH; y++ ) {
			if ( x >= offs && x < 2 * WIDTH_IN_NODES + offs &&
			        y >= offs && y < 2 * DEPTH_IN_NODES + offs ) {
				int cx = ( x - offs ) / WIDTH_IN_NODES;
				int cy = ( y - offs ) / DEPTH_IN_NODES;
				int mx = ( x - offs ) % WIDTH_IN_NODES;
				int my = ( y - offs ) % DEPTH_IN_NODES;
				if ( cellular[ cx ][ cy ]->getNode( mx, my )->island ) {
					GLShape *shape = shapePal->getRandomTreeShape( shapePal );
					int xx = x * OUTDOORS_STEP;
					int yy = y * OUTDOORS_STEP + shape->getHeight();

					// don't put them on roads and in houses
					if ( map->shapeFitsOutdoors( shape, xx, yy, 0 ) ) {
						map->setPosition( xx, yy, 0, shape );
					}
				}
			}
		}
	}

	map->getRender()->initOutdoorsGroundTexture();

	// create a set of rooms for outdoor items
	doorCount = 0;
	roomCount = 0;
	for ( int cx = 0; cx < 2; cx++ ) {
		for ( int cy = 0; cy < 2; cy++ ) {
			//CellularAutomaton *c = cellular[cx][cy];
			room[ roomCount ].x = offset + ( cx * WIDTH_IN_NODES * OUTDOORS_STEP );
			room[ roomCount ].y = offset + ( cy * DEPTH_IN_NODES * OUTDOORS_STEP );
			room[ roomCount ].w = WIDTH_IN_NODES * OUTDOORS_STEP;
			room[ roomCount ].h = DEPTH_IN_NODES * OUTDOORS_STEP;
			room[ roomCount ].valueBonus = 0;
			roomCount++;
		}
	}
	roomMaxWidth = 0;
	roomMaxHeight = 0;
	objectCount = 7 + ( level / 8 ) * 5;
	monsters = true;

	// set the floor, so random positioning works in terrain generator
	// a bit of a song-and-dance with keepFloor is so that random objects aren't placed in rooms
	for ( int x = MAP_OFFSET; x < MAP_WIDTH - MAP_OFFSET; x += MAP_UNIT ) {
		for ( int y = MAP_OFFSET; y < MAP_DEPTH - MAP_OFFSET - MAP_UNIT; y += MAP_UNIT ) {
			map->removeFloorPosition( ( Sint16 )x, ( Sint16 )y + MAP_UNIT );
		}
	}

	// event handler for custom map processing
	if ( !scourge->getSession()->getMap()->inMapEditor() ) {
		scourge->getSession()->getSquirrel()->callMapMethod( "outdoorMapCompleted", map->getName() );
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

#define REGION_SIZE 16
#define QUARTER_SIZE ( REGION_SIZE / 2 )
#define BITMAP_SIZE 128
#define REGION_PER_BITMAP ( BITMAP_SIZE / REGION_SIZE )
#define BITMAPS_PER_ROW 11
#define BITMAPS_PER_COL 10
#define BYTES_PER_PIXEL 3

// a 16x16 block of data (in four 8x8 sections, one per cellular section)
int data_0_0[ QUARTER_SIZE * QUARTER_SIZE ];
int data_1_0[ QUARTER_SIZE * QUARTER_SIZE ];
int data_0_1[ QUARTER_SIZE * QUARTER_SIZE ];
int data_1_1[ QUARTER_SIZE * QUARTER_SIZE ];

void printData() {
	cerr << "-----------------------------------" << endl;
	int p;
	for( int y = 0; y < REGION_SIZE; y++ ) {
		for( int x = 0; x < REGION_SIZE; x++ ) {
			if( x < 8 && y < 8 ) {
				p = data_0_0[ y * QUARTER_SIZE + x ];
			} else if ( x >= 8 && y < 8 ) {
				p = data_1_0[ y * QUARTER_SIZE + x - 8 ];
			} else if( x < 8 && y >= 8 ) {
				p = data_0_1[ ( y - 8 ) * QUARTER_SIZE + x ];
			} else if ( x >= 8 && y >= 8 ) {
				p = data_1_1[ ( y - 8 ) * QUARTER_SIZE + x - 8 ];				
			}
			if( x == 8 ) cerr << " | ";
			fprintf( stderr, " %2d", p );
		}
		if( y == 7 ) cerr << "\n-----------------------------------------";
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

void LandGenerator::generate( Map *map, ShapePalette *shapePal ) {
	// load the correct bitmap (if not already in memory)
	int bitmapX = regionX / REGION_PER_BITMAP;
	int bitmapY = regionY / REGION_PER_BITMAP;
	int bitmapIndex = bitmapY * BITMAPS_PER_ROW + bitmapX;
	cerr << "Needs bitmap index=" << bitmapIndex << " region: " << regionX << "," << regionY << " bitmap: " << bitmapX << "," << bitmapY << endl;
	
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
		
	// select the correct section of the image
//			_filename = "a tile";
//			_width = tileWidth;
//			_height = tileHeight;
//			_hasAlpha = true;
		// The raw data of the source image.
		unsigned char * data = ( unsigned char * ) ( bitmapSurface->pixels );
//		for( int i = 0; i < 128 * 128 * BYTES_PER_PIXEL; i+=BYTES_PER_PIXEL ) {
//			fprintf( stderr, "%02x%02x%02x%02x,", data[i], data[i+1], data[i+2], data[i+3]);
//			if( i > 0 && i % ( 128 * BYTES_PER_PIXEL ) == 0 ) cerr << endl;
//		}
		cerr << "bytesPerPixel: " << bitmapSurface->format->BytesPerPixel <<
			" bitsPerPixel: " << bitmapSurface->format->BitsPerPixel <<
			" pitch=" << bitmapSurface->pitch << endl;

		// The destination image (a single tile)
		std::vector<GLubyte> image( REGION_SIZE * REGION_SIZE * BYTES_PER_PIXEL );

		int rx = regionX % REGION_PER_BITMAP;
		int ry = regionY % REGION_PER_BITMAP;
		int count = 0;
		// where the tile starts in a line
		int offs = rx * REGION_SIZE * BYTES_PER_PIXEL;
		// where the tile ends in a line
		int rest = ( rx + 1 ) * REGION_SIZE * BYTES_PER_PIXEL;
		// Current position in the source data
		int c = offs + ( ry * REGION_SIZE * bitmapSurface->pitch );
		// the following lines extract R,G and B values from any bitmap

		cerr << " c:" << c << " ";
		for ( int i = 0; i < REGION_SIZE * REGION_SIZE; ++i ) {

			if ( i > 0 && i % REGION_SIZE == 0 ) {
				// skip the rest of the line
				c += ( bitmapSurface->pitch - rest );
				// skip the offset (go to where the tile starts)
				c += offs;
				cerr << endl;
				cerr << " c:" << c << " ";
			}

			for ( int p = 0; p < BYTES_PER_PIXEL; p++ ) {
				fprintf( stderr, "%02x", data[c] );
				image[count++] = data[c++];
			}
			cerr << ",";
		}
		cerr << endl << "found " << image.size() << " pixels." << endl;
		//printRGB( &image );
		
	
	// pack data
	for( int i = 0; i < ( REGION_SIZE / 2 ) * ( REGION_SIZE / 2 ); i++ ) {
		offs = ( i / ( REGION_SIZE / 2 ) ) * REGION_SIZE * BYTES_PER_PIXEL + ( i % ( REGION_SIZE / 2 ) ) * BYTES_PER_PIXEL;
		data_0_0[ i ] = image[ offs + 2 ] > 0 ? -1 : ( image[ offs ] > 0 ? 1 : 0 );
		
		offs = ( i / ( REGION_SIZE / 2 ) ) * REGION_SIZE * BYTES_PER_PIXEL + ( ( REGION_SIZE / 2 ) + ( i % ( REGION_SIZE / 2 ) ) ) * BYTES_PER_PIXEL;
		data_1_0[ i ] = image[ offs + 2 ] > 0 ? -1 : ( image[ offs ] > 0 ? 1 : 0 );
		
		offs = ( ( REGION_SIZE / 2 ) + ( i / ( REGION_SIZE / 2 ) ) ) * REGION_SIZE * BYTES_PER_PIXEL + ( i % ( REGION_SIZE / 2 ) ) * BYTES_PER_PIXEL;
		data_0_1[ i ] = image[ offs + 2 ] > 0 ? -1 : ( image[ offs ] > 0 ? 1 : 0 );
		
		offs = ( ( REGION_SIZE / 2 ) + ( i / ( REGION_SIZE / 2 ) ) ) * REGION_SIZE * BYTES_PER_PIXEL + ( ( REGION_SIZE / 2 ) + ( i % ( REGION_SIZE / 2 ) ) ) * BYTES_PER_PIXEL;
		data_1_1[ i ] = image[ offs + 2 ] > 0 ? -1 : ( image[ offs ] > 0 ? 1 : 0 );
	}
	printData();
	
	
	// initialize cellular[][]
	
	cellular[0][0]->initialize( 8, 8, data_0_0 );
	cellular[0][0]->generate( true, true, 4, true );
	cellular[0][0]->makeAccessible( WIDTH_IN_NODES - 1, DEPTH_IN_NODES / 2 );
	cellular[0][0]->makeAccessible( WIDTH_IN_NODES / 2, DEPTH_IN_NODES - 1 );
	cellular[0][0]->makeMinSpace( 4 );
	//cellular[0][0]->print();

	cellular[1][0]->initialize( 8, 8, data_1_0 );
	cellular[1][0]->generate( true, true, 4, true );
	cellular[1][0]->makeAccessible( 0, DEPTH_IN_NODES / 2 );
	cellular[1][0]->makeAccessible( WIDTH_IN_NODES / 2, DEPTH_IN_NODES - 1 );
	cellular[1][0]->makeMinSpace( 4 );
	//cellular[1][0]->print();

	cellular[0][1]->initialize( 8, 8, data_0_1 );
	cellular[0][1]->generate( true, true, 4, true );
	cellular[0][1]->makeAccessible( WIDTH_IN_NODES - 1, DEPTH_IN_NODES / 2 );
	cellular[0][1]->makeAccessible( WIDTH_IN_NODES / 2, 0 );
	cellular[0][1]->makeMinSpace( 4 );
	//cellular[0][1]->print();

	cellular[1][1]->initialize( 8, 8, data_1_1 );
	cellular[1][1]->generate( true, true, 4, true );
	cellular[1][1]->makeAccessible( 0, DEPTH_IN_NODES / 2 );
	cellular[1][1]->makeAccessible( WIDTH_IN_NODES / 2, 0 );
	cellular[1][1]->makeMinSpace( 4 );
	//cellular[1][1]->print();
	
	//printMaze();
	
	createGround();
}

void LandGenerator::createGround() {
	// create the undulating ground
	float amp = 1.0f;
	float freq = 40.0f;
	for ( int x = 0; x < MAP_STEP_WIDTH; x++ ) {
		for ( int y = 0; y < MAP_STEP_DEPTH; y++ ) {
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
	for ( int x = 0; x < 2; x++ ) {
		for ( int y = 0; y < 2; y++ ) {
			cerr << "x=" << x << " y=" << y << endl;
			cellular[x][y]->print(); 
		}
	}
}
