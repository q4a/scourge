/***************************************************************************
                          character.h  -  description
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

#ifndef CHARACTER_H
#define CHARACTER_H

#include "../constants.h"
#include <map>
#include <vector>
#include <string>

using namespace std;

/**
  *@author Gabor Torok
  */
  
class Character  {
private:
  char *name;
  int startingHp, startingMp, skill_bonus, level_progression;
  char *model_name, *skin_name;
  char description[3000];
  char *shortName;
  map<int, int> maxSkill;
  map<int, int> minSkill;

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

  Character(char *name, int startingHp, int startingMp, char *model, char *skin, int skill_bonus, int level_progression, char *shortName);
  ~Character();

  inline char *getName() { return name; };
  inline char *getShortName() { return shortName; };
  inline int getStartingHp() { return startingHp; }  
  inline int getStartingMp() { return startingMp; }  
  inline int getSkillBonus() { return skill_bonus; }  
  inline int getLevelProgression() { return level_progression; }  
  inline char *getModelName() { return model_name; }
  inline char *getSkinName() { return skin_name; }
  inline char *getDescription() { return description; }
  inline int getMaxSkillLevel(int skill) { if(maxSkill.find(skill) == maxSkill.end()) return 100; else return maxSkill[skill]; }
  inline int getMinSkillLevel(int skill) { if(minSkill.find(skill) == minSkill.end()) return 0; else return minSkill[skill]; }
  
  static map<string, Character*> character_class;
  static map<string, Character*> character_class_short;
  static map<string, int> character_index_short;
  static void initCharacters();
  static Character *getCharacterByName(char *p) { string s = p; return character_class[s]; }
  static Character *getCharacterByShortName(char *p) { string s = p; return character_class_short[s]; }
  static int getCharacterIndexByShortName(char *p) {  string s = p; return character_index_short[s]; }

 protected:
  inline void setMinMaxSkill(int skill, int min, int max) { minSkill[skill] = min; maxSkill[skill] = max; }

};

#endif
