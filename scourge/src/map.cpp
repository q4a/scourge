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
  itemCount = 0;
  selX = selY = selZ = MAP_WIDTH + 1;

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
 * Get the distance to the viewer for comparisson purposes.
 * Assumes that the current modelview matrix contains the right
 * rotations, etc.
 * 
 * 	You can extract the (modelview) matrix as usual, into a 16-element array. 
 *	Then you multiply the homogenous point in worldspace (x, y, z, 1) with this matrix, and you have the point's coordinates relative the origin. 
 *	Then you calculate the distance from the origin to the transformed point using pythagoras theorem. Then you have the distance from the viewpoint which you can use for depthsorting.
 */
/*
GLfloat Map::getDistance(float xpos, float ypos, float zpos) {
    GLfloat res[16];
    GLfloat m[16];
    for(int i = 0; i < 16; i++) m[i] = 0;
    m[0] = xpos;
    m[5] = ypos;
    m[10] = zpos;
    m[15] = 1.0f;
    glPushMatrix();
    glMatrixMode(GL_MODELVIEW);
    glMultMatrixf(m);
    glGetFloatv(GL_MODELVIEW, res);
    glPopMatrix();
    return (res[0] * res[0] + res[5] * res[5] + res[10] * res[10]);
}
*/

void Map::drawGround() {
    float xpos2, ypos2, zpos2;

    Shape *shape = NULL;  
    GLuint name;

    for(int yp = 0; yp < MAP_VIEW_DEPTH; yp++) {
        for(int xp = 0; xp < MAP_VIEW_WIDTH; xp++) {
            for(int zp = 0; zp < MAP_VIEW_HEIGHT; zp++) {
                shape = (zp == 0 ? floorPositions[getX() + xp][getY() + yp] : NULL);
                // draw the floor
                if(shape) {

                    xpos2 = ((float)(xp) / GLShape::DIV);
                    ypos2 = (((float)(yp) - (float)shape->getDepth()) / GLShape::DIV);
                    zpos2 = (float)(zp) / GLShape::DIV;
                    name = getX() + xp + (MAP_WIDTH * (getY() + yp)) + (MAP_WIDTH * MAP_DEPTH * zp);
                    //name = ((GLShape*)shape)->getShapePalIndex();

                    glTranslatef( xpos2, ypos2, zpos2);
                    // encode this shape's map location in its name
                    glPushName( name );
                    shape->draw();
                    glPopName();
                    glTranslatef( -xpos2, -ypos2, -zpos2);
                }
            }
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

        if(scourge->getMovingItem()) {
            shape = scourge->getMovingItem()->getShape();
        } else {
            shape = scourge->getShapePalette()->getShape(ShapePalette::LOCATOR_INDEX);
        }
        xpos2 = ((float)(selX - getX()) / GLShape::DIV);
        ypos2 = (((float)(selY - getY()) - (float)shape->getDepth()) / GLShape::DIV);
        zpos2 = (float)(0) / GLShape::DIV;

        name = 0;

        doDrawShape(xpos2, ypos2, zpos2, 
                    shape, 
                    name);
    }
}

void Map::setupShapes() {
  float xpos2, ypos2, zpos2;
  
  Shape *shape = NULL;  
  GLuint name;
  laterCount = stencilCount = otherCount = 0;
  mapChanged = false;
  
  for(int yp = 0; yp < MAP_VIEW_DEPTH; yp++) {
	for(int xp = 0; xp < MAP_VIEW_WIDTH; xp++) {
	  for(int zp = 0; zp < MAP_VIEW_HEIGHT; zp++) {
		// draw the walls
		if(pos[getX() + xp][getY() + yp][zp]) {
		  
		  // draw the shape and item
		  shape = NULL;
		  if(pos[getX() + xp][getY() + yp][zp]->x == getX() + xp &&
			 pos[getX() + xp][getY() + yp][zp]->y == getY() + yp &&
			 pos[getX() + xp][getY() + yp][zp]->z == zp) {
			shape = pos[getX() + xp][getY() + yp][zp]->shape;
			if(shape) {
			  xpos2 = ((float)(xp) / GLShape::DIV);
			  ypos2 = (((float)(yp) - (float)shape->getDepth()) / GLShape::DIV);
			  zpos2 = (float)(zp) / GLShape::DIV;
			  name = getX() + xp + (MAP_WIDTH * (getY() + yp)) + (MAP_WIDTH * MAP_DEPTH * zp);
			  
			  if(shape->isStencil()) {
				//doDrawShape(xpos2, ypos2, zpos2, shape, name);
				stencil[stencilCount].xpos = xpos2;
				stencil[stencilCount].ypos = ypos2;
				stencil[stencilCount].zpos = zpos2;
				stencil[stencilCount].shape = shape;
				stencil[stencilCount].name = name;
				stencilCount++;
			  } else if(!shape->isStencil()) {
				if(shape->drawFirst()) {
				  //doDrawShape(xpos2, ypos2, zpos2, shape, name);
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
		  }
		}
	  }
	}
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
  if(mapChanged) setupShapes();
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
      drawGround();
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
      glEnable(GL_LIGHTING);
      glDepthMask(GL_TRUE);    
      glDisable(GL_BLEND);

      drawLocator();
  }
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

Location *Map::movePosition(Sint16 x, Sint16 y, Sint16 z, Uint16 dir,Shape *newShape) {
	Sint16 nx = x;
	Sint16 ny = y;
	Sint16 nz = z;
	switch(dir) {
	case Constants::MOVE_UP: ny--; break;
	case Constants::MOVE_DOWN: ny++; break;
	case Constants::MOVE_LEFT: nx--; break;
	case Constants::MOVE_RIGHT: nx++; break;
	}
	return movePosition(x,y,z,nx,ny,nz,newShape);
}

Location *Map::movePosition(Sint16 x, Sint16 y, Sint16 z, 
												 Sint16 nx, Sint16 ny, Sint16 nz,
                         Shape *newShape) {
  Location *position = isBlocked(nx, ny, nz, x, y, z, newShape);
	if(position) return position;
	// move position
  removePosition(x, y, z);
	setPosition(nx, ny, nz, newShape);
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

void Map::addItem(Item *item, int x, int y, int z) {
  items[itemCount].item = item;
  items[itemCount].x = x;
  items[itemCount].y = y;
  items[itemCount].z = z;
  itemCount++;
  setItem(x, y, z, item);
}

Item *Map::getItem(Sint16 mapx, Sint16 mapy, Sint16 mapz) {
  for(int i = 0; i < itemCount; i++) {
    if(items[i].x <= mapx && items[i].x + items[i].item->getShape()->getWidth() > mapx &&
       items[i].y >= mapy && items[i].y - items[i].item->getShape()->getDepth() < mapy &&
       items[i].z == mapz) {
      return items[i].item;
    }
  }
  return NULL;
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

