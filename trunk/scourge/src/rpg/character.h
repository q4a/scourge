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
  char description[3000];
  char *shortName;
  map<int, int> maxSkill;
  map<int, int> minSkill;

public:
  map<int, vector<string>*> soundMap;

  Character(char *name, int startingHp, int startingMp, int skill_bonus, int level_progression, char *shortName);
  ~Character();

  inline char *getName() { return name; };
  inline char *getShortName() { return shortName; };
  inline int getStartingHp() { return startingHp; }  
  inline int getStartingMp() { return startingMp; }  
  inline int getSkillBonus() { return skill_bonus; }  
  inline int getLevelProgression() { return level_progression; }  
  inline char *getDescription() { return description; }
  inline int getMaxSkillLevel(int skill) { if(maxSkill.find(skill) == maxSkill.end()) return 100; else return maxSkill[skill]; }
  inline int getMinSkillLevel(int skill) { if(minSkill.find(skill) == minSkill.end()) return 0; else return minSkill[skill]; }
  void addSound(int type, char *file);
  const char *getRandomSound(int type);

  static map<string, Character*> character_class;
  static map<string, Character*> character_class_short;
  static map<string, int> character_index_short;
  static vector<Character*> character_list;
  static void initCharacters();
  static Character *getCharacterByName(char *p) { string s = p; return character_class[s]; }
  static Character *getCharacterByShortName(char *p) { string s = p; return character_class_short[s]; }
  static int getCharacterIndexByShortName(char *p) {  string s = p; return character_index_short[s]; }
  inline static Character *getRandomCharacter() { return character_list[(int)((float)character_list.size()*rand()/RAND_MAX)]; }

 protected:
  inline void setMinMaxSkill(int skill, int min, int max) { minSkill[skill] = min; maxSkill[skill] = max; }
  static void addSounds(int type, char *line, Character *c);
};

#endif
