/***************************************************************************
                         rpgitem.cpp  -  Item class
                             -------------------
    begin                : Sun Sep 28 2003
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
#include "../common/constants.h"
#include "rpgitem.h"
#include "spell.h"
#include "monster.h"
#include "../session.h"
#include "../shapepalette.h"

using namespace std;

// ###### MS Visual C++ specific ###### 
#if defined(_MSC_VER) && defined(_DEBUG)
# define new DEBUG_NEW
# undef THIS_FILE
  static char THIS_FILE[] = __FILE__;
#endif 

RpgItem *RpgItem::items[1000];

// the smaller this number the more likely a container will show up in a room
#define CONTAINER_CHANCE 10.0f

map<int, map<int, vector<const RpgItem*>*>*> RpgItem::typesMap;
map<string, const RpgItem *> RpgItem::itemsByName;
vector<RpgItem*> RpgItem::containers;
vector<RpgItem*> RpgItem::containersNS;
vector<RpgItem*> RpgItem::special;
int RpgItem::itemCount = 0;
std::vector<ItemType> RpgItem::itemTypes;
int RpgItem::randomTypes[ITEM_TYPE_COUNT];
int RpgItem::randomTypeCount = 0;
std::map<std::string, std::string> RpgItem::tagsDescriptions;
char *RpgItem::DAMAGE_TYPE_NAME[] = {
	N_( "Slashing" ),
	N_( "Piercing" ),
	N_( "Crushing" )
};
char RpgItem::DAMAGE_TYPE_LETTER[] = { 'S', 'P', 'C' };
//char *RpgItem::influenceTypeName[] = { "AP", "CTH", "DAM" };
map<int, vector<string> *> RpgItem::soundMap;

namespace {

class Mop {
public:
	Mop() {}
	~Mop() {
		RpgItem::DestroyStatics();
	}
};

Mop mop;

}



RpgItem::RpgItem( char *name, char *displayName,
                  int rareness, int type, float weight, int price,
                  char *desc, char *shortDesc, int equip, int shape_index,
                  int minDepth, int minLevel,
                  int maxCharges,
                  int iconTileX, int iconTileY ) {
	this->name = name;
	this->displayName = displayName;
	this->rareness = rareness;
	this->type = type;
	this->weight = weight;
	this->price = price;
	this->desc = desc;
	this->shortDesc = shortDesc;
	this->shape_index = shape_index;
	this->equip = equip;
	this->minDepth = minDepth;
	this->minLevel = minLevel;
	this->maxCharges = maxCharges;
	this->iconTileX = iconTileX;
	this->iconTileY = iconTileY;
	this->containerWidth = 0;
	this->containerHeight = 0;
	this->containerTexture[0] = 0;

	// initialize the rest to default values

	// weapon
	damage = damageType = damageSkill = parry = ap = range = twohanded = 0;
	for ( int i = 0; i < Skill::SKILL_COUNT; i++ ) {
		for ( int t = 0; t < INFLUENCE_TYPE_COUNT; t++ ) {
			for ( int r = 0; r < INFLUENCE_LIMIT_COUNT; r++ ) {
				this->weaponInfluence[i][t][r].limit = -1;
				this->weaponInfluence[i][t][r].type = 'L';
				this->weaponInfluence[i][t][r].base = -1;
			}
		}
	}

	// armor
	for ( int i = 0; i < DAMAGE_TYPE_COUNT; i++ ) {
		defense[ i ] = 0;
	}
	defenseSkill = dodgePenalty = 0;

	// potion
	potionPower = potionSkill = potionTime = 0;

	// spells
	spellLevel = 0;
}

RpgItem::~RpgItem() {
}

// was in Item with following confusing comment:
// this should really be in RpgItem but that class can't reference ShapePalette and shapes.
void RpgItem::initItems( ShapePalette *shapePal ) {
	ConfigLang *config = ConfigLang::load( "config/item.cfg" );
	initItemTypes( config );
	initSounds( config );
	initTags( config );
	delete config;

	config = ConfigLang::load( "config/weapon.cfg" );
	initItemEntries( config, shapePal );
	delete config;

	config = ConfigLang::load( "config/armor.cfg" );
	initItemEntries( config, shapePal );
	delete config;

	config = ConfigLang::load( "config/magicitem.cfg" );
	initItemEntries( config, shapePal );
	delete config;

	config = ConfigLang::load( "config/otheritem.cfg" );
	initItemEntries( config, shapePal );
	delete config;
}


void RpgItem::initItemTypes( ConfigLang *config ) {
	vector<ConfigNode*> *v = config->getDocument()->
	                         getChildrenByName( "types" );
	vector<ConfigNode*> *vv = ( *v )[0]->
	                          getChildrenByName( "type" );

	char tmp[20];
	for ( unsigned int i = 0; i < vv->size(); i++ ) {
		ConfigNode *node = ( *vv )[i];

		Session::instance->getGameAdapter()->setUpdate( _( "Loading Items" ), i, vv->size() );

		ItemType itemType;
		strcpy( itemType.name, node->getValueAsString( "name" ) );
		itemType.isWeapon = node->getValueAsBool( "isWeapon" );
		itemType.isArmor = node->getValueAsBool( "isArmor" );
		itemType.isRandom = node->getValueAsBool( "isRandom" );
		itemType.isRanged = node->getValueAsBool( "isRanged" );
		itemType.hasSpell = node->getValueAsBool( "hasSpell" );
		itemType.isEnchantable = node->getValueAsBool( "isEnchantable" );
		strcpy( tmp, node->getValueAsString( "defaultDimension" ) );
		char *p = strtok( tmp, "," );
		if ( p ) {
			itemType.backpackWidth = atoi( p );
			itemType.backpackHeight = atoi( strtok( NULL, "," ) );
		} else {
			itemType.backpackWidth = itemType.backpackHeight = 1;
		}
		RpgItem::itemTypes.push_back( itemType );

		if ( itemType.isRandom ) {
			RpgItem::randomTypes[ RpgItem::randomTypeCount++ ] = RpgItem::itemTypes.size() - 1;
		}
	}
}

void RpgItem::initSounds( ConfigLang *config ) {
	vector<ConfigNode*> *v = config->getDocument()->
	                         getChildrenByName( "sounds" );
	char tmp[1000];
	for ( unsigned int i = 0; i < v->size(); i++ ) {
		ConfigNode *node = ( *v )[i];

		Session::instance->getGameAdapter()->setUpdate( _( "Loading Items" ), i, v->size() );

		strcpy( tmp, node->getValueAsString( "sounds" ) );
		char *p = strtok( tmp, "," );
		int type_index = RpgItem::getTypeByName( p );
		vector<string> *sounds = NULL;
		if ( soundMap.find( type_index ) != soundMap.end() ) {
			sounds = soundMap[type_index];
		} else {
			sounds = new vector<string>;
			soundMap[type_index] = sounds;
		}
		p = strtok( NULL, "," );
		while ( p ) {
			string f = p;
			sounds->push_back( f );
			p = strtok( NULL, "," );
		}
	}
}

void RpgItem::initTags( ConfigLang *config ) {
	vector<ConfigNode*> *v = config->getDocument()->
	                         getChildrenByName( "tags" );
	if ( v ) {
		ConfigNode *node = ( *v )[0];
		map<string, ConfigValue*> *values = node->getValues();
		for ( map<string, ConfigValue*>::iterator e = values->begin(); e != values->end(); ++e ) {
			string name = e->first;
			ConfigValue *value = e->second;
			RpgItem::tagsDescriptions[ name ] = value->getAsString();
		}
	}
}

void RpgItem::initItemEntries( ConfigLang *config, ShapePalette *shapePal ) {
	vector<ConfigNode*> *v = config->getDocument()->
	                         getChildrenByName( "item" );

	char name[255], displayName[255], type[255], shape[255];
	char long_description[500], short_description[120];
	char containerTexture[255];
	char temp[1000];
	for ( unsigned int i = 0; i < v->size(); i++ ) {
		ConfigNode *node = ( *v )[i];

		Session::instance->getGameAdapter()->setUpdate( _( "Loading Items" ), i, v->size() );

		// I:rareness,type,weight,price[,shape_index,[backpack_location[,maxCharges[,min_depth[,min_level]]]]]
		snprintf( name, 255, node->getValueAsString( "name" ) );
		snprintf( displayName, 255, node->getValueAsString( "display_name" ) );
		int rareness = toint( node->getValueAsFloat( "rareness" ) );
		snprintf( type, 255, node->getValueAsString( "type" ) );
		float weight = node->getValueAsFloat( "weight" );
		int price = toint( node->getValueAsFloat( "price" ) );

		snprintf( shape, 255, node->getValueAsString( "shape" ) );
		int backpack_location = toint( node->getValueAsFloat( "inventory_location" ) );
		int minDepth = toint( node->getValueAsFloat( "min_depth" ) );
		int minLevel = toint( node->getValueAsFloat( "min_level" ) );
		int maxCharges = toint( node->getValueAsFloat( "max_charges" ) );

		snprintf( long_description, 500, node->getValueAsString( "description" ) );
		snprintf( short_description, 120, node->getValueAsString( "short_description" ) );

		int containerWidth = toint( node->getValueAsFloat( "container_width" ) );
		int containerHeight = toint( node->getValueAsFloat( "container_height" ) );
		snprintf( containerTexture, 255, node->getValueAsString( "container_texture" ) );

		// I:tileX,tileY ( from data/tiles.bmp, count is 1-based )
		snprintf( temp, 1000, node->getValueAsString( "icon" ) );
		int tileX = atoi( strtok( temp, "," ) );
		int tileY = atoi( strtok( NULL, "," ) );

		// resolve strings
		int type_index = RpgItem::getTypeByName( type );
		//cerr << "item: looking for shape: " << shape << endl;
		int shape_index = shapePal->findShapeIndexByName( shape );
		//cerr << "\tindex=" << shape_index << endl;

		RpgItem *last = new RpgItem( name, displayName, rareness, type_index, weight, price,
		                             long_description, short_description,
		                             backpack_location, shape_index,
		                             minDepth, minLevel, maxCharges, tileX - 1, tileY - 1 );
		last->setContainerWidth( containerWidth );
		last->setContainerHeight( containerHeight );
		last->setContainerTexture( containerTexture );
		//GLShape *s = shapePal->findShapeByName(shape);
		//RpgItem::addItem(last, s->getWidth(), s->getDepth(), s->getHeight() );

		int w, d, h;
		shapePal->getShapeDimensions( shape, &w, &d, &h );
		RpgItem::addItem( last, w, d, h );

		last->setSpellLevel( toint( node->getValueAsFloat( "spell_level" ) ) );

		snprintf( temp, 1000, node->getValueAsString( "tags" ) );
		char *p = strtok( temp, "," );
		while ( p ) {
			string s = p;
			last->addTag( s );
			p = strtok( NULL, "," );
		}

		vector<ConfigNode*> *weapons = node->getChildrenByName( "weapon" );
		if ( weapons != NULL && !weapons->empty() ) {
			ConfigNode *weapon = ( *weapons )[0];

			int baseDamage = toint( weapon->getValueAsFloat( "damage" ) );
			const char *p = weapon->getValueAsString( "damage_type" );
			int damageType = 0;
			if ( strlen( p ) ) damageType = RpgItem::getDamageTypeForLetter( p[0] );
			int skill = Skill::getSkillIndexByName( weapon->getValueAsString( "skill" ) );
			int parry = toint( weapon->getValueAsFloat( "parry" ) );
			int ap = toint( weapon->getValueAsFloat( "ap" ) );
			int range = toint( weapon->getValueAsFloat( "range" ) );
			if ( range < static_cast<int>( MIN_DISTANCE ) ) range = static_cast<int>( MIN_DISTANCE );
			int twoHanded = toint( weapon->getValueAsFloat( "two_handed" ) );
			last->setWeapon( baseDamage, damageType, skill, parry, ap, range, twoHanded );
		}

		vector<ConfigNode*> *armors = node->getChildrenByName( "armor" );
		if ( armors != NULL && !armors->empty() ) {
			ConfigNode *armor = ( *armors )[0];

			// A:defense_vs_slashing,defense_vs_piercing,defense_vs_crushing,skill,dodge_penalty
			int defense[ RpgItem::DAMAGE_TYPE_COUNT ];
			defense[ RpgItem::DAMAGE_TYPE_SLASHING ] =
			  toint( armor->getValueAsFloat( "slash_defense" ) );
			defense[ RpgItem::DAMAGE_TYPE_PIERCING ] =
			  toint( armor->getValueAsFloat( "pierce_defense" ) );
			defense[ RpgItem::DAMAGE_TYPE_CRUSHING ] =
			  toint( armor->getValueAsFloat( "crush_defense" ) );
			int skill = Skill::getSkillIndexByName( armor->getValueAsString( "skill" ) );
			int dodgePenalty = toint( armor->getValueAsFloat( "dodge_penalty" ) );
			last->setArmor( defense, skill, dodgePenalty );
		}

		vector<ConfigNode*> *potions = node->getChildrenByName( "potion" );
		if ( potions != NULL && !potions->empty() ) {
			ConfigNode *potion = ( *potions )[0];

			// power,potionSkill,[potionTimeInMinutes]
			int power = toint( potion->getValueAsFloat( "power" ) );


			char const* potionSkill = potion->getValueAsString( "skill" );
			int skill = -1;
			if ( potionSkill != NULL && strlen( potionSkill ) ) {
				skill = Skill::getSkillIndexByName( potionSkill );
				if ( skill < 0 ) {
					// try special potion 'skills' like HP, AC boosts
					skill = Constants::getPotionSkillByName( potionSkill );
					if ( skill == -1 ) {
						cerr << "*** WARNING: cannot find potion_skill: " << potionSkill << endl;
					}
				}
			}

			int time = toint( potion->getValueAsFloat( "time" ) );
			last->setPotion( power, skill, time );
		}

		vector<ConfigNode*> *adjustments = node->getChildrenByName( "skill_adjustment" );
		for ( unsigned int t = 0; adjustments != NULL && t < adjustments->size(); t++ ) {
			ConfigNode *adjustment = ( *adjustments )[t];

			int skill = Skill::
			            getSkillByName( adjustment-> getValueAsString( "skill" ) )->getIndex();

			last->decodeInfluenceBlock( skill
			                      , adjustment->getChildrenByName( "ap" )
			                      , AP_INFLUENCE );
			last->decodeInfluenceBlock( skill
			                      , adjustment->getChildrenByName( "armor" )
			                      , AP_INFLUENCE );
			last->decodeInfluenceBlock( skill
			                      , adjustment->getChildrenByName( "cth" )
			                      , CTH_INFLUENCE );
			last->decodeInfluenceBlock( skill
			                      , adjustment->getChildrenByName( "damage" )
			                      , DAM_INFLUENCE );
		}
	}
}

void RpgItem::decodeInfluenceBlock( int skill, vector<ConfigNode*> *nodes, int influenceType ) {
	char tmp[300];
	if ( nodes ) {
		ConfigNode *node = ( *nodes )[0];
		for ( int t = 0; t < INFLUENCE_LIMIT_COUNT; t++ ) {
			strcpy( tmp, node->getValueAsString( t == MIN_INFLUENCE ? "min" : "max" ) );
			char *p = strtok( tmp, "," );
			if ( p ) {
				WeaponInfluence wi;
				wi.limit = atoi( p ); // ignore (
				p = strtok( NULL, "," );
				wi.type = ( !strcmp( p, "linear" ) ? 'L' : 'E' );
				p = strtok( NULL, "," );
				wi.base = atof( p );
				/*
				cerr << item->getName() << " skill:" << Skill::skills[skill]->getName() <<
				 " influence: "<<
				 ( influenceType == AP_INFLUENCE ? "ap" : ( influenceType == CTH_INFLUENCE ? "cth" : "dam" ) ) <<
				 " " << ( t == MIN_INFLUENCE ? "min" : "max" ) << " " <<
				 "limie=" << wi.limit << " type=" << wi.type << " base=" << wi.base << endl;
				*/
				setWeaponInfluence( skill, influenceType, t, wi );
			}
		}
	}
}

const string RpgItem::getRandomSound() {
	vector<string> *sounds = NULL;
	if ( soundMap.find( getType() ) != soundMap.end() ) {
		sounds = soundMap[getType()];
	}
	if ( !sounds || sounds->empty() )
		return string( "" );
	string s = ( *sounds )[ Util::dice( sounds->size() ) ];
	return s;
}

void RpgItem::addItem( RpgItem *item, int width, int depth, int height ) {
	// store the item
	/*
	cerr << "adding item: " << item->name <<
	" level=" << item->level <<
	" type=" << item->type << endl;
	*/
	items[itemCount++] = item;

	if ( !item->isSpecial() ) {
		// Add item to type->depth maps
		map<int, vector<const RpgItem*>*> *depthMap = NULL;
		if ( typesMap.find( item->type ) != typesMap.end() ) {
			depthMap = typesMap[( const int )( item->type )];
		} else {
			depthMap = new map<int, vector<const RpgItem*>*>();
			typesMap[item->type] = depthMap;
		}
		//cerr << "\ttypesMap.size()=" << typesMap.size() << " type=" << item->type << " (" << itemTypes[item->type].name << ")" << endl;

		// create the min depth map: Add item to every depth level >= item->getMinDepth()
		for ( int currentDepth = item->getMinDepth(); currentDepth < MAX_MISSION_DEPTH; currentDepth++ ) {
			// create the vector
			vector<const RpgItem*> *list = NULL;
			if ( depthMap->find( currentDepth ) != depthMap->end() ) {
				list = ( *depthMap )[( const int )( currentDepth )];
			} else {
				list = new vector<const RpgItem*>();
				( *depthMap )[ currentDepth ] = list;
			}
			//  cerr << "\tlevelMap.size()=" << levelMap->size() << endl;
			list->push_back( item );
			//  cerr << "\tlist.size()=" << list->size() << endl;
		}
	} else {
		special.push_back( item );
	}

	// remember by name
	string s = item->name;
	itemsByName[s] = item;
	//  cerr << "*** Stored name=>" << item->name << "< item=" << item << endl;

	// HACK: do not include "corpse" as a valid container...
	// It should really have a container exclusion flag.
	if ( item->type == CONTAINER && ( item->name != "Corpse"  || item->name != "Backpack" ) ) {
		if ( width >= depth ) {
			containersNS.push_back( item );
		}
		if ( width <= depth ) {
			containers.push_back( item );
		}
	}
}

int RpgItem::getTypeByName( char *name ) {
	for ( int i = 0; i < static_cast<int>( itemTypes.size() ); i++ ) {
		if ( !strcmp( itemTypes[ i ].name, name ) ) return i;
	}
	cerr << "Can't find type >" << name << "< in " << itemTypes.size() << endl;
	for ( int i = 0; i < static_cast<int>( itemTypes.size() ); i++ ) {
		cerr << "\t" << itemTypes[ i ].name << endl;
	}
	exit( 1 );
}

RpgItem *RpgItem::getRandomItem( int depth ) {
	return getRandomItemFromTypes( depth, randomTypes, randomTypeCount );
}

RpgItem *RpgItem::getRandomItemFromTypes( int depth, int types[], int typeCount ) {
	if ( depth < 0 ) depth = 0;
	if ( depth >= MAX_MISSION_DEPTH ) depth = MAX_MISSION_DEPTH - 1;

	int typeIndex = Util::dice( typeCount );
	map<int, vector<const RpgItem*>*> *depthMap = typesMap[types[typeIndex]];
	if ( depthMap && !depthMap->empty() ) {

		// Select this depth's list of items. Since an item is on every list
		// greater than equal to its min. depth, we don't have to roll for
		// a depth value. (Eg.: a item w. minDepth=2 will be on lists 2-10.
		vector<const RpgItem*> *list = ( *depthMap )[depth];

		if ( list && !list->empty() ) {

			// create a new list where each item occurs item->rareness times
			vector<RpgItem*> rareList;
			for ( int i = 0; i < static_cast<int>( list->size() ); i++ ) {
				RpgItem *item = ( RpgItem* )( *list )[i];
				for ( int t = 0; t < item->getRareness(); t++ ) {
					rareList.push_back( item );
				}
			}

			int n = Util::dice( rareList.size() );
			RpgItem *rpgItem = ( RpgItem* )rareList[n];
			return rpgItem;
		}
	}
	return NULL;
}

RpgItem *RpgItem::getRandomContainer() {
	int n = static_cast<int>( Util::roll( 0.0f, CONTAINER_CHANCE * containers.size() ) );
	if ( n >= static_cast<int>( containers.size() ) )
		return NULL;
	return containers[n];
}

RpgItem *RpgItem::getRandomContainerNS() {
	int n = static_cast<int>( Util::roll( 0.0f, CONTAINER_CHANCE * containersNS.size() ) );
	if ( n >= static_cast<int>( containersNS.size() ) ) return NULL;
	return containersNS[n];
}

RpgItem *RpgItem::getItemByName( char const* name ) {
	string s = name;
	RpgItem *item = ( RpgItem * )itemsByName[s];
	//  cerr << "*** Looking for >" << s << "< found=" << item << endl;
	return item;
}

const char *RpgItem::getTagDescription( string tag ) {
	if ( RpgItem::tagsDescriptions.find( tag ) != RpgItem::tagsDescriptions.end() ) {
		return tagsDescriptions[ tag ].c_str();
	} else {
		return tag.c_str();
	}
}

void RpgItem::setWeaponInfluence( int skill, int type, int limit, WeaponInfluence influence ) {
	weaponInfluence[skill][type][limit].limit = influence.limit;
	weaponInfluence[skill][type][limit].type = influence.type;
	weaponInfluence[skill][type][limit].base = influence.base;
}

WeaponInfluence *RpgItem::getWeaponInfluence( int skill, int type, int limit ) {
	return &( weaponInfluence[skill][type][limit] );
}

int RpgItem::getDamageTypeForLetter( char c ) {
	for ( int i = 0; i < DAMAGE_TYPE_COUNT; i++ ) {
		//if( _( DAMAGE_TYPE_NAME[ i ] )[0] == c ) return i;
		if ( DAMAGE_TYPE_LETTER[ i ] == c ) return i;
	}
	std::cerr << "*** Error can't find damage type for letter: " << c << std::endl;
	return DAMAGE_TYPE_SLASHING;
}

char RpgItem::getDamageTypeLetter( int type ) {
	return _( DAMAGE_TYPE_NAME[ type ] )[0];
}


void RpgItem::DestroyStatics() {
	// typesMap clear.
	typedef std::vector<const RpgItem*> ItemVec;
	typedef std::map<int, ItemVec*> TypeMap;
	typedef std::map<int, TypeMap*> TypeMapMap;
	for ( TypeMapMap::iterator itmm = typesMap.begin(); itmm != typesMap.end(); ++itmm ) {
		for ( TypeMap::iterator itm = itmm->second->begin(); itm != itmm->second->end(); ++itm ) {
			delete itm->second;
		}
		delete itmm->second;
	}
	typesMap.clear();
	// items clear.
	for ( int i = 0; i < itemCount; ++i ) {
		delete items[i];
	}
	// soundMap clear.
	typedef std::vector<std::string> StrVec;
	typedef std::map<int, StrVec *> SoundMap;
	for ( SoundMap::iterator ism = soundMap.begin(); ism != soundMap.end(); ++ism ) {
		delete ism->second;
	}
	soundMap.clear();
}
