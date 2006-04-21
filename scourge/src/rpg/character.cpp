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
				lastGroup = Constants::getSkillGroupByName( line );
				Constants::skillGroups[ lastGroup ] = new vector<int>();
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
  this->parent = NULL;
  strcpy( description, "" );
}

Character::~Character(){  
}

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
				sprintf( tmp, "   %s|", Constants::SKILL_GROUP_NAMES[ group ] );
			} else {
				sprintf( tmp, "   %s|", Constants::SKILL_NAMES[ skill ] );
			}
			strcat( s, tmp );
		}
	}	
	strcat( tmp, "|" );
	
	
	// describe capabilities
	
	// describe weapons
	string all = "*";
	if( allowedWeaponTags.find( all ) != allowedWeaponTags.end() ) {
		if( forbiddenWeaponTags.size() > 0 ) {
			strcat( s, "Can use any weapon, except:|" );
			for( set<string>::iterator i = forbiddenWeaponTags.begin(); i != forbiddenWeaponTags.end(); ++i ) {
				string tag = *i;
				if( !strcmp( tag.c_str(), "*" ) ) continue;
				RpgItem::describeTag( tmp, "   Not allowed to use ", tag, ".|", "weapons" );
				strcat( s, tmp );
			}
		} else {
			strcat( s, "Can use any weapon.|" );
		}
	} else if( forbiddenWeaponTags.find( all ) != forbiddenWeaponTags.end() ) {
		if( allowedWeaponTags.size() > 0 ) {
			strcat( s, "Not allowed to use any weapon, except:|" );
			for( set<string>::iterator i = allowedWeaponTags.begin(); i != allowedWeaponTags.end(); ++i ) {
				string tag = *i;
				if( !strcmp( tag.c_str(), "*" ) ) continue;
				RpgItem::describeTag( tmp, "   Can use ", tag, ".|", "weapons" );
				strcat( s, tmp );
			}
		} else {
			strcat( s, "Not allowed to use any weapon.|" );
		}
	} else {
		for( set<string>::iterator i = allowedWeaponTags.begin(); i != allowedWeaponTags.end(); ++i ) {
			string tag = *i;
			if( !strcmp( tag.c_str(), "*" ) ) continue;
			RpgItem::describeTag( tmp, "Can use ", tag, ".|", "weapons" );
			strcat( s, tmp );
		}
		for( set<string>::iterator i = forbiddenWeaponTags.begin(); i != forbiddenWeaponTags.end(); ++i ) {
			string tag = *i;
			if( !strcmp( tag.c_str(), "*" ) ) continue;
			RpgItem::describeTag( tmp, "Not allowed to use ", tag, ".|", "weapons" );
			strcat( s, tmp );
		}
	}

	// describe armor
	if( allowedArmorTags.find( all ) != allowedArmorTags.end() ) {
		if( forbiddenArmorTags.size() > 0 ) {
			strcat( s, "Can use any armor, except:|" );
			for( set<string>::iterator i = forbiddenArmorTags.begin(); i != forbiddenArmorTags.end(); ++i ) {
				string tag = *i;
				if( !strcmp( tag.c_str(), "*" ) ) continue;
				RpgItem::describeTag( tmp, "   Not allowed to use ", tag, ".|", "armor" );
				strcat( s, tmp );
			}
		} else {
			strcat( s, "Can use any armor.|" );
		}
	} else if( forbiddenArmorTags.find( all ) != forbiddenArmorTags.end() ) {
		if( allowedArmorTags.size() > 0 ) {
			strcat( s, "Not allowed to use any armor, except:|" );
			for( set<string>::iterator i = allowedArmorTags.begin(); i != allowedArmorTags.end(); ++i ) {
				string tag = *i;
				if( !strcmp( tag.c_str(), "*" ) ) continue;
				RpgItem::describeTag( tmp, "   Can use ", tag, ".|", "armor" );
				strcat( s, tmp );
			}
		} else {
			strcat( s, "Not allowed to use any armor.|" );
		}
	} else {
		for( set<string>::iterator i = allowedArmorTags.begin(); i != allowedArmorTags.end(); ++i ) {
			string tag = *i;
			if( !strcmp( tag.c_str(), "*" ) ) continue;
			RpgItem::describeTag( tmp, "Can use ", tag, ".|", "armor" );
			strcat( s, tmp );
		}
		for( set<string>::iterator i = forbiddenArmorTags.begin(); i != forbiddenArmorTags.end(); ++i ) {
			string tag = *i;
			if( !strcmp( tag.c_str(), "*" ) ) continue;
			RpgItem::describeTag( tmp, "Not allowed to use ", tag, ".|", "armor" );
			strcat( s, tmp );
		}
	}

	strcat( description, s );
}
