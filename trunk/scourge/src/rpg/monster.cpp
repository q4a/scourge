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
map<string, map<int, vector<string>*>*> Monster::soundMap;
map<int, vector<string>*>* Monster::currentSoundMap;
vector<string> Monster::monsterTypes;
vector<Monster*> Monster::npcs;
map<string, Monster*> Monster::npcPos;
map<string, string> Monster::modelToDescriptiveType;

Monster::Monster(char *type, char* descriptiveType, int level, int hp, int mp, char *model, char *skin, int rareness, int speed, int baseArmor, float scale, bool npc, char *portrait) {
  this->type = type;
  if( descriptiveType ) {
    string modelStr = model;
    string descriptiveTypeStr = descriptiveType;
    modelToDescriptiveType[ modelStr ] = descriptiveTypeStr;
  }
  this->level = level;
  this->hp = hp;
  this->mp = mp;
  this->model_name = model;
  this->skin_name = skin;
  this->speed = speed;
  this->rareness = rareness;
  this->baseArmor = baseArmor;
  this->scale = scale;
  this->npc = npc;
  this->portrait = portrait;
  this->portraitTexture = 0;
  sprintf(description, "FIXME: need a description");
}

Monster::~Monster() {
}

// O(n)! Used only while saving
bool Monster::getIndexOrFindByIndex(Monster **monster, int *index) {
  int current_index = 0;
  for(int level = 0; level < (int)monsters.size(); level++) {
    if(monsters.find(level) != monsters.end()) {
      vector<Monster*> *list = monsters[level];
      for(int i = 0; i < (int)list->size(); i++, current_index++) {
        if(*monster && (*list)[i] == *monster) {
          *index = current_index;
          return true;
        }
        if(!(*monster) && current_index == *index) {
          *monster = (*list)[i];
          return true;
        }
      }
    }
  }
  return false;
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
  char line[255], portrait[255];
  int n = fgetc(fp);
  while(n != EOF) {
    if(n == 'M') {
      // skip ':'
      fgetc(fp);
      // read the rest of the line
      n = Constants::readLine(line, fp);
      strcpy( name, strtok( line, "," ) );
      char *p = strtok( NULL, "," );
      if( p ) {
        strcpy( portrait, p );
      } else {
        strcpy( portrait, "" );
      }

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
      p = strtok(NULL, ",");
      if(p) {
        scale = atof(p);
      }
      bool npc = false;
      int npcStartX = -1;
      int npcStartY = -1;
      p = strtok(NULL, ",");
      if(p) {
        npc = true;
        p = strtok( NULL, "," );
        if(p) {
          npcStartX = atoi(p);
          p = strtok( NULL, "," );
          if(p) {
            npcStartY = atoi(p);
          }
        }
      }

      /*
      cerr << "adding monster: " << name << " level: " << level << 
        " hp: " << hp << " mp: " << mp << " armor: " << armor << 
        " rareness: " << rareness << " scale=" << scale << 
        " speed=" << speed <<
        endl;
        */

      // HACK: for monster-s portrait is descriptive type
      // for npc-s it's the portrait path.
      Monster *m;
      if( npc ) {
        m = new Monster( strdup(name), NULL, level, hp, mp, 
                         strdup(model_name), strdup(skin_name), 
                         rareness, speed, armor, 
                         scale, npc, 
                         ( strlen( portrait ) ? strdup( portrait ) : NULL ) );
      } else {
        m = new Monster( strdup(name), 
                         ( strlen( portrait ) ? strdup( portrait ) : NULL ),
                         level, hp, mp, 
                         strdup(model_name), strdup(skin_name), 
                         rareness, speed, armor, 
                         scale, npc, NULL );
      }
      last_monster = m;
      if( npc ) {
        if( npcStartX > -1 && npcStartY > -1 ) {
          char tmp[80];
          sprintf( tmp, "%d,%d", npcStartX, npcStartY );
          string pos = tmp;
          npcPos[ pos ] = m;
        } else {
          npcs.push_back( m );
        }
      } else {
        vector<Monster*> *list = NULL;
        if(monsters.find(level) == monsters.end()) {
          list = new vector<Monster*>();
          monsters[level] = list;
        } else {
          list = monsters[level];
        }
        list->push_back(last_monster);
      }
      string s = name;
      monstersByName[s] = m;

      // store type and add sounds
      string typeStr = m->getModelName();
      bool found = false;
      for(int i = 0; i < (int)monsterTypes.size(); i++) {
        if(!strcmp(monsterTypes[i].c_str(), m->getModelName())) {
          found = true;
          break;
        }
      }
      if(!found) {
        if( !npc ) monsterTypes.push_back(typeStr);
        
        if( soundMap.find( typeStr ) == soundMap.end() ) {
          currentSoundMap = new map<int, vector<string>*>();      
          soundMap[typeStr] = currentSoundMap;
        } else {
          currentSoundMap = soundMap[typeStr];
        }
        addMd2Sounds( m->getModelName(), currentSoundMap );
      }      
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
    } else if(n == 'W') {
      fgetc(fp);
      n = Constants::readLine(line, fp);

	  string monsterType_str = line;
	  if( soundMap.find( monsterType_str ) == soundMap.end() ) {
		currentSoundMap = new map<int, vector<string>*>();      
		soundMap[monsterType_str] = currentSoundMap;
	  } else {
		currentSoundMap = soundMap[monsterType_str];
	  }      
    } else if(n == 'a' && currentSoundMap) {	  
      fgetc(fp);
      n = Constants::readLine(line, fp);
	  addSound( Constants::SOUND_TYPE_ATTACK, line, currentSoundMap );
    } else if(n == 'h' && currentSoundMap) {
      fgetc(fp);
      n = Constants::readLine(line, fp);
	  addSound( Constants::SOUND_TYPE_HIT, line, currentSoundMap );
    } else if(n == 'P' && last_monster) {
      // skip ':'
      fgetc(fp);
      // read the rest of the line
      n = Constants::readLine(line, fp);
      // add item to monster
      char *p = strtok(line, ",");
      string skillStr = strdup(p);
      int n = atoi(strtok(NULL, ","));
      last_monster->skills[skillStr] = n;
      //cerr << "\tsetting skill level: " << skillStr << "=" << n << endl;
    } else {
      n = Constants::readLine(line, fp);
    }
  }
  fclose(fp);
}

void Monster::addMd2Sounds( char *model_name, map<int, vector<string>*>* currentSoundMap ) {
  char soundFile[5000];

  sprintf( soundFile, "%s/gurp1.wav,%s/gurp2.wav,%s/jump1.wav,%s/land1.wav", 
		  model_name, model_name, model_name, model_name );
  addSound( Constants::SOUND_TYPE_ATTACK, soundFile, currentSoundMap );
  
  sprintf( soundFile, "%s/pain25_1.wav,%s/pain25_2.wav,%s/pain50_1.wav,%s/pain50_2.wav,%s/pain75_1.wav,%s/pain75_2.wav,%s/pain100_1.wav,%s/pain100_2.wav", 
		  model_name, model_name, model_name, model_name, model_name, model_name, model_name, model_name );
  addSound( Constants::SOUND_TYPE_HIT, soundFile, currentSoundMap );
}

/**
 * add a coma-delimited list of sound files
 */
void Monster::addSound( int type, char *line, map<int, vector<string>*>* currentSoundMap ) {
  vector<string> *list;
  if(currentSoundMap->find( type ) == currentSoundMap->end()) {
	list = new vector<string>();
	(*currentSoundMap)[ type ] = list;
  } else {
	list = (*currentSoundMap)[ type ];
  }
  char *p = strtok(line, ",");
  while(p) {
	string sound_str = p;
	list->push_back(sound_str);
	p = strtok(NULL, ",");
  }
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

map<int, vector<string>*>* Monster::getSoundMap( char *monsterType ) {
  string type_str = monsterType;
  if(soundMap.find(type_str) != soundMap.end()) {
	return soundMap[ type_str ];
  } else {
	return NULL;
  }
}

const char *Monster::getRandomSound(int type) {
  string type_str = this->getModelName();
  if(soundMap.find(type_str) != soundMap.end()) {
    map<int, vector<string>*> *innerMap = soundMap[type_str];
    vector<string> *sounds = NULL;
    if(innerMap->find(type) != innerMap->end()) {
      sounds = (*innerMap)[type];
    }
    if(!sounds || !(sounds->size())) return NULL;
    string s = (*sounds)[(int)((float)(sounds->size()) * rand()/RAND_MAX)];
    return s.c_str();
  } else {
    return NULL;
  }
}

int Monster::getSkillLevel(const char *skillName) {
  string skillStr = skillName;
  if(skills.find(skillStr) == skills.end()) return 0;
  else return skills[skillStr];
}

const char *Monster::getRandomMonsterType() {
  int n = (int)((float)monsterTypes.size()*rand()/RAND_MAX);
  return monsterTypes[n].c_str();
}

// this weird function is used on loading to avoid memory leaks...
const char *Monster::getMonsterType(char *type) {
  if(!type || !strlen(type)) return NULL;
  for(int i = 0; i < (int)monsterTypes.size(); i++) {
    string ss = monsterTypes[i];
    if(!strcmp((char*)ss.c_str(), type)) return ss.c_str();
  }
  return NULL;
}

const Monster *Monster::getRandomNpc() {
  int n = (int)((float)npcs.size()*rand()/RAND_MAX);
  return npcs[n];
}

