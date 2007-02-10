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

void Monster::finishCreatureTag( Monster *last_monster ) {
	if( last_monster ) {
		if( last_monster->items.size() ) {
			cerr << "\tinventory=\"";
			for( unsigned int i = 0; i < last_monster->items.size(); i++ ) {
				if( i > 0 ) cerr << ",";
				cerr << last_monster->items[i]->getName();
			}
			cerr << "\"" << endl;
		}
		if( last_monster->spells.size() ) {
			cerr << "\tspells=\"";
			for( unsigned int i = 0; i < last_monster->spells.size(); i++ ) {
				if( i > 0 ) cerr << ",";
				cerr << last_monster->spells[i]->getName();
			}
			cerr << "\"" << endl;
		}
		if( last_monster->skills.size() ) {
			cerr << "\t[skills]" << endl;
			for( map<string,int>::iterator e = last_monster->skills.begin(); e != last_monster->skills.end(); ++e ) {
				string name = e->first;
				int value = e->second;
				cerr << "\t\t" << name << "=" << value << endl;
			}
			cerr << "\t[/skills]" << endl;
		}
		cerr << "[/creature]" << endl;
	}
}

void Monster::initSounds( ConfigLang *config ) {
	vector<ConfigNode*> *v = config->getDocument()->
		getChildrenByName( "sound" );

	for( unsigned int i = 0; i < v->size(); i++ ) {
		ConfigNode *node = (*v)[i];
		
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

//#define CONVERT_CREATURES 1

void Monster::initMonsters() {

	ConfigLang *config = ConfigLang::load( "config/npc.cfg" );
	initCreatures( config );
	delete config;

	config = ConfigLang::load( "config/monster.cfg" );
	initSounds( config );
	initCreatures( config );
	delete config;






	/*
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
			if( !strcmp( skin_name, "*" ) ) strcpy( skin_name, "" );
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
      bool special = false;
      GLuint statemod = 0;
      p = strtok(NULL, ",");
      if(p) {
        npc = ( strstr( p, "npc" ) ? true : false );
        special = ( strstr( p, "special" ) ? true : false );
        p = strtok( NULL, "," );
        if(p) {
          statemod = atoi(p);
        }
      }

#ifdef CONVERT_CREATURES
			finishCreatureTag( last_monster );
			cerr << "[creature]" << endl;
			cerr << "\tname=\"" << name << "\"" << endl;
			if( strlen( portrait ) ) {
				if( npc || special ) {
					cerr << "\tportrait=\"" << portrait << "\"" << endl;
				} else {
					cerr << "\ttype=\"" << portrait << "\"" << endl;
				}
			}
			cerr << "\tlevel=" << level << endl;
			cerr << "\thp=" << hp << endl;
			cerr << "\tmp=" << mp << endl;
			cerr << "\tmodel=\"" << model_name << "\"" << endl;
			cerr << "\tskin=\"" << skin_name << "\"" << endl;
			cerr << "\trareness=" << rareness << endl;
			cerr << "\tspeed=" << speed << endl;
			cerr << "\tarmor=" << armor << endl;
			cerr << "\tscale=" << scale << endl;			
			if( statemod ) cerr << "\tstatemod=" << statemod << endl;
			if( npc ) cerr << "\tnpc=\"true\"" << endl;
			if( special ) cerr << "\tspecial=\"true\"" << endl;
#endif

      // HACK: for monster-s portrait is descriptive type
      // for npc-s it's the portrait path.
      Monster *m;
      if( npc || special ) {
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
//         if( npcStartX > -1 && npcStartY > -1 ) {
//           char tmp[80];
//           sprintf( tmp, "%d,%d", npcStartX, npcStartY );
//           string pos = tmp;
//           npcPos[ pos ] = m;
//         } else {
          npcs.push_back( m );
					m->setStartingStateMod( statemod );
        //}
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
#ifdef CONVERT_CREATURES
	finishCreatureTag( last_monster );
#endif
  fclose(fp);
	*/
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

