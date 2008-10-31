/***************************************************************************
                       monster.h  -  NPC/monster class
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
#pragma once

#include <vector>
#include <map>
#include <stdio.h>
#include "rpgitem.h"
#include "spell.h"
#include "../render/texture.h"

class ConfigLang;

/**
  *@author Gabor Torok
  */

/// A non-player character or monster
class Monster  {

private:
	enum { DESCR_SIZE = 300 };
	std::string type;
	std::string displayName;
	std::string descriptiveType;
	int hp;
	int mp;
	int level;
	std::string model_name;
	std::string skin_name;
	char description[ DESCR_SIZE ];
	int money;
	int speed;
	int baseArmor;
	int rareness;
	float scale;
	bool npc;
	std::string portrait;
	Texture portraitTexture;
	float baseAttackBonus;
	GLuint statemod;
	bool harmless;
	std::vector<RpgItem*> items;
	std::vector<Spell*> spells;
	std::map<std::string, int> skills;

	static std::map<int, std::vector<Monster*>* > monsters;
	static std::map<int, std::vector<std::string>*>* currentSoundMap;
	static std::vector<std::string> monsterTypes;
	static std::vector<Monster*> npcs;
	static std::vector<Monster*> harmlessCreatures;
	static std::map<std::string, std::string> modelToDescriptiveType;

public:
	Monster( char const* type, char const* displayName, char const* descriptiveType, int level, int hp, int mp, char const* model, char const* skin, int rareness, int speed, int baseArmor, float scale, bool npc, char const* portrait, bool harmless );
	~Monster();

	static std::map<std::string, std::map<int, std::vector<std::string>*>*> soundMap;
	static std::map<std::string, Monster*> npcPos;
	static std::map<std::string, Monster*> monstersByName;

	inline bool isHarmless() {
		return harmless;
	}
	inline float getScale() {
		return scale;
	}
	inline int getBaseArmor() {
		return baseArmor;
	}
	inline int getRareness() {
		return rareness;
	}
	inline int getSpeed() {
		return speed;
	}
	inline char const* getType() {
		return type.c_str();
	};
	inline char const* getDisplayName() {
		return displayName.c_str();
	};
	inline static char const* getDescriptiveType( char const* modelName ) {
		std::string modelStr = modelName;
		if ( modelToDescriptiveType.find( modelStr ) == modelToDescriptiveType.end() )
			return NULL;
		else return modelToDescriptiveType[ modelStr ].c_str();
	}
	inline int getHp() {
		return hp;
	}
	inline int getMp() {
		return mp;
	}
	inline int getLevel() {
		return level;
	}
	inline char const* getModelName() {
		return model_name.c_str();
	}
	inline char const* getSkinName() {
		return skin_name.c_str();
	}
	inline char *getDescription() {
		return description;
	}
	inline int getStartingItemCount() {
		return items.size();
	}
	inline RpgItem *getStartingItem( int index ) {
		return items[index];
	}
	inline void addItem( RpgItem *item ) {
		items.push_back( item );
	}
	inline int getStartingSpellCount() {
		return spells.size();
	}
	inline Spell *getStartingSpell( int index ) {
		return spells[index];
	}
	inline void addSpell( Spell *spell ) {
		spells.push_back( spell );
	}
	int getSkillLevel( const char *skillName );
	const char *getRandomSound( int type );
	inline bool isNpc() {
		return npc;
	}
	inline void setNpc( bool b ) {
		npc = b;
	}
	inline char const* getPortrait() {
		return portrait.c_str();
	}
	inline Texture getPortraitTexture() {
		return portraitTexture;
	}
	inline void setPortraitTexture( Texture n ) {
		portraitTexture = n;
	}
	inline float getBaseAttackBonus() {
		return baseAttackBonus;
	}
	inline void setStartingStateMod( GLuint n ) {
		statemod = n;
	}
	inline GLuint getStartingStateMod() {
		return statemod;
	}

	static void initMonsters();
	static void unInitMonsters();
	static Monster *getRandomMonster( int level );
	static Monster *getMonsterByName( char const* name );
	static std::map<int, std::vector<std::string>*>* getSoundMap( char const* monsterType );

	/**
	 * Finds the index of a monster or a monster by an index:
	 * If *monster is NULL, it sets *monster when *index is found.
	 * If *monster is not null, it sets *index when *monster is found.
	 * @return true if search was successful, false otherwise.
	 */
	static bool getIndexOrFindByIndex( Monster **monster, int *index );

	static const char *getRandomMonsterType( int level );
	static const char *getMonsterType( char *type );
	static const Monster *getRandomNpc();
	static const Monster *getRandomHarmless();

protected:
	static void addMd2Sounds( char const* model_name, std::map<int, std::vector<std::string>*>* currentSoundMap );
	/**
	 * add a coma-delimited list of sound files
	 */
	static void addSound( int type, char *line, std::map<int, std::vector<std::string>*>* currentSoundMap );

private:
	static void initCreatures( ConfigLang *config );
	static void initSounds( ConfigLang *config );
};

#endif
