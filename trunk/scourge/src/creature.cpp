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
  this->model_name = character->getModelName();
  this->skin_name = character->getSkinName();
  sprintf(description, "%s the %s", name, character->getName());
  this->speed = 50;
  this->motion = Constants::MOTION_MOVE_TOWARDS;  
  this->armor=0;
  this->bonusArmor=0;
  this->thirst=10;
  this->hunger=10;  
  commonInit();  
}

Creature::Creature(Scourge *scourge, Monster *monster) {
  this->scourge = scourge;
  this->character = NULL;
  this->monster = monster;
  this->name = monster->getType();
  this->model_name = monster->getModelName();
  this->skin_name = monster->getSkinName();
  sprintf(description, "You see %s", monster->getType());
  this->speed = monster->getSpeed();
  this->motion = Constants::MOTION_LOITER;
  this->armor = monster->getBaseArmor();
  this->bonusArmor=0;
  commonInit();
  monsterInit();
}

void Creature::commonInit() {
  this->shape = scourge->getShapePalette()->getCreatureShape(model_name, skin_name);
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
  for(int i = 0; i < Constants::SKILL_COUNT; i++) {
	skillMod[i] = skillBonus[i] = 0;
  }
  this->stateMod = 0;
  this->level = 1;
  this->exp = 0;
  this->hp = 0;
  this->mp = 0;
  this->startingHp = 0;
  this->startingMp = 0;
  this->ac = 0;
  this->targetCreature = NULL;
  this->lastTick = 0;
  this->moveRetrycount = 0;
  this->cornerX = this->cornerY = -1;
  this->lastTurn = 0;
  this->damageEffectCounter = 0;
  this->effectDuration = Constants::DAMAGE_DURATION;
  this->effect = new Effect(scourge->getShapePalette()->getTexture(9));
  this->effectType = Constants::EFFECT_FLAMES;
  this->facingDirection = Constants::MOVE_UP; // good init ?
  this->availableSkillPoints = 0;
  this->minRange = this->maxRange = 0;
  this->failedToMoveWithinRangeAttemptCount = 0;
  this->action = Constants::ACTION_NO_ACTION;
  this->actionItem = NULL;
  this->actionSpell = NULL;
  this->preActionTargetCreature = NULL;
  
  // Yes, monsters have inventory weight issues too
  inventoryWeight =  0.0f;  
  for(int i = 0; i < inventory_count; i++) {
    inventoryWeight += inventory[i]->getRpgItem()->getWeight();
  }  
  this->money = this->level * (int)(10.0f * rand()/RAND_MAX);
  calculateExpOfNextLevel();
}

void Creature::calculateExpOfNextLevel() {
  if(isMonster()) return;
  expOfNextLevel = 0;
  for(int i = 0; i < level; i++) {
	expOfNextLevel += ((i + 1) * character->getLevelProgression());
  }
}

Creature::~Creature(){
  delete effect;
  // do this before deleting the shape
  scourge->getShapePalette()->decrementSkinRefCount(skin_name);
  delete shape;
}

// moving monsters only
bool Creature::move(Uint16 dir, Map *map) {
  if(character) return false;    
  
  // a hack for runaway creatures
  if(!(x > 10 && x < MAP_WIDTH - 10 &&
       y > 10 && y < MAP_DEPTH - 10)) {
    if(monster) cerr << "hack for " << getName() << endl;
    return false;
  }
  
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
  setFacingDirection(dir);
  map->removeCreature(x, y, z);
  Location *loc = map->getBlockingLocation(getShape(), nx, ny, nz);
  Item *item = NULL;
  if(loc) item = loc->item;
  if(!loc || (item && !item->isBlocking())) {

	// pick up item
	if(item) {
	  addInventory(item);
	  char message[100];
	  sprintf(message, "%s picks up %s.", 
			  getName(), 
			  item->getRpgItem()->getName());
	  scourge->getMap()->addDescription(message);
	}

    map->setCreature(nx, ny, nz, this);
    ((MD2Shape*)shape)->setDir(dir);
       
    moveTo(nx, ny, nz);
    setDir(dir);        
    
    return true;
  } else {
    // move back
    map->setCreature(x, y, z, this);    
    return false;
  }
}

bool Creature::follow(Map *map) {
  // find out where the creature should be relative to the formation
  Sint16 px, py, pz;
  getFormationPosition(&px, &py, &pz);
  setSelXY(px, py);
  return true; 
}

void Creature::setSelXY(int x, int y, bool force) { 
  selX = x; 
  selY = y; 
  moveRetrycount = 0; 
  setMotion(Constants::MOTION_MOVE_TOWARDS);   
  if(force) {
	tx = ty = -1;
  }
  // if we're trying to move within a range, try a number of times
  adjustMovementToRange();
}

/**
   Return true only if a range is specified and we're within it.
 */
bool Creature::isInRange() {
  if(maxRange > 0) {
	float d = getDistanceToTarget();
	return (d >= minRange && d < maxRange);
  }
  return false;
}

/**
   Check that we're within range (if range specified).
   If not, try n times, then wait n times before trying again.
*/
void Creature::adjustMovementToRange() {
  if(maxRange <= 0 || !getTargetCreature()) return;
  float d = getDistanceToTarget();
  bool inRange = (d >= minRange && d < maxRange);
  if(inRange) {
	// we're in range: stop moving
	failedToMoveWithinRangeAttemptCount = 0;
	stopMoving();
	return;
  } else if(failedToMoveWithinRangeAttemptCount < MAX_FAILED_MOVE_ATTEMPTS) {
	// we're not in range: move away if too close
	failedToMoveWithinRangeAttemptCount++;

	// if too close, move away
	if(d < minRange) {
	  Sint16 nz;
	  findCorner(&cornerX, &cornerY, &nz);
	  setMotion(Constants::MOTION_MOVE_AWAY);
	}

	return;
  } else if(failedToMoveWithinRangeAttemptCount < MAX_FAILED_MOVE_ATTEMPTS * 2) {
	// we're still not in range: return true to continue attacks and not try to position forever
	failedToMoveWithinRangeAttemptCount++;
	return;
  } else {
	// we're still not in range but continue to try to position creature
	failedToMoveWithinRangeAttemptCount = 0;
	return;
  }
}

void Creature::setTargetCreature(Creature *c) { 
  targetCreature = c; 
  if(!c) {
	setDistanceRange(0, 0);
  }
}

void Creature::stopMoving() {
  bestPathPos = (int)bestPath.size();
  selX = selY = -1;
}

bool Creature::moveToLocator(Map *map) {

  // Don't move when attacking...
  // this is actually wrong, the method should not be called in this
  // case, but the code is simpler this way. (Returning false is 
  // is incorrect.)
  if(((MD2Shape*)getShape())->getAttackEffect()) return false;

  // don't move when in range
  if(isInRange()) return false;

  bool moved = false;
  if(selX > -1) {
	
    // take a step
    if(getMotion() == Constants::MOTION_MOVE_AWAY){    
	  //if(this == scourge->getParty()->getParty(1)) cerr << "Barlett: moving away! attempt=" << failedToMoveWithinRangeAttemptCount << endl;
      moved = gotoPosition(map, cornerX, cornerY, 0, "cornerXY");
    } else {
	  //if(this == scourge->getParty()->getParty(1)) cerr << "Barlett: moving towards! attempt=" << failedToMoveWithinRangeAttemptCount << " min=" << minRange << " max=" << maxRange << endl;
      moved = gotoPosition(map, selX, selY, 0, "selXY");
    }
	// if we've no more steps
	if((int)bestPath.size() <=  bestPathPos && selX > -1) {    

	  // if we're not there yet, and it's possible to stand there, recalc steps
	  if(!(selX == getX() && selY == getY()) &&
		 map->shapeFits(getShape(), selX, selY, -1) &&
		 moveRetrycount < MAX_MOVE_RETRY) {
		
		// don't keep trying forever
		moveRetrycount++;
		tx = ty = -1;
	  } else {
		// do this so the animation switches to "stand"
		stopMoving();
	  }	  
	  
	  // if this is the player, return to regular movement
	  if(this == scourge->getParty()->getPlayer()) {
		scourge->getParty()->setPartyMotion(Constants::MOTION_MOVE_TOWARDS);
	  }
	}
  }
  return moved;
}

bool Creature::anyMovesLeft() {
  return(selX > -1 && (int)bestPath.size() > bestPathPos); 
}

bool Creature::gotoPosition(Map *map, Sint16 px, Sint16 py, Sint16 pz, char *debug) {
  // If the target moved, get the best path to the location
  if(!(tx == px && ty == py)) {
    //    cerr << getName() << " - " << debug << " steps left: " << 
    //      ((int)bestPath.size() - bestPathPos) << " out of " << (int)bestPath.size() << endl;
    tx = px;
    ty = py;
    bestPathPos = 1; // skip 0th position; it's the starting location
    Util::findPath(getX(), getY(), getZ(), px, py, pz, &bestPath, scourge->getMap(), getShape());
  }
  
  if((int)bestPath.size() > bestPathPos) {
    // take a step on the bestPath
    Location location = bestPath[bestPathPos];
    // if we can't step there, someone else has moved there ahead of us
    Uint16 oldDir = dir;
    //dir = next->getDir();
    if(getX() < location.x) dir = Constants::MOVE_RIGHT;
    else if(getX() > location.x) dir = Constants::MOVE_LEFT;
    else if(getY() < location.y) dir = Constants::MOVE_DOWN;
    else if(getY() > location.y) dir = Constants::MOVE_UP;
    setFacingDirection(dir);
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
  
	  /*
		commented out: this doesn't really work... the player will block forever
		if 'creature' can't move

      // if another party member is blocking the player, 
	  // make them move out of the way
	  Creature *creature = position->creature;
      if(this == scourge->getParty()->getPlayer() && 
		 creature && creature->character && scourge->getParty()->getPlayer() != creature) {
		
		creature->moveRetrycount++;
		if(creature->moveRetrycount < MAX_MOVE_RETRY) {
		  Sint16 nz;
		  creature->findCorner(&creature->cornerX, &creature->cornerY, &nz);
		  //	    creature->gotoPosition(map, nx, ny, nz, "corner");
		  creature->setMotion(Constants::MOTION_MOVE_AWAY);
		} else {
		  // do this so the animation switches to "stand"
		  creature->stopMoving();
		}
	  } else {	  
	  */
	  // if we're not at the destination, but it's possible to stand there
	  // try again
	  if(!(selX == getX() && selY == getY()) && 
		 map->shapeFits(getShape(), selX, selY, -1) &&
		 moveRetrycount < MAX_MOVE_RETRY) {
		
		// don't keep trying forever
		moveRetrycount++;
		tx = ty = -1;
	  } else {
		// if we can't get to the destination, stop trying
		// do this so the animation switches to "stand"
		stopMoving();
	  }		
	  //}
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
  if(getX() < scourge->getParty()->getPlayer()->getX() &&
     getY() < scourge->getParty()->getPlayer()->getY()) {
    *px = *py = *pz = 0;
    return;
  }
  if(getX() >= scourge->getParty()->getPlayer()->getX() &&
     getY() < scourge->getParty()->getPlayer()->getY()) {
    *px = MAP_WIDTH;
    *py = *pz = 0;
    return;
  }
  if(getX() < scourge->getParty()->getPlayer()->getX() &&
     getY() >= scourge->getParty()->getPlayer()->getY()) {
    *px = *pz = 0;
    *py = MAP_DEPTH;
    return;
  }
  if(getX() >= scourge->getParty()->getPlayer()->getX() &&
     getY() >= scourge->getParty()->getPlayer()->getY()) {
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

bool Creature::addInventory(Item *item) { 
  if(inventory_count < MAX_INVENTORY_SIZE) {    
	inventory[inventory_count++] = item;
	inventoryWeight += item->getRpgItem()->getWeight(); 
	
	if(item->getRpgItem()->getWeight() + inventoryWeight > 
	   getMaxInventoryWeight()) {
	  char msg[80];
	  sprintf(msg, "%s is overloaded.", getName());
	  scourge->getMap()->addDescription(msg);            
	  setStateMod(Constants::overloaded, true);
	}

	// check if the mission is over
	if(!isMonster() && 
	   scourge->getCurrentMission() &&
	   scourge->getCurrentMission()->itemFound(item->getRpgItem())) {
	  scourge->missionCompleted();
	}

	return true;
  } else{
    return false;
  }
}

int Creature::findInInventory(Item *item) {
  for(int i = 0; i < inventory_count; i++) {
	Item *invItem = inventory[i];
	if(item == invItem) return i;
  }
  return -1;
}

Item *Creature::removeInventory(int index) { 
  Item *item = NULL;
  if(index < inventory_count) {
	// drop item if carrying it
	doff(index);
	// drop from inventory
	item = inventory[index];
	inventoryWeight -= item->getRpgItem()->getWeight();
	if(getStateMod(Constants::overloaded) && inventoryWeight < getMaxInventoryWeight())
    {
    	    char msg[80];
            sprintf(msg, "%s is not overloaded anymore.", getName());
            scourge->getMap()->addDescription(msg);            
            setStateMod(Constants::overloaded, false);
    }
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
	recalcAggregateValues();
  }
  return item;
}

bool Creature::eatDrink(int index){
  return eatDrink(getInventory(index));
}

bool Creature::eatDrink(Item *item) {
  char msg[500];
  char buff[200];
  RpgItem * rpgItem = item->getRpgItem();
  
  int type = rpgItem->getType();    
  //weight = item->getRpgItem()->getWeight();
  int level = rpgItem->getLevel();
  if(type == RpgItem::FOOD){
	if(getHunger() == 10){                
	  sprintf(msg, "%s is not hungry at the moment.", getName()); 
	  scourge->getMap()->addDescription(msg); 
	  return false;
	}            
    
	// TODO : the quality member of rpgItem should indicate if the
	// food is totally healthy or roten or partially roten etc...
	// We eat the item and it gives us "level" hunger points back
	setHunger(getHunger() + level);            
	strcpy(buff, rpgItem->getShortDesc());
	buff[0] = tolower(buff[0]);
	sprintf(msg, "%s eats %s.", getName(), buff);
	scourge->getMap()->addDescription(msg);
	bool b = item->decrementCharges();
	if(b) {
	  sprintf(msg, "%s is used up.", rpgItem->getName());
	  scourge->getMap()->addDescription(msg);
	}
	return b;
  } else if(type == RpgItem::DRINK){
	if(getThirst() == 10){                
	  sprintf(msg, "%s is not thirsty at the moment.", getName()); 
	  scourge->getMap()->addDescription(msg); 
	  return false;
	}
	setThirst(getThirst() + level);
	strcpy(buff, rpgItem->getShortDesc());
	buff[0] = tolower(buff[0]);
	sprintf(msg, "%s drinks %s.", getName(), buff);
	scourge->getMap()->addDescription(msg); 
	// TODO : according to the alcool rate set drunk state or not            
	bool b = item->decrementCharges();
	if(b) {
	  sprintf(msg, "%s is used up.", rpgItem->getName());
	  scourge->getMap()->addDescription(msg);
	}
	return b;
  } else if(type == RpgItem::POTION) {
	// It's a potion            
	// Even if not thirsty, character will always drink a potion
	strcpy(buff, rpgItem->getShortDesc());
	buff[0] = tolower(buff[0]);
	setThirst(getThirst() + level);
	sprintf(msg, "%s drinks from %s.", getName(), buff);
	scourge->getMap()->addDescription(msg); 
	usePotion(item);
	bool b = item->decrementCharges();
	if(b) {
	  sprintf(msg, "%s is used up.", rpgItem->getName());
	  scourge->getMap()->addDescription(msg);
	}
	return b;
  } else {
	scourge->getMap()->addDescription("You cannot eat or drink that!", 1, 0.2f, 0.2f);
	return false;
  }
}

void Creature::usePotion(Item *item) {
  // nothing to do?
  if(item->getRpgItem()->getPotionSkill() == -1) return; 

  int n;
  char msg[255];

  int skill = item->getRpgItem()->getPotionSkill();
  if(skill < 0) {
	switch(-skill - 2) {
	case Constants::HP:
	  n = item->getRpgItem()->getAction();
	  if(n + getHp() > getMaxHp()) 
		n = getMaxHp() - getHp();
	  setHp(getHp() + n);
	  sprintf(msg, "%s heals %d points.", getName(), n);
	  scourge->getMap()->addDescription(msg, 0.2f, 1, 1);
	  startEffect(Constants::EFFECT_SWIRL, (Constants::DAMAGE_DURATION * 4));
	  return;
	case Constants::MP:
	  n = item->getRpgItem()->getAction();
	  if(n + getMp() > getMaxMp()) 
		n = getMaxMp() - getMp();
	  setMp(getMp() + n);
	  sprintf(msg, "%s receives %d magic points.", getName(), n);
	  scourge->getMap()->addDescription(msg, 0.2f, 1, 1);
	  startEffect(Constants::EFFECT_SWIRL, (Constants::DAMAGE_DURATION * 4));
	  return;
	case Constants::AC:
	  {
		bonusArmor += item->getRpgItem()->getAction();
		recalcAggregateValues();
		sprintf(msg, "%s feels impervious to damage!", getName());
		scourge->getMap()->addDescription(msg, 0.2f, 1, 1);
		startEffect(Constants::EFFECT_SWIRL, (Constants::DAMAGE_DURATION * 4));
		
		// add calendar event to remove armor bonus
		// (format : sec, min, hours, days, months, years)
		Date d(0, item->getRpgItem()->getPotionTime(), 0, 0, 0, 0); 
		Event *e = 
		  new PotionExpirationEvent(scourge->getParty()->getCalendar()->getCurrentDate(), 
									d, this, item->getRpgItem(), scourge, 1);
		scourge->getParty()->getCalendar()->scheduleEvent((Event*)e);   // It's important to cast!!		
	  }
	  return;
	default:
	  cerr << "Implement me! (other potion skill boost)" << endl;
	  return;
	}
  } else {
	skillBonus[skill] += item->getRpgItem()->getAction();
	//	recalcAggregateValues();
	sprintf(msg, "%s feels at peace.", getName());
	scourge->getMap()->addDescription(msg, 0.2f, 1, 1);
	startEffect(Constants::EFFECT_SWIRL, (Constants::DAMAGE_DURATION * 4));
	
	// add calendar event to remove armor bonus
	// (format : sec, min, hours, days, months, years)
	Date d(0, item->getRpgItem()->getPotionTime(), 0, 0, 0, 0); 
	Event *e = 
	  new PotionExpirationEvent(scourge->getParty()->getCalendar()->getCurrentDate(), 
								d, this, item->getRpgItem(), scourge, 1);
	scourge->getParty()->getCalendar()->scheduleEvent((Event*)e);   // It's important to cast!!
  }
}

void Creature::setAction(int action, 
						 Item *item, 
						 Spell *spell) {
  this->action = action;
  this->actionItem = item;
  this->actionSpell = spell;
  preActionTargetCreature = getTargetCreature();  
  // zero the clock
  setLastTurn(0);

  char msg[80];
  switch(action) {
  case Constants::ACTION_EAT_DRINK:
	sprintf(msg, "%s will consume %s.", getName(), item->getRpgItem()->getName());
	break;
  case Constants::ACTION_CAST_SPELL:
	sprintf(msg, "%s will cast %s.", getName(), spell->getName());
	break;
  case Constants::ACTION_NO_ACTION:
	// no-op
	preActionTargetCreature = NULL;
	sprintf(msg, "");
	break;
  default:
	cerr << "*** Error: unknown action " << action << endl;
	return;
  }
  if(strlen(msg)) scourge->getMap()->addDescription(msg, 1, 1, 0.5f);
}

void Creature::equipInventory(int index) {
	// doff
	if(doff(index))	return;
	// don
	// FIXME: take into account: two-handed weapons, race/class modifiers, min skill req-s., etc.
	Item *item = getInventory(index);
	for(int i = 0; i < Character::INVENTORY_COUNT; i++) {
		// if the slot is empty and the item can be worn here
		if(item->getRpgItem()->getEquip() & ( 1 << i ) && 
			 equipped[i] == MAX_INVENTORY_SIZE) {
			equipped[i] = index;
			recalcAggregateValues();
			return;
		}
	}
}

int Creature::doff(int index) {
	// doff
	for(int i = 0; i < Character::INVENTORY_COUNT; i++) {
		if(equipped[i] == index) {
			equipped[i] = MAX_INVENTORY_SIZE;
			recalcAggregateValues();
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
		if(inventory[i] == item || 
			 (inventory[i]->getRpgItem()->getType() == RpgItem::CONTAINER &&
				inventory[i]->isContainedItem(item))) 
			return true;
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

// calculate the aggregate values based on equipped items
void Creature::recalcAggregateValues() {
  // calculate the armor (0-100, 100-total protection)
  armor = (monster ? monster->getBaseArmor() : 0);
  for(int i = 0; i < Character::INVENTORY_COUNT; i++) {
	if(equipped[i] != MAX_INVENTORY_SIZE) {
	  Item *item = inventory[equipped[i]];
	  if(item->getRpgItem()->getType() == RpgItem::ARMOR) {
		armor += item->getRpgItem()->getAction();
	  }
	}
  }
  armor += bonusArmor;
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

// return the initiative for a battle round (0-10), the lower the faster the attack
// the method can return negative numbers if the weapon skill is very high (-10 to 10)
int Creature::getInitiative(Item *weapon, Spell *spell) {
  // use the speed skill
  float speed = getSkill(Constants::SPEED);
  // roll for half the luck
  speed += (getSkill(Constants::LUCK / 2) * rand()/RAND_MAX);
  if(spell) {
	speed -= spell->getSpeed();
	speed += getSkill(spell->getSchool()->getSkill());
  } else if(weapon) {
	// add weapon speed (bare hand attack is the fastest, unless weapon skill is very good)
	speed -= weapon->getRpgItem()->getSpeed();
	if(weapon->getRpgItem()->getSkill() > -1) 
	  speed += getSkill(weapon->getRpgItem()->getSkill());
  }
  // at this point a score of 150 is the fastest and 0 is the slowest

  // convert to 0-10 and flip (so 10 is the slowest)
  return (10 - (int)(speed / 15.0f));
}

// return number of projectiles that can be launched simultaniously
// it is a function of speed, coordination and weapon skill
// this method returns a number from 1-10
int Creature::getMaxProjectileCount(Item *item) {
  int n = (int)((double)(getSkill(Constants::SPEED) + 
						 getSkill(Constants::COORDINATION) + 
						 getSkill(item->getRpgItem()->getSkill())) / 30.0f);
  if(n <= 0) n = 1;
  return n;
}

// roll the die for the toHit number. returns a value between 0(total miss) - 100(best hit)
int Creature::getToHit(Item *weapon) {
  float tohit = getSkill(Constants::COORDINATION) + getSkill(Constants::LUCK) / 2;
  if(weapon && weapon->getRpgItem()->getSkill() > -1) {
	tohit += getSkill(weapon->getRpgItem()->getSkill());
  } else {
	tohit += getSkill(Constants::HAND_TO_HAND_COMBAT);
  }
  // so far the max score is 250
  
  // tohit = 75% of tohit + (rand(25% of tohit))
  float score = (0.75f * tohit) + ((0.25f * tohit) * rand()/RAND_MAX);
  // convert to 0-100 value
  return (int)(score / 2.5f);
}

// return the damage as:
// rand(weapon + power + (skill - 50 % weapon))
int Creature::getDamage(Item *weapon) {
  float damage = 0.0f;
  float baseDamage = (weapon ? weapon->getRpgItem()->getAction() : 
					  (getSkill(Constants::POWER) / 10));
  damage = baseDamage;
  damage += (float)getSkill(Constants::POWER) / 10.0f;
  
  float skill = (weapon && weapon->getRpgItem()->getSkill() > -1 ?
				 getSkill(weapon->getRpgItem()->getSkill()) :
				 getSkill(Constants::HAND_TO_HAND_COMBAT));
  damage = damage + (damage * ((skill - 50) / 100.0f) );
  return (int)(damage * rand()/RAND_MAX);
}

/**
 take some damage
*/
bool Creature::takeDamage(int damage, int effect_type) {
  startEffect(effect_type);
  hp -= damage;
  return (hp <= 0);
}

void Creature::startEffect(int effect_type, int duration) {
  // show an effect
  if(isEffectOn()) return;
  effect->deleteParticles();
  resetDamageEffect();
  setEffectType(effect_type);
  effectDuration = duration;
  //setEffectType(Constants::EFFECT_GLOW);
  //setEffectType(Constants::EFFECT_FLAMES);
}

/**
   Get the total value of armor worn and roll for the skill of each piece.
   Monsters' base armor is not rolled (ie. they're experts in using their natural armor.)
 */
int Creature::getSkillModifiedArmor() {
  // calculate the armor (0-100, 100-total protection)
  int armor = (monster ? monster->getBaseArmor() : 0);
  armor += bonusArmor;
  for(int i = 0; i < Character::INVENTORY_COUNT; i++) {
	if(equipped[i] != MAX_INVENTORY_SIZE) {
	  Item *item = inventory[equipped[i]];
	  if(item->getRpgItem()->getType() == RpgItem::ARMOR) {
		int skill_index = (item->getRpgItem()->getSkill() > -1 ? 
						   item->getRpgItem()->getSkill() : 
						   Constants::HAND_DEFEND);
		float skill = (float)getSkill(skill_index);
		int value = item->getRpgItem()->getAction();
		
		// add (value + ((skill-50)% of value)) to armor
		armor += value + (int)( (float)value * ((skill - 50.0f) / 100.0f) );
	  }
	}
  }
  return armor;
}

// add exp after killing a creature
// only called for characters
int Creature::addExperience(Creature *creature_killed) {
  int n = creature_killed->level - getLevel();
  if( n < 1 ) n = 1;
  float m = (float)(creature_killed->getMonster()->getHp()) / 2.0f;
  int delta = n * 10 * (int)((m * rand()/RAND_MAX) + m);
  return addExperience(delta);
}

// add n exp points
// only called for characters
int Creature::addExperience(int delta) {
  exp += delta;

  // level up? (mark as state, with graphic over character)
  if(exp >= expOfNextLevel && !getStateMod(Constants::leveled)) {
	setStateMod(Constants::leveled, true);
	availableSkillPoints = character->getSkillBonus();
  }

  return delta;
}

// add money after a creature is killed
int Creature::addMoney(Creature *creature_killed) {
  int n = creature_killed->level - getLevel();
  if( n < 1 ) n = 1;
  // fixme: use creature_killed->getMonster()->getMoney() instead of 100.0f
  long delta = (long)n * (int)(50.0f * rand()/RAND_MAX);
  money += delta;
  return money;
}

void Creature::monsterInit() {
  // equip starting inventory
  for(int i = 0; i < getMonster()->getStartingItemCount(); i++) {
	addInventory(scourge->newItem(getMonster()->getStartingItem(i)));
	equipInventory(inventory_count - 1);
  }
  // set some skills
  for(int i = 0; i < Constants::SKILL_COUNT; i++) {
    setSkill(i, (int)((float)(10 * level) * rand()/RAND_MAX));
  }
  // add some hp
  startingHp = hp = 4 + (int)((float)(10.0f * level) * rand()/RAND_MAX);
  startingMp = mp = 4 + (int)((float)(4.0f * level) * rand()/RAND_MAX);
}

// only for characters: leveling up
bool Creature::incSkillMod(int index) {
  if(!availableSkillPoints || 
	 getSkill(index) + skillMod[index] >= character->getMaxSkillLevel(index)) return false;
  availableSkillPoints--;
  skillMod[index]++;
  return true;
}

bool Creature::decSkillMod(int index) {
  if(!skillMod[index]) return false;
  availableSkillPoints++;
  skillMod[index]--;
  return true;
}

void Creature::applySkillMod() {
  for(int i = 0; i < Constants::SKILL_COUNT; i++) {
	setSkill(i, getSkill(i) + skillMod[i]);
	skillMod[i] = 0;
  }
  level++;
  hp += character->getStartingHp();
  mp += character->getStartingMp();
  setStateMod(Constants::leveled, false);
  availableSkillPoints = 0;
  calculateExpOfNextLevel();
}

int Creature::getMaxHp() {
  if(isMonster()) {
	return monster->getHp(); // FIXME: incorrect, see monsterInit()
  } else {
	return (character->getStartingHp() * getLevel());
  }
}

int Creature::getMaxMp() {
  if(isMonster()) {
	return monster->getMp(); // FIXME: incorrect, see monsterInit()
  } else {
	return (character->getStartingMp() * getLevel());
  }
}

float Creature::getTargetAngle() {
  if(!targetCreature) return -1.0f;
  return Util::getAngle(getX(), getY(), getShape()->getWidth(), getShape()->getDepth(),
						getTargetCreature()->getX(), getTargetCreature()->getY(), 
						getTargetCreature()->getShape()->getWidth(), 
						getTargetCreature()->getShape()->getHeight());
}

// FIXME: O(n) but there aren't that many spells...
bool Creature::isSpellMemorized(Spell *spell) {
  for(int i = 0; i < (int)spells.size(); i++) {
	if(spells[i] == spell) return true;
  }
  return false;
}

bool Creature::hasTarget() {
  return (getTargetCreature() != NULL);
}

bool Creature::isTargetValid() {
  return (!getTargetCreature()->getStateMod(Constants::dead));
}

void Creature::cancelTarget() {
  setTargetCreature(NULL);
  setDistanceRange(0, 0);  
  if(preActionTargetCreature) setTargetCreature(preActionTargetCreature);
  preActionTargetCreature = NULL;
  setAction(Constants::ACTION_NO_ACTION);
}

void Creature::followTarget() {
  setSelXY(getTargetCreature()->getX(),
		   getTargetCreature()->getY(),
		   true);
}

void Creature::makeTargetRetaliate() {
  char message[200];

  // the target creature gets really upset...
  // this is also an optimization for fps
  if(getTargetCreature()->isMonster() && 
	 !getTargetCreature()->getTargetCreature()) {
	// try to attack the nearest player
	Creature *p = scourge->getParty()->getClosestPlayer(getTargetCreature()->getX(), 
														getTargetCreature()->getY(), 
														getTargetCreature()->getShape()->getWidth(),
														getTargetCreature()->getShape()->getDepth(),
														20);
	// if that's not possible, go for the attacker
	if(!p) p = this;
	getTargetCreature()->setMotion(Constants::MOTION_MOVE_TOWARDS);
	getTargetCreature()->setTargetCreature(p);

	sprintf(message, "...%s is enraged and attacks %s", 
			getTargetCreature()->getName(), 
			p->getName());
	scourge->getMap()->addDescription(message);	
  }
}

float Creature::getDistanceToTarget() {
  if(!getTargetCreature()) return 0.0f;
  return Constants::distance(getX(),  getY(), 
							 getShape()->getWidth(), getShape()->getDepth(),
							 getTargetCreature()->getX(), getTargetCreature()->getY(),
							 getTargetCreature()->getShape()->getWidth(), 
							 getTargetCreature()->getShape()->getDepth());
}
