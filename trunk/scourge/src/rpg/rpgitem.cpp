/***************************************************************************
                          rpgitem.cpp  -  description
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
#include "rpgitem.h"
#include "spell.h"
#include "monster.h"


/**
   These basic objects are enhanced by adding magical capabilities.
 */

/*
 * FIXME: Make this property-driven
 *
 * Remember to change isWeaponItem=, getRandomEnchantableItem, 
 * getRandomItem
 * 
 * when adding a new item or type.
 */
RpgItem *RpgItem::items[1000];

map<int, map<int, vector<const RpgItem*>*>*> RpgItem::typesMap;
map<string, const RpgItem *> RpgItem::itemsByName;
vector<RpgItem*> RpgItem::containers;
vector<RpgItem*> RpgItem::containersNS; 

int RpgItem::itemCount = 0;

char RpgItem::itemTypeStr[ITEM_TYPE_COUNT][40] = {
  "SWORD",
  "AXE",
  "BOW",
  "CONTAINER",
  "ARMOR",
  "FOOD",
  "DRINK",
  "POTION",
  "OTHER",
  "MISSION",
  "SCROLL"
};

int RpgItem::enchantableTypes[] = { SWORD, AXE, BOW, ARMOR };
int RpgItem::enchantableTypeCount = 4;

RpgItem::RpgItem(int index, char *name, int level, int rareness, int type, float weight, int price, int quality, 
				 int action, int speed, char *desc, char *shortDesc, int equip, int shape_index, 
				 int twohanded, int distance, int skill, int maxCharges, int potionSkill,
				 int potionTime) {
  this->index = index;
  this->name = name;
  this->level = level;
  this->rareness = rareness;
  this->type = type;
  this->weight = weight;
  this->price = price;
  this->quality = quality;
  this->action = action;
  this->speed = speed;
  this->desc = desc;
  this->shortDesc = shortDesc;
  this->shape_index = shape_index;
  this->equip = equip;
  this->twohanded = twohanded;
  this->distance = distance;
  this->skill = skill;
  this->maxCharges = maxCharges;
  this->potionSkill = potionSkill;
  this->potionTime = potionTime;
  this->acl = (GLuint)0xffff; // all on
  this->isWeaponItem = (type == SWORD || type == AXE || type == BOW);
}

RpgItem::~RpgItem() {
}

void RpgItem::addItem(RpgItem *item, int width, int depth, int height) {
  // store the item
  cerr << "adding item: " << item->name << 
	" level=" << item->level << 
	" type=" << item->type << endl;
  items[itemCount++] = item;

  // create the level map
  map<int, vector<const RpgItem*>*> *levelMap = NULL;
  if(typesMap.find(item->type) != typesMap.end()){
	levelMap = typesMap[(const int)(item->type)];
  } else {
	levelMap = new map<int, vector<const RpgItem*>*>();
	typesMap[item->type] = levelMap;
  }
  //  cerr << "\ttypesMap.size()=" << typesMap.size() << endl;

  // get a non-pointer ref to the level map
  //  map<int, vector<const RpgItem*>*> map = *typesMap[item->type];
  // create the vector
  vector<const RpgItem*> *list = NULL;
  if(levelMap->find(item->level) != levelMap->end()) {
	list = (*levelMap)[(const int)(item->level)];
  } else {
	list = new vector<const RpgItem*>();
	(*levelMap)[item->level] = list;
  }
  //  cerr << "\tlevelMap.size()=" << levelMap->size() << endl;
  list->push_back(item);
  //  cerr << "\tlist.size()=" << list->size() << endl;

  // remember by name
  string s = item->name;
  itemsByName[s] = item;
  //  cerr << "*** Stored name=>" << item->name << "< item=" << item << endl;

  // HACK: do not include "corpse" as a valid container...
  // It should really have a container exclusion flag.
  if(item->type == CONTAINER && strcmp(item->name, "Corpse"))  {
    if(width >= depth) {
      containersNS.push_back(item);
    }
    if(width <= depth) {
      containers.push_back(item);
    }
  }
}

int RpgItem::getTypeByName(char *name) {
  for(int i = 0; i < ITEM_TYPE_COUNT; i++) {
	if(!strcmp(itemTypeStr[i], name)) return i;
  }
  cerr << "Can't find type >" << name << endl;
  exit(1);
}

RpgItem *RpgItem::getRandomEnchantableItem(int level) {
  return getRandomItemFromTypes(level, enchantableTypes, enchantableTypeCount);
}

bool RpgItem::isEnchantable() {
  for(int i = 0; i < enchantableTypeCount; i++) {
    if(getType() == enchantableTypes[i]) return true;
  }
  return false;
}

RpgItem *RpgItem::getRandomItem(int maxLevel) {
  // item types found in the lying around a dungeon
  int types[] = { SWORD, AXE, BOW, ARMOR, FOOD, DRINK, POTION };
  int typeCount = 7;
  return getRandomItemFromTypes(maxLevel, types, typeCount);
}




RpgItem *RpgItem::getRandomItemFromTypes(int maxLevel, int types[], int typeCount) {
// levels are assumed to be 1-based
  if(maxLevel < 1) {
    cerr << "levels are assumed to be 1-based!!!" << endl;
    exit(1);
  }

  // choose a random level up to maxLevel
  int level = (int)((float)maxLevel * rand()/RAND_MAX) + 1;

  int typeIndex = (int)((float)typeCount * rand()/RAND_MAX);
  map<int, vector<const RpgItem*>*> *levelMap = typesMap[types[typeIndex]];
  if(levelMap && levelMap->size()) {
    vector<const RpgItem*> *list = (*levelMap)[level];

    if(list && list->size()) {

      // create a new list where each item occurs item->rareness times
      vector<RpgItem*> rareList;
      for(int i = 0; i < (int)list->size(); i++) {
        RpgItem *item = (RpgItem*)(*list)[i];
        for(int t = 0; t < item->getRareness(); t++) {
          rareList.push_back(item);
        }
      }

      int n = (int)((float)(rareList.size()) * rand()/RAND_MAX);
      return(RpgItem*)rareList[n];
    }
  }
  return NULL;
}

RpgItem *RpgItem::getRandomContainer() {
  int n = (int)((3.0f * (float)containers.size()) * rand()/RAND_MAX);
  if(n >= (int)containers.size()) return NULL;
  return containers[n];
}

RpgItem *RpgItem::getRandomContainerNS() {
  int n = (int)((3.0f * (float)containersNS.size()) * rand()/RAND_MAX);
  if(n >= (int)containersNS.size()) return NULL;
  return containersNS[n];
}

RpgItem *RpgItem::getItemByName(char *name) {
  string s = name;
  RpgItem *item = (RpgItem *)itemsByName[s];
  //  cerr << "*** Looking for >" << s << "< found=" << item << endl;
  return item;
}















MagicAttrib::MagicAttrib() {
  level = 0;
  bonus = 0;
  damageMultiplier = 1;
  monsterType = NULL;
  cursed = false;
  school = NULL;
  magicDamage = NULL;
  stateModSet = false;
  for(int i = 0; i < Constants::STATE_MOD_COUNT; i++)
    stateMod[i] = 0;
}

MagicAttrib::~MagicAttrib() {
  if(!magicDamage) delete magicDamage;
}

void MagicAttrib::describe(char *s, char *itemName) {
  // e.g.: Lesser broadsword + 3 of nature magic
  char tmp[80];
  strcpy(s, Constants::MAGIC_ITEM_NAMES[level]);
  if(stateModSet) {
    strcat(s, " protective");
  }
  strcat(s, " ");
  strcat(s, itemName);
  if(bonus > 0) {
    sprintf(tmp, " (+%d)", bonus);
    strcat(s, tmp);
  }
  if(school) {
    sprintf(tmp, " of %s magic", school->getShortName());
    strcat(s, tmp);
  }
}

void MagicAttrib::enchant(int level, bool isWeapon) {
  if(level < Constants::LESSER_MAGIC_ITEM) level = Constants::LESSER_MAGIC_ITEM;
  if(level > Constants::DIVINE_MAGIC_ITEM) level = Constants::DIVINE_MAGIC_ITEM;
  this->level = level;

  /**
   * lesser (level 0):
   * bonus, damageMultiplier vs. a monster type
   * 
   * greater (level 1):
   * lesser + higher bonus + magic damage + some basic skills enhanced
   * 
   * champion (level 2):
   * greater + higher magic damage + (1 to 3) good state mods set
   * 
   * divine (level 3):
   * champion + monster_type is NULL + (1 to 3) bad state mods protected
   */
  int n;
  Spell *spell;
  switch(level) {
  case Constants::LESSER_MAGIC_ITEM:
    bonus = (int)(1.0f * rand()/RAND_MAX) + 1;
    if(isWeapon) {
      damageMultiplier = (int)(2.0f * rand()/RAND_MAX);
      monsterType = (char*)Monster::getRandomMonsterType();
    }
    break;
  case Constants::GREATER_MAGIC_ITEM:
    bonus = (int)(2.0f * rand()/RAND_MAX) + 1;
    if(isWeapon) {
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
    if(isWeapon) {
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
    if(isWeapon) {
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
    cerr << "*** Error: unknown magic attrib level: " << level << endl;
  }
}

int MagicAttrib::rollMagicDamage() { 
  return (magicDamage ? magicDamage->roll() : 0); 
}

char *MagicAttrib::describeMagicDamage() { 
  return (magicDamage ? magicDamage->toString() : NULL);
}

void MagicAttrib::debug(char *s, RpgItem *item) {
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
