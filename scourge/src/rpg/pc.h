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

class PlayerChar  {
private:
  char *name;
  int level, exp, hp, ac;
  Character *character;
  RpgItem *inventory[MAX_INVENTORY_SIZE];
  int inventory_count;
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

  // until *someone* writes a pc editor
  static PlayerChar **createHardCodedParty();

};

#endif
