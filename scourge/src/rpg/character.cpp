/***************************************************************************
                          character.cpp  -  description
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

#include "character.h"

using namespace std;

map<string, Character*> Character::character_class;
map<string, Character*> Character::character_class_short;
map<string, int> Character::character_index_short;
vector<Character*> Character::character_list;

void Character::initCharacters() {
  char errMessage[500];
  char s[200];
  sprintf(s, "%s/world/characters.txt", rootDir);
  FILE *fp = fopen(s, "r");
  if(!fp) {        
    sprintf(errMessage, "Unable to find the file: %s!", s);
    cerr << errMessage << endl;
    exit(1);
  }

  Character *last = NULL;
  char name[255];
  char line[255], shortName[10];
  int index = 0;
  int n = fgetc(fp);
  while(n != EOF) {
    if(n == 'C') {
      // skip ':'
      fgetc(fp);
      // read the rest of the line
      n = Constants::readLine(line, fp);

      strcpy(name, strtok(line, ","));
      int hp =  atoi(strtok(NULL, ","));
      int mp =  atoi(strtok(NULL, ","));
      int skill_bonus =  atoi(strtok(NULL, ","));
      int level_progression = atoi(strtok(NULL, ","));
      strcpy(shortName, strtok(NULL, ","));
      float baseAttackBonus = (float)atof( strtok( NULL, "," ) );
      int extraAttackLevel = atoi(strtok(NULL, ","));

      /*
      cerr << "adding character class: " << name << 
      " hp: " << hp << " mp: " << mp << " skill_bonus: " << 
      skill_bonus << " shortName=" << shortName << endl;
      */

      last = new Character( strdup(name), hp, mp, skill_bonus, 
                            level_progression, strdup(shortName),
                            baseAttackBonus, extraAttackLevel );
      string s = name;
      character_class[s] = last;
      string s2 = shortName;
      character_class_short[s2] = last;
      character_index_short[s2] = index++;
      character_list.push_back(last);
    } else if(n == 'D' && last) {
      fgetc(fp);
      n = Constants::readLine(line, fp);
      if(strlen(last->description)) strcat(last->description, " ");
      strcat(last->description, strdup(line));
    } else if(n == 'S' && last) {
      fgetc(fp);
      n = Constants::readLine(line, fp);
      strcpy(name, strtok(line, ","));
      char *p = strtok(NULL, ",");
      int min = 0;
      if(strcmp(p, "*")) min = atoi(p);
      p = strtok(NULL, ",");
      int max = 100;
      if(strcmp(p, "*")) max = atoi(p);
      int skill = Constants::getSkillByName(name);
      if(skill < 0) {
        cerr << "*** WARNING: cannot find skill: " << name << endl;   
      } else {    
        last->setMinMaxSkill(skill, min, max);
      }
    } else if(n == 'c' && last) {
      fgetc(fp);
      n = Constants::readLine(line, fp);
      addSounds(Constants::SOUND_TYPE_COMMAND, line, last);
    } else if(n == 'h' && last) {
      fgetc(fp);
      n = Constants::readLine(line, fp);
      addSounds(Constants::SOUND_TYPE_HIT, line, last);
    } else if(n == 's' && last) {
      fgetc(fp);
      n = Constants::readLine(line, fp);
      addSounds(Constants::SOUND_TYPE_SELECT, line, last);
    } else if(n == 'a' && last) {
      fgetc(fp);
      n = Constants::readLine(line, fp);
      addSounds(Constants::SOUND_TYPE_ATTACK, line, last);
    } else {
      n = Constants::readLine(line, fp);
    }
  }
  fclose(fp);
}

void Character::addSounds(int type, char *line, Character *c) {
  char *p = strtok(line, ",");
  while(p) {
    c->addSound(type, strdup(p));
    p = strtok(NULL, ",");
  }
}

Character::Character(char *name, int startingHp, int startingMp, 
                     int skill_bonus, int level_progression, char *shortName,
                     float baseAttackBonus, int extraAttackLevel ) {  
  this->name = name;
  this->startingHp = startingHp;
  this->startingMp = startingMp;
  this->skill_bonus = skill_bonus;
  this->level_progression = level_progression;
  this->shortName = shortName;
  this->baseAttackBonus = baseAttackBonus;
  this->extraAttackLevel = extraAttackLevel;
  strcpy(description, "");
}

Character::~Character(){  
}

void Character::addSound(int type, char *file) {
  //cerr << "*** Adding sound=" << file << endl;
  string fileStr = file;
  vector<string> *sounds;
  if(soundMap.find(type) == soundMap.end()) {
    sounds = new vector<string>;
    soundMap[type] = sounds;
  } else sounds = soundMap[type];
  sounds->push_back(fileStr);
  //cerr << "\t*** Done." << endl;
}

const char *Character::getRandomSound(int type) {
  vector<string> *sounds = NULL;
  if(soundMap.find(type) != soundMap.end()) {
    sounds = soundMap[type];
  }
  if(!sounds || !(sounds->size())) return NULL;
  string s = (*sounds)[(int)((float)(sounds->size()) * rand()/RAND_MAX)];
  return s.c_str();
}

