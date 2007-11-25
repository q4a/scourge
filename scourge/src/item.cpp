/***************************************************************************
                          item.cpp  -  description
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

#include "item.h"
#include "render/renderlib.h"
#include "rpg/rpglib.h"
#include "session.h"
#include "shapepalette.h"
#include "configlang.h"

using namespace std;

map<int, vector<string> *> Item::soundMap;

Item::Item(Session *session, RpgItem *rpgItem, int level, bool loading) {
  this->session = session;
  this->rpgItem = rpgItem;
  this->level = level;
  this->shapeIndex = this->rpgItem->getShapeIndex();
  this->color = NULL;
  this->shape = session->getShapePalette()->getShape(shapeIndex);
  // for now objects larger than 1 height will block (we can change this later)
  // this is so the player is not blocked by swords and axes on the ground
	this->blocking = ( shape->getHeight() > 1 || 
										 ( rpgItem->getType() == RpgItem::CONTAINER &&
											 strcasecmp( rpgItem->getName(), "corpse" ) ) ||
										 rpgItem->getType() == RpgItem::MISSION );
  this->containedItemCount = 0;
  this->spell = NULL;
  this->containsMagicItem = false;
  this->showCursed = false;
  sprintf( this->itemName, "%s", rpgItem->getDisplayName() );
	inventoryX = inventoryY = 0;

  commonInit( loading );

  currentCharges = rpgItem->getMaxCharges();
  weight = rpgItem->getWeight();
}

Item::~Item(){
	if( textureInMemory != NULL ){
    free( textureInMemory );
    textureInMemory = NULL;
    glDeleteTextures( 1, tex3d );
  }          

	for( int i = 0; i < PARTICLE_COUNT; i++ ) {
		delete iconUnderEffectParticle[i];
		iconUnderEffectParticle[i] = NULL;
		delete iconEffectParticle[i];
		iconEffectParticle[i] = NULL;
	}
}

ItemInfo *Item::save() {
  ItemInfo *info = (ItemInfo*)malloc(sizeof(ItemInfo));
  info->version = PERSIST_VERSION;
  info->level = getLevel();
  strcpy((char*)info->rpgItem_name, getRpgItem()->getName());
  strcpy((char*)info->shape_name, getShape()->getName());
  info->blocking = blocking;
  info->currentCharges = currentCharges;
  info->weight = (Uint32)(weight * 100);
  info->quality = quality;
  info->price = price;
	info->identifiedBits = identifiedBits;

	// spells
  strcpy((char*)info->spell_name, (spell ? spell->getName() : ""));

	// container
  int realCount = 0;
  for(int i = 0; i < containedItemCount; i++) {
    if( containedItems[i] ) info->containedItems[realCount++] = containedItems[i]->save();
  }
  info->containedItemCount = realCount;

	// magic item
  info->bonus = bonus;
  info->damageMultiplier = damageMultiplier;
  info->cursed = cursed;
  info->magicLevel = magicLevel;
  strcpy((char*)info->monster_type, (this->monsterType ? monsterType : ""));
  strcpy((char*)info->magic_school_name, (this->school ? school->getName() : ""));
  info->magicDamage = (school ? saveDice( magicDamage ) : saveEmptyDice());
  for(int i = 0; i < StateMod::STATE_MOD_COUNT; i++) {
    info->stateMod[i] = this->stateMod[i];
  }
  for(int i = 0; i < Skill::SKILL_COUNT; i++) {
    info->skillBonus[i] = this->getSkillBonus(i);
  }

	info->missionId = getMissionId();
	info->missionObjectiveIndex = getMissionObjectiveIndex();
  return info;
}

DiceInfo *Item::saveDice( Dice *dice ) {
  DiceInfo *info = (DiceInfo*)malloc(sizeof(DiceInfo));
  info->version = PERSIST_VERSION;
  info->count = dice->getCount();
  info->sides = dice->getSides();
  info->mod = dice->getMod();
  return info;
}

DiceInfo *Item::saveEmptyDice() {
  DiceInfo *info = (DiceInfo*)malloc(sizeof(DiceInfo));
  info->version = PERSIST_VERSION;
  info->count = 0;
  info->sides = 0;
  info->mod = 0;
  return info;
}

Dice *Item::loadDice( Session *session, DiceInfo *info ) {
  if( !info->count ) return NULL;
  return new Dice( info->count, info->sides, info->mod );
}


Item *Item::load(Session *session, ItemInfo *info) {
  if( !strlen( (char*)info->rpgItem_name )) return NULL;
  Spell *spell = NULL;
  if( strlen((char*)info->spell_name) ) spell = Spell::getSpellByName( (char*)info->spell_name );
  RpgItem *rpgItem = RpgItem::getItemByName( (char*)info->rpgItem_name );
  if( !rpgItem ) {
    cerr << "Error: can't find rpgItem with name:" << (char*)info->rpgItem_name << endl;
    return NULL;
  }
  Item *item = session->newItem( rpgItem, 
                                 info->level, 
                                 spell,
                                 true);
  item->blocking = (info->blocking == 1);
  item->currentCharges = info->currentCharges;
  item->weight = (float)(info->weight) / 100.0f;
  item->quality = info->quality;
  item->price = info->price;
	item->identifiedBits = info->identifiedBits;
  
	// container
	item->containedItemCount = info->containedItemCount;
  int realCount = 0;
  for(int i = 0; i < (int)info->containedItemCount; i++) {
     Item *containedItem = Item::load( session, info->containedItems[i] );
     if( containedItem ) {
			 item->containedItems[ realCount++ ] = containedItem;
			 if( containedItem->isMagicItem() ) item->containsMagicItem = true;
		 }
  }
  item->containedItemCount = realCount;
    
	// magic item
  item->bonus = info->bonus;
  item->damageMultiplier = info->damageMultiplier;
  item->cursed = (info->cursed == 1);
  item->magicLevel = info->magicLevel;
  // get a reference to the real string... (yuck)
  item->monsterType = (char*)Monster::getMonsterType( (char*)info->monster_type );
  // turn off "vs. any creature"
  if( !item->getMonsterType() ) item->damageMultiplier = 1;
  item->school = MagicSchool::getMagicSchoolByName( (char*)info->magic_school_name );
  item->magicDamage = Item::loadDice( session, info->magicDamage );
  for(int i = 0; i < StateMod::STATE_MOD_COUNT; i++) {
    item->stateMod[i] = info->stateMod[i];
  }
  for(int i = 0; i < Skill::SKILL_COUNT; i++) {
    if(info->skillBonus[i]) item->skillBonus[i] = info->skillBonus[i];
  }

	item->setMissionObjectInfo( (int)info->missionId, (int)info->missionObjectiveIndex );

	// re-describe the item. describeMagic is called from commonInit at
	// which point magicLevel can be 0, so it's important to re-describe
	// the item. (since later magicLevel can be -1)
	item->describeMagic(item->itemName, item->rpgItem->getDisplayName());

  return item;
}

bool Item::addContainedItem(Item *item, bool force) { 
  if(containedItemCount < MAX_CONTAINED_ITEMS && 
     (force || !item->isBlocking() || getShape()->fitsInside(item->getShape()))) {
    containedItems[containedItemCount++] = item; 
    if( item->isMagicItem() ) containsMagicItem = true;
    return true;
  } else {
    cerr << "Warning: unable to add to container. Container=" << getRpgItem()->getName() << " item=" << item->getRpgItem()->getName() << endl;
    return false;
  }
} 

Item *Item::removeContainedItem(int index) {
  Item *item = NULL;
  if(index >= 0 && index < containedItemCount) {
    item = containedItems[index];
    containedItemCount--;
    for(int i = index; i < containedItemCount; i++) {
      containedItems[i] = containedItems[i + 1];
    }
    containsMagicItem = false;
    for( int i = 0; i < containedItemCount; i++ ) {
      if( containedItems[i]->isMagicItem() ) containsMagicItem = true;
    }
  }
  return item;
}

Item *Item::getContainedItem(int index) {
  return((index >= 0 && index < containedItemCount) ? containedItems[index] : NULL);
}

bool Item::isContainedItem(Item *item) {
  for(int i = 0; i < containedItemCount; i++) {
    if(containedItems[i] == item || 
       (containedItems[i]->getRpgItem()->getType() == RpgItem::CONTAINER &&
        containedItems[i]->isContainedItem(item))) return true;
  }
  return false;
}

void Item::getDetailedDescription(char *s, bool precise){
  int type;
  RpgItem * rpgItem;

  rpgItem  = getRpgItem();
  type = rpgItem->getType();
  /*
  if(type == RpgItem::DRINK || type == RpgItem::POTION || type == RpgItem::FOOD){
    sprintf(s, "(L:%d,Q:%d,W:%2.2f, N:%d/%d) %s%s", 
            getLevel(), 
            getQuality(), 
            getWeight(),
            getCurrentCharges(),
            getMaxCharges(),
            (precise ? itemName : rpgItem->getShortDesc()),
            (session->getCurrentMission() && 
             session->getCurrentMission()->isMissionItem( this ) ? 
             " *Mission*" : ""));
  } else if(type == RpgItem::SCROLL) {
    sprintf(s, "(L:%d) %s%s", getLevel(), itemName,
            (session->getCurrentMission() && 
             session->getCurrentMission()->isMissionItem( this ) ? 
             " *Mission*" : ""));
  } else {
    sprintf(s, "(L:%d,A:%d,S:%d,Q:%d,W:%2.2f) %s%s", 
            getLevel(), 
            getAction(), 
            getSpeed(), 
            getQuality(), 
            getWeight(),
            (precise ? itemName : rpgItem->getShortDesc()),
            (session->getCurrentMission() && 
             session->getCurrentMission()->isMissionItem( this ) ? 
             " *Mission*" : ""));
  }
  */
	
  if(type == RpgItem::DRINK || type == RpgItem::POTION || type == RpgItem::FOOD){
    sprintf(s, "(L:%d) %s%s%s", 
            getLevel(), 
            ( isCursed() && getShowCursed() ? _( "*Cursed* " ) : "" ),
            (precise ? itemName : rpgItem->getShortDesc()),
            ( missionId > 0 ? _( " *Mission*" ) : ""));
  } else if(type == RpgItem::SCROLL) {
    sprintf(s, "(L:%d) %s%s%s", 
            getLevel(), 
            ( isCursed() && getShowCursed() ? _( "*Cursed* " ) : "" ),
            itemName,
            ( missionId > 0 ? _( " *Mission*" ) : ""));
  } else {
    sprintf(s, "(L:%d) %s%s%s", 
            getLevel(), 
            ( isCursed() && getShowCursed() ? _( "*Cursed* " ) : "" ),
            (precise ? itemName : rpgItem->getShortDesc()),
            ( missionId > 0 ? _( " *Mission*" ) : ""));
  }
}

char *trim( char *s ) {
	for( char *p = s; p; p++ ) {
		if( !( *p == ' ' || *p == '\t' || *p == '\r' || *p == '\n' ) ) {
			return p;
		}
	}
	return s;
}

void Item::initItemTypes( ConfigLang *config ) {
	vector<ConfigNode*> *v = config->getDocument()->
		getChildrenByName( "types" );
	vector<ConfigNode*> *vv = (*v)[0]->
		getChildrenByName( "type" );

	char tmp[20];
	for( unsigned int i = 0; i < vv->size(); i++ ) {
		ConfigNode *node = (*vv)[i];

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
		if( p ) {
			itemType.inventoryWidth = atoi( p );
			itemType.inventoryHeight = atoi( strtok( NULL, "," ) );
		} else {
			itemType.inventoryWidth = itemType.inventoryHeight = 1;
		}
		RpgItem::itemTypes.push_back( itemType );
      
		if( itemType.isRandom ) {
			RpgItem::randomTypes[ RpgItem::randomTypeCount++ ] = RpgItem::itemTypes.size() - 1;
		}
	}
}

void Item::initItemEntries( ConfigLang *config, ShapePalette *shapePal ) {
	vector<ConfigNode*> *v = config->getDocument()->
		getChildrenByName( "item" );

	char name[255], displayName[255], type[255], shape[255];
  char long_description[500], short_description[120];
  char temp[1000];
	for( unsigned int i = 0; i < v->size(); i++ ) {
		ConfigNode *node = (*v)[i];

		Session::instance->getGameAdapter()->setUpdate( _( "Loading Items" ), i, v->size() );
		
		// I:rareness,type,weight,price[,shape_index,[inventory_location[,maxCharges[,min_depth[,min_level]]]]]
		strcpy( name, node->getValueAsString( "name" ) );
		strcpy( displayName, node->getValueAsString( "display_name" ) );
		int rareness = toint( node->getValueAsFloat( "rareness" ) );
		strcpy( type, node->getValueAsString( "type" ) );
		float weight = node->getValueAsFloat( "weight" );
		int price = toint( node->getValueAsFloat( "price" ) );

		strcpy( shape, node->getValueAsString( "shape" ) );
    int inventory_location = toint( node->getValueAsFloat( "inventory_location" ) );
    int minDepth = toint( node->getValueAsFloat( "min_depth" ) );
		int minLevel = toint( node->getValueAsFloat( "min_level" ) );
		int maxCharges = toint( node->getValueAsFloat( "max_charges" ) );
			      
		strcpy( long_description, node->getValueAsString( "description" ) );
    strcpy( short_description, node->getValueAsString( "short_description" ) );
      
		// I:tileX,tileY ( from data/tiles.bmp, count is 1-based )
		strcpy( temp, node->getValueAsString( "icon" ) );
		int tileX = atoi( strtok( temp, "," ) );
		int tileY = atoi( strtok( NULL, "," ) );

		// resolve strings
		int type_index = RpgItem::getTypeByName( type );    
		//cerr << "item: looking for shape: " << shape << endl;
		int shape_index = shapePal->findShapeIndexByName( shape );
		//cerr << "\tindex=" << shape_index << endl;

		RpgItem *last = new RpgItem( strdup( name ), strdup( displayName ), rareness, type_index, weight, price,
												strdup( long_description ), strdup( short_description ),
												inventory_location, shape_index,
												minDepth, minLevel, maxCharges, tileX - 1, tileY - 1 );
		GLShape *s = shapePal->findShapeByName(shape);
		RpgItem::addItem(last, s->getWidth(), s->getDepth(), s->getHeight() );

		last->setSpellLevel( toint( node->getValueAsFloat( "spell_level" ) ) );

		strcpy( temp, node->getValueAsString( "tags" ) );
		char *p = strtok( temp, "," );
		while( p ) {
			string s = strdup( p );
			last->addTag( s );
			p = strtok( NULL, "," );
		}

		vector<ConfigNode*> *weapons = node->getChildrenByName( "weapon" );
		if( weapons != NULL && weapons->size() > 0 ) {
			ConfigNode *weapon = (*weapons)[0];

			int baseDamage = toint( weapon->getValueAsFloat( "damage" ) );
			const char *p = weapon->getValueAsString( "damage_type" );
			int damageType = 0;
			if( strlen( p ) ) damageType = RpgItem::getDamageTypeForLetter( p[0] );
			int skill = Skill::getSkillIndexByName( (char*)weapon->getValueAsString( "skill" ) );
			int parry = toint( weapon->getValueAsFloat( "parry" ) );
			int ap = toint( weapon->getValueAsFloat( "ap" ) );
			int range = toint( weapon->getValueAsFloat( "range" ) );
			if( range < (int)MIN_DISTANCE ) range = (int)MIN_DISTANCE;
			int twoHanded = toint( weapon->getValueAsFloat( "two_handed" ) );
			last->setWeapon( baseDamage, damageType, skill, parry, ap, range, twoHanded );
		}

		vector<ConfigNode*> *armors = node->getChildrenByName( "armor" );
		if( armors != NULL && armors->size() > 0 ) {
			ConfigNode *armor = (*armors)[0];

			// A:defense_vs_slashing,defense_vs_piercing,defense_vs_crushing,skill,dodge_penalty
			int defense[ RpgItem::DAMAGE_TYPE_COUNT ];
			defense[ RpgItem::DAMAGE_TYPE_SLASHING ] = 
				toint( armor->getValueAsFloat( "slash_defense" ) );
			defense[ RpgItem::DAMAGE_TYPE_PIERCING ] = 
				toint( armor->getValueAsFloat( "pierce_defense" ) );
			defense[ RpgItem::DAMAGE_TYPE_CRUSHING ] = 
				toint( armor->getValueAsFloat( "crush_defense" ) );
			int skill = Skill::getSkillIndexByName( (char*)armor->getValueAsString( "skill" ) );
			int dodgePenalty = toint( armor->getValueAsFloat( "dodge_penalty" ) );
			last->setArmor( defense, skill, dodgePenalty );
		}

		vector<ConfigNode*> *potions = node->getChildrenByName( "potion" );
		if( potions != NULL && potions->size() > 0 ) {
			ConfigNode *potion = (*potions)[0];

			// power,potionSkill,[potionTimeInMinutes]
			int power = toint( potion->getValueAsFloat( "power" ) );
			
			
			char *potionSkill = (char*)potion->getValueAsString( "skill" );
			int skill = -1;
      if( potionSkill != NULL && strlen( potionSkill ) ) {
        skill = Skill::getSkillIndexByName( potionSkill );
        if( skill < 0 ) {
          // try special potion 'skills' like HP, AC boosts
          skill = Constants::getPotionSkillByName( potionSkill );
          if( skill == -1 ) {
            cerr << "*** WARNING: cannot find potion_skill: " << potionSkill << endl;
          }
        }
      }

			int time = toint( potion->getValueAsFloat( "time" ) );
			last->setPotion( power, skill, time );
		}

		vector<ConfigNode*> *adjustments = node->getChildrenByName( "skill_adjustment" );
		for( unsigned int t = 0; adjustments != NULL && t < adjustments->size(); t++ ) {
			ConfigNode *adjustment = (*adjustments)[t];

			int skill = Skill::
				getSkillByName( (char*)adjustment->
												getValueAsString( "skill" ) )->getIndex();

			decodeInfluenceBlock( last, 
														skill, 
														adjustment->getChildrenByName( "ap" ), 
														AP_INFLUENCE );
			decodeInfluenceBlock( last, 
														skill, 
														adjustment->getChildrenByName( "armor" ), 
														AP_INFLUENCE );
			decodeInfluenceBlock( last, 
														skill, 
														adjustment->getChildrenByName( "cth" ), 
														CTH_INFLUENCE );
			decodeInfluenceBlock( last, 
														skill, 
														adjustment->getChildrenByName( "damage" ), 
														DAM_INFLUENCE );
		}
	}
}

void Item::decodeInfluenceBlock( RpgItem *item, 
																 int skill, 
																 vector<ConfigNode*> *nodes, 
																 int influenceType ) {
	char tmp[300];
	if( nodes ) {
		ConfigNode *node = (*nodes)[0];
		for( int t = 0; t < INFLUENCE_LIMIT_COUNT; t++ ) {
			strcpy( tmp, node->getValueAsString( t == MIN_INFLUENCE ? "min" : "max" ) );
			char *p = strtok( tmp, "," );
			if( p ) {
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
				item->setWeaponInfluence( skill, influenceType, t, wi );
			}
		}
	}
}

void Item::initSounds( ConfigLang *config ) {
	vector<ConfigNode*> *v = config->getDocument()->
		getChildrenByName( "sounds" );
	char tmp[1000];
	for( unsigned int i = 0; i < v->size(); i++ ) {
		ConfigNode *node = (*v)[i];

		Session::instance->getGameAdapter()->setUpdate( _( "Loading Items" ), i, v->size() );

		strcpy( tmp, node->getValueAsString( "sounds" ) );
		char *p = strtok(tmp, ",");
		int type_index = RpgItem::getTypeByName(p);
		vector<string> *sounds = NULL;
		if(Item::soundMap.find(type_index) != Item::soundMap.end()) {
			sounds = Item::soundMap[type_index];
		} else {
			sounds = new vector<string>;
			Item::soundMap[type_index] = sounds;
		}
		p = strtok(NULL, ",");
		while(p) {
			string f = p;
			sounds->push_back(f);
			p = strtok(NULL, ",");
		}
	}
}

void Item::initTags( ConfigLang *config ) {
	vector<ConfigNode*> *v = config->getDocument()->
		getChildrenByName( "tags" );
	if( v ) {
		ConfigNode *node = (*v)[0];
		map<string, ConfigValue*> *values = node->getValues();
		for( map<string, ConfigValue*>::iterator e = values->begin(); e != values->end(); ++e ) {
			string name = e->first;
			ConfigValue *value = e->second;
			RpgItem::tagsDescriptions[ name ] = value->getAsString();
		}
	}
}

// this should really be in RpgItem but that class can't reference ShapePalette and shapes.
void Item::initItems( ShapePalette *shapePal ) {
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

const string Item::getRandomSound() {
  vector<string> *sounds = NULL;
  if(Item::soundMap.find(this->getRpgItem()->getType()) != Item::soundMap.end()) {
    sounds = Item::soundMap[this->getRpgItem()->getType()];
  }
  if(!sounds || !(sounds->size())) 
		return string("");
  string s = (*sounds)[(int)((float)(sounds->size()) * rand()/RAND_MAX)];
  return s;
}

// return true if the item is used up
bool Item::decrementCharges(){
  float f1;
  int oldCharges;

  oldCharges = getCurrentCharges();            
  if(oldCharges <= 1){
    // The object is totally consummed
    return true;    
  }
  setCurrentCharges(oldCharges - 1);

  // Compute initial weight to be able to compute new weight
  // (without increasing error each time)

  f1 = getWeight();
  f1 *= (float) ( getRpgItem()->getMaxCharges());
  f1 /= (float) oldCharges;
  f1 *= (((float)oldCharges - 1.0f) / (float)(getRpgItem()->getMaxCharges()));            
  setWeight(f1);
  return false;      
}




void Item::commonInit( bool loading ) {
	tex3d[0] = 0;
	textureInMemory = NULL;
	iconEffectTimer = 0;
	iconUnderEffectTimer = 0;
	for( int i = 0; i < PARTICLE_COUNT; i++ ) {
		iconUnderEffectParticle[i] = new ParticleStruct();
		iconUnderEffectParticle[i]->life = -1;
		iconEffectParticle[i] = new ParticleStruct();
		iconEffectParticle[i]->life = -1;
	}
	identifiedBits = 0;
  missionId = missionObjectiveIndex = 0;

  // --------------
  // regular attribs

  weight = rpgItem->getWeight();
  quality = (int)( 50.0f * rand() / RAND_MAX ) + 50; // starts out mostly healthy

  int basePrice = ( this->spell ? this->spell->getExp() : rpgItem->getPrice() );
  price = basePrice + 
    (int)Util::getRandomSum( (float)(basePrice / 2), level );

  // assign a spell to the item
  // the deeper you go, the more likely that items contain spells
  if( rpgItem->hasSpell() &&
			0 == (int)( (float)( MAX_MISSION_DEPTH - ( session->getCurrentMission() ? session->getCurrentMission()->getDepth() : 0 ) ) * rand() / RAND_MAX ) ) {
    this->spell = MagicSchool::getRandomSpell( 1 );
    price += (int)Util::getRandomSum( (float)(basePrice / 2), this->spell->getLevel() );
  }

  // --------------
  // magic attribs

  // init to no-magic
  magicLevel = -1;
  bonus = 0;
  damageMultiplier = 1;
  monsterType = NULL;
  cursed = false;
  school = NULL;
  magicDamage = NULL;
  stateModSet = false;
  for(int i = 0; i < StateMod::STATE_MOD_COUNT; i++) stateMod[i] = 0;

  if( rpgItem->isEnchantable() && !loading ) {
    // roll for magic
    int n = (int)( ( 200.0f - ( level * 1.5f ) ) * rand()/RAND_MAX );
    if( n < 5 ) enchant( Constants::DIVINE_MAGIC_ITEM );
    else if( n < 15 ) enchant( Constants::CHAMPION_MAGIC_ITEM );
    else if( n < 30 ) enchant( Constants::GREATER_MAGIC_ITEM );
    else if( n < 50 ) enchant( Constants::LESSER_MAGIC_ITEM );
  }

  // describe spell-holding items also
  if( magicLevel < 0 && RpgItem::itemTypes[ rpgItem->getType() ].hasSpell ) {
    describeMagic( itemName, rpgItem->getDisplayName() );
  }
}

void Item::enchant( int newMagicLevel ) {
  if( magicLevel != -1 ) return;

  magicLevel = newMagicLevel;

  // item level caps the magic level:
  // 0-9: lesser
  // 10-19: greater
  // 20-29: champion
  // 39+ : divine
  // This is so low level items won't be too powerful.
  int maxMagicLevel = level / 10;
  if( magicLevel > maxMagicLevel ) magicLevel = maxMagicLevel;

  cursed = ( !((int)( 20.0f * rand() / RAND_MAX )) );

  // adjust the price
  price *= ( magicLevel + 2 );

  int n;
  Spell *spell;
  switch(magicLevel) {
  case Constants::LESSER_MAGIC_ITEM:
    bonus = (int)(2.0f * rand()/RAND_MAX) + 1;
    if(rpgItem->isWeapon()) {
      damageMultiplier = (int)(2.0f * rand()/RAND_MAX) + 2;
      monsterType = (char*)Monster::getRandomMonsterType( level );
    }
    n = (int)(3.0f * rand()/RAND_MAX) + 2;
    for(int i = 0; i < n; i++) {
      int skill = SkillGroup::stats->getRandomSkill()->getIndex();
      if(skillBonus.find(skill) == skillBonus.end()) {
        skillBonus[skill] = (int)(2.0f * rand()/RAND_MAX) + 1;
      }
    }    
    break;
  case Constants::GREATER_MAGIC_ITEM:
    bonus = (int)(3.0f * rand()/RAND_MAX) + 1;
    if(rpgItem->isWeapon()) {
      damageMultiplier = (int)(3.0f * rand()/RAND_MAX) + 2;
      monsterType = (char*)Monster::getRandomMonsterType( level );
    }
    spell = MagicSchool::getRandomSpell(1);
    if(spell) {
      school = spell->getSchool();
      magicDamage = new Dice(1, (int)(3.0f * rand()/RAND_MAX) + 1, (int)(3.0f * rand()/RAND_MAX));
    }
    n = (int)(3.0f * rand()/RAND_MAX) + 2;
    for(int i = 0; i < n; i++) {
      int skill = SkillGroup::stats->getRandomSkill()->getIndex();
      if(skillBonus.find(skill) == skillBonus.end()) {
        skillBonus[skill] = (int)(3.0f * rand()/RAND_MAX) + 1;
      }
    }
    break;
  case Constants::CHAMPION_MAGIC_ITEM:
    bonus = (int)(4.0f * rand()/RAND_MAX) + 1;
    if(rpgItem->isWeapon()) {
      damageMultiplier = (int)(3.0f * rand()/RAND_MAX) + 2;
      monsterType = (char*)Monster::getRandomMonsterType( level );
    }
    spell = MagicSchool::getRandomSpell(1);
    if(spell) {
      school = spell->getSchool();
      magicDamage = new Dice(1, (int)(3.0f * rand()/RAND_MAX) + 2, (int)(3.0f * rand()/RAND_MAX));
    }
    n = (int)(3.0f * rand()/RAND_MAX) + 1;
    if(n > 0) stateModSet = true;
    for(int i = 0; i < n; i++) {
      stateMod[ StateMod::getRandomGood()->getIndex() ] = 1;
    }
    n = (int)(3.0f * rand()/RAND_MAX) + 1;
    for(int i = 0; i < n; i++) {
      int skill = SkillGroup::stats->getRandomSkill()->getIndex();
      if(skillBonus.find(skill) == skillBonus.end()) {
        skillBonus[skill] = (int)(4.0f * rand()/RAND_MAX) + 1;
      }
    }
    break;
  case Constants::DIVINE_MAGIC_ITEM:
    bonus = (int)(5.0f * rand()/RAND_MAX) + 1;
    if(rpgItem->isWeapon()) {
      damageMultiplier = (int)(4.0f * rand()/RAND_MAX) + 2;
      monsterType = (char*)Monster::getRandomMonsterType( level );
    }
    spell = MagicSchool::getRandomSpell(1);
    if(spell) {
      school = spell->getSchool();
      magicDamage = new Dice(1, (int)(3.0f * rand()/RAND_MAX) + 3, (int)(3.0f * rand()/RAND_MAX));
    }
    n = (int)(3.0f * rand()/RAND_MAX) + 1;
    if(n > 0) stateModSet = true;
    for(int i = 0; i < n; i++) {
      stateMod[ StateMod::getRandomGood()->getIndex() ] = 1;
    }
    n = (int)(3.0f * rand()/RAND_MAX) + 1;
    if(n > 0) stateModSet = true;
    for(int i = 0; i < n; i++) {
      stateMod[ StateMod::getRandomBad()->getIndex() ] = 2;
    }
    n = (int)(3.0f * rand()/RAND_MAX) + 2;
    for(int i = 0; i < n; i++) {
      int skill = SkillGroup::stats->getRandomSkill()->getIndex();
      if(skillBonus.find(skill) == skillBonus.end()) {
        skillBonus[skill] = (int)(5.0f * rand()/RAND_MAX) + 1;
      }
    }
    break;
  default:
    cerr << "*** Error: unknown magic level: " << magicLevel << endl;
  }

  // turn off "vs. any creature"
  if( !monsterType ) damageMultiplier = 1;

  describeMagic(itemName, rpgItem->getDisplayName());
}

// max about 30 points (must be deterministic)
int Item::getMagicResistance() { 
  return( 3 * ( ( getLevel() / 10 ) + getMagicLevel() ) ); 
}

#define DEBUG_ITEM_ID 0

void Item::describeMagic(char *s, char *itemName) {

	// not for scrolls :-(
	if( rpgItem->getType() == RpgItem::SCROLL ) return;

  // e.g.: Lesser broadsword + 3 of nature magic
  char tmp[80];


	if( magicLevel > -1 ) {
		if( DEBUG_ITEM_ID || isIdentified() ) {
			char format[1000];
			/* This represents the format of an identified item. Please do not translate the strings,
			only rearrange them to work with your language. 
			
			An example of the final string created by the code after parsing this message would be: 
			"Ethereal Greater Protective Slaying Longsword (+4) of the Dragon"
			*/
			strcpy( format, _( "$spellsymbol $magiclevel $protective $slaying $itemname $bonus $symbol" ) );
			char *p = strtok( format, " " );
			strcpy( s, "" );
			while( p ) {
	
				if( !strcmp( p, "$spellsymbol" ) ) {
					if( RpgItem::itemTypes[ rpgItem->getType() ].hasSpell && spell ) {
						if( strlen( s ) ) strcat( s, " " );
						strcat( s, spell->getSymbol() );
					}
				} else if( !strcmp( p, "$magiclevel" ) ) {
					if( magicLevel > -1 ) {
						if( strlen( s ) ) strcat( s, " " );
						strcat( s, _( Constants::MAGIC_ITEM_NAMES[ magicLevel ] ) );
					}
				}	else if( !strcmp( p, "$protective" ) ) {
					if( stateModSet ) {
						if( strlen( s ) ) strcat( s, " " );
						strcat( s, _( "Protective" ) );
					}
				} else if( !strcmp( p, "$slaying" ) ) {
					if( damageMultiplier > 1 ) {
						if( strlen( s ) ) strcat( s, " " );
						strcat( s, _( "Slaying" ) );
					}
					if( strlen( s ) ) strcat( s, " " );
				} else if( !strcmp( p, "$itemname" ) ) {
					if( strlen( s ) ) strcat( s, " " );
					strcat( s, itemName );
				} else if( !strcmp( p, "$bonus" ) ) {
					if(bonus > 0) {
						if( strlen( s ) ) strcat( s, " " );
						sprintf(tmp, " (+%d)", bonus);
						strcat(s, tmp);
					}
				} else if( !strcmp( p, "$symbol" ) ) {
					if( skillBonus.size() > 0 ) {
						if( school ) {
							if( strlen( s ) ) strcat( s, " " );
							strcat( s, school->getSymbol() );
						} else if( stateModSet ) {
							bool stateModFound = false;
							for( int i = 0; i < StateMod::STATE_MOD_COUNT; i++ ) {
								if( stateMod[ i ] > 0 ) {
									if( strlen( s ) ) strcat( s, " " );
									strcat( s, StateMod::stateMods[ i ]->getSymbol() );
									stateModFound = true;
									break;
								}
							}
							if( !stateModFound ) {
								// use the first skill as the noun
								map<int,int>::iterator i = skillBonus.begin();
								int skill = i->first;
								if( strlen( s ) ) strcat( s, " " );
								strcat( s, Skill::skills[ skill ]->getSymbol() );
							}
						}						
					}
				} else if( *p != '$' ) {
					if( strlen( s ) ) strcat( s, " " );
					strcat( s, p );
				}
				p = strtok( NULL, " " );
			}
		} else {
			sprintf( s, "??? %s ???", itemName );
		}
	} else {
		strcpy( s, itemName );
	}
}

bool Item::isSpecial() { 
  return getRpgItem()->isSpecial(); 
}

int Item::rollMagicDamage() { 
  return (magicDamage ? magicDamage->roll() : 0); 
}

char *Item::describeMagicDamage() { 
  return (magicDamage ? magicDamage->toString() : NULL);
}

void Item::debugMagic(char *s) {
  RpgItem *item = getRpgItem();
  cerr << s << endl;
  cerr << "Magic item: " << item->getName() << "(+" << bonus << ")" << endl;
  cerr << "\tdamageMultiplier=" << damageMultiplier << " vs. monsterType=" << (monsterType ? monsterType : "null") << endl;
  cerr << "\tSchool: " << (school ? school->getName() : "null") << endl;
  cerr << "\tstate mods:" << endl;
  for(int i = 0; i < StateMod::STATE_MOD_COUNT; i++) {
    if(this->isStateModSet(i)) cerr << "set: " << StateMod::stateMods[i]->getDisplayName() << endl;
    if(this->isStateModProtected(i)) cerr << "protected: " << StateMod::stateMods[i]->getDisplayName() << endl;
  }
  cerr << "\tskill bonuses:" << endl;
  for(map<int, int>::iterator i=skillBonus.begin(); i!=skillBonus.end(); ++i) {
    int skill = i->first;
    int bonus = i->second;
    cerr << "\t\t" << Skill::skills[skill]->getDisplayName() << " +" << bonus << endl;
  }
  cerr << "-----------" << endl;
}

void Item::setCurrentCharges( int n ) { 
  if( n < 0 ) n=0; 
  if( n>rpgItem->getMaxCharges() )
    n = rpgItem->getMaxCharges(); 
  currentCharges = n; 
} 

void Item::setSpell( Spell *spell ) { 
  this->spell = spell; 
  if( getRpgItem()->getType() == RpgItem::SCROLL ) {
    sprintf( this->itemName, _( "Scroll of %s" ), spell->getDisplayName() ); 
  } else {
    describeMagic( itemName, rpgItem->getDisplayName() );
  }
}

const char *Item::getName() {
  return getItemName();
}

int Item::getIconTileX() {
  return rpgItem->getIconTileX();
}

int Item::getIconTileY() {
  return rpgItem->getIconTileY();
}

int Item::getRange() {
	return getRpgItem()->getRange();
}

int Item::getStorableType() {
  return Storable::ITEM_STORABLE;
}

const char *Item::isStorable() {
  return NULL;
}

char *Item::getType() {
	// how is an item saved in a map? (this is not displayName)
	return getRpgItem()->getName();
}


/**
 * Sets identification bit to true if random function eveluates more than infoDetailLevel
 * with specified cap modifier.
 * @param bit 
 * @param modifier 
 * @param infoDetailLevel 
 */
void Item::trySetIDBit(int bit, float modifier, int infoDetailLevel) {
	//If not yet set
	if(!getIdentifiedBit( bit ))
	{
		if(infoDetailLevel > (int)(modifier * rand()/RAND_MAX)) {
			setIdentifiedBit( bit, true );
		} else {
			setIdentifiedBit( bit, false );
		}
	}
}

void Item::identify( int infoDetailLevel ) {
#ifdef DEBUG_IDENTIFY_ITEM
	infoDetailLevel = 500;
#endif
	//identifiedBits = (Uint32)0x0000;
	if( isMagicItem() ) {
		trySetIDBit(Item::ID_BONUS,100.0f, infoDetailLevel);
		if( getDamageMultiplier() > 1 ) {
			trySetIDBit(Item::ID_DAMAGE_MUL,100.0f, infoDetailLevel);
    		} else {
			setIdentifiedBit( Item::ID_DAMAGE_MUL, true );
		}
		if(getSchool() ) {
			trySetIDBit(Item::ID_MAGIC_DAMAGE,100.0f, infoDetailLevel);
		} else {
			setIdentifiedBit( Item::ID_MAGIC_DAMAGE, true );
		}

		bool found = false;
		for(int i = 0; i < StateMod::STATE_MOD_COUNT; i++) {
			if(isStateModSet(i)) {
				found = true;
				break;
			}
		}
		if( found ) {
			trySetIDBit(Item::ID_STATE_MOD,100.0f, infoDetailLevel);
		} else {
			setIdentifiedBit( Item::ID_STATE_MOD, true );
		}

		found = false;
		for(int i = 0; i < StateMod::STATE_MOD_COUNT; i++) {
			if(isStateModProtected(i)) {
				found = true;
				break;
			}
		}
		if(found) {
			trySetIDBit(Item::ID_PROT_STATE_MOD,100.0f, infoDetailLevel);
		} else {
			setIdentifiedBit( Item::ID_PROT_STATE_MOD, true );
		}

		found = false;
		map<int,int> *skillBonusMap = getSkillBonusMap();
		for(map<int, int>::iterator i=skillBonusMap->begin(); i!=skillBonusMap->end(); ++i) {
			found = true;
			break;
		}
		if(found) {
			trySetIDBit(Item::ID_SKILL_BONUS,100.0f, infoDetailLevel);
		} else {
			setIdentifiedBit( Item::ID_SKILL_BONUS, true );
		}
    
		// cursed is hard to detect
    if( isCursed() ) {
			trySetIDBit(Item::ID_SKILL_BONUS,200.0f, infoDetailLevel);
		} else {
			setIdentifiedBit( Item::ID_CURSED, true );
		}

		if( isIdentified() ) {
			describeMagic( itemName, rpgItem->getDisplayName() );
			session->getGameAdapter()->addDescription( _( "An item was fully identified!" ) );
			// update ui
			session->getGameAdapter()->refreshInventoryUI();
		}
	} else {
		//No need for identification - item not magical
		identifiedBits = (Uint32)0xffff;
	}
	// fprintf( stderr, "skill=%d id=%x\n", infoDetailLevel, identifiedBits );
}

int Item::getInventoryWidth() { 
	return ( getShape()->getIcon() > 0 ? getShape()->getIconWidth() : rpgItem->getInventoryWidth() );
}

int Item::getInventoryHeight() { 
	return ( getShape()->getIcon() > 0 ? getShape()->getIconHeight() : rpgItem->getInventoryHeight() ); 
}

void Item::renderIcon( Scourge *scourge, int x, int y, int w, int h ) {
	if( isMagicItem() ) {
		renderUnderItemIconEffect( scourge, x, y, w, h );
	}
	renderItemIcon( scourge, x, y, w, h );
	if( isMagicItem() ) {
		renderItemIconEffect( scourge, x, y, w, h );
		renderItemIconIdentificationEffect( scourge, x, y, w, h );
	}
}

void Item::renderItemIcon( Scourge *scourge, int x, int y, int w, int h ) {

	//create3dTex( scourge, w, h );

	glColor4f( 1, 1, 1, 1 );
	GLuint tex = ( getShape()->getIcon() > 0 ? 
								 getShape()->getIcon() :
								 session->getShapePalette()->
								 tilesTex[ getRpgItem()->getIconTileX() ][ getRpgItem()->getIconTileY() ] );

	//glEnable( GL_ALPHA_TEST );
	//glAlphaFunc( GL_NOTEQUAL, 0 );
	glColor4f( 1, 1, 1, 1 );
	glEnable( GL_BLEND );
	//glBlendFunc( GL_SRC_ALPHA, GL_ONE );
	glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );
	glBindTexture( GL_TEXTURE_2D, tex );
	glBegin( GL_QUADS );
	glTexCoord2d( 0, 1 );
	glVertex2d( x, y + h );
	glTexCoord2d( 0, 0 );
	glVertex2d( x, y );
	glTexCoord2d( 1, 0 );
	glVertex2d( x + w, y );
	glTexCoord2d( 1, 1 );
	glVertex2d( x + w, y + h );
	glEnd();
	glDisable( GL_BLEND );
	//glDisable( GL_ALPHA_TEST );
}

void Item::create3dTex( Scourge *scourge, float w, float h ) {
	if( textureInMemory ) return;

	// clear the error flags
	Util::getOpenGLError();

  // Create texture and copy minimap date from backbuffer on it    
	unsigned int textureSizeW = 32;
	unsigned int textureSizeH = 32;
  textureInMemory = (unsigned char *) malloc( textureSizeW * textureSizeH * 4);    

	glPushAttrib( GL_ALL_ATTRIB_BITS );
	glDisable( GL_CULL_FACE );
  glEnable( GL_DEPTH_TEST );
	glDepthMask( GL_TRUE );
  glEnable( GL_TEXTURE_2D );
  glDisable( GL_BLEND );
	glDisable( GL_ALPHA_TEST );
	glDisable( GL_SCISSOR_TEST );
	glDisable( GL_STENCIL_TEST );
  //glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );

  glGenTextures(1, tex3d);    
  glBindTexture(GL_TEXTURE_2D, tex3d[0]); 
  glPixelStorei(GL_UNPACK_ALIGNMENT, 1);        
  glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);                                          // filtre appliqué a la texture
  glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR);  
  glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_S, GL_CLAMP );
  glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_T, GL_CLAMP ); 
  glTexImage2D( GL_TEXTURE_2D, 0, GL_RGBA, textureSizeW, textureSizeH, 0,
                GL_RGBA, GL_UNSIGNED_BYTE, textureInMemory );                       
	fprintf(stderr, "OpenGl result for item(%s) glTexImage2D : %s\n", getName(), Util::getOpenGLError());

	glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
	glClearColor( 0, 0, 0, 0 );
	glClearDepth( 1 );
 
	glScalef( ( this->getShape()->getWidth() ) / w,
						( this->getShape()->getHeight() ) / h,
						1 );
  glPushMatrix();  
  glLoadIdentity();
	this->getShape()->rotateIcon();
	glColor4f( 1, 1, 1, 1 );	
	this->getShape()->draw();
	glPopMatrix();
	glScalef( 1, 1, 1 );
	
	//SDL_GL_SwapBuffers( );


  // Copy to a texture
	glPushMatrix();
  glLoadIdentity();
  glEnable(GL_TEXTURE_2D);
  glBindTexture(GL_TEXTURE_2D, tex3d[0]);
  glCopyTexSubImage2D(
                     GL_TEXTURE_2D,
                     0,      // MIPMAP level
                     0,      // x texture offset
                     0,      // y texture offset
                     0,              // x window coordinates
                     scourge->getScreenHeight() - textureSizeH,   // y window coordinates
                     textureSizeW,    // width
                     textureSizeH     // height
                     ); 
	glPopMatrix();
	fprintf(stderr, "OpenGl result for item(%s) glCopyTexSubImage2D: %s\n", getName(), Util::getOpenGLError());          
  
  glPopAttrib();
}

void Item::renderUnderItemIconEffect( Scourge *scourge, int x, int y, int w, int h ) {
	Uint32 t = SDL_GetTicks();
	if( t - iconUnderEffectTimer > 5 ) {
		iconUnderEffectTimer = t;
		for( int i = 0; i < PARTICLE_COUNT; i++ ) {
			if( iconUnderEffectParticle[i]->life < 0 || 
					iconUnderEffectParticle[i]->life >= iconUnderEffectParticle[i]->maxLife ) {
				iconUnderEffectParticle[i]->life = 0;
				iconUnderEffectParticle[i]->maxLife = (int)( 30.0f * rand() / RAND_MAX ) + 30;
				iconUnderEffectParticle[i]->zoom = 5.0f * rand() / RAND_MAX + 10.0f;
				iconUnderEffectParticle[i]->x = (w / 4.0f) * rand() / RAND_MAX + (w * 0.375f);
				iconUnderEffectParticle[i]->y = (h / 4.0f) * rand() / RAND_MAX + (h * 0.375f);
				iconUnderEffectParticle[i]->z = 0;
			}
			iconUnderEffectParticle[i]->zoom += 1.0f;
			//iconUnderEffectParticle[i]->y += 0.25f;
			iconUnderEffectParticle[i]->life++;
		}
	}
	glEnable( GL_TEXTURE_2D );
	glBindTexture( GL_TEXTURE_2D, 
								 scourge->getSession()->getShapePalette()->getNamedTexture( "flame" ) );
	glEnable( GL_BLEND );
	//glBlendFunc( GL_ONE, GL_ONE );
	glBlendFunc(GL_SRC_ALPHA, GL_ONE);
	//glBlendFunc( GL_SRC_ALPHA, GL_ONE );
	//glBlendFunc( GL_ONE, GL_ONE_MINUS_SRC_ALPHA );
	for( int i = 0; i < PARTICLE_COUNT; i++ ) {
		float a = ( 1 - iconUnderEffectParticle[i]->life / (float)iconUnderEffectParticle[i]->maxLife ) / 8.0f;
		glColor4f( Constants::MAGIC_ITEM_COLOR[ getMagicLevel() ]->r * a,
							 Constants::MAGIC_ITEM_COLOR[ getMagicLevel() ]->g * a,
							 Constants::MAGIC_ITEM_COLOR[ getMagicLevel() ]->b * a,
							 0.5f );
		glPushMatrix();
		glTranslatef( iconUnderEffectParticle[i]->x - iconUnderEffectParticle[i]->zoom / 2, 
									iconUnderEffectParticle[i]->y - iconUnderEffectParticle[i]->zoom / 2, 0 );
		//glRotatef( iconUnderEffectParticle[i]->life, 0, 0, 1 );
		glBegin( GL_QUADS );
		glTexCoord2d( 0, 1 );
		glVertex2d( x, y + iconUnderEffectParticle[i]->zoom );
		glTexCoord2d( 0, 0 );
		glVertex2d( x, y );
		glTexCoord2d( 1, 0 );
		glVertex2d( x + iconUnderEffectParticle[i]->zoom, y );
		glTexCoord2d( 1, 1 );
		glVertex2d( x + iconUnderEffectParticle[i]->zoom, y + iconUnderEffectParticle[i]->zoom );
		glEnd();
		glPopMatrix();
	}
	glDisable( GL_BLEND );
	glColor4f( 1, 1, 1, 1 );
}

void Item::renderItemIconEffect( Scourge *scourge, int x, int y, int w, int h ) {
	// draw an effect
	Uint32 t = SDL_GetTicks();
	if( t - iconEffectTimer > 5 ) {
		iconEffectTimer = t;
		for( int i = 0; i < PARTICLE_COUNT / 5; i++ ) {
			if( iconEffectParticle[i]->life < 0 || 
					iconEffectParticle[i]->life >= iconEffectParticle[i]->maxLife ) {
				iconEffectParticle[i]->life = 0;
				iconEffectParticle[i]->maxLife = (int)( 30.0f * rand() / RAND_MAX ) + 30;
				iconEffectParticle[i]->zoom = 0.5f;
				iconEffectParticle[i]->x = (float)w * rand() / RAND_MAX;
				iconEffectParticle[i]->y = (float)h * rand() / RAND_MAX;
				iconEffectParticle[i]->z = 0;
			}
			iconEffectParticle[i]->zoom += ( iconEffectParticle[i]->life >= iconEffectParticle[i]->maxLife / 2.0f ? -1 : 1 ) * 0.5f;
			//iconEffectParticle[i]->y += 0.25f;
			iconEffectParticle[i]->life++;
		}
	}
	glEnable( GL_TEXTURE_2D );
	glBindTexture( GL_TEXTURE_2D, 
								 scourge->getSession()->getShapePalette()->getNamedTexture( "bling" ) );
	glEnable( GL_BLEND );
	//glBlendFunc( GL_ONE, GL_ONE );
	glBlendFunc(GL_SRC_ALPHA, GL_ONE);
	//glBlendFunc( GL_SRC_ALPHA, GL_ONE );
	//glBlendFunc( GL_ONE, GL_ONE_MINUS_SRC_ALPHA );
	for( int i = 0; i < PARTICLE_COUNT / 5; i++ ) {
		float a = ( iconEffectParticle[i]->life / (float)(iconEffectParticle[i]->maxLife) );
		if( a >= 0.5 ) a = 1 - a;
		a = a * 2.0f;
		glColor4f( Constants::MAGIC_ITEM_COLOR[ getMagicLevel() ]->r * a,
							 Constants::MAGIC_ITEM_COLOR[ getMagicLevel() ]->g * a,
							 Constants::MAGIC_ITEM_COLOR[ getMagicLevel() ]->b * a,
							 1 );
		glPushMatrix();
		//glRotatef( 45, 0, 0, 1 );
		glTranslatef( iconEffectParticle[i]->x - iconEffectParticle[i]->zoom / 2, 
									iconEffectParticle[i]->y - iconEffectParticle[i]->zoom / 2, 0 );
		glBegin( GL_QUADS );
		glTexCoord2d( 0, 1 );
		glVertex2d( x, y + iconEffectParticle[i]->zoom );
		glTexCoord2d( 0, 0 );
		glVertex2d( x, y );
		glTexCoord2d( 1, 0 );
		glVertex2d( x + iconEffectParticle[i]->zoom, y );
		glTexCoord2d( 1, 1 );
		glVertex2d( x + iconEffectParticle[i]->zoom, y + iconEffectParticle[i]->zoom );
		glEnd();
		glPopMatrix();
	}
	glDisable( GL_BLEND );
	glColor4f( 1, 1, 1, 1 );
}

void Item::renderItemIconIdentificationEffect( Scourge *scourge, int x, int y, int w, int h ) {
	if( isIdentified() ) {
		glDisable( GL_TEXTURE_2D );
		glColor4f( Constants::MAGIC_ITEM_COLOR[ getMagicLevel() ]->r,
							 Constants::MAGIC_ITEM_COLOR[ getMagicLevel() ]->g,
							 Constants::MAGIC_ITEM_COLOR[ getMagicLevel() ]->b,
							 1 );
		glBegin( GL_LINE_LOOP );
		glVertex2d( x + 1, y + h - 1 );
		glVertex2d( x + 1, y + 1 );
		glVertex2d( x + w - 1, y + 1 );
		glVertex2d( x + w - 1, y + h - 1 );
		glEnd();
		glEnable( GL_TEXTURE_2D );
		glColor4f( 1, 1, 1, 1 );
	} else {
		scourge->getSDLHandler()->texPrint( x + 2, y + 12, "?" );
	}
}		

void Item::getTooltip( char *tooltip ) {
	char tmp[500];
	strcpy( tooltip, getName() );
	if( getRpgItem()->isWeapon() ) {
		sprintf( tmp, "|%s:%d%%(%c)", 
						 _( "DAM" ), getRpgItem()->getDamage(),
						 RpgItem::getDamageTypeLetter( getRpgItem()->getDamageType() ) );
		strcat( tooltip, tmp );
		if( getRpgItem()->getAP() > 0 ) {
			sprintf( tmp, " %s:%d", _( "AP" ), getRpgItem()->getAP() );
			strcat( tooltip, tmp );
		}
		if( getRange() > MIN_DISTANCE ) {
			sprintf( tmp, " %s:%d", _( "RANGE" ), getRange() );
			strcat( tooltip, tmp );
		}
	} else if( getRpgItem()->isArmor() ) {
		strcat( tooltip, "|" );
		strcat( tooltip, _( "DEF" ) );
		strcat( tooltip, ":" );
		for( int i = 0; i < RpgItem::DAMAGE_TYPE_COUNT; i++ ) {
			sprintf(tmp, _( " %d(%c)" ), 
							getRpgItem()->getDefense( i ),
							RpgItem::getDamageTypeLetter( i ) );
			strcat( tooltip, tmp );
		}
	}
	if( getLevel() > 1 ) {
		sprintf( tmp, "|%s:%d", 
						 _( "Level" ), getLevel() );
		strcat( tooltip, tmp );
	}
	if( getRpgItem()->getPotionPower() ) {
		sprintf( tmp, "|%s:%d", _( "Power" ), getRpgItem()->getPotionPower() );
		strcat( tooltip, tmp );
	}
	if( getRpgItem()->getMaxCharges() > 0 &&
			( !getRpgItem()->hasSpell() || getSpell() ) ) {
		sprintf( tmp, "|%s:%d(%d)", _( "Charges" ), getCurrentCharges(), getRpgItem()->getMaxCharges() );
		strcat( tooltip, tmp );
		if( getSpell() ) {
			sprintf( tmp, "|%s:%s", _( "Spell" ), getSpell()->getDisplayName() );
			strcat( tooltip, tmp );
		}
	}
	if( getRpgItem()->getPotionTime() > 0 ) {
		sprintf( tmp, "|%s:%d", _( "Duration" ), getRpgItem()->getPotionTime());
		strcat( tooltip, tmp );
	}
}
