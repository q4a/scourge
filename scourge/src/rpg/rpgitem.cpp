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

RpgItem *RpgItem::items[1000];

map<int, map<int, vector<const RpgItem*>*>*> RpgItem::typesMap;
map<string, const RpgItem *> RpgItem::itemsByName;
vector<RpgItem*> RpgItem::containers;
vector<RpgItem*> RpgItem::containersNS; 
vector<RpgItem*> RpgItem::special;
int RpgItem::itemCount = 0;
std::vector<ItemType> RpgItem::itemTypes;
int RpgItem::randomTypes[ITEM_TYPE_COUNT];
int RpgItem::randomTypeCount = 0;
std::map<std::string,std::string> RpgItem::tagsDescriptions;
char *RpgItem::DAMAGE_TYPE_NAME[] = { "Slashing", "Piercing", "Crushing" };
char RpgItem::DAMAGE_TYPE_LETTER[] = { 'S', 'P', 'C' };
char *RpgItem::influenceTypeName[] = { "AP", "CTH", "DAM" };

RpgItem::RpgItem( char *name, int rareness, int type, float weight, int price, 
									char *desc, char *shortDesc, int equip, int shape_index, 
									int minDepth, int minLevel, 
									int maxCharges,
									int iconTileX, int iconTileY ) {
  this->name = name;
  this->rareness = rareness;
  this->type = type;
  this->weight = weight;
  this->price = price;
  this->desc = desc;
  this->shortDesc = shortDesc;
  this->shape_index = shape_index;
  this->equip = equip;
  this->minDepth = minDepth;
  this->minLevel = minLevel;
  this->maxCharges = maxCharges;
  this->iconTileX = iconTileX;
  this->iconTileY = iconTileY;
  
	// initialize the rest to default values

	// weapon
	damage = damageType = damageSkill = parry = ap = range = twohanded = 0;
	for( int i = 0; i < Skill::SKILL_COUNT; i++ ) {
		for( int t = 0; t < INFLUENCE_TYPE_COUNT; t++ ) {
			for( int r = 0; r < INFLUENCE_LIMIT_COUNT; r++ ) {
				this->weaponInfluence[i][t][r].limit = -1;
				this->weaponInfluence[i][t][r].type = 'L';
				this->weaponInfluence[i][t][r].base = -1;
			}
		}
	}
  
	// armor
	defense = (int*)malloc( sizeof( int ) * DAMAGE_TYPE_COUNT );
	for( int i = 0; i < DAMAGE_TYPE_COUNT; i++ ) {
		defense[ i ] = 0;
	}
	defenseSkill = dodgePenalty = 0;
  
	// potion
	potionPower = potionSkill = potionTime = 0;

	// spells
	spellLevel = 0;
}

RpgItem::~RpgItem() {
	free( defense );
	free( name );
	free( desc );
	free( shortDesc );
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
    //cerr << "\ttypesMap.size()=" << typesMap.size() << " type=" << item->type << " (" << itemTypes[item->type].name << ")" << endl;
  
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
  for( int i = 0; i < (int)itemTypes.size(); i++ ) {
    if( !strcmp( itemTypes[ i ].name, name ) ) return i;
  }
  cerr << "Can't find type >" << name << "< in " << itemTypes.size() << endl;
  for( int i = 0; i < (int)itemTypes.size(); i++ ) {
    cerr << "\t" << itemTypes[ i ].name << endl;
  }
  exit(1);
}

RpgItem *RpgItem::getRandomItem(int depth) {
  return getRandomItemFromTypes( depth, randomTypes, randomTypeCount );
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

void RpgItem::describeTag( char *buffer, char *prefix, string tag, char *postfix, char *token ) {
	strcpy( buffer, prefix );
	if( RpgItem::tagsDescriptions.find( tag ) != RpgItem::tagsDescriptions.end() ) {
		char tmp[255];
		strcpy( tmp, tagsDescriptions[ tag ].c_str() );
		char *p = strtok( tmp, " " );
		while( p ) {
			if( !strcmp( p, "$$" ) ) strcat( buffer, token );
			else strcat( buffer, p );
			p = strtok( NULL, " " );
			if( p ) strcat( buffer, " " );
		}
	} else {
		 strcat( buffer, token );
		 strcat( buffer, " of type " );
		 strcat( buffer, tag.c_str() );
	}
	strcat( buffer, postfix );
}

void RpgItem::setWeaponInfluence( int skill, int type, int limit, WeaponInfluence influence ) {
	weaponInfluence[skill][type][limit].limit = influence.limit;
	weaponInfluence[skill][type][limit].type = influence.type;
	weaponInfluence[skill][type][limit].base = influence.base;
}

WeaponInfluence *RpgItem::getWeaponInfluence( int skill, int type, int limit ) {
	return &(weaponInfluence[skill][type][limit]);
}
