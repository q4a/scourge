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

using namespace std;


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
vector<RpgItem*> RpgItem::special;

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
  "SCROLL",
  "SHIELD",
  "POLE",
};

int RpgItem::enchantableTypes[] = { SWORD, AXE, BOW, MACE, ARMOR };
int RpgItem::enchantableTypeCount = 5;

RpgItem::RpgItem(int index, char *name, int level, int rareness, int type, float weight, int price, int quality, 
                 Dice *action, int speed, char *desc, char *shortDesc, int equip, int shape_index, 
                 int twohanded, int distance, int skill, int minDepth, int minLevel, 
                 int maxCharges, int potionSkill,
                 int potionTime, int iconTileX, int iconTileY, int maxSkillBonus) {
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
  this->minDepth = minDepth;
  this->minLevel = minLevel;
  this->maxCharges = maxCharges;
  this->potionSkill = potionSkill;
  this->potionTime = potionTime;
  this->acl = (GLuint)0xffff; // all on
  this->isWeaponItem = ( type == SWORD || type == AXE || type == BOW || type == MACE || type == POLE );
  this->iconTileX = iconTileX;
  this->iconTileY = iconTileY;
  this->maxSkillBonus = maxSkillBonus;
}

RpgItem::~RpgItem() {
}

void RpgItem::addItem(RpgItem *item, int width, int depth, int height) {
  // store the item
  /*
cerr << "adding item: " << item->name << 
" level=" << item->level << 
" type=" << item->type << endl;
  */
  items[itemCount++] = item;

  if( !item->isSpecial() ) {
    // Add item to type->depth maps
    map<int, vector<const RpgItem*>*> *depthMap = NULL;
    if (typesMap.find(item->type) != typesMap.end()) {
      depthMap = typesMap[(const int)(item->type)];
    } else {
      depthMap = new map<int, vector<const RpgItem*>*>();
      typesMap[item->type] = depthMap;
    }
    //  cerr << "\ttypesMap.size()=" << typesMap.size() << endl;
  
    // create the min depth map: Add item to every depth level >= item->getMinDepth()
    for( int currentDepth = item->getMinDepth(); currentDepth < MAX_MISSION_DEPTH; currentDepth++ ) {
      // create the vector
      vector<const RpgItem*> *list = NULL;
      if (depthMap->find( currentDepth ) != depthMap->end()) {
        list = (*depthMap)[(const int)( currentDepth )];
      } else {
        list = new vector<const RpgItem*>();
        (*depthMap)[ currentDepth ] = list;
      }
      //  cerr << "\tlevelMap.size()=" << levelMap->size() << endl;
      list->push_back(item);
      //  cerr << "\tlist.size()=" << list->size() << endl;
    }
  } else {
    special.push_back( item );
  }

  // remember by name
  string s = item->name;
  itemsByName[s] = item;
  //  cerr << "*** Stored name=>" << item->name << "< item=" << item << endl;

  // HACK: do not include "corpse" as a valid container...
  // It should really have a container exclusion flag.
  if (item->type == CONTAINER && strcmp(item->name, "Corpse")) {
    if (width >= depth) {
      containersNS.push_back(item);
    }
    if (width <= depth) {
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

bool RpgItem::isEnchantable() {
  for(int i = 0; i < enchantableTypeCount; i++) {
    if(getType() == enchantableTypes[i]) return true;
  }
  return false;
}

RpgItem *RpgItem::getRandomItem(int depth) {
  // item types found in the lying around a dungeon
  int types[] = { SWORD, AXE, BOW, MACE, ARMOR, FOOD, DRINK, POTION, POLE };
  int typeCount = 9;
  return getRandomItemFromTypes(depth, types, typeCount);
}

RpgItem *RpgItem::getRandomItemFromTypes(int depth, int types[], int typeCount) {

  if( depth < 0 ) depth = 0;
  if( depth >= MAX_MISSION_DEPTH ) depth = MAX_MISSION_DEPTH - 1;

  int typeIndex = (int)((float)typeCount * rand()/RAND_MAX);
  map<int, vector<const RpgItem*>*> *depthMap = typesMap[types[typeIndex]];
  if(depthMap && depthMap->size()) {

    // Select this depth's list of items. Since an item is on every list
    // greater than equal to its min. depth, we don't have to roll for
    // a depth value. (Eg.: a item w. minDepth=2 will be on lists 2-10.
    vector<const RpgItem*> *list = (*depthMap)[depth];

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
      RpgItem *rpgItem = (RpgItem*)rareList[n];
      return rpgItem;
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

// warning: slow method (use in editor only)
bool RpgItem::isContainer() {
  for( int i = 0; i < (int)containers.size(); i++ ) {
    if( containers[i] == this ) return true;
  }
  for( int i = 0; i < (int)containersNS.size(); i++ ) {
    if( containersNS[i] == this ) return true;
  }
  return false;
}

