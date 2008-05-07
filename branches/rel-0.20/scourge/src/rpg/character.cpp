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
#include "rpg.h"
#include "rpgitem.h"
#include "spell.h"

using namespace std;

map<string, Character*> Character::character_class;
vector<Character*> Character::character_list;
vector<Character*> Character::rootCharacters;

void Character::initCharacters() {
  Character *last = NULL;
  char name[255];
	char displayName[255];
  char parent[255];

	ConfigLang *config = ConfigLang::load( "config/profession.cfg" );
	vector<ConfigNode*> *v = config->getDocument()->
		getChildrenByName( "profession" );

	for( unsigned int i = 0; i < v->size(); i++ ) {
		ConfigNode *node = (*v)[i];

		config->setUpdate( _( "Loading Professions" ), i, v->size() );


		// find the name and the parent
		strcpy( name, node->getValueAsString( "name" ) );
		strcpy( displayName, node->getValueAsString( "display_name" ) );
		strcpy( parent, node->getValueAsString( "parent" ) );
		int minLevelReq = node->getValueAsInt( "min_level" );
		int hp = node->getValueAsInt( "hp" );
		int mp = node->getValueAsInt( "mp" );
		int levelProgression = node->getValueAsInt( "level_progression" );

		last = new Character( strdup( name ),
													strdup( displayName ),
													( strlen( parent ) ? strdup( parent ) : NULL ), 
													hp, mp, 
													levelProgression, minLevelReq );
		string s = name;
		character_class[s] = last;
		character_list.push_back(last);

		strcpy( last->description, node->getValueAsString( "description" ) );
		
		vector<ConfigNode*> *vv = node->getChildrenByName( "skills" );
		if( vv ) {
			ConfigNode *skillNode = (*vv)[0];
			for( map<string,ConfigValue*>::iterator e = skillNode->getValues()->begin();
					 e != skillNode->getValues()->end(); ++e ) {
				string name = e->first;
				ConfigValue *value = e->second;
				last->skills[ Skill::getSkillByName( name.c_str() )->getIndex() ] = static_cast<int>(value->getAsFloat());
			}
		}
		vv = node->getChildrenByName( "groups" );
		if( vv ) {
			ConfigNode *skillNode = (*vv)[0];
			for( map<string,ConfigValue*>::iterator e = skillNode->getValues()->begin();
					 e != skillNode->getValues()->end(); ++e ) {
				string name = e->first;
				ConfigValue *value = e->second;
				SkillGroup *group = SkillGroup::getGroupByName( name.c_str() );
				for( int i = 0; i < group->getSkillCount(); i++ ) {
					last->skills[ group->getSkill( i )->getIndex() ] = static_cast<int>(value->getAsFloat());
				}
			}
		}
		addItemTags( node->getValueAsString( "allowed_weapons" ), &(last->allowedWeaponTags) );
		addItemTags( node->getValueAsString( "forbidden_weapons" ), &(last->forbiddenWeaponTags) );
		addItemTags( node->getValueAsString( "allowed_armor" ), &(last->allowedArmorTags) );
		addItemTags( node->getValueAsString( "forbidden_armor" ), &(last->forbiddenArmorTags) );
	}

	delete config;

  buildTree();
}

void Character::addItemTags( const char *s, set<string> *list ) {
	char line[1000];
	strcpy( line, s );
	char *p = strtok( line, "," );
	while( p ) {				
		//char *lastChar = p + strlen( p ) - 1;
		//*lastChar = '\0';
		string s = strdup( p );
		if( !( *p == '*' ) && RpgItem::tagsDescriptions.find( s ) == RpgItem::tagsDescriptions.end() ) {
			cerr << "*** Warning: item tag has no description: " << s << endl;
		}
		list->insert( s );
		p = strtok( NULL, "," );
	}
}		

Character::Character( char *name, char *displayName, char *parentName, 
                      int startingHp, int startingMp, 
                      int level_progression, int minLevelReq ) {
  this->name = name;
	this->displayName = displayName;
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
	for( int i = 0; i < static_cast<int>(character_list.size()); i++ ) {
		Character *c = character_list[i];
		c->describeProfession();
		if( c->getParentName() ) {
			c->parent = getCharacterByName( c->getParentName() );
			if( !c->parent ) {
				cerr << "Error: Can't find parent: " << c->getParentName() << " for character " << c->getName() << endl;
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

#define ITEM_TYPE_WEAPON 0
#define ITEM_TYPE_ARMOR 1

void Character::describeProfession() {
	char s[1000];
	strcpy( s, "||" );
	enum { TMP_SIZE = 500 };
	char tmp[ TMP_SIZE ];

	// describe the top few skills
	// first loop thru to see if skill-groups are supported
	strcat( s, _( "Skill bonuses to:" ) );
	strcat( s, "|" );
	map<SkillGroup*,int> groupCount;
	for( map<int, int>::iterator i = skills.begin(); i != skills.end(); ++i ) {
		Skill *skill = Skill::skills[ i->first ];
		int value = i->second;
		if( value >= 5 && !skill->getGroup()->isStat() ) {
			if( groupCount.find( skill->getGroup() ) != groupCount.end() ) {
				int n = groupCount[ skill->getGroup() ];
				groupCount[skill->getGroup()] = ( n + 1 );
			} else {
				groupCount[skill->getGroup()] = 1;
			}
		}
	}
	// print group or skill names
	for( map<int, int>::iterator i = skills.begin(); i != skills.end(); ++i ) {
		Skill *skill = Skill::skills[ i->first ];
		int value = i->second;
		if( value >= 5 && !skill->getGroup()->isStat() ) {
			// HACK: already seen this group
			if( groupCount[skill->getGroup()] == -1 ) continue;
			if( groupCount[skill->getGroup()] == skill->getGroup()->getSkillCount() ) {
				groupCount[skill->getGroup()] = -1;
				snprintf( tmp, TMP_SIZE, "   %s|", skill->getGroup()->getDescription() );
			} else {
				snprintf( tmp, TMP_SIZE, "   %s|", skill->getName() );
			}
			strcat( s, tmp );
		}
	}	
	strcat( tmp, "|" );
	
	
	// describe capabilities
	
	// describe weapons
	describeAcl( s, &allowedWeaponTags, &forbiddenWeaponTags, ITEM_TYPE_WEAPON );

	// describe armor
	describeAcl( s, &allowedArmorTags, &forbiddenArmorTags, ITEM_TYPE_ARMOR );

	strcat( description, s );
}

void Character::describeAcl( char *s, set<string> *allowed, set<string> *forbidden, int itemType ) {
	string all = "*";
	if( allowed->find( all ) != allowed->end() ) {
		if( !forbidden->empty() ) {
      if( itemType == ITEM_TYPE_WEAPON ) {
        strcat( s, _( "Can use any weapons, except:" ) );
				strcat( s, "|" );
      } else {
        strcat( s, _( "Can use any armor, except:" ) );
				strcat( s, "|" );
      }
			for( set<string>::iterator i = forbidden->begin(); i != forbidden->end(); ++i ) {
				string tag = *i;
				if( !strcmp( tag.c_str(), "*" ) ) continue;
        strcat( s, "   " );
				strcat( s, RpgItem::getTagDescription( tag ) );
        strcat( s, "|" );
			}
		} else {
      if( itemType == ITEM_TYPE_WEAPON ) {
        strcat( s, _( "Can use any weapons." ) );
				strcat( s, "|" );
      } else {
        strcat( s, _( "Can use any armor." ) );
				strcat( s, "|" );
      }
		}
	} else if( forbidden->find( all ) != forbidden->end() ) {
		if( !allowed->empty() ) {
      if( itemType == ITEM_TYPE_WEAPON ) {
        strcat( s, _( "Not allowed to use any weapons, except:" ) );
				strcat( s, "|" );
      } else {
        strcat( s, _( "Not allowed to use any armor, except:" ) );
				strcat( s, "|" );
      }
			for( set<string>::iterator i = allowed->begin(); i != allowed->end(); ++i ) {
				string tag = *i;
				if( !strcmp( tag.c_str(), "*" ) ) continue;
        strcat( s, "   " );
				strcat( s, RpgItem::getTagDescription( tag ) );
        strcat( s, "|" );
			}
		} else {
      if( itemType == ITEM_TYPE_WEAPON ) {
        strcat( s, _( "Not allowed to use any weapons." ) );
				strcat( s, "|" );
      } else {
        strcat( s, _( "Not allowed to use any armor." ) );
				strcat( s, "|" );
      }
		}
	} else {
		for( set<string>::iterator i = allowed->begin(); i != allowed->end(); ++i ) {
			string tag = *i;
			if( !strcmp( tag.c_str(), "*" ) ) continue;
      if( itemType == ITEM_TYPE_WEAPON ) {
        strcat( s, _( "Can use weapons:" ) );
      } else {
        strcat( s, _( "Can use armor:" ) );
      }
			strcat( s, RpgItem::getTagDescription( tag ) );
      strcat( s, ".|" );
		}
		for( set<string>::iterator i = forbidden->begin(); i != forbidden->end(); ++i ) {
			string tag = *i;
			if( !strcmp( tag.c_str(), "*" ) ) continue;

      if( itemType == ITEM_TYPE_WEAPON ) {
        strcat( s, _( "Not allowed to use weapons:" ) );
      } else {
        strcat( s, _( "Not allowed to use armor:" ) );
      }
			strcat( s, RpgItem::getTagDescription( tag ) );
      strcat( s, ".|" );
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

Character *Character::getRandomCharacter( int level ) {
	Character *c = getRandomCharacter();
	while( c && c->getChildCount() && 
				 c->getChild( 0 )->getMinLevelReq() <= level ) {
		int index = Util::dice( c->getChildCount() );
		c = c->getChild( index );
	}
	return c;
}

