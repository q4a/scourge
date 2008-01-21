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
#include "../common/constants.h"
#include "rpgitem.h"
#include "spell.h"

class ConfigLang;

/**
  *@author Gabor Torok
  */

class Monster  {

 private:
	 enum { DESCR_SIZE = 300 };
  char *type;
	char *displayName;
  char *descriptiveType;
  int hp;
  int mp;
  int level;
  char *model_name;
  char *skin_name;
  char description[ DESCR_SIZE ];
  int money;
  int speed;
  int baseArmor;
  int rareness;
  float scale;
  bool npc;
  char *portrait;
  GLuint portraitTexture;
  float baseAttackBonus;
	GLuint statemod;
	bool harmless;
  std::vector<RpgItem*> items;
  std::vector<Spell*> spells;
  std::map<std::string,int> skills;

  static std::map<int, std::vector<Monster*>* > monsters;
  static std::map<int, std::vector<std::string>*>* currentSoundMap;
  static std::vector<std::string> monsterTypes;
  static std::vector<Monster*> npcs;
	static std::vector<Monster*> harmlessCreatures;
  static std::map<std::string, std::string> modelToDescriptiveType;

public:
  Monster( char *type, char *displayName, char *descriptiveType, int level, int hp, int mp, char *model, char *skin, int rareness, int speed, int baseArmor, float scale, bool npc, char *portrait, bool harmless );
  ~Monster();

  static std::map<std::string, std::map<int, std::vector<std::string>*>*> soundMap;
  static std::map<std::string, Monster*> npcPos;
  static std::map<std::string, Monster*> monstersByName;  

	inline bool isHarmless() { return harmless; }
  inline float getScale() { return scale; }
  inline int getBaseArmor() { return baseArmor; }
  inline int getRareness() { return rareness; }
  inline int getSpeed() { return speed; }
  inline char *getType() { return type; };
	inline char *getDisplayName() { return displayName; };
  inline static char *getDescriptiveType( char *modelName ) {
    std::string modelStr = modelName;
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
  inline float getBaseAttackBonus() { return baseAttackBonus; }
	inline void setStartingStateMod( GLuint n ) { statemod = n; }
	inline GLuint getStartingStateMod() { return statemod; }

  static void initMonsters();
  static Monster *getRandomMonster(int level);
  static Monster *getMonsterByName(char *name);  
  static std::map<int, std::vector<std::string>*>* getSoundMap( char *monsterType );

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
	static const Monster *getRandomHarmless();

protected:
  static void addMd2Sounds( char *model_name, std::map<int, std::vector<std::string>*>* currentSoundMap );
  /**
   * add a coma-delimited list of sound files
   */
  static void addSound( int type, char *line, std::map<int, std::vector<std::string>*>* currentSoundMap );

private:
	static void initCreatures( ConfigLang *config );
	static void initSounds( ConfigLang *config );
};

#endif
