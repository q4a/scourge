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
#include "../constants.h"
#include "../persist.h"

using namespace std;

class Dice {
private:
  char *s;
  int count;
  int sides;
  int mod;
  bool frees;

public:
  Dice(char *s);
  Dice(int count, int sides, int mod);
  ~Dice();
  inline char *toString() { return s;}
  inline int roll() { 
    float n = (float)sides * rand()/RAND_MAX;
    n *= count;
    n += mod;
    return(int)n;
  }

  DiceInfo *save();
  static DiceInfo *saveEmpty();
  static Dice *load(Session *session, DiceInfo *info);
};

class MagicSchool;

class Spell {
 private:
  char *name;
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
  bool creatureTarget, locationTarget, itemTarget, partyTarget;

  static map<string, Spell*> spellMap;

 public:

  Spell(char *name, int level, int mp, int exp, int failureRate, Dice *action, 
		int distance, int targetType, int speed, int effect, bool creatureTarget, 
		bool locationTarget, bool itemTarget, bool partyTarget, MagicSchool *school);
  ~Spell();

  inline char *getName() { return name; }
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
  inline void describe(char *s) { sprintf(s, "%s (L:%d)(M:%d)", name, level, mp); }
  inline void addNotes(char *s) { strcat(notes, s); }
  inline void setSound(char *s) { sound = s; }
  inline char *getSound() { return sound; }

  // what kind of target is allowed for this spell
  inline bool isCreatureTargetAllowed() { return creatureTarget; }
  inline bool isLocationTargetAllowed() { return locationTarget; }
  inline bool isItemTargetAllowed() { return itemTarget; }
  inline bool isPartyTargetAllowed() { return partyTarget; }

  static Spell *getSpellByName(char *name);
};



class MagicSchool {
 private:
  char *name;
  char *shortName;
  char *deity;
  char deityDescription[3000];
  int skill, resistSkill;
  vector<Spell*> spells;

  static MagicSchool *schools[10];
  static int schoolCount;
  static map<string, MagicSchool*> schoolMap;

 public:
  MagicSchool(char *name, char *deity, int skill, int resistSkill);
  ~MagicSchool();

  inline char *getName() { return name; }
  inline char *getShortName() { return shortName; }
  inline char *getDeity() { return deity; }
  inline char *getDeityDescription() { return deityDescription; }
  inline int getSkill() { return skill; }
  inline int getResistSkill() { return resistSkill; }
  inline int getSpellCount() { return spells.size(); }
  inline Spell *getSpell(int index) { return spells[index]; }
  inline void addToDeityDescription( char *s ) { if( strlen( deityDescription ) ) strcat( deityDescription, " " ); strcat( deityDescription, s ); }

  static void initMagic();
  inline static int getMagicSchoolCount() { return schoolCount; }
  inline static MagicSchool *getMagicSchool( int index ) { return schools[index]; }
  static Spell *getRandomSpell(int level);
  static MagicSchool *getMagicSchoolByName( char *s ) { string name = s; return (schoolMap.find(name) == schoolMap.end() ? NULL : schoolMap[name]); }

 protected:
  inline void addSpell( Spell *spell ) { spells.push_back( spell ); }
};

#endif
