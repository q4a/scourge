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

Map::Map(Scourge *scourge){
  zoom = 1.0f;
  zoomIn = zoomOut = false;
  x = y = 0;
  move = 0;
  selectMode = false;
  floorOnly = false;
  selX = selY = selZ = MAP_WIDTH + 1;
  oldLocatorSelX = oldLocatorSelY = oldLocatorSelZ = selZ;
  
  
  mapChanged = true;
  
  descriptionCount = 0;
  
  this->xrot = 0.0f; // if 0, artifacts appear
  this->yrot = 30.0f;
  this->zrot = -45.0f;
  this->xRotating = this->yRotating = this->zRotating = 0.0f;

  this->xpos = (float)(scourge->getSDLHandler()->getScreen()->w) / 2.0f;
  this->ypos = (float)(scourge->getSDLHandler()->getScreen()->h) / 2.0f;
  this->zpos = 0.0f;  
  
  this->scourge = scourge;  
  this->debugGridFlag = false;
  this->drawGridFlag = false;
  
  // initialize shape graph of "in view shapes"
  for(int x = 0; x < MAP_WIDTH; x++) {
	for(int y = 0; y < MAP_DEPTH; y++) {
      floorPositions[x][y] = NULL;
	  for(int z = 0; z < MAP_VIEW_HEIGHT; z++) {
        pos[x][y][z] = NULL;
      }      
    }
  }

  lightMapChanged = true;
}

Map::~Map(){
    for(int xp = 0; xp < MAP_WIDTH; xp++) {
        for(int yp = 0; yp < MAP_DEPTH; yp++) {
            for(int zp = 0; zp < MAP_VIEW_HEIGHT; zp++) {
                if(pos[xp][yp][zp]) {
                    delete pos[xp][yp][zp];
                }
            }
        }
    }
}

void Map::center(Sint16 x, Sint16 y) { 
	// relocate
	this->x = x - MAP_VIEW_WIDTH / 2; 
	this->y = y - MAP_VIEW_DEPTH / 2; 
}

/**
   If 'ground' is true, it draws the ground layer.
   Otherwise the shape arrays (other, stencil, later) are populated.
*/
void Map::setupShapes(bool ground) {
  if(!ground) {
	laterCount = stencilCount = otherCount = 0;
	mapChanged = false;
  }

  int chunkOffsetX = 0;
  int chunkStartX = (getX() - MAP_OFFSET) / MAP_UNIT;
  int mod = (getX() - MAP_OFFSET) % MAP_UNIT;
  if(mod) {
	chunkOffsetX = -mod;
  }
  int chunkEndX = MAP_VIEW_WIDTH / MAP_UNIT + chunkStartX;

  int chunkOffsetY = 0;
  int chunkStartY = (getY() - MAP_OFFSET) / MAP_UNIT;
  mod = (getY() - MAP_OFFSET) % MAP_UNIT;
  if(mod) {
	chunkOffsetY = -mod;
  }
  int chunkEndY = MAP_VIEW_DEPTH / MAP_UNIT + chunkStartY;

  Shape *shape;
  int posX, posY;
  float xpos2, ypos2, zpos2;
  for(int chunkX = chunkStartX; chunkX < chunkEndX; chunkX++) {
	for(int chunkY = chunkStartY; chunkY < chunkEndY; chunkY++) {
	  int doorValue = 0;

	  // if this chunk is not lit, don't draw it
	  if(!lightMap[chunkX][chunkY]) {
		if(ground) continue; 
		else {
		  // see if a door has to be drawn
		  for(int yp = MAP_UNIT_OFFSET + 1; yp < MAP_UNIT; yp++) {
			bool found = false;
			if(chunkX - 1 >= 0 && lightMap[chunkX - 1][chunkY]) {
			  for(int xp = MAP_UNIT - MAP_UNIT_OFFSET; xp < MAP_UNIT; xp++) {
				posX = (chunkX - 1) * MAP_UNIT + xp + MAP_OFFSET;
				posY = chunkY * MAP_UNIT + yp + MAP_OFFSET + 1;
				if(posX >= 0 && posX < MAP_WIDTH && 
				   posY >= 0 && posY < MAP_DEPTH &&
				   pos[posX][posY][0]) {
				  found = true;
				  break;
				}
			  }
			}
			if(!found) doorValue |= Constants::MOVE_LEFT;
			found = false;
			if(chunkX + 1 < MAP_WIDTH / MAP_UNIT && lightMap[chunkX + 1][chunkY]) {
			  for(int xp = 0; xp < MAP_UNIT_OFFSET; xp++) {
				posX = (chunkX + 1) * MAP_UNIT + xp + MAP_OFFSET;
				posY = chunkY * MAP_UNIT + yp + MAP_OFFSET + 1;
				if(posX >= 0 && posX < MAP_WIDTH && 
				   posY >= 0 && posY < MAP_DEPTH &&
				   pos[posX][posY][0]) {
				  found = true;
				  break;
				}
			  }
			}
			if(!found) doorValue |= Constants::MOVE_RIGHT;
		  }
		  for(int xp = MAP_UNIT_OFFSET; xp < MAP_UNIT - MAP_UNIT_OFFSET; xp++) {
			bool found = false;
			if(chunkY - 1 >= 0 && lightMap[chunkX][chunkY - 1]) {
			  for(int yp = MAP_UNIT - MAP_UNIT_OFFSET; yp < MAP_UNIT; yp++) {
				posX = chunkX * MAP_UNIT + xp + MAP_OFFSET;
				posY = (chunkY - 1) * MAP_UNIT + yp + MAP_OFFSET + 1;
				if(posX >= 0 && posX < MAP_WIDTH && 
				   posY >= 0 && posY < MAP_DEPTH &&
				   pos[posX][posY][0]) {
				  found = true;
				  break;
				}
			  }
			}
			if(!found) doorValue |= Constants::MOVE_UP;
			found = false;
			if(chunkY + 1 >= 0 && lightMap[chunkX][chunkY + 1]) {
			  for(int yp = 0; yp < MAP_UNIT_OFFSET; yp++) {
				posX = chunkX * MAP_UNIT + xp + MAP_OFFSET;
				posY = (chunkY + 1) * MAP_UNIT + yp + MAP_OFFSET + 1;
				if(posX >= 0 && posX < MAP_WIDTH && 
				   posY >= 0 && posY < MAP_DEPTH &&
				   pos[posX][posY][0]) {
				  found = true;
				  break;
				}
			  }
			}
			if(!found) doorValue |= Constants::MOVE_DOWN;
		  }
		  if(doorValue == 0) continue;
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

		  if(ground) {
			shape = floorPositions[posX][posY];
			if(shape) {
			  xpos2 = (float)((chunkX - chunkStartX) * MAP_UNIT + 
							  xp + chunkOffsetX) / GLShape::DIV;
			  ypos2 = (float)((chunkY - chunkStartY) * MAP_UNIT - 
							  shape->getDepth() +
							  yp + chunkOffsetY) / GLShape::DIV;
			  drawGroundPosition(posX, posY,
								 xpos2, ypos2,
								 shape); 		  
			}
		  } else {
			for(int zp = 0; zp < MAP_VIEW_HEIGHT; zp++) {
			  if(pos[posX][posY][zp] && 
				 pos[posX][posY][zp]->x == posX &&
				 pos[posX][posY][zp]->y == posY &&
				 pos[posX][posY][zp]->z == zp) {
				shape = pos[posX][posY][zp]->shape;

				// FIXME: this draws more doors than needed... 
				// it should use doorValue to figure out what needs to be drawn
				if((!lightMap[chunkX][chunkY] && 
					(shape == scourge->getShapePalette()->getShape(ShapePalette::NS_DOOR_INDEX) ||
					 shape == scourge->getShapePalette()->getShape(ShapePalette::EW_DOOR_INDEX) ||
					 shape == scourge->getShapePalette()->getShape(ShapePalette::NS_DOOR_TOP_INDEX) ||
					 shape == scourge->getShapePalette()->getShape(ShapePalette::EW_DOOR_TOP_INDEX) ||
					 shape == scourge->getShapePalette()->getShape(ShapePalette::DOOR_SIDE_INDEX))) ||
				   (lightMap[chunkX][chunkY] && shape)) {
				  xpos2 = (float)((chunkX - chunkStartX) * MAP_UNIT + 
								  xp + chunkOffsetX) / GLShape::DIV;
				  ypos2 = (float)((chunkY - chunkStartY) * MAP_UNIT - 
								  shape->getDepth() + 
								  yp + chunkOffsetY) / GLShape::DIV;
				  zpos2 = (float)(zp) / GLShape::DIV;
				  setupPosition(posX, posY, zp,
								xpos2, ypos2, zpos2,
								shape); 		  
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
  shape->draw();
  glPopName();
  glTranslatef( -xpos2, -ypos2, 0.0f);
}

void Map::setupPosition(int posX, int posY, int posZ,
						float xpos2, float ypos2, float zpos2,
						Shape *shape) {
  GLuint name;
  name = posX + (MAP_WIDTH * (posY)) + (MAP_WIDTH * MAP_DEPTH * posZ);		
  if(shape->isStencil()) {
	stencil[stencilCount].xpos = xpos2;
	stencil[stencilCount].ypos = ypos2;
	stencil[stencilCount].zpos = zpos2;
	stencil[stencilCount].shape = shape;
	stencil[stencilCount].name = name;
	stencilCount++;
  } else if(!shape->isStencil()) {
	if(shape->drawFirst()) {
	  other[otherCount].xpos = xpos2;
	  other[otherCount].ypos = ypos2;
	  other[otherCount].zpos = zpos2;
	  other[otherCount].shape = shape;
	  other[otherCount].name = name;
	  otherCount++;
	}
	if(shape->drawLater()) {
	  later[laterCount].xpos = xpos2;
	  later[laterCount].ypos = ypos2;
	  later[laterCount].zpos = zpos2;
	  later[laterCount].shape = shape;
	  later[laterCount].name = name;
	  laterCount++;
	}
  }
}

void Map::drawLocator() {
    float xpos2, ypos2, zpos2;

    Shape *shape = NULL;  
    GLuint name;

    // draw the locator
    if(selX >= getX() && selX < getX() + MAP_VIEW_WIDTH &&
       selY >= getY() && selY < getY() + MAP_VIEW_DEPTH) {

		// don't move objects across walls
		if(scourge->getMovingItem() && 
		   (isWallBetween(selX, selY, 0, 
						  scourge->getParty(0)->getX(),
						  scourge->getParty(0)->getY(),
						  0) ||
			!shapeFits(scourge->getMovingItem()->getShape(), selX, selY, 0)) ) {
		  selX = oldLocatorSelX;
		  selY = oldLocatorSelY;
		}

        if(scourge->getMovingItem()) {
            shape = scourge->getMovingItem()->getShape();
        } else {
            shape = scourge->getShapePalette()->getShape(ShapePalette::LOCATOR_INDEX);
        }
        xpos2 = ((float)(selX - getX()) / GLShape::DIV);
        ypos2 = (((float)(selY - getY() - 1) - (float)shape->getDepth()) / GLShape::DIV);
        zpos2 = (float)(0) / GLShape::DIV;

        name = 0;
        doDrawShape(xpos2, ypos2, zpos2, 
                    shape, 
                    name);
		oldLocatorSelX = selX;
		oldLocatorSelY = selY;
    }
}

void Map::draw(SDL_Surface *surface) {
  // move the map
  if(move & Constants::MOVE_UP) {
    if(scourge->getPlayer()->move(Constants::MOVE_UP, this)) this->y--;
  }
  if(move & Constants::MOVE_DOWN) {
    if(scourge->getPlayer()->move(Constants::MOVE_DOWN, this)) this->y++;
  }
  if(move & Constants::MOVE_LEFT) {
    if(scourge->getPlayer()->move(Constants::MOVE_LEFT, this)) this->x--;
  }
  if(move & Constants::MOVE_RIGHT) {
    if(scourge->getPlayer()->move(Constants::MOVE_RIGHT, this)) this->x++;
  }
  
  if(zoomIn) {
	if(zoom <= 0.5f) {
	  zoomOut = false;
	} else {
	  zoom /= ZOOM_DELTA; 
	  xpos = (int)((float)scourge->getSDLHandler()->getScreen()->w / zoom / 2.0f);
	  ypos = (int)((float)scourge->getSDLHandler()->getScreen()->h / zoom / 2.0f);
	}
  } else if(zoomOut) {
	if(zoom >= 2.8f) {
	  zoomOut = false;
	} else {
	  zoom *= ZOOM_DELTA; 
	  xpos = (int)((float)scourge->getSDLHandler()->getScreen()->w / zoom / 2.0f);
	  ypos = (int)((float)scourge->getSDLHandler()->getScreen()->h / zoom / 2.0f);
	}
  }

  scourge->moveParty();

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
  if(mapChanged) setupShapes(false);
  if(selectMode) {
      for(int i = 0; i < otherCount; i++) doDrawShape(&other[i]);
  } else {  
      // draw the creatures/objects/doors/etc.
      for(int i = 0; i < otherCount; i++) doDrawShape(&other[i]);

      // create a stencil for the walls
      glDisable(GL_DEPTH_TEST);
      glColorMask(0,0,0,0);
      glEnable(GL_STENCIL_TEST);
      glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);
      glStencilFunc(GL_ALWAYS, 1, 1);
      for(int i = 0; i < stencilCount; i++) doDrawShape(&stencil[i]);
  
      // Use the stencil to draw
      glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
      glEnable(GL_DEPTH_TEST);
      glStencilOp(GL_KEEP, GL_KEEP, GL_KEEP);
      glStencilFunc(GL_NOTEQUAL, 1, 0xffffffff);  // draw if stencil!=1
      // draw the ground
	  setupShapes(true);
      glDisable(GL_STENCIL_TEST);
      
      // draw the blended walls
      glEnable(GL_BLEND);  
      glDepthMask(GL_FALSE);
      //glDisable(GL_LIGHTING);
      glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
      for(int i = 0; i < stencilCount; i++) doDrawShape(&stencil[i]);
      //glEnable(GL_LIGHTING);
      glDepthMask(GL_TRUE);    
      glDisable(GL_BLEND);

      // draw the effects
      glEnable(GL_BLEND);  
      glDepthMask(GL_FALSE);
      glDisable(GL_LIGHTING);
      for(int i = 0; i < laterCount; i++) {
          later[i].shape->setupBlending();
          doDrawShape(&later[i]);
          later[i].shape->endBlending();
      }
	  drawShade();
      glEnable(GL_LIGHTING);
      glDepthMask(GL_TRUE);    
      glDisable(GL_BLEND);

      drawLocator();
  }
}

void Map::drawShade() {
  glPushMatrix();
  glLoadIdentity();
  glDisable(GL_DEPTH_TEST);
  glDisable( GL_TEXTURE_2D );
  glBlendFunc(GL_SRC_COLOR, GL_ONE_MINUS_SRC_ALPHA);
  //scourge->setBlendFunc();
  for(int xt = 0; xt < scourge->getSDLHandler()->getScreen()->w; xt+=SHADE_SIZE) {
	for(int yt = 0; yt < scourge->getSDLHandler()->getScreen()->h; yt+=SHADE_SIZE) {
	  int xd = (scourge->getSDLHandler()->getScreen()->w / 2 - 30) - xt;
	  int yd = (scourge->getSDLHandler()->getScreen()->h / 2 - 30) - yt;
	  float dist = (float)((xd * xd) + (yd * yd));
	  float alpha = dist / 100000;
	  glColor4f( 0, 0, 0, alpha);

	  float zp = 0.0f;
	  glBegin( GL_QUADS );
      glVertex3f(xt + SHADE_SIZE, yt + SHADE_SIZE, zp);
      glVertex3f(xt + SHADE_SIZE, yt, zp);
      glVertex3f(xt, yt, zp);
      glVertex3f(xt, yt + SHADE_SIZE, zp);
	  glEnd();

	  glColor4f(1, 1, 1, 1);
	  //	  scourge->getSDLHandler()->texPrint(xt, yt, "%4.2f", alpha);
	  //	  scourge->getSDLHandler()->texPrint(xt, yt + 15, "%4.2f", dist);
	}
  }
  glEnable( GL_TEXTURE_2D );
  glEnable(GL_DEPTH_TEST);
  glPopMatrix();
}

void Map::doDrawShape(DrawLater *later) {
    doDrawShape(later->xpos, later->ypos, later->zpos, later->shape, later->name);
}

void Map::doDrawShape(float xpos2, float ypos2, float zpos2, Shape *shape, GLuint name) {
  glTranslatef( xpos2, ypos2, zpos2);

  // encode this shape's map location in its name
  glPushName( name );
  //glPushName( (GLuint)((GLShape*)shape)->getShapePalIndex() );
  ((GLShape*)shape)->setCameraRot(xrot, yrot, zrot);
  ((GLShape*)shape)->setCameraPos(xpos, ypos, zpos, xpos2, ypos2, zpos2);
  shape->draw();

  glPopName();

  glTranslatef( -xpos2, -ypos2, -zpos2);
}

void Map::showInfoAtMapPos(Uint16 mapx, Uint16 mapy, Uint16 mapz, char *message) {
  float xpos2 = ((float)(mapx - getX()) / GLShape::DIV);
  float ypos2 = ((float)(mapy - getY()) / GLShape::DIV);
  float zpos2 = (float)(mapz) / GLShape::DIV;
  glTranslatef( xpos2, ypos2, zpos2 + 100);

  glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
  //glRasterPos2f( 0, 0 );
  scourge->getSDLHandler()->texPrint(0, 0, "%s", message);

  glTranslatef( -xpos2, -ypos2, -(zpos2 + 100));
}

void Map::showCreatureInfo(Creature *creature) {
  showInfoAtMapPos(creature->getX(), creature->getY(), creature->getZ(), creature->getName());
}

/**
 * Initialize the map view (translater, rotate)
 */
void Map::initMapView(bool ignoreRot) {
  glLoadIdentity();

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

  float startx = -((float)MAP_VIEW_WIDTH / 2.0) / GLShape::DIV;
  float starty = -((float)MAP_VIEW_DEPTH / 2.0) / GLShape::DIV;
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
  Location *position = isBlocked(nx, ny, nz, x, y, z, newCreature->getShape());
  if(position) return position;
  // move position
  removeCreature(x, y, z);
  setCreature(nx, ny, nz, newCreature);
  return NULL;
}

void Map::setFloorPosition(Sint16 x, Sint16 y, Shape *shape) {
  floorPositions[x][y] = shape;
}

Shape *Map::removeFloorPosition(Sint16 x, Sint16 y) {
	Shape *shape = NULL;
  if(floorPositions[x][y]) {
    shape = floorPositions[x][y];
	  floorPositions[x][y] = 0;
  }
	return shape;
}

Location *Map::isBlocked(Sint16 x, Sint16 y, Sint16 z, 
											   Sint16 shapeX, Sint16 shapeY, Sint16 shapeZ, 
											   Shape *s) {
	for(int sx = 0; sx < s->getWidth(); sx++) {
		for(int sy = 0; sy < s->getDepth(); sy++) {
			for(int sz = 0; sz < s->getHeight(); sz++) {
        if(pos[x + sx][y - sy][z + sz] &&
           pos[x + sx][y - sy][z + sz]->shape &&
           !(pos[x + sx][y - sy][z + sz]->x == shapeX &&
						 pos[x + sx][y - sy][z + sz]->y == shapeY &&
						 pos[x + sx][y - sy][z + sz]->z == shapeZ)) {
          return pos[x + sx][y - sy][z + sz];
        }
			}
		}
	}
	return NULL;
}

void Map::switchPlaces(Sint16 x1, Sint16 y1, Sint16 z1,
											 Sint16 x2, Sint16 y2, Sint16 z2) {
	Shape *shape1 = removePosition(x1, y1, z1);
	Shape *shape2 = removePosition(x2, y2, z2);

  Location *position = isBlocked(x2, y2, z2, x1, y1, z1, shape1);
	if(position) {
    // can't do it
  	setPosition(x1, y1, z1, shape1);
	  setPosition(x2, y2, z2, shape2);
    return;
  }
	// move it
	setPosition(x2, y2, z2, shape1);

  position = isBlocked(x1, y1, z1, x2, y2, z2, shape2);
	if(position) {
    // can't do it
    removePosition(x2, y2, z2); // undo previous step
  	setPosition(x1, y1, z1, shape1);
	  setPosition(x2, y2, z2, shape2);
    return;
  }
	// move it
	setPosition(x1, y1, z1, shape2);  
}

Location *Map::getPosition(Sint16 x, Sint16 y, Sint16 z) {
  if(pos[x][y][z] &&
     ((pos[x][y][z]->shape &&
      pos[x][y][z]->x == x &&
      pos[x][y][z]->y == y &&
      pos[x][y][z]->z == z))) return pos[x][y][z];
  return NULL;
}

void Map::addDescription(char *desc) {
  if(descriptionCount > 0) {
    for(int i = descriptionCount; i >= 1; i--) {
      descriptions[i] = descriptions[i - 1];
    }
  }
  if(descriptionCount < 200) descriptionCount++;
  descriptions[0] = desc;
  fprintf(stderr, "Added description. Count=%u\n", descriptionCount);
}

void Map::drawDescriptions() {
  glColor4f(1.0f, 1.0f, 0.4f, 1.0f);
  int y = TOP_GUI_HEIGHT - 5;
  if(descriptionCount <= 5) y = descriptionCount * 15;
  int index =  0;
  while(y > 5 && index < descriptionCount) {    
    glRasterPos2f( (float)5, (float)y );
    scourge->getSDLHandler()->texPrint(5, y, "%s", descriptions[index]);

    y -= 15;
    index++;
  }
}

void Map::handleMouseClick(Uint16 mapx, Uint16 mapy, Uint16 mapz, Uint8 button) {
    if(mapx < MAP_WIDTH) {
        if(button == SDL_BUTTON_RIGHT) {
            fprintf(stderr, "\tclicked map coordinates: x=%u y=%u z=%u\n", mapx, mapy, mapz);
            Location *loc = getPosition(mapx, mapy, mapz);
            if(loc) {
                char *description = NULL;
                Item *item = loc->item;
                fprintf(stderr, "\titem?%s\n", (item ? "yes" : "no"));
                if( item ) {
                    description = item->getShortDescription();
                }          
                if(!description) {
                    Shape *shape = loc->shape;
                    fprintf(stderr, "\tshape?%s\n", (shape ? "yes" : "no"));
                    if(shape) {
                        description = shape->getRandomDescription();
                    }        
                }
                if(description) {
                    //            map->addDescription(x, y, mapx, mapy, mapz, description);
                    addDescription(description);
                }
            }
        }
    }
}

void Map::handleMouseMove(Uint16 mapx, Uint16 mapy, Uint16 mapz) {
    if(mapx < MAP_WIDTH) {
        selX = mapx;
        selY = mapy;
        selZ = mapz;
    }
}

void Map::setPosition(Sint16 x, Sint16 y, Sint16 z, Shape *shape) {
  if(shape) {
	mapChanged = true;
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
        for(int zp = 0; zp < shape->getHeight(); zp++) {
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

void Map::setCreature(Sint16 x, Sint16 y, Sint16 z, Creature *creature) {
  if(creature) {
    if(creature->getShape()) {
	  mapChanged = true;
      for(int xp = 0; xp < creature->getShape()->getWidth(); xp++) {
        for(int yp = 0; yp < creature->getShape()->getDepth(); yp++) {
          for(int zp = 0; zp < creature->getShape()->getHeight(); zp++) {
            if(!pos[x + xp][y - yp][z + zp]) {
              pos[x + xp][y - yp][z + zp] = new Location();
            }
            pos[x + xp][y - yp][z + zp]->item = NULL;
			pos[x + xp][y - yp][z + zp]->shape = creature->getShape();
			pos[x + xp][y - yp][z + zp]->creature = creature;
            pos[x + xp][y - yp][z + zp]->x = x;
            pos[x + xp][y - yp][z + zp]->y = y;
            pos[x + xp][y - yp][z + zp]->z = z;
          }
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
    for(int xp = 0; xp < creature->getShape()->getWidth(); xp++) {
      for(int yp = 0; yp < creature->getShape()->getDepth(); yp++) {
        for(int zp = 0; zp < creature->getShape()->getHeight(); zp++) {	  
		  delete pos[x + xp][y - yp][z + zp];
		  pos[x + xp][y - yp][z + zp] = NULL;		
        }
      }
    }
  }
  return creature;
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
	  if(x1 < x2) x += 1.0f / abs(m);
	  else x += -1.0f / abs(m);
	} else {
	  if(x1 < x2) x += 1.0f;
	  else x -= 1.0f;
	  if(y1 < y2) y += abs(m);
	  else y += -1.0 * abs(m);
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

// the world has changed...
void Map::configureLightMap() {
  fprintf(stderr, "configureLightMap\n");
  lightMapChanged = false;

  // draw nothing at first
  for(int x = 0; x < MAP_WIDTH / MAP_UNIT; x++) {
	for(int y = 0; y < MAP_DEPTH / MAP_UNIT; y++) {
	  lightMap[x][y] = 0;
	}
  }

  int chunkX = (scourge->getParty(0)->getX() + 
				(scourge->getParty(0)->getShape()->getWidth() / 2) - 
				MAP_OFFSET) / MAP_UNIT;
  int chunkY = (scourge->getParty(0)->getY() - 
				(scourge->getParty(0)->getShape()->getDepth() / 2) - 
				MAP_OFFSET) / MAP_UNIT;

  traceLight(chunkX, chunkY);
}

void Map::traceLight(int chunkX, int chunkY) {
  if(chunkX < 0 || chunkX >= MAP_WIDTH / MAP_UNIT ||
	 chunkY < 0 || chunkY >= MAP_DEPTH / MAP_UNIT)
	return;

  // already visited?
  if(lightMap[chunkX][chunkY]) return;

  // let there be light
  lightMap[chunkX][chunkY] = 1;
  
  // can we go N?
  int x, y;
  bool blocked = false;
  x = chunkX * MAP_UNIT + MAP_OFFSET + (MAP_UNIT / 2);
  for(y = chunkY * MAP_UNIT + MAP_OFFSET - (MAP_UNIT / 2);
	  y < chunkY * MAP_UNIT + MAP_OFFSET + (MAP_UNIT / 2);
	  y++) {
   	if(isLocationBlocked(x, y, 0)) {
	  blocked = true;
	  break;
	}
  }
  if(!blocked) traceLight(chunkX, chunkY - 1);
  
  // can we go E?
  blocked = false;
  y = chunkY * MAP_UNIT + MAP_OFFSET + (MAP_UNIT / 2);
  for(x = chunkX * MAP_UNIT + MAP_OFFSET + (MAP_UNIT / 2);
	  x < chunkX * MAP_UNIT + MAP_OFFSET + (MAP_UNIT / 2) + MAP_UNIT;
	  x++) {
	if(isLocationBlocked(x, y, 0)) {
	  blocked = true;
	  break;
	}
  }
  if(!blocked) traceLight(chunkX + 1, chunkY);
  
  // can we go S?
  blocked = false;
  x = chunkX * MAP_UNIT + MAP_OFFSET + (MAP_UNIT / 2);
  for(y = chunkY * MAP_UNIT + MAP_OFFSET + (MAP_UNIT / 2);
	  y < chunkY * MAP_UNIT + MAP_OFFSET + (MAP_UNIT / 2) + MAP_UNIT;
	  y++) {
	if(isLocationBlocked(x, y, 0)) {
	  blocked = true;
	  break;
	}
  }
  if(!blocked) traceLight(chunkX, chunkY + 1);

  // can we go W?
  blocked = false;
  y = chunkY * MAP_UNIT + MAP_OFFSET + (MAP_UNIT / 2);
  for(x = chunkX * MAP_UNIT + MAP_OFFSET - (MAP_UNIT / 2);
	  x < chunkX * MAP_UNIT + MAP_OFFSET + (MAP_UNIT / 2);
	  x++) {
	if(isLocationBlocked(x, y, 0)) {
	  blocked = true;
	  break;
	}
  }
  if(!blocked) traceLight(chunkX - 1, chunkY);
}

bool Map::isLocationBlocked(int x, int y, int z) {
  if(x >= 0 && x < MAP_WIDTH && 
	 y >= 0 && y < MAP_DEPTH && 
	 z >= 0 && z < MAP_VIEW_HEIGHT) {
	Location *pos = getLocation(x, y, z);
	if(pos == NULL || pos->item || pos->creature) { 
	  return false;
	}
  }
  return true;
}
