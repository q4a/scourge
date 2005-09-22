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

#define MOUSE_ROT_DELTA 2

#define ZOOM_DELTA 1.2f
//#define DEBUG_MOUSE_POS

// set to 1 when location cache works
#define USE_LOC_CACHE 0

#define PI 3.14159f

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

const float Map::shadowTransformMatrix[16] = { 
	1, 0, 0, 0,
	0, 1, 0, 0,
	0.5f, -0.5f, 0, 0,
	0, 0, 0, 1 };

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
  LIGHTMAP_ENABLED=1;
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
  
  descriptionCount = 0;
  descriptionsChanged = false;
  for(int i = 0; i < MAX_DESCRIPTION_COUNT; i++)
	descriptions[i] = (char*)malloc(120 * sizeof(char));
  
  this->xrot = 0.0f; // if 0, artifacts appear
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
  
  // initialize shape graph of "in view shapes"
  for(int x = 0; x < MAP_WIDTH; x++) {
	for(int y = 0; y < MAP_DEPTH; y++) {
      floorPositions[x][y] = NULL;
	  for(int z = 0; z < MAP_VIEW_HEIGHT; z++) {
        pos[x][y][z] = NULL;
        effect[x][y][z] = NULL;
      }      
    }
  }
  // Init the pos cache
  for(int x = 0; x < MAX_POS_CACHE; x++) {
    posCache[x] = NULL;
  }
  nbPosCache = -1;

  // initialize the lightmap
  for(int x = 0; x < MAP_WIDTH / MAP_UNIT; x++) {
	for(int y = 0; y < MAP_DEPTH / MAP_UNIT; y++) {
	  lightMap[x][y] = (LIGHTMAP_ENABLED ? 0 : 1);
	}
  }

  lightMapChanged = true;  
  colorAlreadySet = false;
  selectedDropTarget = NULL;
  

  createOverlayTexture();

  addDescription(Constants::getMessage(Constants::WELCOME), 1.0f, 0.5f, 1.0f);
  addDescription("----------------------------------", 1.0f, 0.5f, 1.0f);
}

Map::~Map(){
  reset();
  // delete the descriptions
  for(int i = 0; i < MAX_DESCRIPTION_COUNT; i++)
    free(descriptions[i]);
  // delete the overlay texture
  glDeleteTextures(1, (GLuint*)&overlay_tex);
  delete frustum;
}

void Map::reset() {

  edited = false;
  strcpy( this->name, "" );
  hasWater = false;

  // remove locking info
  clearLocked();

  // remove area effects
  removeAllEffects();
  // clear map
  for(int xp = 0; xp < MAP_WIDTH; xp++) {
    for(int yp = 0; yp < MAP_DEPTH; yp++) {
      if( floorPositions[xp][yp] ) {
        Uint32 key = createPairKey(xp, yp);
        if( water.find(key) != water.end() ) {
          WaterTile *w = water[key];
          free(w);
        }
      }
      for(int zp = 0; zp < MAP_VIEW_HEIGHT; zp++) {
        if(pos[xp][yp][zp]) {
          delete pos[xp][yp][zp];
          pos[xp][yp][zp] = NULL;
        }
      }
    }
  }
  water.clear();


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
  
//  descriptionCount = 0;
//  descriptionsChanged = false;
  
  this->xrot = 0.0f; // if 0, artifacts appear
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
  
  // initialize shape graph of "in view shapes"
  for(int x = 0; x < MAP_WIDTH; x++) {
	for(int y = 0; y < MAP_DEPTH; y++) {
      floorPositions[x][y] = NULL;
	  for(int z = 0; z < MAP_VIEW_HEIGHT; z++) {
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

  // initialize the lightmap
  for(int x = 0; x < MAP_WIDTH / MAP_UNIT; x++) {
    for(int y = 0; y < MAP_DEPTH / MAP_UNIT; y++) {
      lightMap[x][y] = (LIGHTMAP_ENABLED ? 0 : 1);
    }
  }
  lightMapChanged = true;  
  colorAlreadySet = false;
  selectedDropTarget = NULL;
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

/**
   Move and rotate map.
   Modifiers: 
   -CTRL + arrow keys / mouse at edge of screen: rotate map
   -arrow keys / mouse at edge of screen: move map fast
   -SHIFT + arrow keys / mouse at edge of screen: slow move map
   -SHIFT + CTRL + arrow keys / mouse at edge of screen: slow rotate map
 */
void Map::moveMap(int dir) {
  if(SDL_GetModState() & KMOD_CTRL) {
    float rot;
    if(SDL_GetModState() & KMOD_SHIFT){
        rot = 1.5f;
    }
    else{
        rot = 5.0f;
    }
    if(dir & Constants::MOVE_DOWN) setYRot(-1.0f * rot);
	if(dir & Constants::MOVE_UP) setYRot(rot);
	if(dir & Constants::MOVE_RIGHT) setZRot(-1.0f * rot);
	if(dir & Constants::MOVE_LEFT) setZRot(rot);
  } else if( !preferences->getAlwaysCenterMap() ) {
	
	// stop rotating (angle of rotation is kept)
	setYRot(0);
	setZRot(0);

	// normalize z rot to 0-359
	float z = getZRot();
	if(z < 0) z += 360;
	if(z >= 360) z -= 360;
	float zrad = Constants::toRadians(z);

	//	cerr << "-------------------" << endl;
	//	cerr << "x=" << x << " y=" << y << " zrot=" << z << endl;

	mapChanged = resortShapes = true;
	float delta = (SDL_GetModState() & KMOD_SHIFT ? 0.5f : 1.0f);
	if(dir & Constants::MOVE_DOWN) {
	  mapx += delta * sin(zrad);
	  mapy += delta * cos(zrad);
	}
	if(dir & Constants::MOVE_UP) {
	  mapx += delta * -sin(zrad);
	  mapy += delta * -cos(zrad);
	}
	if(dir & Constants::MOVE_LEFT) {
	  mapx += delta * -cos(zrad);
	  mapy += delta * sin(zrad);
	}
	if(dir & Constants::MOVE_RIGHT) {
	  mapx += delta * cos(zrad);
	  mapy += delta * -sin(zrad);
	}

	//	cerr << "xdelta=" << xdelta << " ydelta=" << ydelta << endl;

	if(mapy > MAP_DEPTH - mapViewDepth) mapy = MAP_DEPTH - mapViewDepth;
	if(mapy < 0) mapy = 0;
	if(mapx > MAP_WIDTH - mapViewWidth) mapx = MAP_WIDTH - mapViewWidth;
	if(mapx < 0) mapx = 0;
	//	cerr << "mapx=" << mapx << " mapy=" << mapy << endl;

	x = (int)rint(mapx);
	y = (int)rint(mapy);
	//	cerr << "FINAL: x=" << x << " y=" << y << endl;
  }
}

void Map::removeCurrentEffects() {
  for( map<Uint32, EffectLocation*>::iterator i=currentEffectsMap.begin(); 
       i!=currentEffectsMap.end(); ++i ) {
    Uint32 key = i->first;
    EffectLocation *pos = i->second;
    if( !pos->isEffectOn() ) {
      int x, y, z;
      decodeTripletKey(key, &x, &y, &z);
      currentEffectsMap.erase( i );
      removeEffect( x, y, z );
      resortShapes = mapChanged = true;
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
    if(chunkX < 0 || chunkX > MAP_WIDTH / MAP_UNIT) continue;
    for(int chunkY = chunkStartY; chunkY < chunkEndY; chunkY++) {
      if(chunkY < 0 || chunkY > MAP_DEPTH / MAP_UNIT) continue;
      
      
      
      // remember the chunk's starting pos.
      float chunkPosX = (float)((chunkX - chunkStartX) * MAP_UNIT + 
                                chunkOffsetX) / GLShape::DIV;
      float chunkPosY = (float)((chunkY - chunkStartY) * MAP_UNIT + 
                                chunkOffsetY) / GLShape::DIV;
      
      // frustum testing
      //frustum->CalculateFrustum();
      if(useFrustum && 
         !frustum->CubeInFrustum(chunkPosX, chunkPosY, 0.0f, (float)MAP_UNIT / GLShape::DIV)) 
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
                              xp + chunkOffsetX) / GLShape::DIV;
              ypos2 = (float)((chunkY - chunkStartY) * MAP_UNIT - 
                              shape->getDepth() +
                              yp + chunkOffsetY) / GLShape::DIV;

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
            for(int zp = 0; zp < MAP_VIEW_HEIGHT; zp++) {
              if(lightMap[chunkX][chunkY] &&
                 effect[posX][posY][zp] &&
                 !effect[posX][posY][zp]->isInDelay() ) {
                xpos2 = (float)((chunkX - chunkStartX) * MAP_UNIT + 
                                xp + chunkOffsetX) / GLShape::DIV;
                ypos2 = (float)((chunkY - chunkStartY) * MAP_UNIT - 
                                1 + 
                                yp + chunkOffsetY) / GLShape::DIV;
                zpos2 = (float)(zp) / GLShape::DIV;

                setupPosition(posX, posY, zp,
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
                                      xp + chunkOffsetX) / GLShape::DIV;
                      ypos2 = (float)((chunkY - chunkStartY) * MAP_UNIT - 
                                      shape->getDepth() + 
                                      yp + chunkOffsetY) / GLShape::DIV;
                      zpos2 = (float)(zp) / GLShape::DIV;
                      setupPosition(posX, posY, zp,
                                    xpos2, ypos2, zpos2,
                                    ((MD2Shape*)pos[posX][posY][zp]->shape)->getDebugShape(), 
                                    NULL, NULL, NULL);
                      // end debug
                    }


                    //xpos2 = (pos[posX][posY][zp]->creature->getX() - (GLfloat)getX()) / GLShape::DIV;
                    //ypos2 = (pos[posX][posY][zp]->creature->getY() - (GLfloat)getY() - (GLfloat)(shape->getDepth())) / GLShape::DIV;

                    float xdiff = ( pos[posX][posY][zp]->creature->getX() - (float)(toint(pos[posX][posY][zp]->creature->getX())));
                    float ydiff = ( pos[posX][posY][zp]->creature->getY() - (float)(toint(pos[posX][posY][zp]->creature->getY())));
                    xpos2 = (float)((chunkX - chunkStartX) * MAP_UNIT + 
                                    xp + chunkOffsetX +
                                    xdiff ) / GLShape::DIV;
                    ypos2 = (float)((chunkY - chunkStartY) * MAP_UNIT - 
                                    shape->getDepth() + 
                                    yp + chunkOffsetY + 
                                    ydiff ) / GLShape::DIV;

                  } else {
                    xpos2 = (float)((chunkX - chunkStartX) * MAP_UNIT + 
                                    xp + chunkOffsetX) / GLShape::DIV;
                    ypos2 = (float)((chunkY - chunkStartY) * MAP_UNIT - 
                                    shape->getDepth() + 
                                    yp + chunkOffsetY) / GLShape::DIV;
                  }
                  zpos2 = (float)(zp) / GLShape::DIV;

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

void Map::drawGroundPosition(int posX, int posY,
                             float xpos2, float ypos2,
                             Shape *shape) {
  GLuint name;
  // encode this shape's map location in its name
  name = posX + (MAP_WIDTH * posY);     
  glTranslatef( xpos2, ypos2, 0.0f);
  
  glPushName( name );
  glColor4f(1, 1, 1, 0.9f);
  shape->draw();
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
    
    float sx = ( (float)MAP_UNIT / (float)WATER_TILE_X ) / GLShape::DIV;
    float sy = ( (float)MAP_UNIT / (float)WATER_TILE_Y ) / GLShape::DIV;
    
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
        float sz = ( WATER_HEIGHT + zz ) / GLShape::DIV;
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

void Map::setupPosition(int posX, int posY, int posZ,
                        float xpos2, float ypos2, float zpos2,
                        Shape *shape, RenderedItem *item, RenderedCreature *creature, 
                        EffectLocation *effect) {

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
    damage[damageCount].projectile = NULL;
    damage[damageCount].name = name;
    damage[damageCount].pos = getLocation(posX, posY, posZ);
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
    stencil[stencilCount].projectile = NULL;
    stencil[stencilCount].effect = NULL;
    stencil[stencilCount].name = name;
    stencil[stencilCount].pos = getLocation(posX, posY, posZ);
    stencilCount++;
  } else if(!shape->isStencil()) {
    bool invisible = (creature && creature->getStateMod(Constants::invisible));
    if(!invisible && shape->drawFirst()) {
      other[otherCount].xpos = xpos2;
      other[otherCount].ypos = ypos2;
      other[otherCount].zpos = zpos2;
      other[otherCount].shape = shape;
      other[otherCount].item = item;
      other[otherCount].creature = creature;
      other[otherCount].projectile = NULL;
      other[otherCount].effect = NULL;
      other[otherCount].name = name;
      other[otherCount].pos = getLocation(posX, posY, posZ);
      otherCount++;
    }
    if(shape->drawLater() || invisible) {
      later[laterCount].xpos = xpos2;
      later[laterCount].ypos = ypos2;
      later[laterCount].zpos = zpos2;
      later[laterCount].shape = shape;
      later[laterCount].item = item;
      later[laterCount].creature = creature;
      later[laterCount].projectile = NULL;
      later[laterCount].effect = NULL;
      later[laterCount].name = name;
      later[laterCount].pos = getLocation(posX, posY, posZ);
      laterCount++;
    }
  }
}

void Map::draw() {

  if( adapter->isMouseIsMovingOverMap() && !selectMode ) {
    // save mapChanged (fixes bug where map won't draw initially)
    bool b = mapChanged;
    mapChanged = false;
    selectMode = true;
    // careful this calls draw() again!
    getMapXYZAtScreenXY( &cursorMapX, &cursorMapY, &cursorMapZ );
    getMapXYAtScreenXY( &cursorFlatMapX, &cursorFlatMapY );
    cursorChunkX = ( cursorFlatMapX - MAP_OFFSET ) / MAP_UNIT;
    cursorChunkY = ( cursorFlatMapY - MAP_OFFSET ) / MAP_UNIT;
    selectMode = false;
    mapChanged = b;
    //cerr << "x=" << cursorMapX << " y=" << cursorMapY << " z=" << cursorMapZ << endl;
  }

  if(zoomIn) {
    if(zoom <= settings->getMinZoomIn() ) {
      zoomOut = false;
    } else {
      zoom /= ZOOM_DELTA;
      //      // FIXME:
      //mapViewWidth = mapViewDepth = (int)(100.0f / zoom);
      //center(x + mapViewWidth / 2,
      //       y + mapViewDepth / 2, 
      //       true);

      //mapChanged = true;
    }
  } else if(zoomOut) {
    if(zoom >= settings->getMaxZoomOut() ) {
      zoomOut = false;
    } else {
      zoom *= ZOOM_DELTA; 
      
      // FIXME:
      //mapViewWidth = mapViewDepth = (int)(100.0f / zoom);
      //center(x + mapViewWidth / 2,
      //       y + mapViewDepth / 2, 
      //       true);

      //mapChanged = true;
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


  initMapView();
  if( !selectMode ) frustum->CalculateFrustum();
  if(lightMapChanged) configureLightMap();
  if( currentEffectsMap.size() ) removeCurrentEffects();
  // populate the shape arrays
  if(mapChanged) {
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
  if( selectMode ) {
    for(int i = 0; i < otherCount; i++) doDrawShape(&other[i]);
    for(int i = 0; i < laterCount; i++) doDrawShape(&later[i]);
  } else {  



#ifdef DEBUG_MOUSE_POS
    // debugging mouse position
    if(debugX < MAP_WIDTH && debugX >= 0) {
      DrawLater later2;

      later2.shape = shapes->findShapeByName("LAMP_BASE");

      later2.xpos = ((float)(debugX - getX()) / GLShape::DIV);
      later2.ypos = (((float)(debugY - getY() - 1) - (float)((later2.shape)->getDepth())) / GLShape::DIV);
      later2.zpos = (float)(debugZ) / GLShape::DIV;

      later2.item = NULL;
      later2.creature = NULL;
      later2.projectile = NULL;
      later2.name = 0;
      later2.pos = NULL;
      doDrawShape(&later2);
    }
#endif


    if( preferences->getStencilbuf() &&
        preferences->getStencilBufInitialized() ) {


      // stencil and draw the floor
      //glDisable(GL_DEPTH_TEST);
      //glColorMask(0,0,0,0);
      glEnable(GL_STENCIL_TEST);
      glStencilOp(GL_REPLACE, GL_REPLACE, GL_REPLACE);
      glStencilFunc(GL_ALWAYS, 1, 0xffffffff);
      setupShapes(true, false);

      // shadows
      if( preferences->getShadows() >= Constants::OBJECT_SHADOWS ) {
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

    // draw the creatures/objects/doors/etc.
    DrawLater *playerDrawLater = NULL;
    for(int i = 0; i < otherCount; i++) {
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
    }



    // draw the walls: walls in front of the player will be transparent
    if( playerDrawLater ) {

      if( resortShapes ) {
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
    
    if( preferences->isOvalCutoutShown() &&
        !settings->isGridShowing() ) {
    /*      
        FIXME:
    if( preferences->isOvalCutoutShown() &&
        !settings->isGridShowing() &&
        session->getCurrentMission() ) 
    */        
      drawShade();
    }

    glDisable(GL_BLEND);

    glDepthMask(GL_TRUE);    

    drawProjectiles();
  }
  


  
  if( settings->isGridShowing() ) {

    glDisable( GL_CULL_FACE );
    glDisable( GL_TEXTURE_2D );
    
    glEnable(GL_BLEND);  
    glDepthMask(GL_FALSE);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE);

    // draw the starting position
    float xpos2 = (float)( this->startx - getX() ) / GLShape::DIV;
    float ypos2 = (float)( this->starty - getY() - 1 ) / GLShape::DIV;
    float zpos2 = 0.0f / GLShape::DIV;
    float w = 2.0f /  GLShape::DIV;
    float h = 4.0f /  GLShape::DIV;
    if( useFrustum && 
        frustum->CubeInFrustum( xpos2, ypos2, 0.0f, w / GLShape::DIV ) ) {
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
    float m = 0.5f / GLShape::DIV;

    for(int i = 0; i < chunkCount; i++) {

      float n = (float)MAP_UNIT / GLShape::DIV;
      
      glPushMatrix();
      glTranslatef( chunks[i].x, chunks[i].y - ( 1.0f / GLShape::DIV ), 0 );
      
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

    float xp = (float)(cursorFlatMapX - getX()) / GLShape::DIV;
    float yp = ((float)(cursorFlatMapY - getY()) - 1.0f) / GLShape::DIV;
    float cw = (float)cursorWidth / GLShape::DIV;
    float cd = -(float)cursorDepth / GLShape::DIV;
    m = ( cursorZ ? cursorZ : 0.5f ) / GLShape::DIV;
    float ch = (float)( cursorHeight + cursorZ ) / GLShape::DIV;

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

  if( move && !selectMode ) moveMap( move );
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
  // draw the projectiles
  DrawLater dl;
  for( map<RenderedCreature*, vector<RenderedProjectile*>*>::iterator i = RenderedProjectile::getProjectileMap()->begin(); 
       i != RenderedProjectile::getProjectileMap()->end(); 
       ++i ) {
    //RenderedCreature *creature = i->first;
    vector<RenderedProjectile*> *p = i->second;
    for (vector<RenderedProjectile*>::iterator e=p->begin(); e!=p->end(); ++e) {
      RenderedProjectile *proj = *e;

      // draw it
      dl.xpos = ((proj->getX() - (float)getX()) / GLShape::DIV);
      //		dl.ypos = (((float)(proj->getY() - getY() - 1) - (float)((later2.shape)->getDepth())) / GLShape::DIV);
      dl.ypos = ((proj->getY() - (float)getY() - 1.0f) / GLShape::DIV);
      dl.zpos = (float)(7) / GLShape::DIV;
      dl.shape = proj->getShape();
      dl.creature = NULL;
      dl.item = NULL;
      dl.projectile = proj;
      dl.name = 0;
      dl.pos = NULL;

      //if( proj->getSpell() ) {
      if( proj->getShape()->drawLater() ) {
        glEnable(GL_BLEND);
        glDepthMask(GL_FALSE);
        proj->getShape()->setupBlending();
      }
      doDrawShape(&dl);
      //if( proj->getSpell() ) {
      if( proj->getShape()->drawLater() ) {
        proj->getShape()->endBlending();
        glDisable(GL_BLEND);
        glDepthMask(GL_TRUE);
      }
    }
  }
}

void Map::drawShade() {
  glPushMatrix();
  glLoadIdentity();

  glTranslatef(viewX, viewY, 0);

  //  glDisable(GL_BLEND);
  glDisable(GL_DEPTH_TEST);
  //glDisable( GL_TEXTURE_2D );
  glBlendFunc(GL_DST_COLOR, GL_ZERO);
  //scourge->setBlendFunc();

  glColor4f( 1, 1, 1, 0.5f);

  glBindTexture( GL_TEXTURE_2D, overlay_tex );
  glBegin( GL_QUADS );
  //  glNormal3f(0.0f, 1.0f, 0.0f);
  glTexCoord2f( 0.0f, 0.0f );
  glVertex3f(0, 0, 0);
  glTexCoord2f( 0.0f, 1.0f );
  glVertex3f(0, 
             viewHeight, 0);
  glTexCoord2f( 1.0f, 1.0f );
  glVertex3f(viewWidth, viewHeight, 0);
  glTexCoord2f( 1.0f, 0.0f );
  glVertex3f(viewWidth, 0, 0);
  glEnd();

  //glEnable( GL_TEXTURE_2D );
  glEnable(GL_DEPTH_TEST);
  glPopMatrix();

}

// vary this number from 0.001 - 3.0 to get tighter shading
#define SHADE_LEVEL 1.0f

void Map::createOverlayTexture() {
  // create the dark texture
  unsigned int i, j;
  glGenTextures(1, (GLuint*)&overlay_tex);
//  float tmp = 0.7f;
  for(i = 0; i < OVERLAY_SIZE; i++) {
    for(j = 0; j < OVERLAY_SIZE; j++) {
      float half = ((float)OVERLAY_SIZE - 0.5f) / 2.0f;
      float id = (float)i - half;
      float jd = (float)j - half;
      //float dd = 255.0f - ((255.0f / (half * half / 1.2f)) * (id * id + jd * jd));
      
      float dd = 255.0f - ((255.0f / (half * half / SHADE_LEVEL)) * (id * id + jd * jd));
      if(dd < 0.0f) dd = 0.0f;
      if(dd > 255.0f) dd = 255.0f;
      unsigned char d = (unsigned char)dd;
      overlay_data[i * OVERLAY_SIZE * 3 + j * 3 + 0] = d;
      overlay_data[i * OVERLAY_SIZE * 3 + j * 3 + 1] = d;
      overlay_data[i * OVERLAY_SIZE * 3 + j * 3 + 2] = d;
    }
  }
  glBindTexture(GL_TEXTURE_2D, overlay_tex);
  glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
  //glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
  //glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
  glTexImage2D(GL_TEXTURE_2D, 0, 3, OVERLAY_SIZE, OVERLAY_SIZE, 0, 
			   GL_RGB, GL_UNSIGNED_BYTE, overlay_data);
}

void Map::doDrawShape(DrawLater *later, int effect) {
    doDrawShape(later->xpos, later->ypos, later->zpos, later->shape, later->name, effect, later);
}

void Map::doDrawShape(float xpos2, float ypos2, float zpos2, Shape *shape, 
					  GLuint name, int effect, DrawLater *later) {
  if(shape) ((GLShape*)shape)->useShadow = useShadow;

  // slow on mac os X:
  // glPushAttrib(GL_ENABLE_BIT);

  glPushMatrix();
  if(useShadow) {
      // put shadow above the floor a little
      glTranslatef( xpos2, ypos2, 0.26f / GLShape::DIV);
      glMultMatrixf(shadowTransformMatrix);

    // gray shadows
    //glColor4f( 0, 0, 0, 0.5f );

    // purple shadows
    glColor4f( 0.04f, 0, 0.07f, 0.6f );    

    // debugging red
    //glColor4f(1, 0, 0, 0.5f);
  } else {

    if(shape) shape->setupToDraw();

    glTranslatef( xpos2, ypos2, zpos2);

    if(colorAlreadySet) {
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
  if(effect && later) {
    if(later->creature) {
      // translate hack for md2 models... see: md2shape::draw()
      //glTranslatef( 0, -1 / GLShape::DIV, 0 );
      later->creature->getEffect()->draw(later->creature->getEffectType(),
                                         later->creature->getDamageEffect());
      //glTranslatef( 0, 1 / GLShape::DIV, 0 );
    } else if(later->effect) {
      later->effect->getEffect()->draw(later->effect->getEffectType(),
                                       later->effect->getDamageEffect());
    }
  } else if(later && later->projectile) {
    // orient and draw the projectile
    float f = later->projectile->getAngle() + 90;
    if(f < 0) f += 360;
    if(f >= 360) f -= 360;
    glRotatef( f, 0, 0, 1 );
    // for projectiles, set the correct camera angle
    if(later->projectile->getAngle() < 90) {
      ((GLShape*)shape)->setCameraRot(xrot, yrot, zrot + later->projectile->getAngle() + 90);
    } else if(later->projectile->getAngle() < 180) {
      ((GLShape*)shape)->setCameraRot(xrot, yrot, zrot - later->projectile->getAngle());
    }
    later->projectile->getShape()->draw();
  } else if(later && later->creature && !useShadow) {
    if(later->creature->getStateMod(Constants::invisible)) {
      glColor4f(0.3, 0.8f, 1.0f, 1.0f);    
    } else if(later->creature->getStateMod(Constants::possessed)) {
      glColor4f(1.0, 0.3f, 0.8f, 1.0f);    
    }
    
    // outline mission creatures
    if( adapter->isMissionCreature( later->creature ) ) {
    //if( session->getCurrentMission() &&
        //session->getCurrentMission()->isMissionCreature( later->creature ) ) {
      shape->outline( 0.15f, 0.15f, 0.4f );
    }
    shape->draw();
  } else if( later && later->item && !useShadow ) {
    
    if( later->item->isMagicItem() ) {
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
  
  double obj_x = (mapx - getX() + 1) / GLShape::DIV;
  double obj_y = (mapy - getY() - 2) / GLShape::DIV;
  double obj_z = 0.0f;
  //double obj_z = mapz / GLShape::DIV;
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
  float xpos2 = ((float)(mapx - getX()) / GLShape::DIV);
  float ypos2 = ((float)(mapy - getY()) / GLShape::DIV);
  float zpos2 = (float)(mapz) / GLShape::DIV;
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
  glRotatef( xrot, 0.0f, 1.0f, 0.0f );
  if(!ignoreRot) {
      glRotatef( yrot, 1.0f, 0.0f, 0.0f );  
  }  
  glRotatef( zrot, 0.0f, 0.0f, 1.0f );
  glTranslatef( 0, 0, this->zpos);  

  //  float startx = -(((float)mapViewWidth + (mapx - (float)x)) / 2.0) / GLShape::DIV;
  //  float starty = -(((float)mapViewDepth + (mapy - (float)y)) / 2.0) / GLShape::DIV;
  float startx = -((float)mapViewWidth / 2.0 + (mapx - (float)x)) / GLShape::DIV;
  float starty = -((float)mapViewDepth / 2.0 + (mapy - (float)y)) / GLShape::DIV;
  //float startz = -(float)(MAP_VIEW_HEIGHT) / GLShape::DIV;
  float startz = 0.0;

  glTranslatef( startx, starty, startz );
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

void Map::setFloorPosition(Sint16 x, Sint16 y, Shape *shape) {
  floorPositions[x][y] = shape;
  for(int xp = 0; xp < shape->getWidth(); xp++) {
    for(int yp = 0; yp < shape->getDepth(); yp++) {
      adapter->colorMiniMapPoint(x + xp, y - yp, shape);
    }
  }
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

Shape *Map::removeFloorPosition(Sint16 x, Sint16 y) {
	Shape *shape = NULL;
  if(floorPositions[x][y]) {
    shape = floorPositions[x][y];
    floorPositions[x][y] = 0;
    for(int xp = 0; xp < shape->getWidth(); xp++) {
      for(int yp = 0; yp < shape->getDepth(); yp++) {
        // fixme : is it good or not to erase the minimap too ???       
        adapter->eraseMiniMapPoint(x, y);
      }
    }
  }
  Uint32 key = createPairKey(x, y);
  if( water.find(key) != water.end() ) {
    WaterTile *w = water[key];
    free(w);
    water.erase( key );
  }
	return shape;
}

Location *Map::isBlocked(Sint16 x, Sint16 y, Sint16 z, 
                         Sint16 shapeX, Sint16 shapeY, Sint16 shapeZ, 
                         Shape *s, 
                         int *newz) {
  int zz = z;
  for(int sx = 0; sx < s->getWidth(); sx++) {
    for(int sy = 0; sy < s->getDepth(); sy++) {
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

void Map::addDescription(char *desc, float r, float g, float b) {
  strncpy(descriptions[descriptionCount], desc, 120);
  // zero terminate just in case desc.length > 120
  descriptions[descriptionCount][119] = 0;
  // set the color
  descriptionsColor[descriptionCount].r = r;
  descriptionsColor[descriptionCount].g = g;
  descriptionsColor[descriptionCount].b = b;

  // delete the first one if max reached
  if(descriptionCount == MAX_DESCRIPTION_COUNT - 1) {
    for(int i = 0; i < (MAX_DESCRIPTION_COUNT - 1); i++) {
      strcpy(descriptions[i], descriptions[i + 1]);
      descriptionsColor[i].r = descriptionsColor[i + 1].r;
      descriptionsColor[i].g = descriptionsColor[i + 1].g;
      descriptionsColor[i].b = descriptionsColor[i + 1].b;
    }
  } else {
    descriptionCount++;
  }

  descriptionsChanged = true;
}

void Map::startEffect(Sint16 x, Sint16 y, Sint16 z, 
                      int effect_type, GLuint duration, 
                      int width, int height, GLuint delay) {

  if( x >= MAP_WIDTH || y >= MAP_DEPTH || z >= MAP_VIEW_HEIGHT ) {
    cerr << "*** STARTEFFECT out of bounds: pos=" << x << "," << y << "," << z << endl;
    ((RenderedCreature*)NULL)->getName();
  }

  // show an effect
  if(effect[x][y][z]) {
    if(effect[x][y][z]->isEffectOn() && 
       effect[x][y][z]->effectType == effect_type) {
      return;
    } else {
      return;
    }
  }

  if(!effect[x][y][z]) {
    effect[x][y][z] = new EffectLocation();
  }
  effect[x][y][z]->effect = new Effect( preferences,
                                        shapes, 
                                        width, height );
  effect[x][y][z]->effect->deleteParticles();
  effect[x][y][z]->resetDamageEffect();
  effect[x][y][z]->effectType = effect_type;
  effect[x][y][z]->effectDuration = duration;
  effect[x][y][z]->effectDelay = delay;
  effect[x][y][z]->x = x;
  effect[x][y][z]->y = y;
  effect[x][y][z]->z = z;
  currentEffectsMap[ createTripletKey( x, y, z ) ] = effect[x][y][z];

  // need to do this to make sure effect shows up
  resortShapes = mapChanged = true;
}

void Map::removeEffect(Sint16 x, Sint16 y, Sint16 z) {

  if( x >= MAP_WIDTH || y >= MAP_DEPTH || z >= MAP_VIEW_HEIGHT ) {
    cerr << "*** REMOVEEFFECT out of bounds: pos=" << x << "," << y << "," << z << endl;
    ((RenderedCreature*)NULL)->getName();
  }

  if(effect[x][y][z]) {
    if(effect[x][y][z]->effect) {
      delete effect[x][y][z]->effect;
      effect[x][y][z]->effect = NULL;
    }
    delete effect[x][y][z];
    effect[x][y][z] = NULL;
  }
}

void Map::removeAllEffects() {
  for( int x = 0; x < MAP_WIDTH; x++ ) {
    for( int y = 0; y < MAP_DEPTH; y++ ) {
      for( int z = 0; z < MAP_VIEW_HEIGHT; z++ ) {
        if( effect[x][y][z] ) {
          if(effect[x][y][z]->effect) {
            delete effect[x][y][z]->effect;
            effect[x][y][z]->effect = NULL;
          }
          delete effect[x][y][z];
          effect[x][y][z] = NULL;
        }
      }
    }
  }
  currentEffectsMap.clear();
}

void Map::setPosition(Sint16 x, Sint16 y, Sint16 z, Shape *shape) {
  if(shape) {
    resortShapes = mapChanged = true;
    //cerr << "FIXME: Map::setPosition" << endl;
    for(int xp = 0; xp < shape->getWidth(); xp++) {
      for(int yp = 0; yp < shape->getDepth(); yp++) {
        for(int zp = 0; zp < shape->getHeight(); zp++) {          
          if(!pos[x + xp][y - yp][z + zp]) {
            pos[x + xp][y - yp][z + zp] = new Location();
          }
          pos[x + xp][y - yp][z + zp]->shape = shape;
          pos[x + xp][y - yp][z + zp]->item = NULL;
          pos[x + xp][y - yp][z + zp]->creature = NULL;
          pos[x + xp][y - yp][z + zp]->x = x;
          pos[x + xp][y - yp][z + zp]->y = y;
          pos[x + xp][y - yp][z + zp]->z = z;
          pos[x + xp][y - yp][z + zp]->outlineColor = NULL;
        }
      }
    }
    for(int xp = 0; xp < shape->getWidth(); xp++) {
      for(int yp = 0; yp < shape->getDepth(); yp++) {
        adapter->colorMiniMapPoint(x + xp, y - yp, shape, pos[x + xp][y - yp][0]);
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
    for(int xp = 0; xp < shape->getWidth(); xp++) {
      for(int yp = 0; yp < shape->getDepth(); yp++) {
        // fixme : is it good or not to erase the minimap too ???
        adapter->eraseMiniMapPoint(x + xp, y - yp);
        for(int zp = 0; zp < shape->getHeight(); zp++) {
            //cerr <<"remove pos " << x + xp << "," << y - yp << "," << z + zp<<endl;
		  delete pos[x + xp][y - yp][z + zp];
		  pos[x + xp][y - yp][z + zp] = NULL;          
        }
      }
    }
  }
  return shape;
}

// like getLocation, you can specify any position in the shape to remove it.
Shape *Map::removeLocation(Sint16 x, Sint16 y, Sint16 z) {
  if(pos[x][y][z] && pos[x][y][z]->shape ) 
    return removePosition( pos[x][y][z]->x,
                           pos[x][y][z]->y,
                           pos[x][y][z]->z );
  else return NULL;
}
  
void Map::setItem(Sint16 x, Sint16 y, Sint16 z, RenderedItem *item) {
  if(item) {
    if(item->getShape()) {
	  resortShapes = mapChanged = true;
      for(int xp = 0; xp < item->getShape()->getWidth(); xp++) {
        for(int yp = 0; yp < item->getShape()->getDepth(); yp++) {          
          for(int zp = 0; zp < item->getShape()->getHeight(); zp++) {
			
            if(!pos[x + xp][y - yp][z + zp]) {
              pos[x + xp][y - yp][z + zp] = new Location();
            }

            pos[x + xp][y - yp][z + zp]->item = item;
            pos[x + xp][y - yp][z + zp]->shape = item->getShape();
            pos[x + xp][y - yp][z + zp]->creature = NULL;
            pos[x + xp][y - yp][z + zp]->x = x;
            pos[x + xp][y - yp][z + zp]->y = y;
            pos[x + xp][y - yp][z + zp]->z = z;
            pos[x + xp][y - yp][z + zp]->outlineColor = NULL;
          }
        }
      }
  	}
  }
}

RenderedItem *Map::removeItem(Sint16 x, Sint16 y, Sint16 z) {
  RenderedItem *item = NULL;
  if(pos[x][y][z] &&
     pos[x][y][z]->item &&
     pos[x][y][z]->x == x &&
     pos[x][y][z]->y == y &&
     pos[x][y][z]->z == z) {
	resortShapes = mapChanged = true;
	item = pos[x][y][z]->item;
    for(int xp = 0; xp < item->getShape()->getWidth(); xp++) {
      for(int yp = 0; yp < item->getShape()->getDepth(); yp++) {       
        for(int zp = 0; zp < item->getShape()->getHeight(); zp++) {
		  
		  delete pos[x + xp][y - yp][z + zp];
		  pos[x + xp][y - yp][z + zp] = NULL;		
        }
      }
    }
  }
  return item;
}

// drop items above this one
void Map::dropItemsAbove(int x, int y, int z, RenderedItem *item) {
  int count = 0;
  Location drop[100];
  for(int tx = 0; tx < item->getShape()->getWidth(); tx++) {
	for(int ty = 0; ty < item->getShape()->getDepth(); ty++) {
	  for(int tz = z + item->getShape()->getHeight(); tz < MAP_VIEW_HEIGHT; tz++) {
		Location *loc2 = pos[x + tx][y - ty][tz];
		if(loc2 && loc2->item) {
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
  for(int i = 0; i < count; i++) {
	//cerr << "item " << drop[i].item->getItemName() << " new z=" << drop[i].z << endl;
	setItem(drop[i].x, drop[i].y, drop[i].z, drop[i].item);
  }
}

void Map::setCreature(Sint16 x, Sint16 y, Sint16 z, RenderedCreature *creature) {
  char message[120];  
  if(creature) {
    if(creature->getShape()) {
	  resortShapes = mapChanged = true;
	  while(true) {
		for(int xp = 0; xp < creature->getShape()->getWidth(); xp++) {
		  for(int yp = 0; yp < creature->getShape()->getDepth(); yp++) {
			for(int zp = 0; zp < creature->getShape()->getHeight(); zp++) {
			  //			  cerr <<"adding pos " << x + xp << "," << y - yp << "," << z + zp;
        if(!pos[x + xp][y - yp][z + zp]) {
          if(!USE_LOC_CACHE || nbPosCache < 0){
            //                    cerr << " no cache available!" << endl;
            pos[x + xp][y - yp][z + zp] = new Location();
          }
          else{
            //                    cerr << " cache number : " << nbPosCache << endl;
            pos[x + xp][y - yp][z + zp] = posCache[nbPosCache];
            //posCache[nbPosCache] = NULL;
            nbPosCache--;
          }
        } else if(pos[x + xp][y - yp][z + zp]->item) {
          // creature picks up non-blocking item (this is the only way to handle 
          // non-blocking items. It's also very 'roguelike'.
          RenderedItem *item = pos[x + xp][y - yp][z + zp]->item;
          removeItem(pos[x + xp][y - yp][z + zp]->x,
                     pos[x + xp][y - yp][z + zp]->y,
                     pos[x + xp][y - yp][z + zp]->z);
          creature->pickUpOnMap(item);
          sprintf(message, "%s picks up %s.", 
                  creature->getName(), 
                  item->getItemName());
          addDescription(message);				
          // since the above will have removed some locations, try adding the creature again
          continue;
        }
        pos[x + xp][y - yp][z + zp]->item = NULL;
        pos[x + xp][y - yp][z + zp]->shape = creature->getShape();
        pos[x + xp][y - yp][z + zp]->creature = creature;
        pos[x + xp][y - yp][z + zp]->x = x;
        pos[x + xp][y - yp][z + zp]->y = y;
        pos[x + xp][y - yp][z + zp]->z = z;
        pos[x + xp][y - yp][z + zp]->outlineColor = NULL;
        //creature->moveTo(x, y, z);
			}
		  }
		}
		break;
	  }
	}
  }
}

void Map::moveCreaturePos(Sint16 nx, Sint16 ny, Sint16 nz,
                          Sint16 ox, Sint16 oy, Sint16 oz,
                          RenderedCreature *creature) {
  Location *p = pos[ox][oy][oz];
  if(creature && creature->getShape() &&
     p && p->creature &&
     p->x == ox && p->y == oy && p->z == oz) {
    resortShapes = mapChanged = true;
    
    // remove the old pos
    Location *tmp[MAP_UNIT][MAP_UNIT][MAP_UNIT];
    for(int xp = 0; xp < creature->getShape()->getWidth(); xp++) {
      for(int yp = 0; yp < creature->getShape()->getDepth(); yp++) {
        for(int zp = 0; zp < creature->getShape()->getHeight(); zp++) {
          int oldX = ox + xp;
          int oldY = oy - yp;
          int oldZ = oz + zp;
          tmp[xp][yp][zp] = pos[oldX][oldY][oldZ];
          tmp[xp][yp][zp]->outlineColor = NULL;
          pos[oldX][oldY][oldZ] = NULL;
          if(!(tmp[xp][yp][zp])) cerr << "*** tmp is null!" << endl;
        }
      }
    }

    // pick up any items in the way
    char message[120];
    for(int xp = 0; xp < creature->getShape()->getWidth(); xp++) {
      for(int yp = 0; yp < creature->getShape()->getDepth(); yp++) {
        for(int zp = 0; zp < creature->getShape()->getHeight(); zp++) {
          int newX = nx + xp;
          int newY = ny - yp;
          int newZ = nz + zp;            
            
          if(pos[newX][newY][newZ]) {
            if(pos[newX][newY][newZ]->item) {
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
              addDescription(message);
            } else {
              cerr << "*** Error: when moving " << creature->getName() << " path contained a non-item position." << endl;
            }
          }
        }    
      }
    }
            
    // insert the new pos
    for(int xp = 0; xp < creature->getShape()->getWidth(); xp++) {
      for(int yp = 0; yp < creature->getShape()->getDepth(); yp++) {
        for(int zp = 0; zp < creature->getShape()->getHeight(); zp++) {
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
  RenderedCreature *creature = NULL;
  if(pos[x][y][z] &&
     pos[x][y][z]->creature &&
     pos[x][y][z]->x == x &&
     pos[x][y][z]->y == y &&
     pos[x][y][z]->z == z) {
	resortShapes = mapChanged = true;
    creature = pos[x][y][z]->creature;
	//    cout<<"width : "<< creature->getShape()->getWidth()<<endl;
	//    cout<<"depth: "<< creature->getShape()->getDepth()<<endl;
	//    cout<<"height: "<< creature->getShape()->getHeight()<<endl;
    for(int xp = 0; xp < creature->getShape()->getWidth(); xp++) {
      for(int yp = 0; yp < creature->getShape()->getDepth(); yp++) {
        for(int zp = 0; zp < creature->getShape()->getHeight(); zp++) {
		  //            cerr <<"deleting pos " << x + xp << "," << y - yp << "," << z + zp;
            if(!USE_LOC_CACHE || nbPosCache >= MAX_POS_CACHE - 1){
			  //                cerr << " no cache available!" << endl;
                delete pos[x + xp][y - yp][z + zp];
                pos[x + xp][y - yp][z + zp] = NULL;
            }
            else{
                nbPosCache++;
				//                cerr << " cache number : " << nbPosCache << endl;
                posCache[nbPosCache] = pos[x + xp][y - yp][z + zp];
                pos[x + xp][y - yp][z + zp] = NULL;
                
            }
        }
      }
    }
  }
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
		  if(!isWallBetween(x, y, z1, xx, yy, z2)) return false;
		}
	  }
	}
  }
  return true;	  
}

// FIXME: only uses x,y for now
bool Map::isWallBetween(int x1, int y1, int z1,
						int x2, int y2, int z2) {

  if(x1 == x2 && y1 == y2) return isWall(x1, y1, z1);
  if(x1 == x2) {
	if(y1 > y2) SWAP(y1, y2);
	for(int y = y1; y <= y2; y++) {
	  if(isWall(x1, y, z1)) return true;
	}
	return false;
  }
  if(y1 == y2) {
	if(x1 > x2) SWAP(x1, x2);
	for(int x = x1; x <= x2; x++) {
	  if(isWall(x, y1, z1)) return true;
	}
	return false;
  }
  

  //  fprintf(stderr, "Checking for wall: from: %d,%d to %d,%d\n", x1, y1, x2, y2);
  bool ret = false;
  bool yDiffBigger = (abs(y2 - y1) > abs(x2 - x1));
  float m = (float)(y2 - y1) / (float)(x2 - x1);
  int steps = (yDiffBigger ? abs(y2 - y1) : abs(x2 - x1));
  float x = x1;
  float y = y1;
  for(int i = 0; i < steps; i++) {
	//	fprintf(stderr, "\tat=%f,%f\n", x, y);
	if(isWall((int)x, (int)y, z1)) {
	  //fprintf(stderr, "wall at %f, %f\n", x, y);
	  ret = true;
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
  return ret;
}

bool Map::isWall(int x, int y, int z) {
  Location *loc = getLocation((int)x, (int)y, z);
  return (loc && 
          (!loc->item || loc->item->getShape() != loc->shape) && 
          (!loc->creature || loc->creature->getShape() != loc->shape));
}                             

// FIXME: only uses x, y for now
bool Map::shapeFits(Shape *shape, int x, int y, int z) {
  for(int tx = 0; tx < shape->getWidth(); tx++) {
	for(int ty = 0; ty < shape->getDepth(); ty++) {
	  if(getLocation(x + tx, y - ty, 0)) {
		return false;
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

void Map::setLocked(int doorX, int doorY, int doorZ, bool value) {
  locked[createTripletKey(doorX, doorY, doorZ)] = value;
  Location *p = pos[doorX][doorY][doorZ];
  for(int xp = 0; xp < p->shape->getWidth(); xp++) {
    for(int yp = 0; yp < p->shape->getDepth(); yp++) {
      adapter->colorMiniMapPoint(doorX + xp, 
                                                   doorY - yp, 
                                                   p->shape, 
                                                   p);
    }
  }
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

bool Map::isLocationInLight(int x, int y) {
  int chunkX = (x - MAP_OFFSET) / MAP_UNIT;
  int chunkY = (y - (MAP_OFFSET + 1)) / MAP_UNIT;
  return lightMap[chunkX][chunkY];
}

void Map::handleEvent( SDL_Event *event ) {

  // turn off outlining
  if( lastOutlinedX < MAP_WIDTH ) {
    Location *pos = getLocation( lastOutlinedX, lastOutlinedY, lastOutlinedZ );
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
      setZRot(-event->motion.xrel * MOUSE_ROT_DELTA);
      setYRot(-event->motion.yrel * MOUSE_ROT_DELTA);
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
  if( event->button.button ) {
    if( event->button.button == SDL_BUTTON_MIDDLE ) {
      mouseRot = false;
      setXRot(0);
      setYRot(0);
      setZRot(0);
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

float EditorMapSettings::getMinZoomIn() {
  return 0.05f;
}

float EditorMapSettings::getMaxZoomOut() {
  return 2.8f;
}

float EditorMapSettings::getMaxYRot() {
  return 90.0f;
}


void Map::saveMap( char *name, char *result ) {

  if( !strlen( name ) ) {
    strcpy( result, "You need to name the map first." );
    return;
  }

  MapInfo *info = (MapInfo*)malloc(sizeof(MapInfo));
  info->version = PERSIST_VERSION;

  info->start_x = startx;
  info->start_y = starty;

  info->grid_x = mapGridX;
  info->grid_y = mapGridY;

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

      for( int z = 0; z < MAP_VIEW_HEIGHT; z++ ) {
        if( pos[x][y][z] &&
            pos[x][y][z]->x == x &&
            pos[x][y][z]->y == y &&
            pos[x][y][z]->z == z && 
            !( pos[x][y][z]->creature && 
               !(pos[x][y][z]->creature->isMonster() ) ) ) {

          info->pos[ info->pos_count ] = Persist::createLocationInfo( x, y, z );

          if( pos[x][y][z]->item ) {
            info->pos[ info->pos_count ]->item = pos[x][y][z]->item->save();
          } else if( pos[x][y][z]->creature ) {
            info->pos[ info->pos_count ]->creature = pos[x][y][z]->creature->save();
          } else {
            strncpy( (char*)(info->pos[ info->pos_count ]->shape_name), 
                     pos[x][y][z]->shape->getName(),
                     254 );
            info->pos[ info->pos_count ]->shape_name[254] = 0;
          }

          // FIXME: save door info also

          info->pos_count++;
        }
      }
    }
  }

  char fileName[300];
  sprintf( fileName, "%s/maps/%s.map", rootDir, name );

  cerr << "saving map: " << fileName << endl;

  FILE *fp = fopen( fileName, "wb" );
  File *file = new ZipFile( fp, ZipFile::ZIP_WRITE );
  Persist::saveMap( file, info );
  delete file;

  Persist::deleteMapInfo( info );

  adapter->saveMapData( (const char*)fileName );

  sprintf( result, "Map saved: %s", name );
}

bool Map::loadMap( char *name, char *result, StatusReport *report, int depth, bool changingStory, bool fromRandom ) {
  if( !strlen( name ) ) {
    strcpy( result, "Enter a name of a map to load." );
    return false;
  }

  char fileName[300];
  if( depth > 0 ) {
    sprintf( fileName, "%s/maps/%s%d.map", rootDir, name, depth );
  } else {
    sprintf( fileName, "%s/maps/%s.map", rootDir, name );
  }
  cerr << "Looking for map: " << fileName << endl;

  FILE *fp = fopen( fileName, "rb" );
  if( !fp ) {
    sprintf( result, "Can't find map: %s", name );
    return false;
  }
  if( report ) report->updateStatus( 0, 4 );
  File *file = new ZipFile( fp, ZipFile::ZIP_READ );
  MapInfo *info = Persist::loadMap( file );
  delete file;

  if( report ) report->updateStatus( 1, 4 );

  // reset the map
  reset();

  edited = true;

  // load the theme
  shapes->loadTheme( (const char*)info->theme_name );

  if( report ) report->updateStatus( 2, 4 );

  // Start at the saved start pos. or where the party
  // was on the last level if changing stories.
  if( !changingStory || !( adapter->hasParty() ) || fromRandom ) {
    startx = info->start_x;
    starty = info->start_y;
  } else {
    startx = toint( adapter->getPlayer()->getX() );
    starty = toint( adapter->getPlayer()->getY() );
  }

  mapGridX = info->grid_x;
  mapGridY = info->grid_y;
  
  GLShape *shape;
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

    if( info->pos[i]->item ) {
      RenderedItem *item = adapter->load( info->pos[i]->item );
      if( item ) setItem( info->pos[i]->x, info->pos[i]->y, info->pos[i]->z, item );
      else cerr << "Map::load failed to item at pos: " << info->pos[i]->x << "," << info->pos[i]->y << "," << info->pos[i]->z << endl;
    } else if( info->pos[i]->creature ) {
      RenderedCreature *creature = adapter->load( info->pos[i]->creature );
      if( creature ) {
        setCreature( info->pos[i]->x, info->pos[i]->y, info->pos[i]->z, creature );
        creature->moveTo( info->pos[i]->x, info->pos[i]->y, info->pos[i]->z );
      } else cerr << "Map::load failed to creature at pos: " << info->pos[i]->x << "," << info->pos[i]->y << "," << info->pos[i]->z << endl;
    } else if( strlen( (char*)(info->pos[i]->shape_name) ) ) {
      shape = shapes->
        findShapeByName( (char*)(info->pos[i]->shape_name), true );
      if( shape ) setPosition( info->pos[i]->x, info->pos[i]->y, info->pos[i]->z, shape );
      else cerr << "Map::load failed to find shape: " << info->pos[i]->shape_name <<
        " at pos: " << info->pos[i]->x << "," << info->pos[i]->y << "," << info->pos[i]->z << endl;
    }

    // FIXME: handle door info 
  }
  this->center( info->start_x, info->start_y, true );

  Persist::deleteMapInfo( info );

  if( report ) report->updateStatus( 3, 4 );

  // load map-related data from text file
  adapter->loadMapData( (const char*)fileName );
  
  strcpy( this->name, name );

  /* 
    FIXME: Place the party at the start. This code attempts to find a place
    for the party near the gate (which was set in Scourge::useGate.) Need to 
    find a better AI solution as this code can place the party outside the walls.
    For now always leave "whitespace" around gates in edited levels.
  */
  if( adapter->hasParty() ) {
    int xx = startx;
    int yy = starty;
    int ox = xx;
    int oy = yy;
    int dir = Constants::MOVE_UP;
    int r = 6;
    for( int t = 0; t < adapter->getPartySize(); t++ ) {
      if( !adapter->getParty(t)->getStateMod( Constants::dead ) ) {
        xx = ox;
        yy = oy;
        r = 6;
        while( true ) {
          // can player fit here?
          if( !isBlocked( xx, yy, 0, 0, 0, 0, adapter->getParty(t)->getShape(), NULL ) ) {
            //cerr << "Placed party member: " << t << " at: " << xx << "," << yy << endl;
            adapter->getParty(t)->moveTo( xx, yy, 0 );
            adapter->getParty(t)->setSelXY( xx, yy );
            setCreature( xx, yy, 0, adapter->getParty(t) );
            break;
          }

          // try radially around the player
          switch( dir ) {
          case Constants::MOVE_UP:
            yy--; 
          if( yy <= MAP_OFFSET || abs( oy - yy ) > r ) dir = Constants::MOVE_RIGHT;
          break;
          case Constants::MOVE_RIGHT:
            xx++; 
          if( xx >= MAP_WIDTH - MAP_OFFSET || abs( ox - xx ) > r ) dir = Constants::MOVE_DOWN;
          break;
          case Constants::MOVE_DOWN:
            yy++; 
          if( yy >= MAP_DEPTH - MAP_OFFSET || abs( oy - yy ) > r ) dir = Constants::MOVE_LEFT;
          break;
          case Constants::MOVE_LEFT:
            xx--; 
          if( xx <= MAP_OFFSET || abs( ox - xx ) > r ) {
            dir = Constants::MOVE_UP;
            r += adapter->getParty(t)->getShape()->getWidth();
          }
          break;
          }

          /*
          cerr << "...trying to place party member: " << t << 
            " at: " << xx << "," << yy << 
            " dir: " << dir << " r: " << r << endl;
          */            
        }
      }
      
      /*
      adapter->getParty(t)->moveTo( xx, yy, 0 );
      adapter->getParty(t)->setSelXY( -1, -1 );
      if( !adapter->getParty(t)->getStateMod( Constants::dead ) )
        setCreature( xx, yy, 0, adapter->getParty(t) );
      xx += adapter->getParty(t)->getShape()->getWidth();
      */
    }
  }
  sprintf( result, "Map loaded: %s", name );
  return true;
}

void Map::loadMapLocation( char *name, char *result, int *gridX, int *gridY, int depth ) {
  Uint16 tmpX, tmpY;
  if( !strlen( name ) ) {
    strcpy( result, "Enter a name of a map to load." );
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
    sprintf( result, "Can't find map: %s", name );
    return;
  }
  File *file = new ZipFile( fp, ZipFile::ZIP_READ );
  Persist::loadMapHeader( file, &tmpX, &tmpY );
  delete file;

  *gridX = (int)tmpX;
  *gridY = (int)tmpY;

  sprintf( result, "Map header loaded: %s", name );
}


void Map::getMapXYAtScreenXY( Uint16 *mapx, Uint16 *mapy ) {
  getMapXYAtScreenXY( adapter->getMouseX(), 
                      adapter->getMouseY(), 
                      mapx, 
                      mapy );
}

void Map::getMapXYAtScreenXY(Uint16 x, Uint16 y,
                             Uint16 *mapx, Uint16 *mapy) {
  glPushMatrix();
  
  // Initialize the scene w/o y rotation.
  initMapView(true);
  
  double obj_x, obj_y, obj_z;
  double win_x = (double)x;
  double win_y = (double)adapter->getScreenHeight() - y - 1;
  double win_z = 0.5f;
  
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
  
  glDisable( GL_SCISSOR_TEST );
  
  if(res) {
    //*mapx = levelMap->getX() + (Uint16)(((obj_x) * GLShape::DIV)) - 1;
    //*mapy = levelMap->getY() + (Uint16)(((obj_y) * GLShape::DIV)) + 2;

    *mapx = getX() + (Uint16)(((obj_x) * GLShape::DIV));
    *mapy = getY() + (Uint16)(((obj_y) * GLShape::DIV));

    //*mapz = (Uint16)0;
    //*mapz = (Uint16)(obj_z * GLShape::DIV);
    debugX = *mapx;
    debugY = *mapy;
    debugZ = 0;
  } else {
    //*mapx = *mapy = *mapz = MAP_WIDTH + 1;
    *mapx = *mapy = MAP_WIDTH + 1;
  }
  glPopMatrix();
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

  debugX = *mapx;
  debugY = *mapy;
  debugZ = *mapz;
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

