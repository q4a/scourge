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
map<string, MagicSchool*> MagicSchool::schoolMap;


// FIXME: move this to some other location if needed elsewhere (rpg/dice.cpp?)
Dice::Dice(char *s) {
  this->s = s;
  frees = false;

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

Dice::Dice(int count, int sides, int mod) {
  this->count = count;
  this->sides = sides;
  this->mod = mod;
  s = (char*)malloc(80);
  sprintf(s, "%dd%d", count, sides);
  if(mod) {
    // FIXME: handle negative numbers
    sprintf(s, "+%d", mod);
  }
  frees = true;
}

Dice::~Dice() {
  if(frees) free(s);
}




MagicSchool::MagicSchool(char *name, char *deity, int skill, int resistSkill) {
  this->name = name;
  this->shortName = strdup(name);
  shortName = strtok(shortName, " ");
  this->deity = deity;
  strcpy( this->deityDescription, "" );
  this->skill = skill;
  this->resistSkill = resistSkill;
}

MagicSchool::~MagicSchool() {
  free(shortName);
  shortName = NULL;
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
    if(distance < (int)Constants::MIN_DISTANCE) distance = (int)Constants::MIN_DISTANCE;
	  int targetType = (!strcmp(strtok(NULL, ","), "single") ? 
						SINGLE_TARGET : GROUP_TARGET);
	  int speed = atoi(strtok(NULL, ","));
	  int effect = Constants::getEffectByName(strtok(NULL, ","));
      //cerr << "*** looking up: " << s << " effect=" << effect << endl;
	  char *s = strtok(NULL, ",");
	  bool creatureTarget = (strchr(s, 'C') != NULL);
	  bool locationTarget = (strchr(s, 'L') != NULL);
	  bool itemTarget = (strchr(s, 'I') != NULL);
      bool partyTarget = (strchr(s, 'P') != NULL);
	  int iconTileX = atoi( strtok( NULL, "," ) ) - 1;
	  int iconTileY = atoi( strtok( NULL, "," ) ) - 1;

    // Friendly/Hostile marker
    char *fh = strtok( NULL, "," );
    bool friendly = false;
    int stateModPrereq = -1;
    if( fh ) {
      friendly = ( *fh == 'F' );
      char *prereq = strtok( NULL, "," );
      if( prereq ) {
        // is it a potion state mod?
        int n = Constants::getPotionSkillByName( prereq );
        if( n == -1 ) {
          n = Constants::getStateModByName( (const char*)prereq );
        }
        stateModPrereq = n;
        if( stateModPrereq == -1 ) {
          cerr << "Error: spell=" << name << endl;
          cerr << "\tCan't understand prereq for spell: " << prereq << endl;
        }
      }
    }

	  if(!current) {
      cerr << "*** ignoring spell: " << name << " because no school of magic was specified." << endl;
      continue;
	  }


	  Dice *action = new Dice(strdup(dice));

      /*
	  cerr << "adding spell: " << name << " level: " << level << " mp: " << mp << 
		" exp: " << exp << " failureRate: " << failureRate << 
		" action: " << action->toString() << " distance: " << distance << 
		" targetType: " << targetType << endl;
      */          
	  
	  currentSpell = new Spell( strdup(name), level, mp, exp, failureRate, 
                              action, distance, targetType, speed, effect, 
                              creatureTarget, locationTarget, itemTarget, partyTarget,
                              current, iconTileX, iconTileY, 
                              friendly, stateModPrereq );
	  current->addSpell( currentSpell );
	} else if( n == 'W' && currentSpell ) {
	  fgetc(fp);
	  n = Constants::readLine(line, fp);
	  currentSpell->setSound( strdup(line) );
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
	  int resistSkill = Constants::getSkillByName(strtok(NULL, ","));


	  //cerr << "adding school: " << name << " provider deity: " << notes << " skill=" << skill << " resist skill=" << resistSkill << endl;

	  current = new MagicSchool( strdup(name), strdup(notes), skill, resistSkill );
	  schools[schoolCount++] = current;
    string nameStr = name;
    schoolMap[nameStr] = current;
  } else if( n == 'G' && current ) {
    n = Constants::readLine(line, fp);
    current->addToDeityDescription( line + 1 );
  } else if( n == 'L' && current ) {
    fgetc(fp);
	  n = Constants::readLine(line, fp);
    current->lowDonate.push_back( line );
  } else if( n == 'N' && current ) {
    fgetc(fp);
	  n = Constants::readLine(line, fp);
    current->neutralDonate.push_back( line );
  } else if( n == 'H' && current ) {
    fgetc(fp);
	  n = Constants::readLine(line, fp);
    current->highDonate.push_back( line );  
	} else {
	  n = Constants::readLine(line, fp);
  }
  }
  fclose(fp);
}

const char *MagicSchool::getRandomString( vector<string> *v ) {
  return (*v)[ (int)( (float)( v->size() ) * rand()/RAND_MAX ) ].c_str();
}

const char *MagicSchool::getLowDonateMessage() {
  return getRandomString( &lowDonate );
}

const char *MagicSchool::getNeutralDonateMessage() {
  return getRandomString( &neutralDonate );
}

const char *MagicSchool::getHighDonateMessage() {
  return getRandomString( &highDonate );
}

Spell *MagicSchool::getRandomSpell(int level) {
  int n = (int)((float)getMagicSchoolCount() * rand()/RAND_MAX);
  int c = getMagicSchool(n)->getSpellCount();
  if(c > 0) {
    return getMagicSchool(n)->getSpell((int)((float)c * rand()/RAND_MAX));
  } else {
    return NULL;
  }
}



Spell::Spell(char *name, int level, int mp, int exp, int failureRate, Dice *action, 
			 int distance, int targetType, int speed, int effect, bool creatureTarget, 
			 bool locationTarget, bool itemTarget, bool partyTarget, MagicSchool *school,
			 int iconTileX, int iconTileY, bool friendly, int stateModPrereq ) {
  this->name = name;
  this->sound = NULL;
  this->level = level;
  this->mp = mp;
  this->exp = exp;
  this->failureRate = failureRate;
  this->action = action;
  this->distance = distance;
  this->targetType = targetType;
  this->speed = speed;
  this->effect = effect;
  this->creatureTarget = creatureTarget; 
  this->locationTarget = locationTarget;
  this->itemTarget = itemTarget;
  this->partyTarget = partyTarget;
  this->school = school;
  this->iconTileX = iconTileX;
  this->iconTileY = iconTileY;
  this->friendly = friendly;
  this->stateModPrereq = stateModPrereq;

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


