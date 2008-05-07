/***************************************************************************
                          specialskill.cpp  -  description
                             -------------------
    begin                : Sat Oct 19 2005
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
#include "specialskill.h"
#include "rpg.h"

/**
  *@author Gabor Torok
  */

using namespace std;

vector<SpecialSkill*> SpecialSkill::skills;
map<string,SpecialSkill*> SpecialSkill::skillsByName;

void SpecialSkill::initSkills() {
  int type, event, iconTileX, iconTileY;
  char name[255], line[255], description[2000], 
    prereq[255], action[255], displayName[255];

	ConfigLang *config = ConfigLang::load( "config/ability.cfg" );
	vector<ConfigNode*> *v = config->getDocument()->
		getChildrenByName( "ability" );

	for( unsigned int i = 0; i < v->size(); i++ ) {
		ConfigNode *node = (*v)[i];

		config->setUpdate(_( "Loading Abilities"), i, v->size() );

		strcpy( name, node->getValueAsString( "name" ) );
    strcpy( displayName, node->getValueAsString( "display_name" ) );
    strcpy( prereq, node->getValueAsString( "prereq_function" ) );
		strcpy( action, node->getValueAsString( "action_function" ) );

		type = SpecialSkill::SKILL_TYPE_AUTOMATIC;
		event = SpecialSkill::SKILL_EVENT_ARMOR;
		char const* p = node->getValueAsString( "type" );
		if( p ) {
			switch(*p) {
			case 'A': type = SpecialSkill::SKILL_TYPE_AUTOMATIC; break;
			case 'M': type = SpecialSkill::SKILL_TYPE_MANUAL; break;
			case 'R': type = SpecialSkill::SKILL_TYPE_RECURRING; break;
			default: cerr << "Unknown special skill type: " << (*p) << endl;
				type = SpecialSkill::SKILL_TYPE_MANUAL;
			}
		}
		
		if( type == SpecialSkill::SKILL_TYPE_AUTOMATIC ) {
			p = node->getValueAsString( "event" );
			if( p ) {
				switch(*p) {
				case 'A': event = SpecialSkill::SKILL_EVENT_ARMOR; break;
				case 'D': event = SpecialSkill::SKILL_EVENT_DAMAGE; break;
				default: cerr << "Unknown special skill event: " << (*p) << endl;
					event = SpecialSkill::SKILL_EVENT_ARMOR;
				}
			}
		}

		iconTileX = iconTileY = 0;
		strcpy( line, node->getValueAsString( "icon" ) );
		p = strtok( line, "," );
		if( p ) {
			iconTileX = atoi( p ) - 1;
			if( iconTileX < 0 ) iconTileX = 0;
			p = strtok( NULL, "," );
			if( p ) iconTileY = atoi( p ) - 1;
			if( iconTileY < 0 ) iconTileY = 0;
		}


		strcpy( description, node->getValueAsString( "description" ) );
      
		//cerr << "Storing special skill: " << name << " (" << prereq << "," << action << ")" << endl;
		SpecialSkill *ss = 
			new SpecialSkill( strdup( name ), 
                        strdup( displayName ),
												strdup( description ), 
												type, 
												event,
												strdup( prereq ), 
												strdup( action ),
												iconTileX,
												iconTileY );
		skills.push_back( ss );
		string nameStr = name;
		skillsByName[ nameStr ] = ss;
	}
	
	delete config;
}

SpecialSkill::SpecialSkill( const char *name,
                            const char *displayName,
                            const char *description, 
                            int type,
                            int event,
                            const char *squirrelFuncPrereq,
                            const char *squirrelFuncAction,
                            int iconTileX,
                            int iconTileY ) {
  this->name = name;
  this->displayName = displayName;
  this->description = description;
  this->type = type;
  this->event = event;
  this->squirrelFuncPrereq = squirrelFuncPrereq;
  this->squirrelFuncAction = squirrelFuncAction;
  this->iconTileX = iconTileX;
  this->iconTileY = iconTileY;
}

SpecialSkill::~SpecialSkill() {
}


