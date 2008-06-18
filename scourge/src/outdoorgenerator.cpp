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
#include "sqbinding/sqbinding.h"
#include "rpg/rpglib.h"
#include "creature.h"
#include <vector>

using namespace std;


//good little mountain for turning into lake 
#define SMALL_MOUNTAIN 120 

/**
 * The outdoors is generated by connecting four cellular-automaton-created maps.
 * If we were to draw one big map, the recursions for finding free space take too
 * long.
 */
OutdoorGenerator::OutdoorGenerator( Scourge *scourge, int level, int depth, int maxDepth,
																		bool stairsDown, bool stairsUp, 
																		Mission *mission) : 
TerrainGenerator( scourge, level, depth, maxDepth, stairsDown, stairsUp, mission, 13 ) {
  // init the ground
	for( int x = 0; x < MAP_STEP_WIDTH; x++ ) {
		for( int y = 0; y < MAP_STEP_DEPTH; y++ ) {
			ground[x][y] = 0; //XXX: ?initializes ground[MAP_WIDTH][MAP_DEPTH] only partially 
		}
	}
  this->cellular[0][0] = new CellularAutomaton( WIDTH_IN_NODES, DEPTH_IN_NODES );
	this->cellular[1][0] = new CellularAutomaton( WIDTH_IN_NODES, DEPTH_IN_NODES );
	this->cellular[0][1] = new CellularAutomaton( WIDTH_IN_NODES, DEPTH_IN_NODES );
	this->cellular[1][1] = new CellularAutomaton( WIDTH_IN_NODES, DEPTH_IN_NODES );
	roadX = roadY = MAP_OFFSET + MAP_UNIT;
}

OutdoorGenerator::~OutdoorGenerator() {
  delete cellular[0][0];
	delete cellular[0][1];
	delete cellular[1][0];
	delete cellular[1][1];
}


OutdoorGenerator::AroundMapLooker seen; //we may make it static member?

// refactored to burn stack lot less 
int OutdoorGenerator::getMountainSize( int x, int y, Map *map, AroundMapLooker& lake ) {
	int ret( 0 );
	// the cases we look at something not worth looking at
	if ( x < 0 
			|| x >= MAP_STEP_WIDTH 
			|| y < 0 
			|| y >= MAP_STEP_DEPTH 
			|| map->getGroundHeight( x, y ) < 10 
			|| seen.at( x, y ) ) return ret;
	// look up everything west
	bool mountainAhead;
	int lookWest( x );
	do {
		++ret;
		seen.at( lookWest, y ) = true;
		lake.at( lookWest, y ) = true;
		--lookWest;
		mountainAhead = ( lookWest >= 0 ) 
				&& ( map->getGroundHeight( lookWest, y ) >= 10 );
	} while ( mountainAhead );
	// look up everything east
	int lookEast( x );
	do {
		// "if" for avoiding taking account spot at x,y twice
		if (!seen.at( lookEast, y )) ++ret; 
		seen.at( lookEast, y ) = true;
		lake.at( lookEast, y ) = true;
		++lookEast;
		mountainAhead = ( lookEast < MAP_STEP_WIDTH ) 
				&& ( map->getGroundHeight( lookEast, y ) >= 10 );
	} while ( mountainAhead );
	// look up north and south too  
	for (int lookX = lookWest; lookX <= lookEast; lookX++ ) {
		ret += getMountainSize( lookX, y - 1, map, lake ); 
		ret += getMountainSize( lookX, y + 1, map, lake ); 
	}
	return ret;
}

bool OutdoorGenerator::drawNodes( Map *map, ShapePalette *shapePal ) {

  updateStatus( _( "Loading theme" ) );
  if( map->getPreferences()->isDebugTheme() ) shapePal->loadDebugTheme();
	else shapePal->loadTheme("outdoor");

	map->setHeightMapEnabled( true );

	// add mountains
	int offs = MAP_OFFSET / OUTDOORS_STEP;
	for( int x = 0; x < MAP_STEP_WIDTH; x++ ) {
		for( int y = 0; y < MAP_STEP_DEPTH; y++ ) {
			map->setGroundHeight( x, y, ground[x][y] );
			if( x >= offs && x < 2 * WIDTH_IN_NODES + offs &&
					y >= offs && y < 2 * DEPTH_IN_NODES + offs ) {
				int cx = ( x - offs ) / WIDTH_IN_NODES;
				int cy = ( y - offs ) / DEPTH_IN_NODES;
				int mx = ( x - offs ) % WIDTH_IN_NODES;
				int my = ( y - offs ) % DEPTH_IN_NODES;
				if( cellular[ cx ][ cy ]->getNode( mx, my )->wall ) {
					map->setGroundHeight( x, y, Util::roll( 14.0f, 20.0f ) );
				}
			} else {
				map->setGroundHeight( x, y, Util::roll( 14.0f, 20.0f ) );
			}
		}
	}

	// turn some mountains into lakes
	seen.clear();
	for( int x = 0; x < MAP_STEP_WIDTH; x++ ) {
		for( int y = 0; y < MAP_STEP_DEPTH; y++ ) {
			AroundMapLooker lake;
			int size = getMountainSize( x, y, map, lake );
			if( size > 0 && size < SMALL_MOUNTAIN ) {
				if( 0 == Util::dice( 2 ) ) {
					for ( int posX = 0; posX < MAP_STEP_WIDTH; ++posX ) { 
						for ( int posY = 0; posY < MAP_STEP_DEPTH; ++posY ) {
							if ( lake.at( posX, posY ) )
								map->setGroundHeight( posX, posY, -( map->getGroundHeight( posX, posY ) ) );
						}
					}
				}
			}			
		}
	}
	
	// add a village
	addVillage( map, shapePal );

	// add trees
	for( int x = 0; x < MAP_STEP_WIDTH; x++ ) {
		for( int y = 0; y < MAP_STEP_DEPTH; y++ ) {
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

					// don't put them on roads and in houses
					if( !isShapeOnFloor( shape, x, y, map ) ) {										
						if( !map->isBlocked( xx, yy, 0, 0, 0, 0, shape ) ) {
							map->setPosition( xx, yy, 0, shape );
						}
					}
				}
			}
		}
	}
	
	map->initOutdoorsGroundTexture();

	// create a set of rooms for outdoor items
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

	// set the floor, so random positioning works in terrain generator
	// a bit of a song-and-dance with keepFloor is so that random objects aren't placed in rooms
	for( int x = MAP_OFFSET; x < MAP_WIDTH - MAP_OFFSET; x += MAP_UNIT ) {
		for( int y = MAP_OFFSET; y < MAP_DEPTH - MAP_OFFSET; y += MAP_UNIT ) {
			if( keepFloor.find( x + MAP_WIDTH * y ) != keepFloor.end() ) {
				map->removeFloorPosition( (Sint16)x, (Sint16)y + MAP_UNIT );
			} else {
				map->setFloorPosition( (Sint16)x, (Sint16)y + MAP_UNIT, (Shape*)shapePal->findShapeByName( "FLOOR_TILE" ) );
			}
		}
	}

	// event handler for custom map processing
	if( !scourge->getSession()->getMap()->inMapEditor() ) {
		scourge->getSession()->getSquirrel()->callMapMethod( "outdoorMapCompleted", map->getName() );
	}

	return true;
}

bool OutdoorGenerator::isShapeOnFloor( Shape *shape, int x, int y, Map *map ) {
	int fx = ( ( x - MAP_OFFSET ) / MAP_UNIT ) * MAP_UNIT + MAP_OFFSET;
	int fy = ( ( y - shape->getHeight() - MAP_OFFSET ) / MAP_UNIT ) * MAP_UNIT + MAP_OFFSET + MAP_UNIT;
	if( map->getFloorPosition( fx, fy ) ) return true;
	fx = ( ( x - MAP_OFFSET ) / MAP_UNIT ) * MAP_UNIT + MAP_OFFSET;
	fy = ( ( y - MAP_OFFSET ) / MAP_UNIT ) * MAP_UNIT + MAP_OFFSET + MAP_UNIT;
	if( map->getFloorPosition( fx, fy ) ) return true;
	fx = ( ( x + shape->getWidth() - MAP_OFFSET ) / MAP_UNIT ) * MAP_UNIT + MAP_OFFSET;
	fy = ( ( y - MAP_OFFSET ) / MAP_UNIT ) * MAP_UNIT + MAP_OFFSET + MAP_UNIT;
	if( map->getFloorPosition( fx, fy ) ) return true;
	fx = ( ( x + shape->getWidth() - MAP_OFFSET ) / MAP_UNIT ) * MAP_UNIT + MAP_OFFSET;
	fy = ( ( y - shape->getHeight() - MAP_OFFSET ) / MAP_UNIT ) * MAP_UNIT + MAP_OFFSET + MAP_UNIT;
	if( map->getFloorPosition( fx, fy ) ) return true;
	return false;
}

#define VILLAGE_WIDTH 7
#define VILLAGE_HEIGHT 7

// start party in the middle of the cross-roads
void OutdoorGenerator::getPartyStartingLocation( int *xx, int *yy ) {
	*xx = roadX;
	*yy = roadY;
}

void OutdoorGenerator::addVillage( Map *map, ShapePalette *shapePal ) {
	int x = MAP_OFFSET + ( ( Util::dice( MAP_WIDTH - ( MAP_OFFSET * 4 ) - VILLAGE_WIDTH ) / MAP_UNIT ) * MAP_UNIT );
	int y = MAP_OFFSET + ( ( Util::dice( MAP_DEPTH - ( MAP_OFFSET * 4 ) - VILLAGE_HEIGHT ) / MAP_UNIT ) * MAP_UNIT );
	
	removeLakes( map, x, y );
	
	createRoads( map, shapePal, x, y );
	
	createHouses( map, shapePal, x, y, roadX, roadY - MAP_UNIT );
	
	// the rest of the village is in squirrel
	shapePal->getSession()->getSquirrel()->setGlobalVariable( "villageX", x );
	shapePal->getSession()->getSquirrel()->setGlobalVariable( "villageY", y );
	shapePal->getSession()->getSquirrel()->setGlobalVariable( "villageWidth", VILLAGE_WIDTH * MAP_UNIT );
	shapePal->getSession()->getSquirrel()->setGlobalVariable( "villageHeight", VILLAGE_HEIGHT * MAP_UNIT );
	shapePal->getSession()->getSquirrel()->setGlobalVariable( "villageRoadX", roadX );
	shapePal->getSession()->getSquirrel()->setGlobalVariable( "villageRoadY", roadY - MAP_UNIT );
	
	// fix up the roads
	shapePal->getSession()->getSquirrel()->callNoArgMethod( "villageRoads" );
	
	// add some random items thru town
	shapePal->getSession()->getSquirrel()->callNoArgMethod( "villageShapes" );
	
	// add npc-s
	addNpcs( map, shapePal, x, y, VILLAGE_WIDTH * MAP_UNIT, VILLAGE_HEIGHT * MAP_UNIT );
	
	// add some decor
	addRugs( map, shapePal );
	
	// Hack: limit free space to just houses for now.
	createFreeSpaceMap( map, shapePal );
	
	// add some containers
	addContainersInRooms( map, shapePal );
	
	// add tables, chairs, etc.
	addItemsInEveryRoom( RpgItem::getItemByName("Table"), 1 );
	addItemsInEveryRoom( RpgItem::getItemByName("Chair"), 2 );
	addItemsInEveryRoom( RpgItem::getItemByName("Bed"), 1 );
	addItemsInEveryRoom( RpgItem::getItemByName("Stove"), 1 );
	
	// clean up free space
	deleteFreeSpaceMap( map, shapePal );
}

void OutdoorGenerator::addNpcs( Map *map, ShapePalette *shapePal, int villageX, int villageY, int villageWidth, int villageHeight ) {
	for( int i = 0; i < 10; i++ ) {
		createNpc( map, shapePal, 
		           Util::pickOne( villageX, villageX + villageWidth ), 
		           Util::pickOne( villageY, villageY + villageHeight ) );
	}	
	for( int i = 0; i < roomCount; i++ ) {
		createNpc( map, shapePal, 
		           ( room[ i ].x + room[ i ].w / 2 ) * MAP_UNIT + MAP_OFFSET, 
		           ( room[ i ].y + room[ i ].h / 2 ) * MAP_UNIT + MAP_OFFSET );
	}
}

void OutdoorGenerator::createNpc( Map *map, ShapePalette *shapePal, int x, int y ) {
	Monster *npc;
	char npcName[255];
	shapePal->getSession()->getSquirrel()->callNoArgStringReturnMethod( "getVillageNpcType", npcName );
	if( !strlen( npcName ) ) {
		cerr << "*** getVillageNpcType from squirrel returned no npc name." << endl;
		return;
	} else {
		npc = Monster::getMonsterByName( npcName );
		if( !npc ) {
			cerr << "*** Can't find npc: " << npcName << endl;
			return;
		} else if( !npc->isNpc() ) {
			cerr << "*** Can't find npc: " << npcName << endl;
			return;			
		}
	}
	
	GLShape *shape = scourge->getShapePalette()->getCreatureShape( npc->getModelName(), npc->getSkinName(), npc->getScale(), npc);
  Creature *creature = scourge->getSession()->newCreature( npc, shape );
  int fx, fy;
  creature->findPlace( x, y, &fx, &fy );
  
  if( 0 == Util::dice( 3 ) ) {
  	Mission::createTypedNpc( creature, level, fx, fy );
  }
}

// roads and lakes don't mix well
void OutdoorGenerator::removeLakes( Map *map, int x, int y ) {
	for( int vx = x; vx < x + VILLAGE_WIDTH * MAP_UNIT; vx++ ) {
		for( int vy = y; vy < y + VILLAGE_HEIGHT * MAP_UNIT; vy++ ) {
			flattenChunk( map, vx, vy, Util::roll( 0.3f, 3 ) );
		}
	}	
}

#define HOUSE_SHAPES_SIZE 3
int HOUSE_SHAPES[][2] = { { 2, 2 }, { 2, 3 }, { 3, 2 } };
void OutdoorGenerator::createHouses( Map *map, ShapePalette *shapePal, int x, int y, int roadX, int roadY ) {
	
  // set rooms
  doorCount = 0;
	roomCount = 0;
  roomMaxWidth = 3;
  roomMaxHeight = 3;
  objectCount = 20;
  monsters = true;
  
  int rx = ( roadX - x ) / MAP_UNIT;
  int ry = ( roadY - y ) / MAP_UNIT;
  
  for( int iy = -2; iy < VILLAGE_HEIGHT + 2; iy++ ) {
  	int n = Util::dice( HOUSE_SHAPES_SIZE );
  	int *i = HOUSE_SHAPES[ n ];
  	buildHouse( map, shapePal, x, y, rx - i[0], iy, i[0], i[1] );
  	
  	n = Util::dice( HOUSE_SHAPES_SIZE );
  	i = HOUSE_SHAPES[ n ];
  	buildHouse( map, shapePal, x, y, rx + 1, iy, i[0], i[1] );
  }
  for( int ix = -2; ix < VILLAGE_HEIGHT + 2; ix++ ) {
  	int n = Util::dice( HOUSE_SHAPES_SIZE );
  	int *i = HOUSE_SHAPES[ n ];
  	buildHouse( map, shapePal, x, y, ix, ry - i[1] - 1, i[0], i[1] );
  	
  	n = Util::dice( HOUSE_SHAPES_SIZE );
  	i = HOUSE_SHAPES[ n ];
  	buildHouse( map, shapePal, x, y, ix, ry + 1, i[0], i[1]  );
  }	
}

bool OutdoorGenerator::buildHouse( Map *map, ShapePalette *shapePal, int x, int y, int ix, int iy, int w, int h ) {
	int vx = x + ix * MAP_UNIT;
	int vy = y + iy * MAP_UNIT;
	if( createHouse( map, shapePal, vx, vy, w, h ) ) {
		room[ roomCount ].x = ( vx - MAP_OFFSET ) / MAP_UNIT;
		room[ roomCount ].y = ( vy - MAP_OFFSET ) / MAP_UNIT;
		room[ roomCount ].w = w;
		room[ roomCount ].h = h;
		room[ roomCount ].valueBonus = 0;
		roomCount++;
		return true;
	}
	return false;
}

bool OutdoorGenerator::createHouse( Map *map, ShapePalette *shapePal, int x, int y, int w, int h ) {
	if( !( x >= MAP_OFFSET + MAP_UNIT && 
			y >= MAP_OFFSET + MAP_UNIT && 
			x + w * MAP_UNIT < MAP_WIDTH - MAP_OFFSET -  MAP_UNIT && 
			y + h * MAP_UNIT < MAP_DEPTH - MAP_OFFSET - MAP_UNIT ) ) {
		return false;
	}
	//cerr << "house at: " << x << "," << y << " dim=" << w << "," << h << endl;
	// not on the road
	for( int vx = 0; vx < w; vx++ ) {
		for( int vy = 0; vy < h; vy++ ) {
			int hx = x + vx * MAP_UNIT;
			int hy = y + vy * MAP_UNIT + MAP_UNIT;
			if( hx == roadX && hy >= y && hy < y + VILLAGE_HEIGHT ) {
				return false;
			} else if( hy == roadY && hx >= x && hx < x + VILLAGE_WIDTH ) {
				return false;
			}
			/*
			if( map->getFloorPosition( x + vx * MAP_UNIT, y + vy * MAP_UNIT + MAP_UNIT ) ) {
				//cerr << "\tabandon: on road." << endl;
				return false;
			}
			*/
		}
	}
	// not too close to another house
	for( int vx = -1; vx < w + 1; vx++ ) {
		for( int vy = -1; vy < h + 1; vy++ ) {
			Shape *shape = map->getFloorPosition( x + vx * MAP_UNIT, y + vy * MAP_UNIT + MAP_UNIT ); 
			if( shape == shapePal->findShapeByName( "ROOM_FLOOR_TILE" ) ) {
				//cerr << "\tabandon: too close to another." << endl;
				return false;
			}
		}
	}		
	int door = Util::dice( 4 );
	for( int vx = 0; vx < w; vx++ ) {
		for( int vy = 0; vy < h; vy++ ) {
			int xp = x + vx * MAP_UNIT;
			int yp = y + vy * MAP_UNIT;
			GLShape *shape = shapePal->findShapeByName( "ROOM_FLOOR_TILE" );
			addFloor( map, shapePal, xp, yp, true, shape );
			keepFloor[ xp + MAP_WIDTH * yp ] = shape;
			
			// draw the walls
			if( vx == 0 ) {
				if( vy == 0 ) {
					map->setPosition( xp, yp + 2, 0, shapePal->findShapeByName( "CORNER" ) );
					if( door == 0 || door == 3 ) {
						addEWDoor( map, shapePal, xp, yp );
					} else {					
						map->setPosition( xp, yp + MAP_UNIT, 0, shapePal->findShapeByName( "EW_WALL_EXTRA" ) );
					}
					if( door == 1 || door == 2 ) {
						addNSDoor( map, shapePal, xp + 2, yp + 2 );
					} else {
						map->setPosition( xp + 2, yp + 2, 0, shapePal->findShapeByName( "NS_WALL_EXTRA" ) );
					}
					map->setPosition( xp - 2, yp + MAP_UNIT, MAP_WALL_HEIGHT, shapePal->findShapeByName( "ROOF_NW" ) );
				} else if( vy == h - 1 ) {
					map->setPosition( xp, yp + MAP_UNIT, 0, shapePal->findShapeByName( "CORNER" ) );
					map->setPosition( xp, yp + MAP_UNIT - 2, 0, shapePal->findShapeByName( "EW_WALL_EXTRA" ) );
					if( door == 3 || door == 0 ) {
						addNSDoor( map, shapePal, xp + 2, yp + MAP_UNIT );
					} else {					
						map->setPosition( xp + 2, yp + MAP_UNIT, 0, shapePal->findShapeByName( "NS_WALL_EXTRA" ) );
					}
					map->setPosition( xp - 2, yp + MAP_UNIT + 2, MAP_WALL_HEIGHT, shapePal->findShapeByName( "ROOF_SW" ) );
				} else {
					map->setPosition( xp, yp + MAP_UNIT, 0, shapePal->findShapeByName( "EW_WALL_TWO_EXTRAS" ) );
					map->setPosition( xp - 2, yp + MAP_UNIT, MAP_WALL_HEIGHT, shapePal->findShapeByName( "ROOF_W" ) );
				}
			} else if( vx == w - 1 ) {
				if( vy == 0 ) {
					map->setPosition( xp + MAP_UNIT - 2, yp + 2, 0, shapePal->findShapeByName( "CORNER" ) );
					if( door == 2 || door == 1 ) {
						addEWDoor( map, shapePal, xp + MAP_UNIT - 2, yp );
					} else {					
						map->setPosition( xp + MAP_UNIT - 2, yp + MAP_UNIT, 0, shapePal->findShapeByName( "EW_WALL_EXTRA" ) );
					}
					map->setPosition( xp, yp + 2, 0, shapePal->findShapeByName( "NS_WALL_EXTRA" ) );
					map->setPosition( xp, yp + MAP_UNIT, MAP_WALL_HEIGHT, shapePal->findShapeByName( "ROOF_NE" ) );
				} else if( vy == h - 1 ) {
					map->setPosition( xp + MAP_UNIT - 2, yp + MAP_UNIT, 0, shapePal->findShapeByName( "CORNER" ) );
					map->setPosition( xp + MAP_UNIT - 2, yp + MAP_UNIT - 2, 0, shapePal->findShapeByName( "EW_WALL_EXTRA" ) );
					map->setPosition( xp, yp + MAP_UNIT, 0, shapePal->findShapeByName( "NS_WALL_EXTRA" ) );
					map->setPosition( xp, yp + MAP_UNIT + 2, MAP_WALL_HEIGHT, shapePal->findShapeByName( "ROOF_SE" ) );
				} else {
					map->setPosition( xp + MAP_UNIT - 2, yp + MAP_UNIT, 0, shapePal->findShapeByName( "EW_WALL_TWO_EXTRAS" ) );
					map->setPosition( xp, yp + MAP_UNIT, MAP_WALL_HEIGHT, shapePal->findShapeByName( "ROOF_E" ) );
				}
			} else if( vx > 0 && vx < w - 1 ) {
				if( vy == 0 ) {
					map->setPosition( xp, yp + 2, 0, shapePal->findShapeByName( "NS_WALL_TWO_EXTRAS" ) );
					map->setPosition( xp, yp + MAP_UNIT, MAP_WALL_HEIGHT, shapePal->findShapeByName( "ROOF_N" ) );
				} else if( vy == h - 1 ) {
					map->setPosition( xp, yp + MAP_UNIT, 0, shapePal->findShapeByName( "NS_WALL_TWO_EXTRAS" ) );
					map->setPosition( xp, yp + MAP_UNIT + 2, MAP_WALL_HEIGHT, shapePal->findShapeByName( "ROOF_S" ) );
				}
			}
		}
	}
	return true;
}

void OutdoorGenerator::addEWDoor( Map *map, ShapePalette *shapePal, int x, int y ) {
	map->setPosition( x, y + MAP_UNIT, 0, shapePal->findShapeByName( "CORNER" ) );
	map->setPosition( x, y + MAP_UNIT - 2, 0, shapePal->findShapeByName( "DOOR_SIDE" ) );
	map->setPosition( x, y + MAP_UNIT - 4, 0, shapePal->findShapeByName( "DOOR_SIDE" ) );
	map->setPosition( x, y + MAP_UNIT - 6, 0, shapePal->findShapeByName( "EW_DOOR" ) );
	map->setPosition( x, y + MAP_UNIT - 12, 0, shapePal->findShapeByName( "DOOR_SIDE" ) );
	map->setPosition( x, y + MAP_UNIT - 2, MAP_WALL_HEIGHT - 2, shapePal->findShapeByName( "EW_DOOR_TOP" ) );	
}

void OutdoorGenerator::addNSDoor( Map *map, ShapePalette *shapePal, int x, int y ) {
	map->setPosition( x, y, 0, shapePal->findShapeByName( "DOOR_SIDE" ) );
	map->setPosition( x + 2, y, 0, shapePal->findShapeByName( "DOOR_SIDE" ) );
	map->setPosition( x + 4, y, 0, shapePal->findShapeByName( "NS_DOOR" ) );
	map->setPosition( x + 10, y, 0, shapePal->findShapeByName( "DOOR_SIDE" ) );
	map->setPosition( x, y, MAP_WALL_HEIGHT - 2, shapePal->findShapeByName( "NS_DOOR_TOP" ) );
	map->setPosition( x + 12, y, 0, shapePal->findShapeByName( "CORNER" ) );
}

void OutdoorGenerator::createRoads( Map *map, ShapePalette *shapePal, int x, int y ) {
	roadX = x + ( 1 + Util::dice( VILLAGE_WIDTH - 1 ) ) * MAP_UNIT;
	for( int i = 0; i < VILLAGE_HEIGHT; i++ ) {
		int vy = y + ( i * MAP_UNIT );
		if( i == 0 ) {
			addOutdoorTexture( map, shapePal, roadX, vy, WallTheme::OUTDOOR_THEME_REF_STREET_END, 270 );
		} else if( i == VILLAGE_WIDTH - 1 ) {
			addOutdoorTexture( map, shapePal, roadX, vy, WallTheme::OUTDOOR_THEME_REF_STREET_END, 90 );
		} else {
			addOutdoorTexture( map, shapePal, roadX, vy, WallTheme::OUTDOOR_THEME_REF_STREET, 90.0f );
		}
	}

	roadY = y + ( 1 + Util::dice( VILLAGE_HEIGHT - 1 ) ) * MAP_UNIT;
	for( int i = 0; i < VILLAGE_WIDTH; i++ ) {
		int vx = x + ( i * MAP_UNIT );
		if( vx == roadX ) {
			addOutdoorTexture( map, shapePal, vx, roadY, WallTheme::OUTDOOR_THEME_REF_STREET_CROSS );
		} else if( i == 0 ) {
			addOutdoorTexture( map, shapePal, vx, roadY, WallTheme::OUTDOOR_THEME_REF_STREET_END );
		} else if( i == VILLAGE_WIDTH - 1 ) {
			addOutdoorTexture( map, shapePal, vx, roadY, WallTheme::OUTDOOR_THEME_REF_STREET_END, 180 );
		} else {
			addOutdoorTexture( map, shapePal, vx, roadY, WallTheme::OUTDOOR_THEME_REF_STREET );
		}
	}
}

void OutdoorGenerator::addOutdoorTexture( Map *map, ShapePalette *shapePal, Sint16 mapx, Sint16 mapy, int ref, float angle, bool horiz, bool vert ) {
	int faceCount = shapePal->getCurrentTheme()->getOutdoorFaceCount( ref );
	if( faceCount == 0 ) {
		cerr << "Error: no textures for outdoor theme!" << endl;
		return;
	}
	int w = shapePal->getCurrentTheme()->getOutdoorTextureWidth( ref );
	int h = shapePal->getCurrentTheme()->getOutdoorTextureHeight( ref );

	map->setOutdoorTexture( mapx, mapy + 1, 0, 0, ref, angle, horiz, vert, ROAD_LAYER );
	flattenChunkWithLimits( map, mapx, mapy, w, h, 0, 1 );
}

void OutdoorGenerator::flattenChunkWithLimits( Map *map, Sint16 mapX, Sint16 mapY, Sint16 mapEndX, Sint16 mapEndY, float minLimit, float maxLimit ) {
	for( int x = mapX; x < mapEndX; x++ ) {
		for( int y = mapY; y < mapEndY; y++ ) {
			int xx = x / OUTDOORS_STEP;
			int yy = y / OUTDOORS_STEP;
			if( map->getGroundHeight( xx, yy ) < minLimit ) {
				map->setGroundHeight( xx, yy, minLimit );
			} else if( map->getGroundHeight( xx, yy ) > maxLimit ) {
				map->setGroundHeight( xx, yy, maxLimit );
			}			
		}
	}
}

void OutdoorGenerator::addPath( Map *map, ShapePalette *shapePal, Sint16 mapx, Sint16 mapy, const char *shapeName ) {
	GLShape *shape = shapePal->findShapeByName( shapeName );
	addFloor( map, shapePal, mapx, mapy, false, shape );
	keepFloor[ mapx + MAP_WIDTH * mapy ] = shape;
	for( int cx = -1; cx < 2; cx++ ) {
		for( int cy = -1; cy < 2; cy++ ) {
			flattenPathChunk( map, mapx + ( cx * MAP_UNIT ), mapy + ( cy * MAP_UNIT ) );
		}
	}
}

void OutdoorGenerator::addFloor( Map *map, ShapePalette *shapePal, Sint16 mapx, Sint16 mapy, bool doFlattenChunk, GLShape *shape ) {
  if( map->getFloorPosition( mapx, mapy + MAP_UNIT ) ) return;
	if( doFlattenChunk ) flattenChunk( map, mapx, mapy );
  map->setFloorPosition( mapx, mapy + MAP_UNIT, shape );
}

void OutdoorGenerator::flattenChunk( Map *map, Sint16 mapX, Sint16 mapY, float height ) {
	int chunkX = ( mapX - MAP_OFFSET ) / MAP_UNIT;
	int chunkY = ( mapY - MAP_OFFSET ) / MAP_UNIT;
	for( int x = -OUTDOORS_STEP; x <= MAP_UNIT + OUTDOORS_STEP; x++ ) {
		for( int y = -OUTDOORS_STEP; y <= MAP_UNIT + OUTDOORS_STEP; y++ ) {
			int xx = ( MAP_OFFSET + ( chunkX * MAP_UNIT ) + x ) / OUTDOORS_STEP;
			int yy = ( MAP_OFFSET + ( chunkY * MAP_UNIT ) + y ) / OUTDOORS_STEP;
			map->setGroundHeight( xx, yy, height );
		}
	}
}

void OutdoorGenerator::flattenPathChunk( Map *map, Sint16 mapx, Sint16 mapy ) {
	if( !map->getFloorPosition( mapx, mapy + MAP_UNIT ) ) return;
	int chunkX = ( mapx - MAP_OFFSET ) / MAP_UNIT;
	int chunkY = ( mapy - MAP_OFFSET ) / MAP_UNIT;
	for( int x = OUTDOORS_STEP; x <= MAP_UNIT - OUTDOORS_STEP; x++ ) {
		for( int y = OUTDOORS_STEP; y <= MAP_UNIT - OUTDOORS_STEP; y++ ) {
			int xx = ( MAP_OFFSET + ( chunkX * MAP_UNIT ) + x ) / OUTDOORS_STEP;
			int yy = ( MAP_OFFSET + ( chunkY * MAP_UNIT ) + y ) / OUTDOORS_STEP;
			map->setGroundHeight( xx, yy, 0 );
		}
	}
	for( int x = OUTDOORS_STEP; x <= MAP_UNIT - OUTDOORS_STEP; x++ ) {
		if( map->getFloorPosition( mapx, mapy ) ) {
			int y = 0;
			int xx = ( MAP_OFFSET + ( chunkX * MAP_UNIT ) + x ) / OUTDOORS_STEP;
			int yy = ( MAP_OFFSET + ( chunkY * MAP_UNIT ) + y ) / OUTDOORS_STEP;
			map->setGroundHeight( xx, yy, 0 );
		}
		if( map->getFloorPosition( mapx, mapy + MAP_UNIT + MAP_UNIT ) ) {
			int y = MAP_UNIT - 1;
			int xx = ( MAP_OFFSET + ( chunkX * MAP_UNIT ) + x ) / OUTDOORS_STEP;
			int yy = ( MAP_OFFSET + ( chunkY * MAP_UNIT ) + y ) / OUTDOORS_STEP;
			map->setGroundHeight( xx, yy, 0 );
		}
	}
	for( int y = OUTDOORS_STEP; y <= MAP_UNIT - OUTDOORS_STEP; y++ ) {
		if( map->getFloorPosition( mapx - MAP_UNIT, mapy + MAP_UNIT ) ) {
			int x = 0;
			int xx = ( MAP_OFFSET + ( chunkX * MAP_UNIT ) + x ) / OUTDOORS_STEP;
			int yy = ( MAP_OFFSET + ( chunkY * MAP_UNIT ) + y ) / OUTDOORS_STEP;
			map->setGroundHeight( xx, yy, 0 );
		}
		if( map->getFloorPosition( mapx + MAP_UNIT, mapy + MAP_UNIT ) ) {
			int x = MAP_UNIT - 1;
			int xx = ( MAP_OFFSET + ( chunkX * MAP_UNIT ) + x ) / OUTDOORS_STEP;
			int yy = ( MAP_OFFSET + ( chunkY * MAP_UNIT ) + y ) / OUTDOORS_STEP;
			map->setGroundHeight( xx, yy, 0 );
		}
	}
}

struct ShapeLimit {
  GLShape *shape;
  float start, end;
};
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
  
  float roll = Util::roll( 0.0f, trees[ trees.size() - 1 ].end - 0.001f);

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
}

MapRenderHelper* OutdoorGenerator::getMapRenderHelper() {
	// we need fog
	return MapRenderHelper::helpers[ MapRenderHelper::OUTDOOR_HELPER ];
	//return MapRenderHelper::helpers[ MapRenderHelper::DEBUG_OUTDOOR_HELPER ];
}

// =====================================================
// =====================================================
// generation
//
void OutdoorGenerator::generate( Map *map, ShapePalette *shapePal ) {
  cellular[0][0]->generate( true, true, 4 );
	cellular[0][0]->makeAccessible( WIDTH_IN_NODES - 1, DEPTH_IN_NODES / 2 );
	cellular[0][0]->makeAccessible( WIDTH_IN_NODES / 2, DEPTH_IN_NODES - 1 );
	cellular[0][0]->makeMinSpace( 4 );
	//cellular[0][0]->print();
	
	cellular[1][0]->generate( true, true, 4 );
	cellular[1][0]->makeAccessible( 0, DEPTH_IN_NODES / 2 );
	cellular[1][0]->makeAccessible( WIDTH_IN_NODES / 2, DEPTH_IN_NODES - 1 );
	cellular[1][0]->makeMinSpace( 4 );
	//cellular[1][0]->print();

	cellular[0][1]->generate( true, true, 4 );
	cellular[0][1]->makeAccessible( WIDTH_IN_NODES - 1, DEPTH_IN_NODES / 2 );
	cellular[0][1]->makeAccessible( WIDTH_IN_NODES / 2, 0 );
	cellular[0][1]->makeMinSpace( 4 );
	//cellular[0][1]->print();

	cellular[1][1]->generate( true, true, 4 );
	cellular[1][1]->makeAccessible( 0, DEPTH_IN_NODES / 2 );
	cellular[1][1]->makeAccessible( WIDTH_IN_NODES / 2, 0 );
	cellular[1][1]->makeMinSpace( 4 );
	//cellular[1][1]->print();
	createGround();
}

void OutdoorGenerator::createGround() {
	// create the undulating ground
	float amp = 1.0f;
	float freq = 40.0f;
	for( int x = 0; x < MAP_STEP_WIDTH; x++ ) {
		for( int y = 0; y < MAP_STEP_DEPTH; y++ ) {
			// fixme: use a more sinoid function here
			// ground[x][y] = ( 1.0f * rand() / RAND_MAX );
			float a = Util::roll( 0.25f, amp );
			float f = freq / 2 + Util::roll( 0.25f, freq / 2 );
			ground[x][y] = a + 
				( a * 
					sin( PI / ( 180.0f / static_cast<float>( x * OUTDOORS_STEP * f ) ) ) * 
					cos( PI / ( 180.0f / static_cast<float>( y * OUTDOORS_STEP  * f )) ) );
			if( ground[x][y] < 0 ) ground[x][y] = 0;
		}
	}
}

void OutdoorGenerator::addFurniture(Map *map, ShapePalette *shapePal) {
}

void OutdoorGenerator::addContainers(Map *map, ShapePalette *shapePal) {	
}

void OutdoorGenerator::lockDoors( Map *map, ShapePalette *shapePal ) {
	// don't lock anything
}

void OutdoorGenerator::addMonsters(Map *levelMap, ShapePalette *shapePal) {
	// add a few misc. monsters in the corridors (use objectCount to approx. number of wandering monsters)
	for(int i = 0; i < objectCount; i++) {
		Monster *monster = Monster::getRandomMonster(getBaseMonsterLevel());
		if(!monster) {
			cerr << "Warning: no monsters defined for level: " << level << endl;
			break;
		}
		GLShape *shape = 
			scourge->getShapePalette()->getCreatureShape(monster->getModelName(), 
																									 monster->getSkinName(), 
																									 monster->getScale(),
												 monster);
		Creature *creature = scourge->getSession()->newCreature(monster, shape);
		int x, y;                           
		getRandomLocation(levelMap, creature->getShape(), &x, &y);
		addItem(levelMap, creature, NULL, NULL, x, y);
		creature->moveTo(x, y, 0);
	}
}																															 	

void OutdoorGenerator::printMaze() {
	for( int x = 0; x < 2; x++ ) {
		for( int y = 0; y < 2; y++ ) {
			cerr << "x=" << x << " y=" << y << endl;
			cellular[x][y]->print();
		}
	}
}
/*
void OutdoorGenerator::addRugs( Map *map, ShapePalette *shapePal ) {
     // no rugs
}
*/
void OutdoorGenerator::addTraps( Map *map, ShapePalette *shapePal ) {
     // no traps
}
     
void OutdoorGenerator::deleteFreeSpaceMap( Map *map, ShapePalette *shapePal ) {
	TerrainGenerator::deleteFreeSpaceMap( map, shapePal );
	// remove the floor
	for( int x = MAP_OFFSET; x < MAP_WIDTH - MAP_OFFSET; x += MAP_UNIT ) {
		for( int y = MAP_OFFSET; y < MAP_DEPTH - MAP_OFFSET; y += MAP_UNIT ) {
			if( keepFloor.find( x + MAP_WIDTH * y ) != keepFloor.end() ) {
				map->setFloorPosition( (Sint16)x, (Sint16)y + MAP_UNIT, keepFloor[ x + MAP_WIDTH * y ] );
			} else {
				map->removeFloorPosition( (Sint16)x, (Sint16)y + MAP_UNIT );
			}
		}
	}
}
