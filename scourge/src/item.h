/***************************************************************************
                          item.h  -  description
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

#ifndef ITEM_H
#define ITEM_H

#include "constants.h"
#include "persist.h"
#include "glshape.h"
#include "shapepalette.h"
#include "rpg/rpgitem.h"
#include "rpg/spell.h"
#include <vector>

class Scourge;

/**
  *@author Gabor Torok

  This class is both the UI representation (shape) of the rpgItem and it's state (for example, wear).
  All instances of the RpgItem point to the same RpgItem, but a new Item is created for each.

  */

class Item {
 private:
  RpgItem *rpgItem;
  int shapeIndex;
  Color *color;
  GLShape *shape;
  bool blocking;
  Item *containedItems[MAX_CONTAINED_ITEMS];
  int containedItemCount;
  int currentCharges;
  Spell *spell;
  char itemName[255];





  // Things that change with item level (override rpgitem values)
  int level;
  float weight; 
  int price;
  int action; // damage, defence, potion str.
  int speed; // 0-100, 100-slowest, 0-fastest
  int distance; // how far can it reach?
  int maxCharges;
  int duration;
  int quality;

  // former magic attrib stuff
  int magicLevel;
  int bonus; // e.g.: sword +2
  int damageMultiplier; // 2=double damage, 3=triple, etc.
  char *monsterType; // if not NULL, damageMultiplier only for this type of monster.
  MagicSchool *school; // magic damage by a school (or NULL if N/A)
  Dice *magicDamage; 
  bool cursed;
  int stateMod[Constants::STATE_MOD_COUNT]; // 0=nothing, 1=sets, 2=clears/protects against state mod when worn
  bool stateModSet;
  map<int, int> skillBonus;


public:
  Item(RpgItem *rpgItem, int level=1);
  ~Item();

  ItemInfo *save();
  //ContainedItemInfo saveContainedItems();
  static Item *load(Session *session, ItemInfo *info);

  static map<int, vector<string> *> soundMap;
  
  inline Color *getColor() { return color; }
  inline void setColor(Color *c) { color = c; }
  inline void setShape(GLShape *s) { shape = s; }
  inline GLShape *getShape() { return shape; }
  inline RpgItem *getRpgItem() { return rpgItem; }
  inline bool isBlocking() { return blocking; }
  inline void setBlocking(bool b) { blocking = b; }
  inline int getCurrentCharges() { return currentCharges; }
  inline void setCurrentCharges(int n) { if(n < 0)n=0; if(n>rpgItem->getMaxChargesRpg())n=rpgItem->getMaxChargesRpg(); currentCharges = n; } 
  inline void setWeight(float f) { if(f < 0.0f)f=0.1f; weight=f; }
  inline void setSpell(Spell *spell) { this->spell = spell; sprintf(this->itemName, "Scroll of %s", spell->getName()); }
  inline Spell *getSpell() { return spell; }


  void getDetailedDescription(char *s, bool precise=true);
  inline char *getItemName() { return itemName; }

  inline int getContainedItemCount() { return containedItemCount; }
  // return true if successful
  bool addContainedItem(Item *item, bool force=false);
  // return removed item, or NULL
  Item *removeContainedItem(int index);
  Item *getContainedItem(int index);
  bool isContainedItem(Item *item);

  // return true if the item is used up
  // this method also adjusts weight
  bool decrementCharges();

  const char *getRandomSound();
  
  static void initItems(ShapePalette *shapePal);

  void enchant(int level);


  // level-based attributes
  inline int getLevel() { return level; }
  inline float getWeight() { return weight; }
  inline int getPrice() { return price; }
  inline int getAction() { return action; }
  inline int getSpeed() { return speed; }
  inline int getDistance() { return distance; }
  inline int getMaxCharges() { return maxCharges; }
  inline int getDuration() { return duration; }
  inline int getQuality() { return quality; }

  inline bool isMagicItem() { return ( magicLevel > -1 ); }
  inline map<int,int> *getSkillBonusMap() { return &skillBonus; }
  inline int getSkillBonus(int skill) { return (skillBonus.find(skill) == skillBonus.end() ? 0 : skillBonus[skill]); }
  inline int getMagicLevel() { return magicLevel; }
  inline int getBonus() { return bonus; }
  inline int getDamageMultiplier() { return damageMultiplier; }
  inline char *getMonsterType() { return monsterType; }
  inline MagicSchool *getSchool() { return school; }
  int rollMagicDamage();
  inline int getMagicResistance() { return (7 * (getLevel() + getMagicLevel())); }
  char *describeMagicDamage();
  inline bool isCursed() { return cursed; }
  inline bool isStateModSet(int mod) { return(stateMod[mod] == 1); }
  inline bool isStateModProtected(int mod) { return(stateMod[mod] == 2); }

  void debugMagic(char *s);

 protected:
  void commonInit();
  float getRandomSum( float base, int count );
  void describeMagic(char *s, char *itemName);
};

#endif
