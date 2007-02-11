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
#include "../configlang.h"

using namespace std;

/**
  *@author Gabor Torok
  */

#define UPDATE_MESSAGE "Loading Creatures"

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
	this->statemod = 0;

  // approximate the base attack bonus: magic users get less than warriors
  baseAttackBonus = ( !mp ? 1.0f : 0.75f );

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

void Monster::initSounds( ConfigLang *config ) {
	vector<ConfigNode*> *v = config->getDocument()->
		getChildrenByName( "sound" );

	for( unsigned int i = 0; i < v->size(); i++ ) {
		ConfigNode *node = (*v)[i];

		config->setUpdate( UPDATE_MESSAGE, i, v->size() );
		
		string monsterType_str = node->getValueAsString( "monster" );
		map<int, vector<string>*> *currentSoundMap;
		if( soundMap.find( monsterType_str ) == soundMap.end() ) {
			currentSoundMap = new map<int, vector<string>*>();
			soundMap[monsterType_str] = currentSoundMap;
		} else {
			currentSoundMap = soundMap[monsterType_str];
		}      

		char sounds[1000];
		strcpy( sounds, node->getValueAsString( "attack" ) );
		if( strlen( sounds ) ) {
			addSound( Constants::SOUND_TYPE_ATTACK, sounds, currentSoundMap );
		}
		strcpy( sounds, node->getValueAsString( "hit" ) );
		if( strlen( sounds ) ) {
			addSound( Constants::SOUND_TYPE_HIT, sounds, currentSoundMap );
		}
	}
}

void Monster::initCreatures( ConfigLang *config ) {
	char name[255], model_name[255], skin_name[255];
	char portrait[255], type[255], tmp[3000];

	vector<ConfigNode*> *v = config->getDocument()->
		getChildrenByName( "creature" );

	for( unsigned int i = 0; i < v->size(); i++ ) {
		ConfigNode *node = (*v)[i];

		config->setUpdate( UPDATE_MESSAGE, i, v->size() );

		strcpy( name, node->getValueAsString( "name" ) );
		strcpy( portrait, node->getValueAsString( "portrait" ) );
		strcpy( type, node->getValueAsString( "type" ) );
		strcpy( model_name, node->getValueAsString( "model" ) );
		strcpy( skin_name, node->getValueAsString( "skin" ) );
		if( !strcmp( skin_name, "*" ) ) strcpy( skin_name, "" );
		int level = node->getValueAsInt( "level" );
		int hp = node->getValueAsInt( "hp" );
		int mp = node->getValueAsInt( "mp" );
		int armor = node->getValueAsInt( "armor" );
		int rareness = node->getValueAsInt( "rareness" );
		int speed = node->getValueAsInt( "speed" );
		float scale = node->getValueAsFloat( "scale" );
		bool npc = node->getValueAsBool( "npc" );
		bool special = node->getValueAsBool( "special" );		
		GLuint statemod = node->getValueAsInt( "statemod" );

		Monster *m = 
			new Monster( strdup(name), 
									 ( strlen( type ) ? strdup( type ) : NULL ),
									 level, hp, mp, 
									 strdup(model_name), strdup(skin_name), 
									 rareness, speed, armor, 
									 scale, npc, ( strlen( portrait ) ? strdup( portrait ) : NULL ) );
		if( npc ) {
			npcs.push_back( m );
			m->setStartingStateMod( statemod );
		} else if( special ) {
			// don't add to random monsters list
			// these are placed monsters like Mycotharsius.
		} else {
			vector<Monster*> *list = NULL;
			if(monsters.find(level) == monsters.end()) {
				list = new vector<Monster*>();
				monsters[level] = list;
			} else {
				list = monsters[level];
			}
			list->push_back( m );
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

		// inventory
		strcpy( tmp, node->getValueAsString( "inventory" ) );
		char *p = strtok( tmp, "," );
		while( p ) {
			m->addItem( RpgItem::getItemByName( p ) );
			p = strtok( NULL, "," );
		}

		// spells
		strcpy( tmp, node->getValueAsString( "spells" ) );
		p = strtok( tmp, "," );
		while( p ) {
			m->addSpell( Spell::getSpellByName( p ) );
			p = strtok( NULL, "," );
		}

		// skills
		vector<ConfigNode*> *vv = config->getDocument()->
			getChildrenByName( "skills" );
		if( vv && vv->size() ) {
			ConfigNode *n = (*vv)[0];
			set<string> names;
			n->getKeys( &names );
			for( set<string>::iterator e = names.begin(); e != names.end(); ) {
				string skillStr = *e;
				int value = n->getValueAsInt( name );
				m->skills[ skillStr ] = value;
			}
		}
	}
}

void Monster::initMonsters() {
	ConfigLang *config = ConfigLang::load( "config/npc.cfg" );
	initCreatures( config );
	delete config;

	config = ConfigLang::load( "config/monster.cfg" );
	initSounds( config );
	initCreatures( config );
	delete config;
}

void Monster::addMd2Sounds( char *model_name, map<int, vector<string>*>* currentSoundMap ) {
  char soundFile[5000];

  sprintf( soundFile, "%s/gurp1.wav,%s/gurp2.wav,%s/jump1.wav,%s/land1.wav,%s/fall1.wav,%s/gasp.wav,%s/taunt.wav", 
		  model_name, model_name, model_name, model_name, model_name, model_name, model_name );
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

const char *Monster::getRandomMonsterType( int level ) {
  Monster *m = getRandomMonster( level );
  if( !m ) {
    int n = (int)((float)monsterTypes.size()*rand()/RAND_MAX);
    return monsterTypes[n].c_str();
  } else {
    return m->getType();
  }
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

