/***************************************************************************
                          map.cpp  -  description
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

#include "map.h"
#include "effect.h"
#include "frustum.h"
#include "location.h"
#include "shape.h"
#include "shapes.h"
#include "glshape.h"
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

using namespace std;

//#define DEBUG_MOUSE_POS 1

#define USE_LIGHTING 1

#define MOUSE_ROT_DELTA 2

#define ZOOM_DELTA 1.2f

#define KEEP_MAP_SIZE 0

//#define MVW 100
//#define MVD 110
#define MVW 150
#define MVD 150

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

const float Map::shadowTransformMatrix[16] = { 
	1, 0, 0, 0,
	0, 1, 0, 0,
//	0.5f, -0.5f, 0, 0,
  0.75f, -0.25f, 0, 0,
	0, 0, 0, 1 };
	
MapMemoryManager *Map::mapMemoryManager = new MapMemoryManager();

Map::Map( MapAdapter *adapter, Preferences *preferences, Shapes *shapes ) {
  hasWater = false;

  startx = starty = 128;
  cursorMapX = cursorMapY = cursorMapZ = MAP_WIDTH + 1;
  cursorFlatMapX = cursorFlatMapY = MAP_WIDTH + 1;
  cursorChunkX = cursorChunkY = ( MAP_WIDTH / MAP_UNIT ) + 1;
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
  LIGHTMAP_ENABLED=0;
  zoom = 1.0f;
  zoomIn = zoomOut = false;
  x = y = 0;
  mapx = mapy = 0.0f;
  selectMode = false;
  floorOnly = false;
  useShadow = false;
  //alwaysCenter = true;

  debugX = debugY = debugZ = -1;
  
  mapChanged = true;
  resortShapes = true;

  floorTexWidth = floorTexHeight = 0;
  floorTex = 0;

  mapCenterCreature = NULL;
    
  this->xrot = 0.0f;
	this->yrot = 30.0f;
  this->zrot = 45.0f;
  this->xRotating = this->yRotating = this->zRotating = 0.0f;

  setViewArea(0, 0, 
              adapter->getScreenWidth(), 
              adapter->getScreenHeight());

  float adjust = (float)viewWidth / 800.0f;
  this->xpos = (float)(viewWidth) / 2.0f / adjust;
  this->ypos = (float)(viewHeight) / 2.0f / adjust;
  this->zpos = 0.0f;  

  this->debugGridFlag = false;
  this->drawGridFlag = false;

	for( int x = 0; x < MAP_WIDTH / MAP_UNIT; x++ ) {
		for( int y = 0; y < MAP_DEPTH / MAP_UNIT; y++ ) {
			rugPos[x][y].texture = 0;
		}
	}
  
  // initialize shape graph of "in view shapes"
	for(int x = 0; x < MAP_WIDTH; x++) {
		for(int y = 0; y < MAP_DEPTH; y++) {
			floorPositions[x][y] = NULL;			
			for(int z = 0; z < MAP_VIEW_HEIGHT; z++) {
				pos[x][y][z] = NULL;
				itemPos[x][y] = NULL;
				effect[x][y][z] = NULL;
			}      
		}
  }
  // Init the pos cache
  for(int x = 0; x < MAX_POS_CACHE; x++) {
    posCache[x] = NULL;
  }
  nbPosCache = -1;

	for( int x = 0; x < MAP_WIDTH; x++ ) {
		for( int y = 0; y < MAP_DEPTH; y++ ) {
			ground[x][y] = 0;
			groundTex[x][y] = 0;
		}
	}
	heightMapEnabled = false;
	for( int i = 0; i < 4; i++ )
		debugHeightPosXX[i] = debugHeightPosYY[i] = 0;

  // initialize the lightmap
	for(int x = 0; x < MAP_WIDTH / MAP_UNIT; x++) {
		for(int y = 0; y < MAP_DEPTH / MAP_UNIT; y++) {
			lightMap[x][y] = (LIGHTMAP_ENABLED ? 0 : 1);
		}
	}

  lightMapChanged = true;  
  colorAlreadySet = false;
  selectedDropTarget = NULL;

  helper = NULL;

	laterCount = stencilCount = otherCount = damageCount = 0;

	quakesEnabled = false;

	refreshGroundPos = true;

	outdoorShadow = adapter->getNamedTexture( "outdoors_shadow" );
	outdoorShadowTree = adapter->getNamedTexture( "outdoors_shadow_tree" );
	waterTexture = adapter->getNamedTexture( "water" );

	hackBlockingPos = new Location();
	hackBlockingPos->creature = NULL;
	hackBlockingPos->heightPos = 15;
	hackBlockingPos->item = NULL;
	hackBlockingPos->outlineColor = NULL;
	hackBlockingPos->shape = NULL;
	hackBlockingPos->x = hackBlockingPos->y = hackBlockingPos->z = 0;

  adapter->addDescription(Constants::getMessage(Constants::WELCOME), 1.0f, 0.5f, 1.0f);
  adapter->addDescription("----------------------------------", 1.0f, 0.5f, 1.0f);
}

Map::~Map(){
  reset();
	delete hackBlockingPos;
  delete frustum;
  if( helper ) delete helper;
}

void Map::reset() {
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
  for(int xp = 0; xp < MAP_WIDTH; xp++) {
    for(int yp = 0; yp < MAP_DEPTH; yp++) {
      if( floorPositions[xp][yp] ) {
        Uint32 key = createPairKey(xp, yp);
        if( water.find(key) != water.end() ) {
          WaterTile *w = water[key];
          free(w);
        }
      }
			if(itemPos[xp][yp]) {
				Location *p = itemPos[xp][yp];
				if( deleted.find( p ) == deleted.end() ) deleted.insert( p );
				itemPos[xp][yp] = NULL;
			}
      for(int zp = 0; zp < MAP_VIEW_HEIGHT; zp++) {
        if(pos[xp][yp][zp]) {
          Location *p = pos[xp][yp][zp];
          if( deleted.find( p ) == deleted.end() ) deleted.insert( p );
          pos[xp][yp][zp] = NULL;
        }
      }
    }
  }
  for( set<Location*>::iterator i = deleted.begin(); i != deleted.end(); ++i ) {
    Location *p = *i;
    mapMemoryManager->deleteLocation( p );
  }  
  water.clear();

  //cerr << "reset 5" << endl;
  zoom = 1.0f;
  zoomIn = zoomOut = false;
  x = y = 0;
  mapx = mapy = 0.0f;
  selectMode = false;
  floorOnly = false;
  useShadow = false;
  //alwaysCenter = true;
  debugX = debugY = debugZ = -1;
  mapChanged = true;
  resortShapes = true;
  lastOutlinedX = lastOutlinedY = lastOutlinedZ = MAP_WIDTH;
  floorTexWidth = floorTexHeight = 0;
  floorTex = 0;
  mapCenterCreature = NULL;
  secretDoors.clear();
  
  this->xrot = 0.0f;
  this->yrot = 30.0f;
  this->zrot = 45.0f;
  this->xRotating = this->yRotating = this->zRotating = 0.0f;

  setViewArea(0, 0, 
              adapter->getScreenWidth(), 
              adapter->getScreenHeight());

  float adjust = (float)viewWidth / 800.0f;
  this->xpos = (float)(viewWidth) / 2.0f / adjust;
  this->ypos = (float)(viewHeight) / 2.0f / adjust;
  this->zpos = 0.0f;  

  this->debugGridFlag = false;
  this->drawGridFlag = false;
  
  //cerr << "reset 6" << endl;  

	for( int x = 0; x < MAP_WIDTH / MAP_UNIT; x++ ) {
		for( int y = 0; y < MAP_DEPTH / MAP_UNIT; y++ ) {
			rugPos[x][y].texture = 0;
		}
	}
  
  // initialize shape graph of "in view shapes"
	for(int x = 0; x < MAP_WIDTH; x++) {
		for(int y = 0; y < MAP_DEPTH; y++) {
			floorPositions[x][y] = NULL;
			for(int z = 0; z < MAP_VIEW_HEIGHT; z++) {
				pos[x][y][z] = NULL;
        effect[x][y][z] = NULL;
				itemPos[x][y] = NULL;
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
  for(int x = 0; x < MAP_WIDTH / MAP_UNIT; x++) {
    for(int y = 0; y < MAP_DEPTH / MAP_UNIT; y++) {
      lightMap[x][y] = (LIGHTMAP_ENABLED ? 0 : 1);
    }
  }
  lightMapChanged = true;  
  colorAlreadySet = false;
  selectedDropTarget = NULL;

  if( helper ) helper->reset();

	laterCount = stencilCount = otherCount = damageCount = 0;

	quakesEnabled = false;
	for( int x = 0; x < MAP_WIDTH; x++ ) {
		for( int y = 0; y < MAP_DEPTH; y++ ) {
			ground[x][y] = 0;
			groundTex[x][y] = 0;
		}
	}
	heightMapEnabled = false;
	for( int i = 0; i < 4; i++ )
		debugHeightPosXX[i] = debugHeightPosYY[i] = 0;
	
	refreshGroundPos = true;
}

void Map::setViewArea(int x, int y, int w, int h) {
  //viewX = x;
//  viewY = y;
  viewX = 0;
  viewY = 0;
  viewWidth = w;
  viewHeight = h;

  float adjust = (float)viewWidth / 800.0f;
  if( preferences->getKeepMapSize() ) {
    zoom = (float)adapter->getScreenWidth() / (float)w;
  }
  xpos = (int)((float)viewWidth / zoom / 2.0f / adjust);
  ypos = (int)((float)viewHeight / zoom / 2.0f / adjust);

  refresh();
}

void Map::center(Sint16 x, Sint16 y, bool force) { 
  Sint16 nx = x - mapViewWidth / 2; 
  Sint16 ny = y - mapViewDepth / 2;
  /*
  Sint16 nx = x - (int)(((float)mapViewWidth * 
                         ((float)viewWidth / 
                          (float)scourge->getSDLHandler()->getScreen()->w)) / 2.0f); 
  Sint16 ny = y - (int)(((float)mapViewDepth * 
                         ((float)viewHeight / 
                          (float)scourge->getSDLHandler()->getScreen()->h)) / 2.0f);
  */
  if( preferences->getAlwaysCenterMap() || force ) {
	//  if(scourge->getPreferences()->getAlwaysCenterMap() ||
	//     abs(this->x - nx) > X_CENTER_TOLERANCE ||
	//     abs(this->y - ny) > Y_CENTER_TOLERANCE) {

    // relocate
    this->x = nx;
    this->y = ny;
    this->mapx = nx;
    this->mapy = ny;
  }
}

void Map::removeCurrentEffects() {
  for( map<Uint32, EffectLocation*>::iterator i=currentEffectsMap.begin(); 
       i!=currentEffectsMap.end(); ) {
    Uint32 key = i->first;
    EffectLocation *pos = i->second;
    if( !pos->isEffectOn() ) {
      int x, y, z;
      decodeTripletKey(key, &x, &y, &z);
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

/**
   If 'ground' is true, it draws the ground layer.
   Otherwise the shape arrays (other, stencil, later) are populated.
*/
void Map::setupShapes(bool ground, bool water, int *csx, int *cex, int *csy, int *cey) {
  if(!ground && !water) {
    laterCount = stencilCount = otherCount = damageCount = 0;
    mapChanged = false;
  }

  chunkCount = 0;

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

  if(csx) {
    *csx = chunkStartX;
    *cex = chunkEndX;
    *csy = chunkStartY;
    *cey = chunkEndY;
  }

  Shape *shape;
  int posX, posY;
  float xpos2, ypos2, zpos2;
  for(int chunkX = chunkStartX; chunkX < chunkEndX; chunkX++) {
    for(int chunkY = chunkStartY; chunkY < chunkEndY; chunkY++) {

      // remember the chunk's starting pos.
      float chunkPosX = (float)((chunkX - chunkStartX) * MAP_UNIT + 
                                chunkOffsetX) / DIV;
      float chunkPosY = (float)((chunkY - chunkStartY) * MAP_UNIT + 
                                chunkOffsetY) / DIV;
      
      // frustum testing
      //frustum->CalculateFrustum();
      if(useFrustum && 
         !frustum->CubeInFrustum(chunkPosX, chunkPosY, 0.0f, (float)MAP_UNIT / DIV)) 
        continue;


      // FIXME: works but slow. Use 1 polygon instead (like floor)
      // special cave edge code
      if( !( ground || water ) && 
          floorTexWidth > 0 && 
					!isHeightMapEnabled() &&
          ( chunkX < 0 || chunkY < 0 ) ) {
        for( int yp = CAVE_CHUNK_SIZE; yp < MAP_UNIT + CAVE_CHUNK_SIZE; yp += CAVE_CHUNK_SIZE ) {
          for( int xp = 0; xp < MAP_UNIT; xp += CAVE_CHUNK_SIZE ) {
            xpos2 = (float)((chunkX - chunkStartX) * MAP_UNIT + 
                            xp + chunkOffsetX) / DIV;
            ypos2 = (float)((chunkY - chunkStartY) * MAP_UNIT - 
                            CAVE_CHUNK_SIZE + 
                            yp + chunkOffsetY) / DIV;     
            setupPosition( 0, CAVE_CHUNK_SIZE, 0,
                           xpos2, ypos2, 0,
                           pos[0][CAVE_CHUNK_SIZE][0]->shape, NULL, NULL, 
                           NULL );
          }
        }
      }

      if( chunkX < 0 || chunkX > MAP_WIDTH / MAP_UNIT ||
          chunkY < 0 || chunkY > MAP_DEPTH / MAP_UNIT ) continue;




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
      if(!lightMap[chunkX][chunkY]) {
        if(ground || water) continue;
        else {
          // look to the left
          if(chunkX >= 1 && lightMap[chunkX - 1][chunkY]) drawSide |= Constants::MOVE_LEFT;
          // look to the right
          if(chunkX + 1 < MAP_WIDTH / MAP_UNIT && lightMap[chunkX + 1][chunkY]) drawSide |= Constants::MOVE_RIGHT;
          // look above
          if(chunkY - 1 >= 0 && lightMap[chunkX][chunkY - 1]) drawSide |= Constants::MOVE_UP;
          // look below
          if(chunkY + 1< MAP_DEPTH / MAP_UNIT && lightMap[chunkX][chunkY + 1]) drawSide |= Constants::MOVE_DOWN;
          // if not, skip this chunk
          if(!drawSide) continue;
        }
      }

			if( ( ground || water ) && rugPos[ chunkX ][ chunkY ].texture > 0 ) {
				xpos2 = (float)((chunkX - chunkStartX) * MAP_UNIT + chunkOffsetX) / DIV;
				ypos2 = (float)((chunkY - chunkStartY) * MAP_UNIT + chunkOffsetY) / DIV;
				drawRug( &rugPos[ chunkX ][ chunkY ], xpos2, ypos2, chunkX, chunkY );
			}
      
      for(int yp = 0; yp < MAP_UNIT; yp++) {
        for(int xp = 0; xp < MAP_UNIT; xp++) {
          /**
           In scourge, shape coordinates are given by their
           left-bottom corner. So the +1 for posY moves the
           position 1 unit down the Y axis, which is the
           unit square's bottom left corner.
           */
          posX = chunkX * MAP_UNIT + xp + MAP_OFFSET;
          posY = chunkY * MAP_UNIT + yp + MAP_OFFSET + 1;

          if(ground || water) {
            shape = floorPositions[posX][posY];
            if(shape) {
              xpos2 = (float)((chunkX - chunkStartX) * MAP_UNIT + 
                              xp + chunkOffsetX) / DIV;
              ypos2 = (float)((chunkY - chunkStartY) * MAP_UNIT - 
                              shape->getDepth() +
                              yp + chunkOffsetY) / DIV;

              if( water ) {
								drawWaterPosition(posX, posY,
																	xpos2, ypos2,
																	shape);      
              } else {
                drawGroundPosition(posX, posY,
																	 xpos2, ypos2,
																	 shape);      
              }
            }
          } else {

						if( lightMap[chunkX][chunkY] &&
								itemPos[posX][posY] && 
								itemPos[posX][posY]->x == posX &&
								itemPos[posX][posY]->y == posY ) {
							
							shape = itemPos[posX][posY]->shape;
							
							xpos2 = (float)((chunkX - chunkStartX) * MAP_UNIT + 
															xp + chunkOffsetX) / DIV;
							ypos2 = (float)((chunkY - chunkStartY) * MAP_UNIT - 
															shape->getDepth() + 
															yp + chunkOffsetY) / DIV;
							
							setupPosition( posX, posY, 0,
														 xpos2, ypos2, 0,
														 shape, 
														 itemPos[posX][posY]->item, 
														 NULL, NULL, true );
						}


            for(int zp = 0; zp < MAP_VIEW_HEIGHT; zp++) {
              if(lightMap[chunkX][chunkY] &&
                 effect[posX][posY][zp] &&
                 !effect[posX][posY][zp]->isInDelay() ) {
                xpos2 = (float)((chunkX - chunkStartX) * MAP_UNIT + 
                                xp + chunkOffsetX) / DIV;
                ypos2 = (float)((chunkY - chunkStartY) * MAP_UNIT - 
                                1 + 
                                yp + chunkOffsetY) / DIV;
                zpos2 = (float)(zp) / DIV;

                setupPosition(posX, posY, zp - effect[posX][posY][zp]->z,
                              xpos2, ypos2, zpos2,
                              effect[posX][posY][zp]->effect->getShape(), NULL, NULL,
                              effect[posX][posY][zp]);
              }

              if(pos[posX][posY][zp] && 
                 pos[posX][posY][zp]->x == posX &&
                 pos[posX][posY][zp]->y == posY &&
                 pos[posX][posY][zp]->z == zp) {
                shape = pos[posX][posY][zp]->shape;

                // is this shape visible on the edge an chunk in darkness?
                bool lightEdge = 
                  ( !lightMap[chunkX][chunkY] && shape && !pos[posX][posY][zp]->creature &&
                    ( ( drawSide & Constants::MOVE_DOWN && yp >= MAP_UNIT - MAP_UNIT_OFFSET && shape->getDepth() <= MAP_UNIT_OFFSET ) ||
                      ( drawSide & Constants::MOVE_UP && yp <= MAP_UNIT_OFFSET && shape->getDepth() <= MAP_UNIT_OFFSET ) ||
                      ( drawSide & Constants::MOVE_LEFT && xp < MAP_UNIT_OFFSET && shape->getWidth() <= MAP_UNIT_OFFSET ) ||
                      ( drawSide & Constants::MOVE_RIGHT && xp >= MAP_UNIT - MAP_UNIT_OFFSET && shape->getWidth() <= MAP_UNIT_OFFSET ) )
                    );

                if( shape && ( lightMap[chunkX][chunkY] || lightEdge ) ) {
                  if( pos[posX][posY][zp]->creature ) {

                    if( debugMd2Shapes ) {
                      // debug
                      xpos2 = (float)((chunkX - chunkStartX) * MAP_UNIT + 
                                      xp + chunkOffsetX) / DIV;
                      ypos2 = (float)((chunkY - chunkStartY) * MAP_UNIT - 
                                      shape->getDepth() + 
                                      yp + chunkOffsetY) / DIV;
                      zpos2 = (float)(zp) / DIV;
                      setupPosition(posX, posY, zp,
                                    xpos2, ypos2, zpos2,
                                    ((AnimatedShape*)pos[posX][posY][zp]->shape)->getDebugShape(), 
                                    NULL, NULL, NULL);
                      // end debug
                    }


                    //xpos2 = (pos[posX][posY][zp]->creature->getX() - (GLfloat)getX()) / DIV;
                    //ypos2 = (pos[posX][posY][zp]->creature->getY() - (GLfloat)getY() - (GLfloat)(shape->getDepth())) / DIV;

                    float xdiff = ( pos[posX][posY][zp]->creature->getX() - (float)(toint(pos[posX][posY][zp]->creature->getX())));
                    float ydiff = ( pos[posX][posY][zp]->creature->getY() - (float)(toint(pos[posX][posY][zp]->creature->getY())));
                    xpos2 = (float)((chunkX - chunkStartX) * MAP_UNIT + 
                                    xp + chunkOffsetX +
                                    xdiff ) / DIV;
                    ypos2 = (float)((chunkY - chunkStartY) * MAP_UNIT - 
                                    shape->getDepth() + 
                                    yp + chunkOffsetY + 
                                    ydiff ) / DIV;

										pos[posX][posY][zp]->heightPos = findMaxHeightPos( pos[posX][posY][zp]->creature->getX(), 
																																			 pos[posX][posY][zp]->creature->getY(), 
																																			 pos[posX][posY][zp]->creature->getZ() );


                  } else {
                    xpos2 = (float)((chunkX - chunkStartX) * MAP_UNIT + 
                                    xp + chunkOffsetX) / DIV;
                    ypos2 = (float)((chunkY - chunkStartY) * MAP_UNIT - 
                                    shape->getDepth() + 
                                    yp + chunkOffsetY) / DIV;
                  }
                  zpos2 = (float)(zp) / DIV;

                  setupPosition(posX, posY, zp,
                                xpos2, ypos2, zpos2,
                                shape, pos[posX][posY][zp]->item, pos[posX][posY][zp]->creature, 
                                NULL);
                }
              }
            }
          }
        }
      }
    }
  }
}

void Map::drawRug( Rug *rug, float xpos2, float ypos2, int xchunk, int ychunk ) {
	glPushMatrix();
	glTranslatef( xpos2, ypos2, 0.255f / DIV );
	glRotatef( rug->angle, 0, 0, 1 );
	float f = MAP_UNIT / DIV;
	float offset = 2.5 / DIV;

	float sx, sy, ex, ey;
	// starting section
	if( rug->isHorizontal ) {
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
	glColor4f(1, 1, 1, 0.9f);
	glBindTexture( GL_TEXTURE_2D, rug->texture );
	glBegin( GL_QUADS );
	if( rug->isHorizontal ) {
		glTexCoord2f( 1, 0 );
		glVertex2f( sx, sy );
		glTexCoord2f( 1, 1 );
		glVertex2f( ex, sy );
		glTexCoord2f( 0, 1 );
		glVertex2f( ex, ey );
		glTexCoord2f( 0, 0 );
		glVertex2f( sx, ey );
	} else {
		glTexCoord2f( 0, 0 );
		glVertex2f( sx, sy );
		glTexCoord2f( 1, 0 );
		glVertex2f( ex, sy );
		glTexCoord2f( 1, 1 );
		glVertex2f( ex, ey );
		glTexCoord2f( 0, 1 );
		glVertex2f( sx, ey );
	}
	glEnd();
	glDisable( GL_TEXTURE_2D );
  glPopMatrix();
}

void Map::drawGroundPosition(int posX, int posY,
                             float xpos2, float ypos2,
                             Shape *shape) {
  GLuint name;
  // encode this shape's map location in its name
  name = posX + (MAP_WIDTH * posY);     
  glTranslatef( xpos2, ypos2, 0.0f);
  
  glPushName( name );
  glColor4f(1, 1, 1, 0.9f);
	if( isHeightMapEnabled() ) {
		shape->drawHeightMap( ground, posX, posY );
	} else {
		shape->draw();
	}
  glPopName();

  glTranslatef( -xpos2, -ypos2, 0.0f);
}

void Map::drawWaterPosition(int posX, int posY,
                            float xpos2, float ypos2,
                            Shape *shape) {
  GLuint name;
  // encode this shape's map location in its name
  name = posX + (MAP_WIDTH * posY);     
  glTranslatef( xpos2, ypos2, 0.0f);
  
  // draw water
  Uint32 key = createPairKey( posX, posY );
  if( water.find( key ) != water.end() ) {
    glDisable( GL_CULL_FACE );
    
    float sx = ( (float)MAP_UNIT / (float)WATER_TILE_X ) / DIV;
    float sy = ( (float)MAP_UNIT / (float)WATER_TILE_Y ) / DIV;
    
    int xp = 0;
    int yp = 0;
    while( true ) {
      int stx = xp;
      int sty = yp;
      glBegin( GL_QUADS );
      for( int i = 0; i < 4; i++ ) {
        int wx, wy;
        if( xp == WATER_TILE_X && yp == WATER_TILE_Y ) {
          wx = (posX + MAP_UNIT) * WATER_TILE_X;
          wy = (posY + MAP_UNIT) * WATER_TILE_Y;
        } else if( xp == WATER_TILE_X ) {
          wx = (posX + MAP_UNIT) * WATER_TILE_X;
          wy = posY * WATER_TILE_Y + yp;
        } else if( yp == WATER_TILE_Y ) {
          wx = posX * WATER_TILE_X + xp;
          wy = (posY + MAP_UNIT) * WATER_TILE_Y;
        } else {
          wx = posX * WATER_TILE_X + xp;
          wy = posY * WATER_TILE_Y + yp;
        }

        int xx = wx % WATER_TILE_X;
        int yy = wy % WATER_TILE_Y;
        WaterTile *w = NULL;
        Uint32 key = createPairKey( wx / WATER_TILE_X, wy / WATER_TILE_Y );
        if( water.find(key) != water.end() ) {
          w = water[key];

          Uint32 time = SDL_GetTicks();   
          Uint32 elapsedTime = time - w->lastTime[xx][yy];
          if(elapsedTime >= (Uint32)(1000.0f / WATER_ANIM_SPEED) ) {
            
            w->z[xx][yy] += w->step[xx][yy];
            if( w->z[xx][yy] > WATER_AMP ||
                w->z[xx][yy] < -WATER_AMP ) w->step[xx][yy] *= -1.0f;
            
            w->lastTime[xx][yy] = time;
          }
        }

        float zz = ( w ? w->z[xx][yy] : 0.0f );
        float sz = ( WATER_HEIGHT + zz ) / DIV;
        glColor4f( 0.3f + ( zz / 30.0f ), 
                   0.25f + ( zz / 10.0f ), 
                   0.17f + ( zz / 15.0f ), 
                   0.5f );


        glVertex3f( (float)xp * sx, (float)yp * sy, sz );

        switch( i ) {
        case 0: xp++; break;
        case 1: yp++; break;
        case 2: xp--; break;
        case 3: yp--; break;
        }
        if( xp > WATER_TILE_X || yp > WATER_TILE_Y ) {
          break;
        }
      }
      glEnd();
      xp = stx + 1;
      yp = sty;
      if( xp >= WATER_TILE_X ) {
        xp = 0;
        yp++;
        if( yp >= WATER_TILE_Y ) break;
      }      
    }
    
    
    //glDepthMask( GL_TRUE );
    //glDisable( GL_BLEND );
  }

  glTranslatef( -xpos2, -ypos2, 0.0f);
}

void Map::setupPosition( int posX, int posY, int posZ,
												 float xpos2, float ypos2, float zpos2,
												 Shape *shape, RenderedItem *item, RenderedCreature *creature, 
												 EffectLocation *effect, 
												 bool itemPos ) {

  // This really doesn't make a difference unfortunately.
  //if(!isOnScreen(posX, posY, posZ)) return;

  GLuint name;
  name = posX + (MAP_WIDTH * (posY)) + (MAP_WIDTH * MAP_DEPTH * posZ);    

  // special effects
  if(effect || (creature && creature->isEffectOn())) {
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
    damageCount++;

    // don't draw shape if it's an area effect
    if(!creature) return;
  }

  if(shape->isStencil()) {
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
  } else if(!shape->isStencil()) {
    bool invisible = (creature && creature->getStateMod(StateMod::invisible));
    if(!invisible && shape->drawFirst()) {
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
      otherCount++;
    }
    if(shape->drawLater() || invisible) {
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

void Map::preDraw() {

	if( refreshGroundPos ) {
		createGroundMap();
		refreshGroundPos = false;
	}

  // must move map before calling getMapXY(Z)AtScreenXY!
  if( move && !selectMode ) moveMap( move );

  if(zoomIn) {
    if(zoom <= settings->getMinZoomIn() ) {
      zoomOut = false;
    } else {
      zoom /= ZOOM_DELTA;
    }
    mapChanged = true;
  } else if(zoomOut) {
    if(zoom >= settings->getMaxZoomOut() ) {
      zoomOut = false;
    } else {
      zoom *= ZOOM_DELTA; 
    }
  }

  float adjust = (float)viewWidth / 800.0f;
  xpos = (int)((float)viewWidth / zoom / 2.0f / adjust);
  ypos = (int)((float)viewHeight / zoom / 2.0f / adjust);

  float oldrot;

  oldrot = yrot;
  if(yRotating != 0) yrot+=yRotating;
  if(yrot >= settings->getMaxYRot() || yrot < 0) yrot = oldrot;

  oldrot = zrot;
  if(zRotating != 0) {
    resortShapes = true;
    zrot+=zRotating;
  }
  if(zrot >= 360) zrot -= 360;
  if(zrot < 0) zrot = 360 + zrot;

  if( !selectMode ) initMapView();

  // find cursor location on map (for named shapes)
  if( adapter->isMouseIsMovingOverMap() && !selectMode ) {
    // save mapChanged (fixes bug where map won't draw initially)
    bool b = mapChanged;
    selectMode = true;    
    // careful this calls draw() again!    
    getMapXYZAtScreenXY( &cursorMapX, &cursorMapY, &cursorMapZ );
    selectMode = false;
    mapChanged = b;
  }
  
  if( !selectMode ) frustum->CalculateFrustum();
  if( lightMapChanged ) configureLightMap();
  if( currentEffectsMap.size() ) removeCurrentEffects();
  // populate the shape arrays
  if( mapChanged ) {
    if( settings->isPlayerEnabled() && adapter->getPlayer() ) adapter->getPlayer()->setMapChanged();
    int csx, cex, csy, cey;
    setupShapes(false, false, &csx, &cex, &csy, &cey);
    int shapeCount = laterCount + otherCount + damageCount + stencilCount;
    if( settings->isPlayerEnabled() ) {
      sprintf(mapDebugStr, "E=%d p=%d,%d chunks=(%s %d out of %d) x:%d-%d y:%d-%d shapes=%d", 
              (int)currentEffectsMap.size(),
              ( adapter->getPlayer() ? toint(adapter->getPlayer()->getX()) : -1 ),
              ( adapter->getPlayer() ? toint(adapter->getPlayer()->getY()) : -1 ),
              (useFrustum ? "*" : ""),
              chunkCount, ((cex - csx)*(cey - csy)),
              csx, cex, csy, cey, shapeCount);
      //            shapeCount, laterCount, otherCount, damageCount, stencilCount);
    } else {
      sprintf(mapDebugStr, "E=%d chunks=(%s %d out of %d) x:%d-%d y:%d-%d shapes=%d", 
              (int)currentEffectsMap.size(),
              (useFrustum ? "*" : ""),
              chunkCount, ((cex - csx)*(cey - csy)),
              csx, cex, csy, cey, shapeCount);
    }
    adapter->setDebugStr(mapDebugStr);
  }
}

void Map::postDraw() {
  glDisable( GL_SCISSOR_TEST );

  // cancel mouse-based map movement (middle button)
  if( mouseRot ) {
    setXRot( 0 );
    setYRot( 0 );
    setZRot( 0 );
  }
  if( mouseZoom ) {
    mouseZoom = false;
    setZoomIn( false );
    setZoomOut( false );
  }
}

void Map::draw() {
  if( selectMode ) {
    for(int i = 0; i < otherCount; i++) doDrawShape(&other[i]);
    for(int i = 0; i < laterCount; i++) doDrawShape(&later[i]);
    for(int i = 0; i < stencilCount; i++) 
      if( isSecretDoor( stencil[i].pos ) )
          doDrawShape(&stencil[i]);
  } else {  

#ifdef DEBUG_MOUSE_POS
  // debugging mouse position
    DrawLater later2;
    later2.shape = shapes->findShapeByName("LAMP_BASE");
    later2.xpos = ((float)(cursorFlatMapX - getX()) / DIV);
    later2.ypos = ((float)(cursorFlatMapY - getY()) / DIV);
    later2.zpos = (float)(0) / DIV;
    later2.item = NULL;
    later2.creature = NULL;
    later2.name = 0;
    later2.pos = NULL;
    later2.effect = NULL;
    later2.inFront = false;
		later2.x = cursorFlatMapX - getX();
		later2.y = cursorFlatMapY - getY();
    doDrawShape(&later2);

    //later2.shape = shapes->findShapeByName("LAMP_BASE");
    //later2.xpos = ((float)(debugX - getX()) / DIV);
    //later2.ypos = ((float)(debugY - getY()) / DIV);
    //later2.zpos = (float)(debugZ) / DIV;
    //doDrawShape(&later2);
#endif


    if( preferences->getStencilbuf() &&
        preferences->getStencilBufInitialized() ) {


      // stencil and draw the floor
      //glDisable(GL_DEPTH_TEST);
      //glColorMask(0,0,0,0);
      glEnable(GL_STENCIL_TEST);
      glStencilOp(GL_REPLACE, GL_REPLACE, GL_REPLACE);
      glStencilFunc(GL_ALWAYS, 1, 0xffffffff);
      
      // cave floor and map editor bottom (so cursor shows)
      if( settings->isGridShowing() || floorTexWidth > 0 || isHeightMapEnabled() ) {
				renderFloor();
      } else {
        setupShapes(true, false);
      }

      // shadows
			if( preferences->getShadows() >= Constants::OBJECT_SHADOWS &&
					helper->drawShadow() ) {
        glStencilFunc(GL_EQUAL, 1, 0xffffffff);
        glStencilOp(GL_KEEP, GL_KEEP, GL_INCR); 
        glDisable(GL_TEXTURE_2D);
        glDepthMask(GL_FALSE);
        glEnable(GL_BLEND);
        useShadow = true;
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        for(int i = 0; i < otherCount; i++) {
          doDrawShape(&other[i]);
        }
        if( preferences->getShadows() == Constants::ALL_SHADOWS ) {
          for(int i = 0; i < stencilCount; i++) {
            doDrawShape(&stencil[i]);
          }
        }
        useShadow = false;
        glDisable(GL_BLEND);
        glEnable(GL_TEXTURE_2D);
        glDepthMask(GL_TRUE);
      }

      //glEnable(GL_DEPTH_TEST);
      glDisable(GL_STENCIL_TEST); 
    } else {
      // draw the ground  
      setupShapes(true, false);             

      /*
      // -------------------------------------------
      // Only here to debug glshape changes. After that REMOVE this!
      // -------------------------------------------
      // shadows
      glDisable(GL_TEXTURE_2D);
      glDepthMask(GL_FALSE);
      glEnable(GL_BLEND);
      useShadow = true;
      glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
      for(int i = 0; i < otherCount; i++) {
        doDrawShape(&other[i]);
      }
      if(session->getPreferences()->getShadows() == Constants::ALL_SHADOWS) {
        for(int i = 0; i < stencilCount; i++) {
          doDrawShape(&stencil[i]);
        }
      }
      useShadow = false;
      glDisable(GL_BLEND);
      glEnable(GL_TEXTURE_2D);
      glDepthMask(GL_TRUE);
      // -------------------------------------------
      // Only here to debug glshape changes. After that REMOVE this!
      // -------------------------------------------
      */

    }

    // draw lava flows
    for(int i = 0; i < otherCount; i++) {
      if( other[i].shape->isFlatCaveshape() ) {
        doDrawShape(&other[i]);
      }
    }

    // draw the creatures/objects/doors/etc.
    DrawLater *playerDrawLater = NULL;
    for(int i = 0; i < otherCount; i++) {
      if( other[i].shape->isFlatCaveshape() ) continue;
      if( settings->isPlayerEnabled() ) {
        if( other[i].creature && other[i].creature == adapter->getPlayer() ) 
          playerDrawLater = &(other[i]);
      }
      if(selectedDropTarget && 
         ((selectedDropTarget->creature && selectedDropTarget->creature == other[i].creature) ||
          (selectedDropTarget->item && selectedDropTarget->item == other[i].item))) {
        colorAlreadySet = true;
        glColor4f(0, 1, 1, 1);
      }
      doDrawShape(&other[i]);

			// FIXME: if feeling masochistic, try using stencil buffer to remove shadow-on-shadow effect.
			// draw simple shadow in outdoors
			if( !helper->drawShadow() ) {
				if( other[i].creature ) {
					glColor4f( 0.04f, 0, 0.07f, 0.4f );
					drawGroundTex( outdoorShadow,
												 other[i].creature->getX() + 0.25f,
												 other[i].creature->getY() + 0.25f,
												 ( other[i].creature->getShape()->getWidth() + 2 ) * 0.7f,
												 other[i].creature->getShape()->getDepth() * 0.7f );
				} else if( other[i].pos && other[i].shape && other[i].shape->isOutdoorShadow() ) {
					glColor4f( 0.04f, 0, 0.07f, 0.4f );
					drawGroundTex( outdoorShadowTree,
												 (float)other[i].pos->x - ( other[i].shape->getWidth() / 2.0f ) + ( other[i].shape->getWindValue() / 2.0f ),
												 (float)other[i].pos->y + ( other[i].shape->getDepth() / 2.0f ),
												 other[i].shape->getWidth() * 1.7f,
												 other[i].shape->getDepth() * 1.7f );
				}
			}
    }

    // draw the walls: walls in front of the player will be transparent
    if( playerDrawLater ) {

      if( floorTexWidth == 0 && resortShapes ) {
        sortShapes( playerDrawLater, stencil, stencilCount );
        resortShapes = false;
      }
      
      // draw walls behind the player
      for( int i = 0; i < stencilCount; i++ ) if( !(stencil[i].inFront) ) doDrawShape( &(stencil[i]) );

      // draw walls in front of the player and water effects
      glEnable( GL_BLEND );
      glDepthMask(GL_FALSE);        
      if( hasWater && 
          preferences->getStencilbuf() &&
          preferences->getStencilBufInitialized() ) {
        
        // stencil out the transparent walls (and draw them)
        //glDisable(GL_DEPTH_TEST);
        //glColorMask(0,0,0,0);
        glClear( GL_STENCIL_BUFFER_BIT );
        glEnable(GL_STENCIL_TEST);
        glStencilOp(GL_REPLACE, GL_REPLACE, GL_REPLACE);
        glStencilFunc(GL_ALWAYS, 1, 0xffffffff);
        // draw walls blended in front of the player
        // 6,2 6,4 work well
        // FIXME: blending walls have some artifacts that depth-sorting 
        // is supposed to get rid of but that didn't work for me.
        glBlendFunc( GL_SRC_ALPHA, GL_SRC_COLOR );
        for( int i = 0; i < stencilCount; i++ ) if( stencil[i].inFront ) doDrawShape( &(stencil[i]) );

        // draw the water (except where the transp. walls are)
        glStencilFunc(GL_NOTEQUAL, 1, 0xffffffff);
        glStencilOp(GL_KEEP, GL_KEEP, GL_KEEP); 
        glDisable(GL_TEXTURE_2D);
        glBlendFunc( GL_ONE, GL_SRC_COLOR );
        setupShapes(false, true);
        glEnable(GL_TEXTURE_2D);      
                
        glDisable(GL_STENCIL_TEST); 
      } else {
        // draw transp. walls and water w/o stencil buffer
        glBlendFunc( GL_SRC_ALPHA, GL_SRC_COLOR );
        for( int i = 0; i < stencilCount; i++ ) if( stencil[i].inFront ) doDrawShape( &(stencil[i]) );
        if( hasWater ) {
          glDisable(GL_TEXTURE_2D);
          glBlendFunc( GL_ONE, GL_SRC_COLOR );
          setupShapes(false, true);
          glEnable(GL_TEXTURE_2D);      
        }
      }
      glDisable( GL_BLEND );
      glDepthMask(GL_TRUE);

    } else {
      // no player; just draw the damn walls
      for( int i = 0; i < stencilCount; i++ ) doDrawShape( &(stencil[i]) );
      
      // draw water (has to come after walls to look good)
      if( hasWater ) {
        glEnable(GL_BLEND);  
        glDepthMask(GL_FALSE);
        glDisable(GL_TEXTURE_2D);
        glBlendFunc( GL_ONE, GL_SRC_COLOR );
        setupShapes(false, true);
        glDisable(GL_BLEND);
        glEnable(GL_TEXTURE_2D);      
        glDepthMask(GL_TRUE);
      }
    }

    // draw the effects
    glEnable(GL_TEXTURE_2D);
    glEnable(GL_BLEND);  
    glDepthMask(GL_FALSE);
    for(int i = 0; i < laterCount; i++) {
      later[i].shape->setupBlending();
      doDrawShape(&later[i]);
      later[i].shape->endBlending();
    }
    glBlendFunc(GL_SRC_ALPHA, GL_ONE);
    for(int i = 0; i < damageCount; i++) {
      doDrawShape(&damage[i], 1);
    }
    
    // draw the fog of war or shading
#ifdef USE_LIGHTING
    if( helper ) helper->draw( getX(), getY(), MVW, MVD );
#endif

    glDisable(GL_BLEND);

    glDepthMask(GL_TRUE);    

    drawProjectiles();
  }

  // find the map floor coordinate (must be done after drawing is complete)
	if( !selectMode ) {
		getMapXYAtScreenXY( &cursorFlatMapX, &cursorFlatMapY );    
		cursorChunkX = ( cursorFlatMapX - MAP_OFFSET ) / MAP_UNIT;
		cursorChunkY = ( cursorFlatMapY - MAP_OFFSET ) / MAP_UNIT;
	}

  if( settings->isGridShowing() ) willDrawGrid();
}

void Map::willDrawGrid() {

	glDisable( GL_CULL_FACE );
	glDisable( GL_TEXTURE_2D );

	glEnable(GL_BLEND);  
	glDepthMask(GL_FALSE);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE);

	// draw the starting position
	float xpos2 = (float)( this->startx - getX() ) / DIV;
	float ypos2 = (float)( this->starty - getY() - 1 ) / DIV;
	float zpos2 = 0.0f / DIV;
	float w = 2.0f /  DIV;
	float h = 4.0f /  DIV;
	if( useFrustum && 
			frustum->CubeInFrustum( xpos2, ypos2, 0.0f, w / DIV ) ) {
		for( int i = 0; i < 2; i++ ) {
			glPushMatrix();
			glTranslatef( xpos2, ypos2, zpos2 );
			if( i == 0 ) {
				glColor4f( 1, 0, 0, 0.5f );
				glBegin( GL_TRIANGLES );
			} else {
				glColor4f( 1, 0.7, 0, 0.5f );
				glBegin( GL_LINE_LOOP );
			}

			glVertex3f( 0, 0, 0 );
			glVertex3f( -w, -w, h );
			glVertex3f( w, -w, h );

			glVertex3f( 0, 0, 0 );
			glVertex3f( -w, w, h );
			glVertex3f( w, w, h );

			glVertex3f( 0, 0, 0 );
			glVertex3f( -w, -w, h );
			glVertex3f( -w, w, h );

			glVertex3f( 0, 0, 0 );
			glVertex3f( w, -w, h );
			glVertex3f( w, w, h );




			glVertex3f( 0, 0, h * 2 );
			glVertex3f( -w, -w, h );
			glVertex3f( w, -w, h );

			glVertex3f( 0, 0, h * 2 );
			glVertex3f( -w, w, h );
			glVertex3f( w, w, h );

			glVertex3f( 0, 0, h * 2 );
			glVertex3f( -w, -w, h );
			glVertex3f( -w, w, h );

			glVertex3f( 0, 0, h * 2 );
			glVertex3f( w, -w, h );
			glVertex3f( w, w, h );

			glEnd();
			glPopMatrix();
		}
	}

	int chunkX = ( cursorFlatMapX - MAP_OFFSET ) / MAP_UNIT;
	int chunkY = ( cursorFlatMapY - MAP_OFFSET - 1 ) / MAP_UNIT;
	float m = 0.5f / DIV;

	for(int i = 0; i < chunkCount; i++) {

		float n = (float)MAP_UNIT / DIV;

		glPushMatrix();
		glTranslatef( chunks[i].x, chunks[i].y - ( 1.0f / DIV ), 0 );

		if( chunks[i].cx == chunkX &&
				chunks[i].cy == chunkY ) {
			glColor4f( 0,1,0,1 );
			glLineWidth( 5 );
		} else {
			glColor4f( 1,1,1,1 );
			glLineWidth( 1 );
		}
		glBegin( GL_LINE_LOOP );
		glVertex3f( 0, 0, m );
		glVertex3f( n, 0, m );
		glVertex3f( n, n, m );
		glVertex3f( 0, n, m );
		glEnd();

		glPopMatrix();
	}

	glPushMatrix();

	float xp = (float)(cursorFlatMapX - getX()) / DIV;
	float yp = ((float)(cursorFlatMapY - getY()) - 1.0f) / DIV;
	float cw = (float)cursorWidth / DIV;
	float cd = -(float)cursorDepth / DIV;
	m = ( cursorZ ? cursorZ : 0.5f ) / DIV;
	float ch = (float)( cursorHeight + cursorZ ) / DIV;

	float red = 1.0f;
	float green = 0.9f;
	float blue = 0.15f;
	bool found = false;
	if( cursorFlatMapX < MAP_WIDTH && cursorFlatMapY < MAP_DEPTH ) {
		for( int xx = cursorFlatMapX; xx < cursorFlatMapX + cursorWidth; xx++ ) {
			for( int yy = cursorFlatMapY - 1; yy >= cursorFlatMapY - cursorDepth; yy-- ) {
				for( int zz = 0; zz < cursorHeight; zz++ ) {
					if( pos[xx][yy + 1][zz] ) {
						found = true;
						break;
					}
				}
			}
		}
	}
	if( found ) {
		green = 0.15f;
	}

	glColor4f( red, green, blue, 0.5f );
	glTranslatef( xp, yp, 0 );
	glBegin( GL_QUADS );

	glVertex3f( 0, 0, m );
	glVertex3f( cw, 0, m );
	glVertex3f( cw, cd, m );
	glVertex3f( 0, cd, m );

	glVertex3f( 0, 0, ch );
	glVertex3f( cw, 0, ch );
	glVertex3f( cw, cd, ch );
	glVertex3f( 0, cd, ch );

	glVertex3f( 0, 0, m );
	glVertex3f( cw, 0, m );
	glVertex3f( cw, 0, ch );
	glVertex3f( 0, 0, ch );

	glVertex3f( 0, cd, m );
	glVertex3f( cw, cd, m );
	glVertex3f( cw, cd, ch );
	glVertex3f( 0, cd, ch );

	glVertex3f( 0, 0, m );
	glVertex3f( 0, cd, m );
	glVertex3f( 0, cd, ch );
	glVertex3f( 0, 0, ch );

	glVertex3f( cw, 0, m );
	glVertex3f( cw, cd, m );
	glVertex3f( cw, cd, ch );
	glVertex3f( cw, 0, ch );

	glEnd();
	glPopMatrix();

	glEnable( GL_CULL_FACE );
	glEnable( GL_TEXTURE_2D );

	glDisable(GL_BLEND);  
	glDepthMask(GL_TRUE);
}

void Map::sortShapes( DrawLater *playerDrawLater,
                      DrawLater *shapes,
                      int shapeCount ) {
  GLdouble mm[16];
  glGetDoublev( GL_MODELVIEW_MATRIX, mm );
  GLdouble pm[16];
  glGetDoublev( GL_PROJECTION_MATRIX, pm );
  GLint vp[4];
  glGetIntegerv( GL_VIEWPORT, vp );

  GLdouble playerWinX, playerWinY, playerWinZ;
  gluProject( playerDrawLater->xpos,
              playerDrawLater->ypos,
              0,
              mm, pm, vp,
              &playerWinX, &playerWinY, &playerWinZ );

  char tmp[80];
  map< string, bool > cache;
  GLdouble objX, objY;
  GLdouble wallWinX, wallWinY, wallWinZ;
  for(int i = 0; i < shapeCount; i++) {
    /*
      Since rooms are rectangular, we can do this hack... a wall horizontal 
      wall piece will use the player's x coord. and its y coord.
      A vertical one will use its X and the player's Y.
      This is so that even if the wall extends below the player the entire
      length has the same characteristics.
    */
    if( shapes[i].shape->getWidth() > shapes[i].shape->getDepth() ) {
      objX = playerDrawLater->xpos;
      objY = shapes[i].ypos;
    } else {
      objX = shapes[i].xpos;
      objY = playerDrawLater->ypos;
    }
    bool b;
    sprintf( tmp, "%f,%f", objX, objY );
    string key = tmp;
    if( cache.find( key ) == cache.end() ) {
      gluProject( objX, objY, 0,
                  mm, pm, vp,
                  &wallWinX, &wallWinY, &wallWinZ );
      b = ( wallWinY < playerWinY );
      cache[ key ] = b;
    } else {
      b = cache[ key ];
    }
    shapes[i].inFront = b;
  }
}

void Map::drawProjectiles() {
  for( map<RenderedCreature*, vector<RenderedProjectile*>*>::iterator i = RenderedProjectile::getProjectileMap()->begin(); 
       i != RenderedProjectile::getProjectileMap()->end(); 
       ++i ) {
    //RenderedCreature *creature = i->first;
    vector<RenderedProjectile*> *p = i->second;
    for (vector<RenderedProjectile*>::iterator e=p->begin(); e!=p->end(); ++e) {
      RenderedProjectile *proj = *e;
            
      // calculate the path
      vector<CVector3> path;
			for( int i = 0; i < proj->getStepCount(); i++ ) {
				CVector3 v;
				v.x = ( ( proj->getX( i ) + proj->getRenderer()->getOffsetX() - (float)getX() ) / DIV );
				v.y = ( ( proj->getY( i ) - proj->getRenderer()->getOffsetY() - (float)getY() - 1.0f ) / DIV );
				v.z = ( proj->getZ( i ) + proj->getRenderer()->getOffsetZ() ) / DIV;
				path.push_back( v );
			}
			proj->getRenderer()->drawPath( this, proj, &path );
    }
  }
}

void Map::doDrawShape(DrawLater *later, int effect) {
    doDrawShape(later->xpos, later->ypos, later->zpos, later->shape, later->name, effect, later);
}

void Map::doDrawShape(float xpos2, float ypos2, float zpos2, Shape *shape, 
					  GLuint name, int effect, DrawLater *later) {

  // fog test for creatures
  if( helper && later && later->creature && 
      !helper->isVisible( later->pos->x, 
                          later->pos->y, 
                          later->creature->getShape() ) ) {
    return;
  }

  if(shape) ((GLShape*)shape)->useShadow = useShadow;

  // slow on mac os X:
  // glPushAttrib(GL_ENABLE_BIT);

  glPushMatrix();
	float heightPos = ( later && later->pos ? later->pos->heightPos / DIV : ( later->effect ? later->effect->heightPos : 0 ) );
  if(useShadow) {
		// put shadow above the floor a little
		
		glTranslatef( xpos2, ypos2, ( 0.26f / DIV + heightPos ) );
		glMultMatrixf(shadowTransformMatrix);

    // gray shadows
    //glColor4f( 0, 0, 0, 0.5f );

    // purple shadows
    glColor4f( 0.04f, 0, 0.07f, 0.6f );    

    // debugging red
    //glColor4f(1, 0, 0, 0.5f);
  } else {

    if(shape) shape->setupToDraw();
		
    glTranslatef( xpos2, ypos2, zpos2 + heightPos );


#ifdef DEBUG_SECRET_DOORS    
    if( later && later->pos ) {
      int xp = later->pos->x;
      int yp = later->pos->y;
      int index = xp + MAP_WIDTH * yp;
      if( secretDoors.find( index ) != secretDoors.end() ) {
        glColor4f(1, 0.3f, 0.3f, 1.0f);
        colorAlreadySet = true;
      }
    }
#endif
    if( colorAlreadySet   ) {
      colorAlreadySet = false;
    } else {
      if(later && later->pos && 
         isLocked(later->pos->x, 
                  later->pos->y, 
                  later->pos->z)) {
        glColor4f(1, 0.3f, 0.3f, 1.0f);
      } else {
        //glColor4f(0.72f, 0.65f, 0.55f, 0.5f);
        glColor4f(1, 1, 1, 0.9f);
      }
    }
  }

  glDisable( GL_CULL_FACE );

  // encode this shape's map location in its name
  glPushName( name );
  if(shape) {
    ((GLShape*)shape)->setCameraRot(xrot, yrot, zrot);
    ((GLShape*)shape)->setCameraPos(xpos, ypos, zpos, xpos2, ypos2, zpos2);
    if(later && later->pos) ((GLShape*)shape)->setLocked(isLocked(later->pos->x, later->pos->y, 0));
    else ((GLShape*)shape)->setLocked(false);
  }
  if( effect && later ) {
    if( later->creature ) {
      // translate hack for md2 models... see: md2shape::draw()
      //glTranslatef( 0, -1 / DIV, 0 );
      later->creature->getEffect()->draw(later->creature->getEffectType(),
                                         later->creature->getDamageEffect());
      //glTranslatef( 0, 1 / DIV, 0 );
    } else if(later->effect) {
      later->effect->getEffect()->draw(later->effect->getEffectType(),
                                       later->effect->getDamageEffect());
    }
  } else if( later && later->creature && !useShadow ) {
    if(later->creature->getStateMod(StateMod::invisible)) {
      glColor4f(0.3, 0.8f, 1.0f, 1.0f);    
    } else if(later->creature->getStateMod(StateMod::possessed)) {
      glColor4f(1.0, 0.3f, 0.8f, 1.0f);    
    }
    
    // outline mission creatures
    if( adapter->isMissionCreature( later->creature ) ) {
    //if( session->getCurrentMission() &&
        //session->getCurrentMission()->isMissionCreature( later->creature ) ) {
      shape->outline( 0.15f, 0.15f, 0.4f );
    } else if( later && later->pos && 
				later->pos->outlineColor ) {
			shape->outline( later->pos->outlineColor );
		}
		shape->draw();
  } else if( later && later->item && !useShadow ) {
    
    if( later->item->isSpecial() ) {
      shape->outline( Constants::SPECIAL_ITEM_COLOR );
    } else if( later->item->isMagicItem() ) {
      shape->outline( Constants::MAGIC_ITEM_COLOR[ later->item->getMagicLevel() ] );
    } else if( later->item->getContainsMagicItem() ) {
      shape->outline( 0.8f, 0.8f, 0.3f );
    }

    if( later && later->pos && 
        later->pos->outlineColor &&
        !useShadow ) 
      shape->outline( later->pos->outlineColor );
    shape->draw();

    

  } else {
		if( later && later->pos && 
				later->pos->outlineColor &&
				!useShadow ) 
			shape->outline( later->pos->outlineColor );
		shape->draw();
  }
  glPopName();
  glPopMatrix();
  
  // slow on mac os X
  // glPopAttrib();

  if(shape) ((GLShape*)shape)->useShadow = false;
}                                                                     

bool Map::isOnScreen(Uint16 mapx, Uint16 mapy, Uint16 mapz) {
  glPushMatrix();
  
  // Initialize the scene w/o y rotation.
  initMapView(true);
  
  double obj_x = (mapx - getX() + 1) / DIV;
  double obj_y = (mapy - getY() - 2) / DIV;
  double obj_z = 0.0f;
  //double obj_z = mapz / DIV;
  double win_x, win_y, win_z;
  
  double projection[16];
  double modelview[16];
  GLint viewport[4];
  
  glGetDoublev(GL_PROJECTION_MATRIX, projection);
  glGetDoublev(GL_MODELVIEW_MATRIX, modelview);
  glGetIntegerv(GL_VIEWPORT, viewport);
  
  int res = gluProject(obj_x, obj_y, obj_z,
                       modelview,
                       projection,
                       viewport,
                       &win_x, &win_y, &win_z);
  
  glDisable( GL_SCISSOR_TEST );
  glPopMatrix();
  
  if(res) {
    win_y = adapter->getScreenHeight() - win_y;
    return (win_x >= 0 && win_x < adapter->getScreenWidth() &&
            win_y >= 0 && win_y < adapter->getScreenHeight());
  } return false;
}

/*
void Map::showInfoAtMapPos(Uint16 mapx, Uint16 mapy, Uint16 mapz, char *message) {
  float xpos2 = ((float)(mapx - getX()) / DIV);
  float ypos2 = ((float)(mapy - getY()) / DIV);
  float zpos2 = (float)(mapz) / DIV;
  glTranslatef( xpos2, ypos2, zpos2 + 100);

  //glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
  //glRasterPos2f( 0, 0 );
  scourge->getSDLHandler()->texPrint(0, 0, "%s", message);

  glTranslatef( -xpos2, -ypos2, -(zpos2 + 100));
}
*/

/**
 * Initialize the map view (translater, rotate)
 */
void Map::initMapView( bool ignoreRot ) {
  glLoadIdentity();

  glTranslatef(viewX, viewY, 0);
  glScissor(viewX, 
            adapter->getScreenHeight() - (viewY + viewHeight),
            viewWidth, viewHeight);
  glEnable( GL_SCISSOR_TEST );
  // adjust for screen size
  float adjust = (float)viewWidth / 800.0f;
  glScalef(adjust, adjust, adjust);

  glScalef(zoom, zoom, zoom);
  
  // translate the camera and rotate
  // the offsets ensure that the center of rotation is under the player
  glTranslatef( this->xpos, this->ypos, 0);  
  if( !ignoreRot ) {
    glRotatef( xrot, 0.0f, 1.0f, 0.0f );
    glRotatef( yrot, 1.0f, 0.0f, 0.0f );  
    glRotatef( zrot, 0.0f, 0.0f, 1.0f );
  }
  glTranslatef( 0, 0, this->zpos);  

  // adjust for centered-map movement
  float xdiff = 0;
  float ydiff = 0;
  if( settings->isPlayerEnabled() && ( preferences->getAlwaysCenterMap() || mapCenterCreature ) ) {
    RenderedCreature *c = ( mapCenterCreature ? mapCenterCreature : adapter->getPlayer() );
    if( c ) {
      xdiff = ( c->getX() - (float)( toint( c->getX() ) ) );
      ydiff = ( c->getY() - (float)( toint( c->getY() ) ) );
		}
  }
  float startx = -( (float)mapViewWidth / 2.0 + ( mapx - (float)x + xdiff ) ) / DIV;
  float starty = -( (float)mapViewDepth / 2.0 + ( mapy - (float)y + ydiff ) ) / DIV;
  float startz = 0.0;

  glTranslatef( startx, starty, startz );

	if( quakesEnabled ) {
		Uint32 now = SDL_GetTicks();

		// is it time to quake again?
		if( now >= nextQuakeStartTime ) {
			nextQuakeStartTime = 
				now +
				QUAKE_DELAY + 
				(int)( ( QUAKE_DELAY / 2.0f ) * rand() / RAND_MAX );
			// start a quake unless this is the very first time
			quakeStartTime = ( quakeStartTime == 0 ? nextQuakeStartTime : now );
			if( quakeStartTime == now ) adapter->addDescription( "A tremor shakes the earth..." );
		}

		// is it quaking now?
		if( now - quakeStartTime < QUAKE_DURATION ) {
			if( now - lastQuakeTick >= QUAKE_TICK_FREQ ) {
				quakeOffsX = (3.0f * rand() / RAND_MAX ) / DIV;
				quakeOffsY = (3.0f * rand() / RAND_MAX ) / DIV;
				lastQuakeTick = now;				
			}
		} else {
			quakeOffsX = quakeOffsY = 0;
		}

		glTranslatef( quakeOffsX, quakeOffsY, 0 );
	}
}

Location *Map::moveCreature(Sint16 x, Sint16 y, Sint16 z, Uint16 dir,RenderedCreature *newCreature) {
	Sint16 nx = x;
	Sint16 ny = y;
	Sint16 nz = z;
	switch(dir) {
	case Constants::MOVE_UP: ny--; break;
	case Constants::MOVE_DOWN: ny++; break;
	case Constants::MOVE_LEFT: nx--; break;
	case Constants::MOVE_RIGHT: nx++; break;
	}
	return moveCreature(x, y, z, nx, ny, nz, newCreature);
}

Location *Map::moveCreature(Sint16 x, Sint16 y, Sint16 z, 
                            Sint16 nx, Sint16 ny, Sint16 nz,
                            RenderedCreature *newCreature) {

  // no need to actually move data
  if( x == nx && y == ny && z == nz ) {
    resortShapes = mapChanged = true;
    return NULL;
  }

  //float interX, interY;
  Location *position = isBlocked(nx, ny, nz, x, y, z, newCreature->getShape());
  if(position) return position;

  // move position
  moveCreaturePos(nx, ny, nz, x, y, z, newCreature);

  return NULL;
}

void Map::setRugPosition( Sint16 xchunk, Sint16 ychunk, Rug *rug ) {
	memcpy( &(rugPos[ xchunk ][ ychunk ]), rug, sizeof( Rug ) );
}

void Map::setFloorPosition(Sint16 x, Sint16 y, Shape *shape) {
  floorPositions[x][y] = shape;
  WaterTile *w = (WaterTile*)malloc(sizeof(WaterTile));
  for( int xp = 0; xp < WATER_TILE_X; xp++ ) {
    for( int yp = 0; yp < WATER_TILE_Y; yp++ ) {
      w->z[xp][yp] = ( (2.0f * WATER_AMP) * rand()/RAND_MAX ) - WATER_AMP;
      w->step[xp][yp] = WATER_STEP * ((int)(2.0f * rand()/RAND_MAX) == 0 ? 1 : -1);
      w->lastTime[xp][yp] = 0;
    }
  }
  water[createPairKey(x, y)] = w;
}

void Map::removeRugPosition( Sint16 xchunk, Sint16 ychunk ) {
	rugPos[ xchunk ][ ychunk ].texture = 0;
}

Shape *Map::removeFloorPosition(Sint16 x, Sint16 y) {
	Shape *shape = NULL;
  if(floorPositions[x][y]) {
    shape = floorPositions[x][y];
    floorPositions[x][y] = 0;
  }
  Uint32 key = createPairKey(x, y);
  if( water.find(key) != water.end() ) {
    WaterTile *w = water[key];
    free(w);
    water.erase( key );
  }
	return shape;
}

Location *Map::isBlocked( Sint16 x, Sint16 y, Sint16 z, 
													Sint16 shapeX, Sint16 shapeY, Sint16 shapeZ, 
													Shape *s, 
													int *newz,
													bool useItemPos ) {
  int zz = z;
  for(int sx = 0; sx < s->getWidth(); sx++) {
    for(int sy = 0; sy < s->getDepth(); sy++) {

			if( fabs( getGroundHeight( ( x + sx ) / OUTDOORS_STEP, ( y - sy ) / OUTDOORS_STEP ) ) > 10.0f ) {
				return hackBlockingPos;
			}

      // find the lowest location where this item fits
      int sz = z;
      while(sz < zz + s->getHeight()) {
        Location *loc = pos[x + sx][y - sy][z + sz];
        if(loc && loc->shape && 
           !(loc->x == shapeX && loc->y == shapeY && loc->z == shapeZ)) {
          if(newz && (loc->item || loc->creature)) {
            int tz = loc->z + loc->shape->getHeight();
            if(tz > zz) zz = tz;
            if(zz + s->getHeight() >= MAP_VIEW_HEIGHT) {
              return pos[x + sx][y - sy][z + sz];
            }
            if(zz > sz) sz = zz;
            else break;
          } else if(newz && loc) {
            return pos[x + sx][y - sy][z + sz];
          } else if(!newz && !(loc && loc->item && !loc->item->isBlocking())) {
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
	if( useItemPos && !zz ) {
		for(int sx = 0; sx < s->getWidth(); sx++) {
			for(int sy = 0; sy < s->getDepth(); sy++) {
				Location *loc = itemPos[x + sx][y - sy];
				if( loc && !( loc->x == shapeX && loc->y == shapeY ) ) {
					return loc;
				}
			}
		}
	}

	if(newz) *newz = zz;
  return NULL;
}

Location *Map::getPosition(Sint16 x, Sint16 y, Sint16 z) {
  if(pos[x][y][z] &&
     ((pos[x][y][z]->shape &&
      pos[x][y][z]->x == x &&
      pos[x][y][z]->y == y &&
      pos[x][y][z]->z == z))) return pos[x][y][z];
  return NULL;
}

void Map::startEffect(Sint16 x, Sint16 y, Sint16 z, 
                      int effect_type, GLuint duration, 
                      int width, int height, GLuint delay, 
                      bool forever, DisplayInfo *di ) {

  if( x >= MAP_WIDTH || y >= MAP_DEPTH || z >= MAP_VIEW_HEIGHT ) {
    cerr << "*** STARTEFFECT out of bounds: pos=" << x << "," << y << "," << z << endl;
    ((RenderedCreature*)NULL)->getName();
  }

  // show an effect
  if(effect[x][y][z]) return;

  effect[x][y][z] = mapMemoryManager->newEffectLocation( this, preferences, shapes, width, height );
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

  if( di ) effect[x][y][z]->effect->setDisplayInfo( di );

  // need to do this to make sure effect shows up
  resortShapes = mapChanged = true;
}

void Map::removeEffect(Sint16 x, Sint16 y, Sint16 z) {

  if( x >= MAP_WIDTH || y >= MAP_DEPTH || z >= MAP_VIEW_HEIGHT ) {
    cerr << "*** REMOVEEFFECT out of bounds: pos=" << x << "," << y << "," << z << endl;
    ((RenderedCreature*)NULL)->getName();
  }

  if(effect[x][y][z]) {
    /*
    if(effect[x][y][z]->effect) {
      delete effect[x][y][z]->effect;
      effect[x][y][z]->effect = NULL;
    }
    delete effect[x][y][z];
    */
    mapMemoryManager->deleteEffectLocation( effect[x][y][z] );
    effect[x][y][z] = NULL;
  }
}

void Map::removeAllEffects() {
  for( int x = 0; x < MAP_WIDTH; x++ ) {
    for( int y = 0; y < MAP_DEPTH; y++ ) {
      for( int z = 0; z < MAP_VIEW_HEIGHT; z++ ) {
        if( effect[x][y][z] ) {
          /*
          if(effect[x][y][z]->effect) {
            delete effect[x][y][z]->effect;
            effect[x][y][z]->effect = NULL;
          }
          delete effect[x][y][z];
          */
          mapMemoryManager->deleteEffectLocation( effect[x][y][z] );
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

	float zz;
	if( findMax ) {
		// find the max
		zz = 0;
		for( int i = 0; i < 4; i++ ) {
			if( zz < ground[ debugHeightPosXX[ i ] ][ debugHeightPosYY[ i ] ] ) {
				zz = ground[ debugHeightPosXX[ i ] ][ debugHeightPosYY[ i ] ];
			}
		}
	} else {
		// find the average
		zz = 0;
		int count = 0;
		for( int i = 0; i < 4; i++ ) {
			// skip 'lake' heights
			if( ground[ debugHeightPosXX[ i ] ][ debugHeightPosYY[ i ] ] > 0 ) {
				zz += ground[ debugHeightPosXX[ i ] ][ debugHeightPosYY[ i ] ];
				count++;
			}
		}
		zz /= (float)count;
	}
	if( z < zz ) pos = zz;
	return pos;
}

void Map::setPositionInner( Sint16 x, Sint16 y, Sint16 z, 
														Shape *shape, 
														RenderedItem *item, 
														RenderedCreature *creature ) {
	
	resortShapes = mapChanged = true;
	//cerr << "FIXME: Map::setPosition" << endl;

	bool isNonBlockingItem = ( item && !item->isBlocking() && !z && settings->isItemPosEnabled() );
	Location *p = ( isNonBlockingItem ? itemPos[ x ][ y ] : pos[ x ][ y ][ z ] );
	if( !p ) p = mapMemoryManager->newLocation();
	p->shape = shape;
	p->item = item;
	p->creature = creature;
	p->heightPos = findMaxHeightPos( x, y, z );
	p->x = x;
	p->y = y;
	p->z = z;
	p->outlineColor = NULL;
	

	for(int xp = 0; xp < shape->getWidth(); xp++) {
		for(int yp = 0; yp < shape->getDepth(); yp++) {
			for(int zp = 0; zp < shape->getHeight(); zp++) {
				
				// I _hate_ c++... moving secret doors up causes array roll-over problems.
				if( x + xp < 0 || 
						y - yp < 0 || 
						z + zp < 0 ||
						x + xp >= MAP_WIDTH || 
						y - yp >= MAP_DEPTH || 
						z + zp >= MAP_VIEW_HEIGHT ) break;

				// Either the same old pos reused or nothing there.
				// If these are not true, we're leaking memory.
				//assert( pos[x + xp][y - yp][z + zp] == p ||
								//!( pos[x + xp][y - yp][z + zp] ) );

				if( zp &&
						pos[x + xp][y - yp][z + zp] && 
						pos[x + xp][y - yp][z + zp] != p ) {
					cerr << "error setting position:" << 
						" x=" << ( x + xp ) <<
						" y=" << ( y - yp ) <<
						" z=" << ( z + zp ) <<
						" shape=" << p->shape->getName() << endl;
				} else {
					if( isNonBlockingItem ) 
						itemPos[x + xp][y - yp] = p;
					else
						pos[x + xp][y - yp][z + zp] = p;
				}
			}
		}
	}
}

void Map::setPosition( Sint16 x, Sint16 y, Sint16 z, Shape *shape, DisplayInfo *di ) {
  if(shape) {
		
		setPositionInner( x, y, z, shape, NULL, NULL );

    if( ((GLShape*)shape)->getEffectType() > -1 ) {

      int ex = x + ((GLShape*)shape)->getEffectX();
      int ey = y - shape->getDepth() - ((GLShape*)shape)->getEffectY();
      int ez = z + ((GLShape*)shape)->getEffectZ();

      if( !effect[ex][ey][ez] ) {
        startEffect( ex, ey, ez,
                     ((GLShape*)shape)->getEffectType(),
                     0, 
                     ((GLShape*)shape)->getEffectWidth(), 
                     ((GLShape*)shape)->getEffectDepth(), 
                     0, true, di );
      }
    }
  }
}

Shape *Map::removePosition(Sint16 x, Sint16 y, Sint16 z) {
	Shape *shape = NULL;
	if(pos[x][y][z] &&
		 pos[x][y][z]->shape &&
		 pos[x][y][z]->x == x &&
		 pos[x][y][z]->y == y &&
		 pos[x][y][z]->z == z) {
		resortShapes = mapChanged = true;
		shape = pos[x][y][z]->shape;
		if( ((GLShape*)shape)->getEffectType() > -1 ) {
			int ex = x + ((GLShape*)shape)->getEffectX();
			int ey = y - shape->getDepth() - ((GLShape*)shape)->getEffectY();
			int ez = z + ((GLShape*)shape)->getEffectZ();
			removeEffect( ex, ey, ez );
		}

		Location *p = pos[ x ][ y ][ z ];

    for(int xp = 0; xp < shape->getWidth(); xp++) {
      for(int yp = 0; yp < shape->getDepth(); yp++) {
        for(int zp = 0; zp < shape->getHeight(); zp++) {

          // I _hate_ c++... moving secret doors up causes array roll-over problems.
          if( x + xp < 0 || 
							y - yp < 0 || 
							z + zp < 0 ||
              x + xp >= MAP_WIDTH || 
							y - yp >= MAP_DEPTH || 
							z + zp >= MAP_VIEW_HEIGHT ) break;

					// Assert we're dealing with the right shape
					//assert( pos[ x + xp ][ y - yp ][ z + zp ] == p );
					if( pos[ x + xp ][ y - yp ][ z + zp ] != p ) {
						cerr << "Error removing position:" <<
							" x=" << ( x + xp ) << 
							" y=" << ( y - yp ) << 
							" z=" << ( z + zp ) << endl;
					} else {
						pos[ x + xp ][ y - yp ][ z + zp ] = NULL;          
					}
        }
      }
    }

		// Actually free the shape
		mapMemoryManager->deleteLocation( p );
  }
  return shape;
}

Shape *Map::removeItemPosition( Sint16 x, Sint16 y ) {
	Shape *shape = NULL;
	if( itemPos[x][y] &&
			itemPos[x][y]->shape &&
			itemPos[x][y]->x == x &&
			itemPos[x][y]->y == y ) {
		resortShapes = mapChanged = true;
		shape = itemPos[x][y]->shape;
		if( ((GLShape*)shape)->getEffectType() > -1 ) {
			int ex = x + ((GLShape*)shape)->getEffectX();
			int ey = y - shape->getDepth() - ((GLShape*)shape)->getEffectY();
			int ez = ((GLShape*)shape)->getEffectZ();
			removeEffect( ex, ey, ez );
		}

		Location *p = itemPos[ x ][ y ];

    for(int xp = 0; xp < shape->getWidth(); xp++) {
      for(int yp = 0; yp < shape->getDepth(); yp++) {

				// I _hate_ c++... moving secret doors up causes array roll-over problems.
				if( x + xp < 0 || 
						y - yp < 0 || 
						x + xp >= MAP_WIDTH || 
						y - yp >= MAP_DEPTH ) break;

				// Assert we're dealing with the right shape
				//assert( pos[ x + xp ][ y - yp ][ z + zp ] == p );
				if( itemPos[ x + xp ][ y - yp ] != p ) {
					cerr << "Error removing item position:" <<
						" x=" << ( x + xp ) << 
						" y=" << ( y - yp ) << endl;
				} else {
					itemPos[ x + xp ][ y - yp ] = NULL;          
				}
			}
		}

		// Actually free the shape
		mapMemoryManager->deleteLocation( p );
  }
  return shape;
}

// like getLocation, you can specify any position in the shape to remove it.
Shape *Map::removeLocation(Sint16 x, Sint16 y, Sint16 z) {
  if( pos[x][y][z] && pos[x][y][z]->shape ) 
    return removePosition( pos[x][y][z]->x,
                           pos[x][y][z]->y,
                           pos[x][y][z]->z );
  else return NULL;
}
  
void Map::setItem(Sint16 x, Sint16 y, Sint16 z, RenderedItem *item) {
  if( item && item->getShape() ) {
		setPositionInner( x, y, z, item->getShape(), item, NULL );
	}
}

RenderedItem *Map::removeItem(Sint16 x, Sint16 y, Sint16 z) {
	RenderedItem *item = NULL;
	if( !z && itemPos[x][y] && itemPos[x][y]->item ) {
		item = itemPos[x][y]->item;
		removeItemPosition( x, y );
	} else {
		item = ( pos[x][y][z] ? pos[x][y][z]->item : NULL );
		removePosition( x, y, z );
	}
	return item;
}

// drop items above this one
void Map::dropItemsAbove(int x, int y, int z, RenderedItem *item) {
	int count = 0;
	Location drop[100];
	for (int tx = 0; tx < item->getShape()->getWidth(); tx++) {
		for (int ty = 0; ty < item->getShape()->getDepth(); ty++) {
			for (int tz = z + item->getShape()->getHeight(); tz < MAP_VIEW_HEIGHT; tz++) {
				Location *loc2 = pos[x + tx][y - ty][tz];
				if (loc2 && loc2->item) {
					drop[count].x = loc2->x;
					drop[count].y = loc2->y;
					drop[count].z = loc2->z - item->getShape()->getHeight();
					drop[count].item = loc2->item;
					count++;
					removeItem(loc2->x, loc2->y, loc2->z);
					tz += drop[count - 1].item->getShape()->getHeight() - 1;
				}
			}
		}
	}
	for (int i = 0; i < count; i++) {
		//cerr << "item " << drop[i].item->getItemName() << " new z=" << drop[i].z << endl;
		setItem(drop[i].x, drop[i].y, drop[i].z, drop[i].item);
	}
}

void Map::setCreature(Sint16 x, Sint16 y, Sint16 z, RenderedCreature *creature) {
	char message[120];  
	if( creature && creature->getShape() ) {
		if ( helper && !creature->isMonster() )	helper->visit( creature );

		// pick up any objects in the way
		for( int xp = 0; xp < creature->getShape()->getWidth(); xp++ ) {
			for( int yp = 0; yp < creature->getShape()->getDepth(); yp++ ) {
				for( int zp = 0; zp < creature->getShape()->getHeight(); zp++ ) {
					if( pos[x + xp][y - yp][z + zp] && 
							pos[x + xp][y - yp][z + zp]->item ) {
						// creature picks up non-blocking item (this is the only way to handle 
						// non-blocking items. It's also very 'roguelike'.
						RenderedItem *item = pos[x + xp][y - yp][z + zp]->item;
						removeItem( pos[x + xp][y - yp][z + zp]->x,
												pos[x + xp][y - yp][z + zp]->y,
												pos[x + xp][y - yp][z + zp]->z );
						creature->pickUpOnMap( item );
						sprintf( message, "%s picks up %s.", 
										 creature->getName(), 
										 item->getItemName() );
						adapter->addDescription( message );        
					}
				}
			}
		}

		setPositionInner( x, y, z, creature->getShape(), NULL, creature );
	}
}

void Map::moveCreaturePos(Sint16 nx, Sint16 ny, Sint16 nz,
													Sint16 ox, Sint16 oy, Sint16 oz,
													RenderedCreature *creature) {
	Location *p = pos[ox][oy][oz];
	if( creature && creature->getShape() &&
			p && p->creature &&
			p->x == ox && p->y == oy && p->z == oz ) {
		resortShapes = mapChanged = true;

		// remove the old pos
		Location *tmp[MAP_UNIT][MAP_UNIT][MAP_UNIT];
		for (int xp = 0; xp < creature->getShape()->getWidth(); xp++) {
			for (int yp = 0; yp < creature->getShape()->getDepth(); yp++) {
				for (int zp = 0; zp < creature->getShape()->getHeight(); zp++) {
					int oldX = ox + xp;
					int oldY = oy - yp;
					int oldZ = oz + zp;
					tmp[xp][yp][zp] = pos[oldX][oldY][oldZ];
					tmp[xp][yp][zp]->outlineColor = NULL;
					pos[oldX][oldY][oldZ] = NULL;
					if (!(tmp[xp][yp][zp]))	cerr << "*** tmp is null!" << endl;
				}
			}
		}

		if( helper && !creature->isMonster() ) helper->visit( creature );

		// pick up any items in the way
		char message[120];
		for (int xp = 0; xp < creature->getShape()->getWidth(); xp++) {
			for (int yp = 0; yp < creature->getShape()->getDepth(); yp++) {
				for (int zp = 0; zp < creature->getShape()->getHeight(); zp++) {
					int newX = nx + xp;
					int newY = ny - yp;
					int newZ = nz + zp;            

					if (pos[newX][newY][newZ]) {
						if (pos[newX][newY][newZ]->item) {
							// creature picks up non-blocking item (this is the only way to handle 
							// non-blocking items. It's also very 'roguelike'.)
							RenderedItem *item = pos[newX][newY][newZ]->item;
							removeItem(pos[newX][newY][newZ]->x,
												 pos[newX][newY][newZ]->y,
												 pos[newX][newY][newZ]->z);
							creature->pickUpOnMap(item);
							sprintf(message, "%s picks up %s.", 
											creature->getName(), 
											item->getItemName());
							adapter->addDescription(message);
						} else {
							cerr << "*** Error: when moving " << creature->getName() << " path contained a non-item position." << endl;
						}
					}
				}    
			}
		}

		// insert the new pos
		for (int xp = 0; xp < creature->getShape()->getWidth(); xp++) {
			for (int yp = 0; yp < creature->getShape()->getDepth(); yp++) {
				for (int zp = 0; zp < creature->getShape()->getHeight(); zp++) {
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

	}
}

RenderedCreature *Map::removeCreature(Sint16 x, Sint16 y, Sint16 z) {
  RenderedCreature *creature = ( pos[x][y][z] ? pos[x][y][z]->creature : NULL );
	removePosition( x, y, z );
	return creature;
}

// FIXME: only uses x,y for now
// return false if there is any hole in the walls
bool Map::isWallBetweenShapes(int x1, int y1, int z1,
							  Shape *shape1,
							  int x2, int y2, int z2,
							  Shape *shape2) {
  for(int x = x1; x < x1 + shape1->getWidth(); x++) {
	for(int y = y1; y < y1 + shape1->getDepth(); y++) {
	  for(int xx = x2; xx < x2 + shape2->getWidth(); xx++) {
		for(int yy = y2; yy < y2 + shape2->getDepth(); yy++) {
      Shape *shape = isWallBetween(x, y, z1, xx, yy, z2);
      if( !shape || shape == shape2 ) return false;
		}
	  }
	}
  }
  return true;	  
}

// FIXME: only uses x,y for now
Shape *Map::isWallBetween(int x1, int y1, int z1,
                          int x2, int y2, int z2) {

  if(x1 == x2 && y1 == y2) return isWall(x1, y1, z1);
  if(x1 == x2) {
	if(y1 > y2) SWAP(y1, y2);
	for(int y = y1; y <= y2; y++) {
    Shape *shape = isWall(x1, y, z1);
	  if( shape ) return shape;
	}
	return false;
  }
  if(y1 == y2) {
	if(x1 > x2) SWAP(x1, x2);
	for(int x = x1; x <= x2; x++) {
    Shape *shape = isWall(x, y1, z1);
	  if( shape ) return shape;
	}
	return false;
  }
  

  //  fprintf(stderr, "Checking for wall: from: %d,%d to %d,%d\n", x1, y1, x2, y2);
  Shape *shape = NULL;
  bool yDiffBigger = (abs(y2 - y1) > abs(x2 - x1));
  float m = (float)(y2 - y1) / (float)(x2 - x1);
  int steps = (yDiffBigger ? abs(y2 - y1) : abs(x2 - x1));
  float x = x1;
  float y = y1;
  for(int i = 0; i < steps; i++) {
	//	fprintf(stderr, "\tat=%f,%f\n", x, y);
    Shape *ss = isWall((int)x, (int)y, z1);
	if( ss ) {
	  //fprintf(stderr, "wall at %f, %f\n", x, y);
	  shape = ss;
	  break;
	}
	if(yDiffBigger) {
	  if(y1 < y2) y += 1.0f;
	  else y -= 1.0f;
	  if(x1 < x2) x += 1.0f / abs((int)m);
	  else x += -1.0f / abs((int)m);
	} else {
	  if(x1 < x2) x += 1.0f;
	  else x -= 1.0f;
	  if(y1 < y2) y += abs((int)m);
	  else y += -1.0 * abs((int)m);
	}
  }
  //  fprintf(stderr, "wall in between? %s\n", (ret ? "TRUE": "FALSE"));
  return shape;
}

Shape *Map::isWall(int x, int y, int z) {
  Location *loc = getLocation((int)x, (int)y, z);
  return( loc && 
          (!loc->item || loc->item->getShape() != loc->shape) && 
          (!loc->creature || loc->creature->getShape() != loc->shape) ? loc->shape : NULL );
}                             

bool Map::shapeFits(Shape *shape, int x, int y, int z) {
  for( int tx = 0; tx < shape->getWidth(); tx++ ) {
		for( int ty = 0; ty < shape->getDepth(); ty++ ) {
			for( int tz = 0; tz < shape->getHeight(); tz++ ) {
				if( getLocation( x + tx, y - ty, z + tz ) ) {
					return false;
				}
			}
		}
	}
  return true;
}

// FIXME: only uses x, y for now
Location *Map::getBlockingLocation(Shape *shape, int x, int y, int z) {
  for(int tx = 0; tx < shape->getWidth(); tx++) {
	for(int ty = 0; ty < shape->getDepth(); ty++) {
	  Location *loc = getLocation(x + tx, y - ty, 0);
	  if(loc) return loc;
	}
  }
  return NULL;
}

/**
   Return the drop location, or NULL if none
 */
Location *Map::getDropLocation(Shape *shape, int x, int y, int z) {
  for(int tx = 0; tx < shape->getWidth(); tx++) {
	for(int ty = 0; ty < shape->getDepth(); ty++) {
	  Location *loc = getLocation(x + tx, y - ty, z);
	  if(loc) {
		return loc;
	  }
	}
  }
  return NULL;
}

// the world has changed...
void Map::configureLightMap() {
  lightMapChanged = false;

  // draw nothing at first
  for(int x = 0; x < MAP_WIDTH / MAP_UNIT; x++) {
    for(int y = 0; y < MAP_DEPTH / MAP_UNIT; y++) {
      lightMap[x][y] = ( LIGHTMAP_ENABLED && settings->isLightMapEnabled() ? 0 : 1 );
    }
  }
  if( !( LIGHTMAP_ENABLED && settings->isLightMapEnabled() ) ) return;

  int chunkX = (toint(adapter->getPlayer()->getX()) + 
                (adapter->getPlayer()->getShape()->getWidth() / 2) - 
                MAP_OFFSET) / MAP_UNIT;
  int chunkY = (toint(adapter->getPlayer()->getY()) - 
                (adapter->getPlayer()->getShape()->getDepth() / 2) - 
                MAP_OFFSET) / MAP_UNIT;

  traceLight(chunkX, chunkY, lightMap, false);
}

bool Map::isPositionAccessible(int atX, int atY) {
  // interpret the results: see if the target is "in light"
  int chunkX = (atX - MAP_OFFSET) / MAP_UNIT;
  int chunkY = (atY - MAP_OFFSET) / MAP_UNIT;
  return (accessMap[chunkX][chunkY]);
}

void Map::configureAccessMap(int fromX, int fromY) {
  // create the access map
  for(int x = 0; x < MAP_WIDTH / MAP_UNIT; x++) {
    for(int y = 0; y < MAP_DEPTH / MAP_UNIT; y++) {
      accessMap[x][y] = 0;
    }
  }
  int chunkX = (fromX - MAP_OFFSET) / MAP_UNIT;
  int chunkY = (fromY - MAP_OFFSET) / MAP_UNIT;
  traceLight(chunkX, chunkY, accessMap, true);
}

void Map::traceLight(int chunkX, int chunkY, int lm[MAP_WIDTH / MAP_UNIT][MAP_DEPTH / MAP_UNIT], bool onlyLockedDoors) {
  if(chunkX < 0 || chunkX >= MAP_WIDTH / MAP_UNIT ||
	 chunkY < 0 || chunkY >= MAP_DEPTH / MAP_UNIT)
	return;

  // already visited?
  if(lm[chunkX][chunkY]) return;

  // let there be light
  lm[chunkX][chunkY] = 1;
  
  // can we go N?
  int x, y;
  bool blocked = false;
  x = chunkX * MAP_UNIT + MAP_OFFSET + (MAP_UNIT / 2);
  for(y = chunkY * MAP_UNIT + MAP_OFFSET - (MAP_UNIT / 2);
	  y < chunkY * MAP_UNIT + MAP_OFFSET + (MAP_UNIT / 2);
	  y++) {
   	if(isLocationBlocked(x, y, 0, onlyLockedDoors)) {
	  blocked = true;
	  break;
	}
  }
  if(!blocked) traceLight(chunkX, chunkY - 1, lm, onlyLockedDoors);
  
  // can we go E?
  blocked = false;
  y = chunkY * MAP_UNIT + MAP_OFFSET + (MAP_UNIT / 2);
  for(x = chunkX * MAP_UNIT + MAP_OFFSET + (MAP_UNIT / 2);
	  x < chunkX * MAP_UNIT + MAP_OFFSET + (MAP_UNIT / 2) + MAP_UNIT;
	  x++) {
	if(isLocationBlocked(x, y, 0, onlyLockedDoors)) {
	  blocked = true;
	  break;
	}
  }
  if(!blocked) traceLight(chunkX + 1, chunkY, lm, onlyLockedDoors);
  
  // can we go S?
  blocked = false;
  x = chunkX * MAP_UNIT + MAP_OFFSET + (MAP_UNIT / 2);
  for(y = chunkY * MAP_UNIT + MAP_OFFSET + (MAP_UNIT / 2);
	  y < chunkY * MAP_UNIT + MAP_OFFSET + (MAP_UNIT / 2) + MAP_UNIT;
	  y++) {
	if(isLocationBlocked(x, y, 0, onlyLockedDoors)) {
	  blocked = true;
	  break;
	}
  }
  if(!blocked) traceLight(chunkX, chunkY + 1, lm, onlyLockedDoors);

  // can we go W?
  blocked = false;
  y = chunkY * MAP_UNIT + MAP_OFFSET + (MAP_UNIT / 2);
  for(x = chunkX * MAP_UNIT + MAP_OFFSET - (MAP_UNIT / 2);
	  x < chunkX * MAP_UNIT + MAP_OFFSET + (MAP_UNIT / 2);
	  x++) {
	if(isLocationBlocked(x, y, 0, onlyLockedDoors)) {
	  blocked = true;
	  break;
	}
  }
  if(!blocked) traceLight(chunkX - 1, chunkY, lm, onlyLockedDoors);
}

bool Map::isLocationBlocked(int x, int y, int z, bool onlyLockedDoors) {
  if(x >= 0 && x < MAP_WIDTH && 
     y >= 0 && y < MAP_DEPTH && 
		 z >= 0 && z < MAP_VIEW_HEIGHT) {
		Location *pos = getLocation(x, y, z);
		if(pos == NULL || pos->item || pos->creature) return false;
    if(onlyLockedDoors && isDoor(x, y)) 
      return isLocked(pos->x, pos->y, pos->z);
    if( pos && isSecretDoor( pos ) && pos->z > 0 ) return false;
    if(!((GLShape*)(pos->shape))->isLightBlocking()) return false;
	}
	return true;
}

void Map::drawCube(float x, float y, float z, float r) {
  glBegin(GL_QUADS);
  // front
  glNormal3f(0.0, 0.0, 1.0);
  glVertex3f(-r+x, -r+y, r+z);
  glVertex3f(r+x, -r+y, r+z);
  glVertex3f(r+x, r+y, r+z);
  glVertex3f(-r+x, r+y, r+z);
  
  // back
  glNormal3f(0.0, 0.0, -1.0);
  glVertex3f(r+x, -r+y, -r+z);
  glVertex3f(-r+x, -r+y, -r+z);
  glVertex3f(-r+x, r+y, -r+z);
  glVertex3f(r+x, r+y, -r+z);
  
  // top
  glNormal3f(0.0, 1.0, 0.0);
  glVertex3f(-r+x, r+y, r+z);
  glVertex3f(r+x, r+y, r+z);
  glVertex3f(r+x, r+y, -r+z);
  glVertex3f(-r+x, r+y, -r+z);
  
  // bottom
  glNormal3f(0.0, -1.0, 0.0);
  glVertex3f(-r+x, -r+y, -r+z);
  glVertex3f(r+x, -r+y, -r+z);
  glVertex3f(r+x, -r+y, r+z);
  glVertex3f(-r+x, -r+y, r+z);
  
  // left
  glNormal3f(-1.0, 0.0, 0.0);
  glVertex3f(-r+x, -r+y, -r+z);
  glVertex3f(-r+x, -r+y, r+z);
  glVertex3f(-r+x, r+y, r+z);
  glVertex3f(-r+x, r+y, -r+z);
  
  // right
  glNormal3f(1.0, 0.0, 0.0);
  glVertex3f(r+x, -r+y, r+z);
  glVertex3f(r+x, -r+y, -r+z);
  glVertex3f(r+x, r+y, -r+z);
  glVertex3f(r+x, r+y, r+z);
  
  glEnd();
  
}

/**
 * Find the creatures in this area and add them to the targets array.
 * Returns the number of creatures found. (0 if none.)
 * It's the caller responsibility to create the targets array.
 */
int Map::getCreaturesInArea(int x, int y, int radius, RenderedCreature *targets[]) {
  int count = 0;
  for(int xx = x - radius; xx < x + radius && xx < MAP_WIDTH; xx++) {
    for(int yy = y - radius; yy < y + radius && yy < MAP_DEPTH; yy++) {
      Location *loc = pos[xx][yy][0];      
      if(loc && loc->creature) {
        bool alreadyFound = false;
        for(int i = 0; i < count; i++) {
          if(targets[i] == loc->creature) {
            alreadyFound = true;
            break;
          }
        }
        if(!alreadyFound) {
          targets[count++] = loc->creature;
        }
      }
    }
  }
  return count;
}

bool Map::isDoor(int tx, int ty) {
  if(tx >= 0 && tx < MAP_WIDTH && 
     ty >= 0 && ty < MAP_DEPTH) {
    Location *loc = getLocation(tx, ty, 0);
    return(loc && isDoor(loc->shape));
  }
  return false;
}

bool Map::isDoor(Shape *shape) {
  return(shape == shapes->findShapeByName("EW_DOOR") ||
         shape == shapes->findShapeByName("NS_DOOR"));
}

void Map::setLocked( int doorX, int doorY, int doorZ, bool value ) {
  locked[createTripletKey(doorX, doorY, doorZ)] = value;
  //Location *p = pos[doorX][doorY][doorZ];
}

int Map::toggleLightMap() {
  LIGHTMAP_ENABLED = (LIGHTMAP_ENABLED ? 0 : 1);
  lightMapChanged = true;
  return LIGHTMAP_ENABLED;
} 

bool Map::isLocationVisible(int x, int y) { 
  return (x >= getX() && x < getX() + mapViewWidth &&
          y >= getY() && y < getY() + mapViewDepth);
}

bool Map::isLocationInLight( int x, int y, Shape *shape ) {
  int chunkX = (x - MAP_OFFSET) / MAP_UNIT;
  int chunkY = (y - (MAP_OFFSET + 1)) / MAP_UNIT;
  if( !lightMap[chunkX][chunkY] ) return false;
  return ( helper && helper->isVisible( x, y, shape ) );
}

/**
   Move and rotate map.
   Modifiers: 
   -CTRL + arrow keys / mouse at edge of screen: rotate map
   -arrow keys / mouse at edge of screen: move map fast
   -SHIFT + arrow keys / mouse at edge of screen: slow move map
   -SHIFT + CTRL + arrow keys / mouse at edge of screen: slow rotate map
 */
void Map::moveMap(int dir) {
  if (SDL_GetModState() & KMOD_CTRL) {
    float rot;
    if (SDL_GetModState() & KMOD_SHIFT) {
      rot = 1.5f;
    } else {
      rot = 5.0f;
    }
    if (dir & Constants::MOVE_DOWN) setYRot(-1.0f * rot);
    if (dir & Constants::MOVE_UP) setYRot(rot);
    if (dir & Constants::MOVE_RIGHT) setZRot(-1.0f * rot);
    if (dir & Constants::MOVE_LEFT) setZRot(rot);
  } else if ( !preferences->getAlwaysCenterMap() ) {

    // stop rotating (angle of rotation is kept)
    setYRot(0);
    setZRot(0);

		mapChanged = resortShapes = true;
		float delta = (SDL_GetModState() & KMOD_SHIFT ? 0.5f : 1.0f);
		if( mouseMove ) {

			// normalize z rot to 0-359
			float z = moveAngle - ( getZRot() - 90 );
			if (z < 0) z += 360;
			if (z >= 360) z -= 360;
			float zrad = Constants::toRadians(z);

			mapx += -moveDelta / 5.0f * sin(zrad);
			mapy += moveDelta / 5.0f * cos(zrad);
			moveDelta = 0; // reset to only move when the mouse is moved
		} else {
			
			// normalize z rot to 0-359
			float z = getZRot();
			if (z < 0) z += 360;
			if (z >= 360) z -= 360;
			float zrad = Constants::toRadians(z);

			//	cerr << "-------------------" << endl;
			//	cerr << "x=" << x << " y=" << y << " zrot=" << z << endl;
				
			if (dir & Constants::MOVE_DOWN) {
				mapx += delta * sin(zrad);
				mapy += delta * cos(zrad);
			}
			if (dir & Constants::MOVE_UP) {
				mapx += delta * -sin(zrad);
				mapy += delta * -cos(zrad);
			}
			if (dir & Constants::MOVE_LEFT) {
				mapx += delta * -cos(zrad);
				mapy += delta * sin(zrad);
			}
			if (dir & Constants::MOVE_RIGHT) {
				mapx += delta * cos(zrad);
				mapy += delta * -sin(zrad);
			}
		}

    //	cerr << "xdelta=" << xdelta << " ydelta=" << ydelta << endl;

    if (mapy > MAP_DEPTH - mapViewDepth) mapy = MAP_DEPTH - mapViewDepth;
    if (mapy < 0) mapy = 0;
    if (mapx > MAP_WIDTH - mapViewWidth) mapx = MAP_WIDTH - mapViewWidth;
    if (mapx < 0) mapx = 0;
    //	cerr << "mapx=" << mapx << " mapy=" << mapy << endl;

    x = (int)rint(mapx);
    y = (int)rint(mapy);
    //	cerr << "FINAL: x=" << x << " y=" << y << endl;

  }
}

void Map::handleEvent( SDL_Event *event ) {

  // turn off outlining
  if( lastOutlinedX < MAP_WIDTH ) {
    Location *pos = getLocation( lastOutlinedX, lastOutlinedY, lastOutlinedZ );
		if( !lastOutlinedZ && !pos ) pos = getItemLocation( lastOutlinedX, lastOutlinedY );
    if( pos ) {
      pos->outlineColor = NULL;
      lastOutlinedX = lastOutlinedY = lastOutlinedZ = MAP_WIDTH;
    }
  }

  int ea;
  int mx, my;
  switch(event->type) {
  case SDL_MOUSEMOTION:
    if(mouseRot) {
			adapter->setCursorVisible( false );
      setZRot( mouseRotDir * event->motion.xrel * MOUSE_ROT_DELTA);
      setYRot(-event->motion.yrel * MOUSE_ROT_DELTA);
		} else if( mouseMove ) {
			adapter->setCursorVisible( false );
			int q;
			moveDelta = sqrt( (float)( event->motion.xrel * event->motion.xrel ) + 
												(float)( event->motion.yrel * event->motion.yrel ) ) / zoom;
			Constants::getQuadrantAndAngle( event->motion.xrel, event->motion.yrel, 
																			&q, &moveAngle );
			mouseMoveScreen = true;
			setMove(Constants::MOVE_UP ); // so move != 0
    } else {
      //sdlHandler->applyMouseOffset(event->motion.x, event->motion.y, &mx, &my);
      mx = event->motion.x;
      my = event->motion.y;
      if(mx < 10) {
        mouseMoveScreen = true;
        setMove(Constants::MOVE_LEFT);
      } else if(mx >= adapter->getScreenWidth() - 10) {
        mouseMoveScreen = true;
        setMove(Constants::MOVE_RIGHT);
      } else if(my < 10) {
        mouseMoveScreen = true;
        setMove(Constants::MOVE_UP);
      } else if(my >= adapter->getScreenHeight() - 10) {
        mouseMoveScreen = true;
        setMove(Constants::MOVE_DOWN);
      } else {
        if(mouseMoveScreen) {
          mouseMoveScreen = false;
          removeMove(Constants::MOVE_LEFT | Constants::MOVE_RIGHT);
          removeMove(Constants::MOVE_UP | Constants::MOVE_DOWN);
          setYRot(0.0f);
          setZRot(0.0f);
        }
      }
      
      // highlight the item under the mouse if it's useable
      if( getCursorMapX() < MAP_WIDTH ) {
        Location *pos = getLocation( getCursorMapX(),
                                     getCursorMapY(),
                                     getCursorMapZ() );
				if( !pos ) pos = getItemLocation( getCursorMapX(),
																					getCursorMapY() );
        if( pos && preferences->isOutlineInteractiveItems() ) {
          Color *color = adapter->getOutlineColor( pos );
          if( color ) {
            pos->outlineColor = color;
            lastOutlinedX = pos->x;
            lastOutlinedY = pos->y;
            lastOutlinedZ = pos->z;
          }
        }
      }
    }
    break;
  case SDL_MOUSEBUTTONDOWN:
  if( event->button.button ) {
    if( event->button.button == SDL_BUTTON_MIDDLE ) {
      mouseRot = true;
			mouseRotDir = ( event->button.y - viewY < viewHeight / 2 ? 1 : -1 );
		} else if( event->button.button == SDL_BUTTON_RIGHT ) {
			mouseMove = true;
			mouseMoveX = event->button.x;
			mouseMoveY = event->button.y;
    } if( event->button.button == SDL_BUTTON_WHEELUP ) {
      mouseZoom = true;
      setZoomIn(false);
      setZoomOut(true);
    } if( event->button.button == SDL_BUTTON_WHEELDOWN ) {
      mouseZoom = true;
      setZoomIn(true);
      setZoomOut(false);
    }
  }
  break;  
	case SDL_MOUSEBUTTONUP:
		adapter->setCursorVisible( true );
		if( event->button.button ) {
			if( event->button.button == SDL_BUTTON_MIDDLE ||
					event->button.button == SDL_BUTTON_RIGHT ) {
				mouseRot = false;
				mouseMove = false;
				moveDelta = 0;
				setXRot(0);
				setYRot(0);
				setZRot(0);
				move = 0;
			}
		} 
		break;
  case SDL_KEYDOWN:
  case SDL_KEYUP:
    // xxx_yyy_stop means : "do xxx_yyy action when the corresponding key is up"
    ea = preferences->getEngineAction(event);    
    if(ea == SET_MOVE_DOWN){        
      setMove(Constants::MOVE_DOWN);
    } else if(ea == SET_MOVE_UP){
      setMove(Constants::MOVE_UP);
    } else if(ea == SET_MOVE_RIGHT){
      setMove(Constants::MOVE_RIGHT);
    } else if(ea == SET_MOVE_LEFT){
      setMove(Constants::MOVE_LEFT);
    } else if(ea == SET_MOVE_DOWN_STOP){        
      setYRot(0.0f);
      setYRot(0);
      removeMove(Constants::MOVE_DOWN);
    } else if(ea == SET_MOVE_UP_STOP){
      setYRot(0.0f);
      setYRot(0);
      removeMove(Constants::MOVE_UP);
    } else if(ea == SET_MOVE_RIGHT_STOP){
      setYRot(0.0f);
      setZRot(0);
      removeMove(Constants::MOVE_RIGHT);
    } else if(ea == SET_MOVE_LEFT_STOP){
      setYRot(0.0f);
      setZRot(0);
      removeMove(Constants::MOVE_LEFT);
    } else if(ea == SET_ZOOM_IN){
      setZoomIn(true);
    } else if(ea == SET_ZOOM_OUT){
      setZoomOut(true);
    } else if(ea == SET_ZOOM_IN_STOP){
      setZoomIn(false);
    } else if(ea == SET_ZOOM_OUT_STOP){
      setZoomOut(false);
    }
    break;
    default: break;
  }
}



GameMapSettings::GameMapSettings() {
}

GameMapSettings::~GameMapSettings() {
}

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

float GameMapSettings::getMinZoomIn() {
  return 0.5f;
}

float GameMapSettings::getMaxZoomOut() {
  return 2.8f;
}

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
void Map::saveMap( char *name, char *result, bool absolutePath, int referenceType ) {

  if( !strlen( name ) ) {
    strcpy( result, _( "You need to name the map first." ) );
    return;
  }

  MapInfo *info = (MapInfo*)malloc(sizeof(MapInfo));
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

  strncpy( (char*)info->theme_name, shapes->getCurrentThemeName(), 254 );
  info->theme_name[254] = 0;

  info->pos_count = 0;
  for( int x = 0; x < MAP_WIDTH; x++ ) {
    for( int y = 0; y < MAP_DEPTH; y++ ) {

      if( floorPositions[x][y] ) {
        info->pos[ info->pos_count ] = Persist::createLocationInfo( x, y, 0 );
        strncpy( (char*)(info->pos[ info->pos_count ]->floor_shape_name), 
                 floorPositions[x][y]->getName(),
                 254 );
        info->pos[ info->pos_count ]->floor_shape_name[254] = 0;
        info->pos_count++;
      }

			if( itemPos[x][y] &&
					itemPos[x][y]->x == x &&
					itemPos[x][y]->y == y &&
					itemPos[x][y]->item ) {
        info->pos[ info->pos_count ] = Persist::createLocationInfo( x, y, 0 );
				if( referenceType == REF_TYPE_NAME ) {
					strncpy( (char*)(info->pos[ info->pos_count ]->item_pos_name), 
									 itemPos[x][y]->item->getType(),
									 254 );
					info->pos[ info->pos_count ]->item_pos_name[254] = 0;
				} else {
					info->pos[ info->pos_count ]->item_pos = itemPos[x][y]->item->save();
				}
        info->pos_count++;
      }

      for( int z = 0; z < MAP_VIEW_HEIGHT; z++ ) {
        if( pos[x][y][z] &&
            pos[x][y][z]->x == x &&
            pos[x][y][z]->y == y &&
            pos[x][y][z]->z == z && 
            !( pos[x][y][z]->creature && 
               !(pos[x][y][z]->creature->isMonster() ) ) ) {

          info->pos[ info->pos_count ] = Persist::createLocationInfo( x, y, z );

          if( pos[x][y][z]->item ) {
						if( referenceType == REF_TYPE_NAME ) {
							strncpy( (char*)( info->pos[ info->pos_count ]->item_name ), 
											 pos[x][y][z]->item->getType(), 
											 254 );
						} else {
							info->pos[ info->pos_count ]->item = pos[x][y][z]->item->save();
						}
          } else if( pos[x][y][z]->creature ) {
						if( referenceType == REF_TYPE_NAME ) {
							strncpy( (char*)( info->pos[ info->pos_count ]->monster_name ), 
											 pos[x][y][z]->creature->getType(), 
											 254 );
							info->pos[ info->pos_count ]->monster_name[254] = 0;
						} else {
							info->pos[ info->pos_count ]->creature = pos[x][y][z]->creature->save();
						}
          } else {
            strncpy( (char*)(info->pos[ info->pos_count ]->shape_name), 
                     pos[x][y][z]->shape->getName(),
                     254 );
            info->pos[ info->pos_count ]->shape_name[254] = 0;
          }

					// save the deity locations
					char *p = adapter->getMagicSchoolIndexForLocation( pos[x][y][z] );
					if( p ) {
						strncpy( (char*)info->pos[ info->pos_count ]->magic_school_name, p, 254 );
						info->pos[ info->pos_count ]->magic_school_name[254] = 0;
					}

          info->pos_count++;
        }
      }
    }
  }

	// save rugs
	info->rug_count = 0;
	for( int x = 0; x < MAP_WIDTH / MAP_UNIT; x++ ) {
    for( int y = 0; y < MAP_DEPTH / MAP_UNIT; y++ ) {
			if( rugPos[x][y].texture > 0 ) {
				info->rugPos[ info->rug_count ] = Persist::createRugInfo( x, y );
				info->rugPos[ info->rug_count ]->angle = toint( rugPos[x][y].angle * 100.0f );
				info->rugPos[ info->rug_count ]->isHorizontal = ( rugPos[x][y].isHorizontal ? 1 : 0 );
				info->rugPos[ info->rug_count ]->texture = rugPos[x][y].texture;
				info->rug_count++;
			}
		}
	}

	// save doors
	info->locked_count = 0;
  for( map<Uint32, bool>::iterator i = locked.begin(); i != locked.end(); ++i ) {
		Uint32 key = i->first;
		Uint8 value = (Uint8)(i->second ? 1 : 0);
		info->locked[ info->locked_count++ ] = Persist::createLockedInfo( key, value );
	}
	info->door_count = 0;
  for( map<Uint32, Uint32>::iterator i = doorToKey.begin(); i != doorToKey.end(); ++i ) {
		Uint32 key = i->first;
		Uint32 value = i->second;
		info->door[ info->door_count++ ] = Persist::createDoorInfo( key, value );
	}

	// secret doors
	info->secret_count = 0;
  for( map<int, bool>::iterator i = secretDoors.begin(); i != secretDoors.end(); ++i ) {
		Uint32 key = (Uint32)(i->first);
		Uint8 value = (Uint8)(i->second ? 1 : 0);
		info->secret[ info->secret_count++ ] = Persist::createLockedInfo( key, value );
	}

	// save the fog
	if( helper ) helper->saveHelper( &(info->fog_info) );
		 
	info->edited = edited;

	// save ground height for outdoors maps
	info->heightMapEnabled = (Uint8)( heightMapEnabled ? 1 : 0 );
	for( int gx = 0; gx < MAP_WIDTH / OUTDOORS_STEP; gx++ ) {
		for( int gy = 0; gy < MAP_DEPTH / OUTDOORS_STEP; gy++ ) {
			Uint32 base = ( ground[ gx ][ gy ] < 0 ? NEG_GROUND_HEIGHT : 0x00000000 );
			info->ground[ gx ][ gy ] = (Uint32)( fabs( ground[ gx ][ gy ] ) * 100 ) + base;
		}
	}

  char fileName[300];
	if( absolutePath ) {
		strcpy( fileName, name );
	} else {
		sprintf( fileName, "%s/maps/%s.map", rootDir, name );
	}

  cerr << "saving map: " << fileName << endl;

  FILE *fp = fopen( fileName, "wb" );
  File *file = new ZipFile( fp, ZipFile::ZIP_WRITE );
  Persist::saveMap( file, info );
  delete file;

  Persist::deleteMapInfo( info );

	// save npc info when saving from map editor (ie. map templates)
	if( referenceType == REF_TYPE_NAME ) {
		adapter->saveMapData( (const char*)fileName );
	}

  sprintf( result, "Map saved: %s", name );
}

void Map::initForCave( char *themeName ) {
	if( !themeName ) {
		shapes->loadRandomCaveTheme();
	} else {
		shapes->loadCaveTheme( themeName );
	}

  string ref = WallTheme::themeRefName[ WallTheme::THEME_REF_PASSAGE_FLOOR ];
  GLuint *floorTextureGroup = shapes->getCurrentTheme()->getTextureGroup( ref );
  setFloor( CAVE_CHUNK_SIZE, CAVE_CHUNK_SIZE, floorTextureGroup[ GLShape::TOP_SIDE ] );
}

bool Map::loadMap( char *name, char *result, StatusReport *report, 
									 int level, int depth, 
									 bool changingStory, bool fromRandom, 
									 vector< RenderedItem* > *items, 
									 vector< RenderedCreature* > *creatures, 
									 bool absolutePath,
									 char *templateMapName ) {
  if( !strlen( name ) ) {
    strcpy( result, _( "Enter a name of a map to load." ) );
    return false;
  }

  char fileName[300];
	if( absolutePath ) {
		strcpy( fileName, name );
	} else {
		if( depth > 0 ) {
			sprintf( fileName, "%s/maps/%s%d.map", rootDir, name, depth );
		} else {
			sprintf( fileName, "%s/maps/%s.map", rootDir, name );
		}
	}
  cerr << "Looking for map: " << fileName << endl;

  FILE *fp = fopen( fileName, "rb" );
  if( !fp ) {
    sprintf( result, "Can't find map: %s", name );
    return false;
  }
  if( report ) report->updateStatus( 0, 7, _( "Loading map" ) );
  File *file = new ZipFile( fp, ZipFile::ZIP_READ );
  MapInfo *info = Persist::loadMap( file );
  delete file;

  if( report ) report->updateStatus( 1, 7, _( "Loading theme" ) );

  // reset the map
  reset();

	if( info->map_type == MapRenderHelper::ROOM_HELPER ) {
		// it's a room-type map
		setMapRenderHelper( MapRenderHelper::helpers[ MapRenderHelper::ROOM_HELPER ] );
		// load the theme
		if( getPreferences()->isDebugTheme() ) shapes->loadDebugTheme();
		else shapes->loadTheme( (const char*)info->theme_name );
	} else if( info->map_type == MapRenderHelper::CAVE_HELPER ) {
		// it's a room-type map
		setMapRenderHelper( MapRenderHelper::helpers[ MapRenderHelper::CAVE_HELPER ] );

		// prepare map for cave (load theme, etc.)
		initForCave( (char*)info->theme_name );

		// load the fog
		getMapRenderHelper()->loadHelper( &(info->fog_info) );
	} else if( info->map_type == MapRenderHelper::OUTDOOR_HELPER ) {
		// it's an outdoors type map
		setMapRenderHelper( MapRenderHelper::helpers[ MapRenderHelper::OUTDOOR_HELPER ] );
		// FIXME: needed but not used
		if( getPreferences()->isDebugTheme() ) shapes->loadDebugTheme();
		else shapes->loadRandomTheme();
	} else {
		cerr << "*** error unknown map type: " << info->map_type << endl;
		return false;
	}

	edited = info->edited;

	// load ground heights for outdoors maps
	heightMapEnabled = ( info->heightMapEnabled == 1 );
	for( int gx = 0; gx < MAP_WIDTH / OUTDOORS_STEP; gx++ ) {
		for( int gy = 0; gy < MAP_DEPTH / OUTDOORS_STEP; gy++ ) {
			if( info->ground[ gx ][ gy ] > NEG_GROUND_HEIGHT ) {
				ground[ gx ][ gy ] = ( info->ground[ gx ][ gy ] - NEG_GROUND_HEIGHT ) / -100.0f;
			} else {
				ground[ gx ][ gy ] = info->ground[ gx ][ gy ] / 100.0f;
			}
			
		}
	}
	if( heightMapEnabled ) {
		initOutdoorsGroundTexture();
	}



	setHasWater( info->hasWater == 1 ? true : false );

  if( report ) report->updateStatus( 2, 7, _( "Starting map" ) );

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

  mapGridX = info->grid_x;
  mapGridY = info->grid_y;

  if( report ) report->updateStatus( 3, 7, _( "Initializing map" ) );
  
  GLShape *shape;
	DisplayInfo di;
  for( int i = 0; i < (int)info->pos_count; i++ ) {

    if( info->pos[i]->x >= MAP_WIDTH ||
        info->pos[i]->y >= MAP_DEPTH ||
        info->pos[i]->z >= MAP_VIEW_HEIGHT ) {
      cerr << "*** Skipping invalid map location: " << info->pos[i]->x << "," << info->pos[i]->y << "," << info->pos[i]->z << endl;
      continue;
    }

    if( strlen( (char*)(info->pos[i]->floor_shape_name) ) ) {
      shape = shapes->
        findShapeByName( (char*)(info->pos[i]->floor_shape_name), true );
      if( shape ) setFloorPosition( info->pos[i]->x, info->pos[i]->y, shape );
      else cerr << "Map::load failed to find floor shape: " << info->pos[i]->floor_shape_name <<
        " at pos: " << info->pos[i]->x << "," << info->pos[i]->y << endl;
    }

		if( strlen( (char*)(info->pos[i]->item_pos_name) ) || info->pos[i]->item_pos ) {
			RenderedItem *item;
			if( info->reference_type == REF_TYPE_NAME ) {
				item = adapter->createItem( (char*)( info->pos[i]->item_pos_name ), level, depth );
			} else {
				item = adapter->createItem( info->pos[i]->item_pos );
			}
			if( item ) {
				setItem( info->pos[i]->x, info->pos[i]->y, 0, item );
				if( items ) items->push_back( item );
			} else cerr << "Map::load failed to item at pos: " << info->pos[i]->x << "," << info->pos[i]->y << ",0" << endl;
		}

		// load the deity locations
		MagicSchool *ms = NULL;
		if( strlen( (char*)info->pos[i]->magic_school_name ) ) {
			ms = MagicSchool::getMagicSchoolByName( (char*)info->pos[i]->magic_school_name );
      di.red = ms->getDeityRed();
      di.green = ms->getDeityGreen();
      di.blue = ms->getDeityBlue();
		}

    if( strlen( (char*)( info->pos[i]->item_name ) ) || info->pos[i]->item ) {
			RenderedItem *item;
			if( info->reference_type == REF_TYPE_NAME ) {
				item = adapter->createItem( (char*)( info->pos[i]->item_name ), level, depth );
			} else {
				item = adapter->createItem( info->pos[i]->item );
			}
      if( item ) {
        setItem( info->pos[i]->x, info->pos[i]->y, info->pos[i]->z, item );
        if( items ) items->push_back(  item );
      } else cerr << "Map::load failed to item at pos: " << info->pos[i]->x << "," << info->pos[i]->y << "," << info->pos[i]->z << endl;
    } else if( strlen( (char*)( info->pos[i]->monster_name ) ) || info->pos[i]->creature ) {
			RenderedCreature *creature;
			if( info->reference_type == REF_TYPE_NAME ) {
				creature = adapter->createMonster( (char*)( info->pos[i]->monster_name ) );
			} else {
				creature = adapter->createMonster( info->pos[i]->creature );
			}
      if( creature ) {
        setCreature( info->pos[i]->x, info->pos[i]->y, info->pos[i]->z, creature );
        creature->moveTo( info->pos[i]->x, info->pos[i]->y, info->pos[i]->z );
        if( creatures ) creatures->push_back(  creature );
      } else cerr << "Map::load failed to creature at pos: " << info->pos[i]->x << "," << info->pos[i]->y << "," << info->pos[i]->z << endl;
    } else if( strlen( (char*)(info->pos[i]->shape_name) ) ) {
      shape = shapes->
        findShapeByName( (char*)(info->pos[i]->shape_name), true );
      if( shape ) setPosition( info->pos[i]->x, info->pos[i]->y, info->pos[i]->z, shape, ( ms ? &di : NULL ) );
      else cerr << "Map::load failed to find shape: " << info->pos[i]->shape_name <<
        " at pos: " << info->pos[i]->x << "," << info->pos[i]->y << "," << info->pos[i]->z << endl;
    }

    // load the deity locations
		if( ms ) {
			Location *pos = getPosition( info->pos[i]->x, 
																	 info->pos[i]->y, 
																	 info->pos[i]->z );
			if( !pos ) {
				cerr << "*** error: Can't find position to place deity!" << endl;
			} else {
				adapter->setMagicSchoolIndexForLocation( pos, ms->getName() );
			}
		}
  }

	// load rugs
	Rug rug;
	for( int i = 0; i < (int)info->rug_count; i++ ) {
		//rug.angle = info->rugPos[i]->angle / 100.0f;
		rug.angle = ( 30.0f * rand() / RAND_MAX ) - 15.0f; // fixme?
		rug.isHorizontal = ( info->rugPos[i]->isHorizontal == 1 );
		rug.texture = shapes->getRandomRug(); // fixme?
		setRugPosition( info->rugPos[i]->cx,
										info->rugPos[i]->cy,
										&rug );
	}

	// load doors
	for( int i = 0; i < (int)info->locked_count; i++ ) {
		locked[ info->locked[ i ]->key ] = ( info->locked[ i ]->value == 1 ? true : false );
	}
	for( int i = 0; i < (int)info->door_count; i++ ) {
		doorToKey[ info->door[ i ]->key ] = info->door[ i ]->value;
		keyToDoor[ info->door[ i ]->value ] = info->door[ i ]->key;
	}

	// secret doors
	for( int i = 0; i < (int)info->secret_count; i++ ) {
		secretDoors[ (int)(info->secret[ i ]->key) ] = ( info->secret[ i ]->value == 1 ? true : false );
	}

  if( report ) report->updateStatus( 4, 7, _( "Finishing map" ) );

  this->center( info->start_x, info->start_y, true );

  Persist::deleteMapInfo( info );

  if( report ) report->updateStatus( 5, 7, _( "Loading Creatures" ) );

  // load map-related data from text file
  char txtfileName[300];
	if( templateMapName ) {
		if( depth > 0 ) {
			sprintf( txtfileName, "%s/maps/%s%d.map", rootDir, templateMapName, depth );
		} else {
			sprintf( txtfileName, "%s/maps/%s.map", rootDir, templateMapName );
		}
	} else {
		strcpy( txtfileName, fileName );
	}
  cerr << "Looking for txt file: " << txtfileName << endl;
  adapter->loadMapData( (const char*)txtfileName );
  
  strcpy( this->name, ( templateMapName ? templateMapName : name ) );

  if( report ) report->updateStatus( 6, 7, _( "Starting party" ) );

  /* 
    FIXME: Place the party at the start. This code attempts to find a place
    for the party near the gate (which was set in Scourge::useGate.) Need to 
    find a better AI solution as this code can place the party outside the walls.
    For now always leave "whitespace" around gates in edited levels.
  */
  if( settings->isPlayerEnabled() ) {
    int xx = startx;
    int yy = starty;
    int nx, ny;
    for( int t = 0; t < adapter->getPartySize(); t++ ) {
      if( !adapter->getParty(t)->getStateMod( StateMod::dead ) ) {
        adapter->getParty(t)->findPlace( xx, yy, &nx, &ny );
        xx = nx;
        yy = ny;
      }
    }
  }
  sprintf( result, _( "Map loaded: %s" ), name );
  return true;
}

void Map::loadMapLocation( char *name, char *result, int *gridX, int *gridY, int depth ) {
  Uint16 tmpX, tmpY;
  if( !strlen( name ) ) {
    strcpy( result, _( "Enter a name of a map to load." ) );
    return;
  }

  char fileName[300];
  if( depth > 0 ) {
    sprintf( fileName, "%s/maps/%s%d.map", rootDir, name, depth );
  } else {
    sprintf( fileName, "%s/maps/%s.map", rootDir, name );
  }
  cerr << "loading map header: " << fileName << endl;

  FILE *fp = fopen( fileName, "rb" );
  if( !fp ) {
    sprintf( result, _( "Can't find map: %s" ), name );
    return;
  }
  File *file = new ZipFile( fp, ZipFile::ZIP_READ );
  Persist::loadMapHeader( file, &tmpX, &tmpY );
  delete file;

  *gridX = (int)tmpX;
  *gridY = (int)tmpY;

  sprintf( result, _( "Map header loaded: %s" ), name );
}


void Map::getMapXYAtScreenXY( Uint16 *mapx, Uint16 *mapy ) {
  getMapXYAtScreenXY( adapter->getMouseX(), 
                      adapter->getMouseY(), 
                      mapx, 
                      mapy );
}

void Map::getMapXYAtScreenXY(Uint16 x, Uint16 y,
                             Uint16 *mapx, Uint16 *mapy) {
  double obj_x, obj_y, obj_z;
  double win_x = (double)x;
  double win_y = (double)( adapter->getScreenHeight() - y );
  
  // get the depth buffer value
  float win_z;
  glReadPixels( (int)win_x, (int)win_y, 1, 1, GL_DEPTH_COMPONENT, GL_FLOAT, &win_z );

  double projection[16];
  double modelview[16];
  GLint viewport[4];

  glGetDoublev(GL_PROJECTION_MATRIX, projection);
  glGetDoublev(GL_MODELVIEW_MATRIX, modelview);
  glGetIntegerv(GL_VIEWPORT, viewport);
  
  int res = gluUnProject(win_x, win_y, win_z,
                         modelview,
                         projection,
                         viewport,
                         &obj_x, &obj_y, &obj_z);
  
  if(res) {
    *mapx = getX() + (Uint16)( obj_x * DIV );
    *mapy = getY() + (Uint16)( obj_y * DIV );
    
    debugX = getX() + (int)( obj_x * DIV );
    debugY = getY() + (int)( obj_y * DIV );
    debugZ = (int)( obj_z * DIV );
  } else {
    *mapx = *mapy = MAP_WIDTH + 1;
  }
}   

void Map::getMapXYZAtScreenXY(Uint16 *mapx, Uint16 *mapy, Uint16 *mapz) {
  // only do this if the mouse has moved some (optimization)
//  if(abs(lastX - x) < POSITION_SAMPLE_DELTA && abs(lastY - y) < POSITION_SAMPLE_DELTA) {
//    *mapx = lastMapX;
//    *mapy = lastMapY;
//    *mapz = lastMapZ;
//    return;
//  }

  GLuint buffer[512];
  GLint  hits, viewport[4];

  glGetIntegerv(GL_VIEWPORT, viewport);
  glSelectBuffer(512, buffer);
  glRenderMode(GL_SELECT);
  glInitNames();
  glPushName(0);

  glMatrixMode(GL_PROJECTION);
  glPushMatrix();
  glLoadIdentity();
  gluPickMatrix(adapter->getMouseX(), 
                viewport[3] - adapter->getMouseY(), 
                1, 1, viewport);
  adapter->setView();

  glMatrixMode(GL_MODELVIEW);
  //levelMap->selectMode = true;
  draw();
  //levelMap->selectMode = false;

  glFlush();    
  hits = glRenderMode(GL_RENDER);
  //cerr << "hits=" << hits << endl;
  if(hits > 0) {           // If There Were More Than 0 Hits
    int choose = buffer[4];         // Make Our Selection The First Object
    int depth = buffer[1];          // Store How Far Away It Is

    for(int loop = 0; loop < hits; loop++) {   // Loop Through All The Detected Hits

      //            fprintf(stderr, "\tloop=%d 0=%u 1=%u 2=%u 3=%u 4=%u \n", loop, 
      //                    buffer[loop*5+0], buffer[loop*5+1], buffer[loop*5+2], 
      //                    buffer[loop*5+3],  buffer[loop*5+4]);
      if(buffer[loop*5+4] > 0) {
        decodeName(buffer[loop*5+4], mapx, mapy, mapz);
      }

      // If This Object Is Closer To Us Than The One We Have Selected
      if(buffer[loop*5+1] < GLuint(depth)) {
        choose = buffer[loop*5+4];        // Select The Closer Object
        depth = buffer[loop*5+1];     // Store How Far Away It Is
      }
    }

    //cerr << "choose=" << choose << endl;
    decodeName(choose, mapx, mapy, mapz);
  } else {
    *mapx = *mapy = MAP_WIDTH + 1;
  }

  // Restore the projection matrix
  glMatrixMode(GL_PROJECTION);
  glPopMatrix();

  // Go back to modelview for normal rendering
  glMatrixMode(GL_MODELVIEW);

  /*
  debugX = *mapx;
  debugY = *mapy;
  debugZ = *mapz;
  */
  /*
  lastMapX = *mapx;
  lastMapY = *mapy;
  lastMapZ = *mapz;
  lastX = getSDLHandler()->mouseX;
  lastY = getSDLHandler()->mouseY;
  */
}

void Map::decodeName(int name, Uint16* mapx, Uint16* mapy, Uint16* mapz) {
    char *s;
    if(name > 0) {
        // decode the encoded map coordinates
        *mapz = name / (MAP_WIDTH * MAP_DEPTH);
        if(*mapz > 0)
            name %= (MAP_WIDTH * MAP_DEPTH);
        *mapx = name % MAP_WIDTH;
        *mapy = name / MAP_WIDTH;
        Location *pos = getPosition(*mapx, *mapy, 0);
        if(pos) {
            if(pos->shape) s = pos->shape->getName();
            else if(pos->item && pos->item->getShape()) {
                s = pos->item->getShape()->getName();
            }
        } else s = NULL;
		//        fprintf(stderr, "\tmap coordinates: pos null=%s shape null=%s item null=%s %u,%u,%u name=%s\n", 
		//                (pos ? "no" : "yes"), (pos && pos->shape ? "no" : "yes"), (pos && pos->item ? "no" : "yes"), *mapx, *mapy, *mapz, (s ? s : "NULL"));
    } else {
        *mapx = MAP_WIDTH + 1;
        *mapy = 0;
        *mapz = 0;
		//        fprintf(stderr, "\t---\n");
    }
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
  for( int i = 0; i < (int)unused.size(); i++ ) {
    delete unused[i];
  }
  unused.clear();
  for( int i = 0; i < (int)unusedEffect.size(); i++ ) {
    delete unusedEffect[i];
  }
  unusedEffect.clear();  
}
  
Location *MapMemoryManager::newLocation() {
  Location *pos;
  if( unused.size() ) {
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
  
  return pos;
}

EffectLocation *MapMemoryManager::newEffectLocation( Map *theMap, Preferences *preferences, Shapes *shapes, int width, int height ) {
  EffectLocation *pos;
  if( unusedEffect.size() ) {
    pos = unusedEffect[ unusedEffect.size() - 1 ];
    unusedEffect.pop_back();
    pos->effect->reset();
  } else {
    pos = new EffectLocation();
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
  
void MapMemoryManager::deleteLocation( Location *pos ) {
  if( !maxSize || (int)unused.size() < maxSize ) {
    unused.push_back( pos );
  } else {
    delete pos;
  }
  usedCount--;  
  printStatus();
}

void MapMemoryManager::deleteEffectLocation( EffectLocation *pos ) {
  if( !maxSize || (int)unusedEffect.size() < maxSize ) {
    unusedEffect.push_back( pos );
  } else {
    delete pos;
  }
  usedEffectCount--;  
  printStatus();
}

void MapMemoryManager::printStatus() {
  if( ++accessCount > 5000 ) {
      cerr << "Map size: " << usedCount << " Kb:" << ( (float)(sizeof(Location)*usedCount)/1024.0f ) <<
      " Cache: " << unused.size() << " Kb:" << ( (float)(sizeof(Location)*unused.size())/1024.0f ) << endl;
      cerr << "Effect size: " << usedEffectCount << " Kb:" << ( (float)(sizeof(EffectLocation)*usedEffectCount)/1024.0f ) <<
      " Cache: " << unusedEffect.size() << " Kb:" << ( (float)(sizeof(EffectLocation)*unusedEffect.size())/1024.0f ) << endl;
      accessCount = 0;
  }
}
  
void Map::setMapRenderHelper( MapRenderHelper *helper ) {
  this->helper = helper;
  this->helper->setMap( this );
  LIGHTMAP_ENABLED = this->helper->isLightMapEnabled();
	// diff. default angle in outdoors
	if( !helper->drawShadow() ) this->yrot = settings->getMaxYRot() - 1;
  //lightMapChanged = true;
}

void Map::addSecretDoor( int x, int y ) {
  int index = y * MAP_WIDTH + x;
  secretDoors[ index ] = false;
}

bool Map::isSecretDoor( Location *pos ) {
  return isSecretDoor( pos->x, pos->y );
}
  
bool Map::isSecretDoor( int x, int y ) {
  return( secretDoors.find( y * MAP_WIDTH + x ) != secretDoors.end() ? true : false );
}

bool Map::isSecretDoorDetected( Location *pos ) {
  return isSecretDoorDetected( pos->x, pos->y );
}

bool Map::isSecretDoorDetected( int x, int y ) {
  int index = y * MAP_WIDTH + x;
  if( secretDoors.find( index ) != secretDoors.end() ) {
    return secretDoors[ index ];
  } else {
    return false;
  }
}

void Map::setSecretDoorDetected( Location *pos ) {
  setSecretDoorDetected( pos->x, pos->y );
}

void Map::setSecretDoorDetected( int x, int y ) {
  int index = y * MAP_WIDTH + x;
  if( secretDoors.find( index ) != secretDoors.end() ) {
    secretDoors[ index ] = true;
  }
}

void Map::renderFloor() {
	//float xpos2 = (float)(getX() - mapViewWidth / 2) / DIV;
	//float ypos2 = (float)(getY() - mapViewDepth / 2) / DIV;
	if( settings->isGridShowing() ) {
		glDisable( GL_TEXTURE_2D );
		glColor4f( 0, 0, 0, 0.9f );
	} else {
		glEnable( GL_TEXTURE_2D );
		glColor4f(1, 1, 1, 0.9f);
		glBindTexture( GL_TEXTURE_2D, floorTex );
	}
	glPushMatrix();

	if( isHeightMapEnabled() ) {
		drawHeightMapFloor();
		drawWaterLevel();
	}	else {
		drawFlatFloor();
	}
	glPopMatrix();

	// show floor in map editor
	if( settings->isGridShowing() ) {
		setupShapes(true, false);
	}
}

void Map::drawHeightMapFloor() {
	glDisable( GL_CULL_FACE );
	glColor4f( 1, 1, 1, 1 );
	CVectorTex *p[4];
	float gx, gy;
	
	for( int yy = ( getY() / OUTDOORS_STEP ); yy < ( ( getY() + mapViewDepth ) / OUTDOORS_STEP ) - 1; yy++ ) {
		for( int xx = ( getX() / OUTDOORS_STEP ); xx < ( ( getX() + mapViewWidth ) / OUTDOORS_STEP ) - 1; xx++ ) {
			glBindTexture( GL_TEXTURE_2D, groundPos[ xx ][ yy ].tex );

			p[0] = &( groundPos[ xx ][ yy + 1 ] );
			p[1] = &( groundPos[ xx ][ yy ] );
			p[2] = &( groundPos[ xx + 1 ][ yy ] );
			p[3] = &( groundPos[ xx + 1 ][ yy + 1 ] );
			glBegin( GL_QUADS );
			for( int i = 0; i < 4; i++ ) {
				glTexCoord2f( p[i]->u, p[i]->v );
				glColor4f( p[i]->r, p[i]->g, p[i]->b, p[i]->a );
				gx = p[i]->x - getX() / DIV;
				gy = p[i]->y - getY() / DIV;
				glVertex3f( gx, gy, p[i]->z );
			}
			glEnd();
		}
	}	
}

void Map::drawGroundTex( GLuint tex, float tx, float ty, float tw, float th, bool debug, float angle ) {

	//glEnable( GL_DEPTH_TEST );
	glDepthMask( GL_FALSE );
	glEnable( GL_BLEND );
	glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );
	glDisable( GL_CULL_FACE );
	glEnable( GL_TEXTURE_2D );
	glBindTexture( GL_TEXTURE_2D, tex );

	//glColor4f( 1, 0, 0, 1 );
	//glDepthMask( GL_FALSE );
	//glDisable( GL_DEPTH_TEST );

	// which ground pos?
	float sx = ( tx / (float)OUTDOORS_STEP );
	float sy = ( ( ty - th - 1 ) / (float)OUTDOORS_STEP );
	float ex = ( ( tx + tw ) / (float)OUTDOORS_STEP );
	float ey = ( ( ty - 1 ) / (float)OUTDOORS_STEP );
	if( debug ) cerr << "s=" << sx << "," << sy << " e=" << ex << "," << ey << endl;

	// offset to our texture inside the ground pos
	float offSX = tx - ( sx * OUTDOORS_STEP );
	float offSY = ( ty - th - 1 ) - ( sy * OUTDOORS_STEP );
	float offEX = offSX + tw;
	float offEY = offSY + th;
	if( debug ) {
		cerr << "tex size=" << ( ( ex - sx ) * OUTDOORS_STEP ) << "," << ( ( ey - sy ) * OUTDOORS_STEP ) << " player size=" << tw << endl;
		cerr << "tex=" << ( sx * OUTDOORS_STEP ) << "," << ( sy * OUTDOORS_STEP ) << " player=" << adapter->getPlayer()->getX() << "," << adapter->getPlayer()->getY() << endl;
		cerr << "offs: " << offSX << "," << offSY << " " << offEX << "," << offEY << endl;
	}

	// converted to texture coordinates ( 0-1 )
	offSX = -offSX / ( ( ex - sx ) * OUTDOORS_STEP );
	offSY = -offSY / ( ( ey - sy ) * OUTDOORS_STEP );
	offEX = 1 - ( offEX / ( ( ex - sx ) * OUTDOORS_STEP ) ) + 1;
	offEY = 1 - ( offEY / ( ( ey - sy ) * OUTDOORS_STEP ) ) + 1;
	if( debug ) cerr << "\toffs: " << offSX << "," << offSY << " " << offEX << "," << offEY << endl;

	// don't repeat the texture
	glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP );
	glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP );

	glMatrixMode( GL_TEXTURE );
	glPushMatrix();
	glLoadIdentity();
	
	glTranslatef( 0.5f, 0.5f, 0 );
	glRotatef( angle, 0, 0, 1 );
	glTranslatef( -0.5f, -0.5f, 0 );

	glTranslatef( offSX, offSY, 0 );
	glMatrixMode( GL_MODELVIEW );

								 
	float gx, gy;
	for( int xx = (int)sx; xx <= (int)ex; xx++ ) {
		for( int yy = (int)sy; yy <= (int)ey; yy++ ) {

			float texSx = ( ( xx - sx ) * ( offEX - offSX ) ) / ( ex - sx );
			float texEx = ( ( xx + 1 - sx ) * ( offEX - offSX ) ) / ( ex - sx );
			float texSy = ( ( yy - sy ) * ( offEY - offSY ) ) / ( ey - sy );
			float texEy = ( ( yy + 1 - sy ) * ( offEY - offSY ) ) / ( ey - sy );

			//glBegin( GL_LINE_LOOP );
			glBegin( GL_QUADS );

			glTexCoord2f( texSx, texEy );
			//glColor4f( 1, 1, 1, 1 );
			gx = groundPos[ xx ][ yy + 1 ].x - getX() / DIV;
			gy = groundPos[ xx ][ yy + 1 ].y - getY() / DIV;
			glVertex3f( gx, gy, groundPos[ xx ][ yy + 1 ].z + 0.26f / DIV );


			glTexCoord2f( texSx, texSy );
			//glColor4f( 1, 0, 0, 1 );
			gx = groundPos[ xx ][ yy ].x - getX() / DIV;
			gy = groundPos[ xx ][ yy ].y - getY() / DIV;
			glVertex3f( gx, gy, groundPos[ xx ][ yy ].z + 0.26f / DIV );

			glTexCoord2f( texEx, texSy );
			//glColor4f( 1, 1, 1, 1 );
			gx = groundPos[ xx + 1 ][ yy ].x - getX() / DIV;
			gy = groundPos[ xx + 1 ][ yy ].y - getY() / DIV;
			glVertex3f( gx, gy, groundPos[ xx + 1 ][ yy ].z + 0.26f / DIV );

			glTexCoord2f( texEx, texEy );
			//glColor4f( 1, 1, 1, 1 );
			gx = groundPos[ xx + 1 ][ yy + 1 ].x - getX() / DIV;
			gy = groundPos[ xx + 1 ][ yy + 1 ].y - getY() / DIV;
			glVertex3f( gx, gy, groundPos[ xx + 1 ][ yy + 1 ].z + 0.26f / DIV );

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

	/*
	glDisable( GL_TEXTURE_2D );
	glColor4f( 0, 1, 0, 1 );
	for( int xx = sx; xx <= ex; xx++ ) {
		for( int yy = sy; yy <= ey; yy++ ) {
			glBegin( GL_LINE_LOOP );
			gx = groundPos[ xx ][ yy + 1 ].x - getX() / DIV;
			gy = groundPos[ xx ][ yy + 1 ].y - getY() / DIV;
			glVertex3f( gx, gy, groundPos[ xx ][ yy + 1 ].z + 0.26f / DIV );

			gx = groundPos[ xx ][ yy ].x - getX() / DIV;
			gy = groundPos[ xx ][ yy ].y - getY() / DIV;
			glVertex3f( gx, gy, groundPos[ xx ][ yy ].z + 0.26f / DIV );

			gx = groundPos[ xx + 1 ][ yy ].x - getX() / DIV;
			gy = groundPos[ xx + 1 ][ yy ].y - getY() / DIV;
			glVertex3f( gx, gy, groundPos[ xx + 1 ][ yy ].z + 0.26f / DIV );

			gx = groundPos[ xx + 1 ][ yy + 1 ].x - getX() / DIV;
			gy = groundPos[ xx + 1 ][ yy + 1 ].y - getY() / DIV;
			glVertex3f( gx, gy, groundPos[ xx + 1 ][ yy + 1 ].z + 0.26f / DIV );

			glEnd();
		}
	}
	glEnable( GL_TEXTURE_2D );
	*/
}

void Map::createGroundMap() {
	float w, d, h;
	for( int xx = 0; xx < MAP_WIDTH / OUTDOORS_STEP; xx++ ) {		
		for( int yy = 0; yy < MAP_DEPTH /  OUTDOORS_STEP; yy++ ) {
			w = (float)( xx * OUTDOORS_STEP ) / DIV;
			d = (float)( yy * OUTDOORS_STEP ) / DIV;
			h = ( ground[ xx ][ yy ] ) / DIV;

			groundPos[ xx ][ yy ].x = w;
			groundPos[ xx ][ yy ].y = d;
			groundPos[ xx ][ yy ].z = h;
			//groundPos[ xx ][ yy ].u = ( xx * OUTDOORS_STEP * 32 ) / (float)MAP_WIDTH;
			//groundPos[ xx ][ yy ].v = ( yy * OUTDOORS_STEP * 32 ) / (float)MAP_DEPTH;

			groundPos[ xx ][ yy ].u = ( ( xx % OUTDOOR_FLOOR_TEX_SIZE ) / (float)OUTDOOR_FLOOR_TEX_SIZE ) + ( xx / OUTDOOR_FLOOR_TEX_SIZE );
			groundPos[ xx ][ yy ].v = ( ( yy % OUTDOOR_FLOOR_TEX_SIZE ) / (float)OUTDOOR_FLOOR_TEX_SIZE ) + ( yy / OUTDOOR_FLOOR_TEX_SIZE );

			groundPos[ xx ][ yy ].tex = groundTex[ xx ][ yy ];

			// height-based light
			if( ground[ xx ][ yy ] >= 10 ) {
				if( 0 == (int)( 5.0f * rand() / RAND_MAX ) &&
						ground[ xx + 1 ][ yy ] >= 10 && ground[ xx ][ yy + 1 ] >= 10 && 
						ground[ xx - 1 ][ yy ] >= 10 && ground[ xx ][ yy - 1 ] >= 10 ) {
					// snow
					groundPos[ xx ][ yy ].r = 1;
					groundPos[ xx ][ yy ].g = 1;
					groundPos[ xx ][ yy ].b = 1;
					groundPos[ xx ][ yy ].a = 1;
				} else {
					// ground (rock)
					float n = ( h / ( 13.0f / DIV ) );
					groundPos[ xx ][ yy ].r = n * 0.9f;
					groundPos[ xx ][ yy ].g = n * 0.6f;
					groundPos[ xx ][ yy ].b = n * 0.05f;
					groundPos[ xx ][ yy ].a = 1;
				}
			} else if( ground[ xx ][ yy ] <= -10 ) {
				float n = ( -h / ( 13.0f / DIV ) );
				groundPos[ xx ][ yy ].r = n * 0.05f;
				groundPos[ xx ][ yy ].g = n * 0.4f;
				groundPos[ xx ][ yy ].b = n * 1;
				groundPos[ xx ][ yy ].a = 1;
			} else {
				float n = ( h / ( 6.0f / DIV ) ) * 0.65f + 0.35f;
				if( (int)( 6.0f * rand() / RAND_MAX ) ) {
					groundPos[ xx ][ yy ].r = n * 0.55f;
					groundPos[ xx ][ yy ].g = n;
					groundPos[ xx ][ yy ].b = n * 0.45f;
					groundPos[ xx ][ yy ].a = 1;
				} else {
					groundPos[ xx ][ yy ].r = n;
					groundPos[ xx ][ yy ].g = n;
					groundPos[ xx ][ yy ].b = n * 0.25f;
					groundPos[ xx ][ yy ].a = 1;
				}
			}
			//n++;
		}
	}
	
	// add light
	CVectorTex *p[3];
	for( int xx = 0; xx < MAP_WIDTH / OUTDOORS_STEP; xx++ ) {
		for( int yy = 0; yy < MAP_DEPTH / OUTDOORS_STEP; yy++ ) {
			p[0] = &( groundPos[ xx ][ yy ] );
			p[1] = &( groundPos[ xx + 1 ][ yy ] );
			p[2] = &( groundPos[ xx ][ yy + 1 ] );
			addLight( p[0], p[1], p[2] );
			addLight( p[1], p[0], p[2] );
			addLight( p[2], p[0], p[1] );
		}
	}
}

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

void Map::drawWaterLevel() {
  Uint32 t = SDL_GetTicks();
  if( t - waterMoveTick > WATER_MOVE_SPEED ) {
    waterMoveTick = t;
    waterTexX += WATER_MOVE_DELTA;
    if( waterTexX >= 1.0f ) waterTexX -= 1.0f;
    waterTexY += WATER_MOVE_DELTA;
    if( waterTexY >= 1.0f ) waterTexY -= 1.0f;
  }

	glEnable( GL_TEXTURE_2D );
	glBindTexture( GL_TEXTURE_2D, waterTexture );
	glColor4f( 1, 1, 1, 0.45f );
	glEnable( GL_BLEND );
	glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );
	GLfloat ratio = MAP_UNIT / CAVE_CHUNK_SIZE;
	float w = (float)( mapViewWidth ) / DIV;
	float d = (float)( mapViewDepth ) / DIV;
	//float z = -4 / DIV;
	//glTranslatef( xpos2, ypos2, 0.0f);
	glBegin( GL_QUADS );
	glNormal3f( 0, 0, 1 );
	glTexCoord2f( getX() * DIV * ratio + waterTexX, getY() * DIV * ratio + waterTexY );
	glVertex3f( 0, 0, 0 );
	glTexCoord2f( getX() * DIV * ratio + waterTexX, ( getY() + mapViewDepth ) * DIV * ratio + waterTexY );
	glVertex3f( 0, d, 0 );
	glTexCoord2f( ( getX() + mapViewWidth ) * DIV * ratio + waterTexX, ( getY() + mapViewDepth ) * DIV * ratio + waterTexY );
	glVertex3f( w, d, 0 );
	glTexCoord2f( ( getX() + mapViewWidth ) * DIV * ratio + waterTexX, getY() * DIV * ratio + waterTexY );
	glVertex3f( w, 0, 0 );
	glEnd();
	glDisable( GL_BLEND );
}

void Map::drawFlatFloor() {
	GLfloat ratio = MAP_UNIT / CAVE_CHUNK_SIZE;
	float w = (float)( mapViewWidth ) / DIV;
	float d = (float)( mapViewDepth ) / DIV;
	//glTranslatef( xpos2, ypos2, 0.0f);
	glBegin( GL_QUADS );
	glNormal3f( 0, 0, 1 );
	glTexCoord2f( getX() * DIV * ratio, getY() * DIV * ratio );
	glVertex3f( 0, 0, 0 );
	glTexCoord2f( getX() * DIV * ratio, ( getY() + mapViewDepth ) * DIV * ratio );
	glVertex3f( 0, d, 0 );
	glTexCoord2f( ( getX() + mapViewWidth ) * DIV * ratio, ( getY() + mapViewDepth ) * DIV * ratio );
	glVertex3f( w, d, 0 );
	glTexCoord2f( ( getX() + mapViewWidth ) * DIV * ratio, getY() * DIV * ratio );
	glVertex3f( w, 0, 0 );
	glEnd();
}

void Map::initOutdoorsGroundTexture() {
	// set ground texture
	int ex = MAP_WIDTH / OUTDOORS_STEP;
	int ey = MAP_DEPTH / OUTDOORS_STEP;
	for( int x = 0; x < ex; x += OUTDOOR_FLOOR_TEX_SIZE ) {
		for( int y = 0; y < ey; y += OUTDOOR_FLOOR_TEX_SIZE ) {
			GLuint tex = 0;
			int n = (int)( 3.0f * rand() / RAND_MAX );
			switch( n ) {
			case 0:
				tex = adapter->getNamedTexture( "grass1" ); break;
			case 1:
				tex = adapter->getNamedTexture( "grass2" ); break;
			default:
				tex = adapter->getNamedTexture( "grass" );
			}
			for( int xx = 0; xx < OUTDOOR_FLOOR_TEX_SIZE; xx++ ){
				for( int yy = 0; yy < OUTDOOR_FLOOR_TEX_SIZE; yy++ ) {
					setGroundTex( x + xx, y + yy, tex );				
				}
			}
		}
	}
}
