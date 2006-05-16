/***************************************************************************
                          rpglib.cpp  -  description
                             -------------------
    begin                : Sat May 3 2003
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
#include "rpg.h"
#include "rpgitem.h"

using namespace std;

map<string, Skill*> Skill::skillsByName;
vector<Skill*> Skill::skills;

SkillGroup *SkillGroup::stats;
map<string, SkillGroup *> SkillGroup::groupsByName;
vector<SkillGroup*> SkillGroup::groups;

void Rpg::initRpg() {
  char errMessage[500];
  char s[200];
  sprintf(s, "%s/world/rpg.txt", rootDir);
  FILE *fp = fopen(s, "r");
  if(!fp) {        
    sprintf(errMessage, "Unable to find the file: %s!", s);
    cerr << errMessage << endl;
    exit(1);
  }

	Skill *lastSkill = NULL;
	SkillGroup *lastGroup = NULL;
  char line[255];
	char skillName[80], skillSymbol[80], skillDescription[255];
	char groupName[80], groupDescription[255];
  int n = fgetc(fp);
  while(n != EOF) {
		if( n == 'S' ) {
			fgetc( fp );
			n = Constants::readLine( line, fp );				
			strcpy( skillName, strtok( line, "," ) );
			strcpy( skillSymbol, strtok( NULL, "," ) );
			strcpy( skillDescription, strtok( NULL, "," ) );

			lastSkill = 
				new Skill( skillName, 
									 skillDescription, 
									 skillSymbol, 
									 lastGroup );
    } else if( n == 'G' ) {
      fgetc(fp);
      n = Constants::readLine(line, fp);

			strcpy( groupName, strtok( line, "," ) );
			strcpy( groupDescription, strtok( line, "," ) );
			lastGroup = new SkillGroup( groupName, groupDescription );
		} else if( n == 'P' && lastSkill ) {
			fgetc(fp);
			n = Constants::readLine(line, fp);

			lastSkill->setPreReqMultiplier( atoi( strtok( line, "," ) ) );
			char *p = strtok( NULL, "," );
			while( p ) {
				Skill *stat = Skill::getSkillByName( p );
				if( !stat ) {
					cerr << "*** Error: Can't find stat named: " << p << endl;
					exit( 1 );
				}
				lastSkill->addPreReqStat( stat );
				p = strtok( NULL, "," );
			}
		} else if( n == 'T' ) {
			fgetc(fp);
      n = Constants::readLine(line, fp);
			char *p = strtok( line, "," );
			string name = strdup( p );
			p = strtok( NULL, "," );
			string value = strdup( p );
			RpgItem::tagsDescriptions[ name ] = value;
    } else {
      n = Constants::readLine( line, fp );
    }
  }
  fclose(fp);
}

Skill::Skill( char *name, char *description, char *symbol, SkillGroup *group ) {
	strcpy( this->name, name );
	strcpy( this->description, description );
	strcpy( this->symbol, symbol );
	this->group = group;
	
	// store the skill
	this->index = (int)skills.size();
	skills.push_back( this );
	string s = name;
	skillsByName[ s ] = this;
	group->addSkill( this );
}

Skill::~Skill() {
}

SkillGroup::SkillGroup( char *name, char *description ) {
	strcpy( this->name, name );
	strcpy( this->description, description );
	// hack: first group is the stats
	this->isStatSkill = ( groups.size() == 0 );
	if( isStatSkill ) stats = this;
	
	// store the group
	this->index = (int)groups.size();
	groups.push_back( this );
	string s = name;
	groupsByName[ s ] = this;
}

SkillGroup::~SkillGroup() {
}

