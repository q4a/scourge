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

Creature::Creature(Scourge *scourge, Character *character, char *name) {
  this->scourge = scourge;
  this->character = character;
  this->monster = NULL;
  this->name = name;
  this->shape = scourge->getShapePalette()->getCreatureShape(character->getShapeIndex());
  sprintf(description, "%s the %s", name, character->getName());
  this->speed = 50;
  commonInit();
}

Creature::Creature(Scourge *scourge, Monster *monster) {
  this->scourge = scourge;
  this->character = NULL;
  this->monster = monster;
  this->name = monster->getType();
  this->shape = scourge->getShapePalette()->getCreatureShape(monster->getShapeIndex());
  sprintf(description, "You see %s", monster->getType());
  this->speed = monster->getSpeed();
  commonInit();
  monsterInit();
}

void Creature::commonInit() {
  this->x = this->y = this->z = 0;
  this->dir = Constants::MOVE_UP;
  this->next = NULL;
  this->formation = DIAMOND_FORMATION;
  this->index = 0;
  this->tx = this->ty = -1;  
  this->selX = this->selY = -1;
  this->quadric = gluNewQuadric();
  this->inventory_count = 0;
  for(int i = 0; i < Character::INVENTORY_COUNT; i++) {
	equipped[i] = MAX_INVENTORY_SIZE;
  }
  this->stateMod = 0;
  this->level = 1;
  this->exp = 0;
  this->hp = 0;
  this->ac = 0;
  this->targetCreature = NULL;
  this->lastTick = 0;
}

Creature::~Creature(){
}

bool Creature::move(Uint16 dir, Map *map) {

  // a hack for runaway creatures
  if(!(x > 10 && x < MAP_WIDTH - 10 &&
	   y > 10 && y < MAP_DEPTH - 10)) {
	if(monster) cerr << "hack for " << getName() << endl;
	return false;
  }

  scourge->setPartyMotion(Constants::MOTION_MOVE_TOWARDS);
  int nx = x;
  int ny = y;
  int nz = z;
  switch(dir) {
  case Constants::MOVE_UP:
	ny = y - 1;
	break;
  case Constants::MOVE_DOWN:
	ny = y + 1;
	break;
  case Constants::MOVE_LEFT:
	nx = x - 1;
	break;
  case Constants::MOVE_RIGHT:
	nx = x + 1;
	break;
  }
  map->removeCreature(x, y, z);
  //	if(monster) cerr << "trying to move " << getName() << endl;
  if(map->shapeFits(getShape(), nx, ny, nz)) {
	//if(monster) cerr << "\tshapeFits" << endl;
	map->setCreature(nx, ny, nz, this);
	((MD2Shape*)shape)->setDir(dir);
	moveTo(nx, ny, nz);
	setDir(dir);
	
	return true;
  } else {
	//if(monster) cerr << "\tdoesn't fit" << endl;
	map->setCreature(x, y, z, this);
	
	if(character) {
	  // Out of my way!
	  setMotion(Constants::MOTION_MOVE_AWAY);
	  move(dir, map);
	}
	
	return false;
  }
}

bool Creature::follow(Map *map) {
  // find out where the creature should be
  Sint16 px, py, pz;
  
  if(getMotion() == Constants::MOTION_MOVE_AWAY) {
    findCorner(&px, &py, &pz);
  } else {
	// optimization, only move when the followed creature is moving
  	getFormationPosition(&px, &py, &pz);
  }
  
  return gotoPosition(map, px, py, pz);
}

bool Creature::moveToLocator(Map *map, bool single_step) {
  bool moved = false;
  if(selX > -1) {
	moved = gotoPosition(map, selX, selY, 0);
	// in single step mode, ignore the rest of the path
	//	if(single_step) selX = selY = -1;
  }
  return moved;
}

bool Creature::anyMovesLeft() {
  return(selX > -1 && (int)bestPath.size() > bestPathPos); 
}

bool Creature::gotoPosition(Map *map, Sint16 px, Sint16 py, Sint16 pz) {
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
	  return true;
	} else {
	  dir = oldDir;
	  return false;
	}
  }
  return false;
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
  //	*px = (*(px) * getShape()->getWidth()) + next->getX();
  //	*py = (-(*(py)) * getShape()->getDepth()) + next->getY();
  if(next->getSelX() > -1) {
	*px = (*(px) * getShape()->getWidth()) + next->getSelX();
	*py = (-(*(py)) * getShape()->getDepth()) + next->getSelY();
  } else {
	*px = (*(px) * getShape()->getWidth()) + next->getX();
	*py = (-(*(py)) * getShape()->getDepth()) + next->getY();
  }
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
  if(px > -1) moveTo(px, py, pz);
}

void Creature::setNextDontMove(Creature *next, int index) {
  this->next = next; 
  this->index = index;
}



Item *Creature::removeInventory(int index) { 
  Item *item = NULL;
  if(index < inventory_count) {
	// drop item if carrying it
	doff(index);
	// drop from inventory
	item = inventory[index];
	for(int i = index; i < inventory_count - 1; i++) {
	  inventory[i] = inventory[i + 1];
	}
	inventory_count--;
	// adjust equipped indexes too
	for(int i = 0; i < Character::INVENTORY_COUNT; i++) {
	  if(equipped[i] > index && equipped[i] < MAX_INVENTORY_SIZE) {
		equipped[i]--;
	  }
	}
  }
  return item;
}

void Creature::equipInventory(int index) {
  // doff
  if(doff(index)) return;
  // don
  // FIXME: take into account: two-handed weapons, race/class modifiers, min skill req-s., etc.
  Item *item = getInventory(index);
  for(int i = 0; i < Character::INVENTORY_COUNT; i++) {
	// if the slot is empty and the item can be worn here
	if(item->getRpgItem()->getEquip() & ( 1 << i ) && 
	   equipped[i] == MAX_INVENTORY_SIZE) {
		equipped[i] = index;
		return;
	}
  }
}

int Creature::doff(int index) {
  // doff
  for(int i = 0; i < Character::INVENTORY_COUNT; i++) {
	if(equipped[i] == index) {
	  equipped[i] = MAX_INVENTORY_SIZE;
	  return 1;
	}
  }
  return 0;
}

/**
   Get item at equipped index. (What is at equipped location?)
 */
Item *Creature::getEquippedInventory(int index) {
  int n = equipped[index];
  if(n < MAX_INVENTORY_SIZE) {
	return getInventory(n);
  }
  return NULL;
}

/**
   Get equipped index of inventory index. (Where is the item worn?)
*/
int Creature::getEquippedIndex(int index) {
  for(int i = 0; i < Character::INVENTORY_COUNT; i++) {
	if(equipped[i] == index) return i;
  }
  return -1;
}

bool Creature::isItemInInventory(Item *item) {
  for(int i = 0; i < inventory_count; i++) {
	if(inventory[i] == item) return true;
  }
  return false;
}

Item *Creature::getItemAtLocation(int location) {
  int i;
  for(i = 0; i < Character::INVENTORY_COUNT; i++) {
	if((1 << i) == location) {
	  if(equipped[i] < MAX_INVENTORY_SIZE) {
		return getInventory(equipped[i]);
	  } else {
		return NULL;
	  }
	}
  }
  return NULL;
}

Item *Creature::getBestWeapon(float dist) {
  Item *item = getItemAtLocation(Character::INVENTORY_RIGHT_HAND);
  if(item && item->getRpgItem()->getDistance() >= dist) return item;
  item = getItemAtLocation(Character::INVENTORY_LEFT_HAND);
  if(item && item->getRpgItem()->getDistance() >= dist) return item;
  item = getItemAtLocation(Character::INVENTORY_WEAPON_RANGED);
  if(item && item->getRpgItem()->getDistance() >= dist) return item;
  return NULL;
}


/**
   Create a party programmatically until the party editor is made.
 */
Creature **Creature::createHardCodedParty(Scourge *scourge) {
  Creature **pc = (Creature**)malloc(sizeof(Creature*) * 4);

  pc[0] = new Creature(scourge, Character::character_class[Character::assassin], "Alamont");
  pc[1] = new Creature(scourge, Character::character_class[Character::knight], "Barlett");
  pc[2] = new Creature(scourge, Character::character_class[Character::summoner], "Corinus");
  pc[3] = new Creature(scourge, Character::character_class[Character::naturalist], "Dialante");

  pc[0]->setLevel(1); 
  pc[0]->setExp(300);
  pc[0]->setHp();
  pc[0]->setStateMod(Constants::blessed, true);
  pc[0]->setStateMod(Constants::poisoned, true);
  for(int i = 0; i < Constants::SKILL_COUNT; i++) {
      pc[0]->setSkill(i, (int)(100.0 * rand()/RAND_MAX));
  }
  
  pc[1]->setLevel(1); 
  pc[1]->setExp(200);
  pc[1]->setHp();
  pc[1]->setStateMod(Constants::drunk, true);
  pc[1]->setStateMod(Constants::cursed, true);      
  for(int i = 0; i < Constants::SKILL_COUNT; i++) {
      pc[1]->setSkill(i, (int)(100.0 * rand()/RAND_MAX));
  }
  
  pc[2]->setLevel(1); 
  pc[2]->setExp(150);
  pc[2]->setHp();
  pc[2]->setStateMod(Constants::ac_protected, true);
  pc[2]->setStateMod(Constants::magic_protected, true);
  pc[2]->setStateMod(Constants::cursed, true);        
  for(int i = 0; i < Constants::SKILL_COUNT; i++) {
      pc[2]->setSkill(i, (int)(100.0 * rand()/RAND_MAX));
  }
  
  pc[3]->setLevel(1); 
  pc[3]->setExp(400);
  pc[3]->setHp();
  pc[3]->setStateMod(Constants::possessed, true);          
  for(int i = 0; i < Constants::SKILL_COUNT; i++) {
      pc[3]->setSkill(i, (int)(100.0 * rand()/RAND_MAX));
  }

  // add some items
  pc[0]->addInventory(new Item(RpgItem::items[RpgItem::BASTARD_SWORD]));
  pc[0]->addInventory(new Item(RpgItem::items[RpgItem::DAGGER]));
  pc[2]->addInventory(new Item(RpgItem::items[RpgItem::LONG_SWORD]));
  pc[3]->addInventory(new Item(RpgItem::items[RpgItem::GREAT_SWORD]));
  pc[3]->addInventory(new Item(RpgItem::items[RpgItem::BATTLE_AXE]));
  pc[3]->addInventory(new Item(RpgItem::items[RpgItem::THROWING_AXE]));

  return pc;
}

void Creature::monsterInit() {
  // equip starting inventory
}
