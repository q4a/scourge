/***************************************************************************
                          monster.cpp  -  description
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
#include "monster.h"

/**
  *@author Gabor Torok
  */

map<int, vector<Monster*>* > Monster::monsters;
map<string, Monster*> Monster::monstersByName;

Monster::Monster(char *type, int level, int hp, char *model, char *skin, int baseArmor) {
  this->type = type;
  this->level = level;
  this->hp = hp;
  this->model_name = model;
  this->skin_name = skin;
  speed = 50;
  this->baseArmor = baseArmor;
  sprintf(description, "FIXME: need a description");
}

Monster::~Monster() {
}

void Monster::initMonsters() {
  char errMessage[500];
  char s[200];
  sprintf(s, "%s/world/creatures.txt", rootDir);
  FILE *fp = fopen(s, "r");
  if(!fp) {        
	sprintf(errMessage, "Unable to find the file: %s!", s);
	cerr << errMessage << endl;
	exit(1);
  }

  //int itemCount = 0;
  Monster *last_monster = NULL;
  char name[255], model_name[255], skin_name[255];
  char line[255];
  int n = fgetc(fp);
  while(n != EOF) {
	if(n == 'M') {
	  // skip ':'
	  fgetc(fp);
	  // read the rest of the line
	  n = Constants::readLine(name, fp);
	  n = Constants::readLine(line, fp);

	  strcpy(model_name, strtok(line + 1, ","));
	  strcpy(skin_name, strtok(NULL, ","));
	  int level = atoi(strtok(NULL, ","));
	  int hp =  atoi(strtok(NULL, ","));
	  int armor =  atoi(strtok(NULL, ","));

	  cerr << "adding monster: " << name << " level: " << level << 
		" hp: " << hp << " armor: " << armor << endl;

	  vector<Monster*> *list = NULL;
	  if(monsters.find(level) == monsters.end()) {
		list = new vector<Monster*>();
		monsters[level] = list;
	  } else {
		list = monsters[level];
	  }
	  Monster *m = new Monster( strdup(name), level, hp, strdup(model_name), strdup(skin_name), armor );
	  last_monster = m;
	  list->push_back(last_monster);
	  string s = name;
	  monstersByName[s] = m;
	} else if(n == 'I' && last_monster) {
	  // skip ':'
	  fgetc(fp);
	  // read the rest of the line
	  n = Constants::readLine(line, fp);
	  // add item to monster
	  last_monster->addItem(RpgItem::getItemByName(line));
	} else {
	  n = Constants::readLine(line, fp);
	}
  }
  fclose(fp);
}

Monster *Monster::getRandomMonster(int level) {
  if(monsters.find(level) == monsters.end()) return NULL;
  vector<Monster*> *list = monsters[level];
  int index = (int) ((float)(list->size()) * rand()/RAND_MAX);
  return (*list)[index];
}

Monster *Monster::getMonsterByName(char *name) {
  string s = name;
  if(monstersByName.find(s) == monstersByName.end()) {
	cerr << "Warning: can't find monster " << name << endl;
	return NULL;
  }
  return monstersByName[s];
}
