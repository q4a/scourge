/***************************************************************************
                          monster.h  -  description
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

#ifndef MONSTER_H
#define MONSTER_H

#include <vector>
#include <map>
#include <stdio.h>
#include "../constants.h"
#include "rpgitem.h"
#include "spell.h"

/**
  *@author Gabor Torok
  */

using namespace std;
  
class Monster  {

 private:
  char *type;
  char *descriptiveType;
  int hp;
  int mp;
  int level;
  char *model_name;
  char *skin_name;
  char description[300];
  int money;
  int speed;
  int baseArmor;
  int rareness;
  float scale;
  bool npc;
  char *portrait;
  GLuint portraitTexture;
  vector<RpgItem*> items;
  vector<Spell*> spells;
  map<string,int> skills;

  static map<int, vector<Monster*>* > monsters;
  static map<int, vector<string>*>* currentSoundMap;
  static vector<string> monsterTypes;
  static vector<Monster*> npcs;
  static map<string, string> modelToDescriptiveType;

public:
  Monster(char *type, char *descriptiveType, int level, int hp, int mp, char *model, char *skin, int rareness, int speed, int baseArmor, float scale, bool npc, char *portrait);
  ~Monster();

  static map<string, map<int, vector<string>*>*> soundMap;
  static map<string, Monster*> npcPos;
  static map<string, Monster*> monstersByName;  

  inline float getScale() { return scale; }
  inline int getBaseArmor() { return baseArmor; }
  inline int getRareness() { return rareness; }
  inline int getSpeed() { return speed; }
  inline char *getType() { return type; };
  inline static char *getDescriptiveType( char *modelName ) {
    string modelStr = modelName;
    if( modelToDescriptiveType.find( modelStr ) == modelToDescriptiveType.end() ) 
      return NULL;
    else return (char*)( modelToDescriptiveType[ modelStr ].c_str() );
  }
  inline int getHp() { return hp; }  
  inline int getMp() { return mp; }  
  inline int getLevel() { return level; }  
  inline char *getModelName() { return model_name; }
  inline char *getSkinName() { return skin_name; }
  inline char *getDescription() { return description; }
  inline int getStartingItemCount() { return items.size(); }
  inline RpgItem *getStartingItem(int index) { return items[index]; }
  inline void addItem(RpgItem *item) { items.push_back(item); }
  inline int getStartingSpellCount() { return spells.size(); }
  inline Spell *getStartingSpell(int index) { return spells[index]; }
  inline void addSpell(Spell *spell) { spells.push_back(spell); }
  int getSkillLevel(const char *skillName);
  const char *getRandomSound(int type);
  inline bool isNpc() { return npc; }
  inline char *getPortrait() { return portrait; }
  inline GLuint getPortraitTexture() { return portraitTexture; }
  inline void setPortraitTexture( GLuint n ) { portraitTexture = n; }

  static void initMonsters();
  static Monster *getRandomMonster(int level);
  static Monster *getMonsterByName(char *name);  
  static map<int, vector<string>*>* getSoundMap( char *monsterType );

  /**
   * Finds the index of a monster or a monster by an index:
   * If *monster is NULL, it sets *monster when *index is found.
   * If *monster is not null, it sets *index when *monster is found.
   * @return true if search was successful, false otherwise.
   */
  static bool getIndexOrFindByIndex(Monster **monster, int *index);

  static const char *getRandomMonsterType( int level );
  static const char *getMonsterType(char *type);
  static const Monster *getRandomNpc();

 protected:
  static void addMd2Sounds( char *model_name, map<int, vector<string>*>* currentSoundMap );
  /**
   * add a coma-delimited list of sound files
   */
  static void addSound( int type, char *line, map<int, vector<string>*>* currentSoundMap );

};

#endif
