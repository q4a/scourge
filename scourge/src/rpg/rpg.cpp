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

vector<char*> Rpg::firstSyl;
vector<char*> Rpg::midSyl;
vector<char*> Rpg::endSyl;

void Rpg::initRpg() { 
	Skill *lastSkill = NULL;
	SkillGroup *lastGroup = NULL;
  char line[255];
	char skillName[80], skillSymbol[80], skillDescription[255];
	char groupName[80], groupDescription[255];
	
	ConfigLang *config = ConfigLang::load( "config/rpg.cfg" );
	vector<ConfigNode*> *v = config->getDocument()->
		getChildrenByName( "group" );

	for( unsigned int i = 0; i < v->size(); i++ ) {
		ConfigNode *node = (*v)[i];

		config->setUpdate( "Loading Skills", i, v->size() );

		strcpy( groupName, node->getValueAsString( "name" ) );
		strcpy( groupDescription, node->getValueAsString( "description" ) );
		
		lastGroup = new SkillGroup( groupName, groupDescription );

		vector<ConfigNode*> *vv = node->getChildrenByName( "skill" );
		for( unsigned int i = 0; vv && i < vv->size(); i++ ) {
			ConfigNode *skillNode = (*vv)[i];

			strcpy( skillName, skillNode->getValueAsString( "name" ) );
			strcpy( skillSymbol, skillNode->getValueAsString( "symbol" ) );
			strcpy( skillDescription, skillNode->getValueAsString( "description" ) );
			lastSkill = 
				new Skill( skillName, 
									 skillDescription, 
									 skillSymbol, 
									 lastGroup );

			lastSkill->setPreReqMultiplier( skillNode->getValueAsInt( "prereq_multiplier" ) );
			strcpy( line, skillNode->getValueAsString( "prereq_skills" ) );
			char *p = strtok( line, "," );
			while( p ) {
				Skill *stat = Skill::getSkillByName( p );
				if( !stat ) {
					cerr << "*** Error: Can't find stat named: " << p << endl;
					exit( 1 );
				}
				lastSkill->addPreReqStat( stat );
				p = strtok( NULL, "," );
			}
		}
	}

	v = config->getDocument()->
		getChildrenByName( "names" );
	if( v ) {
		ConfigNode *node = (*v)[0];
		strcpy( line,node ->getValueAsString( "first" ) );
		char *p = strtok( line, "," );
		while( p != NULL ) {
			firstSyl.push_back( strdup( p ) );
			p = strtok( NULL, "," );
		}
		strcpy( line,node ->getValueAsString( "middle" ) );
		p = strtok( line, "," );
		while( p != NULL ) {
			midSyl.push_back( strdup( p ) );
			p = strtok( NULL, "," );
		}
		strcpy( line,node ->getValueAsString( "last" ) );
		p = strtok( line, "," );
		while( p != NULL ) {
			endSyl.push_back( strdup( p ) );
			p = strtok( NULL, "," );
		}
	}

	delete( config );
}

// Create a random, cheeseball, fantasy name
char *Rpg::createName() {
	char tmp[200];
	strcpy( tmp, firstSyl[ (int)( (float)(firstSyl.size()) * rand() / RAND_MAX ) ] );
	int sylCount = (int)( 3.0f * rand() / RAND_MAX );
	for( int i = 0; i < sylCount; i++ ) {
		strcat( tmp, midSyl[ (int)( (float)(midSyl.size()) * rand() / RAND_MAX ) ] );
	}
	if( 0 == (int)( 2.0f * rand() / RAND_MAX ) ) {
		strcat( tmp, endSyl[ (int)( (float)(endSyl.size()) * rand() / RAND_MAX ) ] );
	}
	return strdup( tmp );
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

