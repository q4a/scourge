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

const float Map::shadowTransformMatrix[16] = { 
	1, 0, 0, 0,
	0, 1, 0, 0,
	0.5f, -0.5f, 0, 0,
	0, 0, 0, 1 };

Map::Map(Session *session) {

  hasWater = false;

  mapViewWidth = MVW;
  mapViewDepth = MVD;

  chunkCount = 0;
  frustum = new CFrustum();
  useFrustum = true;

  this->session = session;  
  // only use 1 (disabled) or 0 (enabled)
  LIGHTMAP_ENABLED=1;
  zoom = 1.0f;
  zoomIn = zoomOut = false;
  x = y = 0;
  mapx = mapy = 0.0f;
  selectMode = false;
  floorOnly = false;
  selX = selY = selZ = MAP_WIDTH + 1;
  oldLocatorSelX = oldLocatorSelY = oldLocatorSelZ = selZ;
  useShadow = false;
  //alwaysCenter = true;

  debugX = debugY = debugZ = -1;
  
  mapChanged = true;
  
  descriptionCount = 0;
  descriptionsChanged = false;
  for(int i = 0; i < MAX_DESCRIPTION_COUNT; i++)
	descriptions[i] = (char*)malloc(120 * sizeof(char));
  
  this->xrot = 0.0f; // if 0, artifacts appear
  this->yrot = 30.0f;
  this->zrot = 45.0f;
  this->xRotating = this->yRotating = this->zRotating = 0.0f;

  setViewArea(0, 0, 
              session->getGameAdapter()->getScreenWidth(), 
              session->getGameAdapter()->getScreenHeight());

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

  hasWater = false;

  // remove locking info
  clearLocked();

  // remove area effects
  vector<EffectLocation*>::iterator e=currentEffects.begin();
  for(int i = 0; i < (int)currentEffects.size(); i++) {
    EffectLocation *effectLocation = currentEffects[i];
    if(effectLocation) {
      currentEffects.erase(e);
      e = currentEffects.begin();
      removeEffect(effectLocation->x, effectLocation->y, effectLocation->z);
      i--;
    }
  }
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
  selX = selY = selZ = MAP_WIDTH + 1;
  oldLocatorSelX = oldLocatorSelY = oldLocatorSelZ = selZ;
  useShadow = false;
  //alwaysCenter = true;
  debugX = debugY = debugZ = -1;
  mapChanged = true;
  
//  descriptionCount = 0;
//  descriptionsChanged = false;
  
  this->xrot = 0.0f; // if 0, artifacts appear
  this->yrot = 30.0f;
  this->zrot = 45.0f;
  this->xRotating = this->yRotating = this->zRotating = 0.0f;

  setViewArea(0, 0, 
              session->getGameAdapter()->getScreenWidth(), 
              session->getGameAdapter()->getScreenHeight());

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
  if(session->getUserConfiguration()->getKeepMapSize()) {
    zoom = (float)session->getGameAdapter()->getScreenWidth() / (float)w;
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
  if(session->getUserConfiguration()->getAlwaysCenterMap() || force) {
	//  if(scourge->getUserConfiguration()->getAlwaysCenterMap() ||
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
void Map::move(int dir) {
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
  } else if(!session->getUserConfiguration()->getAlwaysCenterMap()) {
	
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

	mapChanged = true;
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
      frustum->CalculateFrustum();
      if(useFrustum && 
         !frustum->CubeInFrustum(chunkPosX, chunkPosY, 0.0f, (float)MAP_UNIT / GLShape::DIV)) 
        continue;

      // store this chunk
      chunks[chunkCount].x = chunkPosX;
      chunks[chunkCount].y = chunkPosY;
      chunkCount++;
      
            
      
      // FIXME: this works except it draws other doors on the same
      // chunk that should be hidden. To really fix it, we need to
      // keep track of which side of the chunk to draw.
      if(!lightMap[chunkX][chunkY]) {
        if(ground || water) continue;
        else {
          // see if the door is next to a chunk in the light
          bool found = false;
          
          // look to the left
          if(chunkX >= 1 && 
             lightMap[chunkX - 1][chunkY]) {
            posX = chunkX * MAP_UNIT + 1 + MAP_OFFSET;
            posY = chunkY * MAP_UNIT + (MAP_UNIT / 2) + MAP_OFFSET + 1;
            if(isDoor(posX, posY)) found = true;
          }
          // look to the right
          if(!found && 
             chunkX + 1 < MAP_WIDTH / MAP_UNIT && 
             lightMap[chunkX + 1][chunkY]) {
            posX = (chunkX + 1) * MAP_UNIT - 1 + MAP_OFFSET;
            posY = chunkY * MAP_UNIT + (MAP_UNIT / 2) + MAP_OFFSET + 1;
            if(isDoor(posX, posY)) found = true;
          }
          // look above
          if(!found &&
             chunkY - 1 >= 0 && 
             lightMap[chunkX][chunkY - 1]) {
            posX = chunkX * MAP_UNIT + (MAP_UNIT / 2) + MAP_OFFSET;
            posY = chunkY * MAP_UNIT + MAP_OFFSET + 1;
            if(isDoor(posX, posY)) found = true;

          }
          // look below
          if(!found &&
             chunkY + 1< MAP_DEPTH / MAP_UNIT &&
             lightMap[chunkX][chunkY + 1]) {
            posX = chunkX * MAP_UNIT + (MAP_UNIT / 2) + MAP_OFFSET;
            posY = (chunkY + 1) * MAP_UNIT - 2 + MAP_OFFSET + 1;
            if(isDoor(posX, posY)) found = true;
          }

          // if not, skip this chunk
          if(!found) continue;
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
                 effect[posX][posY][zp]) {
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

                // FIXME: this draws more doors than needed... 
                // it should use doorValue to figure out what needs to be drawn
                if((!lightMap[chunkX][chunkY] && 
                    (shape == session->getShapePalette()->findShapeByName("CORNER") ||
                     shape == session->getShapePalette()->findShapeByName("NS_DOOR") ||
                     shape == session->getShapePalette()->findShapeByName("EW_DOOR") ||
                     shape == session->getShapePalette()->findShapeByName("NS_DOOR_TOP") ||
                     shape == session->getShapePalette()->findShapeByName("EW_DOOR_TOP") ||
                     shape == session->getShapePalette()->findShapeByName("DOOR_SIDE"))) ||
                   (lightMap[chunkX][chunkY] && shape)) {


                  if(pos[posX][posY][zp]->creature) {
                    xpos2 = (pos[posX][posY][zp]->creature->getX() - (GLfloat)getX()) / GLShape::DIV;
                    ypos2 = (pos[posX][posY][zp]->creature->getY() - (GLfloat)getY() - (GLfloat)(shape->getDepth())) / GLShape::DIV;
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
                        Shape *shape, Item *item, Creature *creature, 
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
  if(zoomIn) {
    if(zoom <= 0.5f) {
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
    if(zoom >= 2.8f) {
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

  // remove area effects
  vector<EffectLocation*>::iterator e=currentEffects.begin();
  for(int i = 0; i < (int)currentEffects.size(); i++) {
    EffectLocation *effectLocation = currentEffects[i];
    if(effectLocation && !effectLocation->isEffectOn()) {
      currentEffects.erase(e);
      e = currentEffects.begin();
      removeEffect(effectLocation->x, effectLocation->y, effectLocation->z);
      mapChanged = true;
      i--;
    }
  }  

  float oldrot;

  oldrot = yrot;
  if(yRotating != 0) yrot+=yRotating;
  if(yrot >= 55 || yrot < 0) yrot = oldrot;

  oldrot = zrot;
  if(zRotating != 0) zrot+=zRotating;
  if(zrot >= 360) zrot -= 360;
  if(zrot < 0) zrot = 360 + zrot;


  initMapView();
  if(lightMapChanged) configureLightMap();
  // populate the shape arrays
  if(mapChanged) {
    int csx, cex, csy, cey;
    setupShapes(false, false, &csx, &cex, &csy, &cey);
    int shapeCount = laterCount + otherCount + damageCount + stencilCount;
    sprintf(mapDebugStr, "chunks=(%s %d out of %d) x:%d-%d y:%d-%d shapes=%d", 
            (useFrustum ? "*" : ""),
            chunkCount, ((cex - csx)*(cey - csy)),
            csx, cex, csy, cey, shapeCount);
    //            shapeCount, laterCount, otherCount, damageCount, stencilCount);
    session->getGameAdapter()->setDebugStr(mapDebugStr);
  }
  if(selectMode) {
    for(int i = 0; i < otherCount; i++) doDrawShape(&other[i]);
    for(int i = 0; i < laterCount; i++) doDrawShape(&later[i]);
  } else {  



#ifdef DEBUG_MOUSE_POS
    // debugging mouse position
    if(debugX < MAP_WIDTH && debugX >= 0) {
      DrawLater later2;

      later2.shape = session->getShapePalette()->findShapeByName("LAMP_BASE");

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



    // draw the creatures/objects/doors/etc.
    for(int i = 0; i < otherCount; i++) {
      if(selectedDropTarget && 
         ((selectedDropTarget->creature && selectedDropTarget->creature == other[i].creature) ||
          (selectedDropTarget->item && selectedDropTarget->item == other[i].item))) {
        colorAlreadySet = true;
        glColor4f(0, 1, 1, 1);
      }
      doDrawShape(&other[i]);
    }
    // draw the walls
    for(int i = 0; i < stencilCount; i++) doDrawShape(&stencil[i]);

    if(session->getUserConfiguration()->getStencilbuf() &&
       session->getUserConfiguration()->getStencilBufInitialized()) {


      // stencil and draw the floor
      //glDisable(GL_DEPTH_TEST);
      //glColorMask(0,0,0,0);
      glEnable(GL_STENCIL_TEST);
      glStencilOp(GL_REPLACE, GL_REPLACE, GL_REPLACE);
      glStencilFunc(GL_ALWAYS, 1, 0xffffffff);
      setupShapes(true, false);

      /*
      // draw the water
      glEnable(GL_BLEND);  
      glDepthMask(GL_FALSE);
      //glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
      ((Scourge*)this->session->getGameAdapter())->setBlendFunc();
      setupShapes(false, true);
      glDepthMask(GL_TRUE);    
      glDisable(GL_BLEND);
      */

      // shadows
      if(session->getUserConfiguration()->getShadows() >= Constants::OBJECT_SHADOWS) {
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
        if(session->getUserConfiguration()->getShadows() == Constants::ALL_SHADOWS) {
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
      if(session->getUserConfiguration()->getShadows() == Constants::ALL_SHADOWS) {
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

    // draw the water
    if( hasWater ) {
      glDisable(GL_TEXTURE_2D);
      glEnable(GL_BLEND);  
      glDepthMask(GL_FALSE);
      glBlendFunc( GL_ONE, GL_SRC_COLOR );
      setupShapes(false, true);
      glDepthMask(GL_TRUE);    
      glDisable(GL_BLEND);
      glEnable(GL_TEXTURE_2D);
    }
    
    // draw the blended walls
    //glEnable(GL_BLEND);  
    //glDepthMask(GL_FALSE);
    //glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    //for(int i = 0; i < stencilCount; i++) doDrawShape(&stencil[i]);
    //glDepthMask(GL_TRUE);    
    //glDisable(GL_BLEND);

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
    drawShade();
    glDisable(GL_BLEND);

    glDepthMask(GL_TRUE);    

    drawProjectiles();
  }
  
  
#ifdef DEBUG_RENDER
  for(int i = 0; i < chunkCount; i++) {

    float n = (float)MAP_UNIT / GLShape::DIV;

    glDisable( GL_CULL_FACE );
    glDisable( GL_TEXTURE_2D );
    glPushMatrix();
    glTranslatef( chunks[i].x, chunks[i].y, 0 );

    glColor4f( 1,1,1,1 );
    glBegin( GL_LINE_LOOP );
    glVertex3f( 0, 0, 0 );
    glVertex3f( n, 0, 0 );
    glVertex3f( n, n, 0 );
    glVertex3f( 0, n, 0 );
    glEnd();
    glBegin( GL_LINE_LOOP );
    glVertex3f( 0, 0, n );
    glVertex3f( n, 0, n );
    glVertex3f( n, n, n );
    glVertex3f( 0, n, n );
    glEnd();

    glColor4f( 0,1,1,1 );
    glBegin( GL_LINE_LOOP );
    glVertex3f( 0, 0, 0 );
    glVertex3f( n, 0, 0 );
    glVertex3f( n, 0, n );
    glVertex3f( 0, 0, n );
    glEnd();
    glBegin( GL_LINE_LOOP );
    glVertex3f( 0, n, 0 );
    glVertex3f( n, n, 0 );
    glVertex3f( n, n, n );
    glVertex3f( 0, n, n );
    glEnd();

    glPopMatrix();
    glEnable( GL_CULL_FACE );
    glEnable( GL_TEXTURE_2D );
  }
#endif

  glDisable( GL_SCISSOR_TEST );
}

void Map::drawProjectiles() {
  // draw the projectiles
  DrawLater dl;
  vector<Projectile*> removedProjectiles;
  map<Projectile*, Creature*> battleProjectiles;
  map<Creature *, vector<Projectile*>*> *proj = Projectile::getProjectileMap();
  for (map<Creature *, vector<Projectile*>*>::iterator i=proj->begin(); i!=proj->end(); ++i) {
    //Creature *creature = i->first;
    vector<Projectile*> *p = i->second;
    for (vector<Projectile*>::iterator e=p->begin(); e!=p->end(); ++e) {
      Projectile *proj = *e;

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

      if (proj->getSpell()) {
        glEnable(GL_BLEND);
        glDepthMask(GL_FALSE);
        proj->getShape()->setupBlending();
      }
      doDrawShape(&dl);
      if (proj->getSpell()) {
        proj->getShape()->endBlending();
        glDisable(GL_BLEND);
        glDepthMask(GL_TRUE);
      }

      // collision detection
      bool blocked = false;

      // location target projectile hit
      if(proj->atTargetLocation() &&
         proj->getSpell() &&
         proj->getSpell()->isLocationTargetAllowed()) {
//        cerr << "PROJ: reached target, from=" << proj->getCreature()->getName() << endl;                                
        session->getGameAdapter()->fightProjectileHitTurn(proj, (int)proj->getX(), (int)proj->getY());        
        blocked = true;
      }

      Location *loc = getLocation(toint(proj->getX()), toint(proj->getY()), 0);
      if(loc) {
        if(loc->creature && 
           proj->getCreature()->canAttack(loc->creature)) {
          //               cerr << "PROJ: attacks creature, from=" << proj->getCreature()->getName() << endl;
          battleProjectiles[proj] = loc->creature;
          blocked = true;
        } else if(proj->doesStopOnImpact() &&
                  ((loc->item && loc->item->getShape()->getHeight() >= 6) ||
                   (!loc->creature && !loc->item && loc->shape && loc->shape->getHeight() >= 6))) {
          //               cerr << "PROJ: blocked by item or shape, from=" << proj->getCreature()->getName() << endl;                     
          // hit something
          blocked = true;
        }
      }

      // remove finished projectiles
      if( blocked || proj->atTargetLocation() ) {
        
        // DEBUG INFO
        if( !blocked ) {
          cerr << "*** Warning: projectile didn't hit target ***" << endl;
          cerr << "Creature: " << proj->getCreature()->getName() << endl;
          if( proj->getCreature()->getTargetCreature() ) {
            cerr << " target=" << proj->getCreature()->getTargetCreature()->getName() << " at: " << 
              proj->getCreature()->getTargetCreature()->getX() << "," <<
              proj->getCreature()->getTargetCreature()->getY() << " " <<
              " shape: " << proj->getCreature()->getTargetCreature()->getShape()->getWidth() << "," <<
              proj->getCreature()->getTargetCreature()->getShape()->getHeight() << endl;
          } else {
            cerr << " no target." << endl;
          }
          proj->debug();
        }

        removedProjectiles.push_back(proj);
      }
    }
  }

  // fight battles
  for (map<Projectile*, Creature*>::iterator i=battleProjectiles.begin(); i!=battleProjectiles.end(); ++i) {
    Projectile *proj = i->first;
    Creature *creature = i->second;
    session->getGameAdapter()->fightProjectileHitTurn(proj, creature);
  }

  // remove projectiles
  for (vector<Projectile*>::iterator e=removedProjectiles.begin(); e!=removedProjectiles.end(); ++e) {
    Projectile::removeProjectile(*e);
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
      float dd = 255.0f - ((255.0f / (half * half / 1.2f)) * (id * id + jd * jd));
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
  //glPushName( (GLuint)((GLShape*)shape)->getShapePalIndex() );
  if(shape) {
    ((GLShape*)shape)->setCameraRot(xrot, yrot, zrot);
    ((GLShape*)shape)->setCameraPos(xpos, ypos, zpos, xpos2, ypos2, zpos2);
    if(later && later->pos) ((GLShape*)shape)->setLocked(isLocked(later->pos->x, later->pos->y, 0));
    else ((GLShape*)shape)->setLocked(false);
  }
  if(effect && later) {
    if(later->creature) {
      glTranslatef( 0, -1 / GLShape::DIV, 0 );
      later->creature->getEffect()->draw(later->creature->getEffectType(),
                                         later->creature->getDamageEffect());
      glTranslatef( 0, 1 / GLShape::DIV, 0 );
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
    shape->draw();
  } else if( later && later->item && !useShadow ) {
    
    if( later->item->isMagicItem() ) {
      shape->outline( &(Constants::MAGIC_ITEM_COLOR[ later->item->getMagicLevel() ]) );
    } else if( later->item->getContainsMagicItem() ) {
      shape->outline( 0.8f, 0.8f, 0.3f );
    }

    shape->draw();

    

  } else {
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
    win_y = session->getGameAdapter()->getScreenHeight() - win_y;
    return (win_x >= 0 && win_x < session->getGameAdapter()->getScreenWidth() &&
            win_y >= 0 && win_y < session->getGameAdapter()->getScreenHeight());
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
void Map::initMapView(bool ignoreRot) {
  glLoadIdentity();

  glTranslatef(viewX, viewY, 0);
  glScissor(viewX, 
            session->getGameAdapter()->getScreenHeight() - (viewY + viewHeight),
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

Location *Map::moveCreature(Sint16 x, Sint16 y, Sint16 z, Uint16 dir,Creature *newCreature) {
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
                            Creature *newCreature) {

  // no need to actually move data
  if( x == nx && y == ny && z == nz ) {
    mapChanged = true;
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
      session->getGameAdapter()->colorMiniMapPoint(x + xp, y - yp, shape);
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
        session->getGameAdapter()->eraseMiniMapPoint(x, y);
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

void Map::drawDescriptions(ScrollingList *list) {
  if(descriptionsChanged) {
    descriptionsChanged = false;
    list->setLines(descriptionCount, (const char**)descriptions, descriptionsColor);
    list->setSelectedLine(descriptionCount - 1);
  }

  /*
  glPushMatrix();
  glLoadIdentity();
  //glColor4f(1.0f, 1.0f, 0.4f, 1.0f);
  int y = TOP_GUI_HEIGHT - 5;
  if(descriptionCount <= 5) y = descriptionCount * 15;
  int index =  0;
  while(y > 5 && index < descriptionCount) {    
	glColor4f(descriptions[index].r, descriptions[index].g, descriptions[index].b, 1.0f);
    glRasterPos2f( (float)5, (float)y );
    scourge->getSDLHandler()->texPrint(5, y, "%s", descriptions[index].text);

    y -= 15;
    index++;
  }
  glPopMatrix();
  */
}

void Map::handleMouseMove(Uint16 mapx, Uint16 mapy, Uint16 mapz) {
  if(mapx < MAP_WIDTH) {
	selX = mapx;
	selY = mapy;
	selZ = mapz;
  }
}     

void Map::startEffect(Sint16 x, Sint16 y, Sint16 z, 
                      int effect_type, int duration, 
                      int width, int height) {

  if( x >= MAP_WIDTH || y >= MAP_DEPTH || z >= MAP_VIEW_HEIGHT ) {
    cerr << "*** STARTEFFECT out of bounds: pos=" << x << "," << y << "," << z << endl;
    vector<EffectLocation*>::iterator e=currentEffects.begin();
    for(int i = 0; i < (int)currentEffects.size(); i++) {
      EffectLocation *effectLocation = currentEffects[i];
      if(effectLocation) {
        cerr << "pos=" << x << "," << y << "," << z << endl;
      }
    }
    ((Creature*)NULL)->getName();
  }

  // show an effect
  if(effect[x][y][z]) {
    if(effect[x][y][z]->isEffectOn() && 
       effect[x][y][z]->effectType == effect_type) {
      return;
    } else {
      return;
      /* FIXME: if we call removeeffect, it also needs to remove from currentEffects.
      cerr << "*** Warning, removing effect w/o removing from currentEffects!" << endl;
      removeEffect(x, y, z);
      */
    }
  }

  if(!effect[x][y][z]) {
    effect[x][y][z] = new EffectLocation();
  }
  effect[x][y][z]->effect = new Effect(session,
                                       session->getShapePalette(), 
                                       width, height);
  effect[x][y][z]->effect->deleteParticles();
  effect[x][y][z]->resetDamageEffect();
  effect[x][y][z]->effectType = effect_type;
  effect[x][y][z]->effectDuration = duration;
  effect[x][y][z]->x = x;
  effect[x][y][z]->y = y;
  effect[x][y][z]->z = z;

  currentEffects.push_back(effect[x][y][z]);

  // need to do this to make sure effect shows up
  mapChanged = true;
}

void Map::removeEffect(Sint16 x, Sint16 y, Sint16 z) {

  if( x >= MAP_WIDTH || y >= MAP_DEPTH || z >= MAP_VIEW_HEIGHT ) {
    cerr << "*** REMOVEEFFECT out of bounds: pos=" << x << "," << y << "," << z << endl;
    vector<EffectLocation*>::iterator e=currentEffects.begin();
    for(int i = 0; i < (int)currentEffects.size(); i++) {
      EffectLocation *effectLocation = currentEffects[i];
      if(effectLocation) {
        cerr << "pos=" << x << "," << y << "," << z << endl;
      }
    }
    ((Creature*)NULL)->getName();
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

void Map::setPosition(Sint16 x, Sint16 y, Sint16 z, Shape *shape) {
  if(shape) {
    mapChanged = true;
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
        }
      }
    }
    for(int xp = 0; xp < shape->getWidth(); xp++) {
      for(int yp = 0; yp < shape->getDepth(); yp++) {
        session->getGameAdapter()->colorMiniMapPoint(x + xp, y - yp, shape, pos[x + xp][y - yp][0]);
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
	mapChanged = true;
    shape = pos[x][y][z]->shape;
    for(int xp = 0; xp < shape->getWidth(); xp++) {
      for(int yp = 0; yp < shape->getDepth(); yp++) {
        // fixme : is it good or not to erase the minimap too ???
        session->getGameAdapter()->eraseMiniMapPoint(x + xp, y - yp);
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

void Map::setItem(Sint16 x, Sint16 y, Sint16 z, Item *item) {
  if(item) {
    if(item->getShape()) {
	  mapChanged = true;
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
          }
        }
      }
  	}
  }
}

Item *Map::removeItem(Sint16 x, Sint16 y, Sint16 z) {
  Item *item = NULL;
  if(pos[x][y][z] &&
     pos[x][y][z]->item &&
     pos[x][y][z]->x == x &&
     pos[x][y][z]->y == y &&
     pos[x][y][z]->z == z) {
	mapChanged = true;
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
void Map::dropItemsAbove(int x, int y, int z, Item *item) {
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
	cerr << "item " << drop[i].item->getItemName() << " new z=" << drop[i].z << endl;
	setItem(drop[i].x, drop[i].y, drop[i].z, drop[i].item);
  }
}

void Map::setCreature(Sint16 x, Sint16 y, Sint16 z, Creature *creature) {
  char message[120];  
  if(creature) {
    if(creature->getShape()) {
	  mapChanged = true;
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
				Item *item = pos[x + xp][y - yp][z + zp]->item;
				removeItem(pos[x + xp][y - yp][z + zp]->x,
						   pos[x + xp][y - yp][z + zp]->y,
						   pos[x + xp][y - yp][z + zp]->z);
				creature->addInventory(item, true);
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
                          Creature *creature) {
  Location *p = pos[ox][oy][oz];
  if(creature && creature->getShape() &&
     p && p->creature &&
     p->x == ox && p->y == oy && p->z == oz) {
    mapChanged = true;
    
    // remove the old pos
    Location *tmp[MAP_UNIT][MAP_UNIT][MAP_UNIT];
    for(int xp = 0; xp < creature->getShape()->getWidth(); xp++) {
      for(int yp = 0; yp < creature->getShape()->getDepth(); yp++) {
        for(int zp = 0; zp < creature->getShape()->getHeight(); zp++) {
          int oldX = ox + xp;
          int oldY = oy - yp;
          int oldZ = oz + zp;
          tmp[xp][yp][zp] = pos[oldX][oldY][oldZ];
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
              Item *item = pos[newX][newY][newZ]->item;
              removeItem(pos[newX][newY][newZ]->x,
                         pos[newX][newY][newZ]->y,
                         pos[newX][newY][newZ]->z);
              creature->addInventory(item, true);
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
        }
      }
    }

  }
}

Creature *Map::removeCreature(Sint16 x, Sint16 y, Sint16 z) {
  Creature *creature = NULL;
  if(pos[x][y][z] &&
     pos[x][y][z]->creature &&
     pos[x][y][z]->x == x &&
     pos[x][y][z]->y == y &&
     pos[x][y][z]->z == z) {
	mapChanged = true;
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
	  lightMap[x][y] = (LIGHTMAP_ENABLED ? 0 : 1);
	}
  }
  if(!LIGHTMAP_ENABLED) return;

  int chunkX = (toint(session->getParty()->getPlayer()->getX()) + 
                (session->getParty()->getPlayer()->getShape()->getWidth() / 2) - 
                MAP_OFFSET) / MAP_UNIT;
  int chunkY = (toint(session->getParty()->getPlayer()->getY()) - 
                (session->getParty()->getPlayer()->getShape()->getDepth() / 2) - 
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
int Map::getCreaturesInArea(int x, int y, int radius, Creature *targets[]) {
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
  return(shape == session->getShapePalette()->findShapeByName("EW_DOOR") ||
         shape == session->getShapePalette()->findShapeByName("NS_DOOR"));
}

void Map::setLocked(int doorX, int doorY, int doorZ, bool value) {
  locked[createTripletKey(doorX, doorY, doorZ)] = value;
  Location *p = pos[doorX][doorY][doorZ];
  for(int xp = 0; xp < p->shape->getWidth(); xp++) {
    for(int yp = 0; yp < p->shape->getDepth(); yp++) {
      session->getGameAdapter()->colorMiniMapPoint(doorX + xp, 
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

