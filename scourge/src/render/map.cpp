/***************************************************************************
                map.cpp  -  Manages and renders the level map
                             -------------------
    begin                : Sat May 3 2003
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

#include "../common/constants.h"
#include "map.h"
#include "mapmemory.h"
#include "mapsettings.h"
#include "effect.h"
#include "frustum.h"
#include "location.h"
#include "shape.h"
#include "shapes.h"
#include "glshape.h"
#include "virtualshape.h"
#include "md2shape.h"
#include "renderedprojectile.h"
#include "renderedcreature.h"
#include "rendereditem.h"
#include "mapadapter.h"
#include "../io/zipfile.h"
#include "../rpg/spell.h"
#include "../debug.h"
#include "projectilerenderer.h"
#include "../quickhull.h"
#include "indoor.h"
#include "outdoor.h"

using namespace std;

// ###### MS Visual C++ specific ###### 
#if defined(_MSC_VER) && defined(_DEBUG)
# define new DEBUG_NEW
# undef THIS_FILE
  static char THIS_FILE[] = __FILE__;
#endif
  
#define MOUSE_ROT_DELTA 2

#define ZOOM_DELTA 1.2f

#define KEEP_MAP_SIZE 0

#define MVW 100
#define MVD 100

//#define DEBUG_RENDER 1

// this is the clockwise order of movements
int Map::dir_index[] = { Constants::MOVE_UP, Constants::MOVE_LEFT, Constants::MOVE_DOWN, Constants::MOVE_RIGHT };

bool Map::debugMd2Shapes = false;

bool mouseMove = false;
int mouseMoveX = 0;
int mouseMoveY = 0;
float moveAngle = 0;
float moveDelta = 0;


// how long to wait between quakes
#define QUAKE_DELAY 45000

// how long each quake lasts
#define QUAKE_DURATION 4000

// how fast is a quake?
#define QUAKE_TICK_FREQ 50

float quakeOffsX = 0;
float quakeOffsY = 0;
Uint32 quakeStartTime = 0;
Uint32 nextQuakeStartTime = 0;
Uint32 lastQuakeTick = 0;
bool quakeOnce = false;

MapMemoryManager Map::mapMemoryManager;

Map::Map( MapAdapter *adapter, Preferences *preferences, Shapes *shapes ) {
	roofAlphaUpdate = 0;
	roofAlpha = 1;

	hasWater = false;

	startx = starty = 128;
	cursorMapX = cursorMapY = cursorMapZ = MAP_WIDTH + 1;
	cursorFlatMapX = cursorFlatMapY = MAP_WIDTH + 1;
	cursorChunkX = cursorChunkY = ( MAP_CHUNKS_X ) + 1;
	cursorWidth = 1;
	cursorDepth = 1;
	cursorHeight = MAP_WALL_HEIGHT;
	cursorZ = 0;
	lastOutlinedX = lastOutlinedY = lastOutlinedZ = MAP_WIDTH;

	mouseMoveScreen = true;
	mouseZoom = mouseRot = false;
	mouseRotDir = 1;
	move = 0;

	mapViewWidth = MVW;
	mapViewDepth = MVD;

	chunkCount = 0;
	frustum = new CFrustum();
	useFrustum = true;

	this->adapter = adapter;
	this->preferences = preferences;
	this->shapes = shapes;

	// only use 1 (disabled) or 0 (enabled)
	LIGHTMAP_ENABLED = 0;
	zoom = 1.0f;
	zoomIn = zoomOut = false;
	x = y = 0;
	mapx = mapy = 0.0f;
	floorOnly = false;
	//alwaysCenter = true;

	debugX = debugY = debugZ = -1;

	mapChanged = true;
	resortShapes = true;
	groundVisible = true;

	floorTexWidth = floorTexHeight = 0;
	
	mapCenterCreature = NULL;

	this->xrot = 0.0f;
	this->yrot = 30.0f;
	this->zrot = 45.0f;
	this->xRotating = this->yRotating = this->zRotating = 0.0f;

	setViewArea( 0, 0,
	             adapter->getScreenWidth(),
	             adapter->getScreenHeight() );

	float adjust = static_cast<float>( viewWidth ) / 800.0f;
	this->xpos = static_cast<float>( viewWidth ) / 2.0f / adjust;
	this->ypos = static_cast<float>( viewHeight ) / 2.0f / adjust;
	this->zpos = 0.0f;

	this->debugGridFlag = false;
	this->drawGridFlag = false;


	// initialize shape graph of "in view shapes"
	for ( int x = 0; x < MAP_WIDTH; x++ ) {
		for ( int y = 0; y < MAP_DEPTH; y++ ) {
			floorPositions[x][y] = NULL;
			itemPos[x][y] = NULL;
			for ( int z = 0; z < MAP_VIEW_HEIGHT; z++ ) {
				pos[x][y][z] = NULL;
				effect[x][y][z] = NULL;
			}
		}
	}
	// Init the pos cache
	for ( int x = 0; x < MAX_POS_CACHE; x++ ) {
		posCache[x] = NULL;
	}
	nbPosCache = -1;

	for ( int x = 0; x < MAP_WIDTH; x++ ) {
		for ( int y = 0; y < MAP_DEPTH; y++ ) {
			ground[x][y] = 0;
		}
	}
	heightMapEnabled = false;
	for ( int i = 0; i < 4; i++ )
		debugHeightPosXX[i] = debugHeightPosYY[i] = 0;


	// initialize the lightmap
	for ( int x = 0; x < MAP_CHUNKS_X; x++ ) {
		for ( int y = 0; y < MAP_CHUNKS_Y; y++ ) {
			lightMap[x][y] = ( LIGHTMAP_ENABLED ? 0 : 1 );
		}
	}

	lightMapChanged = true;
	selectedDropTarget = NULL;

	helper = NULL;

	laterCount = stencilCount = otherCount = damageCount = roofCount = lightCount = 0;

	quakesEnabled = false;

	refreshGroundPos = true;

	outdoorShadow = adapter->getNamedTexture( "outdoors_shadow" );
	outdoorShadowTree = adapter->getNamedTexture( "outdoors_shadow_tree" );
	//waterTexture = adapter->getNamedTexture( "water" );

	hackBlockingPos = new Location();
	hackBlockingPos->creature = NULL;
	hackBlockingPos->heightPos = 15;
	hackBlockingPos->item = NULL;
	hackBlockingPos->outlineColor = NULL;
	hackBlockingPos->shape = NULL;
	hackBlockingPos->x = hackBlockingPos->y = hackBlockingPos->z = 0;

	selectedTrapIndex = -1;

	isCurrentlyUnderRoof = isRoofShowing = true;
	weather = WEATHER_CLEAR;

	gridEnabled = true;
	
	indoor = new Indoor( this );
	outdoor = new Outdoor( this );
	
	adapter->writeLogMessage( Constants::getMessage( Constants::WELCOME ), Constants::MSGTYPE_SYSTEM );
	adapter->writeLogMessage( "----------------------------------", Constants::MSGTYPE_SYSTEM );
}

Map::~Map() {
	removeAllEffects();
	// can't just call reset() since it consults with external objects
	set<Location*> deleted;
	for ( int xp = 0; xp < MAP_WIDTH; xp++ ) {
		for ( int yp = 0; yp < MAP_DEPTH; yp++ ) {
			if ( floorPositions[xp][yp] ) {
				Uint32 key = createPairKey( xp, yp );
				if ( water.find( key ) != water.end() ) {
					delete water[ key ];
					water.erase( key );
				}
			}
			if ( itemPos[xp][yp] ) {
				Location *p = itemPos[xp][yp];
				if ( deleted.find( p ) == deleted.end() ) deleted.insert( p );
				itemPos[xp][yp] = NULL;
			}
			for ( int zp = 0; zp < MAP_VIEW_HEIGHT; zp++ ) {
				if ( pos[xp][yp][zp] ) {
					Location *p = pos[xp][yp][zp];
					if ( deleted.find( p ) == deleted.end() ) deleted.insert( p );
					pos[xp][yp][zp] = NULL;
				}
			}
		}
	}
	for ( set<Location*>::iterator i = deleted.begin(); i != deleted.end(); ++i ) {
		Location *p = *i;
		mapMemoryManager.deleteLocation( p );
	}
	water.clear();

	delete hackBlockingPos;
	delete frustum;
	// did not create it: delete helper;

	for ( size_t i = 0; i < trapList.size(); ++i ) {
		for ( size_t j = 0; j < trapList[i].hull.size(); ++j ) {
			delete trapList[i].hull[j];
		}
	}
}

/// After work is done, reset everything.

void Map::reset() {

	creatureMap.clear();
	creatureEffectMap.clear();
	creatureLightMap.clear();

	roofAlphaUpdate = 0;
	roofAlpha = 1;

	//cerr << "reset 1" << endl;
	edited = false;
	strcpy( this->name, "" );
	hasWater = false;

	//cerr << "reset 2" << endl;
	// remove locking info
	clearLocked();

	//cerr << "reset 3" << endl;
	// remove area effects
	removeAllEffects();

	//cerr << "reset 4" << endl;
	// clear map
	set<Location*> deleted;
	for ( int xp = 0; xp < MAP_WIDTH; xp++ ) {
		for ( int yp = 0; yp < MAP_DEPTH; yp++ ) {
			if ( floorPositions[xp][yp] ) {
				Uint32 key = createPairKey( xp, yp );
				if ( water.find( key ) != water.end() ) {
					delete water[ key ];
					water.erase( key );
				}
			}
			if ( itemPos[xp][yp] ) {
				Location *p = itemPos[xp][yp];
				if ( deleted.find( p ) == deleted.end() ) deleted.insert( p );
				itemPos[xp][yp] = NULL;
			}
			for ( int zp = 0; zp < MAP_VIEW_HEIGHT; zp++ ) {
				if ( pos[xp][yp][zp] ) {
					Location *p = pos[xp][yp][zp];
					if ( deleted.find( p ) == deleted.end() ) deleted.insert( p );
					pos[xp][yp][zp] = NULL;
				}
			}
		}
	}
	for ( set<Location*>::iterator i = deleted.begin(); i != deleted.end(); ++i ) {
		Location *p = *i;
		mapMemoryManager.deleteLocation( p );
	}
	water.clear();

	//cerr << "reset 5" << endl;
	zoom = 1.0f;
	zoomIn = zoomOut = false;
	x = y = 0;
	mapx = mapy = 0.0f;
	floorOnly = false;
	//alwaysCenter = true;
	debugX = debugY = debugZ = -1;
	mapChanged = true;
	groundVisible = true;
	resortShapes = true;
	lastOutlinedX = lastOutlinedY = lastOutlinedZ = MAP_WIDTH;
	floorTexWidth = floorTexHeight = 0;
	floorTex.clear();
	mapCenterCreature = NULL;
	secretDoors.clear();

	this->xrot = 0.0f;
	this->yrot = 30.0f;
	this->zrot = 45.0f;
	this->xRotating = this->yRotating = this->zRotating = 0.0f;

	setViewArea( 0, 0,
	             adapter->getScreenWidth(),
	             adapter->getScreenHeight() );

	float adjust = static_cast<float>( viewWidth ) / 800.0f;
	this->xpos = static_cast<float>( viewWidth ) / 2.0f / adjust;
	this->ypos = static_cast<float>( viewHeight ) / 2.0f / adjust;
	this->zpos = 0.0f;

	this->debugGridFlag = false;
	this->drawGridFlag = false;

	//cerr << "reset 6" << endl;

	for ( int x = 0; x < MAP_CHUNKS_X; x++ ) {
		for ( int y = 0; y < MAP_CHUNKS_Y; y++ ) {
			rugPos[x][y].texture.clear();
		}
	}

	// initialize shape graph of "in view shapes"
	for ( int x = 0; x < MAP_WIDTH; x++ ) {
		for ( int y = 0; y < MAP_DEPTH; y++ ) {
			floorPositions[x][y] = NULL;
			itemPos[x][y] = NULL;
			for ( int z = 0; z < MAP_VIEW_HEIGHT; z++ ) {
				pos[x][y][z] = NULL;
				effect[x][y][z] = NULL;
			}
		}
	}
	// Init the pos cache
	//for(int x = 0; x < MAX_POS_CACHE; x++) {
	//  posCache[x] = NULL;
	//}
	//nbPosCache = -1;

	//cerr << "reset 7" << endl;
	// initialize the lightmap
	for ( int x = 0; x < MAP_CHUNKS_X; x++ ) {
		for ( int y = 0; y < MAP_CHUNKS_Y; y++ ) {
			lightMap[x][y] = ( LIGHTMAP_ENABLED ? 0 : 1 );
		}
	}
	lightMapChanged = true;
	selectedDropTarget = NULL;

	if ( helper ) helper->reset();

	laterCount = stencilCount = otherCount = damageCount = roofCount = lightCount = 0;

	quakesEnabled = false;
	for ( int x = 0; x < MAP_WIDTH; x++ ) {
		for ( int y = 0; y < MAP_DEPTH; y++ ) {
			ground[x][y] = 0;
			groundTex[x][y].clear();
			for ( int z = 0; z < MAX_OUTDOOR_LAYER; z++ ) {
				int tx = x / OUTDOORS_STEP;
				int ty = y / OUTDOORS_STEP;
				outdoorTex[tx][ty][z].texture.clear();
			}
		}
	}
	heightMapEnabled = false;
	for ( int i = 0; i < 4; i++ )
		debugHeightPosXX[i] = debugHeightPosYY[i] = 0;

	refreshGroundPos = true;

	isCurrentlyUnderRoof = isRoofShowing = true;

	clearTraps();

	gates.clear();
	teleporters.clear();

	gridEnabled = true;
}

/// Sets the size and position of the viewport.

void Map::setViewArea( int x, int y, int w, int h ) {
	//viewX = x;
//  viewY = y;
	viewX = 0;
	viewY = 0;
	viewWidth = w;
	viewHeight = h;

	float adjust = static_cast<float>( viewWidth ) / 800.0f;
	if ( preferences->getKeepMapSize() ) {
		zoom = static_cast<float>( adapter->getScreenWidth() ) / static_cast<float>( w );
	}
	xpos = static_cast<int>( static_cast<float>( viewWidth ) / zoom / 2.0f / adjust );
	ypos = static_cast<int>( static_cast<float>( viewHeight ) / zoom / 2.0f / adjust );

	refresh();
}

/// Centers view on map position x,y.

void Map::center( Sint16 x, Sint16 y, bool force ) {
	Sint16 nx = x - mapViewWidth / 2;
	Sint16 ny = y - mapViewDepth / 2;
	if ( preferences->getAlwaysCenterMap() || force ) {
		// relocate
		this->x = nx;
		this->y = ny;
		this->mapx = nx;
		this->mapy = ny;
	}
}

/// Removes effects that are done playing.

void Map::removeCurrentEffects() {
	for ( map<Uint32, EffectLocation*>::iterator i = currentEffectsMap.begin();
	        i != currentEffectsMap.end(); ) {
		Uint32 key = i->first;
		EffectLocation *pos = i->second;
		if ( !pos->isEffectOn() ) {
			int x, y, z;
			decodeTripletKey( key, &x, &y, &z );
			currentEffectsMap.erase( i++ );
			removeEffect( x, y, z );
			resortShapes = mapChanged = true;
		} else {
			++i;
		}
	}
}

bool Map::checkLightMap( int chunkX, int chunkY ) {
	return !helper->isLightMapEnabled() || lightMap[chunkX][chunkY];
}

/// Sets up the shapes array for a specified layer.

/// The position and dimensions of the map chunk to draw are to be provided.
/// If forGround is true, it sets up the ground layer.
/// If forWater is true, it sets up the indoors water layer.
/// Otherwise it populates the shapes array.

void Map::setupShapes( bool forGround, bool forWater, int *csx, int *cex, int *csy, int *cey ) {
	if ( !forGround && !forWater ) {
		laterCount = stencilCount = otherCount = damageCount = roofCount = lightCount = 0;
		creatureMap.clear();
		creatureEffectMap.clear();
		creatureLightMap.clear();
		trapSet.clear();
		mapChanged = false;
	}

	chunkCount = 0;
	int chunkOffsetX, chunkOffsetY;
	int chunkStartX, chunkStartY;
	int chunkEndX, chunkEndY;
	calculateChunkInfo( &chunkOffsetX, &chunkOffsetY, &chunkStartX, &chunkStartY, &chunkEndX, &chunkEndY );
	if ( csx ) {
		*csx = chunkStartX;
		*cex = chunkEndX;
		*csy = chunkStartY;
		*cey = chunkEndY;
	}

	set<Location*> seenPos;
	Shape *shape;
	int posX, posY;
	float xpos2, ypos2, zpos2;
	for ( int chunkX = chunkStartX; chunkX < chunkEndX; chunkX++ ) {
		for ( int chunkY = chunkStartY; chunkY < chunkEndY; chunkY++ ) {

			// remember the chunk's starting pos.
			float chunkPosX = static_cast<float>( ( chunkX - chunkStartX ) * MAP_UNIT + chunkOffsetX ) * MUL;
			float chunkPosY = static_cast<float>( ( chunkY - chunkStartY ) * MAP_UNIT + chunkOffsetY ) * MUL;

			// frustum testing (including extra for roof pieces)
			if ( useFrustum && !frustum->CubeInFrustum( chunkPosX, chunkPosY, 0.0f, static_cast<float>( MAP_UNIT ) * MUL ) ) {
				continue;
			}


			// FIXME: works but slow. Use 1 polygon instead (like floor)
			// special cave edge code
			if ( !( forGround || forWater ) && floorTexWidth > 0 && !isHeightMapEnabled() && ( chunkX < 0 || chunkY < 0 ) ) {
				for ( int yp = CAVE_CHUNK_SIZE; yp < MAP_UNIT + CAVE_CHUNK_SIZE; yp += CAVE_CHUNK_SIZE ) {
					for ( int xp = 0; xp < MAP_UNIT; xp += CAVE_CHUNK_SIZE ) {
						xpos2 = static_cast<float>( ( chunkX - chunkStartX ) * MAP_UNIT + xp + chunkOffsetX ) * MUL;
						ypos2 = static_cast<float>( ( chunkY - chunkStartY ) * MAP_UNIT - CAVE_CHUNK_SIZE + yp + chunkOffsetY ) * MUL;
						setupPosition( 0, CAVE_CHUNK_SIZE, 0, xpos2, ypos2, 0, pos[0][CAVE_CHUNK_SIZE][0], NULL );
					}
				}
			}

			if ( chunkX < 0 || chunkX > MAP_CHUNKS_X || chunkY < 0 || chunkY > MAP_CHUNKS_Y )
				continue;

			// store this chunk
			chunks[chunkCount].x = chunkPosX;
			chunks[chunkCount].y = chunkPosY;
			chunks[chunkCount].cx = chunkX;
			chunks[chunkCount].cy = chunkY;
			chunkCount++;

			// FIXME: this works except it draws other doors on the same
			// chunk that should be hidden. To really fix it, we need to
			// keep track of which side of the chunk to draw.
			Uint16 drawSide = 0;
			if ( !checkLightMap( chunkX, chunkY ) ) {
				if ( forGround || forWater ) continue;
				else {
					// look to the left
					if ( chunkX >= 1 && lightMap[chunkX - 1][chunkY] ) drawSide |= Constants::MOVE_LEFT;
					// look to the right
					if ( chunkX + 1 < MAP_CHUNKS_X && lightMap[chunkX + 1][chunkY] ) drawSide |= Constants::MOVE_RIGHT;
					// look above
					if ( chunkY - 1 >= 0 && lightMap[chunkX][chunkY - 1] ) drawSide |= Constants::MOVE_UP;
					// look below
					if ( chunkY + 1 < MAP_CHUNKS_Y && lightMap[chunkX][chunkY + 1] ) drawSide |= Constants::MOVE_DOWN;
					// if not, skip this chunk
					if ( !drawSide ) continue;
				}
			}

			// draw rugs
			if ( ( forGround || forWater ) && hasRugAtPosition( chunkX, chunkY ) ) {
				xpos2 = static_cast<float>( ( chunkX - chunkStartX ) * MAP_UNIT + chunkOffsetX ) * MUL;
				ypos2 = static_cast<float>( ( chunkY - chunkStartY ) * MAP_UNIT + chunkOffsetY ) * MUL;
				getRender()->drawRug( &rugPos[ chunkX ][ chunkY ], xpos2, ypos2, chunkX, chunkY );
			}

			for ( int yp = 0; yp < MAP_UNIT; yp++ ) {
				for ( int xp = 0; xp < MAP_UNIT; xp++ ) {
					/**
					 In scourge, shape coordinates are given by their
					 left-bottom corner. So the +1 for posY moves the
					 position 1 unit down the Y axis, which is the
					 unit square's bottom left corner.
					 */
					posX = chunkX * MAP_UNIT + xp + MAP_OFFSET;
					posY = chunkY * MAP_UNIT + yp + MAP_OFFSET + 1;

					// show traps
					int trapIndex = getTrapAtLoc( posX, posY );
					if ( trapIndex != -1 && checkLightMap( chunkX, chunkY ) )
						trapSet.insert( ( Uint8 )trapIndex );

					if ( forGround || forWater ) {
						shape = floorPositions[posX][posY];
						if ( shape ) {
							//cerr << "pos=" << posX << "," << posY << endl;
							//cerr << "\tfloor shape=" << shape->getName() << endl;
							xpos2 = static_cast<float>( ( chunkX - chunkStartX ) * MAP_UNIT + xp + chunkOffsetX ) * MUL;
							ypos2 = static_cast<float>( ( chunkY - chunkStartY ) * MAP_UNIT - shape->getDepth() + yp + chunkOffsetY ) * MUL;

							if ( forWater ) {
								getRender()->drawWaterPosition( posX, posY, xpos2, ypos2, shape );
							} else {
								getRender()->drawGroundPosition( posX, posY, xpos2, ypos2, shape );
							}
						}
					} else {

						if ( checkLightMap( chunkX, chunkY ) && itemPos[posX][posY] &&
						        itemPos[posX][posY]->x == posX && itemPos[posX][posY]->y == posY ) {

							shape = itemPos[posX][posY]->shape;

							xpos2 = static_cast<float>( ( chunkX - chunkStartX ) * MAP_UNIT + xp + chunkOffsetX ) * MUL;
							ypos2 = static_cast<float>( ( chunkY - chunkStartY ) * MAP_UNIT - shape->getDepth() + yp + chunkOffsetY ) * MUL;

							setupPosition( posX, posY, 0, xpos2, ypos2, 0, itemPos[posX][posY], NULL );
						}

						checkUnderRoof();

						for ( int zp = 0; zp < MAP_VIEW_HEIGHT; zp++ ) {
							if ( checkLightMap( chunkX, chunkY ) && effect[posX][posY][zp] && !effect[posX][posY][zp]->isInDelay() ) {
								xpos2 = static_cast<float>( ( chunkX - chunkStartX ) * MAP_UNIT + xp + chunkOffsetX ) * MUL;
								ypos2 = static_cast<float>( ( chunkY - chunkStartY ) * MAP_UNIT - 1 + yp + chunkOffsetY ) * MUL;
								zpos2 = static_cast<float>( zp ) * MUL;

								setupPosition( posX, posY, zp - effect[posX][posY][zp]->z, xpos2, ypos2, zpos2,
								               NULL, effect[posX][posY][zp] );
							}

							Location *location = pos[posX][posY][zp];
							if ( location && seenPos.find( location ) == seenPos.end() ) {
								seenPos.insert( location );
								//if( location && location->x == posX && location->y == posY && location->z == zp ) {
								setupLocation( location, drawSide, chunkStartX, chunkStartY, chunkOffsetX, chunkOffsetY );
							}
						}
					}
				}
			}
		}
	}
}

/// Update the "under the roof" status. Return true if the "under the roof" status has changed.

bool Map::checkUnderRoof() {
	// skip roofs if inside
	bool oldRoof = isCurrentlyUnderRoof;
	isCurrentlyUnderRoof = true;
	if ( settings->isGridShowing() ) {
		isCurrentlyUnderRoof = !isRoofShowing;
	} else if ( adapter->getPlayer() ) {
		int px = toint( adapter->getPlayer()->getX() + adapter->getPlayer()->getShape()->getWidth() / 2 );
		int py = toint( adapter->getPlayer()->getY() - 1 - adapter->getPlayer()->getShape()->getDepth() / 2 );
		Location *roof = getLocation( px, py, MAP_WALL_HEIGHT );
		if ( !( roof && roof->shape && roof->shape->isRoof() ) ) {
			isCurrentlyUnderRoof = false;
		}
	}
	return ( isCurrentlyUnderRoof != oldRoof );
}

/// Sets up a map location and checks whether it is visible/accessible.

void Map::setupLocation( Location *location, Uint16 drawSide, int chunkStartX, int chunkStartY, int chunkOffsetX, int chunkOffsetY ) {

	int posX, posY, posZ;
	bool lightEdge;
	float xpos2, ypos2, zpos2;
	int chunkX, chunkY;
	calculateLocationInfo( location, chunkStartX, chunkStartY, chunkOffsetX, chunkOffsetY, drawSide,
	                       &posX, &posY, &posZ, &xpos2, &ypos2, &zpos2, &chunkX, &chunkY, &lightEdge );

	// visible or on the edge or a roof piece
	if ( checkLightMap( chunkX, chunkY ) || lightEdge ) {
		if ( location->creature ) {
			location->heightPos = findMaxHeightPos( location->creature->getX(),
			                                        location->creature->getY(),
			                                        location->creature->getZ() );
		}

		//if( !useFrustum || frustum->ShapeInFrustum( xpos2, ypos2, zpos2, shape ) ) {
		setupPosition( posX, posY, posZ, xpos2, ypos2, zpos2, location, NULL );
		//}
	}

}

void Map::setupPosition( int posX, int posY, int posZ,
                         float xpos2, float ypos2, float zpos2,
                         Location *pos,
                         EffectLocation *effect ) {

	// This really doesn't make a difference unfortunately.
	//if(!isOnScreen(posX, posY, posZ)) return;

	GLuint name;
	name = posX + ( MAP_WIDTH * ( posY ) ) + ( MAP_WIDTH * MAP_DEPTH * posZ );
	
	// lights
	if( pos && ( ( pos->shape && pos->shape->getLightEmitter() ) || ( pos->creature == adapter->getPlayer() ) ) ) {
		lights[lightCount].set( this, xpos2, ypos2, zpos2, effect, pos, name, posX, posY, false, true, false );
		if( pos->creature ) {
			creatureLightMap[pos->creature] = &(lights[lightCount]);
		}
		lightCount++;		
	}

	// special effects
	if ( effect || ( pos && pos->creature && pos->creature->isEffectOn() && !pos->shape->isRoof() ) ) {
		damage[damageCount].set( this, xpos2, ypos2, zpos2, effect, pos, name, posX, posY, false, false, true );
		if ( pos && pos->creature ) {
			creatureEffectMap[pos->creature] = &( damage[damageCount] );
		}
		damageCount++;

		// don't draw shape if it's an area effect
		if ( !( pos && pos->creature ) ) return;
	}

	// roofs
	if ( pos->shape->isRoof() ) {
		roof[roofCount].set( this, xpos2, ypos2, zpos2, effect, pos, name, posX, posY, false, false, false );
		roofCount++;
	} else if ( pos->shape->isStencil() ) {
		// walls
		stencil[stencilCount].set( this, xpos2, ypos2, zpos2, effect, pos, name, posX, posY, false, false, false );
		stencilCount++;
	} else if( pos->shape->isBlended() ) {
		// torches, teleporters, etc.
		later[laterCount].set( this, xpos2, ypos2, zpos2, effect, pos, name, posX, posY, false, false, false );
		laterCount++;
	} else {
		// items, creatures, etc.
		other[otherCount].set( this, xpos2, ypos2, zpos2, effect, pos, name, posX, posY, false, false, false );
		if ( pos->creature ) {
			creatureMap[pos->creature] = &( other[otherCount] );
		}
		otherCount++;
	}
}

/// Current camera zoom as a percentage value.

float Map::getZoomPercent() {
	return ( zoom - settings->getMinZoomIn() ) / ( settings->getMaxZoomOut() - settings->getMinZoomIn() );
}

void Map::draw() {
	preDraw();
	getRender()->draw();
	postDraw();
}

/// Initialization before doing the actual drawing.

void Map::preDraw() {
	if ( refreshGroundPos ) {
		getRender()->createGroundMap();
		refreshGroundPos = false;
	}

	// must move map before calling getMapXY(Z)AtScreenXY!
	if ( move ) moveMap( move );

	if ( zoomIn ) {
		if ( zoom <= settings->getMinZoomIn() ) {
			zoomOut = false;
		} else {
			zoom /= ZOOM_DELTA;
			mapChanged = true;
		}
	} else if ( zoomOut ) {
		if ( zoom >= settings->getMaxZoomOut() ) {
			zoomOut = false;
		} else {
			zoom *= ZOOM_DELTA;
		}
	}

	float adjust = static_cast<float>( viewWidth ) / 800.0f;
	xpos = static_cast<int>( static_cast<float>( viewWidth ) / zoom / 2.0f / adjust );
	ypos = static_cast<int>( static_cast<float>( viewHeight ) / zoom / 2.0f / adjust );

	float oldrot;

	oldrot = yrot;
	if ( yRotating != 0 ) yrot += yRotating;
	if ( yrot >= settings->getMaxYRot() || yrot < 0 ) yrot = oldrot;

	oldrot = zrot;
	if ( zRotating != 0 ) {
		resortShapes = true;
		zrot += zRotating;
	}
	if ( zrot >= 360 ) zrot -= 360;
	if ( zrot < 0 ) zrot = 360 + zrot;

	if ( settings->isPlayerEnabled() && ( preferences->getAlwaysCenterMap() || mapCenterCreature ) ) {
		mapChanged = resortShapes = true;
	}
	initMapView();

	frustum->CalculateFrustum();
	if ( lightMapChanged && helper->isLightMapEnabled() ) configureLightMap();
	if ( !currentEffectsMap.empty() ) removeCurrentEffects();
	// populate the shape arrays
	if ( mapChanged ) {
		int csx, cex, csy, cey;
		setupShapes( false, false, &csx, &cex, &csy, &cey );
	}
}

/// Draws traps, and cleans up after drawing the main 3D view.

void Map::postDraw() {
	if ( adapter->isMouseIsMovingOverMap() ) {
		// find the map coordinates (must be done after drawing is complete)
		Location *pos = NULL;
		getMapXYZAtScreenXY( &cursorMapX, &cursorMapY, &cursorMapZ, &pos );
		cursorFlatMapX = cursorMapX;
		cursorFlatMapY = cursorMapY;
		cursorChunkX = ( cursorFlatMapX - MAP_OFFSET ) / MAP_UNIT;
		cursorChunkY = ( cursorFlatMapY - MAP_OFFSET ) / MAP_UNIT;
		if ( pos ) {
			cursorMapX = pos->x;
			cursorMapY = pos->y;
			cursorMapZ = pos->z;
		}
	}	

	glDisable( GL_SCISSOR_TEST );

	// cancel mouse-based map movement (middle button)
	if ( mouseRot ) {
		setXRot( 0 );
		setYRot( 0 );
		setZRot( 0 );
	}
	if ( mouseZoom ) {
		mouseZoom = false;
		setZoomIn( false );
		setZoomOut( false );
	}
}

bool Map::isValidPosition( int x, int y, int z ) {
	return x >= MAP_OFFSET && x <= MAP_WIDTH - MAP_OFFSET &&
		y >= MAP_OFFSET && y <= MAP_DEPTH - MAP_OFFSET &&
		z >= 0 && z < MAP_VIEW_HEIGHT;
}



/// Sorts the shapes in respect to the player's position.

/// Since rooms are rectangular, we can do this hack... a wall horizontal
/// wall piece will use the player's x coord. and its y coord.
/// A vertical one will use its X and the player's Y.
/// This is so that even if the wall extends below the player the entire
/// length has the same characteristics.

void Map::sortShapes( RenderedLocation *playerDrawLater, RenderedLocation *shapes, int shapeCount ) {
	GLdouble mm[16];
	glGetDoublev( GL_MODELVIEW_MATRIX, mm );
	GLdouble pm[16];
	glGetDoublev( GL_PROJECTION_MATRIX, pm );
	GLint vp[4];
	glGetIntegerv( GL_VIEWPORT, vp );

	GLdouble playerWinX, playerWinY, playerWinZ;
	gluProject( playerDrawLater->xpos, playerDrawLater->ypos, 0, mm, pm, vp, &playerWinX, &playerWinY, &playerWinZ );

	set< int > xset, yset;
	map< string, bool > cache;
	GLdouble objX, objY;
	for ( int i = 0; i < shapeCount; i++ ) {
		// skip square shapes the first time around
		if ( shapes[i].pos->shape->getWidth() == shapes[i].pos->shape->getDepth() ) {
			shapes[i].inFront = false;
			continue;
		} else if ( shapes[i].pos->shape->getWidth() > shapes[i].pos->shape->getDepth() ) {
			objX = playerDrawLater->xpos;
			objY = shapes[i].ypos;
			yset.insert( toint( shapes[i].ypos ) );
		} else {
			objX = shapes[i].xpos;
			objY = playerDrawLater->ypos;
			xset.insert( toint( shapes[i].xpos ) );
		}
		shapes[i].inFront = isShapeInFront( playerWinY, objX, objY, &cache, mm, pm, vp );
	}
	// now process square shapes: if their x or y lies on a wall-line, they're transparent
	for ( int i = 0; i < shapeCount; i++ ) {
		if ( shapes[i].pos->shape->getWidth() == shapes[i].pos->shape->getDepth() ) {
			if ( xset.find( toint( shapes[i].xpos ) ) != xset.end() ) {
				objX = shapes[i].xpos;
				objY = playerDrawLater->ypos;
			} else if ( yset.find( toint( shapes[i].ypos ) ) != yset.end() ) {
				objX = playerDrawLater->xpos;
				objY = shapes[i].ypos;
			} else {
				continue;
			}
			shapes[i].inFront = isShapeInFront( playerWinY, objX, objY, &cache, mm, pm, vp );
		}
	}
}

/// Is there a shape in front of the player, obscuring him/her?

bool Map::isShapeInFront( GLdouble playerWinY, GLdouble objX, GLdouble objY, map< string, bool > *cache, GLdouble *mm, GLdouble *pm, GLint *vp ) {

	GLdouble wallWinX, wallWinY, wallWinZ;
	bool b;
	char tmp[80];

	snprintf( tmp, 80, "%f,%f", objX, objY );
	string key = tmp;
	if ( cache->find( key ) == cache->end() ) {
		gluProject( objX, objY, 0, mm, pm, vp, &wallWinX, &wallWinY, &wallWinZ );
		b = ( wallWinY < playerWinY );
		( *cache )[ key ] = b;
	} else {
		b = ( *cache )[ key ];
	}
	return b;
}




/// Is the x,y,z location currently on the screen?

bool Map::isOnScreen( Uint16 mapx, Uint16 mapy, Uint16 mapz ) {
	glPushMatrix();

	// Initialize the scene w/o y rotation.
	initMapView( true );

	double obj_x = ( mapx - getX() + 1 ) * MUL;
	double obj_y = ( mapy - getY() - 2 ) * MUL;
	double obj_z = 0.0f;
	//double obj_z = mapz * MUL;
	double win_x, win_y, win_z;

	double projection[16];
	double modelview[16];
	GLint viewport[4];

	glGetDoublev( GL_PROJECTION_MATRIX, projection );
	glGetDoublev( GL_MODELVIEW_MATRIX, modelview );
	glGetIntegerv( GL_VIEWPORT, viewport );

	int res = gluProject( obj_x, obj_y, obj_z, modelview, projection, viewport, &win_x, &win_y, &win_z );

	glDisable( GL_SCISSOR_TEST );
	glPopMatrix();

	if ( res ) {
		win_y = adapter->getScreenHeight() - win_y;
		return ( win_x >= 0 && win_x < adapter->getScreenWidth() && win_y >= 0 && win_y < adapter->getScreenHeight() );
	}
	return false;
}

/// Converts a map position to a screen coordinate (where on the screen is tile x,y?)

void Map::getScreenXYAtMapXY( Uint16 mapx, Uint16 mapy, Uint16 *screenx, Uint16 *screeny ) {
	glPushMatrix();

	// Initialize the scene with y rotation.
	initMapView( false );

	double obj_x = ( mapx - getX() + 1 ) * MUL;
	double obj_y = ( mapy - getY() - 2 ) * MUL;
	double obj_z = 0.0f;
	//double obj_z = mapz * MUL;
	double win_x, win_y, win_z;

	double projection[16];
	double modelview[16];
	GLint viewport[4];

	glGetDoublev( GL_PROJECTION_MATRIX, projection );
	glGetDoublev( GL_MODELVIEW_MATRIX, modelview );
	glGetIntegerv( GL_VIEWPORT, viewport );

	int res = gluProject( obj_x, obj_y, obj_z, modelview, projection, viewport, &win_x, &win_y, &win_z );

	glDisable( GL_SCISSOR_TEST );
	glPopMatrix();

	if ( res ) {
		win_y = adapter->getScreenHeight() - win_y;
		if ( win_x < 0 ) win_x = 0;
		if ( win_x > adapter->getScreenWidth() - 1 ) win_x = adapter->getScreenWidth() - 1;
		if ( win_y < 0 ) win_y = 0;
		if ( win_y > adapter->getScreenHeight() - 1 ) win_y = adapter->getScreenHeight() - 1;
		*screenx = static_cast<Uint16>( win_x );
		*screeny = static_cast<Uint16>( win_y );
	}
}

int Map::getPanningFromMapXY( Uint16 mapx, Uint16 mapy ) {
	Uint16 screenx, screeny;
	getScreenXYAtMapXY( mapx, mapy, &screenx, &screeny );
	float panning = ( static_cast<float>( screenx ) / adapter->getScreenWidth() ) * 255;
	return toint( panning );
}

/*
void Map::showInfoAtMapPos(Uint16 mapx, Uint16 mapy, Uint16 mapz, char *message) {
  float xpos2 = (static_cast<float>(mapx - getX()) * MUL);
  float ypos2 = (static_cast<float>(mapy - getY()) * MUL);
  float zpos2 = static_cast<float>(mapz) * MUL;
  glTranslatef( xpos2, ypos2, zpos2 + 100);

  //glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
  //glRasterPos2f( 0, 0 );
  scourge->getSDLHandler()->texPrint(0, 0, "%s", message);

  glTranslatef( -xpos2, -ypos2, -(zpos2 + 100));
}
*/

/// Positions the "camera".

void Map::setPos( float x, float y, float z ) {
	this->x = ( int )x;
	this->y = ( int )y;
	this->mapx = x;
	this->mapy = y;
	mapChanged = true;
}

/// Initializes the map view (translate, rotate)

void Map::initMapView( bool ignoreRot ) {
	glLoadIdentity();

	glTranslatef( viewX, viewY, 0 );
	glScissor( viewX, adapter->getScreenHeight() - ( viewY + viewHeight ), viewWidth, viewHeight );
	glEnable( GL_SCISSOR_TEST );
	// adjust for screen size
	float adjust = static_cast<float>( viewWidth ) / 800.0f;
	glScalef( adjust, adjust, adjust );

	glScalef( zoom, zoom, zoom );

	// translate the camera and rotate
	// the offsets ensure that the center of rotation is under the player
	glTranslatef( this->xpos, this->ypos, 0 );
	if ( !ignoreRot ) {
		glRotatef( xrot, 0.0f, 1.0f, 0.0f );
		glRotatef( yrot, 1.0f, 0.0f, 0.0f );
		glRotatef( zrot, 0.0f, 0.0f, 1.0f );
	}
	glTranslatef( 0.0f, 0.0f, this->zpos );

	// adjust for centered-map movement
	float xdiff = 0;
	float ydiff = 0;
	if ( settings->isPlayerEnabled() && ( preferences->getAlwaysCenterMap() || mapCenterCreature ) ) {
		RenderedCreature *c = ( mapCenterCreature ? mapCenterCreature : adapter->getPlayer() );
		if ( c ) {
			xdiff = ( c->getX() - static_cast<float>( toint( c->getX() ) ) );
			ydiff = ( c->getY() - static_cast<float>( toint( c->getY() ) ) );
		}
	}
	float startx = -( static_cast<float>( mapViewWidth ) / 2.0 + ( mapx - static_cast<float>( x ) + xdiff ) - 4 ) * MUL;
	float starty = -( static_cast<float>( mapViewDepth ) / 2.0 + ( mapy - static_cast<float>( y ) + ydiff ) - 8 ) * MUL;
	float startz = 0.0;

	glTranslatef( startx, starty, startz );

	if ( quakesEnabled ) {
		quake();
	}
}

void Map::doQuake() {
	Uint32 now = SDL_GetTicks();

	// is it time to quake again?
	if ( now >= nextQuakeStartTime ) {
		nextQuakeStartTime =
		  now +
		  QUAKE_DELAY +
		  Util::dice( QUAKE_DELAY / 2 );
		// start a quake unless this is the very first time
		quakeStartTime = ( quakeStartTime == 0 ? nextQuakeStartTime : now );
		if ( quakeStartTime == now ) adapter->writeLogMessage( _( "A tremor shakes the earth..." ) );
	}

	// is it quaking now?
	if ( now - quakeStartTime < QUAKE_DURATION ) {
		if ( now - lastQuakeTick >= QUAKE_TICK_FREQ ) {
			quakeOffsX = Util::roll( 0.0f, 3.0f * MUL );
			quakeOffsY = Util::roll( 0.0f, 3.0f * MUL );
			lastQuakeTick = now;
		}
	} else {
		if ( quakeOnce ) {
			quakeOnce = false;
			quakesEnabled = false;
		}
		quakeOffsX = quakeOffsY = 0;
	}

	glTranslatef( quakeOffsX, quakeOffsY, 0 );
}

/// Trigger a quake (the screen shivers for a while).

void Map::quake() {
	quakesEnabled = true;
	nextQuakeStartTime = SDL_GetTicks() + 2 * QUAKE_DELAY;
	quakeStartTime = SDL_GetTicks();
	quakeOnce = true;
}

/// Moves a creature (instantly) by 1 tile into the specified direction.

/// if you can't move to this spot (blocked) returns the blocking shape,
/// otherwise returns NULL and moves the shape.

Location *Map::moveCreature( Sint16 x, Sint16 y, Sint16 z, Uint16 dir, RenderedCreature *newCreature ) {
	Sint16 nx = x;
	Sint16 ny = y;
	Sint16 nz = z;
	switch ( dir ) {
	case Constants::MOVE_UP: ny--; break;
	case Constants::MOVE_DOWN: ny++; break;
	case Constants::MOVE_LEFT: nx--; break;
	case Constants::MOVE_RIGHT: nx++; break;
	}
	return moveCreature( x, y, z, nx, ny, nz, newCreature );
}

/// Moves a creature (instantly).

Location *Map::moveCreature( Sint16 x, Sint16 y, Sint16 z, Sint16 nx, Sint16 ny, Sint16 nz, RenderedCreature *newCreature ) {

	// no need to actually move data
	if ( x == nx && y == ny && z == nz ) {
		return NULL;
	}

	//float interX, interY;
	Location *position = isBlocked( nx, ny, nz, x, y, z, newCreature->getShape() );
	if ( position ) return position;

	// move position
	moveCreaturePos( nx, ny, nz, x, y, z, newCreature );

	return NULL;
}

void Map::setRugPosition( Sint16 xchunk, Sint16 ychunk, Rug *rug ) {
	rugPos[ xchunk ][ ychunk ] = *rug;
}

/// Creates a shape on the floor (indoors).

void Map::setFloorPosition( Sint16 x, Sint16 y, Shape *shape ) {
	if ( x < 0 || y < 0 || x >= MAP_WIDTH || y >= MAP_DEPTH ) {
		cerr << "*** floor position out of bounds: " << x << "," << y << endl;
		//((RenderedCreature*)NULL)->getName();
		return;
	}

	floorPositions[x][y] = shape;
	WaterTile *w = new WaterTile;
	for ( int xp = 0; xp < WATER_TILE_X; xp++ ) {
		for ( int yp = 0; yp < WATER_TILE_Y; yp++ ) {
			w->z[xp][yp] = Util::roll( -WATER_AMP, WATER_AMP );
			w->step[xp][yp] = WATER_STEP * ( Util::dice( 2 ) ? 1 : -1 );
			w->lastTime[xp][yp] = 0;
		}
	}
	water[createPairKey( x, y )] = w;
}

void Map::removeRugPosition( Sint16 xchunk, Sint16 ychunk ) {
	rugPos[ xchunk ][ ychunk ].texture.clear();
}

/// Removes items lying on the floor and water at the specified location.

Shape *Map::removeFloorPosition( Sint16 x, Sint16 y ) {
	Shape *shape = NULL;
	if ( floorPositions[x][y] ) {
		shape = floorPositions[x][y];
		floorPositions[x][y] = 0;
	}
	Uint32 key = createPairKey( x, y );
	if ( water.find( key ) != water.end() ) {
		delete water[key];
		water.erase( key );
	}
	return shape;
}

/// Can the specified shape be moved from one x,y,z position to another x,y,z position?

/// Can shape at shapeX, shapeY, shapeZ move to location x, y, z?
/// returns NULL if ok, or the blocking Shape* otherwise.
/// if newz is not null, it will ignore blocking "item"-s and instead stack the new
/// shape on top, returning the new z position in newz.

Location *Map::isBlocked( Sint16 x, Sint16 y, Sint16 z, Sint16 shapeX, Sint16 shapeY, Sint16 shapeZ, Shape *s, int *newz, bool useItemPos ) {
	int zz = z;
	for ( int sx = 0; sx < s->getWidth(); sx++ ) {
		for ( int sy = 0; sy < s->getDepth(); sy++ ) {
			if ( !s->isInside( sx, sy ) ) continue;
			if ( fabs( getGroundHeight( ( x + sx ) / OUTDOORS_STEP, ( y - sy ) / OUTDOORS_STEP ) ) > 10.0f ) {
				return hackBlockingPos;
			}

			// find the lowest location where this item fits
			int sz = z;
			while ( sz < zz + s->getHeight() ) {
				Location *loc = pos[x + sx][y - sy][z + sz];
				if ( loc && loc->shape && !loc->shape->isRoof() &&
				        !( loc->x == shapeX && loc->y == shapeY && loc->z == shapeZ ) ) {
					if ( newz && ( loc->item || loc->creature ) ) {
						int tz = loc->z + loc->shape->getHeight();
						if ( tz > zz ) zz = tz;
						if ( zz + s->getHeight() >= MAP_VIEW_HEIGHT ) {
							return pos[x + sx][y - sy][z + sz];
						}
						if ( zz > sz ) sz = zz;
						else break;
					} else if ( newz && loc ) {
						return pos[x + sx][y - sy][z + sz];
					} else if ( !newz && !( loc && loc->item && !loc->item->isBlocking() ) ) {
						return pos[x + sx][y - sy][z + sz];
					} else {
						sz++;
					}
				} else {
					sz++;
				}
			}
		}
	}

	// check itemPos space: cancel if item cannot be stored here... maybe better to
	// create a list of items at each location? (in itemPos[] only, inter-locking shapes
	// are not supported by pos[]) For now, only 1 item per pos in itemPos[].
	if ( useItemPos && !zz ) {
		for ( int sx = 0; sx < s->getWidth(); sx++ ) {
			for ( int sy = 0; sy < s->getDepth(); sy++ ) {
				if ( !s->isInside( sx, sy ) ) continue;
				Location *loc = itemPos[x + sx][y - sy];
				if ( loc && !( loc->x == shapeX && loc->y == shapeY ) ) {
					return loc;
				}
			}
		}
	}

	if ( newz )
		*newz = zz;
	return NULL;
}

/** This one only returns if the shape originates at xyz. */

Location *Map::getPosition( Sint16 x, Sint16 y, Sint16 z ) {
	if ( pos[x][y][z] && ( ( pos[x][y][z]->shape && pos[x][y][z]->x == x && pos[x][y][z]->y == y && pos[x][y][z]->z == z ) ) )
		return pos[x][y][z];
	return NULL;
}

/// Stops a visual effect.

void Map::stopEffect( Sint16 x, Sint16 y, Sint16 z ) {
	removeEffect( x, y, z );
}

/// Starts a visual effect.

void Map::startEffect( Sint16 x, Sint16 y, Sint16 z, int effect_type, GLuint duration, int width, int height, GLuint delay, bool forever, DisplayInfo *di ) {

	if ( x >= MAP_WIDTH || y >= MAP_DEPTH || z >= MAP_VIEW_HEIGHT ) {
		cerr << "*** STARTEFFECT out of bounds: pos=" << x << "," << y << "," << z << endl;
		( ( RenderedCreature* )NULL )->getName();
	}

	// show an effect
	if ( effect[x][y][z] ) return;

	effect[x][y][z] = mapMemoryManager.newEffectLocation( this, preferences, shapes, width, height );
	/*
	effect[x][y][z]->effect = new Effect( preferences,
	                                      shapes,
	                                      width, height );
	effect[x][y][z]->effect->deleteParticles();
	*/
	effect[x][y][z]->resetDamageEffect();
	effect[x][y][z]->effectType = effect_type;
	effect[x][y][z]->effectDuration = duration;
	effect[x][y][z]->effectDelay = delay;
	effect[x][y][z]->forever = forever;
	effect[x][y][z]->x = x;
	effect[x][y][z]->y = y;
	effect[x][y][z]->z = z;
	effect[x][y][z]->effect->setSize( width, height );
	effect[x][y][z]->heightPos = findMaxHeightPos( x, y, z );
	currentEffectsMap[ createTripletKey( x, y, z ) ] = effect[x][y][z];

	if ( di )
		effect[x][y][z]->effect->setDisplayInfo( di );

	// need to do this to make sure effect shows up
	resortShapes = mapChanged = true;
}

/// Removes the visual effect at x,y,z.

void Map::removeEffect( Sint16 x, Sint16 y, Sint16 z ) {

	if ( x >= MAP_WIDTH || y >= MAP_DEPTH || z >= MAP_VIEW_HEIGHT ) {
		cerr << "*** REMOVEEFFECT out of bounds: pos=" << x << "," << y << "," << z << endl;
		( ( RenderedCreature* )NULL )->getName();
	}

	if ( effect[x][y][z] ) {
		mapMemoryManager.deleteEffectLocation( effect[x][y][z] );
		effect[x][y][z] = NULL;
	}
}

/// Removes all special visual effects (spells, etc.)

void Map::removeAllEffects() {
	for ( int x = 0; x < MAP_WIDTH; x++ ) {
		for ( int y = 0; y < MAP_DEPTH; y++ ) {
			for ( int z = 0; z < MAP_VIEW_HEIGHT; z++ ) {
				if ( effect[x][y][z] ) {
					mapMemoryManager.deleteEffectLocation( effect[x][y][z] );
					effect[x][y][z] = NULL;
				}
			}
		}
	}
	currentEffectsMap.clear();
}

float Map::findMaxHeightPos( float x, float y, float z, bool findMax ) {
	float pos = 0;
	float xp = ( x ) / OUTDOORS_STEP;
	float yp = ( y ) / OUTDOORS_STEP;

	int xx = toint( xp );
	int yy = toint( yp );

	debugHeightPosXX[0] = xx;
	debugHeightPosYY[0] = yy;

	debugHeightPosXX[1] = xx;
	debugHeightPosYY[1] = yy - 1;

	debugHeightPosXX[2] = xx + 1;
	debugHeightPosYY[2] = yy - 1;

	debugHeightPosXX[3] = xx + 1;
	debugHeightPosYY[3] = yy;

	float zz = 0;
	if ( findMax ) {
		// find the max
		for ( int i = 0; i < 4; i++ ) {
			if ( zz < ground[ debugHeightPosXX[ i ] ][ debugHeightPosYY[ i ] ] ) {
				zz = ground[ debugHeightPosXX[ i ] ][ debugHeightPosYY[ i ] ];
			}
		}
	} else {
		// find the average
		int count = 0;
		for ( int i = 0; i < 4; i++ ) {
			// skip 'lake' heights
			if ( ground[ debugHeightPosXX[ i ] ][ debugHeightPosYY[ i ] ] > 0 ) {
				zz += ground[ debugHeightPosXX[ i ] ][ debugHeightPosYY[ i ] ];
				count++;
			}
		}
		if ( zz > 0 && count > 0 ) zz /= static_cast<float>( count );
	}
	if ( z < zz ) pos = zz;
	return pos;
}

/// Returns true if any of the tiles in the specified area is textured.

bool Map::hasOutdoorTexture( int x, int y, int width, int height ) {
	for ( int xx = x; xx < x + width; xx++ ) {
		for ( int yy = y - height - 1; yy <= y; yy++ ) {
			int tx = x / OUTDOORS_STEP;
			int ty = y / OUTDOORS_STEP;
			for ( int z = 0; z < MAX_OUTDOOR_LAYER; z++ ) {
				if ( outdoorTex[tx][ty][z].outdoorThemeRef != -1 ) return true;
			}
		}
	}
	return false;
}

/// Sets the outdoor ground texture for a given map position (detailed version).

void Map::setOutdoorTexture( int x, int y, float offsetX, float offsetY, int ref,
                             float angle, bool horizFlip, bool vertFlip, int z ) {
	int faceCount = getShapes()->getCurrentTheme()->getOutdoorFaceCount( ref );
	if ( faceCount == 0 ) {
		//cerr << "Map Error: no textures for outdoor theme! ref=" << WallTheme::outdoorThemeRefName[ref] << endl;
		return;
	}
	Texture* textureGroup = getShapes()->getCurrentTheme()->getOutdoorTextureGroup( ref );
	int width = getShapes()->getCurrentTheme()->getOutdoorTextureWidth( ref );
	int height = getShapes()->getCurrentTheme()->getOutdoorTextureHeight( ref );

	int tx = x / OUTDOORS_STEP;
	int ty = ( y - height - 1 ) / OUTDOORS_STEP;
	int tw = static_cast<int>( width / OUTDOORS_STEP );
	int th = static_cast<int>( height / OUTDOORS_STEP );
	outdoorTex[tx][ty][z].offsetX = offsetX;
	outdoorTex[tx][ty][z].offsetY = offsetY;
	outdoorTex[tx][ty][z].angle = angle;
	outdoorTex[tx][ty][z].horizFlip = horizFlip;
	outdoorTex[tx][ty][z].vertFlip = vertFlip;
	outdoorTex[tx][ty][z].outdoorThemeRef = ref;

	// computed values
	outdoorTex[tx][ty][z].width = tw;
	outdoorTex[tx][ty][z].height = th;
	outdoorTex[tx][ty][z].texture = textureGroup[ Util::dice( faceCount ) ];
	mapChanged = true;
}

/// Deletes the outdoor ground texture at the specified coordinates.

void Map::removeOutdoorTexture( int x, int y, float width, float height, int z ) {
	int tx = x / OUTDOORS_STEP;
	int ty = ( y - height - 1 ) / OUTDOORS_STEP;
	outdoorTex[tx][ty][z].outdoorThemeRef = -1;
	outdoorTex[tx][ty][z].texture.clear();
	mapChanged = true;
}


void Map::setPositionInner( Sint16 x, Sint16 y, Sint16 z,
                            Shape *shape,
                            RenderedItem *item,
                            RenderedCreature *creature ) {
	if ( x < 0 || y < 0 || z < 0 ||
	        x >= MAP_WIDTH || y >= MAP_DEPTH || z >= MAP_VIEW_HEIGHT ) {
		cerr << "*** Error can't set position outside bounds:" << x << "," << y << "," << z << endl;
		return;
	}

	resortShapes = mapChanged = true;

	bool isNonBlockingItem = ( item && !item->isBlocking() && !z && settings->isItemPosEnabled() );
	Location *p = ( isNonBlockingItem ? itemPos[ x ][ y ] : pos[ x ][ y ][ z ] );
	if ( !p ) {
		p = mapMemoryManager.newLocation();
	}

	p->shape = shape;
	p->item = item;
	p->creature = creature;
	p->heightPos = findMaxHeightPos( x, y, z );
	p->x = x;
	p->y = y;
	p->z = z;
	p->outlineColor = NULL;
	p->texIndex = -1;
	if ( p->shape->getTextureCount() > 3 ) {
		// pick one of the texture groups (3 textures + variants)
		int n = Util::dice( p->shape->getTextureCount() - 2 );
		// -1 means, use the correct default texture (correct for the side of the glShape)
		p->texIndex = ( n == 0 ? -1 : n + 2 );
	}

	for ( int xp = 0; xp < shape->getWidth(); xp++ ) {
		for ( int yp = 0; yp < shape->getDepth(); yp++ ) {
			if ( !shape->isInside( xp, yp ) ) continue;
			for ( int zp = 0; zp < shape->getHeight(); zp++ ) {

				// I _hate_ c++... moving secret doors up causes array roll-over problems.
				if ( x + xp < 0 || y - yp < 0 || z + zp < 0 || x + xp >= MAP_WIDTH || y - yp >= MAP_DEPTH || z + zp >= MAP_VIEW_HEIGHT )
					break;

				// Either the same old pos reused or nothing there.
				// If these are not true, we're leaking memory.
				//assert( pos[x + xp][y - yp][z + zp] == p ||
				//!( pos[x + xp][y - yp][z + zp] ) );

				if ( zp && pos[x + xp][y - yp][z + zp] && pos[x + xp][y - yp][z + zp] != p ) {
					cerr << "error setting position:" << " x=" << ( x + xp ) << " y=" << ( y - yp ) << " z=" << ( z + zp ) <<
					" shape=" << p->shape->getName() << endl;
				} else {
					if ( isNonBlockingItem )
						itemPos[x + xp][y - yp] = p;
					else
						pos[x + xp][y - yp][z + zp] = p;
				}
			}
		}
	}

	// remember gates and teleporters
	if ( shape == shapes->findShapeByName( "GATE_UP" ) ||
	        shape == shapes->findShapeByName( "GATE_DOWN" ) ||
	        shape == shapes->findShapeByName( "GATE_DOWN_OUTDOORS" ) ) {
		gates.insert( p );
	} else if ( shape == shapes->findShapeByName( "TELEPORTER" ) ) {
		teleporters.insert( p );
	}
}

/// Places a shape at a x,y,z position (extended version).

void Map::setPosition( Sint16 x, Sint16 y, Sint16 z, Shape *shape, DisplayInfo *di ) {
	GLShape* gls = dynamic_cast<GLShape*>( shape );
	if ( gls ) {

		if ( gls->hasVirtualShapes() ) {
//   cerr << "Adding virtual shapes for: " << shape->getName() << endl;
			for ( unsigned int n = 0; n < gls->getVirtualShapes()->size(); n++ ) {
				VirtualShape *vs = ( VirtualShape* )( gls->getVirtualShapes()->at( n ) );
//    cerr << "virtual shape: offset=" << vs->getOffsetX() << "," << vs->getOffsetY() << "," << vs->getOffsetZ() << " dim=" <<
//     vs->getWidth() << "," << vs->getDepth() << "," << vs->getHeight() << endl;
				setPositionInner( x + vs->getOffsetX(), y + vs->getOffsetY(), z + vs->getOffsetZ(), vs, NULL, NULL );
			}
		} else {
			setPositionInner( x, y, z, shape, NULL, NULL );

			if ( gls->getEffectType() > -1 ) {

				int ex = x + gls->getEffectX();
				int ey = y - shape->getDepth() - gls->getEffectY();
				int ez = z + gls->getEffectZ();

				if ( !effect[ex][ey][ez] ) {
					startEffect( ex, ey, ez, gls->getEffectType(), 0, gls->getEffectWidth(), gls->getEffectDepth(), 0, true, di );
				}
			}
		}

		// squirrel trimmings
		adapter->shapeAdded( shape->getName(), x, y, z );
	}
}

/// Deletes the shape at the specified map position.

Shape *Map::removePosition( Sint16 x, Sint16 y, Sint16 z ) {
	Shape *shape = NULL;
	if ( pos[x][y][z] && pos[x][y][z]->shape && pos[x][y][z]->x == x && pos[x][y][z]->y == y && pos[x][y][z]->z == z ) {
		resortShapes = mapChanged = true;
		shape = pos[x][y][z]->shape;
		if ( ( ( GLShape* )shape )->getEffectType() > -1 ) {
			int ex = x + ( ( GLShape* )shape )->getEffectX();
			int ey = y - shape->getDepth() - ( ( GLShape* )shape )->getEffectY();
			int ez = z + ( ( GLShape* )shape )->getEffectZ();
			removeEffect( ex, ey, ez );
		}

		Location *p = pos[ x ][ y ][ z ];

		for ( int xp = 0; xp < shape->getWidth(); xp++ ) {
			for ( int yp = 0; yp < shape->getDepth(); yp++ ) {
				if ( !shape->isInside( xp, yp ) ) continue;
				for ( int zp = 0; zp < shape->getHeight(); zp++ ) {

					// I _hate_ c++... moving secret doors up causes array roll-over problems.
					if ( x + xp < 0 || y - yp < 0 || z + zp < 0 || x + xp >= MAP_WIDTH || y - yp >= MAP_DEPTH || z + zp >= MAP_VIEW_HEIGHT )
						break;

					// Assert we're dealing with the right shape
					//assert( pos[ x + xp ][ y - yp ][ z + zp ] == p );
					if ( pos[ x + xp ][ y - yp ][ z + zp ] != p ) {
						cerr << "Error removing position:" << " x=" << ( x + xp ) << " y=" << ( y - yp ) << " z=" << ( z + zp ) << endl;
					} else {
						pos[ x + xp ][ y - yp ][ z + zp ] = NULL;
					}
				}
			}
		}

		// Actually free the shape
		mapMemoryManager.deleteLocation( p );

		// forget gates and teleporters
		if ( shape == shapes->findShapeByName( "GATE_UP" ) ||
		        shape == shapes->findShapeByName( "GATE_DOWN" ) ||
		        shape == shapes->findShapeByName( "GATE_DOWN_OUTDOORS" ) ) {
			gates.erase( p );
		} else if ( shape == shapes->findShapeByName( "TELEPORTER" ) ) {
			teleporters.erase( p );
		}
	}
	return shape;
}

/// Deletes an item at x,y.

Shape *Map::removeItemPosition( Sint16 x, Sint16 y ) {
	Shape *shape = NULL;
	if ( itemPos[x][y] && itemPos[x][y]->shape && itemPos[x][y]->x == x && itemPos[x][y]->y == y ) {
		resortShapes = mapChanged = true;
		shape = itemPos[x][y]->shape;
		if ( ( ( GLShape* )shape )->getEffectType() > -1 ) {
			int ex = x + ( ( GLShape* )shape )->getEffectX();
			int ey = y - shape->getDepth() - ( ( GLShape* )shape )->getEffectY();
			int ez = ( ( GLShape* )shape )->getEffectZ();
			removeEffect( ex, ey, ez );
		}

		Location *p = itemPos[ x ][ y ];

		for ( int xp = 0; xp < shape->getWidth(); xp++ ) {
			for ( int yp = 0; yp < shape->getDepth(); yp++ ) {
				if ( !shape->isInside( xp, yp ) ) continue;
				// I _hate_ c++... moving secret doors up causes array roll-over problems.
				if ( x + xp < 0 || y - yp < 0 || x + xp >= MAP_WIDTH || y - yp >= MAP_DEPTH )
					break;

				// Assert we're dealing with the right shape
				//assert( pos[ x + xp ][ y - yp ][ z + zp ] == p );
				if ( itemPos[ x + xp ][ y - yp ] != p ) {
					cerr << "Error removing item position:" << " x=" << ( x + xp ) << " y=" << ( y - yp ) << endl;
				} else {
					itemPos[ x + xp ][ y - yp ] = NULL;
				}
			}
		}

		// Actually free the shape
		mapMemoryManager.deleteLocation( p );
	}
	return shape;
}

/// Deletes the shape at x,y,z.

Shape *Map::removeLocation( Sint16 x, Sint16 y, Sint16 z ) {
	if ( pos[x][y][z] && pos[x][y][z]->shape )
		return removePosition( pos[x][y][z]->x, pos[x][y][z]->y, pos[x][y][z]->z );
	else return NULL;
}

/// Places an item at x,y,z.

void Map::setItem( Sint16 x, Sint16 y, Sint16 z, RenderedItem *item ) {
	if ( item && item->getShape() ) {
		setPositionInner( x, y, z, item->getShape(), item, NULL );
	}
}

/// Deletes the item at x,y,z.

RenderedItem *Map::removeItem( Sint16 x, Sint16 y, Sint16 z ) {
	RenderedItem *item = NULL;
	if ( !z && itemPos[x][y] && itemPos[x][y]->item ) {
		item = itemPos[x][y]->item;
		removeItemPosition( x, y );
	} else {
		item = ( pos[x][y][z] ? pos[x][y][z]->item : NULL );
		removePosition( x, y, z );
	}
	return item;
}

/// Drops all items above the specified item.

void Map::dropItemsAbove( int x, int y, int z, RenderedItem *item ) {
	int count = 0;
	Location drop[100];
	for ( int tx = 0; tx < item->getShape()->getWidth(); tx++ ) {
		for ( int ty = 0; ty < item->getShape()->getDepth(); ty++ ) {
			if ( !item->getShape()->isInside( tx, ty ) ) continue;
			for ( int tz = z + item->getShape()->getHeight(); tz < MAP_VIEW_HEIGHT; tz++ ) {
				Location *loc2 = pos[x + tx][y - ty][tz];
				if ( loc2 && loc2->item ) {
					drop[count].x = loc2->x;
					drop[count].y = loc2->y;
					drop[count].z = loc2->z - item->getShape()->getHeight();
					drop[count].item = loc2->item;
					count++;
					removeItem( loc2->x, loc2->y, loc2->z );
					tz += drop[count - 1].item->getShape()->getHeight() - 1;
				}
			}
		}
	}
	for ( int i = 0; i < count; i++ ) {
		//cerr << "item " << drop[i].item->getItemName() << " new z=" << drop[i].z << endl;
		setItem( drop[i].x, drop[i].y, drop[i].z, drop[i].item );
	}
}

/// Makes the specified creature appear at x,y,z.

void Map::setCreature( Sint16 x, Sint16 y, Sint16 z, RenderedCreature *creature ) {
	if ( creature && creature->getShape() ) {
		if ( helper && creature->isPartyMember() ) helper->visit( creature );

		// pick up any objects in the way
		for ( int xp = 0; xp < creature->getShape()->getWidth(); xp++ ) {
			for ( int yp = 0; yp < creature->getShape()->getDepth(); yp++ ) {
				if ( !creature->getShape()->isInside( xp, yp ) ) continue;
				for ( int zp = 0; zp < creature->getShape()->getHeight(); zp++ ) {
					if ( pos[x + xp][y - yp][z + zp] && pos[x + xp][y - yp][z + zp]->item ) {
						// creature picks up non-blocking item (this is the only way to handle
						// non-blocking items. It's also very 'roguelike'.
						RenderedItem *item = pos[x + xp][y - yp][z + zp]->item;
						removeItem( pos[x + xp][y - yp][z + zp]->x, pos[x + xp][y - yp][z + zp]->y, pos[x + xp][y - yp][z + zp]->z );
						creature->pickUpOnMap( item );
						char message[120];
						snprintf( message, 120, _( "%s picks up %s." ), creature->getName(), item->getItemName() );
						adapter->writeLogMessage( message );
					}
				}
			}
		}

		setPositionInner( x, y, z, creature->getShape(), NULL, creature );
	}
}

/// Moves a creature instantly to a new position. Picks up any items at the new position.

void Map::moveCreaturePos( Sint16 nx, Sint16 ny, Sint16 nz, Sint16 ox, Sint16 oy, Sint16 oz, RenderedCreature *creature ) {
	Location *p = pos[ox][oy][oz];
	if ( creature && creature->getShape() && p && p->creature && p->x == ox && p->y == oy && p->z == oz ) {

		// remove the old pos
		Location *tmp[MAP_UNIT][MAP_UNIT][MAP_UNIT];
		for ( int xp = 0; xp < creature->getShape()->getWidth(); xp++ ) {
			for ( int yp = 0; yp < creature->getShape()->getDepth(); yp++ ) {
				if ( !creature->getShape()->isInside( xp, yp ) ) continue;
				for ( int zp = 0; zp < creature->getShape()->getHeight(); zp++ ) {
					int oldX = ox + xp;
					int oldY = oy - yp;
					int oldZ = oz + zp;
					tmp[xp][yp][zp] = pos[oldX][oldY][oldZ];
					pos[oldX][oldY][oldZ] = NULL;
					if ( !( tmp[xp][yp][zp] ) )
						cerr << "*** tmp is null!" << endl;
					else
						tmp[xp][yp][zp]->outlineColor = NULL;
				}
			}
		}

		if ( helper && creature->isPartyMember() ) helper->visit( creature );

		// pick up any items in the way
		for ( int xp = 0; xp < creature->getShape()->getWidth(); xp++ ) {
			for ( int yp = 0; yp < creature->getShape()->getDepth(); yp++ ) {
				if ( !creature->getShape()->isInside( xp, yp ) ) continue;
				for ( int zp = 0; zp < creature->getShape()->getHeight(); zp++ ) {
					int newX = nx + xp;
					int newY = ny - yp;
					int newZ = nz + zp;

					if ( pos[newX][newY][newZ] ) {
						if ( pos[newX][newY][newZ]->item ) {
							// creature picks up non-blocking item (this is the only way to handle
							// non-blocking items. It's also very 'roguelike'.)
							RenderedItem *item = pos[newX][newY][newZ]->item;
							removeItem( pos[newX][newY][newZ]->x, pos[newX][newY][newZ]->y, pos[newX][newY][newZ]->z );
							creature->pickUpOnMap( item );
							char message[120];
							snprintf( message, 120, _( "%s picks up %s." ), creature->getName(), item->getItemName() );
							adapter->writeLogMessage( message );
						} else {
							cerr << "*** Error: when moving " << creature->getName() << " path contained a non-item position." << endl;
						}
					}
				}
			}
		}

		// insert the new pos
		for ( int xp = 0; xp < creature->getShape()->getWidth(); xp++ ) {
			for ( int yp = 0; yp < creature->getShape()->getDepth(); yp++ ) {
				if ( !creature->getShape()->isInside( xp, yp ) ) continue;
				for ( int zp = 0; zp < creature->getShape()->getHeight(); zp++ ) {
					int newX = nx + xp;
					int newY = ny - yp;
					int newZ = nz + zp;

					// copy
					pos[newX][newY][newZ] = tmp[xp][yp][zp];
					pos[newX][newY][newZ]->item = NULL;
					pos[newX][newY][newZ]->shape = creature->getShape();
					pos[newX][newY][newZ]->creature = creature;
					pos[newX][newY][newZ]->x = nx;
					pos[newX][newY][newZ]->y = ny;
					pos[newX][newY][newZ]->z = nz;
					pos[newX][newY][newZ]->outlineColor = NULL;
				}
			}
		}

		// instead of repainting the entire map, just update this creature's rendering info
		// resortShapes = mapChanged = true;
		RenderedLocation *later = creatureMap[creature];
		if ( later ) {
			if ( later->pos->creature != creature ) {
				cerr << "*** Error: creatureMap is damaged!!! creature=" << creature->getName() << endl;
				cerr << "\tlocation: shape=" << ( later->pos->shape ? later->pos->shape->getName() : "null" ) <<
				" item=" << ( later->pos->item ? later->pos->item->getItemName() : "null" ) <<
				" creature=" << ( later->pos->creature ? later->pos->creature->getName() : "null" ) <<
				endl;
				//creatureMap.clear();
				resortShapes = mapChanged = true;
			} else {
				//cerr << "*** adjusting rendering info for " << creature->getName() << endl;
				// resort shapes if the player moved
				if ( adapter->getPlayer() == creature ) {
					checkUnderRoof();
					//cerr << "*** player moved" << endl;
					resortShapes = true;
				}

				int chunkOffsetX, chunkOffsetY;
				int chunkStartX, chunkStartY;
				int chunkEndX, chunkEndY;
				calculateChunkInfo( &chunkOffsetX, &chunkOffsetY, &chunkStartX, &chunkStartY, &chunkEndX, &chunkEndY );

				Location *location = pos[nx][ny][nz];
				int posX, posY, posZ;
				bool lightEdge;
				float xpos2, ypos2, zpos2;
				int chunkX, chunkY;
				calculateLocationInfo( location, chunkStartX, chunkStartY, chunkOffsetX, chunkOffsetY, 0,
				                       &posX, &posY, &posZ, &xpos2, &ypos2, &zpos2,
				                       &chunkX, &chunkY, &lightEdge );

				location->heightPos = findMaxHeightPos( location->creature->getX(), location->creature->getY(), location->creature->getZ() );
				later->xpos = xpos2;
				later->ypos = ypos2;
				later->zpos = zpos2;
				//later->pos = location;
				//later->inFront = false;
				later->x = posX;
				later->y = posY;

				// also move the creature's effect
				RenderedLocation *effect = creatureEffectMap[creature];
				if ( effect ) {
					effect->xpos = xpos2;
					effect->ypos = ypos2;
					effect->zpos = zpos2;
					effect->x = posX;
					effect->y = posY;
				}
				
				// also move the creature's light
				RenderedLocation *light = creatureLightMap[creature];
				if( light ) {
					light->xpos = xpos2;
					light->ypos = ypos2;
					light->zpos = zpos2;
					light->x = posX;
					light->y = posY;
				}
			}
		} else if ( helper && helper->isVisible( toint( creature->getX() ), toint( creature->getY() ), creature->getShape() ) ) {
//			cerr << "!!! 1" << creature->getName() << " " << SDL_GetTicks() << " pos=" << 
//				toint( creature->getX() ) << "," << toint( creature->getY() ) << "," << toint( creature->getZ() ) << endl;
			Location *pos = getLocation( (int)( creature->getX() ), (int)( creature->getY() ), (int)( creature->getZ() ) );
			if ( pos ) {
//				cerr << "!!! 2" << creature->getName() << " " << SDL_GetTicks() << endl;
				int chunkX, chunkY;
				getChunk( pos->x, pos->y, &chunkX, &chunkY );
				if ( checkLightMap( chunkX, chunkY ) ) {
					// If this creature has no creatureMap entry but should now be visible,
					// it means the creature wondered into the visible area: repaint everything.
					// An optimization here would be to restrict setupShapes to the creatures that changed.
					resortShapes = mapChanged = true;
//					cerr << "!!! 3" << creature->getName() << " " << SDL_GetTicks() << endl;
				}
			}
		}
	}
}

void Map::getChunk( int mapX, int mapY, int *chunkX, int *chunkY ) {
	*chunkX = ( mapX - MAP_OFFSET ) / MAP_UNIT;
	*chunkY = ( mapY - 1 - MAP_OFFSET ) / MAP_UNIT;
}

/// Sets up map location info.

void Map::calculateLocationInfo( Location *location,
                                 int chunkStartX, int chunkStartY,
                                 int chunkOffsetX, int chunkOffsetY,
                                 Uint16 drawSide,
                                 int *posX, int *posY, int *posZ,
                                 float *xpos, float *ypos, float *zpos,
                                 int *chunkX, int *chunkY,
                                 bool *lightEdge ) {
	*posX = location->x;
	*posY = location->y;
	*posZ = location->z;
	getChunk( location->x, location->y, &( *chunkX ), &( *chunkY ) );
	//*chunkX = ( *posX - MAP_OFFSET ) / MAP_UNIT;
	//*chunkY = ( *posY - 1 - MAP_OFFSET ) / MAP_UNIT;
	int xp = ( *posX - MAP_OFFSET ) % MAP_UNIT;
	int yp = ( *posY - 1 - MAP_OFFSET ) % MAP_UNIT;
	int zp = *posZ;

	// is this shape visible on the edge an chunk in darkness?
	*lightEdge =
	  ( !checkLightMap( *chunkX, *chunkY ) && location->shape && !location->creature &&
	    ( ( drawSide & Constants::MOVE_DOWN && yp >= MAP_UNIT - MAP_UNIT_OFFSET && location->shape->getDepth() <= MAP_UNIT_OFFSET ) ||
	      ( drawSide & Constants::MOVE_UP && yp <= MAP_UNIT_OFFSET && location->shape->getDepth() <= MAP_UNIT_OFFSET ) ||
	      ( drawSide & Constants::MOVE_LEFT && xp < MAP_UNIT_OFFSET && location->shape->getWidth() <= MAP_UNIT_OFFSET ) ||
	      ( drawSide & Constants::MOVE_RIGHT && xp >= MAP_UNIT - MAP_UNIT_OFFSET && location->shape->getWidth() <= MAP_UNIT_OFFSET ) )
	  );

	*xpos = static_cast<float>( ( *chunkX - chunkStartX ) * MAP_UNIT + xp + chunkOffsetX ) * MUL;
	*ypos = static_cast<float>( ( *chunkY - chunkStartY ) * MAP_UNIT + yp - location->shape->getDepth() + chunkOffsetY ) * MUL;
	*zpos = static_cast<float>( zp ) * MUL;
}

/// Sets up chunk info.

void Map::calculateChunkInfo( int *chunkOffsetX, int *chunkOffsetY,
                              int *chunkStartX, int *chunkStartY,
                              int *chunkEndX, int *chunkEndY ) {
	*chunkOffsetX = 0;
	*chunkStartX = ( getX() - MAP_OFFSET ) / MAP_UNIT;
	int mod = ( getX() - MAP_OFFSET ) % MAP_UNIT;
	if ( mod ) {
		*chunkOffsetX = -mod;
	}
	*chunkEndX = mapViewWidth / MAP_UNIT + *chunkStartX;

	*chunkOffsetY = 0;
	*chunkStartY = ( getY() - MAP_OFFSET ) / MAP_UNIT;
	mod = ( getY() - MAP_OFFSET ) % MAP_UNIT;
	if ( mod ) {
		*chunkOffsetY = -mod;
	}
	*chunkEndY = mapViewDepth / MAP_UNIT + *chunkStartY;
}

/// Removes a creature from the map.

RenderedCreature *Map::removeCreature( Sint16 x, Sint16 y, Sint16 z ) {
	RenderedCreature *creature = ( pos[x][y][z] ? pos[x][y][z]->creature : NULL );
	removePosition( x, y, z );
	return creature;
}

bool Map::isWallBetweenLocations( Location *pos1, Location *pos2 ) {
	if( pos1 && pos1->shape && pos2 && pos2->shape ) {
		Shape *shape = isWallBetween( pos1->x + pos1->shape->getWidth() / 2, pos1->y - pos1->shape->getDepth() / 2, pos1->z + pos1->shape->getHeight() / 2, 
			                            pos2->x + pos2->shape->getWidth() / 2, pos2->y - pos2->shape->getDepth() / 2, pos2->z + pos2->shape->getHeight() / 2,
			                            pos1->shape );
		return( shape && !( shape == pos1->shape || shape == pos2->shape ) );
	} else {
		return false;
	}
}

/// Is there a wall between the two given shapes?

// FIXME: only uses x,y for now
// return false if there is any hole in the walls
bool Map::isWallBetweenShapes( int x1, int y1, int z1, Shape *shape1, int x2, int y2, int z2, Shape *shape2 ) {
	for ( int x = x1; x < x1 + shape1->getWidth(); x++ ) {
		for ( int y = y1; y < y1 + shape1->getDepth(); y++ ) {
			for ( int xx = x2; xx < x2 + shape2->getWidth(); xx++ ) {
				for ( int yy = y2; yy < y2 + shape2->getDepth(); yy++ ) {
					Shape *shape = isWallBetween( x, y, z1, xx, yy, z2, shape1 );
					if ( !shape || shape == shape2 )
						return false;
				}
			}
		}
	}
	return true;
}

/// Is there a wall between the two given positions?

// FIXME: only uses x,y for now
Shape *Map::isWallBetween( int x1, int y1, int z1, int x2, int y2, int z2, Shape *ignoreShape ) {

	if ( x1 == x2 && y1 == y2 )
		return isWall( x1, y1, z1 );
	if ( x1 == x2 ) {
		if ( y1 > y2 )
			SWAP( y1, y2 );
		for ( int y = y1; y <= y2; y++ ) {
			Shape *shape = isWall( x1, y, z1 );
			if ( shape )
				return shape;
		}
		return false;
	}
	if ( y1 == y2 ) {
		if ( x1 > x2 ) SWAP( x1, x2 );
		for ( int x = x1; x <= x2; x++ ) {
			Shape *shape = isWall( x, y1, z1 );
			if ( shape )
				return shape;
		}
		return false;
	}


	//  fprintf(stderr, "Checking for wall: from: %d,%d to %d,%d\n", x1, y1, x2, y2);
	Shape *shape = NULL;
	bool yDiffBigger = ( abs( y2 - y1 ) > abs( x2 - x1 ) );
	float m = static_cast<float>( y2 - y1 ) / static_cast<float>( x2 - x1 );
	int steps = ( yDiffBigger ? abs( y2 - y1 ) : abs( x2 - x1 ) );
	float x = x1;
	float y = y1;
	for ( int i = 0; i < steps; i++ ) {
		Shape *ss = isWall( static_cast<int>( x ), static_cast<int>( y ), z1 );
		if ( ss && ss != ignoreShape ) {
			shape = ss;
			break;
		}
		if ( yDiffBigger ) {
			if ( y1 < y2 )
				y += 1.0f;
			else
				y -= 1.0f;

			if ( x1 < x2 )
				x += 1.0f / abs( static_cast<int>( m ) );
			else
				x += -1.0f / abs( static_cast<int>( m ) );
		} else {
			if ( x1 < x2 )
				x += 1.0f;
			else
				x -= 1.0f;

			if ( y1 < y2 )
				y += abs( static_cast<int>( m ) );
			else
				y += -1.0 * abs( static_cast<int>( m ) );
		}
	}
	//  fprintf(stderr, "wall in between? %s\n", (ret ? "TRUE": "FALSE"));
	return shape;
}

/// Is there a wall at the given position?

Shape *Map::isWall( int x, int y, int z ) {
	Location *loc = getLocation( static_cast<int>( x ), static_cast<int>( y ), z );
	return( loc && ( !loc->item || loc->item->getShape() != loc->shape ) &&
	        ( !loc->creature || loc->creature->getShape() != loc->shape ) ? loc->shape : NULL );
}

/// Is there enough room for the shape at the specified position?

bool Map::shapeFits( Shape *shape, int x, int y, int z ) {
	for ( int tx = 0; tx < shape->getWidth(); tx++ ) {
		for ( int ty = 0; ty < shape->getDepth(); ty++ ) {
			if ( !shape->isInside( tx, ty ) ) continue;
			for ( int tz = 0; tz < shape->getHeight(); tz++ ) {
				if ( getLocation( x + tx, y - ty, z + tz ) ) {
					return false;
				}
			}
		}
	}
	return true;
}

/// Checks whether a shape can be placed at x,y,z (outdoors version).

bool Map::shapeFitsOutdoors( GLShape *shape, int x, int y, int z ) {
	bool b = shapeFits( shape, x, y, z ) && !coversDoor( shape, x, y );
	if ( b ) {
		int h = getGroundHeight( x / OUTDOORS_STEP, y / OUTDOORS_STEP );
		b = h >= -3 && h < 3;
		if ( b ) {
			int fx = ( ( x - MAP_OFFSET )  / MAP_UNIT ) * MAP_UNIT + MAP_OFFSET;
			int fy = ( ( y - MAP_OFFSET )  / MAP_UNIT ) * MAP_UNIT + MAP_OFFSET + MAP_UNIT;
			b = getFloorPosition( fx, fy ) == NULL;
		}
	}
	return b;
}

/// Does this shape placed at x,y cover a door so it can't open?

bool Map::coversDoor( Shape *shape, int x, int y ) {
	for ( int ty = y - shape->getDepth() - 6; ty < y + 6; ty++ ) {
		for ( int tx = x - 6; tx < x + shape->getWidth() + 6; tx++ ) {
			if ( isDoor( tx, ty ) ) return true;
		}
	}
	return false;
}

/// Returns whether the shape would be blocked by something else when placed here.

// FIXME: only uses x, y for now
Location *Map::getBlockingLocation( Shape *shape, int x, int y, int z ) {
	for ( int tx = 0; tx < shape->getWidth(); tx++ ) {
		for ( int ty = 0; ty < shape->getDepth(); ty++ ) {
			if ( !shape->isInside( tx, ty ) ) continue;
			Location *loc = getLocation( x + tx, y - ty, 0 );
			if ( loc )
				return loc;
		}
	}
	return NULL;
}

/// If the shape can be dropped at x,y,z, return the location.

Location *Map::getDropLocation( Shape *shape, int x, int y, int z ) {
	for ( int tx = 0; tx < shape->getWidth(); tx++ ) {
		for ( int ty = 0; ty < shape->getDepth(); ty++ ) {
			if ( !shape->isInside( tx, ty ) ) continue;
			Location *loc = getLocation( x + tx, y - ty, z );
			if ( loc ) {
				return loc;
			}
		}
	}
	return NULL;
}

/// Sets up the light map.

/// The light map is used indoors and determines which tiles on the level
/// are currently visible or not (because sight is blocked by a door etc.)

void Map::configureLightMap() {
	lightMapChanged = false;
	groundVisible = false;

	// draw nothing at first
	for ( int x = 0; x < MAP_CHUNKS_X; x++ ) {
		for ( int y = 0; y < MAP_CHUNKS_Y; y++ ) {
			lightMap[x][y] = ( LIGHTMAP_ENABLED && settings->isLightMapEnabled() ? 0 : 1 );
		}
	}
	if ( !( LIGHTMAP_ENABLED && settings->isLightMapEnabled() ) )
		return;

	int chunkX = ( toint( adapter->getPlayer()->getX() ) + ( adapter->getPlayer()->getShape()->getWidth() / 2 ) - MAP_OFFSET ) / MAP_UNIT;
	int chunkY = ( toint( adapter->getPlayer()->getY() ) - ( adapter->getPlayer()->getShape()->getDepth() / 2 ) - MAP_OFFSET ) / MAP_UNIT;

	traceLight( chunkX, chunkY, lightMap, false );
}

/// Can the given position currently be reached?

bool Map::isPositionAccessible( int atX, int atY ) {
	// interpret the results: see if the target is "in light"
	int chunkX = ( atX - MAP_OFFSET ) / MAP_UNIT;
	int chunkY = ( atY - MAP_OFFSET ) / MAP_UNIT;
	return ( accessMap[chunkX][chunkY] != 0 );
}

/// Configures the "access map" (it stores which map tiles are accessible).

void Map::configureAccessMap( int fromX, int fromY ) {
	// create the access map
	for ( int x = 0; x < MAP_CHUNKS_X; x++ ) {
		for ( int y = 0; y < MAP_CHUNKS_Y; y++ ) {
			accessMap[x][y] = 0;
		}
	}
	int chunkX = ( fromX - MAP_OFFSET ) / MAP_UNIT;
	int chunkY = ( fromY - MAP_OFFSET ) / MAP_UNIT;
	traceLight( chunkX, chunkY, accessMap, true );
}

/// Determines (in indoor areas) which parts of the map are currently visible.

void Map::traceLight( int chunkX, int chunkY, int lm[MAP_CHUNKS_X][MAP_CHUNKS_Y], bool onlyLockedDoors ) {
	if ( chunkX < 0 || chunkX >= MAP_CHUNKS_X || chunkY < 0 || chunkY >= MAP_CHUNKS_Y )
		return;

	// already visited?
	if ( lm[chunkX][chunkY] )
		return;

	// let there be light
	lm[chunkX][chunkY] = 1;

	// if there is no roof here, enable the ground
	if ( !getLocation( MAP_OFFSET + chunkX * MAP_UNIT + ( MAP_UNIT / 2 ),
	                   MAP_OFFSET + chunkY * MAP_UNIT + ( MAP_UNIT / 2 ),
	                   MAP_WALL_HEIGHT )  ) {
		groundVisible = true;
	}

	// can we go N?
	int x, y;
	bool blocked = false;
	x = chunkX * MAP_UNIT + MAP_OFFSET + ( MAP_UNIT / 2 );
	for ( y = chunkY * MAP_UNIT + MAP_OFFSET - ( MAP_UNIT / 2 ); y < chunkY * MAP_UNIT + MAP_OFFSET + ( MAP_UNIT / 2 ); y++ ) {
		if ( isLocationBlocked( x, y, 0, onlyLockedDoors ) ) {
			blocked = true;
			break;
		}
	}
	if ( !blocked )
		traceLight( chunkX, chunkY - 1, lm, onlyLockedDoors );

	// can we go E?
	blocked = false;
	y = chunkY * MAP_UNIT + MAP_OFFSET + ( MAP_UNIT / 2 );
	for ( x = chunkX * MAP_UNIT + MAP_OFFSET + ( MAP_UNIT / 2 ); x < chunkX * MAP_UNIT + MAP_OFFSET + ( MAP_UNIT / 2 ) + MAP_UNIT; x++ ) {
		if ( isLocationBlocked( x, y, 0, onlyLockedDoors ) ) {
			blocked = true;
			break;
		}
	}
	if ( !blocked )
		traceLight( chunkX + 1, chunkY, lm, onlyLockedDoors );

	// can we go S?
	blocked = false;
	x = chunkX * MAP_UNIT + MAP_OFFSET + ( MAP_UNIT / 2 );
	for ( y = chunkY * MAP_UNIT + MAP_OFFSET + ( MAP_UNIT / 2 ); y < chunkY * MAP_UNIT + MAP_OFFSET + ( MAP_UNIT / 2 ) + MAP_UNIT; y++ ) {
		if ( isLocationBlocked( x, y, 0, onlyLockedDoors ) ) {
			blocked = true;
			break;
		}
	}
	if ( !blocked )
		traceLight( chunkX, chunkY + 1, lm, onlyLockedDoors );

	// can we go W?
	blocked = false;
	y = chunkY * MAP_UNIT + MAP_OFFSET + ( MAP_UNIT / 2 );
	for ( x = chunkX * MAP_UNIT + MAP_OFFSET - ( MAP_UNIT / 2 ); x < chunkX * MAP_UNIT + MAP_OFFSET + ( MAP_UNIT / 2 ); x++ ) {
		if ( isLocationBlocked( x, y, 0, onlyLockedDoors ) ) {
			blocked = true;
			break;
		}
	}
	if ( !blocked )
		traceLight( chunkX - 1, chunkY, lm, onlyLockedDoors );
}

/// Is the location blocked by something you can't walk through (except creatures)?

bool Map::isLocationBlocked( int x, int y, int z, bool onlyLockedDoors ) {
	if ( x >= 0 && x < MAP_WIDTH && y >= 0 && y < MAP_DEPTH && z >= 0 && z < MAP_VIEW_HEIGHT ) {
		Location *pos = getLocation( x, y, z );

		bool ret = true;
		if ( pos == NULL || pos->item || pos->creature )
			ret = false;
		else if ( onlyLockedDoors && isDoor( x, y ) )
			ret = isLocked( pos->x, pos->y, pos->z );
		else if ( pos && isSecretDoor( pos ) && pos->z > 0 )
			ret = false;
		else if ( !( ( GLShape* )( pos->shape ) )->isLightBlocking() )
			ret = false;
		return ret;
	}
	return true;
}

/// Finds creatures in a specified area and adds them to a targets array.

/// Find the creatures in this area and add them to the targets array.
/// Returns the number of creatures found. (0 if none.)
/// It's the caller responsibility to create the targets array.

int Map::getCreaturesInArea( int x, int y, int radius, RenderedCreature *targets[] ) {
	int count = 0;
	for ( int xx = x - radius; xx < x + radius && xx < MAP_WIDTH; xx++ ) {
		for ( int yy = y - radius; yy < y + radius && yy < MAP_DEPTH; yy++ ) {
			Location *loc = pos[xx][yy][0];
			if ( loc && loc->creature ) {
				bool alreadyFound = false;
				for ( int i = 0; i < count; i++ ) {
					if ( targets[i] == loc->creature ) {
						alreadyFound = true;
						break;
					}
				}
				if ( !alreadyFound ) {
					targets[count++] = loc->creature;
				}
			}
		}
	}
	return count;
}

/// Is there a door at tx,ty?

bool Map::isDoor( int tx, int ty ) {
	if ( tx >= 0 && tx < MAP_WIDTH && ty >= 0 && ty < MAP_DEPTH ) {
		Location *loc = getLocation( tx, ty, 0 );
		return( loc && isDoor( loc->shape ) );
	}
	return false;
}

/// Is this shape a door?

bool Map::isDoor( Shape *shape ) {
	return( shape == shapes->findShapeByName( "EW_DOOR" ) || shape == shapes->findShapeByName( "NS_DOOR" ) );
}

/// Is this shape a door or door frame?

bool Map::isDoorType( Shape *shape, bool includeCorner ) {
	return( shape == shapes->findShapeByName( "EW_DOOR" ) || shape == shapes->findShapeByName( "NS_DOOR" ) ||
	        shape == shapes->findShapeByName( "EW_DOOR_TOP" ) || shape == shapes->findShapeByName( "NS_DOOR_TOP" ) ||
	        shape == shapes->findShapeByName( "DOOR_SIDE" ) || ( includeCorner && shape == shapes->findShapeByName( "CORNER" ) ) );
}

/// Sets the locked state of the door at x,y,z.

void Map::setLocked( int doorX, int doorY, int doorZ, bool value ) {
	locked[createTripletKey( doorX, doorY, doorZ )] = value;
	//Location *p = pos[doorX][doorY][doorZ];
}

int Map::toggleLightMap() {
	LIGHTMAP_ENABLED = ( LIGHTMAP_ENABLED ? 0 : 1 );
	lightMapChanged = true;
	return LIGHTMAP_ENABLED;
}

/// Is the location currently within the screen borders?

bool Map::isLocationVisible( int x, int y ) {
	return ( x >= getX() && x < getX() + mapViewWidth && y >= getY() && y < getY() + mapViewDepth );
}

/// Is the location/shape currently visible on an indoor map?

bool Map::isLocationInLight( int x, int y, Shape *shape ) {
	int chunkX = ( x - MAP_OFFSET ) / MAP_UNIT;
	int chunkY = ( y - ( MAP_OFFSET + 1 ) ) / MAP_UNIT;
	if ( !checkLightMap( chunkX, chunkY ) ) return false;
	return ( helper && helper->isVisible( x, y, shape ) );
}

/// Map view move handler.

/// Move and rotate map.
/// Modifiers:
/// -CTRL + arrow keys / mouse at edge of screen: rotate map
/// -arrow keys / mouse at edge of screen: move map fast
/// -SHIFT + arrow keys / mouse at edge of screen: slow move map
/// -SHIFT + CTRL + arrow keys / mouse at edge of screen: slow rotate map

void Map::moveMap( int dir ) {
	if ( SDL_GetModState() & KMOD_CTRL ) {
		float rot;
		if ( SDL_GetModState() & KMOD_SHIFT ) {
			rot = 1.5f;
		} else {
			rot = 5.0f;
		}
		if ( dir & Constants::MOVE_DOWN ) setYRot( -1.0f * rot );
		if ( dir & Constants::MOVE_UP ) setYRot( rot );
		if ( dir & Constants::MOVE_RIGHT ) setZRot( -1.0f * rot );
		if ( dir & Constants::MOVE_LEFT ) setZRot( rot );
	} else if ( !preferences->getAlwaysCenterMap() ) {

		// stop rotating (angle of rotation is kept)
		setYRot( 0 );
		setZRot( 0 );

		mapChanged = resortShapes = true;
		float delta = ( SDL_GetModState() & KMOD_SHIFT ? 0.5f : 1.0f );
		if ( mouseMove ) {

			// normalize z rot to 0-359
			float z = moveAngle - ( getZRot() - 90 );
			if ( z < 0 ) z += 360;
			if ( z >= 360 ) z -= 360;
			//float zrad = Constants::toRadians(z);

			mapx += -moveDelta / 5.0f * Constants::sinFromAngle( z );
			mapy += moveDelta / 5.0f * Constants::cosFromAngle( z );
			moveDelta = 0; // reset to only move when the mouse is moved
		} else {

			// normalize z rot to 0-359
			float z = getZRot();
			if ( z < 0 ) z += 360;
			if ( z >= 360 ) z -= 360;
			//float zrad = Constants::toRadians(z);

			// cerr << "-------------------" << endl;
			// cerr << "x=" << x << " y=" << y << " zrot=" << z << endl;

			if ( dir & Constants::MOVE_DOWN ) {
				mapx += delta * Constants::sinFromAngle( z );
				mapy += delta * Constants::cosFromAngle( z );
			}
			if ( dir & Constants::MOVE_UP ) {
				mapx += delta * -Constants::sinFromAngle( z );
				mapy += delta * -Constants::cosFromAngle( z );
			}
			if ( dir & Constants::MOVE_LEFT ) {
				mapx += delta * -Constants::cosFromAngle( z );
				mapy += delta * Constants::sinFromAngle( z );
			}
			if ( dir & Constants::MOVE_RIGHT ) {
				mapx += delta * Constants::cosFromAngle( z );
				mapy += delta * -Constants::sinFromAngle( z );
			}
		}

		// cerr << "xdelta=" << xdelta << " ydelta=" << ydelta << endl;

		if ( mapy > MAP_DEPTH - mapViewDepth ) mapy = MAP_DEPTH - mapViewDepth;
		if ( mapy < 0 ) mapy = 0;
		if ( mapx > MAP_WIDTH - mapViewWidth ) mapx = MAP_WIDTH - mapViewWidth;
		if ( mapx < 0 ) mapx = 0;
		// cerr << "mapx=" << mapx << " mapy=" << mapy << endl;

		x = static_cast<int>( rint( mapx ) );
		y = static_cast<int>( rint( mapy ) );
		// cerr << "FINAL: x=" << x << " y=" << y << endl;

	}
}

void Map::handleEvent( SDL_Event *event ) {

	// turn off outlining
	if ( lastOutlinedX < MAP_WIDTH ) {
		Location *pos = getLocation( lastOutlinedX, lastOutlinedY, lastOutlinedZ );
		if ( !lastOutlinedZ && !pos ) pos = getItemLocation( lastOutlinedX, lastOutlinedY );
		if ( pos ) {
			pos->outlineColor = NULL;
			lastOutlinedX = lastOutlinedY = lastOutlinedZ = MAP_WIDTH;
		}
	}

	int ea;
	int mx, my;
	switch ( event->type ) {
	case SDL_MOUSEMOTION:
		if ( mouseRot ) {
			adapter->setCursorVisible( false );
			setZRot( mouseRotDir * event->motion.xrel * MOUSE_ROT_DELTA );
			setYRot( -event->motion.yrel * MOUSE_ROT_DELTA );
		} else if ( mouseMove ) {
			adapter->setCursorVisible( false );
			int q;
			moveDelta = sqrt( static_cast<float>( event->motion.xrel * event->motion.xrel ) +
			                  static_cast<float>( event->motion.yrel * event->motion.yrel ) ) / zoom;
			Constants::getQuadrantAndAngle( event->motion.xrel, event->motion.yrel, &q, &moveAngle );
			mouseMoveScreen = true;
			setMove( Constants::MOVE_UP ); // so move != 0
		} else {
			//sdlHandler->applyMouseOffset(event->motion.x, event->motion.y, &mx, &my);
			mx = event->motion.x;
			my = event->motion.y;
			if ( mx < 10 ) {
				mouseMoveScreen = true;
				setMove( Constants::MOVE_LEFT );
			} else if ( mx >= adapter->getScreenWidth() - 10 ) {
				mouseMoveScreen = true;
				setMove( Constants::MOVE_RIGHT );
			} else if ( my < 10 ) {
				mouseMoveScreen = true;
				setMove( Constants::MOVE_UP );
			} else if ( my >= adapter->getScreenHeight() - 10 ) {
				mouseMoveScreen = true;
				setMove( Constants::MOVE_DOWN );
			} else {
				if ( mouseMoveScreen ) {
					mouseMoveScreen = false;
					removeMove( Constants::MOVE_LEFT | Constants::MOVE_RIGHT );
					removeMove( Constants::MOVE_UP | Constants::MOVE_DOWN );
					setYRot( 0.0f );
					setZRot( 0.0f );
				}
			}

			// highlight the item under the mouse if it's useable
			//cerr << "pos=" << getCursorMapX() << "," << getCursorMapY() << endl;
			if ( getCursorMapX() < MAP_WIDTH ) {
				Location *pos = getLocation( getCursorMapX(), getCursorMapY(), getCursorMapZ() );
				if ( !pos ) pos = getItemLocation( getCursorMapX(), getCursorMapY() );
				if ( pos && preferences->isOutlineInteractiveItems() ) {
					Color *color = adapter->getOutlineColor( pos );
					if ( color ) {
						pos->outlineColor = color;
						lastOutlinedX = pos->x;
						lastOutlinedY = pos->y;
						lastOutlinedZ = pos->z;
					}
				}
			}

			if ( getCursorFlatMapX() < MAP_WIDTH ) {
				// highlight traps too
				selectedTrapIndex = getTrapAtLoc( getCursorFlatMapX(), getCursorFlatMapY() + 2 );
			}
		}
		break;
	case SDL_MOUSEBUTTONDOWN:
		if ( event->button.button ) {
			if ( event->button.button == SDL_BUTTON_MIDDLE ) {
				mouseRot = true;
				mouseRotDir = ( event->button.y - viewY < viewHeight / 2 ? 1 : -1 );
			} else if ( event->button.button == SDL_BUTTON_RIGHT ) {
				mouseMove = true;
				mouseMoveX = event->button.x;
				mouseMoveY = event->button.y;
			}
			if ( event->button.button == SDL_BUTTON_WHEELUP ) {
				mouseZoom = true;
				setZoomIn( false );
				setZoomOut( true );
			}
			if ( event->button.button == SDL_BUTTON_WHEELDOWN ) {
				mouseZoom = true;
				setZoomIn( true );
				setZoomOut( false );
			}
		}
		break;
	case SDL_MOUSEBUTTONUP:
		adapter->setCursorVisible( true );
		if ( event->button.button ) {
			if ( event->button.button == SDL_BUTTON_MIDDLE || event->button.button == SDL_BUTTON_RIGHT ) {
				mouseRot = false;
				mouseMove = false;
				moveDelta = 0;
				setXRot( 0 );
				setYRot( 0 );
				setZRot( 0 );
				move = 0;
			}
		}
		break;
	case SDL_KEYDOWN:
	case SDL_KEYUP:
		// xxx_yyy_stop means : "do xxx_yyy action when the corresponding key is up"
		ea = preferences->getEngineAction( event );
		if ( ea == SET_MOVE_DOWN ) {
			setMove( Constants::MOVE_DOWN );
		} else if ( ea == SET_MOVE_UP ) {
			setMove( Constants::MOVE_UP );
		} else if ( ea == SET_MOVE_RIGHT ) {
			setMove( Constants::MOVE_RIGHT );
		} else if ( ea == SET_MOVE_LEFT ) {
			setMove( Constants::MOVE_LEFT );
		} else if ( ea == SET_MOVE_DOWN_STOP ) {
			setYRot( 0.0f );
			setYRot( 0 );
			removeMove( Constants::MOVE_DOWN );
		} else if ( ea == SET_MOVE_UP_STOP ) {
			setYRot( 0.0f );
			setYRot( 0 );
			removeMove( Constants::MOVE_UP );
		} else if ( ea == SET_MOVE_RIGHT_STOP ) {
			setYRot( 0.0f );
			setZRot( 0 );
			removeMove( Constants::MOVE_RIGHT );
		} else if ( ea == SET_MOVE_LEFT_STOP ) {
			setYRot( 0.0f );
			setZRot( 0 );
			removeMove( Constants::MOVE_LEFT );
		} else if ( ea == SET_ZOOM_IN ) {
			setZoomIn( true );
		} else if ( ea == SET_ZOOM_OUT ) {
			setZoomOut( true );
		} else if ( ea == SET_ZOOM_IN_STOP ) {
			setZoomIn( false );
		} else if ( ea == SET_ZOOM_OUT_STOP ) {
			setZoomOut( false );
		}
		break;
	default: break;
	}
}

#define NEG_GROUND_HEIGHT 0x00100000
void Map::saveMap( const string& name, string& result, bool absolutePath, int referenceType ) {

	if ( name.length() == 0 ) {
		result = _( "You need to name the map first." );
		return;
	}

	MapInfo *info = new MapInfo;
	info->version = PERSIST_VERSION;

	info->reference_type = referenceType;

	info->start_x = startx;
	info->start_y = starty;

	info->grid_x = mapGridX;
	info->grid_y = mapGridY;

	info->hasWater = ( hasWater ? 1 : 0 );

	info->map_type = ( helper == MapRenderHelper::helpers[ MapRenderHelper::CAVE_HELPER ] ?
	                   MapRenderHelper::CAVE_HELPER :
	                   ( helper == MapRenderHelper::helpers[ MapRenderHelper::ROOM_HELPER ] ?
	                     MapRenderHelper::ROOM_HELPER :
	                     MapRenderHelper::OUTDOOR_HELPER ) );

	strncpy( ( char* )info->theme_name, shapes->getCurrentThemeName(), 254 );
	info->theme_name[254] = 0;

	info->pos_count = 0;
	for ( int x = 0; x < MAP_WIDTH; x++ ) {
		for ( int y = 0; y < MAP_DEPTH; y++ ) {

			if ( floorPositions[x][y] ) {
				info->pos[ info->pos_count ] = Persist::createLocationInfo( x, y, 0 );
				strncpy( ( char* )info->pos[ info->pos_count ]->floor_shape_name, floorPositions[x][y]->getName(), 254 );
				info->pos[ info->pos_count ]->floor_shape_name[254] = 0;
				info->pos_count++;
			}

			if ( itemPos[x][y] && itemPos[x][y]->x == x && itemPos[x][y]->y == y && itemPos[x][y]->item ) {
				info->pos[ info->pos_count ] = Persist::createLocationInfo( x, y, 0 );
				if ( referenceType == REF_TYPE_NAME ) {
					strncpy( ( char* )info->pos[ info->pos_count ]->item_pos_name, itemPos[x][y]->item->getType(), 254 );
					info->pos[ info->pos_count ]->item_pos_name[254] = 0;
				} else {
					info->pos[ info->pos_count ]->item_pos = itemPos[x][y]->item->save();
				}
				info->pos_count++;
			}

			// for virtual shapes, only save the reference shape pointed to by the virutal shape that is actually drawn
			for ( int z = 0; z < MAP_VIEW_HEIGHT; z++ ) {
				if ( pos[x][y][z] && 
						pos[x][y][z]->x == x && pos[x][y][z]->y == y && pos[x][y][z]->z == z && 
						!( pos[x][y][z]->shape && pos[x][y][z]->shape->isVirtual() && !( ( VirtualShape* )pos[x][y][z]->shape )->isDrawn() ) &&
						!( pos[x][y][z]->creature && pos[x][y][z]->creature->isPartyMember() ) ) {

					info->pos[ info->pos_count ] = Persist::createLocationInfo( x, y, z );

					if ( pos[x][y][z]->item ) {
						if ( referenceType == REF_TYPE_NAME ) {
							strncpy( ( char* )info->pos[ info->pos_count ]->item_name, pos[x][y][z]->item->getType(), 254 );
						} else {
							info->pos[ info->pos_count ]->item = pos[x][y][z]->item->save();
						}
					} else if ( pos[x][y][z]->creature ) {
						if ( referenceType == REF_TYPE_NAME ) {
							strncpy( ( char* )info->pos[ info->pos_count ]->monster_name, pos[x][y][z]->creature->getType(), 254 );
							info->pos[ info->pos_count ]->monster_name[254] = 0;
						} else {
							info->pos[ info->pos_count ]->creature = pos[x][y][z]->creature->save();
						}
					} else {
						if ( pos[x][y][z]->shape->isVirtual() ) {
							VirtualShape *vs = ( VirtualShape* )pos[x][y][z]->shape;
							strncpy( ( char* )( info->pos[ info->pos_count ]->shape_name ), vs->getRef()->getName(), 254 );
							info->pos[ info->pos_count ]->x -= vs->getOffsetX();
							info->pos[ info->pos_count ]->y -= vs->getOffsetY();
							info->pos[ info->pos_count ]->z -= vs->getOffsetZ();
						} else {
							strncpy( ( char* )( info->pos[ info->pos_count ]->shape_name ), pos[x][y][z]->shape->getName(), 254 );
						}
						info->pos[ info->pos_count ]->shape_name[254] = 0;
					}

					// save the deity locations
					char const* p = adapter->getMagicSchoolIndexForLocation( pos[x][y][z] );
					if ( p ) {
						strncpy( ( char* )info->pos[ info->pos_count ]->magic_school_name, p, 254 );
						info->pos[ info->pos_count ]->magic_school_name[254] = 0;
					}

					info->pos_count++;
				}
			}
		}
	}

	// save rugs
	info->rug_count = 0;
	for ( int x = 0; x < MAP_CHUNKS_X; x++ ) {
		for ( int y = 0; y < MAP_CHUNKS_Y; y++ ) {
			if ( rugPos[x][y].texture.isSpecified() ) {
				info->rugPos[ info->rug_count ] = Persist::createRugInfo( x, y );
				info->rugPos[ info->rug_count ]->angle = toint( rugPos[x][y].angle * 100.0f );
				info->rugPos[ info->rug_count ]->isHorizontal = ( rugPos[x][y].isHorizontal ? 1 : 0 );
				info->rugPos[ info->rug_count ]->texture = 666; // -=K=-: no point to store any GPU texture names? rugPos[x][y].texture;
				info->rug_count++;
			}
		}
	}

	// save doors
	info->locked_count = 0;
	for ( map<Uint32, bool>::iterator i = locked.begin(); i != locked.end(); ++i ) {
		Uint32 key = i->first;
		Uint8 value = ( Uint8 )( i->second ? 1 : 0 );
		info->locked[ info->locked_count++ ] = Persist::createLockedInfo( key, value );
	}
	info->door_count = 0;
	for ( map<Uint32, Uint32>::iterator i = doorToKey.begin(); i != doorToKey.end(); ++i ) {
		Uint32 key = i->first;
		Uint32 value = i->second;
		info->door[ info->door_count++ ] = Persist::createDoorInfo( key, value );
	}

	// secret doors
	info->secret_count = 0;
	for ( map<int, bool>::iterator i = secretDoors.begin(); i != secretDoors.end(); ++i ) {
		Uint32 key = ( Uint32 )( i->first );
		Uint8 value = ( Uint8 )( i->second ? 1 : 0 );
		info->secret[ info->secret_count++ ] = Persist::createLockedInfo( key, value );
	}

	// save the fog
	if ( helper ) helper->saveHelper( &( info->fog_info ) );

	info->edited = edited;

	// save ground height for outdoors maps
	// save the outdoor textures
	info->heightMapEnabled = ( Uint8 )( heightMapEnabled ? 1 : 0 );
	info->outdoorTextureInfoCount = 0;
	for ( int gx = 0; gx < MAP_TILES_X; gx++ ) {
		for ( int gy = 0; gy < MAP_TILES_Y; gy++ ) {
			Uint32 base = ( ground[ gx ][ gy ] < 0 ? NEG_GROUND_HEIGHT : 0x00000000 );
			info->ground[ gx ][ gy ] = ( Uint32 )( fabs( ground[ gx ][ gy ] ) * 100 ) + base;
			for ( int z = 0; z < MAX_OUTDOOR_LAYER; z++ ) {
				if ( outdoorTex[ gx ][ gy ][ z ].texture.isSpecified() ) {
					OutdoorTextureInfo *oti = new OutdoorTextureInfo;
					int height = getShapes()->getCurrentTheme()->getOutdoorTextureHeight( outdoorTex[ gx ][ gy ][z].outdoorThemeRef );
					int mx = gx * OUTDOORS_STEP;
					int my = gy * OUTDOORS_STEP + height + 1;
					oti->x = mx;
					oti->y = my;
					oti->angle = outdoorTex[ gx ][ gy ][z].angle * 1000;
					oti->horizFlip = outdoorTex[ gx ][ gy ][z].horizFlip;
					oti->vertFlip = outdoorTex[ gx ][ gy ][z].vertFlip;
					oti->offsetX = outdoorTex[ gx ][ gy ][z].offsetX * 1000;
					oti->offsetY = outdoorTex[ gx ][ gy ][z].offsetY * 1000;
					oti->outdoorThemeRef = outdoorTex[ gx ][ gy ][z].outdoorThemeRef;
					oti->z = z;
					info->outdoorTexture[ info->outdoorTextureInfoCount++ ] = oti;
				}
			}
		}
	}

	// save traps
	info->trapCount = ( Uint8 )trapList.size();
	for ( unsigned int i = 0; i < trapList.size(); i++ ) {
		Trap trap = trapList[ i ];
		info->trap[i] = Persist::createTrapInfo( trap.r.x, trap.r.y, trap.r.w, trap.r.h, trap.type, trap.discovered, trap.enabled );
	}

	string fileName;
	if ( absolutePath ) {
		fileName = name;
	} else {
		fileName = rootDir + ( "/maps/" + name + ".map" );
	}

	cerr << "saving map: " << fileName << endl;

	FILE *fp = fopen( fileName.c_str(), "wb" );
	File *file = new ZipFile( fp, ZipFile::ZIP_WRITE );
	Persist::saveMap( file, info );

	delete file;

	Persist::deleteMapInfo( info );

	// save npc info when saving from map editor (ie. map templates)
	if ( referenceType == REF_TYPE_NAME ) {
		adapter->saveMapData( fileName );
	}

	result += "Map saved: ";
	result += name;
}

/// Initializes the textures for cave levels.

void Map::initForCave( char *themeName ) {
	if ( !themeName ) {
		shapes->loadRandomCaveTheme();
	} else {
		shapes->loadCaveTheme( themeName );
	}

	string ref = WallTheme::themeRefName[ WallTheme::THEME_REF_PASSAGE_FLOOR ];
	Texture* floorTextureGroup = shapes->getCurrentTheme()->getTextureGroup( ref );
	setFloor( CAVE_CHUNK_SIZE, CAVE_CHUNK_SIZE, floorTextureGroup[ GLShape::TOP_SIDE ] );
}

bool Map::loadMap( const string& name, std::string& result, StatusReport *report,
                   int level, int depth,
                   bool changingStory, bool fromRandom,
                   bool goingUp, bool goingDown,
                   vector< RenderedItem* > *items,
                   vector< RenderedCreature* > *creatures,
                   bool absolutePath,
                   char *templateMapName ) {
	if ( !name.length() ) {
		result = _( "Enter a name of a map to load." );
		return false;
	}

	stringstream tmpFileName;
	if ( absolutePath ) {
		tmpFileName << name;
	} else {
		if ( depth > 0 ) {
			tmpFileName << rootDir << "/maps/" << name << depth << ".map";
		} else {
			tmpFileName << rootDir << "/maps/" << name << ".map";
		}
	}
	string fileName = tmpFileName.str();
	cerr << "Looking for map: " << fileName << endl;

	FILE *fp = fopen( fileName.c_str(), "rb" );
	if ( !fp ) {
		result = "Can't find map: " + fileName;
		return false;
	}
	if ( report ) report->updateStatus( 0, 7, _( "Loading map" ) );
	File *file = new ZipFile( fp, ZipFile::ZIP_READ );
	MapInfo *info = Persist::loadMap( file );
	delete file;

	if ( report ) report->updateStatus( 1, 7, _( "Loading theme" ) );

	// reset the map
	reset();

	if ( info->map_type == MapRenderHelper::ROOM_HELPER ) {
		// it's a room-type map
		setMapRenderHelper( MapRenderHelper::helpers[ MapRenderHelper::ROOM_HELPER ] );
		// load the theme
		if ( getPreferences()->isDebugTheme() ) shapes->loadDebugTheme();
		else shapes->loadTheme( ( const char* )info->theme_name );
	} else if ( info->map_type == MapRenderHelper::CAVE_HELPER ) {
		// it's a room-type map
		setMapRenderHelper( MapRenderHelper::helpers[ MapRenderHelper::CAVE_HELPER ] );

		// prepare map for cave (load theme, etc.)
		initForCave( ( char* )info->theme_name );

		// load the fog
		getMapRenderHelper()->loadHelper( &( info->fog_info ) );
	} else if ( info->map_type == MapRenderHelper::OUTDOOR_HELPER ) {
		// it's an outdoors type map
		setMapRenderHelper( MapRenderHelper::helpers[ MapRenderHelper::OUTDOOR_HELPER ] );
		if ( getPreferences()->isDebugTheme() ) shapes->loadDebugTheme();
		else shapes->loadTheme( ( const char* )info->theme_name );
	} else {
		cerr << "*** error unknown map type: " << info->map_type << endl;
		return false;
	}

	edited = info->edited != 0;

	// load ground heights for outdoors maps
	heightMapEnabled = ( info->heightMapEnabled == 1 );
	for ( int gx = 0; gx < MAP_TILES_X; gx++ ) {
		for ( int gy = 0; gy < MAP_TILES_Y; gy++ ) {
			if ( info->ground[ gx ][ gy ] > NEG_GROUND_HEIGHT ) {
				ground[ gx ][ gy ] = ( info->ground[ gx ][ gy ] - NEG_GROUND_HEIGHT ) / -100.0f;
			} else {
				ground[ gx ][ gy ] = info->ground[ gx ][ gy ] / 100.0f;
			}
		}
	}

	for ( int i = 0; i < ( int )info->outdoorTextureInfoCount; i++ ) {
		OutdoorTextureInfo *oti = info->outdoorTexture[i];
		setOutdoorTexture( oti->x, oti->y,
		                   oti->offsetX / 1000.0f, oti->offsetY / 1000.0f,
		                   oti->outdoorThemeRef, oti->angle / 1000.0f,
		                   oti->horizFlip != 0, oti->vertFlip != 0, oti->z );
	}

	if ( heightMapEnabled ) {
		outdoor->initOutdoorsGroundTexture();
	}

	setHasWater( info->hasWater == 1 ? true : false );

	if ( report ) report->updateStatus( 2, 7, _( "Starting map" ) );

	/*
	// Start at the saved start pos. or where the party
	// was on the last level if changing stories.
	// When using an absolutePath (ie. saved generated maps) also start at the last
	// saved position.
	if( absolutePath || !changingStory || !( settings->isPlayerEnabled() ) || fromRandom ) {
	  startx = info->start_x;
	  starty = info->start_y;
	} else {
	  startx = toint( adapter->getPlayer()->getX() );
	  starty = toint( adapter->getPlayer()->getY() );
	}
	*/
	cerr << "LOADMAP: using saved starting position. goingUp=" << goingUp << " goingDown=" << goingDown << endl;
	startx = info->start_x;
	starty = info->start_y;
	float start_dist = -1;

	mapGridX = info->grid_x;
	mapGridY = info->grid_y;

	if ( report ) report->updateStatus( 3, 7, _( "Initializing map" ) );

	GLShape *shape;
	DisplayInfo di;
	for ( int i = 0; i < static_cast<int>( info->pos_count ); i++ ) {

		if ( info->pos[i]->x >= MAP_WIDTH || info->pos[i]->y >= MAP_DEPTH || info->pos[i]->z >= MAP_VIEW_HEIGHT ) {
			cerr << "*** Skipping invalid map location: " << info->pos[i]->x << "," << info->pos[i]->y << "," << info->pos[i]->z << endl;
			continue;
		}

		if ( strlen( ( char* )( info->pos[i]->floor_shape_name ) ) ) {
			shape = shapes->findShapeByName( ( char* )( info->pos[i]->floor_shape_name ) );
			if ( shape )
				setFloorPosition( info->pos[i]->x, info->pos[i]->y, shape );
			else
				cerr << "Map::load failed to find floor shape: " << info->pos[i]->floor_shape_name <<
				" at pos: " << info->pos[i]->x << "," << info->pos[i]->y << endl;
		}

		if ( strlen( ( char* )( info->pos[i]->item_pos_name ) ) || info->pos[i]->item_pos ) {
			RenderedItem *item;
			if ( info->reference_type == REF_TYPE_NAME ) {
				item = adapter->createItem( ( char* )( info->pos[i]->item_pos_name ), level, depth );
			} else {
				item = adapter->createItem( info->pos[i]->item_pos );
			}
			if ( item ) {
				setItem( info->pos[i]->x, info->pos[i]->y, 0, item );
				if ( items )
					items->push_back( item );
			} else
				cerr << "Map::load failed to item at pos: " << info->pos[i]->x << "," << info->pos[i]->y << ",0" << endl;
		}

		// load the deity locations
		MagicSchool *ms = NULL;
		if ( strlen( ( char* )info->pos[i]->magic_school_name ) ) {
			ms = MagicSchool::getMagicSchoolByName( ( char* )info->pos[i]->magic_school_name );
			di.red = ms->getDeityRed();
			di.green = ms->getDeityGreen();
			di.blue = ms->getDeityBlue();
		}

		// load the items
		if ( strlen( ( char* )( info->pos[i]->item_name ) ) || info->pos[i]->item ) {
			RenderedItem *item;
			if ( info->reference_type == REF_TYPE_NAME ) {
				item = adapter->createItem( ( char* )( info->pos[i]->item_name ), level, depth );
			} else {
				item = adapter->createItem( info->pos[i]->item );
			}
			if ( item ) {
				setItem( info->pos[i]->x, info->pos[i]->y, info->pos[i]->z, item );
				if ( items ) items->push_back(  item );
			} else cerr << "Map::load failed to item at pos: " << info->pos[i]->x << "," << info->pos[i]->y << "," << info->pos[i]->z << endl;
		} else if ( strlen( ( char* )( info->pos[i]->monster_name ) ) || info->pos[i]->creature ) {
			RenderedCreature *creature;
			if ( info->reference_type == REF_TYPE_NAME ) {
				creature = adapter->createMonster( ( char* )( info->pos[i]->monster_name ) );
			} else {
				creature = adapter->createMonster( info->pos[i]->creature );
			}
			if ( creature ) {
				setCreature( info->pos[i]->x, info->pos[i]->y, info->pos[i]->z, creature );
				creature->moveTo( info->pos[i]->x, info->pos[i]->y, info->pos[i]->z );
				if ( creatures ) creatures->push_back(  creature );
			} else cerr << "Map::load failed to creature at pos: " << info->pos[i]->x << "," << info->pos[i]->y << "," << info->pos[i]->z << endl;
		} else if ( strlen( ( char* )( info->pos[i]->shape_name ) ) ) {
			shape = shapes->findShapeByName( ( char* )( info->pos[i]->shape_name ) );
			if ( shape ) {
				setPosition( info->pos[i]->x, info->pos[i]->y, info->pos[i]->z, shape, ( ms ? &di : NULL ) );
				if ( settings->isPlayerEnabled() ) {
					if ( ( goingUp && !strcmp( ( char* )info->pos[i]->shape_name, "GATE_DOWN" ) ) ||
					        ( goingDown && !strcmp( ( char* )info->pos[i]->shape_name, "GATE_UP" ) ) ||
					        ( goingUp && !strcmp( ( char* )info->pos[i]->shape_name, "GATE_DOWN_OUTDOORS" ) ) ) {
						float d = ( info->pos[i]->x - adapter->getPlayer()->getX() ) * ( info->pos[i]->x - adapter->getPlayer()->getX() ) +
						          ( info->pos[i]->y - adapter->getPlayer()->getY() ) * ( info->pos[i]->y - adapter->getPlayer()->getY() );
						if ( start_dist == -1 || d < start_dist ) {
							startx = info->pos[i]->x;
							starty = info->pos[i]->y;
							start_dist = d;
							cerr << "LOADMAP: using stairs as starting position. Dist=" << start_dist << endl;
						}
					}
				}
			} else
				cerr << "Map::load failed to find shape: " << info->pos[i]->shape_name <<
				" at pos: " << info->pos[i]->x << "," << info->pos[i]->y << "," << info->pos[i]->z << endl;
		}

		// load the deity locations
		if ( ms ) {
			Location *pos = getPosition( info->pos[i]->x, info->pos[i]->y, info->pos[i]->z );
			if ( !pos ) {
				cerr << "*** error: Can't find position to place deity!" << endl;
			} else {
				adapter->setMagicSchoolIndexForLocation( pos, ms->getName() );
			}
		}
	}

	// load rugs
	Rug rug;
	for ( int i = 0; i < static_cast<int>( info->rug_count ); i++ ) {
		//rug.angle = info->rugPos[i]->angle / 100.0f;
		rug.angle = Util::roll( -15.0f, 15.0f ); // fixme?
		rug.isHorizontal = ( info->rugPos[i]->isHorizontal == 1 );
		rug.texture = shapes->getRandomRug(); // fixme?
		setRugPosition( info->rugPos[i]->cx, info->rugPos[i]->cy, &rug );
	}

	// load traps
	for ( int i = 0; i < info->trapCount; i++ ) {
		TrapInfo *trapInfo = info->trap[ i ];
		int index = addTrap( trapInfo->x, trapInfo->y, trapInfo->w, trapInfo->h );
		Trap *trap = getTrapLoc( index );
		trap->discovered = ( trapInfo->discovered != 0 );
		trap->enabled = ( trapInfo->enabled != 0 );
		trap->type = static_cast<int>( trapInfo->type );
	}

	// load doors
	for ( int i = 0; i < static_cast<int>( info->locked_count ); i++ ) {
		locked[ info->locked[ i ]->key ] = ( info->locked[ i ]->value == 1 ? true : false );
	}
	for ( int i = 0; i < static_cast<int>( info->door_count ); i++ ) {
		doorToKey[ info->door[ i ]->key ] = info->door[ i ]->value;
		keyToDoor[ info->door[ i ]->value ] = info->door[ i ]->key;
	}

	// secret doors
	for ( int i = 0; i < static_cast<int>( info->secret_count ); i++ ) {
		secretDoors[ static_cast<int>( info->secret[ i ]->key ) ] = ( info->secret[ i ]->value == 1 ? true : false );
	}

	if ( report ) report->updateStatus( 4, 7, _( "Finishing map" ) );

	this->center( info->start_x, info->start_y, true );

	Persist::deleteMapInfo( info );

	if ( report ) report->updateStatus( 5, 7, _( "Loading Creatures" ) );

	// load map-related data from text file
	stringstream txtfileName;
	if ( templateMapName ) {
		if ( depth > 0 ) {
			txtfileName << rootDir << "/maps/" << templateMapName << depth << ".map";
		} else {
			txtfileName << rootDir << "/maps/" << templateMapName << ".map";
		}
	} else {
		txtfileName << fileName;
	}
	cerr << "Looking for txt file: " << txtfileName.str() << endl;
	adapter->loadMapData( txtfileName.str() );

	strcpy( this->name, ( templateMapName ? templateMapName : name.c_str() ) );

	if ( report ) report->updateStatus( 6, 7, _( "Starting party" ) );

	/*
	  FIXME: Place the party at the start. This code attempts to find a place
	  for the party near the gate (which was set in Scourge::useGate.) Need to
	  find a better AI solution as this code can place the party outside the walls.
	  For now always leave "whitespace" around gates in edited levels.
	*/
	if ( settings->isPlayerEnabled() ) {
		int xx = startx;
		int yy = starty;
		int nx, ny;
		for ( int t = 0; t < adapter->getPartySize(); t++ ) {
			if ( !adapter->getParty( t )->getStateMod( StateMod::dead ) ) {
				adapter->getParty( t )->findPlace( xx, yy, &nx, &ny );
				xx = nx;
				yy = ny;
			}
		}
	}
	result = _( "Map loaded: " ) + name;

	return true;
}

void Map::loadMapLocation( const std::string& name, std::string& result, int *gridX, int *gridY, int depth ) {
	Uint16 tmpX, tmpY;
	if ( name.empty() ) {
		result = _( "Enter a name of a map to load." );
		return;
	}

	stringstream fileName;
	if ( depth > 0 ) {
		fileName << rootDir << "/maps/" << name << depth << ".map";
	} else {
		fileName << rootDir << "/maps/" << name << ".map";
	}
	//cerr << "loading map header: " << fileName.str() << endl;

	FILE *fp = fopen( fileName.str().c_str(), "rb" );
	if ( !fp ) {
		result = _( "Can't find map: " ) + name;
		return;
	}
	File *file = new ZipFile( fp, ZipFile::ZIP_READ );
	Persist::loadMapHeader( file, &tmpX, &tmpY );
	delete file;

	*gridX = static_cast<int>( tmpX );
	*gridY = static_cast<int>( tmpY );

	result = _( "Map header loaded: " ) + name;
}


void Map::getMapXYAtScreenXY( Uint16 *mapx, Uint16 *mapy ) {
	getMapXYAtScreenXY( adapter->getMouseX(), adapter->getMouseY(), mapx, mapy );
}

void Map::getMapXYAtScreenXY( Uint16 x, Uint16 y, Uint16 *mapx, Uint16 *mapy ) {
	double obj_x, obj_y, obj_z;
	double win_x = static_cast<double>( x );
	double win_y = static_cast<double>( adapter->getScreenHeight() - y );

	// get the depth buffer value
	float win_z;
	glReadPixels( static_cast<int>( win_x ), static_cast<int>( win_y ), 1, 1, GL_DEPTH_COMPONENT, GL_FLOAT, &win_z );

	double projection[16];
	double modelview[16];
	GLint viewport[4];

	glGetDoublev( GL_PROJECTION_MATRIX, projection );
	glGetDoublev( GL_MODELVIEW_MATRIX, modelview );
	glGetIntegerv( GL_VIEWPORT, viewport );

	int res = gluUnProject( win_x, win_y, win_z, modelview, projection, viewport, &obj_x, &obj_y, &obj_z );

	if ( res ) {
		*mapx = getX() + ( Uint16 )( obj_x / MUL );
		*mapy = getY() + ( Uint16 )( obj_y / MUL );

		debugX = getX() + static_cast<int>( obj_x / MUL );
		debugY = getY() + static_cast<int>( obj_y / MUL );
		debugZ = static_cast<int>( obj_z / MUL );
	} else {
		*mapx = *mapy = MAP_WIDTH + 1;
	}
}

// these are arrived at by trial-and-error
#define RAY_PICK_MIN_T 0.6f
#define RAY_PICK_MAX_T 0.8f
#define RAY_PICK_STEP_COUNT 30
void Map::getMapXYZAtScreenXY( Uint16 *mapx, Uint16 *mapy, Uint16 *mapz, Location **pos ) {
	double win_x = static_cast<double>( adapter->getMouseX() );
	double win_y = static_cast<double>( adapter->getScreenHeight() - adapter->getMouseY() );

	double projection[16];
	double modelview[16];
	GLint viewport[4];

	glGetDoublev( GL_PROJECTION_MATRIX, projection );
	glGetDoublev( GL_MODELVIEW_MATRIX, modelview );
	glGetIntegerv( GL_VIEWPORT, viewport );

	// the near plane's mouse coordinates
	double px, py, pz;
	int res = gluUnProject( win_x, win_y, 0.0f, modelview, projection, viewport, &px, &py, &pz );

	// the far plane's mouse coordinates
	double qx, qy, qz;
	res = gluUnProject( win_x, win_y, 1.0f, modelview, projection, viewport, &qx, &qy, &qz );

	// Take steps along the pick-ray. A t of 0 to 1 is the line segment from the near to the far plane.
#if DEBUG_MOUSE_POS == 1
	cerr << "------------------------------" << endl;
#endif
	for ( int i = 0; i < RAY_PICK_STEP_COUNT; i++ ) {
		float t = RAY_PICK_MIN_T + ( ( ( float )i / ( float )RAY_PICK_STEP_COUNT ) * ( RAY_PICK_MAX_T - RAY_PICK_MIN_T ) );
		double ox = px + t * ( qx - px );
		double oy = py + t * ( qy - py );
		double oz = pz + t * ( qz - pz );

		double mx = getX() + ( ox / MUL );
		double my = getY() + ( oy / MUL ) + 2.0f;
		float gh = ( mx >= 0 && mx < MAP_WIDTH && my >= 0 && my < MAP_DEPTH ? findMaxHeightPos( mx, my, 0 ) : 0 );
		double mz = ( ( oz ) / MUL ) - gh;
#if DEBUG_MOUSE_POS == 1
		cerr << "map: " << mx << "," << my << "," << mz << " ground=" << gh << endl;
#endif
		*mapx = ( Uint16 )mx;
		*mapy = ( Uint16 )my;

		// hit something?
		for ( int xx = -1; xx < 1; xx++ ) {
			for ( int yy = -1; yy < 1; yy++ ) {
				for ( int zz = -1; zz < 1; zz++ ) {

					float fx = mx + xx;
					float fy = my + yy;
					float fz = mz + zz;

					if ( fx >= 0 && fx < MAP_WIDTH &&
					        fy >= 0 && fy < MAP_DEPTH &&
					        fz >= 0 && fz < MAP_VIEW_HEIGHT ) {
						Location *location = getLocation( ( int )fx, ( int )fy, ( int )fz );
						if ( location && ( location )->shape && ( ( location )->creature || ( location )->item || ( location )->shape->isInteractive() || isSecretDoor( location ) ) ) {
							Shape *shape = ( location )->shape;
							if ( mx >= ( location )->x + shape->getOffsX() / MUL &&
							        mx < ( location )->x + shape->getOffsX() / MUL + shape->getWidth() &&
							        my >= ( location )->y + shape->getOffsY() / MUL - shape->getDepth() &&
							        my < ( location )->y + shape->getOffsY() / MUL + 1 &&
							        mz >= ( location )->z + shape->getOffsZ() / MUL &&
							        mz < ( location )->z + shape->getOffsZ() / MUL + shape->getHeight() ) {
								*pos = location;
								*mapx = fx;
								*mapy = fy;
								*mapz = fz;
								snprintf( mapDebugStr, DEBUG_SIZE, "map: %.2f,%.2f,%.2f pos:%d,%d,%d - %d,%d,%d",
								          mx, my, mz,
								          location->x, location->y - shape->getDepth() + 1, location->z,
								          location->x + shape->getWidth(), location->y + 1, location->z + shape->getHeight() );
								adapter->setDebugStr( mapDebugStr );
								return;
							}
						}
					}
				}
			}
		}
		if ( toint( mz ) <= 0.25f ) {
			*pos = NULL;
			*mapz = 0;
			snprintf( mapDebugStr, DEBUG_SIZE, "map: %d,%d,%d", *mapx, *mapy, *mapz );
			adapter->setDebugStr( mapDebugStr );
			return;
		}
	}
	*pos = NULL;
	*mapz = 0;
	adapter->setDebugStr( "map: " );
}

/// Defines the render helper that should be used (dungeon, cave, outdoor...).

void Map::setMapRenderHelper( MapRenderHelper *helper ) {
	this->helper = helper;
	this->helper->setMap( this );
	LIGHTMAP_ENABLED = this->helper->isLightMapEnabled();
	// diff. default angle in outdoors
	if ( !helper->drawShadow() ) this->yrot = settings->getMaxYRot() - 1;
	//lightMapChanged = true;
}

/// Adds a secret door at x,y.

void Map::addSecretDoor( int x, int y ) {
	int index = y * MAP_WIDTH + x;
	secretDoors[ index ] = false;
	resortShapes = mapChanged = true;
}

/// Deletes the secret door at x,y.

void Map::removeSecretDoor( int x, int y ) {
	int index = y * MAP_WIDTH + x;
	if ( secretDoors.find( index ) != secretDoors.end() ) {
		secretDoors.erase( index );
		resortShapes = mapChanged = true;
	}
}

/// Is here a secret door?

bool Map::isSecretDoor( Location *pos ) {
	return isSecretDoor( pos->x, pos->y );
}

/// Is there a secret door at x,y?

bool Map::isSecretDoor( int x, int y ) {
	return( secretDoors.find( y * MAP_WIDTH + x ) != secretDoors.end() ? true : false );
}

/// Did we detect this secret door?

bool Map::isSecretDoorDetected( Location *pos ) {
	return isSecretDoorDetected( pos->x, pos->y );
}

/// Did we detect the secret door at x,y?

bool Map::isSecretDoorDetected( int x, int y ) {
	int index = y * MAP_WIDTH + x;
	if ( secretDoors.find( index ) != secretDoors.end() ) {
		return secretDoors[ index ];
	} else {
		return false;
	}
}

/// Mark a secret door as detected.

void Map::setSecretDoorDetected( Location *pos ) {
	setSecretDoorDetected( pos->x, pos->y );
}

/// Mark a secret door as detected.

void Map::setSecretDoorDetected( int x, int y ) {
	int index = y * MAP_WIDTH + x;
	if ( secretDoors.find( index ) != secretDoors.end() ) {
		secretDoors[ index ] = true;
	}
}

/// Adds a trap of size w,h at x,y.

int Map::addTrap( int x, int y, int w, int h ) {
	Trap trap;
	trap.r.x = x;
	trap.r.y = y;
	trap.r.w = w;
	trap.r.h = h;
	trap.type = 0;
	trap.discovered = false;
	trap.enabled = true;
	Uint8 trapIndex = ( Uint8 )trapList.size();
	vector<CVector2*> points;
	for ( int xx = x; xx < x + w; xx++ ) {
		for ( int yy = y; yy < y + h; yy++ ) {
			trapPos[ ( xx * ( Uint32 )MAP_WIDTH ) + yy ] = trapIndex;
		}
	}

	// find the convex hull
	for ( int xx = x; xx <= x + w; xx++ ) {
		for ( int yy = y - 2; yy <= y + h - 2; yy++ ) {
			if ( !isWall( static_cast<int>( xx ), static_cast<int>( yy ), 0 ) ) {
				CVector2 *p = new CVector2;
				p->x = xx;
				p->y = yy;
				points.push_back( p );
			}
		}
	}
	QuickHull::findConvexHull( &points, &( trap.hull ) );

	trapList.push_back( trap );
	mapChanged = true;
	return trapIndex;
}

/// Clears the traps.

void Map::clearTraps() {
	for ( unsigned int i = 0; i < trapList.size(); i++ ) {
		Trap& trap = trapList[ i ];
		while ( !trap.hull.empty() ) {
			delete trap.hull.back();
			trap.hull.pop_back();
		}
	}
	trapPos.clear();
	trapList.clear();
	trapSet.clear();
	selectedTrapIndex = -1;
}

/// Removes the trap specified by index.

void Map::removeTrap( int trapIndex ) {
	if ( static_cast<int>( trapList.size() ) > trapIndex ) {
		Trap trap = trapList[ trapIndex ];
		for ( int xx = trap.r.x; xx < trap.r.x + trap.r.w; xx++ ) {
			for ( int yy = trap.r.y; yy < trap.r.y + trap.r.h; yy++ ) {
				trapPos.erase( ( xx * ( Uint32 )MAP_WIDTH ) + yy );
			}
		}
		trapList.erase( trapList.begin() + trapIndex );
		mapChanged = true;
	}
}

/// Returns the trap at map pos x,y, or -1 if no trap found.

int Map::getTrapAtLoc( int x, int y ) {
	Uint32 key = ( x * ( Uint32 )MAP_WIDTH ) + y;
	if ( trapPos.find( key ) == trapPos.end() ) return -1;
	else return trapPos[ key ];
}

/// Returns the map location of the trap specified by index.

Trap *Map::getTrapLoc( int trapIndex ) {
	if ( static_cast<int>( trapList.size() ) <= trapIndex || trapIndex < 0 ) return NULL;
	else return &( trapList[ trapIndex ] );
}


/// Is it safe to put the shape on this map tile?

bool Map::canFit( int x, int y, Shape *shape ) {
	if ( x < MAP_OFFSET || x >= MAP_WIDTH - MAP_OFFSET ||
	        y < MAP_OFFSET || y >= MAP_DEPTH - MAP_OFFSET ) {
		return false;
	}
	int fx = ( ( x - MAP_OFFSET ) / MAP_UNIT ) * MAP_UNIT + MAP_OFFSET;
	int fy = ( ( y - MAP_OFFSET ) / MAP_UNIT ) * MAP_UNIT + MAP_OFFSET + MAP_UNIT;
	if ( isHeightMapEnabled() ) {
		int gx = fx / OUTDOORS_STEP;
		int gy = fy / OUTDOORS_STEP;
		if ( ground[ gx ][ gy ] < 10 && ground[ gx ][ gy ] > -10 ) {
			return( !isBlocked( x, y, 0, 0, 0, 0, shape, NULL ) ? true : false );
		}
	} else {
		Shape *floor = floorPositions[fx][fy];
		if ( floor ) {
			return( !isBlocked( x, y, 0, 0, 0, 0, shape, NULL ) ? true : false );
		}
	}
	return false;
}

/// Nothing on this map tile?

bool Map::isEmpty( int x, int y ) {
	if ( x < MAP_OFFSET || x >= MAP_WIDTH - MAP_OFFSET ||
	        y < MAP_OFFSET || y >= MAP_DEPTH - MAP_OFFSET ) {
		return false;
	}
	return( getLocation( x, y, 0 ) == NULL ? true : false );
}

bool Map::inMapEditor() {
	return settings->isGridShowing();
}

/// Determines which type of weather the map will have.

int Map::generateWeather() {
	if ( Util::dice( 3 ) == 0 && heightMapEnabled ) {
		weather = Util::pickOne( 1, MAX_WEATHER );
	} else {
		weather = WEATHER_CLEAR;
	}
	return weather;
}

/// Sets whether house roofs should be shown.

void Map::setRoofShowing( bool b ) {
	isRoofShowing = b;
	mapChanged = true;
}

/// Forces a refresh of the map state.

void Map::refresh() {
	mapChanged = lightMapChanged = resortShapes = true;
}
