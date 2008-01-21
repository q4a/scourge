/***************************************************************************
                          spell.h  -  description
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

#include <map>
#include <string>
#include <vector>
#include "../common/constants.h"
#include "../storable.h"
#include "../util.h"

class Dice {
private:
	enum { S_SIZE = 80 };
	char s[ S_SIZE ];
  int count;
  int sides;
  int mod;

public:
  Dice(char *s);
  Dice(int count, int sides, int mod);
  ~Dice();
  inline char *toString() { return s;}
  inline int roll() { 
    int n = 0;
    for ( int i = 0; i < count; i++ ) {
      n += Util::pickOne( 1, sides );
	}
    return n + mod;
  }
  inline int getMax() { return count * sides + mod; }
  inline int getMin() { return count + mod; }
  inline int getCount() { return count; }
  inline int getSides() { return sides; }
  inline int getMod() { return mod; }
};

class MagicSchool;

class Spell : public Storable {
 private:
  char *name;
  char *displayName;
  char *sound;
  int level;
  int mp;
  int exp;
  int failureRate;
  Dice *action;
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
  char *symbol;

  static std::map<std::string, Spell*> spellMap;

 public:

  Spell(char *name, char *displayName, char *symbol, int level, int mp, int exp, int failureRate, Dice *action, 
        int distance, int targetType, int speed, int effect, bool creatureTarget, 
        bool locationTarget, bool itemTarget, bool partyTarget, bool doorTarget,
				MagicSchool *school,
        int iconTileX, int iconTileY, bool friendly, int stateModPrereq );
  ~Spell();

  inline char *getSymbol() { return symbol; }
  inline int getStorableType() { return Storable::SPELL_STORABLE; }
  inline const char *isStorable() { return NULL; }

  inline bool isFriendly() { return friendly; }
  inline bool hasStateModPrereq() { return( stateModPrereq != -1 ); }
  inline int getStateModPrereq() { 
    return( stateModPrereq < -1 ? 
            ( -stateModPrereq ) - 2 : // potion skill
            stateModPrereq ); // regular state mod
  }
  inline bool isStateModPrereqAPotionSkill() { return( stateModPrereq < -1 ); }
  inline int getIconTileX() { return iconTileX; }
  inline int getIconTileY() { return iconTileY; }
  inline const char *getName() { return (const char*)name; }
  inline const char *getDisplayName() { return (const char*)displayName; }
  inline int getAction() { return action->roll(); }
  inline int getLevel()  { return level; }
  inline int getMp() { return mp; }
  inline int getExp() { return exp; }
  inline int getFailureRate() { return failureRate; }
  inline int getDistance() { return distance; }
  inline int getTargetType() { return targetType; }  
  inline char *getNotes() { return notes; }
  inline int getSpeed() { return speed; }
  inline int getEffect() { return effect; }
  inline MagicSchool *getSchool() { return school; }
  inline bool isRangedSpell() { return distance > 1; }  
  inline void describe(char *s, size_t count) { snprintf(s, count, "%s (L:%d)(M:%d)", name, level, mp); }
  inline void addNotes(char *s) { strcat(notes, s); }
  inline void setSound(char *s) { sound = s; }
  inline char *getSound() { return sound; }

  // what kind of target is allowed for this spell
  inline bool isCreatureTargetAllowed() { return creatureTarget; }
  inline bool isLocationTargetAllowed() { return locationTarget; }
  inline bool isItemTargetAllowed() { return itemTarget; }
  inline bool isPartyTargetAllowed() { return partyTarget; }
	inline bool isDoorTargetAllowed() { return doorTarget; }

  static Spell *getSpellByName(char *name);

};



class MagicSchool {
 private:
  char *name;
  char *displayName;
  char *shortName;
  char *deity;
  char deityDescription[3000];
  float red, green, blue;
  char *symbol;
  int skill, resistSkill;
  std::vector<Spell*> spells;
  std::vector<std::string> lowDonate, neutralDonate, highDonate;

  static MagicSchool *schools[10];
  static int schoolCount;
  static std::map<std::string, MagicSchool*> schoolMap;

 public:
  MagicSchool(char *name, char *displayName, char *deity, int skill, int resistSkill, float red, float green, float blue, char *symbol);
  ~MagicSchool();

  inline char *getName() { return name; }
  inline char *getDisplayName() { return displayName; }
  inline char *getShortName() { return shortName; }
  inline char *getDeity() { return deity; }
  inline char *getDeityDescription() { return deityDescription; }
  inline int getSkill() { return skill; }
  inline int getResistSkill() { return resistSkill; }
  inline int getSpellCount() { return spells.size(); }
  inline Spell *getSpell(int index) { return spells[index]; }
  inline void addToDeityDescription( char *s ) { if( strlen( deityDescription ) ) strcat( deityDescription, " " ); strcat( deityDescription, s ); }
  inline float getDeityRed() { return red; }
  inline float getDeityGreen() { return green; }
  inline float getDeityBlue() { return blue; }
  inline char *getSymbol() { return symbol; }

  static void initMagic();
  inline static int getMagicSchoolCount() { return schoolCount; }
  inline static MagicSchool *getMagicSchool( int index ) { return schools[index]; }
  static Spell *getRandomSpell(int level);
  static MagicSchool *getMagicSchoolByName( char *s ) { std::string name = s; return (schoolMap.find(name) == schoolMap.end() ? NULL : schoolMap[name]); }
  static MagicSchool *getRandomSchool() { return getMagicSchool( getRandomSchoolIndex() ); }
	static int getRandomSchoolIndex() { return Util::dice( schoolCount ); }

  const char *getLowDonateMessage();
  const char *getNeutralDonateMessage();
  const char *getHighDonateMessage();

 protected:
  inline void addSpell( Spell *spell ) { spells.push_back( spell ); }
  const char *getRandomString( std::vector<std::string> *v );

};

#endif
