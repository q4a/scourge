/***************************************************************************
                          creature.cpp  -  description
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

#include "creature.h"

/**
 * Formations are defined by 4 set of coordinates in 2d space.
 * These starting positions assume dir=Constants::MOVE_UP
 */
static const Sint16 layout[][4][2] = {
  { {0, 0}, {-1, 1}, {1, 1}, {0, 2} },  // DIAMOND_FORMATION
  { {0, 0}, {-1, 1}, {0, 1}, {-1, 2} },  // STAGGERED_FORMATION
  { {0, 0}, {1, 0}, {1, 1}, {0, 1} },   // SQUARE_FORMATION
  { {0, 0}, {0, 1}, {0, 2}, {0, 3} },   // ROW_FORMATION
  { {0, 0}, {-2, 2}, {0, 2}, {2, 2} },  // SCOUT_FORMATION
  { {0, 0}, {-1, 1}, {1, 1}, {0, 3} }   // CROSS_FORMATION
};

Creature::Creature(Scourge *scourge, GLShape *shape, PlayerChar *pc) {
  this->shape = shape;
  this->scourge = scourge;
  this->pc = pc;
  this->x = this->y = this->z = 0;
  this->dir = Constants::MOVE_UP;
  this->next = NULL;
  this->formation = DIAMOND_FORMATION;
  this->index = 0;
  this->tx = this->ty = -1;  
  this->selX = this->selY = -1;
  this->quadric = gluNewQuadric();
}

Creature::~Creature(){
}

bool Creature::move(Uint16 dir, Map *map) {
  Uint16 oldDir;
	while(true) {
    oldDir = this->dir;
    this->dir = dir;    

		Location *position = map->moveCreature(x, y, z, dir, this);
		if(position == NULL) {
			switch(dir) {
			case Constants::MOVE_UP:
				moveTo(x, y - 1, z);
				break;
			case Constants::MOVE_DOWN:
				moveTo(x, y + 1, z);
				break;
			case Constants::MOVE_LEFT:
				moveTo(x - 1, y, z);
				break;
			case Constants::MOVE_RIGHT:
				moveTo(x + 1, y, z);
				break;
			}
      ((MD2Shape*)shape)->setDir(dir);
      scourge->setPartyMotion(Constants::MOTION_MOVE_TOWARDS);
			return true;
		} else {
			Creature *partyMember = scourge->isPartyMember(position);
			if(partyMember) {
      
#ifdef ENABLE_PARTY_SWAP
				// if it's another party member, switch places and try again
        // This works, but causes an irritating 'jump'
				map->switchPlaces(partyMember->getX(), partyMember->getY(), partyMember->getZ(),
													getX(), getY(), getZ());
				Sint16 tmpX = getX();
				Sint16 tmpY = getY();
				Sint16 tmpZ = getZ();
				moveTo(partyMember->getX(), partyMember->getY(), partyMember->getZ());
				partyMember->moveTo(tmpX, tmpY, tmpZ);
        // adjust the map's center if the player was moved
        if(partyMember == scourge->getPlayer()) {
          scourge->getMap()->center(partyMember->getX(), partyMember->getY());
        } else if(this == scourge->getPlayer()) {
          scourge->getMap()->center(getX(), getY());
        }
        return true;
#else
        // Out of my way!
        partyMember->setMotion(Constants::MOTION_MOVE_AWAY);
        scourge->moveParty();
        return false;
#endif        
			} else {
        this->dir = oldDir;
        scourge->setPartyMotion(Constants::MOTION_MOVE_TOWARDS);
				return false;
			}
		}
	}
}

void Creature::follow(Map *map) {
  // find out where the creature should be
	Sint16 px, py, pz;

  if(getMotion() == Constants::MOTION_MOVE_AWAY) {
    findCorner(&px, &py, &pz);
  } else {
  	getFormationPosition(&px, &py, &pz);   
  }

  gotoPosition(map, px, py, pz);
  
}

void Creature::moveToLocator(Map *map) {
    if(selX > -1) {
        gotoPosition(map, selX, selY, 0);
    }
}

void Creature::gotoPosition(Map *map, Sint16 px, Sint16 py, Sint16 pz) {
    // If the target moved, get the best path to the location
    if(!(tx == px && ty == py)) {
        tx = px;
      ty = py;
      bestPathPos = 1; // skip 0th position; it's the starting location
      Util::findPath(getX(), getY(), getZ(), px, py, pz, &bestPath, scourge->getMap(), getShape());
    }

    if((int)bestPath.size() > bestPathPos) {
      // take a step on the bestPath
      Location location = bestPath.at(bestPathPos);
      // if we can't step there, someone else has moved there ahead of us
      Uint16 oldDir = dir;
      //dir = next->getDir();
      if(getX() < location.x) dir = Constants::MOVE_RIGHT;
      else if(getX() > location.x) dir = Constants::MOVE_LEFT;
      else if(getY() < location.y) dir = Constants::MOVE_DOWN;
      else if(getY() > location.y) dir = Constants::MOVE_UP;
      Location *position = map->moveCreature(getX(), getY(), getZ(),
                                             location.x, location.y, getZ(),
                                             this);
      if(!position) {
        bestPathPos++;
        moveTo(location.x, location.y, getZ());
        ((MD2Shape*)shape)->setDir(dir);      
      } else {
        dir = oldDir;
      }
    }
}

void Creature::getFormationPosition(Sint16 *px, Sint16 *py, Sint16 *pz) {
	Sint16 dx = layout[formation][index][0];
	Sint16 dy = -layout[formation][index][1];

  // get the angle
  float angle = 0;
  if(next->getDir() == Constants::MOVE_RIGHT) angle = 270.0;
  else if(next->getDir() == Constants::MOVE_DOWN) angle = 180.0;
  else if(next->getDir() == Constants::MOVE_LEFT) angle = 90.0;

  // rotate points
  if(angle != 0) { 
    Util::rotate(dx, dy, px, py, angle);
  } else {
    *px = dx;
    *py = dy;
  }

  // translate
	*px = (*(px) * getShape()->getWidth()) + next->getX();
	*py = (-(*(py)) * getShape()->getDepth()) + next->getY();
	*pz = next->getZ();
}

/**
  Used to move away from the player. Find the nearest corner of the map.
*/
void Creature::findCorner(Sint16 *px, Sint16 *py, Sint16 *pz) {
  if(getX() < scourge->getPlayer()->getX() &&
     getY() < scourge->getPlayer()->getY()) {
    *px = *py = *pz = 0;
    return;
  }
  if(getX() >= scourge->getPlayer()->getX() &&
     getY() < scourge->getPlayer()->getY()) {
    *px = MAP_WIDTH;
    *py = *pz = 0;
    return;
  }
  if(getX() < scourge->getPlayer()->getX() &&
     getY() >= scourge->getPlayer()->getY()) {
    *px = *pz = 0;
    *py = MAP_DEPTH;
    return;
  }
  if(getX() >= scourge->getPlayer()->getX() &&
     getY() >= scourge->getPlayer()->getY()) {
    *px = MAP_WIDTH;
    *py = MAP_DEPTH;
    *pz = 0;
    return;
  }    
}

void Creature::setNext(Creature *next, int index) { 
  this->next = next; 
  this->index = index;
  // stand in formation
  Sint16 px, py, pz;
  getFormationPosition(&px, &py, &pz);
  moveTo(px, py, pz);
}

void Creature::setNextDontMove(Creature *next, int index) {
  this->next = next; 
  this->index = index;
}

