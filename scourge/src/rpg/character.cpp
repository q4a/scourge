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
#include "rpgitem.h"
#include "spell.h"

using namespace std;

map<string, Character*> Character::character_class;
vector<Character*> Character::character_list;
vector<Character*> Character::rootCharacters;

void Character::initCharacters() {
  char errMessage[500];
  char s[200];
  sprintf(s, "%s/world/professions.txt", rootDir);
  FILE *fp = fopen(s, "r");
  if(!fp) {        
    sprintf(errMessage, "Unable to find the file: %s!", s);
    cerr << errMessage << endl;
    exit(1);
  }

  Character *last = NULL;
	int lastGroup = 0;
  char compoundName[255];
  char name[255];
  char parent[255];
  char line[255], dupLine[255];
  int n = fgetc(fp);
  while(n != EOF) {
    if(n == 'P') {
      // skip ':'
      fgetc(fp);
      // read the rest of the line
      n = Constants::readLine(line, fp);

      // find the name and the parent
      strcpy( dupLine, line );
      strcpy( compoundName, strtok( dupLine, "," ) );      
      strcpy( name, strtok( compoundName, ":" ) );
      char *p = strtok( NULL, ":" );
      if( p ) {
        strcpy( parent, p );
      } else {
        strcpy( parent, "" );
      }      
      
      int minLevelReq, hp, mp, levelProgression;
      minLevelReq = hp = mp = levelProgression = 0;
      p = strtok( line, "," );			
			cerr << line << endl;
      if( p ) {
        // ignore the first token
        p = strtok( NULL, "," );				
        if( p ) {
          minLevelReq = atoi( p );
          p = strtok( NULL, "," );					
          if( p ) {
            hp = atoi( p );
            mp = atoi( strtok( NULL, "," ) );
            levelProgression = atoi(strtok(NULL, ","));
          }
        }
      }

      //cerr << "adding character class: " << name << 
//        " parent: " << parent <<
        //" hp: " << hp << " mp: " << mp << " min leel: " << 
        //minLevelReq << " levelProg=" << levelProgression << endl;

      last = new Character( strdup( name ), 
                            ( strlen( parent ) ? strdup( parent ) : NULL ), 
                            hp, mp, 
                            levelProgression, minLevelReq );
      string s = name;
      character_class[s] = last;
      character_list.push_back(last);
    } else if( n == 'D' && last ) {
      fgetc( fp );
      n = Constants::readLine( line, fp );
      if( strlen( last->description ) ) strcat( last->description, " " );
      strcat( last->description, strdup( line ) );
    } else if( n == 'S' ) {
			fgetc( fp );
			n = Constants::readLine( line, fp );
			if( last ) {
				char *p = strtok( line, "," );
				strcpy( name, p );
				last->skills[ Constants::getSkillByName( name ) ] = atoi( strtok( NULL, "," ) );
			} else {
				char *p = strtok( line, "," );
				// skill_name is already set in constants.cpp (it has to be in the same order as the values)
				//Constants::SKILL_NAMES[ skillCount ] = (char*)malloc( sizeof( char ) * strlen( p ) + 1 );
				//strcpy( Constants::SKILL_NAMES[ skillCount ], p );
								
				int skill = Constants::getSkillByName( p );
				if( skill < 0 ) {
					cerr << "*** Error: Can't find skill by name: " << p << endl;
				}

				p = strtok( NULL, "," );
				Constants::SKILL_USE[ skill ] = atoi( p );

				p = strtok( NULL, "," );
				Constants::SKILL_SYMBOL[ skill ] = (char*)malloc( sizeof( char ) * strlen( p ) + 1 );
				strcpy( Constants::SKILL_SYMBOL[ skill ], p );
				
				p = strtok( NULL, "," );
				Constants::SKILL_DESCRIPTION[ skill ] = (char*)malloc( sizeof( char ) * strlen( p ) + 1 );
				strcpy( Constants::SKILL_DESCRIPTION[ skill ], p );

				Constants::skillGroups[ lastGroup ]->push_back( skill );
				Constants::groupSkillMap[ skill ] = lastGroup;
			}
    } else if( n == 'G' ) {
      fgetc(fp);
      n = Constants::readLine(line, fp);
			if( last ) {
				char *p = strtok( line, "," );
				strcpy( name, p );				
				int value = atoi( strtok( NULL, "," ) );
				int group = Constants::getSkillGroupByName( name );
				for( int i = 0; i < Constants::getSkillGroupCount( group ); i++ ) {
					last->skills[ Constants::getSkillGroupSkill( group, i ) ] = value;
				}
			}	else {
				char *p = strtok( line, "," );
				lastGroup = Constants::getSkillGroupByName( p );
				Constants::skillGroups[ lastGroup ] = new vector<int>();

				p = strtok( NULL, "," );
				Constants::SKILL_GROUP_DESCRIPTION[ lastGroup ] = (char*)malloc( sizeof( char ) * strlen( p ) + 1 );
				strcpy( Constants::SKILL_GROUP_DESCRIPTION[ lastGroup ], p );
			}
		} else if( n == 'T' ) {
			fgetc(fp);
      n = Constants::readLine(line, fp);
			char *p = strtok( line, "," );
			string name = strdup( p );
			p = strtok( NULL, "," );
			string value = strdup( p );
			RpgItem::tagsDescriptions[ name ] = value;
		/*
    } else if( n == 'C' && last ) {
		*/
    } else if( n == 'W' && last ) {
			fgetc(fp);
      n = Constants::readLine(line, fp);
			char *p = strtok( line, "," );
			while( p ) {				
				char *lastChar = p + strlen( p ) - 1;
				bool allowed = ( *lastChar == '+' );				
				*lastChar = '\0';
				string s = strdup( p );
				if( !( *p == '*' ) && RpgItem::tagsDescriptions.find( s ) == RpgItem::tagsDescriptions.end() ) {
					cerr << "*** Warning: item tag has no description: " << s << endl;
				}
				if( allowed ) last->allowedWeaponTags.insert( s );
				else last->forbiddenWeaponTags.insert( s );
				p = strtok( NULL, "," );
			}
    } else if( n == 'A' && last ) {
			fgetc(fp);
      n = Constants::readLine(line, fp);
			char *p = strtok( line, "," );
			while( p ) {
				char *lastChar = p + strlen( p ) - 1;
				bool allowed = ( *lastChar == '+' );				
				*lastChar = '\0';
				string s = strdup( p );
				if( !( *p == '*' ) && RpgItem::tagsDescriptions.find( s ) == RpgItem::tagsDescriptions.end() ) {
					cerr << "*** Warning: item tag has no description: " << s << endl;
				}
				if( allowed ) last->allowedArmorTags.insert( s );
				else last->forbiddenArmorTags.insert( s );
				p = strtok( NULL, "," );
			}
    } else {
      n = Constants::readLine( line, fp );
    }
  }
  fclose(fp);

  buildTree();
}

Character::Character( char *name, char *parentName, 
                      int startingHp, int startingMp, 
                      int level_progression, int minLevelReq ) {
  this->name = name;
  this->parentName = parentName;
  this->startingHp = startingHp;
  this->startingMp = startingMp;
  this->level_progression = level_progression;
	this->minLevelReq = minLevelReq;
  this->parent = NULL;
  strcpy( description, "" );
}

Character::~Character(){  
}

#define MIN_STARTING_MP 2
void Character::buildTree() {
  for( int i = 0; i < (int)character_list.size(); i++ ) {
    Character *c = character_list[i];
		c->describeProfession();
    if( c->getParentName() ) {
      c->parent = getCharacterByName( c->getParentName() );
      if( !c->parent ) {
        cerr << "Error: Can't find parent: " << c->getParentName() << 
          " for character " << c->getName() << endl;
        exit( 1 );
      }
			// inherit some stats
			c->startingHp = c->parent->startingHp;
			c->startingMp = c->parent->startingMp;			
			if( c->startingMp <= 0 ) {
				// sanity check: if skills include a magic skill, add min. amount of MP
				for( int i = 0; i < MagicSchool::getMagicSchoolCount(); i++ ) {
					if( c->getSkill( MagicSchool::getMagicSchool( i )->getSkill() ) > -1 ) {
						c->startingMp = MIN_STARTING_MP;
						break;
					}
					if( c->getSkill( MagicSchool::getMagicSchool( i )->getResistSkill() ) > -1 ) {
						c->startingMp = MIN_STARTING_MP;
						break;
					}
				}
			}
			c->level_progression = c->parent->level_progression;
      c->parent->children.push_back( c );
    } else {
      rootCharacters.push_back( c );
    }
  }
}

void Character::describeProfession() {
	char s[1000];
	strcpy( s, "||" );

	char tmp[500];

	// describe the top few skills
	// first loop thru to see if skill-groups are supported
	strcat( s, "Skill bonuses to:|" );
	map<int,int> groupCount;
	for( map<int, int>::iterator i = skills.begin(); i != skills.end(); ++i ) {
		int skill = i->first;
		int value = i->second;
		int group = Constants::getGroupForSkill( skill );
		if( value >= 5 && group != Constants::BASIC_GROUP ) {
			if( groupCount.find( group ) != groupCount.end() ) {
				int n = groupCount[ group ];
				groupCount[group] = ( n + 1 );
			} else {
				groupCount[group] = 1;
			}
		}
	}
	// print group or skill names
	for( map<int, int>::iterator i = skills.begin(); i != skills.end(); ++i ) {
		int skill = i->first;
		int value = i->second;
		int group = Constants::getGroupForSkill( skill );
		if( value >= 5 && group != Constants::BASIC_GROUP ) {
			// HACK: already seen this group
			if( groupCount[group] == -1 ) continue;
			if( groupCount[group] == Constants::getSkillGroupCount( group ) ) {
				groupCount[group] = -1;
				sprintf( tmp, "   %s|", Constants::SKILL_GROUP_DESCRIPTION[ group ] );
			} else {
				sprintf( tmp, "   %s|", Constants::SKILL_NAMES[ skill ] );
			}
			strcat( s, tmp );
		}
	}	
	strcat( tmp, "|" );
	
	
	// describe capabilities
	
	// describe weapons
	describeAcl( s, &allowedWeaponTags, &forbiddenWeaponTags, "weapons" );

	// describe armor
	describeAcl( s, &allowedArmorTags, &forbiddenArmorTags, "armor" );

	strcat( description, s );
}

void Character::describeAcl( char *s, set<string> *allowed, set<string> *forbidden, char *itemType ) {
	char tmp[500];
	string all = "*";
	if( allowed->find( all ) != allowed->end() ) {
		if( forbidden->size() > 0 ) {
			strcat( s, "Can use any " ); strcat( s, itemType ); strcat( s, ", except:|" );
			for( set<string>::iterator i = forbidden->begin(); i != forbidden->end(); ++i ) {
				string tag = *i;
				if( !strcmp( tag.c_str(), "*" ) ) continue;
				RpgItem::describeTag( tmp, "   Not allowed to use ", tag, ".|", itemType );
				strcat( s, tmp );
			}
		} else {
			strcat( s, "Can use any "); strcat( s, itemType ); strcat( s, ".|" );
		}
	} else if( forbidden->find( all ) != forbidden->end() ) {
		if( allowed->size() > 0 ) {
			strcat( s, "Not allowed to use any "); strcat( s, itemType ); strcat( s, ", except:|" );
			for( set<string>::iterator i = allowed->begin(); i != allowed->end(); ++i ) {
				string tag = *i;
				if( !strcmp( tag.c_str(), "*" ) ) continue;
				RpgItem::describeTag( tmp, "   Can use ", tag, ".|", itemType );
				strcat( s, tmp );
			}
		} else {
			strcat( s, "Not allowed to use any "); strcat( s, itemType ); strcat( s, ".|" );
		}
	} else {
		for( set<string>::iterator i = allowed->begin(); i != allowed->end(); ++i ) {
			string tag = *i;
			if( !strcmp( tag.c_str(), "*" ) ) continue;
			RpgItem::describeTag( tmp, "Can use ", tag, ".|", itemType );
			strcat( s, tmp );
		}
		for( set<string>::iterator i = forbidden->begin(); i != forbidden->end(); ++i ) {
			string tag = *i;
			if( !strcmp( tag.c_str(), "*" ) ) continue;
			RpgItem::describeTag( tmp, "Not allowed to use ", tag, ".|", itemType );
			strcat( s, tmp );
		}
	}
}

bool Character::canEquip( RpgItem *item ) {	
	if( item->isWeapon() ) {
		return canEquip( item, &allowedWeaponTags, &forbiddenWeaponTags );
	} else if( item->isArmor() ) {
		return canEquip( item, &allowedArmorTags, &forbiddenArmorTags );
	}
	return true;
}			

bool Character::canEquip( RpgItem *item, set<string> *allowed, set<string> *forbidden ) {
	string all = "*";
	if( allowed->find( all ) != allowed->end() ) {
		for( set<string>::iterator e = forbidden->begin(); e != forbidden->end(); ++e ) {
			string tag = *e;
			if( item->hasTag( tag ) ) return false;
		}
		return true;
	} else if( forbidden->find( all ) != forbidden->end() ) {
		for( set<string>::iterator e = allowed->begin(); e != allowed->end(); ++e ) {
			string tag = *e;
			if( item->hasTag( tag ) ) return true;
		}
		return false;
	} else {
		for( set<string>::iterator e = forbidden->begin(); e != forbidden->end(); ++e ) {
			string tag = *e;
			if( item->hasTag( tag ) ) return false;
		}
		for( set<string>::iterator e = allowed->begin(); e != allowed->end(); ++e ) {
			string tag = *e;
			if( item->hasTag( tag ) ) return true;
		}
		return true;
	}
}
