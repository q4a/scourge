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

/**
   These basic objects are enhanced by adding magical capabilities.
 */

RpgItem *RpgItem::items[1000];

map<int, map<int, vector<const RpgItem*>*>*> RpgItem::typesMap;
map<string, const RpgItem *> RpgItem::itemsByName;

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
  "MISSION"
};

RpgItem::RpgItem(int index, char *name, int level, int type, float weight, int price, int quality, 
				 int action, int speed, char *desc, char *shortDesc, int equip, int shape_index, 
				 int twohanded, int distance, int skill, int maxCharges) {
  this->index = index;
  this->name = name;
  this->level = level;
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
}

RpgItem::~RpgItem() {
}

void RpgItem::addItem(RpgItem *item) {
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
}

int RpgItem::getTypeByName(char *name) {
  for(int i = 0; i < ITEM_TYPE_COUNT; i++) {
	if(!strcmp(itemTypeStr[i], name)) return i;
  }
  cerr << "Can't find type >" << name << endl;
  exit(1);
}

RpgItem *RpgItem::getRandomItem(int maxLevel) {
  // item types found in the lying around a dungeon
  int types[] = { SWORD, AXE, BOW, ARMOR, FOOD, DRINK };
  int typeCount = 6;

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
	  int n = (int)((float)((*list).size()) * rand()/RAND_MAX);
	  return (RpgItem*)(*list)[n];
	}
  }
  return NULL;
}

RpgItem *RpgItem::getRandomContainer() {
  int n = (int)(10.0 * rand()/RAND_MAX);
  switch(n) {
  case 0: return getItemByName("Bookshelf");
  case 1: return getItemByName("Chest");
  default: return NULL;
  }
}

RpgItem *RpgItem::getRandomContainerNS() {
  int n = (int)(10.0 * rand()/RAND_MAX);
  switch(n) {
  case 0: return getItemByName("Bookshelf2");
  case 1: return getItemByName("Chest2");
  default: return NULL;
  }
}

RpgItem *RpgItem::getItemByName(char *name) {
  string s = name;
  RpgItem *item = (RpgItem *)itemsByName[s];
  //  cerr << "*** Looking for >" << s << "< found=" << item << endl;
  return item;
}
