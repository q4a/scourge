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

using namespace std;

class Dice {
 private:
  char *s;
  int count;
  int sides;
  int mod;

 public:
  Dice(char *s);
  ~Dice();
  inline char *toString() { return s; }
  inline int roll() { return ((int)((float)sides * rand()/RAND_MAX) * count) + mod; }
};

class MagicSchool;

class Spell {
 private:
  char *name;
  int level;
  int mp;
  int exp;
  int failureRate;
  Dice *action;
  int distance;
  int targetType;
  char notes[1000];
  int speed;
  bool defaultTarget;
  MagicSchool *school;  

  static map<string, Spell*> spellMap;

 public:

  Spell(char *name, int level, int mp, int exp, int failureRate, Dice *action, 
		int distance, int targetType, int speed, bool defaultTarget, MagicSchool *school);
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
  inline bool useDefaultTarget() { return defaultTarget; }
  inline MagicSchool *getSchool() { return school; }
  inline bool isRangedSpell() { return distance > 1; }  
  inline void describe(char *s) { sprintf(s, "%s (L:%d)(M:%d)", name, level, mp); }
  inline void addNotes(char *s) { strcat(notes, s); }

  static Spell *getSpellByName(char *name);
};



class MagicSchool {
 private:
  char *name;
  char *deity;
  int skill;
  vector<Spell*> spells;

  static MagicSchool *schools[10];
  static int schoolCount;

 public:
  MagicSchool(char *name, char *deity, int skill);
  ~MagicSchool();

  inline char *getName() { return name; }
  inline char *getDeity() { return deity; }
  inline int getSkill() { return skill; }
  inline int getSpellCount() { return spells.size(); }
  inline Spell *getSpell(int index) { return spells[index]; }

  static void initMagic();
  inline static int getMagicSchoolCount() { return schoolCount; }
  inline static MagicSchool *getMagicSchool( int index ) { return schools[index]; }

 protected:
  inline void addSpell( Spell *spell ) { spells.push_back( spell ); }
};

#endif
