/***************************************************************************
                          spell.cpp  -  description
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
#include "spell.h"

MagicSchool *MagicSchool::schools[10];
int MagicSchool::schoolCount = 0;
map<string, Spell*> Spell::spellMap;


// FIXME: move this to some other location if needed elsewhere (rpg/dice.cpp?)
Dice::Dice(char *s) {
  this->s = s;

  char tmp[80];
  strcpy(tmp, s);
  if(strchr(tmp, 'd')) {
	count = atoi(strtok(tmp, "d"));
	sides = atoi(strtok(NULL, "+"));
	char *p = strtok(NULL, "+");
	if(p) mod = atoi(p);
	else mod = 0;
  } else {
	mod = atoi(s);
  }

  //  cerr << "DICE: count=" << count << " sides=" << sides << " mod=" << mod << " str=" << this->s << endl;
  //  cerr << "\troll 1:" << roll() << endl;
  //  cerr << "\troll 2:" << roll() << endl;
  //  cerr << "\troll 3:" << roll() << endl;
}

Dice::~Dice() {
}


MagicSchool::MagicSchool(char *name, char *deity, int skill) {
  this->name = name;
  this->deity = deity;
  this->skill = skill;
}

MagicSchool::~MagicSchool() {
}

void MagicSchool::initMagic() {
  char errMessage[500];
  char s[200];
  sprintf(s, "%s/world/spells.txt", rootDir);
  FILE *fp = fopen(s, "r");
  if(!fp) {        
	sprintf(errMessage, "Unable to find the file: %s!", s);
	cerr << errMessage << endl;
	exit(1);
  }

  MagicSchool *current = NULL;
  Spell *currentSpell = NULL;
  char name[255], notes[255], dice[255];
  char line[255];
  int n = fgetc(fp);
  while(n != EOF) {
	if( n == 'P' ) {
	  // skip ':'
	  fgetc(fp);
	  // read the rest of the line
	  n = Constants::readLine(line, fp);

	  // name, level, mana, exp, failure rate, action, distance, single/group target, notes
	  strcpy(name, strtok(line, ","));
	  int level =  atoi(strtok(NULL, ","));
	  int mp =  atoi(strtok(NULL, ","));
	  int exp =  atoi(strtok(NULL, ","));
	  int failureRate = atoi(strtok(NULL, ","));
	  strcpy(dice, strtok(NULL, ","));
	  int distance = atoi(strtok(NULL, ","));
	  int targetType = (strcmp(strtok(NULL, ","), "single") ? 
						Spell::SINGLE_TARGET : Spell::GROUP_TARGET);
	  int speed = atoi(strtok(NULL, ","));

	  if(!current) {
		cerr << "*** ignoring spell: " << name << " because no school of magic was specified." << endl;
		continue;
	  }


	  Dice *action = new Dice(strdup(dice));

	  cerr << "adding spell: " << name << " level: " << level << " mp: " << mp << 
		" exp: " << exp << " failureRate: " << failureRate << 
		" action: " << action->toString() << " distance: " << distance << 
		" targetType: " << targetType << endl;
	  
	  currentSpell = new Spell( strdup(name), level, mp, exp, failureRate, 
								action, distance, targetType, speed, current );
	  current->addSpell( currentSpell );
	} else if( n == 'D' && currentSpell ) {
	  fgetc(fp);
	  n = Constants::readLine(line, fp);
	  currentSpell->addNotes(" ");
	  currentSpell->addNotes(line);
	} else if( n == 'S' ) {
	  fgetc(fp);
	  n = Constants::readLine(line, fp);

	  strcpy(name, strtok(line, ","));
	  strcpy(notes, strtok(NULL, ","));
	  int skill = Constants::getSkillByName(strtok(NULL, ","));


	  cerr << "adding school: " << name << " provider deity: " << notes << endl;

	  current = new MagicSchool( strdup(name), strdup(notes), skill );
	  schools[schoolCount++] = current;
	} else {
	  n = Constants::readLine(line, fp);
	}
  }
  fclose(fp);
}



Spell::Spell(char *name, int level, int mp, int exp, int failureRate, Dice *action, 
			 int distance, int targetType, int speed, MagicSchool *school) {
  this->name = name;
  this->level = level;
  this->mp = mp;
  this->exp = exp;
  this->failureRate = failureRate;
  this->action = action;
  this->distance = distance;
  this->targetType = targetType;
  this->speed = speed;
  this->school = school;

  strcpy(this->notes, "");

  string s = name;
  spellMap[s] = this;
}

Spell::~Spell() {
}

Spell *Spell::getSpellByName(char *name) {
  string s = name;
  if(spellMap.find(s) == spellMap.end()) {
	cerr << "ERROR: can't find spell: " << name << endl;
	exit(1);
  }
  return spellMap[s];
}
