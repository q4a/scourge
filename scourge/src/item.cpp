/***************************************************************************
                          item.cpp  -  description
                             -------------------
    begin                : Sun Sep 28 2003
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

#include "item.h"
#include "render/renderlib.h"
#include "rpg/rpglib.h"
#include "session.h"
#include "shapepalette.h"

using namespace std;

map<int, vector<string> *> Item::soundMap;

Item::Item(Session *session, RpgItem *rpgItem, int level, bool loading) {
  this->session = session;
  this->rpgItem = rpgItem;
  this->level = level;
  this->shapeIndex = this->rpgItem->getShapeIndex();
  this->color = NULL;
  this->shape = session->getShapePalette()->getShape(shapeIndex);
  // for now objects larger than 1 height will block (we can change this later)
  // this is so the player is not blocked by swords and axes on the ground
  this->blocking = (shape->getHeight() > 1 || 
                    rpgItem->getType() == RpgItem::CONTAINER ||
                    rpgItem->getType() == RpgItem::MISSION);
  this->containedItemCount = 0;
  this->spell = NULL;
  this->containsMagicItem = false;
  this->showCursed = false;
  sprintf(this->itemName, "%s", rpgItem->getName());

  commonInit( loading );

  currentCharges = rpgItem->getMaxChargesRpg();
  weight = rpgItem->getWeightRpg();
}

Item::~Item(){
}

ItemInfo *Item::save() {
  ItemInfo *info = (ItemInfo*)malloc(sizeof(ItemInfo));
  info->version = PERSIST_VERSION;
  info->level = getLevel();
  strcpy((char*)info->rpgItem_name, getRpgItem()->getName());
  strcpy((char*)info->shape_name, getShape()->getName());
  info->blocking = blocking;
  info->currentCharges = currentCharges;
  info->weight = (Uint32)(weight * 100);
  info->quality = quality;
  info->price = price;
  info->speed = speed;
  info->distance = distance;
  info->maxCharges = maxCharges;
  info->duration = duration;
  strcpy((char*)info->spell_name, (spell ? spell->getName() : ""));
  int realCount = 0;
  for(int i = 0; i < containedItemCount; i++) {
    if( containedItems[i] ) info->containedItems[realCount++] = containedItems[i]->save();
  }
  info->containedItemCount = realCount;

  info->bonus = bonus;
  info->damageMultiplier = damageMultiplier;
  info->cursed = cursed;
  info->magicLevel = magicLevel;
  strcpy((char*)info->monster_type, (this->monsterType ? monsterType : ""));
  strcpy((char*)info->magic_school_name, (this->school ? school->getName() : ""));
  info->magicDamage = (school ? saveDice( magicDamage ) : saveEmptyDice());
  for(int i = 0; i < Constants::STATE_MOD_COUNT; i++) {
    info->stateMod[i] = this->stateMod[i];
  }
  for(int i = 0; i < Constants::SKILL_COUNT; i++) {
    info->skillBonus[i] = this->getSkillBonus(i);
  }

  return info;
}

DiceInfo *Item::saveDice( Dice *dice ) {
  DiceInfo *info = (DiceInfo*)malloc(sizeof(DiceInfo));
  info->version = PERSIST_VERSION;
  info->count = dice->getCount();
  info->sides = dice->getSides();
  info->mod = dice->getMod();
  return info;
}

DiceInfo *Item::saveEmptyDice() {
  DiceInfo *info = (DiceInfo*)malloc(sizeof(DiceInfo));
  info->version = PERSIST_VERSION;
  info->count = 0;
  info->sides = 0;
  info->mod = 0;
  return info;
}

Dice *Item::loadDice( Session *session, DiceInfo *info ) {
  if( !info->count ) return NULL;
  return new Dice( info->count, info->sides, info->mod );
}


/*
ContainedItemInfo Item::saveContainedItems() {
  ContainedItemInfo info;
  info->version = PERSIST_VERSION;
  info->containedItemCount = containedItemCount;
  for(int i = 0; i < containedItemCount; i++) {
    info->containedItems[i] = containedItems[i]->save();
  }
  return info;
}
*/

Item *Item::load(Session *session, ItemInfo *info) {
  if( !strlen( (char*)info->rpgItem_name )) return NULL;
  Spell *spell = NULL;
  if( strlen((char*)info->spell_name) ) spell = Spell::getSpellByName( (char*)info->spell_name );
  RpgItem *rpgItem = RpgItem::getItemByName( (char*)info->rpgItem_name );
  if( !rpgItem ) {
    cerr << "Error: can't find rpgItem with name:" << (char*)info->rpgItem_name << endl;
    return NULL;
  }
  Item *item = session->newItem( rpgItem, 
                                 info->level, 
                                 spell,
                                 true);
  item->blocking = (info->blocking == 1);
  item->currentCharges = info->currentCharges;
  item->weight = (float)(info->weight) / 100.0f;
  item->quality = info->quality;
  item->price = info->price;
  item->speed = info->speed;
  item->distance = info->distance;
  item->maxCharges = info->maxCharges;
  item->duration = info->duration;  
  item->containedItemCount = info->containedItemCount;
  int realCount = 0;
  for(int i = 0; i < (int)info->containedItemCount; i++) {
     Item *containedItem = Item::load( session, info->containedItems[i] );
     if( containedItem ) item->containedItems[ realCount++ ] = containedItem;
  }
  info->containedItemCount = realCount;
    
  item->bonus = info->bonus;
  item->damageMultiplier = info->damageMultiplier;
  item->cursed = (info->cursed == 1);
  item->magicLevel = info->magicLevel;
  // get a reference to the real string... (yuck)
  item->monsterType = (char*)Monster::getMonsterType( (char*)info->monster_type );
  // turn off "vs. any creature"
  if( !item->getMonsterType() ) item->damageMultiplier = 1;
  item->school = MagicSchool::getMagicSchoolByName( (char*)info->magic_school_name );
  item->magicDamage = Item::loadDice( session, info->magicDamage );
  for(int i = 0; i < Constants::STATE_MOD_COUNT; i++) {
    item->stateMod[i] = info->stateMod[i];
  }
  for(int i = 0; i < Constants::SKILL_COUNT; i++) {
    if(info->skillBonus[i]) item->skillBonus[i] = info->skillBonus[i];
  }

  if( item->isMagicItem() )
    item->describeMagic(item->itemName, item->rpgItem->getName());

  return item;
}

bool Item::addContainedItem(Item *item, bool force) { 
  if(containedItemCount < MAX_CONTAINED_ITEMS && 
     (force || !item->isBlocking() || getShape()->fitsInside(item->getShape()))) {
    containedItems[containedItemCount++] = item; 
    if( item->isMagicItem() ) containsMagicItem = true;
    return true;
  } else {
    cerr << "Warning: unable to add to container. Container=" << getRpgItem()->getName() << " item=" << item->getRpgItem()->getName() << endl;
    return false;
  }
} 

Item *Item::removeContainedItem(int index) {
  Item *item = NULL;
  if(index >= 0 && index < containedItemCount) {
    item = containedItems[index];
    containedItemCount--;
    for(int i = index; i < containedItemCount; i++) {
      containedItems[i] = containedItems[i + 1];
    }
    containsMagicItem = false;
    for( int i = 0; i < containedItemCount; i++ ) {
      if( containedItems[i]->isMagicItem() ) containsMagicItem = true;
    }
  }
  return item;
}

Item *Item::getContainedItem(int index) {
  return((index >= 0 && index < containedItemCount) ? containedItems[index] : NULL);
}

bool Item::isContainedItem(Item *item) {
  for(int i = 0; i < containedItemCount; i++) {
    if(containedItems[i] == item || 
       (containedItems[i]->getRpgItem()->getType() == RpgItem::CONTAINER &&
        containedItems[i]->isContainedItem(item))) return true;
  }
  return false;
}

void Item::getDetailedDescription(char *s, bool precise){
  int type;
  RpgItem * rpgItem;

  rpgItem  = getRpgItem();
  type = rpgItem->getType();
  /*
  if(type == RpgItem::DRINK || type == RpgItem::POTION || type == RpgItem::FOOD){
    sprintf(s, "(L:%d,Q:%d,W:%2.2f, N:%d/%d) %s%s", 
            getLevel(), 
            getQuality(), 
            getWeight(),
            getCurrentCharges(),
            getMaxCharges(),
            (precise ? itemName : rpgItem->getShortDesc()),
            (session->getCurrentMission() && 
             session->getCurrentMission()->isMissionItem( this ) ? 
             " *Mission*" : ""));
  } else if(type == RpgItem::SCROLL) {
    sprintf(s, "(L:%d) %s%s", getLevel(), itemName,
            (session->getCurrentMission() && 
             session->getCurrentMission()->isMissionItem( this ) ? 
             " *Mission*" : ""));
  } else {
    sprintf(s, "(L:%d,A:%d,S:%d,Q:%d,W:%2.2f) %s%s", 
            getLevel(), 
            getAction(), 
            getSpeed(), 
            getQuality(), 
            getWeight(),
            (precise ? itemName : rpgItem->getShortDesc()),
            (session->getCurrentMission() && 
             session->getCurrentMission()->isMissionItem( this ) ? 
             " *Mission*" : ""));
  }
  */

  if(type == RpgItem::DRINK || type == RpgItem::POTION || type == RpgItem::FOOD){
    sprintf(s, "(L:%d) %s%s%s", 
            getLevel(), 
            ( isCursed() && getShowCursed() ? "*Cursed* " : "" ),
            (precise ? itemName : rpgItem->getShortDesc()),
            (session->getCurrentMission() && 
             session->getCurrentMission()->isMissionItem( this ) ? 
             " *Mission*" : ""));
  } else if(type == RpgItem::SCROLL) {
    sprintf(s, "(L:%d) %s%s%s", 
            getLevel(), 
            ( isCursed() && getShowCursed() ? "*Cursed* " : "" ),
            itemName,
            (session->getCurrentMission() && 
             session->getCurrentMission()->isMissionItem( this ) ? 
             " *Mission*" : ""));
  } else {
    sprintf(s, "(L:%d) %s%s%s", 
            getLevel(), 
            ( isCursed() && getShowCursed() ? "*Cursed* " : "" ),
            (precise ? itemName : rpgItem->getShortDesc()),
            (session->getCurrentMission() && 
             session->getCurrentMission()->isMissionItem( this ) ? 
             " *Mission*" : ""));
  }
}

// this should really be in RpgItem but that class can't reference ShapePalette and shapes.
void Item::initItems(ShapePalette *shapePal) {
  char errMessage[500];
  char s[200];
  sprintf(s, "%s/world/items.txt", rootDir);
  FILE *fp = fopen(s, "r");
  if(!fp) {        
    sprintf(errMessage, "Unable to find the file: %s!", s);
    cerr << errMessage << endl;
    exit(1);
  }

  int itemCount = 0, potionTime = 0;
  char name[255], type[255], shape[255], skill[255], potionSkill[255];
  char long_description[500], short_description[120];
  char line[255];
  RpgItem *last = NULL;
  int n = fgetc(fp);
  while(n != EOF) {
    if( n == 'T' ) {
      // skip ':'
      fgetc(fp);
      n = Constants::readLine( line, fp );
      ItemType itemType;
      strcpy( itemType.name, strtok( line, "," ) );
      char *p = strtok( NULL, "," );
      itemType.isWeapon = ( p && *p == '1' ? true : false );
      p = strtok( NULL, "," );
      itemType.isArmor = ( p && *p == '1' ? true : false );
      p = strtok( NULL, "," );
      itemType.isRandom = ( p && *p == '1' ? true : false );
      p = strtok( NULL, "," );
      itemType.isRanged = ( p && *p == '1' ? true : false );
      p = strtok( NULL, "," );
      itemType.hasSpell = ( p && *p == '1' ? true : false );
      p = strtok( NULL, "," );
      itemType.isEnchantable = ( p && *p == '1' ? true : false );
      RpgItem::itemTypes.push_back( itemType );
      if( itemType.isRandom ) {
        RpgItem::randomTypes[ RpgItem::randomTypeCount++ ] = RpgItem::itemTypes.size() - 1;
      }
    } else if( n == 'I' ) {
      // skip ':'
      fgetc(fp);
      // read the rest of the line
      n = Constants::readLine(name, fp);
      n = Constants::readLine(line, fp);
      int rareness = atoi(strtok(line + 1, ","));
      char *p = strtok(NULL, ",");
      char *action = NULL;
      int speed = 0;
      int distance = 0;
      int maxCharges = 0;
      if(p) {
        action = strdup(p);
        speed = atoi(strtok(NULL, ","));
        distance = atoi(strtok(NULL, ","));
        maxCharges = atoi(strtok(NULL, ","));
        p = strtok(NULL, ",");
        if(p) strcpy(potionSkill, p);
        else strcpy(potionSkill, "");
        p = strtok(NULL, ",");
        potionTime = (p ? atoi(p) : 0);
      }
      n = Constants::readLine(line, fp);
      strcpy(type, strtok(line + 1, ","));
      float weight = strtod(strtok(NULL, ","), NULL);
      int price = atoi(strtok(NULL, ","));

      int inventory_location = 0;
      int twohanded = 0;
      strcpy(shape, "");
      strcpy(skill, "");
      int minDepth = 0;
      int minLevel = 0;
      p = strtok(NULL, ",");    
      if(p) {
        strcpy(shape, p);
        p = strtok(NULL, ",");
        if(p) {
          inventory_location = atoi(p);
          twohanded = atoi(strtok(NULL, ","));
          strcpy(skill, strtok(NULL, ","));
          p = strtok( NULL, "," );
          if( p ) {
            minDepth = atoi( p );
            p = strtok( NULL, "," );
            if( p ) {
              minLevel = atoi( p );
            }
          }
        }
      }

      n = Constants::readLine(line, fp);
      strcpy(long_description, line + 1);

      n = Constants::readLine(line, fp);
      strcpy(short_description, line + 1);
      
      n = Constants::readLine(line, fp);
      int tileX = atoi( strtok( line + 1, "," ) );
      int tileY = atoi( strtok( NULL, "," ) );
      char *bonusSkillStr = strtok( NULL, "," );
      int maxBonusSkill = ( bonusSkillStr ? atoi( bonusSkillStr ) : -1 );

      // resolve strings
      int type_index = RpgItem::getTypeByName(type);    
      //cerr << "item: looking for shape: " << shape << endl;
      int shape_index = shapePal->findShapeIndexByName(shape);
      //cerr << "\tindex=" << shape_index << endl;
      int skill_index = Constants::getSkillByName(skill);
      if(skill_index < 0) {
        if(strlen(skill)) cerr << "*** WARNING: cannot find skill: " << skill << endl;
        skill_index = 0;
      }
      int potion_skill = -1;
      if(potionSkill != NULL && strlen(potionSkill)) {
        potion_skill = Constants::getSkillByName(potionSkill);
        if(potion_skill < 0) {
          // try special potion 'skills' like HP, AC boosts
          potion_skill = Constants::getPotionSkillByName(potionSkill);
          if(potion_skill == -1) {
            cerr << "*** WARNING: cannot find potion_skill: " << potionSkill << endl;
          }
        }
        //cerr << "**** potionSkill=" << potionSkill << " potion_skill=" << potion_skill << endl;
      }
      if(distance < (int)MIN_DISTANCE) distance = (int)MIN_DISTANCE;
      last = new RpgItem( itemCount++, strdup(name), 1, rareness, type_index, 
                          weight, price, 100, 
                          ( action && strlen( action ) ? new Dice( action ) : NULL ), 
                          speed, strdup(long_description), 
                          strdup(short_description), 
                          inventory_location, shape_index, 
                          twohanded, distance, 
                          skill_index, minDepth, minLevel, 
                          maxCharges, potion_skill, potionTime, 
                          tileX - 1, tileY - 1, maxBonusSkill );
      GLShape *s = shapePal->findShapeByName(shape);
      RpgItem::addItem(last, s->getWidth(), s->getDepth(), s->getHeight() );   
    } else if(n == 'A' && last) {
      // skip ':'
      fgetc(fp);
      // read the rest of the line
      n = Constants::readLine(line, fp);
      char *p = strtok(line, ",");
      while(p) {
        int len = strlen(p);
        bool value = (p[len - 1] == '+');
        if(p[0] == '*') {
          last->setAllAcl(value);
        } else {
          char shortName[3];
          shortName[0] = p[0];
          shortName[1] = p[1];
          shortName[2] = 0;
          last->setAcl(Character::getCharacterIndexByShortName(shortName), value);
        }
        p = strtok(NULL, ",");
      }
    } else if(n == 'S') {
      fgetc(fp);
      n = Constants::readLine(line, fp);
      char *p = strtok(line, ",");
      int type_index = RpgItem::getTypeByName(p);
      vector<string> *sounds = NULL;
      if(Item::soundMap.find(type_index) != Item::soundMap.end()) {
        sounds = Item::soundMap[type_index];
      } else {
        sounds = new vector<string>;
        Item::soundMap[type_index] = sounds;
      }
      p = strtok(NULL, ",");
      while(p) {
        string f = p;
        sounds->push_back(f);
        p = strtok(NULL, ",");
      }
    } else {
      n = Constants::readLine(line, fp);
    }
  }
  fclose(fp);
}

const char *Item::getRandomSound() {
  vector<string> *sounds = NULL;
  if(Item::soundMap.find(this->getRpgItem()->getType()) != Item::soundMap.end()) {
    sounds = Item::soundMap[this->getRpgItem()->getType()];
  }
  if(!sounds || !(sounds->size())) return NULL;
  string s = (*sounds)[(int)((float)(sounds->size()) * rand()/RAND_MAX)];
  return s.c_str();
}

// return true if the item is used up
bool Item::decrementCharges(){
  float f1;
  int oldCharges;

  oldCharges = getCurrentCharges();            
  if(oldCharges <= 1){
    // The object is totally consummed
    return true;    
  }
  setCurrentCharges(oldCharges - 1);

  // Compute initial weight to be able to compute new weight
  // (without increasing error each time)

  f1 = getWeight();
  f1 *= (float) (getMaxCharges());
  f1 /= (float) oldCharges;
  f1 *= (((float)oldCharges - 1.0f) / (float)(getMaxCharges()));            
  setWeight(f1);
  return false;      
}




void Item::commonInit( bool loading ) {

  // --------------
  // regular attribs

  weight = rpgItem->getWeightRpg();
  quality = rpgItem->getQualityRpg();

  int basePrice = ( this->spell ? this->spell->getExp() : rpgItem->getPriceRpg() );
  price = basePrice + 
    (int)Util::getRandomSum( (float)(basePrice / 2), level );

  //action = (int)Util::getRandomSum( (float)(rpgItem->getActionRpg()), level / 2 );  

  if( rpgItem->getSpeedRpg() ) {
    speed = rpgItem->getSpeedRpg() - (int)Util::getRandomSum( 1, level / 7 );
    if( speed < 3 ) speed = 3;
  } else {
    speed = rpgItem->getSpeedRpg();
  }

  if( rpgItem->isRangedWeapon() ) {
    distance = rpgItem->getDistanceRpg() + 
      (int)Util::getRandomSum( 1, level / 4 );
  } else distance = rpgItem->getDistanceRpg();

  if( rpgItem->getMaxChargesRpg() ) {
    maxCharges = rpgItem->getMaxChargesRpg() + 
      (int)Util::getRandomSum( (float)(rpgItem->getMaxChargesRpg() / 2), level / 4 );
  } else maxCharges = rpgItem->getMaxChargesRpg();

  if( rpgItem->getDurationRpg() ) {
    duration = rpgItem->getDurationRpg() + 
      (int)Util::getRandomSum( (float)( rpgItem->getDurationRpg() / 2 ), level / 4 );
  } else duration = rpgItem->getDurationRpg();

  // assign a spell to the item
  // the deeper you go, the more likely that items contain spells
  if( RpgItem::itemTypes[ rpgItem->getType() ].hasSpell &&
      0 == (int)( (float)( MAX_MISSION_DEPTH - ( session->getCurrentMission() ? session->getCurrentMission()->getDepth() : 0 ) ) * rand() / RAND_MAX ) ) {
    this->spell = MagicSchool::getRandomSpell( 1 );
    price += (int)Util::getRandomSum( (float)(basePrice / 2), this->spell->getLevel() );
  }

  // --------------
  // magic attribs

  // init to no-magic
  magicLevel = -1;
  bonus = 0;
  damageMultiplier = 1;
  monsterType = NULL;
  cursed = false;
  school = NULL;
  magicDamage = NULL;
  stateModSet = false;
  for(int i = 0; i < Constants::STATE_MOD_COUNT; i++) stateMod[i] = 0;

  if( rpgItem->isEnchantable() && !loading ) {
    // roll for magic
    int n = (int)( ( 200.0f - ( level * 1.5f ) ) * rand()/RAND_MAX );
    if( n < 5 ) enchant( Constants::DIVINE_MAGIC_ITEM );
    else if( n < 15 ) enchant( Constants::CHAMPION_MAGIC_ITEM );
    else if( n < 30 ) enchant( Constants::GREATER_MAGIC_ITEM );
    else if( n < 50 ) enchant( Constants::LESSER_MAGIC_ITEM );
  }

  // describe spell-holding items also
  if( magicLevel < 0 && RpgItem::itemTypes[ rpgItem->getType() ].hasSpell ) {
    describeMagic( itemName, rpgItem->getName() );
  }
}

void Item::enchant( int newMagicLevel ) {
  if( magicLevel != -1 ) return;

  magicLevel = newMagicLevel;

  // item level caps the magic level:
  // 0-9: lesser
  // 10-19: greater
  // 20-29: champion
  // 39+ : divine
  // This is so low level items won't be too powerful.
  int maxMagicLevel = level / 10;
  if( magicLevel > maxMagicLevel ) magicLevel = maxMagicLevel;

  cursed = ( !((int)( 20.0f * rand() / RAND_MAX )) );

  // adjust the price
  price *= ( magicLevel + 2 );

  int n;
  Spell *spell;
  switch(magicLevel) {
  case Constants::LESSER_MAGIC_ITEM:
    bonus = (int)(1.0f * rand()/RAND_MAX) + 1;
    if(rpgItem->isWeapon()) {
      damageMultiplier = (int)(2.0f * rand()/RAND_MAX) + 2;
      monsterType = (char*)Monster::getRandomMonsterType( level );
    }
    n = (int)(3.0f * rand()/RAND_MAX) + 2;
    for(int i = 0; i < n; i++) {
      int skill = Constants::getRandomBasicSkill();
      if(skillBonus.find(skill) == skillBonus.end()) {
        skillBonus[skill] = (int)(5.0f * rand()/RAND_MAX) + 1;
      }
    }    
    break;
  case Constants::GREATER_MAGIC_ITEM:
    bonus = (int)(2.0f * rand()/RAND_MAX) + 1;
    if(rpgItem->isWeapon()) {
      damageMultiplier = (int)(3.0f * rand()/RAND_MAX) + 2;
      monsterType = (char*)Monster::getRandomMonsterType( level );
    }
    spell = MagicSchool::getRandomSpell(1);
    if(spell) {
      school = spell->getSchool();
      magicDamage = new Dice(1, (int)(3.0f * rand()/RAND_MAX) + 1, (int)(3.0f * rand()/RAND_MAX));
    }
    n = (int)(3.0f * rand()/RAND_MAX) + 2;
    for(int i = 0; i < n; i++) {
      int skill = Constants::getRandomBasicSkill();
      if(skillBonus.find(skill) == skillBonus.end()) {
        skillBonus[skill] = (int)(8.0f * rand()/RAND_MAX) + 1;
      }
    }
    break;
  case Constants::CHAMPION_MAGIC_ITEM:
    bonus = (int)(3.0f * rand()/RAND_MAX) + 1;
    if(rpgItem->isWeapon()) {
      damageMultiplier = (int)(3.0f * rand()/RAND_MAX) + 2;
      monsterType = (char*)Monster::getRandomMonsterType( level );
    }
    spell = MagicSchool::getRandomSpell(1);
    if(spell) {
      school = spell->getSchool();
      magicDamage = new Dice(1, (int)(3.0f * rand()/RAND_MAX) + 2, (int)(3.0f * rand()/RAND_MAX));
    }
    n = (int)(3.0f * rand()/RAND_MAX) + 1;
    if(n > 0) stateModSet = true;
    for(int i = 0; i < n; i++) {
      stateMod[Constants::getRandomGoodStateMod()] = 1;
    }
    n = (int)(3.0f * rand()/RAND_MAX) + 1;
    for(int i = 0; i < n; i++) {
      int skill = Constants::getRandomBasicSkill();
      if(skillBonus.find(skill) == skillBonus.end()) {
        skillBonus[skill] = (int)(10.0f * rand()/RAND_MAX) + 1;
      }
    }
    break;
  case Constants::DIVINE_MAGIC_ITEM:
    bonus = (int)(3.0f * rand()/RAND_MAX) + 2;
    if(rpgItem->isWeapon()) {
      damageMultiplier = (int)(4.0f * rand()/RAND_MAX) + 2;
      monsterType = (char*)Monster::getRandomMonsterType( level );
    }
    spell = MagicSchool::getRandomSpell(1);
    if(spell) {
      school = spell->getSchool();
      magicDamage = new Dice(1, (int)(3.0f * rand()/RAND_MAX) + 3, (int)(3.0f * rand()/RAND_MAX));
    }
    n = (int)(3.0f * rand()/RAND_MAX) + 1;
    if(n > 0) stateModSet = true;
    for(int i = 0; i < n; i++) {
      stateMod[Constants::getRandomGoodStateMod()] = 1;
    }
    n = (int)(3.0f * rand()/RAND_MAX) + 1;
    if(n > 0) stateModSet = true;
    for(int i = 0; i < n; i++) {
      stateMod[Constants::getRandomBadStateMod()] = 2;
    }
    n = (int)(3.0f * rand()/RAND_MAX) + 2;
    for(int i = 0; i < n; i++) {
      int skill = Constants::getRandomBasicSkill();
      if(skillBonus.find(skill) == skillBonus.end()) {
        skillBonus[skill] = (int)(12.0f * rand()/RAND_MAX) + 1;
      }
    }
    break;
  default:
    cerr << "*** Error: unknown magic level: " << magicLevel << endl;
  }

  // turn off "vs. any creature"
  if( !monsterType ) damageMultiplier = 1;

  describeMagic(itemName, rpgItem->getName());
}

// max about 30 points (must be deterministic)
int Item::getMagicResistance() { 
  return( 3 * ( ( getLevel() / 10 ) + getMagicLevel() ) ); 
}

void Item::describeMagic(char *s, char *itemName) {
  // e.g.: Lesser broadsword + 3 of nature magic
  char tmp[80];

  strcpy( s, "" );

  // Stored spell
  if( RpgItem::itemTypes[ rpgItem->getType() ].hasSpell && spell ) {
    strcat( s, spell->getSymbol() );
    strcat( s, " " );
  }

  // Lesser, Greater, etc.
  if( magicLevel > -1 ) {
    strcat( s, Constants::MAGIC_ITEM_NAMES[ magicLevel ] );

    // Protective if stateMods are changed
    if( stateModSet ) {
      strcat( s, " Protective" );
    }
  
    // Slaying if there is a multiplier
    if( damageMultiplier > 1 ) {
      strcat( s, " Slaying" );
    }
      
    strcat(s, " ");
  }

  // the item's name
  strcat(s, itemName);
  
  // the bonus
  if(bonus > 0) {
    sprintf(tmp, " (+%d)", bonus);
    strcat(s, tmp);
  }
  
  // Describe the item.
  // (this code has to be deterministic b/c it's called by 'load' also)
  if( skillBonus.size() > 0 ) {
    strcat( s, " of the " );

    // use state_mod or magic school as the adjective
    // e.g.: of the [ice|dire|planar|etc] Boar
    if( school ) {
      sprintf( tmp, "%s ", school->getSymbol() );
      strcat( s, tmp );
    } else if( stateModSet ) {
      for( int i = 0; i < Constants::STATE_MOD_COUNT; i++ ) {
        if( stateMod[ i ] > 0 ) {
          sprintf( tmp, "%s ", Constants::STATE_SYMBOLS[ i ] );
          strcat( s, tmp );
          break;
        }
      }
    }

    // use the first skill as the noun
    map<int,int>::iterator i = skillBonus.begin();
    int skill = i->first;
    sprintf( tmp, "%s", Constants::SKILL_SYMBOL[ skill ] );
    strcat( s, tmp );
  }  
}

bool Item::isSpecial() { 
  return getRpgItem()->isSpecial(); 
}

int Item::rollMagicDamage() { 
  return (magicDamage ? magicDamage->roll() : 0); 
}

char *Item::describeMagicDamage() { 
  return (magicDamage ? magicDamage->toString() : NULL);
}

void Item::debugMagic(char *s) {
  RpgItem *item = getRpgItem();
  cerr << s << endl;
  cerr << "Magic item: " << item->getName() << "(+" << bonus << ")" << endl;
  cerr << "\tdamageMultiplier=" << damageMultiplier << " vs. monsterType=" << (monsterType ? monsterType : "null") << endl;
  cerr << "\tSchool: " << (school ? school->getName() : "null") << endl;
  cerr << "\tstate mods:" << endl;
  for(int i = 0; i < Constants::STATE_MOD_COUNT; i++) {
    if(this->isStateModSet(i)) cerr << "set: " << Constants::STATE_NAMES[i] << endl;
    if(this->isStateModProtected(i)) cerr << "protected: " << Constants::STATE_NAMES[i] << endl;
  }
  cerr << "\tskill bonuses:" << endl;
  for(map<int, int>::iterator i=skillBonus.begin(); i!=skillBonus.end(); ++i) {
    int skill = i->first;
    int bonus = i->second;
    cerr << "\t\t" << Constants::SKILL_NAMES[skill] << " +" << bonus << endl;
  }
  cerr << "-----------" << endl;
}

void Item::setCurrentCharges( int n ) { 
  if( n < 0 ) n=0; 
  if( n>rpgItem->getMaxChargesRpg() )
    n = rpgItem->getMaxChargesRpg(); 
  currentCharges = n; 
} 

void Item::setSpell( Spell *spell ) { 
  this->spell = spell; 
  sprintf( this->itemName, "Scroll of %s", spell->getName() ); 
}

