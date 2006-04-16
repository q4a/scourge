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

      cerr << "adding character class: " << name << 
        " parent: " << parent <<
        " hp: " << hp << " mp: " << mp << " min leel: " << 
        minLevelReq << " levelProg=" << levelProgression << endl;

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
		/*
    } else if( n == 'C' && last ) {
    } else if( n == 'W' && last ) {
    } else if( n == 'A' && last ) {
    */
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
