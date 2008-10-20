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
#include "maprenderhelper.h"
#include "../debug.h"
#include "projectilerenderer.h"
#include "../quickhull.h"

using namespace std;


// set to 1 to enable bounding box and ground grid drawing
#define DEBUG_MOUSE_POS 0

#define USE_LIGHTING 1

#define MOUSE_ROT_DELTA 2

#define ZOOM_DELTA 1.2f

#define KEEP_MAP_SIZE 0

#define MVW 100
#define MVD 100

#define WATER_AMP 0.25f
#define WATER_ANIM_SPEED 10.0f
#define WATER_HEIGHT 1.2f
#define WATER_STEP 0.07f

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

const float Map::shadowTransformMatrix[16] = {
	1, 0, 0, 0,
	0, 1, 0, 0,
// 0.5f, -0.5f, 0, 0,
	0.75f, -0.25f, 0, 0,
	0, 0, 0, 1
};

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
	useShadow = false;
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
	colorAlreadySet = false;
	selectedDropTarget = NULL;

	helper = NULL;

	laterCount = stencilCount = otherCount = damageCount = roofCount = 0;

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

	adapter->writeLogMessage( Constants::getMessage( Constants::WELCOME ), Constants::MSGTYPE_SYSTEM );
	adapter->writeLogMessage( "----------------------------------", Constants::MSGTYPE_SYSTEM );
}

Map::~Map() {
	reset();
	delete hackBlockingPos;
	delete frustum;
	delete helper;
}

/// After work is done, reset everything.

void Map::reset() {

	creatureMap.clear();
	creatureEffectMap.clear();

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
	useShadow = false;
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
	colorAlreadySet = false;
	selectedDropTarget = NULL;

	if ( helper ) helper->reset();

	laterCount = stencilCount = otherCount = damageCount = roofCount = 0;

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



	/*
	int chunkOffsetX = 0;
	int chunkStartX = (getX() - MAP_OFFSET) / MAP_UNIT;
	int mod = (getX() - MAP_OFFSET) % MAP_UNIT;
	if(mod) {
	  chunkOffsetX = -mod;
	}
	int chunkEndX = mapViewWidth / MAP_UNIT + chunkStartX;

	int chunkOffsetY = 0;
	int chunkStartY = (getY() - MAP_OFFSET) / MAP_UNIT;
	mod = (getY() - MAP_OFFSET) % MAP_UNIT;
	if(mod) {
	  chunkOffsetY = -mod;
	}
	int chunkEndY = mapViewDepth / MAP_UNIT + chunkStartY;

	int posX, posY;
	for(int chunkX = chunkStartX; chunkX < chunkEndX; chunkX++) {
	  if(chunkX < 0 || chunkX > MAP_WIDTH / MAP_UNIT) continue;
	  for(int chunkY = chunkStartY; chunkY < chunkEndY; chunkY++) {
	    if(chunkY < 0 || chunkY > MAP_DEPTH / MAP_UNIT) continue;
	    for(int yp = 0; yp < MAP_UNIT; yp++) {
	      for(int xp = 0; xp < MAP_UNIT; xp++) {

	//           In scourge, shape coordinates are given by their
	//           left-bottom corner. So the +1 for posY moves the
	//           position 1 unit down the Y axis, which is the
	//           unit square's bottom left corner.
	        posX = chunkX * MAP_UNIT + xp + MAP_OFFSET;
	        posY = chunkY * MAP_UNIT + yp + MAP_OFFSET + 1;

	        for(int zp = 0; zp < MAP_VIEW_HEIGHT; zp++) {
	          if( effect[posX][posY][zp] && !effect[posX][posY][zp]->isEffectOn() ) {
	            removeEffect( posX, posY, zp );
	            mapChanged = true;
	          }
	        }
	      }
	    }
	  }
	}
	*/
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
		laterCount = stencilCount = otherCount = damageCount = roofCount = 0;
		creatureMap.clear();
		creatureEffectMap.clear();
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
						setupPosition( 0, CAVE_CHUNK_SIZE, 0, xpos2, ypos2, 0, pos[0][CAVE_CHUNK_SIZE][0]->shape, NULL, NULL, NULL );
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
				drawRug( &rugPos[ chunkX ][ chunkY ], xpos2, ypos2, chunkX, chunkY );
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
								drawWaterPosition( posX, posY, xpos2, ypos2, shape );
							} else {
								drawGroundPosition( posX, posY, xpos2, ypos2, shape );
							}
						}
					} else {

						if ( checkLightMap( chunkX, chunkY ) && itemPos[posX][posY] &&
						        itemPos[posX][posY]->x == posX && itemPos[posX][posY]->y == posY ) {

							shape = itemPos[posX][posY]->shape;

							xpos2 = static_cast<float>( ( chunkX - chunkStartX ) * MAP_UNIT + xp + chunkOffsetX ) * MUL;
							ypos2 = static_cast<float>( ( chunkY - chunkStartY ) * MAP_UNIT - shape->getDepth() + yp + chunkOffsetY ) * MUL;

							setupPosition( posX, posY, 0, xpos2, ypos2, 0, shape, itemPos[posX][posY]->item, NULL, NULL, true );
						}

						checkUnderRoof();

						for ( int zp = 0; zp < MAP_VIEW_HEIGHT; zp++ ) {
							if ( checkLightMap( chunkX, chunkY ) && effect[posX][posY][zp] && !effect[posX][posY][zp]->isInDelay() ) {
								xpos2 = static_cast<float>( ( chunkX - chunkStartX ) * MAP_UNIT + xp + chunkOffsetX ) * MUL;
								ypos2 = static_cast<float>( ( chunkY - chunkStartY ) * MAP_UNIT - 1 + yp + chunkOffsetY ) * MUL;
								zpos2 = static_cast<float>( zp ) * MUL;

								setupPosition( posX, posY, zp - effect[posX][posY][zp]->z, xpos2, ypos2, zpos2,
								               effect[posX][posY][zp]->effect->getShape(), NULL, NULL, effect[posX][posY][zp] );
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
		setupPosition( posX, posY, posZ, xpos2, ypos2, zpos2, location->shape, location->item, location->creature, NULL );
		//}
	}

}

/// Draws the rugs in Scourge HQ.

void Map::drawRug( Rug *rug, float xpos2, float ypos2, int xchunk, int ychunk ) {
	glPushMatrix();
	glTranslatef( xpos2, ypos2, 0.255f * MUL );
	glRotatef( rug->angle, 0.0f, 0.0f, 1.0f );
	float f = MAP_UNIT * MUL;
	float offset = 2.5f * MUL;

	float sx, sy, ex, ey;
	// starting section
	if ( rug->isHorizontal ) {
		sx = offset;
		sy = offset * 2;
		ex = f - offset;
		ey = f - offset * 2;
	} else {
		sy = offset;
		sx = offset * 2;
		ey = f - offset;
		ex = f - offset * 2;
	}

	glDisable( GL_CULL_FACE );
	glEnable( GL_TEXTURE_2D );
	glColor4f( 1.0f, 1.0f, 1.0f, 0.9f );
	rug->texture.glBind();
	glBegin( GL_TRIANGLE_STRIP );
	if ( rug->isHorizontal ) {
		glTexCoord2f( 1.0f, 0.0f );
		glVertex2f( sx, sy );
		glTexCoord2f( 1.0f, 1.0f );
		glVertex2f( ex, sy );
		glTexCoord2f( 0.0f, 0.0f );
		glVertex2f( sx, ey );
		glTexCoord2f( 0.0f, 1.0f );
		glVertex2f( ex, ey );
	} else {
		glTexCoord2f( 0.0f, 0.0f );
		glVertex2f( sx, sy );
		glTexCoord2f( 1.0f, 0.0f );
		glVertex2f( ex, sy );
		glTexCoord2f( 0.0f, 1.0f );
		glVertex2f( sx, ey );
		glTexCoord2f( 1.0f, 1.0f );
		glVertex2f( ex, ey );
	}
	glEnd();
	glDisable( GL_TEXTURE_2D );
	glPopMatrix();
}

/// Draws a shape sitting on the ground at the specified map coordinates.

void Map::drawGroundPosition( int posX, int posY,
                              float xpos2, float ypos2,
                              Shape *shape ) {
	GLuint name;
	// encode this shape's map location in its name
	name = posX + ( MAP_WIDTH * posY );
	glTranslatef( xpos2, ypos2, 0.0f );

	glPushName( name );
	glColor4f( 1.0f, 1.0f, 1.0f, 0.9f );
	if ( isHeightMapEnabled() ) {
		shape->drawHeightMap( ground, posX, posY );
	} else {
		shape->draw();
	}
	glPopName();

	glTranslatef( -xpos2, -ypos2, 0.0f );
}

/// Draws a shape placed on an indoor water tile.

void Map::drawWaterPosition( int posX, int posY,
                             float xpos2, float ypos2,
                             Shape *shape ) {
	GLuint name;
	// encode this shape's map location in its name
	name = posX + ( MAP_WIDTH * posY );
	glTranslatef( xpos2, ypos2, 0.0f );

	// draw water
	Uint32 key = createPairKey( posX, posY );
	if ( water.find( key ) != water.end() ) {
		glDisable( GL_CULL_FACE );

		float sx = ( static_cast<float>( MAP_UNIT ) / static_cast<float>( WATER_TILE_X ) ) * MUL;
		float sy = ( static_cast<float>( MAP_UNIT ) / static_cast<float>( WATER_TILE_Y ) ) * MUL;

		int xp = 0;
		int yp = 0;
		while ( true ) {
			int stx = xp;
			int sty = yp;
			glBegin( GL_TRIANGLE_STRIP );
			for ( int i = 0; i < 4; i++ ) {
				int wx, wy;
				if ( xp == WATER_TILE_X && yp == WATER_TILE_Y ) {
					wx = ( posX + MAP_UNIT ) * WATER_TILE_X;
					wy = ( posY + MAP_UNIT ) * WATER_TILE_Y;
				} else if ( xp == WATER_TILE_X ) {
					wx = ( posX + MAP_UNIT ) * WATER_TILE_X;
					wy = posY * WATER_TILE_Y + yp;
				} else if ( yp == WATER_TILE_Y ) {
					wx = posX * WATER_TILE_X + xp;
					wy = ( posY + MAP_UNIT ) * WATER_TILE_Y;
				} else {
					wx = posX * WATER_TILE_X + xp;
					wy = posY * WATER_TILE_Y + yp;
				}

				int xx = wx % WATER_TILE_X;
				int yy = wy % WATER_TILE_Y;
				WaterTile *w = NULL;
				Uint32 key = createPairKey( wx / WATER_TILE_X, wy / WATER_TILE_Y );
				if ( water.find( key ) != water.end() ) {
					w = water[key];

					Uint32 time = SDL_GetTicks();
					Uint32 elapsedTime = time - w->lastTime[xx][yy];
					if ( elapsedTime >= ( Uint32 )( 1000.0f / WATER_ANIM_SPEED ) ) {

						w->z[xx][yy] += w->step[xx][yy];
						if ( w->z[xx][yy] > WATER_AMP ||
						        w->z[xx][yy] < -WATER_AMP ) w->step[xx][yy] *= -1.0f;

						w->lastTime[xx][yy] = time;
					}
				}

				float zz = ( w ? w->z[xx][yy] : 0.0f );
				float sz = ( WATER_HEIGHT + zz ) * MUL;
				glColor4f( 0.3f + ( zz / 30.0f ),
				           0.25f + ( zz / 10.0f ),
				           0.17f + ( zz / 15.0f ),
				           0.5f );


				glVertex3f( static_cast<float>( xp ) * sx, static_cast<float>( yp ) * sy, sz );

				switch ( i ) {
				case 0: xp++; break;
				case 1: yp++; xp--; break;
				case 2: xp++; break;
				case 3: yp--; xp--; break;
				}
				if ( xp > WATER_TILE_X || yp > WATER_TILE_Y ) {
					break;
				}
			}
			glEnd();
			xp = stx + 1;
			yp = sty;
			if ( xp >= WATER_TILE_X ) {
				xp = 0;
				yp++;
				if ( yp >= WATER_TILE_Y ) break;
			}
		}


		//glDepthMask( GL_TRUE );
		//glDisable( GL_BLEND );
	}

	glTranslatef( -xpos2, -ypos2, 0.0f );
}


void Map::setupPosition( int posX, int posY, int posZ,
                         float xpos2, float ypos2, float zpos2,
                         Shape *shape, RenderedItem *item, RenderedCreature *creature,
                         EffectLocation *effect,
                         bool itemPos ) {

	// This really doesn't make a difference unfortunately.
	//if(!isOnScreen(posX, posY, posZ)) return;

	GLuint name;
	name = posX + ( MAP_WIDTH * ( posY ) ) + ( MAP_WIDTH * MAP_DEPTH * posZ );

	// special effects
	if ( ( effect || ( creature && creature->isEffectOn() ) ) && !shape->isRoof() ) {
		damage[damageCount].xpos = xpos2;
		damage[damageCount].ypos = ypos2;
		damage[damageCount].zpos = zpos2;
		damage[damageCount].shape = shape;
		damage[damageCount].item = item;
		damage[damageCount].creature = creature;
		damage[damageCount].effect = effect;
		damage[damageCount].name = name;
		damage[damageCount].pos = ( itemPos ? getItemLocation( posX, posY ) : getLocation( posX, posY, posZ ) );
		damage[damageCount].inFront = false;
		damage[damageCount].x = posX;
		damage[damageCount].y = posY;
		if ( creature ) {
			creatureEffectMap[creature] = &( damage[damageCount] );
		}
		damageCount++;

		// don't draw shape if it's an area effect
		if ( !creature ) return;
	}

	if ( shape->isRoof() ) {
		roof[roofCount].xpos = xpos2;
		roof[roofCount].ypos = ypos2;
		roof[roofCount].zpos = zpos2;
		roof[roofCount].shape = shape;
		roof[roofCount].item = item;
		roof[roofCount].creature = creature;
		roof[roofCount].effect = NULL;
		roof[roofCount].name = name;
		roof[roofCount].pos = ( itemPos ? getItemLocation( posX, posY ) : getLocation( posX, posY, posZ ) );
		roof[roofCount].inFront = false;
		roof[roofCount].x = posX;
		roof[roofCount].y = posY;
		roofCount++;
	} else if ( shape->isStencil() ) {
		stencil[stencilCount].xpos = xpos2;
		stencil[stencilCount].ypos = ypos2;
		stencil[stencilCount].zpos = zpos2;
		stencil[stencilCount].shape = shape;
		stencil[stencilCount].item = item;
		stencil[stencilCount].creature = creature;
		stencil[stencilCount].effect = NULL;
		stencil[stencilCount].name = name;
		stencil[stencilCount].pos = ( itemPos ? getItemLocation( posX, posY ) : getLocation( posX, posY, posZ ) );
		stencil[stencilCount].inFront = false;
		stencil[stencilCount].x = posX;
		stencil[stencilCount].y = posY;
		stencilCount++;
	} else if ( !shape->isStencil() ) {
		if ( shape->drawFirst() ) {
			other[otherCount].xpos = xpos2;
			other[otherCount].ypos = ypos2;
			other[otherCount].zpos = zpos2;
			other[otherCount].shape = shape;
			other[otherCount].item = item;
			other[otherCount].creature = creature;
			other[otherCount].effect = NULL;
			other[otherCount].name = name;
			other[otherCount].pos = ( itemPos ? getItemLocation( posX, posY ) : getLocation( posX, posY, posZ ) );
			other[otherCount].inFront = false;
			other[otherCount].x = posX;
			other[otherCount].y = posY;
			if ( creature ) {
				creatureMap[creature] = &( other[otherCount] );
			}
			otherCount++;
		}
		if ( shape->drawLater() ) {
			later[laterCount].xpos = xpos2;
			later[laterCount].ypos = ypos2;
			later[laterCount].zpos = zpos2;
			later[laterCount].shape = shape;
			later[laterCount].item = item;
			later[laterCount].creature = creature;
			later[laterCount].effect = NULL;
			later[laterCount].name = name;
			later[laterCount].pos = ( itemPos ? getItemLocation( posX, posY ) : getLocation( posX, posY, posZ ) );
			later[laterCount].inFront = false;
			later[laterCount].x = posX;
			later[laterCount].y = posY;
			laterCount++;
		}
	}
}

/// Current camera zoom as a percentage value.

float Map::getZoomPercent() {
	return ( zoom - settings->getMinZoomIn() ) / ( settings->getMaxZoomOut() - settings->getMinZoomIn() );
}

/// Initialization before doing the actual drawing.

void Map::preDraw() {
	if ( refreshGroundPos ) {
		createGroundMap();
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
		//if( settings->isPlayerEnabled() && adapter->getPlayer() ) adapter->getPlayer()->setMapChanged();
		int csx, cex, csy, cey;
		setupShapes( false, false, &csx, &cex, &csy, &cey );
#ifdef SHOW_FPS
//     int shapeCount = laterCount + otherCount + damageCount + stencilCount;
//     if( settings->isPlayerEnabled() ) {
//       snprintf(mapDebugStr, DEBUG_SIZE, "c=%d,%d p=%d,%d chunks=(%s %d out of %d) x:%d-%d y:%d-%d shapes=%d trap=%d zoom=%.2f xrot=%.2f yrot=%.2f zrot=%.2f",
//       cursorFlatMapX, cursorFlatMapY + 1,
//       ( adapter->getPlayer() ? toint(adapter->getPlayer()->getX()) : -1 ),
//       ( adapter->getPlayer() ? toint(adapter->getPlayer()->getY()) : -1 ),
//       (useFrustum ? "*" : ""),
//       chunkCount, ((cex - csx)*(cey - csy)),
//       csx, cex, csy, cey, shapeCount, selectedTrapIndex,
//       zoom, xrot, yrot, zrot );
//       //            shapeCount, laterCount, otherCount, damageCount, stencilCount);
//     } else {
//       snprintf(mapDebugStr, DEBUG_SIZE, "E=%d chunks=(%s %d out of %d) x:%d-%d y:%d-%d shapes=%d",
//               static_cast<int>(currentEffectsMap.size()), (useFrustum ? "*" : ""), chunkCount, ((cex - csx)*(cey - csy)),
//               csx, cex, csy, cey, shapeCount);
//     }
//     adapter->setDebugStr(mapDebugStr);
#endif
	}
}

/// Draws traps, and cleans up after drawing the main 3D view.

void Map::postDraw() {
	drawTraps();

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

/// Draws the complete 3D view.

void Map::draw() {
	if ( helper->isIndoors() ) {
		drawIndoors();
	} else {
		drawOutdoors();
	}

	drawProjectiles();

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

	if ( DEBUG_MOUSE_POS || ( settings->isGridShowing() && gridEnabled ) ) {
		willDrawGrid();
	}
}

/// Renders the 3D view for indoor levels.

void Map::drawIndoors() {
	if ( preferences->getStencilbuf() && preferences->getStencilBufInitialized() ) {
		// stencil and draw the floor
		//glDisable(GL_DEPTH_TEST);
		//glColorMask(0,0,0,0);
		glEnable( GL_STENCIL_TEST );
		glStencilOp( GL_REPLACE, GL_REPLACE, GL_REPLACE );
		glStencilFunc( GL_ALWAYS, 1, 0xffffffff );

		// cave floor and map editor bottom (so cursor shows)
		if ( settings->isGridShowing() || floorTexWidth > 0 || isHeightMapEnabled() ) {
			renderFloor();
		} else {
			setupShapes( true, false );
		}

		// shadows
		if ( preferences->getShadows() >= Constants::OBJECT_SHADOWS &&
		        helper->drawShadow() ) {
			glStencilFunc( GL_EQUAL, 1, 0xffffffff );
			glStencilOp( GL_KEEP, GL_KEEP, GL_INCR );
			glDisable( GL_TEXTURE_2D );
			glDepthMask( GL_FALSE );
			glEnable( GL_BLEND );
			useShadow = true;
			glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );
			for ( int i = 0; i < otherCount; i++ ) {
				doDrawShape( &other[i] );
			}
			if ( preferences->getShadows() == Constants::ALL_SHADOWS ) {
				for ( int i = 0; i < stencilCount; i++ ) {
					doDrawShape( &stencil[i] );
				}
			}
			useShadow = false;
			glDisable( GL_BLEND );
			glEnable( GL_TEXTURE_2D );
			glDepthMask( GL_TRUE );
		}

		//glEnable(GL_DEPTH_TEST);
		glDisable( GL_STENCIL_TEST );
	} else {
		// draw the ground
		setupShapes( true, false );
	}

	// draw lava flows
	for ( int i = 0; i < otherCount; i++ ) {
		if ( other[i].shape->isFlatCaveshape() ) {
			doDrawShape( &other[i] );
		}
	}

	// draw the creatures/objects/doors/etc.
	DrawLater *playerDrawLater = NULL;
	for ( int i = 0; i < otherCount; i++ ) {
		if ( other[i].shape->isFlatCaveshape() ) continue;
		if ( settings->isPlayerEnabled() ) {
			if ( other[i].creature && other[i].creature == adapter->getPlayer() )
				playerDrawLater = &( other[i] );
		}
		if ( selectedDropTarget && ( ( selectedDropTarget->creature && selectedDropTarget->creature == other[i].creature ) ||
		                             ( selectedDropTarget->item && selectedDropTarget->item == other[i].item ) ) ) {
			colorAlreadySet = true;
			glColor4f( 0.0f, 1.0f, 1.0f, 1.0f );
		}
		doDrawShape( &other[i] );

		// FIXME: if feeling masochistic, try using stencil buffer to remove shadow-on-shadow effect.
		// draw simple shadow in outdoors
		if ( !helper->drawShadow() ) {
			if ( other[i].creature ) {
				glColor4f( 0.04f, 0.0f, 0.07f, 0.4f );
				drawGroundTex( outdoorShadow, other[i].creature->getX() + 0.25f, other[i].creature->getY() + 0.25f, ( other[i].creature->getShape()->getWidth() + 2 ) * 0.7f, other[i].creature->getShape()->getDepth() * 0.7f );
			} else if ( other[i].pos && other[i].shape && other[i].shape->isOutdoorShadow() ) {
				glColor4f( 0.04f, 0.0f, 0.07f, 0.4f );
				drawGroundTex( outdoorShadowTree,  static_cast<float>( other[i].pos->x ) - ( other[i].shape->getWidth() / 2.0f ) + ( other[i].shape->getWindValue() / 2.0f ), static_cast<float>( other[i].pos->y ) + ( other[i].shape->getDepth() / 2.0f ), other[i].shape->getWidth() * 1.7f, other[i].shape->getDepth() * 1.7f );
			}
		}
	}

	// draw the walls: walls in front of the player will be transparent
	if ( playerDrawLater ) {

		if ( floorTexWidth == 0 && resortShapes ) {
			if ( helper->isShapeSortingEnabled() ) {
				sortShapes( playerDrawLater, stencil, stencilCount );
			}
			resortShapes = false;
		}

		// draw walls behind the player
		for ( int i = 0; i < stencilCount; i++ ) if ( !( stencil[i].inFront ) ) doDrawShape( &( stencil[i] ) );

		// draw walls in front of the player and water effects
		glEnable( GL_BLEND );
		glDepthMask( GL_FALSE );
		if ( hasWater && preferences->getStencilbuf() && preferences->getStencilBufInitialized() ) {

			// stencil out the transparent walls (and draw them)
			//glDisable(GL_DEPTH_TEST);
			//glColorMask(0,0,0,0);
			glClear( GL_STENCIL_BUFFER_BIT );
			glEnable( GL_STENCIL_TEST );
			glStencilOp( GL_REPLACE, GL_REPLACE, GL_REPLACE );
			glStencilFunc( GL_ALWAYS, 1, 0xffffffff );
			// draw walls blended in front of the player
			// 6,2 6,4 work well
			// FIXME: blending walls have some artifacts that depth-sorting
			// is supposed to get rid of but that didn't work for me.
			//glBlendFunc( GL_SRC_ALPHA, GL_SRC_COLOR );
			glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );
			for ( int i = 0; i < stencilCount; i++ ) {
				if ( stencil[i].inFront ) {
					glColor4f( 1.0f, 1.0f, 1.0f, 0.45f );
					colorAlreadySet = true;
					doDrawShape( &( stencil[i] ) );
				}
			}

			// draw the water (except where the transp. walls are)
			glStencilFunc( GL_NOTEQUAL, 1, 0xffffffff );
			glStencilOp( GL_KEEP, GL_KEEP, GL_KEEP );
			glDisable( GL_TEXTURE_2D );
			glBlendFunc( GL_ONE, GL_SRC_COLOR );
			setupShapes( false, true );
			glEnable( GL_TEXTURE_2D );

			glDisable( GL_STENCIL_TEST );
		} else {
			// draw transp. walls and water w/o stencil buffer
			//        glBlendFunc( GL_SRC_ALPHA, GL_SRC_COLOR );
			glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );
			for ( int i = 0; i < stencilCount; i++ ) {
				if ( stencil[i].inFront ) {
					glColor4f( 1.0f, 1.0f, 1.0f, 0.45f );
					colorAlreadySet = true;
					doDrawShape( &( stencil[i] ) );
				}
			}
			if ( hasWater ) {
				glDisable( GL_TEXTURE_2D );
				glBlendFunc( GL_ONE, GL_SRC_COLOR );
				setupShapes( false, true );
				glEnable( GL_TEXTURE_2D );
			}
		}
		glDepthMask( GL_TRUE );

	} else {
		// no player; just draw the damn walls
		for ( int i = 0; i < stencilCount; i++ )
			doDrawShape( &( stencil[i] ) );

		// draw water (has to come after walls to look good)
		if ( hasWater ) {
			glEnable( GL_BLEND );
			glDepthMask( GL_FALSE );
			glDisable( GL_TEXTURE_2D );
			glBlendFunc( GL_ONE, GL_SRC_COLOR );
			setupShapes( false, true );
			glDepthMask( GL_TRUE );
		}
	}

	// draw the effects
	glEnable( GL_TEXTURE_2D );
	glEnable( GL_BLEND );
	glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );

	// draw the roofs
	Uint32 now = SDL_GetTicks();
	if ( now - roofAlphaUpdate > 25 ) {
		roofAlphaUpdate = now;
		if ( isCurrentlyUnderRoof ) {
			if ( roofAlpha > 0 ) {
				roofAlpha -= 0.05f;
			} else {
				roofAlpha = 0;
			}
		} else {
			if ( roofAlpha < 1 ) {
				roofAlpha += 0.05f;
			} else {
				roofAlpha = 1;
			}
		}
	}
	if ( roofAlpha > 0 ) {
		for ( int i = 0; i < roofCount; i++ ) {
			( ( GLShape* )roof[i].shape )->setAlpha( roofAlpha );
			doDrawShape( &roof[i] );
		}
	}

	glDepthMask( GL_FALSE );

	for ( int i = 0; i < laterCount; i++ ) {
		later[i].shape->setupBlending();
		doDrawShape( &later[i] );
		later[i].shape->endBlending();
	}
	glBlendFunc( GL_SRC_ALPHA, GL_ONE );
	for ( int i = 0; i < damageCount; i++ ) {
		doDrawShape( &damage[i], 1 );
	}

	// draw the fog of war or shading
#ifdef USE_LIGHTING
#if DEBUG_MOUSE_POS == 0
	if ( helper && !adapter->isInMovieMode() && !( isCurrentlyUnderRoof && !groundVisible ) ) helper->draw( getX(), getY(), MVW, MVD );
#endif
#endif

	glDisable( GL_BLEND );
	glDepthMask( GL_TRUE );
}

/// Renders the 3D view for outdoor levels.

void Map::drawOutdoors() {
	// draw the ground
	renderFloor();

	// draw the creatures/objects/doors/etc.
	for ( int i = 0; i < otherCount; i++ ) {
		if ( selectedDropTarget && ( ( selectedDropTarget->creature && selectedDropTarget->creature == other[i].creature ) ||
		                             ( selectedDropTarget->item && selectedDropTarget->item == other[i].item ) ) ) {
			colorAlreadySet = true;
			glColor4f( 0.0f, 1.0f, 1.0f, 1.0f );
		}
		doDrawShape( &other[i] );

		// FIXME: if feeling masochistic, try using stencil buffer to remove shadow-on-shadow effect.
		// draw simple shadow in outdoors
		if ( other[i].creature ) {
			glColor4f( 0.04f, 0.0f, 0.07f, 0.4f );
			drawGroundTex( outdoorShadow, other[i].creature->getX() + 0.25f, other[i].creature->getY() + 0.25f, ( other[i].creature->getShape()->getWidth() + 2 ) * 0.7f, other[i].creature->getShape()->getDepth() * 0.7f );
		} else if ( other[i].pos && other[i].shape && other[i].shape->isOutdoorShadow() ) {
			glColor4f( 0.04f, 0.0f, 0.07f, 0.4f );
			drawGroundTex( outdoorShadowTree, static_cast<float>( other[i].pos->x ) - ( other[i].shape->getWidth() / 2.0f ) + ( other[i].shape->getWindValue() / 2.0f ), static_cast<float>( other[i].pos->y ) + ( other[i].shape->getDepth() / 2.0f ), other[i].shape->getWidth() * 1.7f, other[i].shape->getDepth() * 1.7f );
		}
	}

	for ( int i = 0; i < stencilCount; i++ ) doDrawShape( &( stencil[i] ) );

	// draw the effects
	glEnable( GL_TEXTURE_2D );
	glEnable( GL_BLEND );
	drawRoofs();

	glDepthMask( GL_FALSE );
	drawEffects();

	// draw the fog of war or shading
#ifdef USE_LIGHTING
#if DEBUG_MOUSE_POS == 0
	if ( helper && !adapter->isInMovieMode() && !( isCurrentlyUnderRoof && !groundVisible ) ) {
		helper->draw( getX(), getY(), MVW, MVD );
	}
#endif
#endif

	glDisable( GL_BLEND );
	glDepthMask( GL_TRUE );
}

/// Draws creature effects and damage counters.

void Map::drawEffects() {
	for ( int i = 0; i < laterCount; i++ ) {
		later[i].shape->setupBlending();
		doDrawShape( &later[i] );
		later[i].shape->endBlending();
	}
	glBlendFunc( GL_SRC_ALPHA, GL_ONE );
	for ( int i = 0; i < damageCount; i++ ) {
		doDrawShape( &damage[i], 1 );
	}
}

/// Draws the roofs on outdoor levels, including the fading.

void Map::drawRoofs() {
	// draw the roofs
	Uint32 now = SDL_GetTicks();
	if ( now - roofAlphaUpdate > 25 ) {
		roofAlphaUpdate = now;
		if ( isCurrentlyUnderRoof ) {
			if ( roofAlpha > 0 ) {
				roofAlpha -= 0.05f;
			} else {
				roofAlpha = 0;
			}
		} else {
			if ( roofAlpha < 1 ) {
				roofAlpha += 0.05f;
			} else {
				roofAlpha = 1;
			}
		}
	}
	if ( roofAlpha > 0 ) {
//  glEnable( GL_BLEND );
//  glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );
		for ( int i = 0; i < roofCount; i++ ) {
			( ( GLShape* )roof[i].shape )->setAlpha( roofAlpha );
			doDrawShape( &roof[i] );
		}
//    glDisable( GL_BLEND );
	}
}

void Map::willDrawGrid() {

	glDisable( GL_CULL_FACE );
	glDisable( GL_TEXTURE_2D );

	glEnable( GL_BLEND );
	glDepthMask( GL_FALSE );
	glBlendFunc( GL_SRC_ALPHA, GL_ONE );

	// draw the starting position
	float xpos2 = static_cast<float>( this->startx - getX() ) * MUL;
	float ypos2 = static_cast<float>( this->starty - getY() - 1 ) * MUL;
	float zpos2 = 0.0f * MUL;
	float w = 2.0f * MUL;
	float h = 4.0f * MUL;
	if ( useFrustum && frustum->CubeInFrustum( xpos2, ypos2, 0.0f, w * MUL ) ) {
		for ( int i = 0; i < 2; i++ ) {
			glPushMatrix();
			glTranslatef( xpos2, ypos2, zpos2 );
			if ( i == 0 ) {
				glColor4f( 1.0f, 0.0f, 0.0f, 0.5f );
				glBegin( GL_TRIANGLES );
			} else {
				glColor4f( 1.0f, 0.7f, 0.0f, 0.5f );
				glBegin( GL_LINE_LOOP );
			}

			glVertex3f( 0.0f, 0.0f, 0.0f );
			glVertex3f( -w, -w, h );
			glVertex3f( w, -w, h );

			glVertex3f( 0.0f, 0.0f, 0.0f );
			glVertex3f( -w, w, h );
			glVertex3f( w, w, h );

			glVertex3f( 0.0f, 0.0f, 0.0f );
			glVertex3f( -w, -w, h );
			glVertex3f( -w, w, h );

			glVertex3f( 0.0f, 0.0f, 0.0f );
			glVertex3f( w, -w, h );
			glVertex3f( w, w, h );


			glVertex3f( 0.0f, 0.0f, h * 2 );
			glVertex3f( -w, -w, h );
			glVertex3f( w, -w, h );

			glVertex3f( 0.0f, 0.0f, h * 2 );
			glVertex3f( -w, w, h );
			glVertex3f( w, w, h );

			glVertex3f( 0.0f, 0.0f, h * 2 );
			glVertex3f( -w, -w, h );
			glVertex3f( -w, w, h );

			glVertex3f( 0.0f, 0.0f, h * 2 );
			glVertex3f( w, -w, h );
			glVertex3f( w, w, h );

			glEnd();
			glPopMatrix();
		}
	}

	glDisable( GL_DEPTH_TEST );
	glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );
	int chunkX = ( cursorFlatMapX - MAP_OFFSET ) / MAP_UNIT;
	int chunkY = ( cursorFlatMapY - MAP_OFFSET - 1 ) / MAP_UNIT;
	float m = 0.5f * MUL;
	int const TMPLEN = 100;
	char tmp[TMPLEN];
	for ( int i = 0; i < chunkCount; i++ ) {

		float n = static_cast<float>( MAP_UNIT ) * MUL;

		glPushMatrix();
		glTranslatef( chunks[i].x, chunks[i].y - ( 1.0f * MUL ), 0 );

		if ( chunks[i].cx == chunkX && chunks[i].cy == chunkY ) {
			glColor4f( 0.0f, 1.0f, 0.0f, 0.25f );
			glLineWidth( 1 );
			snprintf( tmp, TMPLEN, "%d,%d", ( chunkX * MAP_UNIT + MAP_OFFSET ), ( chunkY * MAP_UNIT + MAP_OFFSET + 1 ) );
			adapter->texPrint( 0, 0, tmp );
			for ( int xx = 1; xx < MAP_UNIT; xx++ ) {
				glBegin( GL_LINES );
				glVertex3f( 0, xx * MUL, m );
				glVertex3f( n, xx * MUL, m );
				glEnd();
				glBegin( GL_LINES );
				glVertex3f( xx * MUL, 0, m );
				glVertex3f( xx * MUL, n, m );
				glEnd();
			}
		} else {
			glColor4f( 1.0f, 1.0f, 1.0f, 0.25f );
			glLineWidth( 1 );
		}
		glBegin( GL_LINE_LOOP );
		glVertex3f( 0.0f, 0.0f, m );
		glVertex3f( n, 0.0f, m );
		glVertex3f( n, n, m );
		glVertex3f( 0.0f, n, m );
		glEnd();
		glPopMatrix();
	}

	glPushMatrix();

	float xp = static_cast<float>( cursorFlatMapX - getX() ) * MUL;
	float yp = ( static_cast<float>( cursorFlatMapY - getY() ) - 1.0f ) * MUL;
	float cw = static_cast<float>( cursorWidth ) * MUL;
	float cd = -static_cast<float>( cursorDepth ) * MUL;
	m = ( cursorZ ? cursorZ : 0.5f ) * MUL;
	float ch = static_cast<float>( cursorHeight + cursorZ ) * MUL;

	float red = 1.0f;
	float green = 0.9f;
	float blue = 0.15f;
	bool found = false;
	if ( cursorFlatMapX < MAP_WIDTH && cursorFlatMapY < MAP_DEPTH ) {
		for ( int xx = cursorFlatMapX; xx < cursorFlatMapX + cursorWidth; xx++ ) {
			for ( int yy = cursorFlatMapY - 1; yy >= cursorFlatMapY - cursorDepth; yy-- ) {
				for ( int zz = 0; zz < cursorHeight; zz++ ) {
					if ( pos[xx][yy + 1][zz] ) {
						found = true;
						break;
					}
				}
			}
		}
	}
	if ( found ) {
		green = 0.15f;
	}

	// draw the cursor
	glColor4f( red, green, blue, 0.25f );
	glTranslatef( xp, yp, 0.0f );
	glBegin( GL_QUADS );

	glVertex3f( 0.0f, 0.0f, m );
	glVertex3f( cw, 0.0f, m );
	glVertex3f( cw, cd, m );
	glVertex3f( 0.0f, cd, m );

	glVertex3f( 0.0f, 0.0f, ch );
	glVertex3f( cw, 0.0f, ch );
	glVertex3f( cw, cd, ch );
	glVertex3f( 0.0f, cd, ch );

	glVertex3f( 0.0f, 0.0f, m );
	glVertex3f( cw, 0.0f, m );
	glVertex3f( cw, 0.0f, ch );
	glVertex3f( 0.0f, 0.0f, ch );

	glVertex3f( 0.0f, cd, m );
	glVertex3f( cw, cd, m );
	glVertex3f( cw, cd, ch );
	glVertex3f( 0.0f, cd, ch );

	glVertex3f( 0.0f, 0.0f, m );
	glVertex3f( 0.0f, cd, m );
	glVertex3f( 0.0f, cd, ch );
	glVertex3f( 0.0f, 0.0f, ch );

	glVertex3f( cw, 0.0f, m );
	glVertex3f( cw, cd, m );
	glVertex3f( cw, cd, ch );
	glVertex3f( cw, 0.0f, ch );

	glEnd();
	glPopMatrix();

	glEnable( GL_CULL_FACE );
	glEnable( GL_TEXTURE_2D );

	glDisable( GL_BLEND );
	glDepthMask( GL_TRUE );
	glEnable( GL_DEPTH_TEST );
}

/// Sorts the shapes in respect to the player's position.

/// Since rooms are rectangular, we can do this hack... a wall horizontal
/// wall piece will use the player's x coord. and its y coord.
/// A vertical one will use its X and the player's Y.
/// This is so that even if the wall extends below the player the entire
/// length has the same characteristics.

void Map::sortShapes( DrawLater *playerDrawLater, DrawLater *shapes, int shapeCount ) {
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
		if ( shapes[i].shape->getWidth() == shapes[i].shape->getDepth() ) {
			shapes[i].inFront = false;
			continue;
		} else if ( shapes[i].shape->getWidth() > shapes[i].shape->getDepth() ) {
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
		if ( shapes[i].shape->getWidth() == shapes[i].shape->getDepth() ) {
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

/// Draws the projectiles.

void Map::drawProjectiles() {
	for ( map<RenderedCreature*, vector<RenderedProjectile*>*>::iterator i = RenderedProjectile::getProjectileMap()->begin();
	        i != RenderedProjectile::getProjectileMap()->end(); ++i ) {
		//RenderedCreature *creature = i->first;
		vector<RenderedProjectile*> *p = i->second;
		for ( vector<RenderedProjectile*>::iterator e = p->begin(); e != p->end(); ++e ) {
			RenderedProjectile *proj = *e;

			// calculate the path
			vector<CVector3> path;
			for ( int i = 0; i < proj->getStepCount(); i++ ) {
				CVector3 v;
				v.x = ( ( proj->getX( i ) + proj->getRenderer()->getOffsetX() - static_cast<float>( getX() ) ) * MUL );
				v.y = ( ( proj->getY( i ) - proj->getRenderer()->getOffsetY() - static_cast<float>( getY() ) - 1.0f ) * MUL );
				v.z = ( proj->getZ( i ) + proj->getRenderer()->getOffsetZ() ) * MUL;
				path.push_back( v );
			}
			proj->getRenderer()->drawPath( this, proj, &path );
		}
	}
}

/// Draws a shape stored in a DrawLater object.

void Map::doDrawShape( DrawLater *later, int effect ) {
	doDrawShape( later->xpos, later->ypos, later->zpos, later->shape, effect, later );
}

void Map::doDrawShape( float xpos2, float ypos2, float zpos2, Shape *shape, int effect, DrawLater *later ) {

	// fog test for creatures
	if ( helper && later && later->creature && !adapter->isInMovieMode() && !helper->isVisible( later->pos->x, later->pos->y, later->creature->getShape() ) ) {
		return;
	}

	if ( shape ) ( ( GLShape* )shape )->useShadow = useShadow;

	// slow on mac os X:
	// glPushAttrib(GL_ENABLE_BIT);

	glPushMatrix();

	float heightPos = 0.0f;
	if ( later && later->pos ) {
		GLShape *s = ( GLShape* )later->pos->shape;
		if ( s->isVirtual() ) {
			s = ( ( VirtualShape* )s )->getRef();
		}

		if ( !s->getIgnoreHeightMap() ) {
			heightPos = later->pos->heightPos * MUL;
		}
	} else if ( later && later->effect ) {
		heightPos = later->effect->heightPos;
	}

	float xdiff = 0;
	float ydiff = 0;
	if ( later && later->creature ) {
		xdiff = ( later->creature->getX() - static_cast<float>( toint( later->creature->getX() ) ) );
		ydiff = ( later->creature->getY() - static_cast<float>( toint( later->creature->getY() ) ) );
	}

	if ( useShadow ) {
		// put shadow above the floor a little
		glTranslatef( xpos2 + xdiff * MUL, ypos2 + ydiff * MUL, ( 0.26f * MUL + heightPos ) );
		glMultMatrixf( shadowTransformMatrix );
		// purple shadows
		glColor4f( 0.04f, 0.0f, 0.07f, 0.6f );
	} else {
		glTranslatef( xpos2 + xdiff * MUL, ypos2 + ydiff * MUL, zpos2 + heightPos );

		if ( later && later->pos ) {
			glTranslatef( later->pos->moveX, later->pos->moveY, later->pos->moveZ );
			glRotatef( later->pos->angleX, 1.0f, 0.0f, 0.0f );
			glRotatef( later->pos->angleY, 0.0f, 1.0f, 0.0f );
			glRotatef( later->pos->angleZ, 0.0f, 0.0f, 1.0f );
		}

		if ( later && later->creature ) {
			glTranslatef( later->creature->getOffsetX(), later->creature->getOffsetY(), later->creature->getOffsetZ() );
		}

#ifdef DEBUG_SECRET_DOORS
		if ( later && later->pos ) {
			int xp = later->pos->x;
			int yp = later->pos->y;
			int index = xp + MAP_WIDTH * yp;
			if ( secretDoors.find( index ) != secretDoors.end() ) {
				glColor4f( 1.0f, 0.3f, 0.3f, 1.0f );
				colorAlreadySet = true;
			}
		}
#endif

		// show detected secret doors
		if ( later && later->pos ) {
			if ( isSecretDoor( later->pos ) && ( isSecretDoorDetected( later->pos ) || settings->isGridShowing() ) ) {
				glColor4f( 0.3f, 0.7f, 0.3f, 1.0f );
				colorAlreadySet = true;
			}
		}

		if ( colorAlreadySet   ) {
			colorAlreadySet = false;
		} else {
			if ( later && later->pos && isLocked( later->pos->x, later->pos->y, later->pos->z ) ) {
				glColor4f( 1.0f, 0.3f, 0.3f, 1.0f );
			} else {
				//glColor4f(0.72f, 0.65f, 0.55f, 0.5f);
				glColor4f( 1.0f, 1.0f, 1.0f, 0.9f );
			}
		}
	}

	glDisable( GL_CULL_FACE );

	if ( shape ) {
		( ( GLShape* )shape )->setCameraRot( xrot, yrot, zrot );
		( ( GLShape* )shape )->setCameraPos( xpos, ypos, zpos, xpos2, ypos2, zpos2 );
		if ( later && later->pos ) ( ( GLShape* )shape )->setLocked( isLocked( later->pos->x, later->pos->y, 0 ) );
		else ( ( GLShape* )shape )->setLocked( false );
	}
	if ( effect && later ) {
		if ( later->creature ) {
			// translate hack for md2 models... see: md2shape::draw()
			//glTranslatef( 0, -1 * MUL, 0 );
			later->creature->getEffect()->draw( later->creature->getEffectType(),
			                                    later->creature->getDamageEffect() );
			//glTranslatef( 0, 1 * MUL, 0 );
		} else if ( later->effect ) {
			later->effect->getEffect()->draw( later->effect->getEffectType(),
			                                  later->effect->getDamageEffect() );
		}
	} else if ( later && later->creature && !useShadow ) {
		if ( later->creature->getStateMod( StateMod::invisible ) ) {
			glColor4f( 0.3f, 0.8f, 1.0f, 0.5f );
			glEnable( GL_BLEND );
			//glDepthMask( GL_FALSE );
			glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );
		} else if ( later->creature->getStateMod( StateMod::possessed ) ) {
			glColor4f( 1.0, 0.3f, 0.8f, 1.0f );
		}

		// outline mission creatures
		if ( adapter->isMissionCreature( later->creature ) ) {
			//if( session->getCurrentMission() &&
			//session->getCurrentMission()->isMissionCreature( later->creature ) ) {
			shape->outline( 0.15f, 0.15f, 0.4f );
		} else if ( later->creature->isBoss() ) {
			shape->outline( 0.4f, 0.15f, 0.4f );
		} else if ( later && later->pos &&
		            later->pos->outlineColor ) {
			shape->outline( later->pos->outlineColor );
		}
		shape->draw();

		if ( later->creature->getStateMod( StateMod::invisible ) ) {
			glDisable( GL_BLEND );
			//glDepthMask( GL_TRUE );
		}

	} else if ( later && later->item && !useShadow ) {

		if ( later->item->isSpecial() ) {
			shape->outline( Constants::SPECIAL_ITEM_COLOR );
		} else if ( later->item->isMagicItem() ) {
			shape->outline( Constants::MAGIC_ITEM_COLOR[ later->item->getMagicLevel() ] );
		} else if ( later->item->getContainsMagicItem() ) {
			shape->outline( 0.8f, 0.8f, 0.3f );
		}

		if ( later && later->pos && later->pos->outlineColor && !useShadow )
			shape->outline( later->pos->outlineColor );
		shape->draw();



	} else {
		if ( later ) {
			bool sides[6];
			findOccludedSides( later, sides );
			shape->setOccludedSides( sides );
		}
		if ( later && later->pos ) {
			if ( later->pos->outlineColor && !useShadow ) {
				shape->outline( later->pos->outlineColor );
			}
			if ( shape->getTextureCount() > 3 ) {
				// select which alternate texture to use
				shape->setTextureIndex( later->pos->texIndex );
			}
		}
		shape->draw();
	}

#if DEBUG_MOUSE_POS == 1
	if ( shape && !useShadow && ( ( later && later->item ) || ( later && later->creature ) || shape->isInteractive() ) ) {
		glDisable( GL_DEPTH_TEST );
		glDepthMask( GL_FALSE );
		glDisable( GL_CULL_FACE );
		glDisable( GL_TEXTURE_2D );
		glColor4f( 1, 1, 1, 1 );
		glBegin( GL_LINE_LOOP );
		glVertex3f( 0, 0, 0 );
		glVertex3f( 0, shape->getDepth() * MUL, 0 );
		glVertex3f( shape->getWidth() * MUL, shape->getDepth() * MUL, 0 );
		glVertex3f( shape->getWidth() * MUL, 0, 0 );
		glEnd();
		glBegin( GL_LINE_LOOP );
		glVertex3f( 0, 0, shape->getHeight() * MUL );
		glVertex3f( 0, shape->getDepth() * MUL, shape->getHeight() * MUL );
		glVertex3f( shape->getWidth() * MUL, shape->getDepth() * MUL, shape->getHeight() * MUL );
		glVertex3f( shape->getWidth() * MUL, 0, shape->getHeight() * MUL );
		glEnd();
		glBegin( GL_LINES );
		glVertex3f( 0, 0, 0 );
		glVertex3f( 0, 0, shape->getHeight() * MUL );
		glEnd();
		glBegin( GL_LINES );
		glVertex3f( 0, shape->getDepth() * MUL, 0 );
		glVertex3f( 0, shape->getDepth() * MUL, shape->getHeight() * MUL );
		glEnd();
		glBegin( GL_LINES );
		glVertex3f( shape->getWidth() * MUL, shape->getDepth() * MUL, 0 );
		glVertex3f( shape->getWidth() * MUL, shape->getDepth() * MUL, shape->getHeight() * MUL );
		glEnd();
		glBegin( GL_LINES );
		glVertex3f( shape->getWidth() * MUL, 0, 0 );
		glVertex3f( shape->getWidth() * MUL, 0, shape->getHeight() * MUL );
		glEnd();
		glDepthMask( GL_TRUE );
		glEnable( GL_DEPTH_TEST );
	}
#endif

	// in the map editor outline virtual shapes
	if ( shape->isVirtual() && settings->isGridShowing() && gridEnabled ) {

		if ( heightPos > 1 ) {
			cerr << "heightPos=" << heightPos << " for virtual shape " << shape->getName() << endl;
		}
		if ( later && later->pos && later->pos->z > 0 ) {
			cerr << "z=" << later->pos->z << " for virtual shape " << shape->getName() << endl;
		}

		glColor4f( 0.75f, 0.75f, 1.0f, 1.0f );

		float z = ( shape->getHeight() + 0.25f ) * MUL;
		float lowZ = 0.25f * MUL;

		float wm = shape->getWidth() * MUL;
		float dm = shape->getDepth() * MUL;

		glPushMatrix();
		glTranslatef( 0.0f, 20.0f, z );
		adapter->texPrint( 0, 0, "virtual" );
		glPopMatrix();

		glDisable( GL_TEXTURE_2D );
		glBegin( GL_LINE_LOOP );
		glVertex3f( 0.0f, 0.0f, z );
		glVertex3f( 0.0f, dm, z );
		glVertex3f( wm, dm, z );
		glVertex3f( wm, 0.0f, z );

		glVertex3f( 0.0f, 0.0f, z );
		glVertex3f( 0.0f, 0.0f, lowZ );
		glVertex3f( wm, 0.0f, lowZ );
		glVertex3f( wm, 0.0f, z );

		glVertex3f( 0.0f, dm, z );
		glVertex3f( 0.0f, dm, lowZ );
		glVertex3f( wm, dm, lowZ );
		glVertex3f( wm, dm, z );

		glVertex3f( 0.0f, 0.0f, z );
		glVertex3f( 0.0f, 0.0f, lowZ );
		glVertex3f( 0.0f, dm, lowZ );
		glVertex3f( 0.0f, dm, z );

		glVertex3f( wm, 0.0f, z );
		glVertex3f( wm, 0.0f, lowZ );
		glVertex3f( wm, dm, lowZ );
		glVertex3f( wm, dm, z );

		glEnd();
		glEnable( GL_TEXTURE_2D );
	}

	glPopMatrix();

	// slow on mac os X
	// glPopAttrib();

	if ( shape )
		( ( GLShape* )shape )->useShadow = false;
}

/// Determines which sides of a shape are not visible for various reasons.

void Map::findOccludedSides( DrawLater *later, bool *sides ) {
	if ( colorAlreadySet || !later || !later->pos || !later->shape || !later->shape->isStencil() || ( later->shape && isDoor( later->shape ) ) ) {
		sides[Shape::BOTTOM_SIDE] = sides[Shape::N_SIDE] = sides[Shape::S_SIDE] =
		                                                     sides[Shape::E_SIDE] = sides[Shape::W_SIDE] = sides[Shape::TOP_SIDE] = true;
		return;
	}

	sides[Shape::BOTTOM_SIDE] = sides[Shape::N_SIDE] =
	                              sides[Shape::S_SIDE] = sides[Shape::E_SIDE] =
	                                                       sides[Shape::W_SIDE] = sides[Shape::TOP_SIDE] = false;

	int x, y;
	Location *pos;
	for ( x = later->pos->x; x < later->pos->x + later->pos->shape->getWidth(); x++ ) {
		y = later->y - later->pos->shape->getDepth();
		pos = getLocation( x, y, later->pos->z );
		if ( !pos || !pos->shape->isStencil() || ( !isLocationInLight( x, y, pos->shape ) && !isDoorType( pos->shape ) ) ) {
			sides[Shape::N_SIDE] = true;
		}

		y = later->y + 1;
		pos = getLocation( x, y, later->pos->z );
		if ( !pos || !pos->shape->isStencil() || ( !isLocationInLight( x, y, pos->shape ) && !isDoorType( pos->shape ) ) ) {
			sides[Shape::S_SIDE] = true;
		}

		if ( sides[Shape::N_SIDE] && sides[Shape::S_SIDE] ) {
			break;
		}
	}


	for ( y = later->pos->y - later->pos->shape->getDepth() + 1; y <= later->pos->y; y++ ) {
		x = later->x - 1;
		pos = getLocation( x, y, later->pos->z );
		if ( !pos || !pos->shape->isStencil() || ( !isLocationInLight( x, y, pos->shape ) && !isDoorType( pos->shape ) ) ) {
			sides[Shape::W_SIDE] = true;
		}

		x = later->x + later->pos->shape->getWidth();
		pos = getLocation( x, y, later->pos->z );
		if ( !pos || !pos->shape->isStencil() || ( !isLocationInLight( x, y, pos->shape ) && !isDoorType( pos->shape ) ) ) {
			sides[Shape::E_SIDE] = true;
		}

		if ( sides[Shape::W_SIDE] && sides[Shape::E_SIDE] ) {
			break;
		}
	}


	for ( x = later->pos->x; x < later->pos->x + later->pos->shape->getWidth(); x++ ) {
		for ( y = later->pos->y - later->pos->shape->getDepth() + 1; !sides[Shape::TOP_SIDE] && y <= later->pos->y; y++ ) {
			pos = getLocation( x, y, later->pos->z + later->pos->shape->getHeight() );
			if ( !pos || !pos->shape->isStencil() || ( !isLocationInLight( x, y, pos->shape ) && !isDoorType( pos->shape ) ) ) {
				sides[Shape::TOP_SIDE] = true;
				break;
			}
		}
	}
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
	if ( x < MAP_OFFSET || y < MAP_OFFSET || x >= MAP_WIDTH - MAP_OFFSET || y >= MAP_DEPTH - MAP_OFFSET ) {
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
	if ( x < MAP_OFFSET || y < MAP_OFFSET || z < 0 ||
	        x >= MAP_WIDTH - MAP_OFFSET || y >= MAP_DEPTH - MAP_OFFSET || z >= MAP_VIEW_HEIGHT ) {
		cerr << "*** Error can't set position outside bounds:" << x << "," << y << "," << z << endl;
		//((RenderedCreature*)NULL)->getName();
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
				for ( int zp = 0; zp < creature->getShape()->getHeight(); zp++ ) {
					int oldX = ox + xp;
					int oldY = oy - yp;
					int oldZ = oz + zp;
					tmp[xp][yp][zp] = pos[oldX][oldY][oldZ];
					tmp[xp][yp][zp]->outlineColor = NULL;
					pos[oldX][oldY][oldZ] = NULL;
					if ( !( tmp[xp][yp][zp] ) )
						cerr << "*** tmp is null!" << endl;
				}
			}
		}

		if ( helper && creature->isPartyMember() ) helper->visit( creature );

		// pick up any items in the way
		for ( int xp = 0; xp < creature->getShape()->getWidth(); xp++ ) {
			for ( int yp = 0; yp < creature->getShape()->getDepth(); yp++ ) {
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
		DrawLater *later = creatureMap[creature];
		if ( later ) {
			if ( later->creature != creature ) {
				cerr << "*** Error: creatureMap is damaged!!! creature=" << creature->getName() << endl;
				cerr << "\tlocation: shape=" << ( later->shape ? later->shape->getName() : "null" ) <<
				" item=" << ( later->item ? later->item->getItemName() : "null" ) <<
				" creature=" << ( later->creature ? later->creature->getName() : "null" ) <<
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
				DrawLater *effect = creatureEffectMap[creature];
				if ( effect ) {
					effect->xpos = xpos2;
					effect->ypos = ypos2;
					effect->zpos = zpos2;
					effect->x = posX;
					effect->y = posY;
				}
			}
		} else if ( helper && helper->isVisible( toint( creature->getX() ), toint( creature->getY() ), creature->getShape() ) ) {
			Location *pos = getLocation( toint( creature->getX() ), toint( creature->getY() ), toint( creature->getZ() ) );
			if ( pos ) {
				int chunkX, chunkY;
				getChunk( pos->x, pos->y, &chunkX, &chunkY );
				if ( checkLightMap( chunkX, chunkY ) ) {
					// If this creature has no creatureMap entry but should now be visible,
					// it means the creature wondered into the visible area: repaint everything.
					// An optimization here would be to restrict setupShapes to the creatures that changed.
					resortShapes = mapChanged = true;
//     cerr << "!!! " << creature->getName() << " " << SDL_GetTicks() << endl;
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

/// Is there a wall between the two given shapes?

// FIXME: only uses x,y for now
// return false if there is any hole in the walls
bool Map::isWallBetweenShapes( int x1, int y1, int z1, Shape *shape1, int x2, int y2, int z2, Shape *shape2 ) {
	for ( int x = x1; x < x1 + shape1->getWidth(); x++ ) {
		for ( int y = y1; y < y1 + shape1->getDepth(); y++ ) {
			for ( int xx = x2; xx < x2 + shape2->getWidth(); xx++ ) {
				for ( int yy = y2; yy < y2 + shape2->getDepth(); yy++ ) {
					Shape *shape = isWallBetween( x, y, z1, xx, yy, z2 );
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
Shape *Map::isWallBetween( int x1, int y1, int z1, int x2, int y2, int z2 ) {

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
		if ( ss ) {
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

/// It draws... a cube. Unused.

void Map::drawCube( float x, float y, float z, float r ) {
	glBegin( GL_QUADS );
	// front
//  glNormal3f(0.0f, 0.0f, 1.0f);
	glVertex3f( -r + x, -r + y, r + z );
	glVertex3f( r + x, -r + y, r + z );
	glVertex3f( r + x, r + y, r + z );
	glVertex3f( -r + x, r + y, r + z );

	// back
//  glNormal3f(0.0f, 0.0f, -1.0f);
	glVertex3f( r + x, -r + y, -r + z );
	glVertex3f( -r + x, -r + y, -r + z );
	glVertex3f( -r + x, r + y, -r + z );
	glVertex3f( r + x, r + y, -r + z );

	// top
//  glNormal3f(0.0f, 1.0f, 0.0f);
	glVertex3f( -r + x, r + y, r + z );
	glVertex3f( r + x, r + y, r + z );
	glVertex3f( r + x, r + y, -r + z );
	glVertex3f( -r + x, r + y, -r + z );

	// bottom
//  glNormal3f(0.0f, -1.0f, 0.0f);
	glVertex3f( -r + x, -r + y, -r + z );
	glVertex3f( r + x, -r + y, -r + z );
	glVertex3f( r + x, -r + y, r + z );
	glVertex3f( -r + x, -r + y, r + z );

	// left
//  glNormal3f(-1.0f, 0.0f, 0.0f);
	glVertex3f( -r + x, -r + y, -r + z );
	glVertex3f( -r + x, -r + y, r + z );
	glVertex3f( -r + x, r + y, r + z );
	glVertex3f( -r + x, r + y, -r + z );

	// right
//  glNormal3f(1.0f, 0.0f, 0.0f);
	glVertex3f( r + x, -r + y, r + z );
	glVertex3f( r + x, -r + y, -r + z );
	glVertex3f( r + x, r + y, -r + z );
	glVertex3f( r + x, r + y, r + z );

	glEnd();

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



GameMapSettings::GameMapSettings() {
}

GameMapSettings::~GameMapSettings() {
}

/// Does it have a "light map" (stores which parts of the level are visible)?

bool GameMapSettings::isLightMapEnabled() {
	return true;
}

bool GameMapSettings::isGridShowing() {
	return false;
}

bool GameMapSettings::isPlayerEnabled() {
	return true;
}

bool GameMapSettings::isItemPosEnabled() {
	return true;
}

/// How far can the camera zoom in?

float GameMapSettings::getMinZoomIn() {
	return 0.85f;
}

/// How far can the camera zoom out?

float GameMapSettings::getMaxZoomOut() {
	return 2.8f;
}

/// How "high" can the camera be (degrees)?

float GameMapSettings::getMaxYRot() {
	return 55.0f;
}





EditorMapSettings::EditorMapSettings() {
}

EditorMapSettings::~EditorMapSettings() {
}

bool EditorMapSettings::isLightMapEnabled() {
	return false;
}

bool EditorMapSettings::isGridShowing() {
	return true;
}

bool EditorMapSettings::isPlayerEnabled() {
	return false;
}

bool EditorMapSettings::isItemPosEnabled() {
	return false;
}

float EditorMapSettings::getMinZoomIn() {
	return 0.05f;
}

float EditorMapSettings::getMaxZoomOut() {
	return 2.8f;
}

float EditorMapSettings::getMaxYRot() {
	return 90.0f;
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
				if ( pos[x][y][z] && pos[x][y][z]->x == x && pos[x][y][z]->y == y && pos[x][y][z]->z == z && !( pos[x][y][z]->shape && pos[x][y][z]->shape->isVirtual() && !( ( VirtualShape* )pos[x][y][z]->shape )->isDrawn() ) ) {

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
					char *p = adapter->getMagicSchoolIndexForLocation( pos[x][y][z] );
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
					OutdoorTextureInfo *oti = ( OutdoorTextureInfo* )malloc( sizeof( OutdoorTextureInfo ) );
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
		initOutdoorsGroundTexture();
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

/**
  Maxsize of 0 means unlimited size cache.
*/
MapMemoryManager::MapMemoryManager( int maxSize ) {
	this->maxSize = maxSize;
	this->accessCount = 0;
	this->usedCount = 0;
	// make room for at least this many pointers
	unused.reserve( 200000 );

	this->usedEffectCount = 0;
	unusedEffect.reserve( 50000 );
}

MapMemoryManager::~MapMemoryManager() {
	for ( int i = 0; i < static_cast<int>( unused.size() ); i++ ) {
		delete unused[i];
	}
	unused.clear();
	for ( int i = 0; i < static_cast<int>( unusedEffect.size() ); i++ ) {
		delete unusedEffect[i];
	}
	unusedEffect.clear();
}

/// Creates a new, empty map location. Reuses an unused location if possible.

Location *MapMemoryManager::newLocation() {
	Location *pos;
	if ( !unused.empty() ) {
		pos = unused[ unused.size() - 1 ];
		unused.pop_back();
	} else {
		pos = new Location();
	}
	usedCount++;

	printStatus();

	// reset it
	pos->x = pos->y = pos->z = 0;
	pos->heightPos = 0;
	pos->shape = NULL;
	pos->item = NULL;
	pos->creature = NULL;
	pos->outlineColor = NULL;
	pos->texIndex = NULL;

	return pos;
}

/// Creates a new, empty effect location. Reuses an unused location if possible.

EffectLocation *MapMemoryManager::newEffectLocation( Map *theMap, Preferences *preferences, Shapes *shapes, int width, int height ) {
	EffectLocation *pos;
	if ( !unusedEffect.empty() ) {
		pos = unusedEffect[ unusedEffect.size() - 1 ];
		unusedEffect.pop_back();
		pos->effect->reset();
	} else {
		pos = new EffectLocation;
		//pos->effect = new Effect( theMap, preferences, shapes, 4, 4 );
		pos->effect = new Effect( theMap, preferences, shapes, width, height );
		pos->effect->deleteParticles();
	}
	usedEffectCount++;

	printStatus();

	// reset it
	pos->x = pos->y = pos->z = 0;
	pos->effectDuration = 0;
	pos->damageEffectCounter = 0;
	pos->effectType = 0;
	pos->effectDelay = 0;
	pos->heightPos = 0;

	return pos;
}

/// Deletes a map location and adds it to the "unused" array.

void MapMemoryManager::deleteLocation( Location *pos ) {
	if ( !maxSize || static_cast<int>( unused.size() ) < maxSize ) {
		unused.push_back( pos );
	} else {
		delete pos;
	}
	usedCount--;
	printStatus();
}

/// Deletes the special effect at a location and adds it to the "unused" array.

void MapMemoryManager::deleteEffectLocation( EffectLocation *pos ) {
	if ( !maxSize || static_cast<int>( unusedEffect.size() ) < maxSize ) {
		unusedEffect.push_back( pos );
	} else {
		delete pos;
	}
	usedEffectCount--;
	printStatus();
}

/// Outputs memory status to stderr.

void MapMemoryManager::printStatus() {
	if ( ++accessCount > 5000 ) {
		cerr << "Map size: " << usedCount << " Kb:" << ( static_cast<float>( sizeof( Location )*usedCount ) / 1024.0f ) <<
		" Cache: " << unused.size() << " Kb:" << ( static_cast<float>( sizeof( Location )*unused.size() ) / 1024.0f ) << endl;
		cerr << "Effect size: " << usedEffectCount << " Kb:" << ( static_cast<float>( sizeof( EffectLocation )*usedEffectCount ) / 1024.0f ) <<
		" Cache: " << unusedEffect.size() << " Kb:" << ( static_cast<float>( sizeof( EffectLocation )*unusedEffect.size() ) / 1024.0f ) << endl;
		accessCount = 0;
	}
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

/// Draws the floor/ground of the map.

void Map::renderFloor() {
	glEnable( GL_TEXTURE_2D );
	glColor4f( 1.0f, 1.0f, 1.0f, 0.9f );
	if ( floorTex.isSpecified() ) floorTex.glBind();
	glPushMatrix();
	if ( isHeightMapEnabled() ) {
		if ( groundVisible || settings->isGridShowing() ) {
			drawHeightMapFloor();
			drawWaterLevel();
		}
		setupShapes( true, false );
	} else {
		if ( settings->isGridShowing() ) {
			glDisable( GL_TEXTURE_2D );
			glColor4f( 0.0f, 0.0f, 0.0f, 0.0f );
		}
		drawFlatFloor();
	}
	glPopMatrix();

	// show floor in map editor
	if ( settings->isGridShowing() ) {
		setupShapes( true, false );
	}
}

/// Draws the ground on outdoor maps.

bool Map::drawHeightMapFloor() {
	CVectorTex *p[4];
	float gx, gy;

	bool ret = true;

	glDisable( GL_CULL_FACE );
	glEnable( GL_TEXTURE_2D );

	int startX = ( getX() / OUTDOORS_STEP );
	int startY = ( getY() / OUTDOORS_STEP );
	int endX = ( ( getX() + mapViewWidth ) / OUTDOORS_STEP ) - 1;
	int endY = ( ( getY() + mapViewDepth ) / OUTDOORS_STEP ) - 1;

	for ( int yy = startY; yy < endY; yy++ ) {
		for ( int xx = startX; xx < endX; xx++ ) {

			//int chunkX = ( ( xx * OUTDOORS_STEP ) - MAP_OFFSET ) / MAP_UNIT;
			//int chunkY = ( ( ( yy + 1 ) * OUTDOORS_STEP ) - ( MAP_OFFSET + 1 ) ) / MAP_UNIT;
			//if( lightMap[chunkX][chunkY] ) {
			groundPos[ xx ][ yy ].tex.glBind();
			//} else {
			//glDisable( GL_TEXTURE_2D );
			//}

			p[0] = &( groundPos[ xx ][ yy ] );
			p[1] = &( groundPos[ xx + 1 ][ yy ] );
			p[2] = &( groundPos[ xx ][ yy + 1 ] );
			p[3] = &( groundPos[ xx + 1 ][ yy + 1 ] );
			glBegin( GL_TRIANGLE_STRIP );
			for ( int i = 0; i < 4; i++ ) {
				//if( lightMap[chunkX][chunkY] ) {
				glTexCoord2f( p[i]->u, p[i]->v );
				glColor4f( p[i]->r, p[i]->g, p[i]->b, p[i]->a );
				//} else {
				//glColor4f( 0, 0, 0, 0 );
				//}
				gx = p[i]->x - getX() * MUL;
				gy = p[i]->y - getY() * MUL;
				glVertex3f( gx, gy, p[i]->z );
			}
			glEnd();
		}
	}

	//glEnable( GL_DEPTH_TEST );
	glDepthMask( GL_FALSE );
	glEnable( GL_BLEND );
	glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );

	// draw outdoor textures
	//cerr << "from: " << getX() << "," << getY() << " to: " << ( getX() + mapViewWidth ) << "," << ( getY() + mapViewDepth ) << endl;
	for ( int z = 0; z < MAX_OUTDOOR_LAYER; z++ ) {
		for ( int yy = startY; yy < endY; yy++ ) {
			for ( int xx = startX; xx < endX; xx++ ) {
				if ( outdoorTex[xx][yy][z].texture.isSpecified() ) {
					drawOutdoorTex( outdoorTex[xx][yy][z].texture, xx + outdoorTex[xx][yy][z].offsetX, yy + outdoorTex[xx][yy][z].offsetY, outdoorTex[xx][yy][z].width, outdoorTex[xx][yy][z].height, outdoorTex[xx][yy][z].angle );
				}
			}
		}
	}

	//glDisable( GL_DEPTH_TEST );
	glDepthMask( GL_TRUE );
	glDisable( GL_BLEND );

	if ( DEBUG_MOUSE_POS || ( settings->isGridShowing() && gridEnabled ) ) {
		//glDisable( GL_DEPTH_TEST );
		glEnable( GL_BLEND );
		glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );
		glDisable( GL_TEXTURE_2D );
		glColor4f( 0.4f, 0.4f, 0.4f, 0.3f );
		for ( int yy = startY; yy < endY; yy++ ) {
			for ( int xx = startX; xx < endX; xx++ ) {

				p[0] = &( groundPos[ xx ][ yy + 1 ] );
				p[1] = &( groundPos[ xx ][ yy ] );
				p[2] = &( groundPos[ xx + 1 ][ yy ] );
				p[3] = &( groundPos[ xx + 1 ][ yy + 1 ] );
				glBegin( GL_LINE_LOOP );
				for ( int i = 0; i < 4; i++ ) {
					gx = p[i]->x - getX() * MUL;
					gy = p[i]->y - getY() * MUL;
					glVertex3f( gx, gy, p[i]->z + 0.05f * MUL );
				}
				glEnd();
			}
		}
		glEnable( GL_TEXTURE_2D );
		//glEnable( GL_DEPTH_TEST );
		glDisable( GL_BLEND );
	}

	return ret;
}

/// Draws a ground texture on outdoor maps. Uses OUTDOORS_STEP coordinates.

void Map::drawOutdoorTex( Texture tex, float tx, float ty, float tw, float th, float angle ) {
	tex.glBind();

	glMatrixMode( GL_TEXTURE );
	glPushMatrix();
	glLoadIdentity();

	glTranslatef( 0.5f, 0.5f, 0.0f );
	glRotatef( angle, 0.0f, 0.0f, 1.0f );
	glTranslatef( -0.5f, -0.5f, 0.0f );

	//glTranslatef( offSX, offSY, 0 );
	glMatrixMode( GL_MODELVIEW );

	int sx = tx;
	int sy = ty;
	int ex = tx + tw;
	if ( ex == sx ) ex++;
	int ey = ty + th;
	if ( ey == sy ) ey++;

	int DIFF_Z = 0.01f * MUL;
	CVectorTex *p;
	for ( int xx = sx; xx < ex; xx++ ) {
		for ( int yy = sy; yy < ey; yy++ ) {

			float texSx = ( ( xx - sx ) ) / ( float )( ex - sx );
			float texEx = ( ( xx + 1 - sx ) ) / ( float )( ex - sx );
			float texSy = ( ( yy - sy ) ) / ( float )( ey - sy );
			float texEy = ( ( yy + 1 - sy ) ) / ( float )( ey - sy );

			//glBegin( GL_LINE_LOOP );
			glBegin( GL_TRIANGLE_STRIP );

			p = &groundPos[ xx ][ yy ];
			glColor4f( p->r, p->g, p->b, p->a );
			glTexCoord2f( texSx, texSy );
			glVertex3f( p->x - getX() * MUL, p->y - getY() * MUL, p->z + DIFF_Z );

			p = &groundPos[ xx + 1 ][ yy ];
			glColor4f( p->r, p->g, p->b, p->a );
			glTexCoord2f( texEx, texSy );
			glVertex3f( p->x - getX() * MUL, p->y - getY() * MUL, p->z + DIFF_Z );

			p = &groundPos[ xx ][ yy + 1 ];
			glColor4f( p->r, p->g, p->b, p->a );
			glTexCoord2f( texSx, texEy );
			glVertex3f( p->x - getX() * MUL, p->y - getY() * MUL, p->z + DIFF_Z );

			p = &groundPos[ xx + 1 ][ yy + 1 ];
			glColor4f( p->r, p->g, p->b, p->a );
			glTexCoord2f( texEx, texEy );
			glVertex3f( p->x - getX() * MUL, p->y - getY() * MUL, p->z + DIFF_Z );

			glEnd();
		}
	}

	glMatrixMode( GL_TEXTURE );
	glPopMatrix();
	glMatrixMode( GL_MODELVIEW );

#ifdef DEBUG_HEIGHT_MAP
	debugGround( sx, sy, ex, ey );
#endif
}

/// Draws a ground texture on outdoor maps. Uses map coordinates.

/// Draw a texture on top of the ground map. This is useful for drawing shadows or
/// selection circles on top of un-even terrain.

#define GROUND_TEX_Z_OFFSET 0.26f

void Map::drawGroundTex( Texture tex, float tx, float ty, float tw, float th, float angle ) {

	//glEnable( GL_DEPTH_TEST );
	glDepthMask( GL_FALSE );
	glEnable( GL_BLEND );
	glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );
	glDisable( GL_CULL_FACE );
	glEnable( GL_TEXTURE_2D );
	tex.glBind();

	//glColor4f( 1, 0, 0, 1 );
	//glDepthMask( GL_FALSE );
	//glDisable( GL_DEPTH_TEST );

	// which ground pos?
	float sx = ( tx / static_cast<float>( OUTDOORS_STEP ) );
	float sy = ( ( ty - th - 1 ) / static_cast<float>( OUTDOORS_STEP ) );
	float ex = ( ( tx + tw ) / static_cast<float>( OUTDOORS_STEP ) );
	float ey = ( ( ty - 1 ) / static_cast<float>( OUTDOORS_STEP ) );
#ifdef DEBUG_RENDER
	cerr << "s=" << sx << "," << sy << " e=" << ex << "," << ey << endl;
#endif

	// offset to our texture inside the ground pos
	float offSX = tx - ( sx * OUTDOORS_STEP );
	float offSY = ( ty - th - 1 ) - ( sy * OUTDOORS_STEP );
	float offEX = offSX + tw;
	float offEY = offSY + th;

#ifdef DEBUG_RENDER
	cerr << "tex size=" << ( ( ex - sx ) * OUTDOORS_STEP ) << "," << ( ( ey - sy ) * OUTDOORS_STEP ) << " player size=" << tw << endl;
	cerr << "tex=" << ( sx * OUTDOORS_STEP ) << "," << ( sy * OUTDOORS_STEP ) << " player=" << adapter->getPlayer()->getX() << "," << adapter->getPlayer()->getY() << endl;
	cerr << "offs: " << offSX << "," << offSY << " " << offEX << "," << offEY << endl;
#endif

	// converted to texture coordinates ( 0-1 )
	offSX = -offSX / ( ( ex - sx ) * OUTDOORS_STEP );
	offSY = -offSY / ( ( ey - sy ) * OUTDOORS_STEP );
	offEX = 1 - ( offEX / ( ( ex - sx ) * OUTDOORS_STEP ) ) + 1;
	offEY = 1 - ( offEY / ( ( ey - sy ) * OUTDOORS_STEP ) ) + 1;
#ifdef DEBUG_RENDER
	cerr << "\toffs: " << offSX << "," << offSY << " " << offEX << "," << offEY << endl;
#endif

	// don't repeat the texture
	glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP );
	glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP );

	glMatrixMode( GL_TEXTURE );
	glPushMatrix();
	glLoadIdentity();

	glTranslatef( 0.5f, 0.5f, 0.0f );
	glRotatef( angle, 0.0f, 0.0f, 1.0f );
	glTranslatef( -0.5f, -0.5f, 0.0f );

	glTranslatef( offSX, offSY, 0.0f );
	glMatrixMode( GL_MODELVIEW );


	float gx, gy;
	for ( int xx = static_cast<int>( sx ); xx <= static_cast<int>( ex ); xx++ ) {
		for ( int yy = static_cast<int>( sy ); yy <= static_cast<int>( ey ); yy++ ) {

			float texSx = ( ( xx - sx ) * ( offEX - offSX ) ) / ( ex - sx );
			float texEx = ( ( xx + 1 - sx ) * ( offEX - offSX ) ) / ( ex - sx );
			float texSy = ( ( yy - sy ) * ( offEY - offSY ) ) / ( ey - sy );
			float texEy = ( ( yy + 1 - sy ) * ( offEY - offSY ) ) / ( ey - sy );

			//glBegin( GL_LINE_LOOP );
			glBegin( GL_TRIANGLE_STRIP );

			glTexCoord2f( texSx, texSy );
			//glColor4f( 1, 0, 0, 1 );
			gx = groundPos[ xx ][ yy ].x - getX() * MUL;
			gy = groundPos[ xx ][ yy ].y - getY() * MUL;
			glVertex3f( gx, gy, groundPos[ xx ][ yy ].z + GROUND_TEX_Z_OFFSET * MUL );

			glTexCoord2f( texEx, texSy );
			//glColor4f( 1, 1, 1, 1 );
			gx = groundPos[ xx + 1 ][ yy ].x - getX() * MUL;
			gy = groundPos[ xx + 1 ][ yy ].y - getY() * MUL;
			glVertex3f( gx, gy, groundPos[ xx + 1 ][ yy ].z + GROUND_TEX_Z_OFFSET * MUL );

			glTexCoord2f( texSx, texEy );
			//glColor4f( 1, 1, 1, 1 );
			gx = groundPos[ xx ][ yy + 1 ].x - getX() * MUL;
			gy = groundPos[ xx ][ yy + 1 ].y - getY() * MUL;
			glVertex3f( gx, gy, groundPos[ xx ][ yy + 1 ].z + GROUND_TEX_Z_OFFSET * MUL );

			glTexCoord2f( texEx, texEy );
			//glColor4f( 1, 1, 1, 1 );
			gx = groundPos[ xx + 1 ][ yy + 1 ].x - getX() * MUL;
			gy = groundPos[ xx + 1 ][ yy + 1 ].y - getY() * MUL;
			glVertex3f( gx, gy, groundPos[ xx + 1 ][ yy + 1 ].z + GROUND_TEX_Z_OFFSET * MUL );

			glEnd();
		}
	}

	glMatrixMode( GL_TEXTURE );
	glPopMatrix();
	glMatrixMode( GL_MODELVIEW );


	// switch back to repeating the texture
	glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT );
	glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT );

	//glDisable( GL_DEPTH_TEST );
	glDepthMask( GL_TRUE );
	glDisable( GL_BLEND );

#ifdef DEBUG_HEIGHT_MAP
	debugGround( sx, sy, ex, ey );
#endif
}

void Map::debugGround( int sx, int sy, int ex, int ey ) {
	glDisable( GL_TEXTURE_2D );
	glColor4f( 0.0f, 1.0f, 0.0f, 1.0f );
	float gx, gy;
	for ( int xx = sx; xx <= ex; xx++ ) {
		for ( int yy = sy; yy <= ey; yy++ ) {
			glBegin( GL_LINE_LOOP );
			gx = groundPos[ xx ][ yy + 1 ].x - getX() * MUL;
			gy = groundPos[ xx ][ yy + 1 ].y - getY() * MUL;
			glVertex3f( gx, gy, groundPos[ xx ][ yy + 1 ].z + GROUND_TEX_Z_OFFSET * MUL );

			gx = groundPos[ xx ][ yy ].x - getX() * MUL;
			gy = groundPos[ xx ][ yy ].y - getY() * MUL;
			glVertex3f( gx, gy, groundPos[ xx ][ yy ].z + GROUND_TEX_Z_OFFSET * MUL );

			gx = groundPos[ xx + 1 ][ yy ].x - getX() * MUL;
			gy = groundPos[ xx + 1 ][ yy ].y - getY() * MUL;
			glVertex3f( gx, gy, groundPos[ xx + 1 ][ yy ].z + GROUND_TEX_Z_OFFSET * MUL );

			gx = groundPos[ xx + 1 ][ yy + 1 ].x - getX() * MUL;
			gy = groundPos[ xx + 1 ][ yy + 1 ].y - getY() * MUL;
			glVertex3f( gx, gy, groundPos[ xx + 1 ][ yy + 1 ].z + GROUND_TEX_Z_OFFSET * MUL );

			glEnd();
		}
	}
	glEnable( GL_TEXTURE_2D );
}

/// Sets up the outdoor ground heightfield including texturing and lighting.

void Map::createGroundMap() {
	float w, d, h;
	for ( int xx = 0; xx < MAP_TILES_X; xx++ ) {
		for ( int yy = 0; yy < MAP_TILES_Y; yy++ ) {
			w = static_cast<float>( xx * OUTDOORS_STEP ) * MUL;
			d = static_cast<float>( yy * OUTDOORS_STEP - 1 ) * MUL;
			h = ( ground[ xx ][ yy ] ) * MUL;

			groundPos[ xx ][ yy ].x = w;
			groundPos[ xx ][ yy ].y = d;
			groundPos[ xx ][ yy ].z = h;
			//groundPos[ xx ][ yy ].u = ( xx * OUTDOORS_STEP * 32 ) / static_cast<float>(MAP_WIDTH);
			//groundPos[ xx ][ yy ].v = ( yy * OUTDOORS_STEP * 32 ) / static_cast<float>(MAP_DEPTH);

			groundPos[ xx ][ yy ].u = ( ( xx % OUTDOOR_FLOOR_TEX_SIZE ) / static_cast<float>( OUTDOOR_FLOOR_TEX_SIZE ) ) + ( xx / OUTDOOR_FLOOR_TEX_SIZE );
			groundPos[ xx ][ yy ].v = ( ( yy % OUTDOOR_FLOOR_TEX_SIZE ) / static_cast<float>( OUTDOOR_FLOOR_TEX_SIZE ) ) + ( yy / OUTDOOR_FLOOR_TEX_SIZE );

			groundPos[ xx ][ yy ].tex = groundTex[ xx ][ yy ];

			// height-based light
			if ( ground[ xx ][ yy ] >= 10 ) {
				// ground (rock)
				float n = ( h / ( 13.0f * MUL ) );
				groundPos[ xx ][ yy ].r = n * 0.5f;
				groundPos[ xx ][ yy ].g = n * 0.6f;
				groundPos[ xx ][ yy ].b = n * 1.0f;
				groundPos[ xx ][ yy ].a = 1;
			} else if ( ground[ xx ][ yy ] <= -10 ) {
				// water
				float n = ( -h / ( 13.0f * MUL ) );
				groundPos[ xx ][ yy ].r = n * 0.05f;
				groundPos[ xx ][ yy ].g = n * 0.4f;
				groundPos[ xx ][ yy ].b = n * 1;
				groundPos[ xx ][ yy ].a = 1;
			} else {
				float n = ( h / ( 6.0f * MUL ) ) * 0.65f + 0.35f;
				if ( Util::dice( 6 ) ) {
					//groundPos[ xx ][ yy ].r = n * 0.55f;
					groundPos[ xx ][ yy ].r = n;
					groundPos[ xx ][ yy ].g = n;
					//groundPos[ xx ][ yy ].b = n * 0.45f;
					groundPos[ xx ][ yy ].b = n;
					groundPos[ xx ][ yy ].a = 1;
				} else {
					groundPos[ xx ][ yy ].r = n;
					groundPos[ xx ][ yy ].g = n;
					//groundPos[ xx ][ yy ].b = n * 0.25f;
					groundPos[ xx ][ yy ].b = n;
					groundPos[ xx ][ yy ].a = 1;
				}
			}
			//n++;
		}
	}


	// add light
	CVectorTex *p[3];
	for ( int xx = 0; xx < MAP_TILES_X; xx++ ) {
		for ( int yy = 0; yy < MAP_TILES_Y; yy++ ) {
			p[0] = &( groundPos[ xx ][ yy ] );
			p[1] = &( groundPos[ xx + 1 ][ yy ] );
			p[2] = &( groundPos[ xx ][ yy + 1 ] );
			addLight( p[0], p[1], p[2] );
			addLight( p[1], p[0], p[2] );
			addLight( p[2], p[0], p[1] );
		}
	}
}

/// Adds a light source.

void Map::addLight( CVectorTex *pt, CVectorTex *a, CVectorTex *b ) {
	float v[3], u[3], normal[3];

	v[0] = pt->x - a->x;
	v[1] = pt->y - a->y;
	v[2] = pt->z - a->z;
	Util::normalize( v );

	u[0] = pt->x - b->x;
	u[1] = pt->y - b->y;
	u[2] = pt->z - b->z;
	Util::normalize( u );

	Util::cross_product( u, v, normal );
	float light = Util::getLight( normal );
	pt->r *= light;
	pt->g *= light;
	pt->b *= light;
}

#define WATER_MOVE_SPEED 80
Uint32 waterMoveTick = 0;
#define WATER_MOVE_DELTA 0.005f
GLfloat waterTexX = 0;
GLfloat waterTexY = 0;

/// Draws the water level on outdoor maps.

void Map::drawWaterLevel() {
	Uint32 t = SDL_GetTicks();
	if ( t - waterMoveTick > WATER_MOVE_SPEED ) {
		waterMoveTick = t;
		waterTexX += WATER_MOVE_DELTA;
		if ( waterTexX >= 1.0f ) waterTexX -= 1.0f;
		waterTexY += WATER_MOVE_DELTA;
		if ( waterTexY >= 1.0f ) waterTexY -= 1.0f;
	}

	glEnable( GL_TEXTURE_2D );
	getShapes()->getCurrentTheme()->getOutdoorTextureGroup( WallTheme::OUTDOOR_THEME_REF_WATER )[0].glBind();
	glEnable( GL_BLEND );
	glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );
	GLfloat ratio = MAP_UNIT / CAVE_CHUNK_SIZE;
	float w = static_cast<float>( mapViewWidth ) * MUL;
	float d = static_cast<float>( mapViewDepth ) * MUL;
	//float z = -4 * MUL;
	//glTranslatef( xpos2, ypos2, 0.0f);
	glColor4f( 1.0f, 1.0f, 1.0f, 0.35f );
// glNormal3f( 0.0f, 0.0f, 1.0f );
	glBegin( GL_TRIANGLE_STRIP );
	glTexCoord2f( getX() / MUL * ratio + waterTexX, getY() / MUL * ratio + waterTexY );
	glVertex3f( 0.0f, 0.0f, -0.3f );
	glTexCoord2f( ( getX() + mapViewWidth ) / MUL * ratio + waterTexX, getY() / MUL * ratio + waterTexY );
	glVertex3f( w, 0.0f, -0.3f );
	glTexCoord2f( getX() / MUL * ratio + waterTexX, ( getY() + mapViewDepth ) / MUL * ratio + waterTexY );
	glVertex3f( 0.0f, d, -0.3f );
	glTexCoord2f( ( getX() + mapViewWidth ) / MUL * ratio + waterTexX, ( getY() + mapViewDepth ) / MUL * ratio + waterTexY );
	glVertex3f( w, d, -0.3f );
	glEnd();
	//glDisable( GL_BLEND );
}

/// Draws the indoors floor as a single quad.

void Map::drawFlatFloor() {
	GLfloat ratio = MAP_UNIT / CAVE_CHUNK_SIZE;
	float w = static_cast<float>( mapViewWidth ) * MUL;
	float d = static_cast<float>( mapViewDepth ) * MUL;
	//glTranslatef( xpos2, ypos2, 0.0f);
// glNormal3f( 0.0f, 0.0f, 1.0f );
	glBegin( GL_TRIANGLE_STRIP );
	glTexCoord2f( getX() / MUL * ratio, getY() / MUL * ratio );
	glVertex3f( 0.0f, 0.0f, 0.0f );
	glTexCoord2f( ( getX() + mapViewWidth ) / MUL * ratio, getY() / MUL * ratio );
	glVertex3f( w, 0.0f, 0.0f );
	glTexCoord2f( getX() / MUL * ratio, ( getY() + mapViewDepth ) / MUL * ratio );
	glVertex3f( 0.0f, d, 0.0f );
	glTexCoord2f( ( getX() + mapViewWidth ) / MUL * ratio, ( getY() + mapViewDepth ) / MUL * ratio );
	glVertex3f( w, d, 0.0f );
	glEnd();
}

/// Initializes the outdoor ground textures. Takes height into account.

void Map::initOutdoorsGroundTexture() {
	// set ground texture

	map<int, int> texturesUsed;

	int ex = MAP_TILES_X;
	int ey = MAP_TILES_Y;
	// ideally the below would be refs[ex][ey] but that won't work in C++... :-(
	int refs[MAP_WIDTH][MAP_DEPTH];
	for ( int x = 0; x < ex; x += OUTDOOR_FLOOR_TEX_SIZE ) {
		for ( int y = 0; y < ey; y += OUTDOOR_FLOOR_TEX_SIZE ) {
			bool high = isRockTexture( x, y );
			bool low = isLakebedTexture( x, y );
			// if it's both high and low, make rock texture. Otherwise mountain sides will be drawn with lakebed texture.
			int r = high ? WallTheme::OUTDOOR_THEME_REF_ROCK :
			        ( low ? WallTheme::OUTDOOR_THEME_REF_LAKEBED :
			          WallTheme::OUTDOOR_THEME_REF_GRASS );
			Texture tex = getThemeTex( r );
			for ( int xx = 0; xx < OUTDOOR_FLOOR_TEX_SIZE; xx++ ) {
				for ( int yy = 0; yy < OUTDOOR_FLOOR_TEX_SIZE; yy++ ) {
					refs[x + xx][y + yy] = r;
					setGroundTex( x + xx, y + yy, tex );
				}
			}
		}
	}

	for ( int x = OUTDOOR_FLOOR_TEX_SIZE; x < ex - OUTDOOR_FLOOR_TEX_SIZE; x += OUTDOOR_FLOOR_TEX_SIZE ) {
		for ( int y = OUTDOOR_FLOOR_TEX_SIZE; y < ey - OUTDOOR_FLOOR_TEX_SIZE; y += OUTDOOR_FLOOR_TEX_SIZE ) {
			if ( refs[x][y] != WallTheme::OUTDOOR_THEME_REF_GRASS ) {
				bool w = refs[x - OUTDOOR_FLOOR_TEX_SIZE][y] == refs[x][y] ? true : false;
				bool e = refs[x + OUTDOOR_FLOOR_TEX_SIZE][y] == refs[x][y] ? true : false;
				bool s = refs[x][y + OUTDOOR_FLOOR_TEX_SIZE] == refs[x][y] ? true : false;
				bool n = refs[x][y - OUTDOOR_FLOOR_TEX_SIZE] == refs[x][y] ? true : false;
				if ( !( w && e && s && n ) ) {
					applyGrassEdges( x, y, w, e, s, n );
				}
			}
		}
	}

	addHighVariation( WallTheme::OUTDOOR_THEME_REF_SNOW, GROUND_LAYER );
	addHighVariation( WallTheme::OUTDOOR_THEME_REF_SNOW_BIG, GROUND_LAYER );

}

/// Sets up a smoothly blended grass edge.

/// It takes a couple of parameters: The x,y map position and four parameters
/// that specify in which direction(s) to apply the blending.

void Map::applyGrassEdges( int x, int y, bool w, bool e, bool s, bool n ) {
	int angle = 0;
	int sx = x;
	int sy = y + 1 + OUTDOOR_FLOOR_TEX_SIZE;
	int ref = -1;
	if ( !w && !s && !e ) {
		angle = 0;
		ref = WallTheme::OUTDOOR_THEME_REF_GRASS_TIP;
	} else if ( !e && !s && !n ) {
		angle = 90;
		ref = WallTheme::OUTDOOR_THEME_REF_GRASS_TIP;
	} else if ( !e && !n && !w ) {
		angle = 180;
		ref = WallTheme::OUTDOOR_THEME_REF_GRASS_TIP;
	} else if ( !w && !n && !s ) {
		angle = 270;
		ref = WallTheme::OUTDOOR_THEME_REF_GRASS_TIP;

	} else if ( !w && !e ) {
		angle = 0;
		ref = WallTheme::OUTDOOR_THEME_REF_GRASS_NARROW;
	} else if ( !n && !s ) {
		angle = 90;
		ref = WallTheme::OUTDOOR_THEME_REF_GRASS_NARROW;

	} else if ( !w && !s ) {
		angle = 90;
		ref = WallTheme::OUTDOOR_THEME_REF_GRASS_CORNER;
	} else if ( !e && !s ) {
		angle = 180;
		ref = WallTheme::OUTDOOR_THEME_REF_GRASS_CORNER;
	} else if ( !e && !n ) {
		angle = 270;
		ref = WallTheme::OUTDOOR_THEME_REF_GRASS_CORNER;
	} else if ( !w && !n ) {
		angle = 0;
		ref = WallTheme::OUTDOOR_THEME_REF_GRASS_CORNER;

	} else if ( !e ) {
		angle = 180;
		ref = WallTheme::OUTDOOR_THEME_REF_GRASS_EDGE;
	} else if ( !w ) {
		angle = 0;
		ref = WallTheme::OUTDOOR_THEME_REF_GRASS_EDGE;
	} else if ( !n ) {
		angle = 270;
		ref = WallTheme::OUTDOOR_THEME_REF_GRASS_EDGE;
	} else if ( !s ) {
		angle = 90;
		ref = WallTheme::OUTDOOR_THEME_REF_GRASS_EDGE;
	}

	if ( ref > -1 ) {
		setOutdoorTexture( sx * OUTDOORS_STEP, sy * OUTDOORS_STEP, 0, 0, ref, angle, false, false, GROUND_LAYER );
	}
}

/// Returns a random variation of the outdoor texture specified by ref.

Texture Map::getThemeTex( int ref ) {
	int faceCount = getShapes()->getCurrentTheme()->getOutdoorFaceCount( ref );
	Texture* textureGroup = getShapes()->getCurrentTheme()->getOutdoorTextureGroup( ref );
	return textureGroup[ Util::dice( faceCount ) ];
}

/// Adds semi-random height variation to an outdoor map.

/// Higher parts of the map are randomly selected, their height value
/// set to z and textured with the referenced theme specific texture.

void Map::addHighVariation( int ref, int z ) {
	int width = getShapes()->getCurrentTheme()->getOutdoorTextureWidth( ref );
	int height = getShapes()->getCurrentTheme()->getOutdoorTextureHeight( ref );
	int outdoor_w = width / OUTDOORS_STEP;
	int outdoor_h = height / OUTDOORS_STEP;
	int ex = MAP_TILES_X;
	int ey = MAP_TILES_Y;
	for ( int x = 0; x < ex; x += outdoor_w ) {
		for ( int y = 0; y < ey; y += outdoor_h ) {
			if ( isAllHigh( x, y, outdoor_w, outdoor_h ) && !Util::dice( 10 ) &&
			        !hasOutdoorTexture( x, y, width, height ) ) {
				setOutdoorTexture( x * OUTDOORS_STEP, ( y + outdoor_h + 1 ) * OUTDOORS_STEP,
				                   0, 0, ref, Util::dice( 4 ) * 90.0f, false, false, z );
			}
		}
	}
}

/// Should a rock texture be applied to this map position due to its height?

bool Map::isRockTexture( int x, int y ) {
	bool high = false;
	for ( int xx = 0; xx < OUTDOOR_FLOOR_TEX_SIZE + 1; xx++ ) {
		for ( int yy = 0; yy < OUTDOOR_FLOOR_TEX_SIZE + 1; yy++ ) {
			if ( ground[ x + xx ][ y + yy ] > 10 ) {
				high = true;
				break;
			}
		}
	}
	return high;
}


bool Map::isLakebedTexture( int x, int y ) {
	bool low = false;
	for ( int xx = 0; xx < OUTDOOR_FLOOR_TEX_SIZE + 1; xx++ ) {
		for ( int yy = 0; yy < OUTDOOR_FLOOR_TEX_SIZE + 1; yy++ ) {
			if ( ground[ x + xx ][ y + yy ] < -10 ) {
				low = true;
				break;
			}
		}
	}
	return low;
}

/// Are all map tiles in the specified area high above "sea level"?

bool Map::isAllHigh( int x, int y, int w, int h ) {
	bool high = true;
	for ( int xx = 0; xx < w + 1; xx++ ) {
		for ( int yy = 0; yy < h + 1; yy++ ) {
			if ( ground[ x + xx ][ y + yy ] < 10 ) {
				high = false;
				break;
			}
		}
	}
	return high;
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

/// Draws the traps.

void Map::drawTraps() {
	for ( set<Uint8>::iterator i = trapSet.begin(); i != trapSet.end(); i++ ) {
		Trap *trap = getTrapLoc( static_cast<int>( *i ) );

		if ( trap->discovered || settings->isGridShowing() || DEBUG_TRAPS ) {

			glEnable( GL_BLEND );
			glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );
			glDisable( GL_CULL_FACE );
			glDisable( GL_TEXTURE_2D );

			// get the color
			// FIXME: colors should be ref-d from scourgeview.cpp colors
			if ( !trap->enabled ) {
				//ret = disabledTrapColor;
				glColor4f( 0.5, 0.5, 0.5, 0.5f );
			} else if ( trap->discovered ) {
				if ( trap == getTrapLoc( getSelectedTrapIndex() ) ) {
					//ret = outlineColor;
					glColor4f( 0.3f, 0.3f, 0.5f, 0.5f );
				} else {
					// ret = enabledTrapColor;
					glColor4f( 1.0f, 1.0f, 0.0f, 0.5f );
				}
			} else {
				// ret = debugTrapColor;
				glColor4f( 0.0f, 1.0f, 1.0f, 0.5f );
			}

			glLineWidth( 3 );
			//glBegin( GL_POLYGON );
			glBegin( GL_LINE_LOOP );
			for ( unsigned int i = 0; i < trap->hull.size(); i++ ) {
				CVector2 *p = trap->hull[ i ];
				glVertex3f( ( p->x - getX() ) * MUL, ( p->y - getY() ) * MUL, 0.5f * MUL );
			}
			glEnd();
			glLineWidth( 1 );
			glEnable( GL_TEXTURE_2D );
			//glDisable( GL_BLEND );
		}
	}
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
