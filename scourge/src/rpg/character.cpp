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

char Character::inventory_location[][80] = {
  "head",
  "neck",
  "back",
  "chest",
  "left hand",
  "right hand",
  "belt",
  "legs",
  "feet",
  "ring1",
  "ring2",
  "ring3",
  "ring4",
  "ranged weapon"
};

map<string, Character*> Character::character_class;

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
  char name[255], model[255], skin[255];
  char line[255];
  int n = fgetc(fp);
  while(n != EOF) {
	if(n == 'C') {
	  // skip ':'
	  fgetc(fp);
	  // read the rest of the line
	  n = Constants::readLine(line, fp);

	  strcpy(name, strtok(line, ","));
	  strcpy(model, strtok(NULL, ","));
	  strcpy(skin, strtok(NULL, ","));
	  int hp =  atoi(strtok(NULL, ","));
	  int skill_bonus =  atoi(strtok(NULL, ","));
	  int level_progression = atoi(strtok(NULL, ","));

	  cerr << "adding character class: " << name << " model: " << model << 
		" skin: " << skin << " hp: " << hp << " skill_bonus: " << skill_bonus << endl;

	  last = new Character( strdup(name), hp, strdup(model), strdup(skin), skill_bonus, level_progression );
	  string s = name;
	  character_class[s] = last;
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
	} else {
	  n = Constants::readLine(line, fp);
	}
  }
  fclose(fp);
}

Character::Character(char *name, int startingHp, char *model, char *skin, int skill_bonus, int level_progression ) {  
  this->name = name;
  this->startingHp = startingHp;
  this->model_name = model;
  this->skin_name = skin;
  this->skill_bonus = skill_bonus;
  this->level_progression = level_progression;
  strcpy(description, "");
}

Character::~Character(){  
}
