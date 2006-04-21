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
#ifndef RPG_ITEM_H
#define RPG_ITEM_H

#include <map>
#include <string>
#include <vector>
#include <set>
#include "../constants.h"
#include "character.h"

/*
 * Remember to change isWeaponItem=, getRandomEnchantableItem, 
 * getRandomItem, getRandomContainer, getRandomContainerNS
 * 
 * when adding a new item or type.
 */

class Monster;
class Dice;
class MagicSchool;
class Spell;
class RpgItem;

typedef struct _ItemType {
  char name[40];
  bool isWeapon, isArmor, isRandom, isRanged, hasSpell, isEnchantable;
} ItemType;

class RpgItem {
 private:

  int index;
  char *name, *desc, *shortDesc;
  int level;
  int rareness;
  int type;
  float weight; 
  int price, quality;
  Dice *action; // damage, defence, potion str.
  int speed; // 0-100, 100-slowest, 0-fastest
  int shape_index;
  int twohanded;
  int distance; // how far can it reach?
  int equip; // where can it be worn?
  int skill; // which skill to check when using the item
  int minDepth; // 0-based min. depth where the item occurs
  int minLevel; // if >0 the item is "special" and only one instance exists
  int maxCharges;
  int potionSkill; // which skill does this potion effect?
  int potionTime;
  bool isWeaponItem;
  int iconTileX, iconTileY;
  int maxSkillBonus; // max coord. bonus applied to armor and shields
	std::set<std::string> tags;

  static std::map<int, std::map<int, std::vector<const RpgItem*>*>*> typesMap;
  static std::map<std::string, const RpgItem *> itemsByName;
  static std::vector<RpgItem*> containers;
  static std::vector<RpgItem*> containersNS;

  // linear for now... if there are too many of these we can organize them 
  // in a type->depth map.
  static std::vector<RpgItem*> special;	

 public:

   enum {
     SWORD=0,
     AXE,
     BOW,
     MACE,
     CONTAINER,
     ARMOR,
     FOOD,
     DRINK,
     POTION,
     OTHER,
     MISSION,
     SCROLL,
     POLE,
     WAND,
     RING,
     AMULET,
     STAFF,

     ITEM_TYPE_COUNT
   };

  static std::vector<ItemType> itemTypes;
  static int randomTypes[ITEM_TYPE_COUNT];
  static int randomTypeCount;

  enum twoHandedType {
    NOT_TWO_HANDED=0,
    ONLY_TWO_HANDED,
    OPTIONAL_TWO_HANDED
  };

  static RpgItem *items[1000];
  static int itemCount;

	static std::map<std::string,std::string> tagsDescriptions;
	static void describeTag( char *buffer, char *prefix, std::string tag, char *postfix, char *token );

  RpgItem(int index, char *name, int level, int rareness, int type, float weight, int price, int quality, 
          Dice *action, int speed, char *desc, char *shortDesc, int equip, int shape_index, 
          int twohanded=NOT_TWO_HANDED, int distance=1, int skill=-1, int minDepth=0, int minLevel=0, 
          int maxCharges=0,
          int potionSkill=-1, int potionTime=0, int iconTileX=0, int iconTileY=0,
          int maxSkillBonus=-1);
  ~RpgItem();

	inline void addTag( char *tag ) {
		std::string s = tag;
		tags.insert( s );
	}
	inline bool hasTag( char *tag ) {
		std::string s = tag;
		return( tags.find( s ) != tags.end() );
	}


  // -Rpg in the name to not accidentally call it instead of item->getXYZ().
  inline int getLevelRpg()  { return level; }  
  inline float getWeightRpg() { return weight; }
  inline int getPriceRpg() { return price; }
  inline int getSpeedRpg() { return speed; }
  inline int getDistanceRpg() { return distance; }
  inline int getMaxChargesRpg() { return maxCharges; }
  inline int getDurationRpg() { return potionTime; }
  inline int getQualityRpg() { return quality; }

  inline int getIndex() { return index; }
  inline char *getName() { return name; }
  inline int getRareness()  { return rareness; }
  inline int getShapeIndex() { return shape_index; }
  inline char *getShortDesc() { return shortDesc; }  
  inline char *getLongDesc() { return desc; }  
  inline int getEquip() { return equip; }
  inline int getSkill() { return skill; } 
  inline int getMinDepth() { return minDepth; }
  inline int getMinLevel() { return minLevel; }
  inline bool isSpecial() { return( minLevel > 0 ); }
  inline int getType() { return type; }
  inline int getPotionSkill() { return potionSkill; }
  inline bool isWeapon() { return this->isWeaponItem; }
  inline int getIconTileX() { return this->iconTileX; }
  inline int getIconTileY() { return this->iconTileY; }
  inline int getMaxSkillBonus() { return this->maxSkillBonus; }
  inline int getTwoHanded() { return this->twohanded; }
  inline Dice *getAction() { return this->action; }

  // FIXME: make this more specific to item
  // e.g. multi-attack items, like sword of fireballs
  inline bool isRangedWeapon() { return type == BOW; }

  bool isEnchantable();

  bool isContainer();

  static RpgItem *getRandomItem(int depth);
  static RpgItem *getRandomItemFromTypes(int level, int types[], int typeCount);
  static RpgItem *getRandomContainer();
  static RpgItem *getRandomContainerNS();
  inline static RpgItem *getItem(int index) { return items[index]; }

  static int getTypeByName(char *name);
  static void addItem(RpgItem *item, int width, int depth, int height);
  static RpgItem *getItemByName(char *name);
  inline static std::map<std::string, const RpgItem *> *getItemMap() { return &itemsByName; }
  inline static int getSpecialCount() { return special.size(); }
  inline static RpgItem *getSpecial( int index ) { return special[index]; }
  
};

#endif
