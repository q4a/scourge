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
#ifndef RPG_ITEM_H
#define RPG_ITEM_H
#pragma once

#include <map>
#include <string>
#include <vector>
#include <set>
#include "character.h"
#include "rpg.h"

/*
 * Remember to change isWeaponItem=, getRandomEnchantableItem,
 * getRandomItem, getRandomContainer, getRandomContainerNS
 *
 * when adding a new item or type.
 */

class Monster;
class Dice;
class MagicSchool;
class Spell;
class RpgItem;
class GameAdapter;
class ShapePalette;

/// Describes a type or class of items.
struct ItemType {
	char name[40];
	bool isWeapon, isArmor, isRandom, isRanged, hasSpell, isEnchantable;
	int backpackWidth, backpackHeight;
};

enum {
	AP_INFLUENCE = 0,
	CTH_INFLUENCE,
	DAM_INFLUENCE,
	INFLUENCE_TYPE_COUNT
};

enum {
	MIN_INFLUENCE = 0,
	MAX_INFLUENCE,
	INFLUENCE_LIMIT_COUNT
};

/// A special weapon bonus etc.

struct WeaponInfluence {
	int limit;
	char type;
	float base;
};

/// Describes a specific "base item".
class RpgItem {
private:

	// the basics
	char *name, *desc, *shortDesc, *displayName;
	int rareness;
	int type;
	float weight;
	int price;
	int shape_index;
	int equip; // where can it be worn?
	int minDepth; // 0-based min. depth where the item occurs
	int minLevel; // if >0 the item is "special" and only one instance exists
	int maxCharges; // max, how many uses?
	int iconTileX, iconTileY; // its graphic (1-based)
	int containerWidth, containerHeight; // the container window size
	char containerTexture[255];
	std::set<std::string> tags; // the tags of this object

	// weapon
	int damage; // % of strength bonus
	int damageType; // a damageType value
	int damageSkill; // which skill to check when using the item
	int parry; // what % of the damage skill to use to parry
	int ap; // ap cost
	int range; // weapon range
	int twohanded; // a twoHandedType value
	WeaponInfluence weaponInfluence[200][3][2]; // influence values

	// armor
	int *defense;
	int defenseSkill; // the defense skill used
	int dodgePenalty; // dodge penalty

	// potion
	int potionPower; // how strong is the brew?
	int potionSkill; // which skill does this potion effect?
	int potionTime; // how long does it last (in minutes)

	// spells
	int spellLevel;



	static std::map<int, std::map<int, std::vector<const RpgItem*>*>*> typesMap;
	static std::map<std::string, const RpgItem *> itemsByName;
	static std::vector<RpgItem*> containers;
	static std::vector<RpgItem*> containersNS;

	// linear for now... if there are too many of these we can organize them
	// in a type->depth map.
	static std::vector<RpgItem*> special;

	static void initItemTypes( ConfigLang *config );
	static void initSounds( ConfigLang *config );
	static void initTags( ConfigLang *config );
	static void initItemEntries( ConfigLang *config, ShapePalette *shapePal );
	void decodeInfluenceBlock( int skill, std::vector<ConfigNode*> *nodes, int influenceType );

public:

	enum {
		SWORD = 0,
		AXE,
		BOW,
		MACE,
		CONTAINER,
		ARMOR,
		FOOD,
		DRINK,
		POTION,
		OTHER,
		MISSION,
		SCROLL,
		POLE,
		WAND,
		RING,
		AMULET,
		STAFF,

		ITEM_TYPE_COUNT
	};

	static std::map<int, std::vector<std::string> *> soundMap;
	static std::vector<ItemType> itemTypes;
	static int randomTypes[ITEM_TYPE_COUNT];
	static int randomTypeCount;

	enum twoHandedType {
		NOT_TWO_HANDED = 0,
		ONLY_TWO_HANDED,
		OPTIONAL_TWO_HANDED
	};

	enum damageType {
		DAMAGE_TYPE_SLASHING = 0,
		DAMAGE_TYPE_PIERCING,
		DAMAGE_TYPE_CRUSHING,
		DAMAGE_TYPE_COUNT
	};
	static char *DAMAGE_TYPE_NAME[];
	static char DAMAGE_TYPE_LETTER[];
	static int getDamageTypeForLetter( char c );
	static char getDamageTypeLetter( int type );

	static RpgItem *items[1000];
	static int itemCount;

	static std::map<std::string, std::string> tagsDescriptions;
	static const char *getTagDescription( std::string tag );

	static void initItems( ShapePalette *shapePal );

	//static char *influenceTypeName[ INFLUENCE_TYPE_COUNT ];

	RpgItem( char *name, char *displayName,
	         int rareness, int type, float weight, int price,
	         char *desc, char *shortDesc, int equip, int shape_index,
	         int minDepth = 0, int minLevel = 0,
	         int maxCharges = 0,
	         int iconTileX = 0, int iconTileY = 0 );
	~RpgItem();

	// tags
	inline void addTag( std::string s ) {
		tags.insert( s );
	}
	inline bool hasTag( char *tag ) {
		std::string s = tag;
		return hasTag( s );
	}
	inline bool hasTag( std::string s ) {
		return( tags.find( s ) != tags.end() );
	}

	inline char *getName() {
		return name;
	}
	inline char *getDisplayName() {
		return displayName;
	}
	inline int getRareness()  {
		return rareness;
	}
	inline float getWeight() {
		return weight;
	}
	inline int getPrice() {
		return price;
	}
	inline int getMaxCharges() {
		return maxCharges;
	}
	inline int getShapeIndex() {
		return shape_index;
	}
	inline char *getShortDesc() {
		return shortDesc;
	}
	inline char *getLongDesc() {
		return desc;
	}
	inline int getEquip() {
		return equip;
	}
	inline int getMinDepth() {
		return minDepth;
	}
	inline int getMinLevel() {
		return minLevel;
	}
	inline int getType() {
		return type;
	}
	inline int getIconTileX() {
		return this->iconTileX;
	}
	inline int getIconTileY() {
		return this->iconTileY;
	}
	inline int getContainerWidth() {
		return this->containerWidth;
	}
	inline int getContainerHeight() {
		return this->containerHeight;
	}
	inline void setContainerWidth(int n) {
		this->containerWidth = n;
	}
	inline void setContainerHeight(int n) {
		this->containerHeight = n;
	}
	inline void setContainerTexture(char *s) {
		strcpy( this->containerTexture, s );
	}
	inline char *getContainerTexture() {
		return this->containerTexture;
	}

	// weapon
	inline void setWeapon( int damage, int damageType, int damageSkill, int parry, int ap, int range, int twohanded ) {
		this->damage = damage;
		this->damageType = damageType;
		this->damageSkill = damageSkill;
		this->parry = parry;
		this->ap = ap;
		this->range = range;
		this->twohanded = twohanded;
	}
	inline int getDamage() {
		return damage;
	}
	inline int getDamageType() {
		return damageType;
	}
	inline int getDamageSkill() {
		return damageSkill;
	}
	inline int getParry() {
		return parry;
	}
	inline int getAP() {
		return ap;
	}
	inline int getRange() {
		return range;
	}
	inline int getTwoHanded() {
		return twohanded;
	}
	void setWeaponInfluence( int skill, int type, int limit, WeaponInfluence influence );
	WeaponInfluence *getWeaponInfluence( int skill, int type, int limit );

	// armor
	inline void setArmor( int *defense, int defenseSkill, int dodgePenalty ) {
		for ( int i = 0; i < DAMAGE_TYPE_COUNT; i++ ) this->defense[ i ] = defense[ i ];
		this->defenseSkill = defenseSkill;
		this->dodgePenalty = dodgePenalty;
	}
	inline int getDefense( int damageType ) {
		return defense[ damageType ];
	}
	inline int getDefenseSkill() {
		return defenseSkill;
	}
	inline int getDodgePenalty() {
		return dodgePenalty;
	}

	// potion
	inline void setPotion( int potionPower, int potionSkill, int potionTime ) {
		this->potionPower = potionPower;
		this->potionSkill = potionSkill;
		this->potionTime = potionTime;
	}
	inline int getPotionPower() {
		return potionPower;
	}
	inline int getPotionSkill() {
		return potionSkill;
	}
	inline int getPotionTime() {
		return potionTime;
	}

	// spells
	inline void setSpellLevel( int spellLevel ) {
		this->spellLevel = spellLevel;
	}
	inline int getSpellLevel() {
		return spellLevel;
	}


	// query methods
	inline bool isWeapon() {
		return itemTypes[ type ].isWeapon;
	}
	inline bool isArmor() {
		return itemTypes[ type ].isArmor;
	}
	inline bool isSpecial() {
		return( minLevel > 0 );
	}
	// FIXME: make this more specific to item
	// e.g. multi-attack items, like sword of fireballs
	//inline bool isRangedWeapon() { return type == BOW; }
	inline bool isRangedWeapon() {
		return( itemTypes[ type ].isRanged );
	}
	inline bool isEnchantable() {
		return( itemTypes[ type ].isEnchantable );
	}
	inline bool isContainer() {
		return( type == CONTAINER ? true : false );
	}
	inline bool isScroll() {
		return( type == SCROLL ? true : false );
	}
	inline bool hasSpell() {
		return( itemTypes[ type ].hasSpell );
	}
	inline bool isOther() {
		return( type == OTHER ? true : false );
	}

	inline int getBackpackWidth() {
		return itemTypes[ type ].backpackWidth;
	}
	inline int getBackpackHeight() {
		return itemTypes[ type ].backpackHeight;
	}


	const std::string getRandomSound();

	static RpgItem *getRandomItem( int depth );
	static RpgItem *getRandomItemFromTypes( int level, int types[], int typeCount );
	static RpgItem *getRandomContainer();
	static RpgItem *getRandomContainerNS();
	inline static RpgItem *getItem( int index ) {
		return items[index];
	}

	static int getTypeByName( char *name );
	static void addItem( RpgItem *item, int width, int depth, int height );
	static RpgItem *getItemByName( char const* name );
	inline static std::map<std::string, const RpgItem *> *getItemMap() {
		return &itemsByName;
	}
	inline static int getSpecialCount() {
		return special.size();
	}
	inline static RpgItem *getSpecial( int index ) {
		return special[index];
	}
};

#endif
