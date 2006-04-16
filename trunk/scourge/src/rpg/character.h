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

/**
  *@author Gabor Torok
  */
  
class Character  {
private:
  char *name;
  char *parentName;
  int startingHp, startingMp, level_progression;
  int minLevelReq;
  char description[3000];
  Character *parent;
  std::vector<Character*> children;
  std::map<int,int> skills;
  std::vector<std::string> capabilities;

public:
  Character( char *name, char *parentName, 
             int startingHp, int startingMp, 
             int level_progression, int minLevelReq );
  ~Character();

  inline char *getName() { return name; };
  inline int getStartingHp() { return startingHp; }  
  inline int getStartingMp() { return startingMp; }  
  inline int getLevelProgression() { return level_progression; }  
  inline char *getDescription() { return description; }
  inline int getMinLevelReq() { return minLevelReq; }
  inline char *getParentName() { return parentName; }
  inline Character *getParent() { return parent; }
  inline int getChildCount() { return children.size(); }
  inline Character *getChild( int index ) { return children[index]; }
  inline int getCapabilityCount() { return capabilities.size(); }
  inline const char *getCapability( int index ) { return capabilities[index].c_str(); }
  inline int getSkill( int skillIndex ) {
    return( skills.find( skillIndex ) == skills.end() ? -1 : skills[skillIndex] );
  }

  static std::map<std::string, Character*> character_class;  
  static std::vector<Character*> character_list;  
  static std::vector<Character*> rootCharacters;
  static Character *getCharacterByName(char *p) { std::string s = p; return character_class[s]; }
  inline static Character *getRandomCharacter() { return rootCharacters[(int)((float)rootCharacters.size()*rand()/RAND_MAX)]; }

  static void initCharacters();
  static void buildTree();
};

#endif

