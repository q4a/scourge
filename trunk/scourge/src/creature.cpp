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
  this->stateMod = 0;
  this->level = 1;
  this->exp = 0;
  this->hp = 0;
  this->ac = 0;
  this->targetCreature = NULL;
  this->lastTick = 0;
  this->moveRetrycount = 0;
  this->cornerX = this->cornerY = -1;
  this->lastTurn = 0;
  this->damageEffectCounter = 0;
  this->effect = new Effect(scourge->getShapePalette()->getTexture(9));
  this->effectType = Constants::EFFECT_FLAMES;
  this->facingDirection = Constants::MOVE_UP; // good init ?
  
  // Yes, monsters have inventory weight issues too
  inventoryWeight =  0.0f;  
  for(int i = 0; i < inventory_count; i++) {
    inventoryWeight += inventory[i]->getRpgItem()->getWeight();
  }  
  maxInventoryWeight = computeMaxInventoryWeight();
}

Creature::~Creature(){
  delete effect;
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
}

void Creature::stopMoving() {
  bestPathPos = (int)bestPath.size();
  selX = selY = -1;
}

bool Creature::moveToLocator(Map *map, bool single_step) {

  // Don't move when attacking...
  // this is actually wrong, the method should not be called in this
  // case, but the code is simpler this way. (Returning false is 
  // is incorrect.)
  if(((MD2Shape*)getShape())->getAttackEffect()) return false;

  bool moved = false;
  if(selX > -1) {
    // take a step
    if(getMotion() == Constants::MOTION_MOVE_AWAY){    
      moved = gotoPosition(map, cornerX, cornerY, 0, "cornerXY");
    } else {
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
	  if(this == scourge->getPlayer()) {
		scourge->setPartyMotion(Constants::MOTION_MOVE_TOWARDS);
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
    Location location = bestPath.at(bestPathPos);
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
      if(this == scourge->getPlayer() && 
		 creature && creature->character && scourge->getPlayer() != creature) {
		
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

bool Creature::addInventory(Item *item) { 
  if(inventory_count < MAX_INVENTORY_SIZE) {    
        inventory[inventory_count++] = item;
        inventoryWeight += item->getRpgItem()->getWeight(); 
         
        if(item->getRpgItem()->getWeight() + inventoryWeight > 
            maxInventoryWeight)
        {
    	    char msg[80];
            sprintf(msg, "%s is overloaded.", getName());
            scourge->getMap()->addDescription(msg);            
            setStateMod(Constants::overloaded, true);
        }
        return true;
  } 
  else{
    return false;
  }
}

Item *Creature::removeInventory(int index) { 
  Item *item = NULL;
  if(index < inventory_count) {
	// drop item if carrying it
	doff(index);
	// drop from inventory
	item = inventory[index];
	inventoryWeight -= item->getRpgItem()->getWeight();
	if(getStateMod(Constants::overloaded) && inventoryWeight < maxInventoryWeight)
    {
    	    char msg[80];
            sprintf(msg, "%s is no more overloaded.", getName());
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
    char msg[80];
    char buff[80];
    //Item * item = getInventory(index);
    RpgItem * rpgItem = getInventory(index)->getRpgItem();
    int type;
    //float weight;
    
    type = rpgItem->getType();    
    if(type!=RpgItem::FOOD && type!=RpgItem::DRINK && type!=RpgItem::POTION){              
        scourge->getMap()->addDescription("You cannot eat or drink that!");   
        return false;       
    }
    else{
        //weight = item->getRpgItem()->getWeight();
        level = rpgItem->getLevel();
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
            return(computeNewItemWeight(rpgItem));                              
        }
        else if(type == RpgItem::DRINK){
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
            return(computeNewItemWeight(rpgItem));  
            
        }
        else{   
            // It's a potion            
            // Even if not thirsty, character will always drink a potion
            strcpy(buff, rpgItem->getShortDesc());
            buff[0] = tolower(buff[0]);
            setThirst(getThirst() + level);
            sprintf(msg, "%s drinks %s.", getName(), buff);
            scourge->getMap()->addDescription(msg); 
            // TODO : add potion effect
            return(computeNewItemWeight(rpgItem));                                   
        }        
    }
}

bool Creature::computeNewItemWeight(RpgItem * rpgItem){
    float f1;
    int oldCharges;
    
    oldCharges = rpgItem->getCurrentCharges();            
    if(oldCharges <= 1){
        // The object is totally consummed
        return true;    
    }            
    rpgItem->setCurrentCharges(oldCharges - 1);
            
    // Compute initial weight to be able to compute new weight
    // (without increasing error each time)
    
    f1 = rpgItem->getWeight();
    f1 *= (float) rpgItem->getMaxCharges();
    f1 /= (float) oldCharges;
    f1 *= (((float)oldCharges - 1.0f) / (float)rpgItem->getMaxCharges());            
    rpgItem->setWeight(f1);
    return false;      
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
int Creature::getInitiative(Item *weapon) {
  // use the speed skill
  float speed = skills[Constants::SPEED];
  // roll for half the luck
  speed += (skills[Constants::LUCK / 2] * rand()/RAND_MAX);
  // add weapon speed (bare hand attack is the fastest, unless weapon skill is very good)
  if(weapon) {
	speed -= weapon->getRpgItem()->getSpeed();
	if(weapon->getRpgItem()->getSkill() > -1) 
	  speed += skills[weapon->getRpgItem()->getSkill()];
  }
  // at this point a score of 150 is the fastest and 0 is the slowest

  // convert to 0-10 and flip (so 10 is the slowest)
  return (10 - (int)(speed / 15.0f));
}

// roll the die for the toHit number. returns a value between 0(total miss) - 100(best hit)
int Creature::getToHit(Item *weapon) {
  float tohit = skills[Constants::COORDINATION] + skills[Constants::LUCK] / 2;
  if(weapon && weapon->getRpgItem()->getSkill() > -1) {
	tohit += skills[weapon->getRpgItem()->getSkill()];
  } else {
	tohit += skills[Constants::HAND_TO_HAND_COMBAT];
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
					  (skills[Constants::POWER] / 10));
  damage = baseDamage;
  damage += (float)skills[Constants::POWER] / 10.0f;
  
  float skill = (weapon && weapon->getRpgItem()->getSkill() > -1 ?
				 skills[weapon->getRpgItem()->getSkill()] :
				 skills[Constants::HAND_TO_HAND_COMBAT]);
  damage = damage + (damage * ((skill - 50) / 100.0f) );
  return (int)(damage * rand()/RAND_MAX);
}

// take some damage
bool Creature::takeDamage(int damage) {
  // show an effect
  resetDamageEffect();
  setEffectType(Constants::EFFECT_GLOW);
  //setEffectType(Constants::EFFECT_FLAMES);

  hp -= damage;
  return (hp <= 0);
}

/**
   Create a party programmatically until the party editor is made.
 */
Creature **Creature::createHardCodedParty(Scourge *scourge) {
  Creature **pc = (Creature**)malloc(sizeof(Creature*) * 4);

	// FIXME: consider using newCreature here
	// the end of startMission would have to be modified to not delete the party
	// also in scourge, where-ever creatureCount is used to mean all monsters would have
	// to change (maybe that's a good thing too... same logic for party and monsters)
  pc[0] = new Creature(scourge, Character::character_class[Character::assassin], "Alamont");
  pc[1] = new Creature(scourge, Character::character_class[Character::knight], "Barlett");
  pc[2] = new Creature(scourge, Character::character_class[Character::summoner], "Corinus");
  pc[3] = new Creature(scourge, Character::character_class[Character::naturalist], "Dialante");

  pc[0]->setLevel(1); 
  pc[0]->setExp(300);
  pc[0]->setHp();
  pc[0]->setHunger(8);
  pc[0]->setThirst(7); 
  pc[0]->setStateMod(Constants::blessed, true);
  pc[0]->setStateMod(Constants::poisoned, true);
  for(int i = 0; i < Constants::SKILL_COUNT; i++) {
      pc[0]->setSkill(i, (int)(20.0 * rand()/RAND_MAX));
  }
  
  pc[1]->setLevel(1); 
  pc[1]->setExp(200);
  pc[1]->setHp();
  pc[1]->setHunger(10);
  pc[1]->setThirst(9);
  pc[1]->setStateMod(Constants::drunk, true);
  pc[1]->setStateMod(Constants::cursed, true);      
  for(int i = 0; i < Constants::SKILL_COUNT; i++) {
      pc[1]->setSkill(i, (int)(20.0 * rand()/RAND_MAX));
  }
  
  pc[2]->setLevel(1); 
  pc[2]->setExp(150);
  pc[2]->setHp();
  pc[2]->setHunger(3);
  pc[2]->setThirst(2);
  pc[2]->setStateMod(Constants::ac_protected, true);
  pc[2]->setStateMod(Constants::magic_protected, true);
  pc[2]->setStateMod(Constants::cursed, true);        
  for(int i = 0; i < Constants::SKILL_COUNT; i++) {
      pc[2]->setSkill(i, (int)(20.0 * rand()/RAND_MAX));
  }
  
  pc[3]->setLevel(1); 
  pc[3]->setExp(400);
  pc[3]->setHp();
  pc[3]->setHunger(10);
  pc[3]->setThirst(10);
  pc[3]->setStateMod(Constants::possessed, true);          
  for(int i = 0; i < Constants::SKILL_COUNT; i++) {
      pc[3]->setSkill(i, (int)(20.0 * rand()/RAND_MAX));
  }
  
  // Compute the new maxInventoryWeight for each pc, according to its POWER skill
  for(int i = 0; i < 4; i++) {
      pc[i]->setMaxInventoryWeight(pc[i]->computeMaxInventoryWeight());
  }

  // add some items
  pc[0]->addInventory(scourge->newItem(RpgItem::getItemByName("Bastard sword")));
  pc[0]->addInventory(scourge->newItem(RpgItem::getItemByName("Dagger")));
  pc[1]->addInventory(scourge->newItem(RpgItem::getItemByName("Smallbow")));
  pc[1]->addInventory(scourge->newItem(RpgItem::getItemByName("Apple")));
  pc[1]->addInventory(scourge->newItem(RpgItem::getItemByName("Apple")));
  pc[2]->addInventory(scourge->newItem(RpgItem::getItemByName("Long sword")));
  pc[2]->addInventory(scourge->newItem(RpgItem::getItemByName("Wine barrel")));
  pc[2]->addInventory(scourge->newItem(RpgItem::getItemByName("Mutton meat")));
  pc[3]->addInventory(scourge->newItem(RpgItem::getItemByName("Great sword")));
  pc[3]->addInventory(scourge->newItem(RpgItem::getItemByName("Battleaxe")));
  pc[3]->addInventory(scourge->newItem(RpgItem::getItemByName("Throwing axe")));
  

  return pc;
}

void Creature::monsterInit() {
  // equip starting inventory
  for(int i = 0; i < Monster::ITEM_COUNT; i++) {
	if(getMonster()->getStartingItem(i)) {
	  addInventory(scourge->newItem(getMonster()->getStartingItem(i)));
	  equipInventory(inventory_count - 1);
	}
  }
  // set some skills
  for(int i = 0; i < Constants::SKILL_COUNT; i++) {
    setSkill(i, (int)((float)(10 * level) * rand()/RAND_MAX));
  }
  // add some hp
  hp = 4 + (int)((float)(10.0f * level) * rand()/RAND_MAX);
}
