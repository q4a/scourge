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
#include "session.h"

map<int, vector<string> *> Item::soundMap;

Item::Item(RpgItem *rpgItem, int level) {
  this->rpgItem = rpgItem;
  this->level = level;
  this->shapeIndex = this->rpgItem->getShapeIndex();
  this->color = NULL;
  this->shape = ShapePalette::getInstance()->getShape(shapeIndex);
  // for now objects larger than 1 height will block (we can change this later)
  // this is so the player is not blocked by swords and axes on the ground
  this->blocking = (shape->getHeight() > 1 || 
                    rpgItem->getType() == RpgItem::CONTAINER ||
                    rpgItem->getType() == RpgItem::MISSION);
  this->containedItemCount = 0;
  currentCharges = rpgItem->getMaxCharges();
  weight = rpgItem->getWeight();
  this->spell = NULL;
  this->magic = NULL;
  sprintf(this->itemName, "%s", rpgItem->getName());

  commonInit();
}

Item::~Item(){
  if(magic) {
    delete magic;
    magic = NULL;
  }
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
  strcpy((char*)info->spell_name, (spell ? spell->getName() : ""));
  info->containedItemCount = containedItemCount;
  for(int i = 0; i < containedItemCount; i++) {
    info->containedItems[i] = containedItems[i]->save();
  }
  info->magic = (magic ? magic->save() : MagicAttrib::saveEmpty());
  return info;
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
  Item *item = session->newItem( RpgItem::getItemByName( (char*)info->rpgItem_name ), 
                                 info->level, 
                                 spell);
  item->blocking = (info->blocking == 1);
  item->currentCharges = info->currentCharges;
  item->weight = (float)(info->weight) / 100.0f;
  item->containedItemCount = info->containedItemCount;
  for(int i = 0; i < (int)info->containedItemCount; i++) {
    item->containedItems[i] = Item::load( session, info->containedItems[i] );
  }    
  MagicAttrib *magic = MagicAttrib::load( session, info->magic );
  if( magic ) {
    item->magic = magic;
    magic->describe(item->itemName, item->getRpgItem()->getName());
  }
  return item;
}

bool Item::addContainedItem(Item *item, bool force) { 
  if(containedItemCount < MAX_CONTAINED_ITEMS && 
     (force || !item->isBlocking() || getShape()->fitsInside(item->getShape()))) {
    containedItems[containedItemCount++] = item; 
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
  if(type == RpgItem::DRINK || type == RpgItem::POTION || type == RpgItem::FOOD){
    sprintf(s, "(Q:%d,W:%2.2f, N:%d/%d) %s", 
            rpgItem->getQuality(), 
            rpgItem->getWeight(),
            getCurrentCharges(),
            rpgItem->getMaxCharges(),
            (precise ? itemName : rpgItem->getShortDesc()));
  } else if(type == RpgItem::SCROLL) {
    sprintf(s, "%s", itemName);
  } else {
    sprintf(s, "(A:%d,S:%d,Q:%d,W:%2.2f) %s", 
            rpgItem->getAction(), 
            rpgItem->getSpeed(), 
            rpgItem->getQuality(), 
            rpgItem->getWeight(),
            (precise ? itemName : rpgItem->getShortDesc()));
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
    if(n == 'I') {
      // skip ':'
      fgetc(fp);
      // read the rest of the line
      n = Constants::readLine(name, fp);
      n = Constants::readLine(line, fp);
      int level = atoi(strtok(line + 1, ","));
      int rareness = atoi(strtok(NULL, ","));
      char *p = strtok(NULL, ",");
      int action = 0;
      int speed = 0;
      int distance = 0;
      int maxCharges = 0;
      if(p) {
        action = atoi(p);
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
      p = strtok(NULL, ",");    
      if(p) {
        strcpy(shape, p);
        p = strtok(NULL, ",");
        if(p) {
          inventory_location = atoi(p);
          twohanded = atoi(strtok(NULL, ","));
          strcpy(skill, strtok(NULL, ","));
        }
      }

      n = Constants::readLine(line, fp);
      strcpy(long_description, line + 1);

      n = Constants::readLine(line, fp);
      strcpy(short_description, line + 1);

      // resolve strings
      int type_index = RpgItem::getTypeByName(type);    
      cerr << "item: looking for shape: " << shape << endl;
      int shape_index = shapePal->findShapeIndexByName(shape);
      cerr << "\tindex=" << shape_index << endl;
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
        cerr << "**** potionSkill=" << potionSkill << " potion_skill=" << potion_skill << endl;
      }
      if(distance < (int)Constants::MIN_DISTANCE) distance = (int)Constants::MIN_DISTANCE;
      last = new RpgItem(itemCount++, strdup(name), level, rareness, type_index, 
                         weight, price, 100, 
                         action, speed, strdup(long_description), 
                         strdup(short_description), 
                         inventory_location, shape_index, 
                         twohanded, 
                         (distance < (int)Constants::MIN_DISTANCE ? 
                          (int)Constants::MIN_DISTANCE : distance), 
                         skill_index, maxCharges, potion_skill, potionTime);
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
  f1 *= (float) (getRpgItem()->getMaxCharges());
  f1 /= (float) oldCharges;
  f1 *= (((float)oldCharges - 1.0f) / (float)(getRpgItem()->getMaxCharges()));            
  setWeight(f1);
  return false;      
}

void Item::enchant(int level) {
  if(magic) return;
  magic = new MagicAttrib();
  magic->enchant(level, rpgItem->isWeapon());
  magic->describe(itemName, rpgItem->getName());
  return;
}



void Item::commonInit() {

  // --------------
  // regular attribs

  //  weight = rpgItem->getWeight() - ( level * ( rpgItem->getWeight() / (float)MAX_LEVEL ) );
  weight = rpgItem->getWeight();
  price = rpgItem->getPrice() + (int)((float)( rpgItem->getPrice() * ( level / 2 ) ) * 
                                      rand() / RAND_MAX );
  action = rpgItem->getAction() + (int)((float)( rpgItem->getAction() * ( level / 2 ) ) * 
                                        rand() / RAND_MAX );
  speed = rpgItem->getSpeed() - (int)((float)( rpgItem->getSpeed() * ( level / 10 ) ) * 
                                      rand() / RAND_MAX );
  if( speed < 3 ) speed = 3;
  distance = rpgItem->getDistance() + (int)((float)( rpgItem->getDistance() * ( level / 2 ) ) * 
                                            rand() / RAND_MAX );
  maxCharges = rpgItem->getMaxCharges() + (int)((float)( rpgItem->getMaxCharges() * ( level / 2 ) ) * 
                                                rand() / RAND_MAX );
  duration = rpgItem->getDuration() + (int)((float)( rpgItem->getDuration() * ( level / 2 ) ) * 
                                            rand() / RAND_MAX );


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

  if( !rpgItem->isEnchantable() ) return;

  // roll for magic
  int n = (int)( 100.0f * rand()/RAND_MAX );
  if( n < 5 ) magicLevel = Constants::DIVINE_MAGIC_ITEM;
  else if( n < 15 ) magicLevel = Constants::CHAMPION_MAGIC_ITEM;
  else if( n < 30 ) magicLevel = Constants::GREATER_MAGIC_ITEM;
  else if( n < 50 ) magicLevel = Constants::LESSER_MAGIC_ITEM;

  if( magicLevel == -1 ) return;


  Spell *spell;
  switch(magicLevel) {
  case Constants::LESSER_MAGIC_ITEM:
    bonus = (int)(1.0f * rand()/RAND_MAX) + 1;
    if(rpgItem->isWeapon()) {
      damageMultiplier = (int)(2.0f * rand()/RAND_MAX);
      monsterType = (char*)Monster::getRandomMonsterType();
    }
    break;
  case Constants::GREATER_MAGIC_ITEM:
    bonus = (int)(2.0f * rand()/RAND_MAX) + 1;
    if(rpgItem->isWeapon()) {
      damageMultiplier = (int)(3.0f * rand()/RAND_MAX);
      monsterType = (char*)Monster::getRandomMonsterType();
    }
    spell = MagicSchool::getRandomSpell(1);
    if(spell) {
      school = spell->getSchool();
      magicDamage = new Dice(1, (int)(3.0f * rand()/RAND_MAX) + 1, (int)(3.0f * rand()/RAND_MAX));
    }
    n = (int)(3.0f * rand()/RAND_MAX);
    for(int i = 0; i < n; i++) {
      int skill = Constants::getRandomBasicSkill();
      if(skillBonus.find(skill) == skillBonus.end()) {
        skillBonus[skill] = (int)(8.0f * rand()/RAND_MAX);
      }
    }
    break;
  case Constants::CHAMPION_MAGIC_ITEM:
    bonus = (int)(3.0f * rand()/RAND_MAX) + 1;
    if(rpgItem->isWeapon()) {
      damageMultiplier = (int)(3.0f * rand()/RAND_MAX);
      monsterType = (char*)Monster::getRandomMonsterType();
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
        skillBonus[skill] = (int)(10.0f * rand()/RAND_MAX);
      }
    }
    break;
  case Constants::DIVINE_MAGIC_ITEM:
    bonus = (int)(3.0f * rand()/RAND_MAX) + 2;
    if(rpgItem->isWeapon()) {
      damageMultiplier = (int)(4.0f * rand()/RAND_MAX);
      monsterType = NULL;
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
        skillBonus[skill] = (int)(12.0f * rand()/RAND_MAX);
      }
    }
    break;
  default:
    cerr << "*** Error: unknown magic level: " << magicLevel << endl;
  }
}

int Item::rollMagicDamage() { 
  return (magicDamage ? magicDamage->roll() : 0); 
}

char *Item::describeMagicDamage() { 
  return (magicDamage ? magicDamage->toString() : NULL);
}
