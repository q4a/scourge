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
  "MACE",
  "CONTAINER",
  "ARMOR",
  "FOOD",
  "DRINK",
  "POTION",
  "OTHER",
  "MISSION",
  "SCROLL"
};

int RpgItem::enchantableTypes[] = { SWORD, AXE, BOW, MACE, ARMOR };
int RpgItem::enchantableTypeCount = 5;

RpgItem::RpgItem(int index, char *name, int level, int rareness, int type, float weight, int price, int quality, 
				 int action, int speed, char *desc, char *shortDesc, int equip, int shape_index, 
				 int twohanded, int distance, int skill, int maxCharges, int potionSkill,
				 int potionTime, int iconTileX, int iconTileY) {
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
  this->isWeaponItem = ( type == SWORD || type == AXE || type == BOW || type == MACE );
  this->iconTileX = iconTileX;
  this->iconTileY = iconTileY;
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
  int types[] = { SWORD, AXE, BOW, MACE, ARMOR, FOOD, DRINK, POTION };
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
