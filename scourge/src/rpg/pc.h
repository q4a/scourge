/***************************************************************************
                          pc.h  -  description
                             -------------------
    begin                : Mon Jul 7 2003
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

#ifndef PC_H
#define PC_H

#include "../constants.h"
#include "character.h"
#include "rpgitem.h"

/**
  *@author Gabor Torok
  */

#define MAX_INVENTORY_SIZE 200

class RpgItem;

class PlayerChar  {
 public:
  
  // inventory locations
  static const int INVENTORY_HEAD = 1;
  static const int INVENTORY_NECK = 2;
  static const int INVENTORY_BACK = 4;
  static const int INVENTORY_CHEST = 8;
  static const int INVENTORY_LEFT_HAND = 16;
  static const int INVENTORY_RIGHT_HAND = 32;
  static const int INVENTORY_BELT = 64;
  static const int INVENTORY_LEGS = 128;
  static const int INVENTORY_FEET = 256;
  static const int INVENTORY_RING1 = 512;
  static const int INVENTORY_RING2 = 1024;
  static const int INVENTORY_RING3 = 2048;
  static const int INVENTORY_RING4 = 4096;
  static const int INVENTORY_WEAPON_RANGED = 8192;
  static const int INVENTORY_COUNT = 14;
  static char inventory_location[][80];

 private:
  char *name;
  int level, exp, hp, ac;
  Character *character;
  RpgItem *inventory[MAX_INVENTORY_SIZE];
  int inventory_count;
  int equipped[INVENTORY_COUNT];
  int skills[Constants::SKILL_COUNT];
  GLuint stateMod;

 public:
  PlayerChar(char *name, Character *character);
  ~PlayerChar();

  inline char *getName() { return name; }
  inline Character *getCharacter() { return character; }  
  inline int getLevel() { return level; }
  inline int getExp() { return exp; }
  inline int getHp() { return hp; }
  inline int getAc() { return ac; }
  inline RpgItem *getInventory(int index) { return inventory[index]; }
  inline int getInventoryCount() { return inventory_count; }
  inline int getSkill(int index) { return skills[index]; }
  inline bool getStateMod(int mod) { return (stateMod & (1 << mod) ? true : false); }


  inline void setName(char *s) { name = s; }
  inline void setCharacter(Character *c) { character = c; }  
  inline void setLevel(int n) { level = n; }
  inline void setExp(int n) { exp = n; }
  inline void setHp(int n) { hp = n; }
  inline void setHp() { hp = getCharacter()->getStartingHp(); }
  inline void setAc(int n) { ac = n; }
  inline void setInventory(int index, RpgItem *item) { 
	if(index < inventory_count) inventory[index] = item; 
  }
  inline void addInventory(RpgItem *item) { inventory[inventory_count++] = item; }
  RpgItem *removeInventory(int index);
  inline void setSkill(int index, int value) { skills[index] = value; }
  inline void setStateMod(int mod, bool setting) { 
	if(setting) stateMod |= (1 << mod); 
	else stateMod &= ((GLuint)0xffff - (GLuint)(1 << mod)); 
  }
  void equipInventory(int index);

  // get the item at the given equip-index (inventory location)
  RpgItem *getEquippedInventory(int index);

  // return the equip index (inventory location) for an inventory index
  int getEquippedIndex(int index);

  // until *someone* writes a pc editor
  static PlayerChar **createHardCodedParty();
};

#endif
