/***************************************************************************
           character.h  -  Class describing a playable character
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
#pragma once

#include "../util.h"
#include <map>
#include <set>
#include <vector>
#include <string>

/**
  *@author Gabor Torok
  */

class RpgItem;
class Skill;
class SkillGroup;
class Character;

/// Collection of all characters.

class Characters {
public:
	Characters(); // was Character::initCharacters()
	~Characters(); // was Character::unInitCharacters()

	Character* getRandom() {
		return rootCharacters[ Util::dice( rootCharacters.size() ) ];
	}

	static int getRootIndexByName( char const* p );
	static Character* getByName( char const* p ) {
		if ( instance == NULL ) { 
			std::cerr << "*** Characters::getByName() Characters uninitialized" << std::endl;
			return NULL;
		}
		return instance->getCharacterByName( p );
	}
	static Character* getRootByIndex( int i ) {
		if ( instance == NULL ) { 
			std::cerr << "*** Characters::getRootByIndex() Characters uninitialized" << std::endl;
			return NULL;
		}
		return instance->rootCharacters[i];
	}
	static int getRootCount() {
		if ( instance == NULL ) { 
			std::cerr << "*** Characters::getRootCount() Characters uninitialized" << std::endl;
			return NULL;
		}
		return instance->rootCharacters.size();
	}

private:
	std::map<std::string, Character*> character_class;
	std::vector<Character*> character_list;
	std::vector<Character*> rootCharacters;
	// Character is designed to know all of its kind 
	friend class Character;
	static Characters* instance;

	void addItemTags( const char *s, std::set<std::string> *list );
	void buildTree();
	
	Character* getCharacterByName( char const* p ) {
		std::string s = p; return character_class[s];
	}
	
	// undefine all unused implicits
	Characters( Characters const& that ); // undefined
	Characters& operator=( Characters const& that ); // undefined

};

/// A playable character.

class Character  {
	friend class Characters;
private:
	std::string name;
	std::string displayName;
	std::string parentName;
	int startingHp, startingMp, level_progression;
	int minLevelReq;
	char description[3000];
	Character *parent;
	std::vector<Character*> children;
	std::map<int, int> skills;
	std::map<std::string, int> groups;
	std::vector<std::string> capabilities;
	std::set<std::string> allowedWeaponTags;
	std::set<std::string> forbiddenWeaponTags;
	std::set<std::string> allowedArmorTags;
	std::set<std::string> forbiddenArmorTags;

public:
	Character(  char const* name, char const* displayName, char const* parentName,
	           int startingHp, int startingMp,
	           int level_progression, int minLevelReq );
	~Character();

	static Character *getRandomCharacter();
	static Character *getRandomCharacter( int level );

	inline char const* getName() {
		return name.c_str();
	};
	inline char const* getDisplayName() {
		return displayName.c_str();
	};
	inline int getStartingHp() {
		return startingHp;
	}
	inline int getStartingMp() {
		return startingMp;
	}
	inline int getLevelProgression() {
		return level_progression;
	}
	inline char *getDescription() {
		return description;
	}
	inline int getMinLevelReq() {
		return minLevelReq;
	}
	inline char const* getParentName() {
		return parentName.c_str();
	}
	inline Character *getParent() {
		return parent;
	}
	inline int getChildCount() {
		return children.size();
	}
	inline Character *getChild( int index ) {
		return children[index];
	}
	inline int getCapabilityCount() {
		return capabilities.size();
	}
	inline const char *getCapability( int index ) {
		return capabilities[index].c_str();
	}
	inline int getSkill( int skillIndex ) {
		return( skills.find( skillIndex ) == skills.end() ? -1 : skills[skillIndex] );
	}
	bool canEquip( RpgItem *item );
	// FIXME: hard-coded for now
	inline int getSkillBonus() {
		return 5;
	}

	void finishProfessionTag();
	void printSet( std::set<std::string> *s, char *tagName );
protected:
	void describeProfession();
	bool canEquip( RpgItem *item, std::set<std::string> *allowed, std::set<std::string> *forbidden );
	void describeAcl( char *s, std::set<std::string> *allowed, std::set<std::string> *forbidden, int itemType );
};

#endif

