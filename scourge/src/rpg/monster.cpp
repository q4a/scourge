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

Monster::Monster(char *type, int level, int hp, int mp, char *model, char *skin, int rareness, int speed, int baseArmor, float scale, int w, int d, int h) {
  this->type = type;
  this->level = level;
  this->hp = hp;
  this->mp = mp;
  this->model_name = model;
  this->skin_name = skin;
  this->speed = speed;
  this->rareness = rareness;
  this->baseArmor = baseArmor;
  this->scale = scale;
  this->w = w;
  this->d = d;
  this->h = h;
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
      int hp = atoi(strtok(NULL, ","));
      int mp = atoi(strtok(NULL, ","));
      int armor = atoi(strtok(NULL, ","));
      int rareness = atoi(strtok(NULL, ","));
      int speed = atoi(strtok(NULL, ","));

      float scale = 0.0f;
      int width = 0;
      int depth = 0;
      int height = 0;
      char *p = strtok(NULL, ",");
      if(p) {
        scale = atof(p);
        width = atoi(strtok(NULL, ","));
        depth = atoi(strtok(NULL, ","));
        height = atoi(strtok(NULL, ","));
      }

      cerr << "adding monster: " << name << " level: " << level << 
        " hp: " << hp << " mp: " << mp << " armor: " << armor << 
        " rareness: " << rareness << " scale=" << scale << 
        " width=" << width << " depth=" << depth << " height=" << height << 
        " speed=" << speed <<
        endl;

      vector<Monster*> *list = NULL;
      if(monsters.find(level) == monsters.end()) {
        list = new vector<Monster*>();
        monsters[level] = list;
      } else {
        list = monsters[level];
      }
      Monster *m = new Monster( strdup(name), level, hp, mp, 
                                strdup(model_name), strdup(skin_name), 
                                rareness, speed, armor, 
                                scale, width, depth, height );
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
    } else if(n == 'S' && last_monster) {
      // skip ':'
      fgetc(fp);
      // read the rest of the line
      n = Constants::readLine(line, fp);
      // add spell to monster
      last_monster->addSpell(Spell::getSpellByName(line));
    } else {
      n = Constants::readLine(line, fp);
    }
  }
  fclose(fp);
}

Monster *Monster::getRandomMonster(int level) {
  if(monsters.find(level) == monsters.end()) return NULL;
  vector<Monster*> *list = monsters[level];

  // create a new list where each monster occurs monster->rareness times
  vector<Monster*> rareList;
  for(int i = 0; i < (int)list->size(); i++) {
    Monster *monster = (*list)[i];
    for(int t = 0; t < monster->getRareness(); t++) {
      rareList.push_back(monster);
    }
  }

  // select a random entry from the rare list
  int index = (int) ((float)(rareList.size()) * rand()/RAND_MAX);
  return rareList[index];
}

Monster *Monster::getMonsterByName(char *name) {
  string s = name;
  if(monstersByName.find(s) == monstersByName.end()) {
    cerr << "Warning: can't find monster " << name << endl;
    return NULL;
  }
  return monstersByName[s];
}
