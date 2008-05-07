/***************************************************************************
                          rpg.h  -  description
                             -------------------
    begin                : Sat May 3 2003
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

#ifndef RPG_H
#define RPG_H

#include "../common/constants.h"
#include "../configlang.h"
#include "../util.h"
#include <vector>
#include <map>

class SkillGroup;

class Rpg {
private:
	static std::vector<char*> firstSyl;
	static std::vector<char*> midSyl;
	static std::vector<char*> endSyl;

public:	
	static void initRpg();
	static char *createName();

protected:
	static void initSkills( ConfigLang *config );
	static void initNames( ConfigLang *config );
	static void initStateMods( ConfigLang *config );
};

class Skill {
private:
	char name[80];
  char displayName[255];
	char description[120];
	char symbol[80];
	std::vector<Skill*> preReqStats;
	int preReqStatMultiplier;
	int index;
	SkillGroup *group;

public:

	// FIXME: this should be generated from professions.txt
	// =======================================
	// Note: order must match that in world/rpg.txt!
	// =======================================
	enum {
		SPEED=0,
		COORDINATION,
		POWER,
		IQ,
		LEADERSHIP,
		LUCK,
		PIETY,
		LORE,
		
		MELEE_WEAPON,
		RANGED_WEAPON,
		LARGE_WEAPON,
		HAND_TO_HAND_COMBAT,
		
		SHIELD_DEFEND,
		ARMOR_DEFEND,
		DODGE_ATTACK,
		
		LIFE_AND_DEATH_MAGIC,
		DECEIT_MAGIC,
		CONFRONTATION_MAGIC,
		RESIST_LIFE_AND_DEATH_MAGIC,
		RESIST_DECEIT_MAGIC,
		RESIST_CONFRONTATION_MAGIC,
		
		NATURE_MAGIC,
		AWARENESS_MAGIC,
		RESIST_NATURE_MAGIC,
		RESIST_AWARENESS_MAGIC,
		
		HISTORY_MAGIC,
		RESIST_HISTORY_MAGIC,
		ENCHANT_ITEM,
		IDENTIFY_ITEM,
		IDENTIFY_CREATURE,
		
		OPEN_LOCK,
		FIND_TRAP,
		FIND_SECRET_DOOR,
		MOVE_UNDETECTED,
		STEALING,

		SKILL_COUNT
	};

	Skill( char *name, char *displayName, char *description, char *symbol, SkillGroup *group );
	~Skill();

	inline void setPreReqMultiplier( int n ) { preReqStatMultiplier = n; }
	inline int getPreReqMultiplier() { return preReqStatMultiplier; }
	inline void addPreReqStat( Skill *stat ) { preReqStats.push_back( stat ); }
	inline int getPreReqStatCount() { return preReqStats.size(); }
	inline Skill *getPreReqStat( int index ) { return preReqStats[ index ]; }

	inline char *getName() { return name; }
	inline char const* getDisplayName() { return displayName; }
	inline char *getDescription() { return description; }
	inline char *getSymbol() { return symbol; }
	inline SkillGroup *getGroup() { return group; }
	inline int getIndex() { return index; }

	static std::map<std::string, Skill*> skillsByName;

	static inline Skill *getSkillByName( char const* name ) {
		std::string s = name;
		return( skillsByName.find( s ) == skillsByName.end() ? NULL : skillsByName[ s ] );
	}

	static inline int getSkillIndexByName( char const* name ) {
		Skill *skill = getSkillByName( name );
		return( skill ? skill->getIndex() : -1 );
	}

	// fast access
	static std::vector<Skill*> skills;	
};

class SkillGroup {
private:
	char name[80];
	char displayName[255];
	char description[255];
	int index;
	bool isStatSkill;
	std::vector<Skill*> skills;

public:
	SkillGroup( char *name, char *displayName, char *description );
	~SkillGroup();

	inline bool isStat() { return isStatSkill; }
	inline char *getName() { return name; }
	inline char *getDisplayName() { return displayName; }
	inline char *getDescription() { return description; }
	inline int getIndex() { return index; }

	inline int getSkillCount() { return skills.size(); }
	inline Skill *getSkill( int index ) { return skills[ index ]; }
	inline void addSkill( Skill *skill ) { skills.push_back( skill ); }
	inline Skill *getRandomSkill() { return getSkill( Util::dice( getSkillCount() ) ); }
	
	static SkillGroup *stats;
	static std::map<std::string, SkillGroup *> groupsByName;
	static inline SkillGroup *getGroupByName( char const* name ) {
		std::string s = name;
		return( groupsByName.find( s ) == groupsByName.end() ? NULL : groupsByName[ s ] );
	}

	// fast access
	static std::vector<SkillGroup*> groups;
};

class StateMod {
private:
	char name[80];
	char displayName[255];
	char symbol[255];
	char setState[255];
	char unsetState[255];
	int type;
	int index;

public:
	enum {
		blessed=0,
		empowered,
		enraged,
		ac_protected,
		magic_protected,
		drunk,
		poisoned,
		cursed,
		possessed,
		blinded,
		paralysed,
		invisible,
		overloaded,
		dead,
		asleep,
  
  	// must be last
		STATE_MOD_COUNT
	};

	enum {
		NEITHER=0,
		BAD,
		GOOD
	};

	StateMod( char const* name, char const* displayName, char const* symbol
	        , char const* setState, char const* unsetState, int type, int index );
	~StateMod();

	static std::map<std::string, StateMod*> stateModsByName;
	static std::vector<StateMod*> stateMods;
	static std::vector<StateMod*> goodStateMods;
	static std::vector<StateMod*> badStateMods;
	static inline StateMod *getStateModByName( char *name ) {
		std::string s = name;
		return( stateModsByName.find( s ) == stateModsByName.end() ? NULL : stateModsByName[ s ] );
	}

	static StateMod *getRandomGood();
	static StateMod *getRandomBad();

	bool isStateModTransitionWanted( bool setting );

	inline char *getName() { return name; }
	inline char *getDisplayName() { return displayName; }
	inline char *getSetState() { return setState; }
	inline char *getUnsetState() { return unsetState; }
	inline int getType() { return type; }
	inline char *getSymbol() { return symbol; }
	inline int getIndex() { return index; }
};

#endif

