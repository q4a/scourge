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

/*
class MagicAttrib {
private:
  int bonus; // e.g.: sword +2
  int damageMultiplier; // 2=double damage, 3=triple, etc.
  char *monsterType; // if not NULL, damageMultiplier only for this type of monster.
  MagicSchool *school; // magic damage by a school (or NULL if N/A)
  Dice *magicDamage; 
  bool cursed;
  int stateMod[Constants::STATE_MOD_COUNT]; // 0=nothing, 1=sets, 2=clears/protects against state mod when worn
  int level;
  bool stateModSet;
  map<int, int> skillBonus;

public:
  MagicAttrib();
  ~MagicAttrib();

  MagicAttribInfo *save();
  static MagicAttribInfo *saveEmpty();
  static MagicAttrib *load(Session *session, MagicAttribInfo *info);

  inline map<int,int> *getSkillBonusMap() { return &skillBonus; }
  inline int getSkillBonus(int skill) { return (skillBonus.find(skill) == skillBonus.end() ? 0 : skillBonus[skill]); }
  inline int getLevel() { return level; }
  inline int getBonus() { return bonus; }
  inline int getDamageMultiplier() { return damageMultiplier; }
  inline char *getMonsterType() { return monsterType; }
  inline MagicSchool *getSchool() { return school; }
  int rollMagicDamage();
  inline int getMagicResistance() { return (7 * getLevel()); }
  char *describeMagicDamage();
  inline bool isCursed() { return cursed; }
  inline bool isStateModSet(int mod) { return(stateMod[mod] == 1); }
  inline bool isStateModProtected(int mod) { return(stateMod[mod] == 2); }
  void debug(char *s, RpgItem *item);

   //Create a magic attribute obj. depending on the level.
   //(0=lesser,1=greater,2=champion,3=divine)
  void enchant(int level, bool isWeapon);

   //Write the description of this item into buffer s.
   //S has to be large enough to hold the description. (~255 chars)
  void describe(char *s, char *itemName);
};
*/

class RpgItem {
 private:

  int index;
  char *name, *desc, *shortDesc;
  int level;
  int rareness;
  int type;
  float weight; 
  int price, quality;
  int action; // damage, defence, potion str.
  int speed; // 0-100, 100-slowest, 0-fastest
  int shape_index;
  int twohanded;
  int distance; // how far can it reach?
  int equip; // where can it be worn?
  int skill; // which skill to check when using the item
  int minDepth; // 0-based min. depth where the item occurs
  int maxCharges;
  int potionSkill; // which skill does this potion effect?
  int potionTime;
  GLuint acl; // 1 bit per character class
  bool isWeaponItem;
  int iconTileX, iconTileY;

  static std::map<int, std::map<int, std::vector<const RpgItem*>*>*> typesMap;
  static std::map<std::string, const RpgItem *> itemsByName;
  static std::vector<RpgItem*> containers;
  static std::vector<RpgItem*> containersNS;

 public:

  enum itemTypes {
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
	
	// must be last
	ITEM_TYPE_COUNT
  };
  static char itemTypeStr[ITEM_TYPE_COUNT][40];

  enum twoHandedType {
	NOT_TWO_HANDED=0,
	ONLY_TWO_HANDED,
	OPTIONAL_TWO_HANDED
  };

  static RpgItem *items[1000];
  static int itemCount;

  static int enchantableTypes[];
  static int enchantableTypeCount;
  
  RpgItem(int index, char *name, int level, int rareness, int type, float weight, int price, int quality, 
		  int action, int speed, char *desc, char *shortDesc, int equip, int shape_index, 
		  int twohanded=NOT_TWO_HANDED, int distance=1, int skill=-1, int minDepth=0, int maxCharges=0,
		  int potionSkill=-1, int potionTime=0, int iconTileX=0, int iconTileY=0);
  ~RpgItem();


  // -Rpg in the name to not accidentally call it instead of item->getXYZ().
  inline int getLevelRpg()  { return level; }  
  inline float getWeightRpg() { return weight; }
  inline int getPriceRpg() { return price; }
  inline int getActionRpg() { return action; }
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
  inline int getType() { return type; }
  inline int getPotionSkill() { return potionSkill; }
  inline bool getAcl(int index) { return (acl & (1 << index) ? true : false); }
  inline void setAcl(int index, bool value) { if(value) acl |= (1 << index); else acl &= ((GLuint)0xffff - (GLuint)(1 << index)); }
  inline void setAllAcl(bool value) { if(value) acl = (GLuint)0xffff; else acl = (GLuint)0; }
  inline GLuint getAllAcl() { return acl; }
  inline bool isWeapon() { return this->isWeaponItem; }
  inline int getIconTileX() { return this->iconTileX; }
  inline int getIconTileY() { return this->iconTileY; }

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
  
};

#endif
