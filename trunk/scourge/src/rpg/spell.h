/***************************************************************************
                 spell.h  -  Spell and magic school classes
                             -------------------
    begin                : Sun Sep 28 2003
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
#ifndef SPELL_H
#define SPELL_H
#pragma once

#include <map>
#include <string>
#include <vector>
#include "../storable.h"
#include "../util.h"

/// A dice. You know at least one belongs in every RPG.
class Dice {
private:
	enum { S_SIZE = 80 };
	char s[ S_SIZE ];
	int count;
	int sides;
	int mod;

public:
	Dice( char const* s );
	Dice( int count, int sides, int mod );
	~Dice();
	inline char *toString() {
		return s;
	}
	inline int roll() {
		int n = 0;
		for ( int i = 0; i < count; i++ ) {
			n += Util::pickOne( 1, sides );
		}
		return n + mod;
	}
	inline int getMax() {
		return count * sides + mod;
	}
	inline int getMin() {
		return count + mod;
	}
	inline int getCount() {
		return count;
	}
	inline int getSides() {
		return sides;
	}
	inline int getMod() {
		return mod;
	}
};

class MagicSchool;

/// A magical spell.
class Spell : public Storable {
private:
	std::string name;
	std::string displayName;
	std::string sound;
	int level;
	int mp;
	int exp;
	int failureRate;
	Dice action;
	int distance;
	int targetType;
	char notes[1000];
	int speed;
	int effect;
	MagicSchool *school;
	bool creatureTarget, locationTarget, itemTarget, partyTarget, doorTarget;
	int iconTileX, iconTileY;
	bool friendly;
	int stateModPrereq;
	std::string symbol;

	static std::map<std::string, Spell*> spellMap;

public:

	Spell( char const* name, char const* displayName, char const* symbol, int level, int mp, int exp, int failureRate, char const* action,
	       int distance, int targetType, int speed, int effect, bool creatureTarget,
	       bool locationTarget, bool itemTarget, bool partyTarget, bool doorTarget,
	       MagicSchool *school,
	       int iconTileX, int iconTileY, bool friendly, int stateModPrereq );
	~Spell();

	/// The "symbol" of the spell (used in various descriptions).
	inline char const* getSymbol() {
		return symbol.c_str();
	}
	inline int getStorableType() {
		return Storable::SPELL_STORABLE;
	}
	inline const char *isStorable() {
		return NULL;
	}

	/// Is it for friendly or hostile targets?
	inline bool isFriendly() {
		return friendly;
	}
	/// Does it affect a state mod?
	inline bool hasStateModPrereq() {
		return( stateModPrereq != -1 );
	}
	/// Which state mod/potion skill is affected?
	inline int getStateModPrereq() {
		return( stateModPrereq < -1 ?
		        ( -stateModPrereq ) - 2 : // potion skill
		        stateModPrereq ); // regular state mod
	}
	/// Does it affect a "potion skill" (HP, MP, armor)?
	inline bool isStateModPrereqAPotionSkill() {
		return( stateModPrereq < -1 );
	}
	/// Icon tile within spells.png.
	inline int getIconTileX() {
		return iconTileX;
	}
	/// Icon tile within spells.png.
	inline int getIconTileY() {
		return iconTileY;
	}
	/// Unlocalized internal name of the spell.
	inline const char *getName() {
		return name.c_str();
	}
	/// Localized name of the spell.
	inline const char *getDisplayName() {
		return displayName.c_str();
	}
	/// Returns a damage roll (without further modifiers).
	inline int getAction() {
		return action.roll();
	}

	void getAttack( int casterLevel, float *maxP, float *minP );

	/// The spell's level.
	inline int getLevel()  {
		return level;
	}
	/// Magic point cost.
	inline int getMp() {
		return mp;
	}
	/// Experience gained by successfully casting the spell.
	inline int getExp() {
		return exp;
	}
	/// Probability of failure.
	inline int getFailureRate() {
		return failureRate;
	}
	/// Action radius of the spell.
	inline int getDistance() {
		return distance;
	}
	/// Which type of target is allowed?
	inline int getTargetType() {
		return targetType;
	}
	/// The spell's long description.
	inline char const* getNotes() {
		return notes;
	}
	/// How fast can it be cast?
	inline int getSpeed() {
		return speed;
	}
	/// The visual effect displayed for the spell.
	inline int getEffect() {
		return effect;
	}
	/// The spell's magic school.
	inline MagicSchool *getSchool() {
		return school;
	}
	/// Unused.
	inline bool isRangedSpell() {
		return distance > 1;
	}
	/// Short one-line description of the spell.
	inline void describe( char *s, size_t count ) {
		snprintf( s, count, _( "%s (L:%d)(M:%d)" ), displayName.c_str(), level, mp );
	}
	/// Adds a string to the long description.
	inline void addNotes( char *s ) {
		strcat( notes, s );
	}
	/// The sound to play when the spell is cast.
	inline void setSound( char const* s ) {
		sound = s;
	}
	inline char const* getSound() {
		return sound.c_str();
	}

	/// Can it target creatures?
	inline bool isCreatureTargetAllowed() {
		return creatureTarget;
	}
	/// Can it target a map location (some point on the floor etc.)?
	inline bool isLocationTargetAllowed() {
		return locationTarget;
	}
	/// Can it target items?
	inline bool isItemTargetAllowed() {
		return itemTarget;
	}
	/// Can it target a party member?
	inline bool isPartyTargetAllowed() {
		return partyTarget;
	}
	/// Can it target doors?
	inline bool isDoorTargetAllowed() {
		return doorTarget;
	}

	static Spell *getSpellByName( char *name );

};

/// A magic school.
class MagicSchool {
private:
	std::string name;
	std::string displayName;
	std::string shortName;
	std::string deity;
	char deityDescription[3000];
	float red, green, blue;
	std::string symbol;
	int skill, resistSkill;
	std::vector<Spell*> spells;
	std::vector<std::string> lowDonate, neutralDonate, highDonate;
	float baseAlignment;

	static MagicSchool *schools[10];
	static int schoolCount;
	static std::map<std::string, MagicSchool*> schoolMap;

public:
	MagicSchool( char const* name, char const* displayName, char const* deity, int skill, int resistSkill, float alignment, float red, float green, float blue, char const* symbol );
	~MagicSchool();

	/// The school's internal, unlocalized name.
	inline char const* getName() {
		return name.c_str();
	}
	/// The school's localized name.
	inline char const* getDisplayName() {
		return displayName.c_str();
	}
	inline char const* getShortName() {
		return shortName.c_str();
	}
	/// Returns the deity associated to the school.
	inline char const* getDeity() {
		return deity.c_str();
	}
	/// The associated deity's localized name.
	inline char *getDeityDescription() {
		return deityDescription;
	}
	/// The school's associated skill.
	inline int getSkill() {
		return skill;
	}
	/// The skill used to resist spells from this school.
	inline int getResistSkill() {
		return resistSkill;
	}
	/// Number of spells in this school.
	inline int getSpellCount() {
		return spells.size();
	}
	/// Returns a spell from the school by index.
	inline Spell *getSpell( int index ) {
		return spells[index];
	}
	/// Adds a string to the description of the school's associated deity.
	inline void addToDeityDescription( char *s ) {
		if ( strlen( deityDescription ) ) strcat( deityDescription, " " ); strcat( deityDescription, s );
	}
	/// The deity's color. Used to color the effect above pools for example.
	inline float getDeityRed() {
		return red;
	}
	inline float getDeityGreen() {
		return green;
	}
	inline float getDeityBlue() {
		return blue;
	}
	/// The "symbol" of the school (used in various descriptions).
	inline char const* getSymbol() {
		return symbol.c_str();
	}

	/// The base alignment (chaotic, neutral or lawful) of the school.
	inline float getBaseAlignment() {
		return baseAlignment;
	}

	static void initMagic();

	/// Number of magic schools in the game.
	inline static int getMagicSchoolCount() {
		return schoolCount;
	}
	/// Gets magic school by index.
	inline static MagicSchool *getMagicSchool( int index ) {
		return schools[index];
	}
	static Spell *getRandomSpell( int level );
	/// Gets magic school by its internal name.
	static MagicSchool *getMagicSchoolByName( char const* s ) {
		std::string name = s; return ( schoolMap.find( name ) == schoolMap.end() ? NULL : schoolMap[name] );
	}
	/// Returns a random magic school.
	static MagicSchool *getRandomSchool() {
		return getMagicSchool( getRandomSchoolIndex() );
	}
	/// Returns an index to a random magic school.
	static int getRandomSchoolIndex() {
		return Util::dice( schoolCount );
	}

	const char *getLowDonateMessage();
	const char *getNeutralDonateMessage();
	const char *getHighDonateMessage();

protected:
	/// Adds a spell to this school.
	inline void addSpell( Spell *spell ) {
		spells.push_back( spell );
	}
	const char *getRandomString( std::vector<std::string> *v );

};

#endif
