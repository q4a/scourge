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

#define MOVE_DELAY 7

/**
 * Formations are defined by 4 set of coordinates in 2d space.
 * These starting positions assume dir=Constants::MOVE_UP
 */
static const Sint16 layout[][4][2] = {
  { {0, 0}, {-1, 1}, {1, 1}, {0, 2}},  // DIAMOND_FORMATION
  { {0, 0}, {-1, 1}, {0, 1}, {-1, 2}},  // STAGGERED_FORMATION
  { {0, 0}, {1, 0}, {1, 1}, {0, 1}},   // SQUARE_FORMATION
  { {0, 0}, {0, 1}, {0, 2}, {0, 3}},   // ROW_FORMATION
  { {0, 0}, {-2, 2}, {0, 2}, {2, 2}},  // SCOUT_FORMATION
  { {0, 0}, {-1, 1}, {1, 1}, {0, 3}}   // CROSS_FORMATION
};

Creature::Creature(Session *session, Character *character, char *name) {
  this->session = session;
  this->character = character;
  this->monster = NULL;
  this->name = name;
  this->model_name = character->getModelName();
  this->skin_name = character->getSkinName();
  sprintf(description, "%s the %s", name, character->getName());
  this->speed = 5; // start neutral speed
  this->motion = Constants::MOTION_MOVE_TOWARDS;  
  this->armor=0;
  this->bonusArmor=0;
  this->thirst=10;
  this->hunger=10;  
  this->shape = session->getShapePalette()->getCreatureShape(model_name, skin_name);
  commonInit();  
}

Creature::Creature(Session *session, Monster *monster, GLShape *shape) {
  this->session = session;
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
  this->shape = shape;
  commonInit();
  monsterInit();
}

void Creature::commonInit() {
  this->lastMove = 0;
  this->moveCount = 0;
  this->x = this->y = this->z = 0;
  this->dir = Constants::MOVE_UP;
  this->next = NULL;
  this->formation = DIAMOND_FORMATION;
  this->index = 0;
  this->tx = this->ty = -1;  
  this->selX = this->selY = -1;
  this->bestPathPos = 0;
  this->inventory_count = 0;
  for(int i = 0; i < Constants::INVENTORY_COUNT; i++) {
    equipped[i] = MAX_INVENTORY_SIZE;
  }
  for(int i = 0; i < Constants::SKILL_COUNT; i++) {
    skillMod[i] = skillBonus[i] = 0;
  }
  this->stateMod = 0;
  this->protStateMod = 0;
  this->level = 1;
  this->exp = 0;
  this->hp = 0;
  this->mp = 0;
  this->startingHp = 0;
  this->startingMp = 0;
  this->ac = 0;
  this->targetCreature = NULL;
  this->targetX = this->targetY = this->targetZ = 0;
  this->targetItem = NULL;
  this->lastTick = 0;
  this->moveRetrycount = 0;
  this->cornerX = this->cornerY = -1;
  this->lastTurn = 0;
  this->damageEffectCounter = 0;
  this->effectDuration = Constants::DAMAGE_DURATION;
  this->effect = new Effect(session, session->getShapePalette(), shape);
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
  this->battle = new Battle(session, this);
  lastEnchantDate.setDate(-1, -1, -1, -1, -1, -1);
}

Creature::~Creature(){
  if(this->character) free( name );
  session->getGameAdapter()->removeBattle(battle);
  delete battle;
  delete effect;
  // do this before deleting the shape
  session->getShapePalette()->decrementSkinRefCount(skin_name);
  delete shape;
}

CreatureInfo *Creature::save() {
  CreatureInfo *info = (CreatureInfo*)malloc(sizeof(CreatureInfo));
  info->version = PERSIST_VERSION;
  strncpy((char*)info->name, getName(), 254);
  info->name[254] = 0;
  if(isMonster()) {
    strcpy((char*)info->character_name, "");
    strcpy((char*)info->monster_name, monster->getType());
  } else {
    strcpy((char*)info->character_name, character->getName());
    strcpy((char*)info->monster_name, "");    
  }
  info->hp = hp;
  info->mp = mp;
  info->exp = exp;
  info->level = level;
  info->money = money;
  info->stateMod = stateMod;
  info->protStateMod = protStateMod;
  info->x = toint(x);
  info->y = toint(y);
  info->z = toint(z);
  info->dir = dir;
  info->speed = speed;
  info->motion = motion;
  info->armor = armor;
  info->bonusArmor = bonusArmor;
  info->bonusArmor = 0;
  info->thirst = thirst;
  info->hunger = hunger;
  info->availableSkillPoints = availableSkillPoints;
  for(int i = 0; i < Constants::SKILL_COUNT; i++) {
    info->skills[i] = skills[i];
    info->skillMod[i] = skillMod[i];
    info->skillBonus[i] = skillBonus[i];
  }

  // inventory
  info->inventory_count = inventory_count;
  for(int i = 0; i < inventory_count; i++) {
    info->inventory[i] = inventory[i]->save();
    //info->containedItems[i] = inventory[i]->saveContainedItems();
  }
  for(int i = 0; i < Constants::INVENTORY_COUNT; i++) {
    info->equipped[i] = equipped[i];
  }

  // spells
  int count = 0;
  for(int i = 0; i < MagicSchool::getMagicSchoolCount(); i++) {
    MagicSchool *school = MagicSchool::getMagicSchool(i);
    for(int t = 0; t < school->getSpellCount(); t++) {
      Spell *spell = school->getSpell(t);
      if(isSpellMemorized(spell)) {
        strcpy((char*)info->spell_name[count++], spell->getName());
      }
    }
  }
  info->spell_count = count;

  return info;
}

Creature *Creature::load(Session *session, CreatureInfo *info) {
  Creature *creature = NULL;
  // FIXME: figure out how/when to call session->newCreature, vs. new Creature()
  // Maybe all creatures should be constructed via session->newCreature() but
  // not destroyed in session->deleteCreaturesAndItems()?
  if(!strlen((char*)info->character_name)) {
    cerr << "FIXME: Creature::load(): Loading monster: maybe call session->newCreature?" << endl;
    ((Creature*)NULL)->getName(); // cause an error
    /*
    creature = new Creature(session, 
                            Monster::getMonsterByName((char*)info->monster_name),
                            strdup((char*)info->name));
    */                            
  } else {
    // for now it's ok to call new Creature() for characters. This will change once we save NPC-s.
    creature = new Creature(session, 
                            Character::getCharacterByName((char*)info->character_name), 
                            strdup((char*)info->name));
  }
  cerr << "*** LOAD: creature=" << info->name << endl;
  creature->setHp( info->hp );
  creature->setMp( info->mp );
  creature->setExp( info->exp );
  creature->setLevel( info->level );
  creature->setMoney( info->money );
  creature->moveTo( info->x, info->y, info->z );
  creature->setDir( info->dir );
  //creature->setSpeed( info->speed );
  creature->setMotion( info->motion );
  //creature->setArmor( info->armor );
  
  // info->bonusArmor: can't be used until calendar is also persisted
  //creature->setBonusArmor( info->bonusArmor );

  creature->setThirst( info->thirst );
  creature->setHunger( info->hunger );
  creature->setAvailableSkillPoints( info->availableSkillPoints );
  for(int i = 0; i < Constants::SKILL_COUNT; i++) {
    creature->setSkill( i, info->skills[i] );
    creature->skillMod[i] = info->skillMod[i];
    // info->skillBonus: can't be used until calendar is also persisted
    //creature->setSkillBonus( i, info->skillBonus[i] );
  }
  
  // stateMod and protStateMod not useful until calendar is also persisted
  //creature->stateMod = info->stateMod;
  //creature->protStateMod = info->protStateMod;
  // these two don't req. events:
  if(info->stateMod & (1 << Constants::dead)) creature->setStateMod(Constants::dead, true);
  if(info->stateMod & (1 << Constants::leveled)) creature->setStateMod(Constants::leveled, true);

  // inventory
  //creature->inventory_count = info->inventory_count;
  for(int i = 0; i < (int)info->inventory_count; i++) {
    Item *item = Item::load( session, info->inventory[i] );
    if(item) creature->addInventory( item, true );
  }
  for(int i = 0; i < Constants::INVENTORY_COUNT; i++) {
    if(info->equipped[i] < MAX_INVENTORY_SIZE) { 
      creature->equipInventory(info->equipped[i]);
    } else {
      creature->equipped[i] = info->equipped[i];
    }
  }

  // spells
  for(int i = 0; i < (int)info->spell_count; i++) {
    Spell *spell = Spell::getSpellByName( (char*)info->spell_name[i] );
    creature->addSpell( spell );
  }

  return creature;
}

void Creature::calculateExpOfNextLevel() {
  if(isMonster()) return;
  expOfNextLevel = 0;
  for(int i = 0; i < level; i++) {
    expOfNextLevel += ((i + 1) * character->getLevelProgression());
  }
}

void Creature::switchDirection(bool force) {
  int n = (int)(10.0f * rand()/RAND_MAX);
  if(n == 0 || force) {
    int dir = (int)(4.0f * rand()/RAND_MAX);
    switch(dir) {
    case 0: setDir(Constants::MOVE_UP); break;
    case 1: setDir(Constants::MOVE_DOWN); break;
    case 2: setDir(Constants::MOVE_LEFT); break;
    case 3: setDir(Constants::MOVE_RIGHT); break;
    }
  }
}

// moving monsters only
bool Creature::move(Uint16 dir, Map *map) {
  if(character) return false;

  //Uint32 t = SDL_GetTicks();
  //if(t - lastMove < (Uint32)(getSpeed() * MOVE_DELAY * (session->getUserConfiguration()->getGameSpeedLevel() + 1))) return true;
  //if(t - lastMove < (Uint32)(getSpeed() * MOVE_DELAY)) return true;
  //cerr << "*** move(): creature=" << getName() << 
  //  " speed=" << (t - lastMove) << 
  //  " vs. " << (getSpeed() * MOVE_DELAY * (session->getUserConfiguration()->getGameSpeedLevel() + 1)) << endl;
  //lastMove = t;

  switchDirection(false);

  // a hack for runaway creatures
  if(!(x > 10 && x < MAP_WIDTH - 10 &&
       y > 10 && y < MAP_DEPTH - 10)) {
    if(monster) cerr << "hack for " << getName() << endl;
    switchDirection(true);
    return false;
  }

  GLfloat nx = x;
  GLfloat ny = y;
  GLfloat nz = z;
  GLfloat step = 1.0f / ( session->getGameAdapter()->getFps() / ( 10.0f ) );
  step *= ( 1.0f - ((GLfloat)(getSpeed()) / 10.0f ) );
//  step /= ((GLfloat)(session->getUserConfiguration()->getGameSpeedLevel() + 1) * 0.25f);
  switch(dir) {
  case Constants::MOVE_UP:    
    ny = y - step;
    break;
  case Constants::MOVE_DOWN:    
    ny = y + step;
    break;
  case Constants::MOVE_LEFT:
    nx = x - step;
    break;
  case Constants::MOVE_RIGHT:    
    nx = x + step;
    break;
  }
  setFacingDirection(dir);
  

  if(!map->moveCreature(toint(x), toint(y), toint(z), 
                        toint(nx), toint(ny), toint(nz), this)) {
    ((MD2Shape*)shape)->setDir(dir);
    moveTo(nx, ny, nz);
    setDir(dir);        
    return true;
  } else {
    switchDirection(true);
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
  
  // play command sound
  if(x > -1 && 
     session->getParty()->getPlayer() == this && 
     0 == (int)((float)(session->getUserConfiguration()->getSoundFreq()) * rand()/RAND_MAX) &&
     !getStateMod(Constants::dead)) {
    session->playSound(getCharacter()->getRandomSound(Constants::SOUND_TYPE_COMMAND));
  }
}

/**
   Return true only if a range is specified and we're within it.
 */
/*
bool Creature::isInRange() {
  if(maxRange > 0) {
    float d = getDistanceToTarget();
    //cerr << "\tisInRange: dist=" << d << " min=" << minRange << " max=" << maxRange << endl;
    return(d >= minRange && d < maxRange);
  }
  return false;
}
 */

/**
   Check that we're within range (if range specified).
   If not, try n times, then wait n times before trying again.
*/
void Creature::adjustMovementToRange() {
  if(maxRange <= 0 || !hasTarget()) return;
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

bool Creature::moveToLocator(Map *map) {
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
      // if this is the player, return to regular movement
      if(this == session->getParty()->getPlayer()) {
        session->getParty()->setPartyMotion(Constants::MOTION_MOVE_TOWARDS);
      }
    }
  }
  return moved;
}

bool Creature::gotoPosition(Map *map, Sint16 px, Sint16 py, Sint16 pz, char *debug) {

  // creature speed
  //Uint32 t = SDL_GetTicks();
  //if(t - lastMove < (Uint32)(getSpeed() * MOVE_DELAY * (session->getUserConfiguration()->getGameSpeedLevel() + 1))) return true;
  
  
//  if(t - lastMove < (Uint32)(session->getUserConfiguration()->getGameSpeedLevel() * MOVE_DELAY)) return true;
//  lastMove = t;

  // If the target moved, get the best path to the location
  if(!(tx == px && ty == py)) {
    tx = px;
    ty = py;
    bestPathPos = 1; // skip 0th position; it's the starting location
    Util::findPath(toint(getX()), toint(getY()), toint(getZ()), 
                   px, py, pz, &bestPath, session->getMap(), getShape());
  }

  if((int)bestPath.size() > bestPathPos && 
     ((MD2Shape*)getShape())->getCurrentAnimation() == MD2_RUN ) {

    // take a step on the bestPath
    Location location = bestPath[bestPathPos];

    GLfloat newX = getX();
    GLfloat newY = getY();
    GLfloat step = 1.0f / ( session->getGameAdapter()->getFps() / ( 10.0f ) ); 
    step *= ( 1.0f - ((GLfloat)(getSpeed()) / 10.0f ));
    //step /= ((GLfloat)(session->getUserConfiguration()->getGameSpeedLevel() + 1) * 0.25f);
    float lx = (float)(location.x);
    float ly = (float)(location.y);
    float mx = lx - getX();
    float my = ly - getY();
    if( abs(my) > step ) {
      if( my > 0 ) {
        newY += step;
      } else {
        newY -= step;
      }
    }
    if( abs(mx) > step ) {
      if( mx > 0 ) {
        newX += step;
      } else {
        newX -= step;
      }
    }

    //if( !strcmp(getName(), "Alamont") ) 
//      cerr << "x=" << x << "," << y << " bestPathPos=" << bestPathPos << " location=" << location.x << "," << location.y << endl;

    Location *position = map->moveCreature(toint(getX()), toint(getY()), toint(getZ()),
                                           toint(newX), toint(newY), toint(getZ()),
                                           this);


    /**
     * Sending newX,newY as new coordinates could be a wall.
     * location.x,location.y are the only known non-wall coordinates.
     * This is what's causing the creatures to "hang" not
     * want to move around obsticles.
     * To fix this, we need to test for walls and undo one of the
     * directional steps (newX change or newY change) and try again.
     * 
     * Situation:
     * (X-wall, s-start, e-end
     * 
     * XXXXXXXs
     *       e
     * 
     * The path using toint(float-s) would go through the wall.
     */
    if(position && ( newX != getX() && newY != getY() )) {
      position = map->moveCreature(toint(getX()), toint(getY()), toint(getZ()),
                                   toint(getX()), toint(newY), toint(getZ()),
                                   this);
      if( !position ) {
        newX = getX();
      } else {
        position = map->moveCreature(toint(getX()), toint(getY()), toint(getZ()),
                                     toint(newX), toint(getY()), toint(getZ()),
                                     this);
        if( !position ) {
          newY = getY();
        }
      }
    }


    if(!position) {
      angle = Util::getAngle( newX, newY, 1, 1,
                              getX(), getY(), 1, 1 );
      moveTo( newX, newY, getZ() );
      ((MD2Shape*)shape)->setAngle( angle + 180.0f );
      if( toint(newX) == toint(lx) && toint(newY) == toint(ly) ) {
        bestPathPos++;
      }
      return true;
    } else {
      // if we're not at the destination, but it's possible to stand there
      // try again
      if(!(selX == toint(getX()) && selY == toint(getY())) && 
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
    }
  }
  return false;
}

void Creature::stopMoving() {
  bestPathPos = (int)bestPath.size();
  selX = selY = -1;
}

bool Creature::anyMovesLeft() {
  return(selX > -1 && (int)bestPath.size() > bestPathPos); 
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
    *px = (*(px) * getShape()->getWidth()) + toint(next->getX());
    *py = (-(*(py)) * getShape()->getDepth()) + toint(next->getY());
  }
  *pz = toint(next->getZ());
}

/**
  Used to move away from the player. Find the nearest corner of the map.
*/
void Creature::findCorner(Sint16 *px, Sint16 *py, Sint16 *pz) {
  if(getX() < session->getParty()->getPlayer()->getX() &&
     getY() < session->getParty()->getPlayer()->getY()) {
    *px = *py = *pz = 0;
    return;
  }
  if(getX() >= session->getParty()->getPlayer()->getX() &&
     getY() < session->getParty()->getPlayer()->getY()) {
    *px = MAP_WIDTH;
    *py = *pz = 0;
    return;
  }
  if(getX() < session->getParty()->getPlayer()->getX() &&
     getY() >= session->getParty()->getPlayer()->getY()) {
    *px = *pz = 0;
    *py = MAP_DEPTH;
    return;
  }
  if(getX() >= session->getParty()->getPlayer()->getX() &&
     getY() >= session->getParty()->getPlayer()->getY()) {
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

bool Creature::addInventory(Item *item, bool force) { 
  if(inventory_count < MAX_INVENTORY_SIZE &&
     (force || !item->isBlocking() || getShape()->fitsInside(item->getShape()))) {
    inventory[inventory_count++] = item;
    inventoryWeight += item->getRpgItem()->getWeight(); 

    if(item->getRpgItem()->getWeight() + inventoryWeight > 
       getMaxInventoryWeight()) {
      char msg[80];
      sprintf(msg, "%s is overloaded.", getName());
      session->getMap()->addDescription(msg);            
      setStateMod(Constants::overloaded, true);
    }

    // check if the mission is over
    if(!isMonster() && 
       session->getCurrentMission() &&
       session->getCurrentMission()->itemFound(item->getRpgItem())) {
      session->getGameAdapter()->missionCompleted();
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
      session->getMap()->addDescription(msg);            
      setStateMod(Constants::overloaded, false);
    }
    for(int i = index; i < inventory_count - 1; i++) {
      inventory[i] = inventory[i + 1];
    }
    inventory_count--;
    // adjust equipped indexes too
    for(int i = 0; i < Constants::INVENTORY_COUNT; i++) {
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
      session->getMap()->addDescription(msg); 
      return false;
    }

    // TODO : the quality member of rpgItem should indicate if the
    // food is totally healthy or roten or partially roten etc...
    // We eat the item and it gives us "level" hunger points back
    setHunger(getHunger() + level);            
    strcpy(buff, rpgItem->getShortDesc());
    buff[0] = tolower(buff[0]);
    sprintf(msg, "%s eats %s.", getName(), buff);
    session->getMap()->addDescription(msg);
    bool b = item->decrementCharges();
    if(b) {
      sprintf(msg, "%s is used up.", item->getItemName());
      session->getMap()->addDescription(msg);
    }
    return b;
  } else if(type == RpgItem::DRINK){
    if(getThirst() == 10){                
      sprintf(msg, "%s is not thirsty at the moment.", getName()); 
      session->getMap()->addDescription(msg); 
      return false;
    }
    setThirst(getThirst() + level);
    strcpy(buff, rpgItem->getShortDesc());
    buff[0] = tolower(buff[0]);
    sprintf(msg, "%s drinks %s.", getName(), buff);
    session->getMap()->addDescription(msg); 
    // TODO : according to the alcool rate set drunk state or not            
    bool b = item->decrementCharges();
    if(b) {
      sprintf(msg, "%s is used up.", item->getItemName());
      session->getMap()->addDescription(msg);
    }
    return b;
  } else if(type == RpgItem::POTION) {
    // It's a potion            
    // Even if not thirsty, character will always drink a potion
    strcpy(buff, rpgItem->getShortDesc());
    buff[0] = tolower(buff[0]);
    setThirst(getThirst() + level);
    sprintf(msg, "%s drinks from %s.", getName(), buff);
    session->getMap()->addDescription(msg); 
    usePotion(item);
    bool b = item->decrementCharges();
    if(b) {
      sprintf(msg, "%s is used up.", item->getItemName());
      session->getMap()->addDescription(msg);
    }
    return b;
  } else {
    session->getMap()->addDescription("You cannot eat or drink that!", 1, 0.2f, 0.2f);
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
      session->getMap()->addDescription(msg, 0.2f, 1, 1);
      startEffect(Constants::EFFECT_SWIRL, (Constants::DAMAGE_DURATION * 4));
      return;
    case Constants::MP:
      n = item->getRpgItem()->getAction();
      if(n + getMp() > getMaxMp())
        n = getMaxMp() - getMp();
      setMp(getMp() + n);
      sprintf(msg, "%s receives %d magic points.", getName(), n);
      session->getMap()->addDescription(msg, 0.2f, 1, 1);
      startEffect(Constants::EFFECT_SWIRL, (Constants::DAMAGE_DURATION * 4));
      return;
    case Constants::AC:
      {
        bonusArmor += item->getRpgItem()->getAction();
        recalcAggregateValues();
        sprintf(msg, "%s feels impervious to damage!", getName());
        session->getMap()->addDescription(msg, 0.2f, 1, 1);
        startEffect(Constants::EFFECT_SWIRL, (Constants::DAMAGE_DURATION * 4));

        // add calendar event to remove armor bonus
        // (format : sec, min, hours, days, months, years)
        Date d(0, item->getRpgItem()->getPotionTime(), 0, 0, 0, 0); 
        Event *e = 
        new PotionExpirationEvent(session->getParty()->getCalendar()->getCurrentDate(), 
                                  d, this, item->getRpgItem(), session, 1);
        session->getParty()->getCalendar()->scheduleEvent((Event*)e);   // It's important to cast!!		
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
    session->getMap()->addDescription(msg, 0.2f, 1, 1);
    startEffect(Constants::EFFECT_SWIRL, (Constants::DAMAGE_DURATION * 4));

    // add calendar event to remove armor bonus
    // (format : sec, min, hours, days, months, years)
    Date d(0, item->getRpgItem()->getPotionTime(), 0, 0, 0, 0); 
    Event *e = 
    new PotionExpirationEvent(session->getParty()->getCalendar()->getCurrentDate(), 
                              d, this, item->getRpgItem(), session, 1);
    session->getParty()->getCalendar()->scheduleEvent((Event*)e);   // It's important to cast!!
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
    this->battle->invalidate();
    sprintf(msg, "%s will consume %s.", getName(), item->getItemName());
    break;
  case Constants::ACTION_CAST_SPELL:
    this->battle->invalidate();
    sprintf(msg, "%s will cast %s.", getName(), spell->getName());
    break;
  case Constants::ACTION_NO_ACTION:
    // no-op
    preActionTargetCreature = NULL;
    strcpy(msg, "");
    break;
  default:
    cerr << "*** Error: unknown action " << action << endl;
    return;
  }
  if(strlen(msg)) session->getMap()->addDescription(msg, 1, 1, 0.5f);
}

void Creature::equipInventory(int index) {
  this->battle->invalidate();
  // doff
  if(doff(index)) return;
  // don
  // FIXME: take into account: two-handed weapons, race/class modifiers, min skill req-s., etc.
  Item *item = getInventory(index);
  for(int i = 0; i < Constants::INVENTORY_COUNT; i++) {
    // if the slot is empty and the item can be worn here
    if(item->getRpgItem()->getEquip() & ( 1 << i ) && 
       equipped[i] == MAX_INVENTORY_SIZE) {
      equipped[i] = index;

      // handle magic attrib settings
      if(item->getMagicAttrib()) {

        item->getMagicAttrib()->debug("Equip: ", item->getRpgItem());

        // set the good attributes
        for(int i = 0; i < Constants::STATE_MOD_COUNT; i++) {
          if(item->getMagicAttrib()->isStateModSet(i)) {
            this->setStateMod(i, true);
          }
        }
        // set the protected attributes
        for(int i = 0; i < Constants::STATE_MOD_COUNT; i++) {
          if(item->getMagicAttrib()->isStateModProtected(i)) {
            this->setProtectedStateMod(i, true);
          }
        }
        // skill bonuses
        map<int,int> *m = item->getMagicAttrib()->getSkillBonusMap();
        for(map<int,int>::iterator e=m->begin(); e!=m->end(); ++e) {
          int skill = e->first;
          int bonus = e->second;
          setSkillBonus(skill, getSkillBonus(skill) + bonus);
        }
        // if armor, enhance magic resistance
        if(!item->getRpgItem()->isWeapon() && 
           item->getMagicAttrib()->getSchool()) {
          int skill = item->getMagicAttrib()->getSchool()->getResistSkill();
          setSkillBonus(skill, getSkillBonus(skill) + item->getMagicAttrib()->getMagicResistance());
        }
        // refresh map for invisibility, etc.
        session->getMap()->refresh();
      }
      recalcAggregateValues();
      return;
    }
  }
}

int Creature::doff(int index) {
  // doff
  for(int i = 0; i < Constants::INVENTORY_COUNT; i++) {
    if(equipped[i] == index) {
      Item *item = getInventory(index);
      equipped[i] = MAX_INVENTORY_SIZE;

      // handle magic attrib settings
      if(item->getMagicAttrib()) {

        item->getMagicAttrib()->debug("Doff: ", item->getRpgItem());

        // set the good attributes
        for(int i = 0; i < Constants::STATE_MOD_COUNT; i++) {
          if(item->getMagicAttrib()->isStateModSet(i)) {
            this->setStateMod(i, false);
          }
        }
        // set the protected attributes
        for(int i = 0; i < Constants::STATE_MOD_COUNT; i++) {
          if(item->getMagicAttrib()->isStateModProtected(i)) {
            this->setProtectedStateMod(i, false);
          }
        }
        // skill bonus
        map<int,int> *m = item->getMagicAttrib()->getSkillBonusMap();
        for(map<int,int>::iterator e=m->begin(); e!=m->end(); ++e) {
          int skill = e->first;
          int bonus = e->second;
          setSkillBonus(skill, getSkillBonus(skill) - bonus);
        }
        // if armor, enhance magic resistance
        if(!item->getRpgItem()->isWeapon() && 
           item->getMagicAttrib()->getSchool()) {
          int skill = item->getMagicAttrib()->getSchool()->getResistSkill();
          setSkillBonus(skill, getSkillBonus(skill) - item->getMagicAttrib()->getMagicResistance());
        }

        // refresh map for invisibility, etc.
        session->getMap()->refresh();
      }

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
  for(int i = 0; i < Constants::INVENTORY_COUNT; i++) {
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
  for(i = 0; i < Constants::INVENTORY_COUNT; i++) {
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
  for(int i = 0; i < Constants::INVENTORY_COUNT; i++) {
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
  Item *item = getItemAtLocation(Constants::INVENTORY_RIGHT_HAND);
  if(item && item->getRpgItem()->getDistance() >= dist) return item;
  item = getItemAtLocation(Constants::INVENTORY_LEFT_HAND);
  if(item && item->getRpgItem()->getDistance() >= dist) return item;
  item = getItemAtLocation(Constants::INVENTORY_WEAPON_RANGED);
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
  return(10 - (int)(speed / 15.0f));
}

// return number of projectiles that can be launched simultaniously
// it is a function of speed, level, and weapon skill
// this method returns a number from 1-10
int Creature::getMaxProjectileCount(Item *item) {
  int n = (int)((double)(getSkill(Constants::SPEED) + 
                         (getLevel() * 10) + 
                         getSkill(item->getRpgItem()->getSkill())) / 30.0f);
  if(n <= 0) n = 1;
  return n;
}

// roll the die for the toHit number. returns a value between 0(total miss) - 100(best hit)
int Creature::getToHit(Item *weapon, int *maxToHit, int *rolledToHit) {
  float tohit = getSkill(Constants::COORDINATION) + getSkill(Constants::LUCK) / 2;
  if(weapon && weapon->getRpgItem()->getSkill() > -1) {
    tohit += getSkill(weapon->getRpgItem()->getSkill());
  } else {
    tohit += getSkill(Constants::HAND_TO_HAND_COMBAT);
  }
  // so far the max score is 250

  if(maxToHit) *maxToHit = (int)(tohit / 2.5f);

  // roll it
  float score = (tohit * rand()/RAND_MAX);
  
  // convert to 0-100 value
  int ret = (int)(score / 2.5f);
  
  if(rolledToHit) *rolledToHit = ret;
  return ret;
}

// return the damage as:
int Creature::getDamage(Item *weapon, int *maxDamage, int *rolledDamage) {
  float damage = 0.0f;
  // get the base damage
  float baseDamage = (weapon ? weapon->getRpgItem()->getAction() : 
                      (getSkill(Constants::POWER) / 10));
  damage = baseDamage;

  // add strength proficiency (10% of strength=0-10 pts)
  damage += (float)getSkill(Constants::POWER) / 10.0f;

  float skill = (weapon && weapon->getRpgItem()->getSkill() > -1 ?
                 getSkill(weapon->getRpgItem()->getSkill()) :
                 getSkill(Constants::HAND_TO_HAND_COMBAT));
  
  // add 50% of value, more via skill proficiency
  if(skill > 50.0f) {
    damage += (damage * ((skill - 50) / 100.0f) );
  }

  if(maxDamage) *maxDamage = (int)damage;

  // return the 70% of value + 30% random
  int ret = (int)((damage * .7) + ((damage * 0.3) * rand()/RAND_MAX));

  if(rolledDamage) *rolledDamage = ret;
  return ret;
}

/**
 take some damage
*/
bool Creature::takeDamage(int damage, int effect_type) {
  hp -= damage;
  // if creature dies start effect at its location
  if(hp > 0) {
    startEffect(effect_type);
    int pain = (int)(3.0f * rand()/RAND_MAX);
    getShape()->setCurrentAnimation(pain == 0 ? (int)MD2_PAIN1 : (pain == 1 ? (int)MD2_PAIN2 : (int)MD2_PAIN3));
  } else if(effect_type != Constants::EFFECT_GLOW) {
    session->getMap()->startEffect(toint(getX()), toint(getY()), toint(getZ()), 
                                   effect_type, (Constants::DAMAGE_DURATION * 4), 
                                   getShape()->getWidth(), getShape()->getDepth());
  }
  return(hp <= 0);
}

void Creature::startEffect(int effect_type, int duration) {
  // show an effect
  if(isEffectOn() && effect_type == getEffectType()) {
    return;
  }
  effect->deleteParticles();
  resetDamageEffect();
  setEffectType(effect_type);
  effectDuration = duration;

  // need to do this to make sure effect shows up
  session->getMap()->refresh();
}

/**
   Get the total value of armor worn and roll for the skill of each piece.
   Monsters' base armor is not rolled (ie. they're experts in using their natural armor.)
 */
int Creature::getSkillModifiedArmor() {
  // calculate the armor (0-100, 100-total protection)
  int armor = (monster ? monster->getBaseArmor() : 0);
  armor += bonusArmor;
  for(int i = 0; i < Constants::INVENTORY_COUNT; i++) {
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
        
        // magic armor?
        if(item->getMagicAttrib()) {
          armor += item->getMagicAttrib()->getBonus();
        }
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
  float m;
  if(creature_killed->isMonster()) {
    m = (float)(creature_killed->getMonster()->getHp()) / 2.0f;
  } else {
    m = (float)(creature_killed->getLevel() * 10) / 2.0f;
  }
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
  // set some skills
  for(int i = 0; i < Constants::SKILL_COUNT; i++) {
    int n = monster->getSkillLevel(Constants::SKILL_NAMES[i]);
    setSkill(i, ( n > 0 ? n : (int)((float)(10 * level) * rand()/RAND_MAX)) );
  }
  // equip starting inventory
  for(int i = 0; i < getMonster()->getStartingItemCount(); i++) {
    addInventory(session->newItem(getMonster()->getStartingItem(i)), true);
    equipInventory(inventory_count - 1);
  }
  // add spells
  for(int i = 0; i < getMonster()->getStartingSpellCount(); i++) {
    addSpell(getMonster()->getStartingSpell(i));
  }

  // add some hp
  //startingHp = hp = 4 + (int)((float)(10.0f * level) * rand()/RAND_MAX);
  //startingMp = mp = 4 + (int)((float)(4.0f * level) * rand()/RAND_MAX);

  startingHp = hp = monster->getHp() + (int)((float)(10.0f * level) * rand()/RAND_MAX);
  startingMp = mp = monster->getMp() + (int)((float)(4.0f * level) * rand()/RAND_MAX);

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
    return(character->getStartingHp() * getLevel());
  }
}

int Creature::getMaxMp() {
  if(isMonster()) {
    return monster->getMp(); // FIXME: incorrect, see monsterInit()
  } else {
    return(character->getStartingMp() * getLevel());
  }
}

float Creature::getTargetAngle() {
  //if(!targetCreature) return -1.0f;
  if(!targetCreature) return angle;
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

// FIXME: O(n) but there aren't that many spells...
// return true if spell was added, false if creature already had this spell
bool Creature::addSpell(Spell *spell) { 
  for(vector<Spell*>::iterator e = spells.begin(); e != spells.end(); ++e) {
    Spell *thisSpell = *e;
    if(thisSpell == spell) return false;
  }
  spells.push_back(spell); 
  return true;
}

// this assumes that hasTarget() was called first.
bool Creature::isTargetValid() {
  // is it a non-creature target? (item or location)
  if(!getTargetCreature()) return true;
  if(getTargetCreature()->getStateMod(Constants::dead)) return false;
  // when attacking, attack the opposite kind (unless possessed)
  // however, you can cast spells on anyone
  if(getAction() == Constants::ACTION_NO_ACTION && 
     !canAttack(getTargetCreature())) return false;
  return true;
}

bool Creature::canAttack(Creature *creature) {
  // when attacking, attack the opposite kind (unless possessed)
  return (getStateMod(Constants::possessed) == 
          (isMonster() == creature->isMonster()));
}

void Creature::cancelTarget() {
  setTargetCreature(NULL);
  setTargetItem(0, 0, 0, NULL);
  setDistanceRange(0, 0);
  if(preActionTargetCreature) setTargetCreature(preActionTargetCreature);
  preActionTargetCreature = NULL;
  setAction(Constants::ACTION_NO_ACTION);
  if(isMonster()) setMotion(Constants::MOTION_LOITER);     
}

void Creature::followTarget() {
  setSelXY(getTargetX(),
           getTargetY(),
           true);
}

// FIXME: make this more intelligent: potions, spells, scrolls, etc.
void Creature::decideMonsterAction() {
  if(!isMonster()) return;

  // does monster need to be healed?

  // increase MP, AC or skill (via potion)?
  Creature *p;
  if(getStateMod(Constants::possessed)) {
    p = session->getClosestVisibleMonster(toint(getX()), toint(getY()), 
                                          getShape()->getWidth(),
                                          getShape()->getDepth(),
                                          20);
  } else {
    p = session->getParty()->getClosestPlayer(toint(getX()), toint(getY()), 
                                              getShape()->getWidth(),
                                              getShape()->getDepth(),
                                              20);
  }
  if(p) {
    float dist = Constants::distance(getX(),  getY(), 
                                     getShape()->getWidth(), getShape()->getDepth(),
                                     p->getX(), p->getY(),
                                     p->getShape()->getWidth(), 
                                     p->getShape()->getDepth());
    
    // can monster use magic?
    if(getSpellCount()) {

      /*              
        FIXME:          
                  
        Other considerations:
        -heal other monsters (by spell)?
        -attack by spell?
        -group attack vs. single target?
        
        For now, just assume that monsters only know attack spells 
        where the target can be a single creature. Later when spell
        casting is smarter (by monsters) we'll need to implement some
        way to classify spells (attack, help (hp+,ac+,etc.), group vs. 
        single target, etc.)
      */
      for(int i = 0; i < getSpellCount(); i++) {
        Spell *spell = getSpell(i);
        if(spell->getMp() < getMp()) {
          if((spell->getDistance() == 1 && dist <= Constants::MIN_DISTANCE) ||
             (spell->getDistance() > 1 && dist > Constants::MIN_DISTANCE)) {
            setAction(Constants::ACTION_CAST_SPELL, 
                      NULL,
                      spell);
            setMotion(Constants::MOTION_MOVE_TOWARDS);
            setTargetCreature(p);
            return;
          }
        }
      }
    }

    // attack with item
    setMotion(Constants::MOTION_MOVE_TOWARDS);
    setTargetCreature(p);
    //setDistanceRange(0, Constants::MIN_DISTANCE);
  }
}

float Creature::getDistanceToTarget() {
  if(!hasTarget()) return 0.0f;
  if(getTargetCreature()) {
    return Constants::distance(getX(),  getY(), 
                               getShape()->getWidth(), getShape()->getDepth(),
                               getTargetCreature()->getX(), getTargetCreature()->getY(),
                               getTargetCreature()->getShape()->getWidth(), 
                               getTargetCreature()->getShape()->getDepth());
  } else if(getTargetItem()) {
    return Constants::distance(getX(),  getY(), 
                               getShape()->getWidth(), getShape()->getDepth(),
                               getTargetX(), getTargetY(),
                               getTargetItem()->getShape()->getWidth(), 
                               getTargetItem()->getShape()->getDepth());
  } else {
    return Constants::distance(getX(),  getY(), 
                               getShape()->getWidth(), getShape()->getDepth(),
                               getTargetX(), getTargetY(), 1, 1);
  }
}

// sets min exp for current level
void Creature::setExp() {
  if(isMonster()) return;
  expOfNextLevel = 0;
  for(int i = 0; i < level - 1; i++) {
    expOfNextLevel += ((i + 1) * character->getLevelProgression());
  }
}

