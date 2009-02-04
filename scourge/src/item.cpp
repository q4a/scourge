/***************************************************************************
               item.cpp  -  Class representing a specific item
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

#include "common/constants.h"
#include "item.h"
#include "render/renderlib.h"
#include "rpg/rpglib.h"
#include "session.h"
#include "shapepalette.h"
#include "configlang.h"
#include "creature.h"

using namespace std;

// ###### MS Visual C++ specific ###### 
#if defined(_MSC_VER) && defined(_DEBUG)
# define new DEBUG_NEW
# undef THIS_FILE
  static char THIS_FILE[] = __FILE__;
#endif 


Item::Item( Session *session, RpgItem *rpgItem, int level, bool loading ) {
	this->session = session;
	this->rpgItem = rpgItem;
	this->level = level;
	this->shapeIndex = ( rpgItem ? this->rpgItem->getShapeIndex() : 0 );
	this->color = NULL;
	this->shape = session->getShapePalette()->getShape( shapeIndex );
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
	snprintf( this->itemName, ITEM_NAME_SIZE, "%s", rpgItem->getDisplayName() );
	backpackX = backpackY = 0;
	inventoryOf = NULL;

	commonInit( loading );

	currentCharges = rpgItem->getMaxCharges();
	weight = rpgItem->getWeight();
}

Item::~Item() {
	/* unused:
	if ( textureInMemory != NULL ) {
	 free( textureInMemory );
	 textureInMemory = NULL;
	 glDeleteTextures( 1, tex3d );
	}
	*/

	for ( int i = 0; i < PARTICLE_COUNT; i++ ) {
		delete iconUnderEffectParticle[i];
		iconUnderEffectParticle[i] = NULL;
		delete iconEffectParticle[i];
		iconEffectParticle[i] = NULL;
	}
}

ItemInfo *Item::save() {
	ItemInfo *info = new ItemInfo;
	info->version = PERSIST_VERSION;
	info->level = getLevel();
	strcpy( ( char* )info->rpgItem_name, getRpgItem()->getName() );
	strcpy( ( char* )info->shape_name, getShape()->getName() );
	info->blocking = blocking;
	info->currentCharges = currentCharges;
	info->weight = ( Uint32 )( weight * 100 );
	info->quality = quality;
	info->price = price;
	info->identifiedBits = identifiedBits;

	// spells
	strcpy( ( char* )info->spell_name, ( spell ? spell->getName() : "" ) );

	// container
	int realCount = 0;
	for ( int i = 0; i < containedItemCount; i++ ) {
		if ( containedItems[i] ) info->containedItems[realCount++] = containedItems[i]->save();
	}
	info->containedItemCount = realCount;

	// magic item
	info->bonus = bonus;
	info->damageMultiplier = damageMultiplier;
	info->cursed = cursed;
	info->magicLevel = magicLevel;
	strcpy( ( char* )info->monster_type, ( this->monsterType ? monsterType : "" ) );
	strcpy( ( char* )info->magic_school_name, ( this->school ? school->getName() : "" ) );
	info->magicDamage = ( school ? saveDice( magicDamage ) : saveEmptyDice() );
	for ( int i = 0; i < StateMod::STATE_MOD_COUNT; i++ ) {
		info->stateMod[i] = this->stateMod[i];
	}
	for ( int i = 0; i < Skill::SKILL_COUNT; i++ ) {
		info->skillBonus[i] = this->getSkillBonus( i );
	}

	info->missionId = getMissionId();
	info->missionObjectiveIndex = getMissionObjectiveIndex();
	return info;
}

DiceInfo *Item::saveDice( Dice *dice ) {
	DiceInfo *info = new DiceInfo;
	info->version = PERSIST_VERSION;
	info->count = dice->getCount();
	info->sides = dice->getSides();
	info->mod = dice->getMod();
	return info;
}

DiceInfo *Item::saveEmptyDice() {
	DiceInfo *info = new DiceInfo;
	info->version = PERSIST_VERSION;
	info->count = 0;
	info->sides = 0;
	info->mod = 0;
	return info;
}

/// The item's damage dice (number of dices, sides per dice, modifier; example: 2d6+2 )

Dice *Item::loadDice( Session *session, DiceInfo *info ) {
	if ( !info->count ) return NULL;
	return new Dice( info->count, info->sides, info->mod );
}


Item *Item::load( Session *session, ItemInfo *info ) {
	if ( !strlen( ( char* )info->rpgItem_name ) ) return NULL;
	Spell *spell = NULL;
	if ( strlen( ( char* )info->spell_name ) ) spell = Spell::getSpellByName( ( char* )info->spell_name );
	RpgItem *rpgItem = RpgItem::getItemByName( ( char* )info->rpgItem_name );
	if ( !rpgItem ) {
		cerr << "Error: can't find rpgItem with name:" << ( char* )info->rpgItem_name << endl;
		return NULL;
	}
	Item *item = session->newItem( rpgItem,
	                               info->level,
	                               spell,
	                               true );
	item->blocking = ( info->blocking == 1 );
	item->currentCharges = info->currentCharges;
	item->weight = static_cast<float>( info->weight ) / 100.0f;
	item->quality = info->quality;
	item->price = info->price;
	item->identifiedBits = info->identifiedBits;

	// container
	item->containedItemCount = 0;
	for ( int i = 0; i < static_cast<int>( info->containedItemCount ); i++ ) {
		Item *containedItem = Item::load( session, info->containedItems[i] );
		if ( containedItem ) {
			item->addContainedItem( containedItem );
		}
	}

	// magic item
	item->bonus = info->bonus;
	item->damageMultiplier = info->damageMultiplier;
	item->cursed = ( info->cursed == 1 );
	item->magicLevel = info->magicLevel;
	// get a reference to the real string... (yuck)
	item->monsterType = Monster::getMonsterType( ( char* )info->monster_type );
	// turn off "vs. any creature"
	if ( !item->getMonsterType() ) item->damageMultiplier = 1;
	item->school = MagicSchool::getMagicSchoolByName( ( char* )info->magic_school_name );
	item->magicDamage = Item::loadDice( session, info->magicDamage );
	for ( int i = 0; i < StateMod::STATE_MOD_COUNT; i++ ) {
		item->stateMod[i] = info->stateMod[i];
	}
	for ( int i = 0; i < Skill::SKILL_COUNT; i++ ) {
		if ( info->skillBonus[i] ) item->skillBonus[i] = info->skillBonus[i];
	}

	item->setMissionObjectInfo( static_cast<int>( info->missionId ), static_cast<int>( info->missionObjectiveIndex ) );

	// re-describe the item. describeMagic is called from commonInit at
	// which point magicLevel can be 0, so it's important to re-describe
	// the item. (since later magicLevel can be -1)
	item->describeMagic( item->rpgItem->getDisplayName() );

	return item;
}

/// Puts another item inside this item.

bool Item::addContainedItem( Item *item, int itemX, int itemY, bool force ) {
	if ( containedItemCount < MAX_CONTAINED_ITEMS && ( findInventoryPosition( item, itemX, itemY, true ) || force ) ) {
		containedItems[containedItemCount++] = item;
		if ( item->isMagicItem() ) containsMagicItem = true;
		return true;
	} else {
		cerr << "Warning: unable to add to container. Container=" << getRpgItem()->getName() << " item=" << item->getRpgItem()->getName() << endl;
		cerr << "\tcontainer: " << getName() << endl;
		cerr << "\tcontainedItemCount " << containedItemCount << " < " << MAX_CONTAINED_ITEMS << endl;
//		cerr << "\tforce: " << force << endl;
		return false;
	}
}

/// Find an inventory position for an item

/// note: optimize this,
/// current O(n^2)

bool Item::findInventoryPosition( Item *item, int posX, int posY, bool useExistingLocationForSameItem ) {
	if ( item ) {
		int colCount = getRpgItem()->getContainerWidth();
		int rowCount = getRpgItem()->getContainerHeight();

		int selX = -1;
		int selY = -1;

		for ( int xx = 0; xx < colCount; xx++ ) {
			for ( int yy = 0; yy < rowCount; yy++ ) {
				if ( xx + item->getBackpackWidth() <= colCount &&
				        yy + item->getBackpackHeight() <= rowCount &&
				        checkInventoryLocation( item, useExistingLocationForSameItem, xx, yy ) ) {
					if ( posX == xx && posY == yy ) {
						selX = xx;
						selY = yy;
						break;
					} else if ( selX == -1 ) {
						selX = xx;
						selY = yy;
					}
				}
			}
		}

		if ( selX > -1 ) {
			item->setBackpackLocation( selX, selY );
			return true;
		}
	}
	return false;
}

/// Checks whether an item fits into the inventory at pos xx,yy.

bool Item::checkInventoryLocation( Item *item, bool useExistingLocationForSameItem, int xx, int yy ) {
	SDL_Rect itemRect;
	itemRect.x = xx;
	itemRect.y = yy;
	itemRect.w = item->getBackpackWidth();
	itemRect.h = item->getBackpackHeight();
	for ( int t = 0; t < getContainedItemCount(); t++ ) {
		Item *i = getContainedItem( t );
		if( inventoryOf && inventoryOf->isEquipped( i ) ) {
			continue;
		}
		if ( i == item ) {
			if ( useExistingLocationForSameItem ) {
				return true;
			} else {
				continue;
			}
		}

		SDL_Rect iRect;
		iRect.x = i->getBackpackX();
		iRect.y = i->getBackpackY();
		iRect.w = i->getBackpackWidth();
		iRect.h = i->getBackpackHeight();

		if ( SDLHandler::intersects( &itemRect, &iRect ) ) return false;
	}
	return true;
}

/// Removes a contained item by index.

void Item::removeContainedItem( Item *item ) {
	for( int t = 0; t < containedItemCount; t++ ) {
		if( item == containedItems[ t ] ) {
			containedItemCount--;
			for ( int i = t; i < containedItemCount; i++ ) {
				containedItems[i] = containedItems[i + 1];
			}
			containsMagicItem = false;
			for ( int i = 0; i < containedItemCount; i++ ) {
				if ( containedItems[i]->isMagicItem() ) containsMagicItem = true;
			}
			return;
		}
	}
}

/// Returns a contained item by index.

Item *Item::getContainedItem( int index ) {
	return( ( index >= 0 && index < containedItemCount ) ? containedItems[index] : NULL );
}

void Item::setContainedItem( int index, Item *item ) {
	if( index >= 0 && index < containedItemCount ) {
		containedItems[ index ] = item;
	}
}

/// Returns whether the item is inside a container.

bool Item::isContainedItem( Item *item ) {
	for ( int i = 0; i < containedItemCount; i++ ) {
		if ( containedItems[i] == item ||
		        ( containedItems[i]->getRpgItem()->getType() == RpgItem::CONTAINER &&
		          containedItems[i]->isContainedItem( item ) ) ) return true;
	}
	return false;
}

/// Creates a brief one-line description of the item.

void Item::getDetailedDescription( std::string& s, bool precise ) {
	RpgItem * rpgItem  = getRpgItem();
	int type = rpgItem->getType();
	char str[20];
	snprintf( str, 20, _( "(L:%d) " ), getLevel() );
	s = str;
	if ( isCursed() && getShowCursed() ) s += _( "*Cursed* " );

	if ( type == RpgItem::SCROLL ) {
		s += itemName;
	} else {
		s += precise ? itemName : rpgItem->getShortDesc();
	}

	if ( missionId > 0 ) s += _( " *Mission*" );
}

char *trim( char *s ) {
	for ( char *p = s; p; p++ ) {
		if ( !( *p == ' ' || *p == '\t' || *p == '\r' || *p == '\n' ) ) {
			return p;
		}
	}
	return s;
}

/// Decrements the number of charges/uses. Returns true if the item is used up.

bool Item::decrementCharges() {
	float f1;
	int oldCharges;

	oldCharges = getCurrentCharges();
	if ( oldCharges <= 1 ) {
		// The object is totally consummed
		return true;
	}
	setCurrentCharges( oldCharges - 1 );

	// Compute initial weight to be able to compute new weight
	// (without increasing error each time)

	f1 = getWeight();
	f1 *= static_cast<float>( getRpgItem()->getMaxCharges() );
	f1 /= static_cast<float>( oldCharges );
	f1 *= ( ( static_cast<float>( oldCharges ) - 1.0f ) / static_cast<float>( getRpgItem()->getMaxCharges() ) );
	setWeight( f1 );
	return false;
}




void Item::commonInit( bool loading ) {
	// unused: tex3d[0] = 0;
	// unused: textureInMemory = NULL;
	iconEffectTimer = 0;
	iconUnderEffectTimer = 0;
	for ( int i = 0; i < PARTICLE_COUNT; i++ ) {
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
	quality = Util::pickOne( 50, 100 ); // starts out mostly healthy

	int basePrice = ( this->spell ? this->spell->getExp() : rpgItem->getPrice() );
	price = basePrice + static_cast<int>( Util::getRandomSum( static_cast<float>( basePrice / 2 ), level ) );

	// assign a spell to the item
	// the deeper you go, the more likely that items contain spells
	if ( rpgItem->hasSpell() &&
	        0 == Util::dice( MAX_MISSION_DEPTH - ( session->getCurrentMission() ? session->getCurrentMission()->getDepth() : 0 ) ) ) {
		this->spell = MagicSchool::getRandomSpell( 1 );
		price += static_cast<int>( Util::getRandomSum( static_cast<float>( basePrice / 2 ), this->spell->getLevel() ) );
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
	for ( int i = 0; i < StateMod::STATE_MOD_COUNT; i++ ) stateMod[i] = 0;

	if ( rpgItem->isEnchantable() && !loading && session->getGameAdapter()->getCurrentDepth() > 0 ) {
		// roll for magic
		int n = Util::dice( static_cast<int>( 200.0f - ( level * 1.5f ) ) );
		if ( n < 5 && session->getGameAdapter()->getCurrentDepth() >= 4 ) enchant( Constants::DIVINE_MAGIC_ITEM );
		else if ( n < 10 && session->getGameAdapter()->getCurrentDepth() >= 3 ) enchant( Constants::CHAMPION_MAGIC_ITEM );
		else if ( n < 20 && session->getGameAdapter()->getCurrentDepth() >= 2 ) enchant( Constants::GREATER_MAGIC_ITEM );
		else if ( n < 30 && session->getGameAdapter()->getCurrentDepth() >= 1 ) enchant( Constants::LESSER_MAGIC_ITEM );
	}

	// describe spell-holding items also
	if ( magicLevel < 0 && RpgItem::itemTypes[ rpgItem->getType() ].hasSpell ) {
		describeMagic( rpgItem->getDisplayName() );
	}
}

/// Enchants the item.

void Item::enchant( int newMagicLevel ) {
	if ( magicLevel != -1 ) return;

	magicLevel = newMagicLevel;
	if ( magicLevel >= Constants::MAGIC_ITEM_LEVEL_COUNT ) magicLevel = Constants::MAGIC_ITEM_LEVEL_COUNT - 1;

	// item level caps the magic level:
	// 0-9: lesser
	// 10-19: greater
	// 20-29: champion
	// 39+ : divine
	// This is so low level items won't be too powerful.
	int maxMagicLevel = level / 10;
	if ( magicLevel > maxMagicLevel ) magicLevel = maxMagicLevel;

	cursed = ( 0 == Util::dice( 20 ) );

	// adjust the price
	price *= ( magicLevel + 2 );

	int n;
	Spell *spell;
	switch ( magicLevel ) {
	case Constants::LESSER_MAGIC_ITEM:
		bonus = Util::pickOne( 1, 2 );
		if ( rpgItem->isWeapon() ) {
			damageMultiplier = Util::pickOne( 2, 3 );
			monsterType = Monster::getRandomMonsterType( level );
		}
		n = Util::pickOne( 2, 4 );
		for ( int i = 0; i < n; i++ ) {
			int skill = SkillGroup::stats->getRandomSkill()->getIndex();
			if ( skillBonus.find( skill ) == skillBonus.end() ) {
				skillBonus[skill] = Util::pickOne( 1, 2 );
			}
		}
		break;
	case Constants::GREATER_MAGIC_ITEM:
		bonus = Util::pickOne( 1, 3 );
		if ( rpgItem->isWeapon() ) {
			damageMultiplier = Util::pickOne( 2, 4 );
			monsterType = Monster::getRandomMonsterType( level );
		}
		spell = MagicSchool::getRandomSpell( 1 );
		if ( spell ) {
			school = spell->getSchool();
			magicDamage = new Dice( 1, Util::pickOne( 1, 3 ), Util::dice( 3 ) );
		}
		n = Util::pickOne( 2, 4 );
		for ( int i = 0; i < n; i++ ) {
			int skill = SkillGroup::stats->getRandomSkill()->getIndex();
			if ( skillBonus.find( skill ) == skillBonus.end() ) {
				skillBonus[skill] = Util::pickOne( 1, 3 );
			}
		}
		break;
	case Constants::CHAMPION_MAGIC_ITEM:
		bonus = Util::pickOne( 1, 4 );
		if ( rpgItem->isWeapon() ) {
			damageMultiplier = Util::pickOne( 2, 4 );
			monsterType = Monster::getRandomMonsterType( level );
		}
		spell = MagicSchool::getRandomSpell( 1 );
		if ( spell ) {
			school = spell->getSchool();
			magicDamage = new Dice( 1, Util::pickOne( 2, 4 ), Util::dice( 3 ) );
		}
		n = Util::pickOne( 1, 3 );
		if ( n > 0 ) stateModSet = true;
		for ( int i = 0; i < n; i++ ) {
			stateMod[ StateMod::getRandomGood()->getIndex() ] = 1;
		}
		n = Util::pickOne( 1, 3 );
		for ( int i = 0; i < n; i++ ) {
			int skill = SkillGroup::stats->getRandomSkill()->getIndex();
			if ( skillBonus.find( skill ) == skillBonus.end() ) {
				skillBonus[skill] = Util::pickOne( 1, 4 );
			}
		}
		break;
	case Constants::DIVINE_MAGIC_ITEM:
		bonus = Util::pickOne( 1, 5 );
		if ( rpgItem->isWeapon() ) {
			damageMultiplier = Util::pickOne( 2, 5 );
			monsterType = Monster::getRandomMonsterType( level );
		}
		spell = MagicSchool::getRandomSpell( 1 );
		if ( spell ) {
			school = spell->getSchool();
			magicDamage = new Dice( 1, Util::pickOne( 3, 5 ), Util::dice( 3 ) );
		}
		n = Util::pickOne( 1, 3 );
		if ( n > 0 ) stateModSet = true;
		for ( int i = 0; i < n; i++ ) {
			stateMod[ StateMod::getRandomGood()->getIndex() ] = 1;
		}
		n = Util::pickOne( 1, 3 );
		if ( n > 0 ) stateModSet = true;
		for ( int i = 0; i < n; i++ ) {
			stateMod[ StateMod::getRandomBad()->getIndex() ] = 2;
		}
		n = Util::pickOne( 2, 4 );
		for ( int i = 0; i < n; i++ ) {
			int skill = SkillGroup::stats->getRandomSkill()->getIndex();
			if ( skillBonus.find( skill ) == skillBonus.end() ) {
				skillBonus[skill] = Util::pickOne( 1, 5 );
			}
		}
		break;
	default:
		cerr << "*** Error: unknown magic level: " << magicLevel << endl;
	}

	// turn off "vs. any creature"
	if ( !monsterType ) damageMultiplier = 1;

	describeMagic( rpgItem->getDisplayName() );
}

// max about 30 points (must be deterministic)
int Item::getMagicResistance() {
	return( 3 * ( ( getLevel() / 10 ) + getMagicLevel() ) );
}

#define DEBUG_ITEM_ID 0

/// Adds the magical properties, symbols etc. to the item's name.

void Item::describeMagic( char const* displayName ) {

	// not for scrolls :-(
	if ( rpgItem->getType() == RpgItem::SCROLL ) return;

	// e.g.: Lesser broadsword + 3 of nature magic
	enum { TMP_SIZE = 80 };
	char tmp[TMP_SIZE];


	if ( magicLevel > -1 ) {
		if ( DEBUG_ITEM_ID || isIdentified() ) {
			char format[1000];
			/* This represents the format of an identified item. Please do not translate the strings,
			only rearrange them to work with your language.

			An example of the final string created by the code after parsing this message would be:
			"Ethereal Greater Protective Slaying Longsword (+4) of the Dragon"
			*/
			strcpy( format, _( "$spellsymbol $magiclevel $protective $slaying $itemname $bonus $symbol" ) );
			char *p = strtok( format, " " );
			strcpy( itemName, "" );
			while ( p ) {

				if ( !strcmp( p, "$spellsymbol" ) ) {
					if ( RpgItem::itemTypes[ rpgItem->getType() ].hasSpell && spell ) {
						if ( strlen( itemName ) ) strcat( itemName, " " );
						strcat( itemName, spell->getSymbol() );
					}
				} else if ( !strcmp( p, "$magiclevel" ) ) {
					if ( magicLevel > -1 ) {
						if ( strlen( itemName ) ) strcat( itemName, " " );
						strcat( itemName, _( Constants::MAGIC_ITEM_NAMES[ magicLevel ] ) );
					}
				} else if ( !strcmp( p, "$protective" ) ) {
					if ( stateModSet ) {
						if ( strlen( itemName ) ) strcat( itemName, " " );
						strcat( itemName, _( "Protective" ) );
					}
				} else if ( !strcmp( p, "$slaying" ) ) {
					if ( damageMultiplier > 1 ) {
						if ( strlen( itemName ) ) strcat( itemName, " " );
						strcat( itemName, _( "Slaying" ) );
					}
					if ( strlen( itemName ) ) strcat( itemName, " " );
				} else if ( !strcmp( p, "$itemname" ) ) {
					if ( strlen( itemName ) ) strcat( itemName, " " );
					strcat( itemName, displayName );
				} else if ( !strcmp( p, "$bonus" ) ) {
					if ( bonus > 0 ) {
						if ( strlen( itemName ) ) strcat( itemName, " " );
						snprintf( tmp, TMP_SIZE, " (+%d)", bonus );
						strcat( itemName, tmp );
					}
				} else if ( !strcmp( p, "$symbol" ) ) {
					if ( skillBonus.size() > 0 ) {
						if ( school ) {
							if ( strlen( itemName ) ) strcat( itemName, " " );
							strcat( itemName, school->getSymbol() );
						} else if ( stateModSet ) {
							bool stateModFound = false;
							for ( int i = 0; i < StateMod::STATE_MOD_COUNT; i++ ) {
								if ( stateMod[ i ] > 0 ) {
									if ( strlen( itemName ) ) strcat( itemName, " " );
									strcat( itemName, StateMod::stateMods[ i ]->getSymbol() );
									stateModFound = true;
									break;
								}
							}
							if ( !stateModFound ) {
								// use the first skill as the noun
								map<int, int>::iterator i = skillBonus.begin();
								int skill = i->first;
								if ( strlen( itemName ) ) strcat( itemName, " " );
								strcat( itemName, Skill::skills[ skill ]->getSymbol() );
							}
						}
					}
				} else if ( *p != '$' ) {
					if ( strlen( itemName ) ) strcat( itemName, " " );
					strcat( itemName, p );
				}
				p = strtok( NULL, " " );
			}
		} else {
			snprintf( itemName, ITEM_NAME_SIZE, "??? %s ???", displayName );
		}
	} else {
		strcpy( itemName, displayName );
	}
}

/// Is this a special (mission or story related) item?

bool Item::isSpecial() {
	return getRpgItem()->isSpecial();
}

/// Rolls additional magical damage.

int Item::rollMagicDamage() {
	return ( magicDamage ? magicDamage->roll() : 0 );
}

/// Returns the bonus damage (e.g. "+3") as a string.

char *Item::describeMagicDamage() {
	return ( magicDamage ? magicDamage->toString() : NULL );
}

void Item::debugMagic( char *s ) {
	RpgItem *item = getRpgItem();
	cerr << s << endl;
	cerr << "Magic item: " << item->getName() << "(+" << bonus << ")" << endl;
	cerr << "\tdamageMultiplier=" << damageMultiplier << " vs. monsterType=" << ( monsterType ? monsterType : "null" ) << endl;
	cerr << "\tSchool: " << ( school ? school->getName() : "null" ) << endl;
	cerr << "\tstate mods:" << endl;
	for ( int i = 0; i < StateMod::STATE_MOD_COUNT; i++ ) {
		if ( this->isStateModSet( i ) ) cerr << "set: " << StateMod::stateMods[i]->getDisplayName() << endl;
		if ( this->isStateModProtected( i ) ) cerr << "protected: " << StateMod::stateMods[i]->getDisplayName() << endl;
	}
	cerr << "\tskill bonuses:" << endl;
	for ( map<int, int>::iterator i = skillBonus.begin(); i != skillBonus.end(); ++i ) {
		int skill = i->first;
		int bonus = i->second;
		cerr << "\t\t" << Skill::skills[skill]->getDisplayName() << " +" << bonus << endl;
	}
	cerr << "-----------" << endl;
}

/// Sets the number of remaining charges/uses.

void Item::setCurrentCharges( int n ) {
	if ( n < 0 ) n = 0;
	if ( n > rpgItem->getMaxCharges() )
		n = rpgItem->getMaxCharges();
	currentCharges = n;
}

/// Sets the item's attached spell. Can create spell scrolls.

void Item::setSpell( Spell *spell ) {
	this->spell = spell;
	if ( getRpgItem()->getType() == RpgItem::SCROLL ) {
		snprintf( itemName, ITEM_NAME_SIZE, _( "Scroll of %s" ), spell->getDisplayName() );
	} else {
		describeMagic( rpgItem->getDisplayName() );
	}
}

/// The item's localized name.

const char *Item::getName() {
	return getItemName();
}

/// Which icon tile from tiles.png to use? (deprecated)

int Item::getIconTileX() {
	return rpgItem->getIconTileX();
}

/// Which icon tile from tiles.png to use? (deprecated)

int Item::getIconTileY() {
	return rpgItem->getIconTileY();
}

/// Max shooting distance for ranged weapons.

int Item::getRange() {
	return getRpgItem()->getRange();
}

/// Items are always storable (in a quickspell slot).

int Item::getStorableType() {
	return Storable::ITEM_STORABLE;
}

const char *Item::isStorable() {
	return NULL;
}

/// The type of item (weapon, armor, special etc.).

char const* Item::getType() {
	// how is an item saved in a map? (this is not displayName)
	return getRpgItem()->getName();
}

/// Tries to identify an unidentified item property.

/// Sets identification bit to true if random function eveluates more than infoDetailLevel
/// with specified cap modifier.
/// @param bit
/// @param modifier
/// @param infoDetailLevel

void Item::trySetIDBit( int bit, float modifier, int infoDetailLevel ) {
	//If not yet set
	if ( !getIdentifiedBit( bit ) ) {
		if ( infoDetailLevel > static_cast<int>( Util::roll( 0.0f, modifier ) ) ) {
			setIdentifiedBit( bit, true );
		} else {
			setIdentifiedBit( bit, false );
		}
	}
}

/// Tries to identify unidentified item properties.

/// A higher value of infoDetailLevel decreases the success
/// rate.

void Item::identify( int infoDetailLevel ) {
#ifdef DEBUG_IDENTIFY_ITEM
	infoDetailLevel = 500;
#endif
	//identifiedBits = (Uint32)0x0000;
	if ( isMagicItem() ) {
		trySetIDBit( Item::ID_BONUS, 100.0f, infoDetailLevel );
		if ( getDamageMultiplier() > 1 ) {
			trySetIDBit( Item::ID_DAMAGE_MUL, 100.0f, infoDetailLevel );
		} else {
			setIdentifiedBit( Item::ID_DAMAGE_MUL, true );
		}
		if ( getSchool() ) {
			trySetIDBit( Item::ID_MAGIC_DAMAGE, 100.0f, infoDetailLevel );
		} else {
			setIdentifiedBit( Item::ID_MAGIC_DAMAGE, true );
		}

		bool found = false;
		for ( int i = 0; i < StateMod::STATE_MOD_COUNT; i++ ) {
			if ( isStateModSet( i ) ) {
				found = true;
				break;
			}
		}
		if ( found ) {
			trySetIDBit( Item::ID_STATE_MOD, 100.0f, infoDetailLevel );
		} else {
			setIdentifiedBit( Item::ID_STATE_MOD, true );
		}

		found = false;
		for ( int i = 0; i < StateMod::STATE_MOD_COUNT; i++ ) {
			if ( isStateModProtected( i ) ) {
				found = true;
				break;
			}
		}
		if ( found ) {
			trySetIDBit( Item::ID_PROT_STATE_MOD, 100.0f, infoDetailLevel );
		} else {
			setIdentifiedBit( Item::ID_PROT_STATE_MOD, true );
		}

		found = false;
		map<int, int> *skillBonusMap = getSkillBonusMap();
		for ( map<int, int>::iterator i = skillBonusMap->begin(); i != skillBonusMap->end(); ++i ) {
			found = true;
			break;
		}
		if ( found ) {
			trySetIDBit( Item::ID_SKILL_BONUS, 100.0f, infoDetailLevel );
		} else {
			setIdentifiedBit( Item::ID_SKILL_BONUS, true );
		}

		// cursed is hard to detect
		if ( isCursed() ) {
			trySetIDBit( Item::ID_SKILL_BONUS, 200.0f, infoDetailLevel );
		} else {
			setIdentifiedBit( Item::ID_CURSED, true );
		}

		if ( isIdentified() ) {
			describeMagic( rpgItem->getDisplayName() );
			session->getGameAdapter()->writeLogMessage( _( "An item was fully identified!" ) );
			// update ui
			session->getGameAdapter()->refreshBackpackUI();
		}
	} else {
		//No need for identification - item not magical
		identifiedBits = ( Uint32 )0xffff;
	}
	// fprintf( stderr, "skill=%d id=%x\n", infoDetailLevel, identifiedBits );
}

/// Backpack x size.

int Item::getBackpackWidth() {
	return ( getShape()->getIcon().isSpecified() ? getShape()->getIconWidth() : rpgItem->getBackpackWidth() );
}

/// Backpack y size.

int Item::getBackpackHeight() {
	return ( getShape()->getIcon().isSpecified() ? getShape()->getIconHeight() : rpgItem->getBackpackHeight() );
}

void Item::renderIcon( Scourge *scourge, SDL_Rect *rect, int gridSize, bool smallIcon ) {
	int iw = getBackpackWidth() * gridSize;
	int ih = getBackpackHeight() * gridSize;

	int iy = rect->y;
	if ( rect->h - ih > gridSize ) iy += rect->h - ih - gridSize;
	renderIcon( scourge, rect->x + ( rect->w - iw ) / 2, iy, iw, ih, smallIcon );
}

/// Renders the item's icon and any overlaid effects
void Item::renderIcon( Scourge *scourge, int x, int y, int w, int h, bool smallIcon ) {
	Texture tex;
	int rw, rh, ox, oy, iw, ih;
	// getItemIconInfo( &tex, &rw, &rh, &ox, &oy, &iw, &ih, w, h, smallIcon );
	getItemIconInfo( &tex, &rw, &rh, &ox, &oy, &iw, &ih, w, h, false );
	glPushMatrix();
	glTranslatef( x + ox, y + oy, 0 );
// if ( !smallIcon ) {
	if ( w > 0 && h > 0 ) glScalef( rw / static_cast<float>( w ), rh / static_cast<float>( h ), 1 );
	if ( isMagicItem() ) {
		renderUnderItemIconEffect( scourge, 0, 0, rw, rh, iw, ih );
	}
// }
	// renderItemIcon( scourge, 0, 0, rw, rh, smallIcon );
	renderItemIcon( scourge, 0, 0, rw, rh, false );
// if ( !smallIcon ) {
	if ( isMagicItem() ) {
		renderItemIconEffect( scourge, 0, 0, rw, rh, iw, ih );
		renderItemIconIdentificationEffect( scourge, 0, 0, rw, rh );
	}
	glScalef( 1, 1, 1 );
// }
	glPopMatrix();
}

/// Returns the item's icon with additional info.

/// The following is returned: The OpenGL texture, width and height of the texture,
/// and the top left corner of the item graphic within the texture.
/// When smallIcon is false, it returns the backpack graphic, else the small icon.

void Item::getItemIconInfo( Texture* texp, int *rwp, int *rhp, int *oxp, int *oyp, int *iw, int *ih, int w, int h, bool smallIcon ) {
	Texture tex;
	int rw, rh, ox, oy;
	if ( !smallIcon && getShape()->getIcon().isSpecified() ) {
		tex = getShape()->getIcon();
		*iw = getShape()->getIconWidth() * 32;
		*ih = getShape()->getIconHeight() * 32;
		if ( getShape()->getIconWidth() > getShape()->getIconHeight() ) {
			rw = w;
			rh = static_cast<int>( getShape()->getIconHeight() * rw / static_cast<float>( getShape()->getIconWidth() ) );
			ox = 0;
			oy = ( h - rh ) / 2;
		} else {
			rh = h;
			rw = static_cast<int>( getShape()->getIconWidth() * rh / static_cast<float>( getShape()->getIconHeight() ) );
			oy = 0;
			ox = ( w - rw ) / 2;
		}
	} else {
		tex = session->getShapePalette()->tilesTex[ getRpgItem()->getIconTileX() ][ getRpgItem()->getIconTileY() ];
		*iw = w;
		*ih = h;
		rw = w;
		rh = h;
		ox = oy = 0;
	}
	*texp = tex;
	*rwp = rw;
	*rhp = rh;
	*oxp = ox;
	*oyp = oy;
}

/// Renders the item's icon (in lists, the backpack etc.)

void Item::renderItemIcon( Scourge *scourge, int x, int y, int w, int h, bool smallIcon ) {
	glColor4f( 1, 1, 1, 1 );
	glEnable( GL_BLEND );
	glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );
	glEnable( GL_TEXTURE_2D );
	getItemIconTexture( smallIcon ).glBind();
	glBegin( GL_TRIANGLE_STRIP );
	glTexCoord2d( 0, 0 );
	glVertex2d( x, y );
	glTexCoord2d( 1, 0 );
	glVertex2d( x + w, y );
	glTexCoord2d( 0, 1 );
	glVertex2d( x, y + h );
	glTexCoord2d( 1, 1 );
	glVertex2d( x + w, y + h );
	glEnd();
	glDisable( GL_BLEND );
}

/// Returns the item's icon texture.

Texture Item::getItemIconTexture( bool smallIcon ) {
	return ( !smallIcon && getShape()->getIcon().isSpecified() ? getShape()->getIcon() :
	         session->getShapePalette()->tilesTex[ getRpgItem()->getIconTileX() ][ getRpgItem()->getIconTileY() ] );
}

/// Creates an icon texture from a 3D view of the item.

/* unused
void Item::create3dTex( Scourge *scourge, float w, float h ) {
 if ( textureInMemory ) return;

 // clear the error flags
 Util::getOpenGLError();

 // Create texture and copy minimap date from backbuffer on it
 unsigned int textureSizeW = 32;
 unsigned int textureSizeH = 32;
 textureInMemory = ( unsigned char * ) malloc( textureSizeW * textureSizeH * 4 );

 glPushAttrib( GL_ALL_ATTRIB_BITS );

 glDisable( GL_CULL_FACE );
 glEnable( GL_DEPTH_TEST );
 glDepthMask( GL_TRUE );
 glEnable( GL_TEXTURE_2D );
 glDisable( GL_BLEND );
 glDisable( GL_SCISSOR_TEST );
 glDisable( GL_STENCIL_TEST );

 glGenTextures( 1, tex3d );
 glBindTexture( GL_TEXTURE_2D, tex3d[0] );
 glPixelStorei( GL_UNPACK_ALIGNMENT, 1 );
 glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );                                      // filtre appliqu� a la texture
 glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
 glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP );
 glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP );
 glTexImage2D( GL_TEXTURE_2D, 0, ( scourge->getPreferences()->getBpp() > 16 ? GL_RGBA : GL_RGBA4 ), textureSizeW, textureSizeH, 0,
               GL_RGBA, GL_UNSIGNED_BYTE, textureInMemory );
 fprintf( stderr, "OpenGl result for item(%s) glTexImage2D : %s\n", getName(), Util::getOpenGLError() );

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
 glBindTexture( GL_TEXTURE_2D, tex3d[0] );
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
 fprintf( stderr, "OpenGl result for item(%s) glCopyTexSubImage2D: %s\n", getName(), Util::getOpenGLError() );

 glDisable( GL_TEXTURE_2D );
 glPopAttrib();
}
*/

void Item::renderUnderItemIconEffect( Scourge *scourge, int x, int y, int w, int h, int iw, int ih ) {
	Uint32 t = SDL_GetTicks();
	if ( t - iconUnderEffectTimer > 5 ) {
		iconUnderEffectTimer = t;
		for ( int i = 0; i < PARTICLE_COUNT; i++ ) {
			if ( iconUnderEffectParticle[i]->life < 0 ||
			        iconUnderEffectParticle[i]->life >= iconUnderEffectParticle[i]->maxLife ) {
				iconUnderEffectParticle[i]->life = 0;
				iconUnderEffectParticle[i]->maxLife = Util::pickOne( 30, 59 );
				iconUnderEffectParticle[i]->zoom = Util::roll( 10.0f, 15.0f );
				iconUnderEffectParticle[i]->x = Util::roll( 0.0f, w / 4.0f ) + ( w * 0.375f );
				iconUnderEffectParticle[i]->y = Util::roll( 0.0f, h / 4.0f ) + ( h * 0.375f );
				iconUnderEffectParticle[i]->z = 0;
			}
			iconUnderEffectParticle[i]->zoom += 1.0f;
			//iconUnderEffectParticle[i]->y += 0.25f;
			iconUnderEffectParticle[i]->life++;
		}
	}
	glEnable( GL_TEXTURE_2D );
	scourge->getSession()->getShapePalette()->getNamedTexture( "flame" ).glBind();
	glEnable( GL_BLEND );
	//glBlendFunc( GL_ONE, GL_ONE );
	glBlendFunc( GL_SRC_ALPHA, GL_ONE );
	//glBlendFunc( GL_SRC_ALPHA, GL_ONE );
	//glBlendFunc( GL_ONE, GL_ONE_MINUS_SRC_ALPHA );
	for ( int i = 0; i < PARTICLE_COUNT; i++ ) {
		float a = ( 1 - iconUnderEffectParticle[i]->life / static_cast<float>( iconUnderEffectParticle[i]->maxLife ) ) / 8.0f;
		glColor4f( Constants::MAGIC_ITEM_COLOR[ getMagicLevel() ]->r * a,
		           Constants::MAGIC_ITEM_COLOR[ getMagicLevel() ]->g * a,
		           Constants::MAGIC_ITEM_COLOR[ getMagicLevel() ]->b * a,
		           0.5f );
		glPushMatrix();
		glTranslatef( x, y, 0 );
		glTranslatef( iconUnderEffectParticle[i]->x - iconUnderEffectParticle[i]->zoom / 2,
		              iconUnderEffectParticle[i]->y - iconUnderEffectParticle[i]->zoom / 2, 0 );
		//glRotatef( iconUnderEffectParticle[i]->life, 0, 0, 1 );
		glBegin( GL_TRIANGLE_STRIP );
		glTexCoord2d( 0, 0 );
		glVertex2d( 0, 0 );
		glTexCoord2d( 1, 0 );
		glVertex2d( iconUnderEffectParticle[i]->zoom, 0 );
		glTexCoord2d( 0, 1 );
		glVertex2d( 0, iconUnderEffectParticle[i]->zoom );
		glTexCoord2d( 1, 1 );
		glVertex2d( iconUnderEffectParticle[i]->zoom, iconUnderEffectParticle[i]->zoom );
		glEnd();
		glPopMatrix();
	}
	glDisable( GL_BLEND );
	glColor4f( 1, 1, 1, 1 );
}

/// Renders the "blinking stars" effect for magical items.

void Item::renderItemIconEffect( Scourge *scourge, int x, int y, int w, int h, int iw, int ih ) {
	int particleCount = 3 * ( getMagicLevel() + 1 );
	// draw an effect
	Uint32 t = SDL_GetTicks();
	if ( t - iconEffectTimer > 5 ) {
		iconEffectTimer = t;
		for ( int i = 0; i < particleCount; i++ ) {
			if ( iconEffectParticle[i]->life < 0 ||
			        iconEffectParticle[i]->life >= iconEffectParticle[i]->maxLife ) {
				iconEffectParticle[i]->life = 0;
				iconEffectParticle[i]->maxLife = Util::pickOne( 30, 59 );
				iconEffectParticle[i]->zoom = 0.5f;
				iconEffectParticle[i]->x = static_cast<float>( w ) * Util::mt_rand();
				iconEffectParticle[i]->y = static_cast<float>( h ) * Util::mt_rand();
				iconEffectParticle[i]->z = 0;
			}
			iconEffectParticle[i]->zoom += ( iconEffectParticle[i]->life >= iconEffectParticle[i]->maxLife / 2.0f ? -1 : 1 ) * 0.5f;
			//iconEffectParticle[i]->y += 0.25f;
			iconEffectParticle[i]->life++;
		}
	}
	glEnable( GL_TEXTURE_2D );
	scourge->getSession()->getShapePalette()->getNamedTexture( "bling" ).glBind();
	glEnable( GL_BLEND );
	//glBlendFunc( GL_ONE, GL_ONE );
	glBlendFunc( GL_SRC_ALPHA, GL_ONE );
	//glBlendFunc( GL_SRC_ALPHA, GL_ONE );
	//glBlendFunc( GL_ONE, GL_ONE_MINUS_SRC_ALPHA );
	for ( int i = 0; i < particleCount; i++ ) {
		float a = ( iconEffectParticle[i]->life / static_cast<float>( iconEffectParticle[i]->maxLife ) );
		if ( a >= 0.5 ) a = 1 - a;
		a = a * 2.0f;
		glColor4f( Constants::MAGIC_ITEM_COLOR[ getMagicLevel() ]->r * a,
		           Constants::MAGIC_ITEM_COLOR[ getMagicLevel() ]->g * a,
		           Constants::MAGIC_ITEM_COLOR[ getMagicLevel() ]->b * a,
		           1 );
		glPushMatrix();
		glTranslatef( iconEffectParticle[i]->x - iconEffectParticle[i]->zoom / 2,
		              iconEffectParticle[i]->y - iconEffectParticle[i]->zoom / 2, 0 );
		if ( getMagicLevel() >= Constants::DIVINE_MAGIC_ITEM ) {
			glRotatef( 360.0f * a, 0, 0, 1 );
		}
		glBegin( GL_TRIANGLE_STRIP );
		glTexCoord2d( 0, 0 );
		glVertex2d( x, y );
		glTexCoord2d( 1, 0 );
		glVertex2d( x + iconEffectParticle[i]->zoom, y );
		glTexCoord2d( 0, 1 );
		glVertex2d( x, y + iconEffectParticle[i]->zoom );
		glTexCoord2d( 1, 1 );
		glVertex2d( x + iconEffectParticle[i]->zoom, y + iconEffectParticle[i]->zoom );
		glEnd();
		glPopMatrix();
	}
	glDisable( GL_BLEND );
	glColor4f( 1, 1, 1, 1 );
}

/// Renders the effect for unidentified items. Currently only prints "?"

void Item::renderItemIconIdentificationEffect( Scourge *scourge, int x, int y, int w, int h ) {
	if ( isIdentified() ) {
		/*
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
		*/
	} else {
		scourge->getSDLHandler()->texPrint( x + 2, y + 12, "?" );
	}
}

/// Item's tooltip text (for the backpack).

void Item::getTooltip( char *tooltip ) {
	enum { TMP_SIZE = 500 };
	char tmp[ TMP_SIZE ];
	strcpy( tooltip, getName() );
	if ( getRpgItem()->isWeapon() ) {
		snprintf( tmp, TMP_SIZE, "|%s:%d%%(%c)",
		          _( "DAM" ), getRpgItem()->getDamage(),
		          RpgItem::getDamageTypeLetter( getRpgItem()->getDamageType() ) );
		strcat( tooltip, tmp );
		if ( getRpgItem()->getAP() > 0 ) {
			snprintf( tmp, TMP_SIZE, " %s:%d", _( "AP" ), getRpgItem()->getAP() );
			strcat( tooltip, tmp );
		}
		if ( getRange() > MIN_DISTANCE ) {
			snprintf( tmp, TMP_SIZE, " %s:%d", _( "RANGE" ), getRange() );
			strcat( tooltip, tmp );
		}
	} else if ( getRpgItem()->isArmor() ) {
		strcat( tooltip, "|" );
		strcat( tooltip, _( "DEF" ) );
		strcat( tooltip, ":" );
		for ( int i = 0; i < RpgItem::DAMAGE_TYPE_COUNT; i++ ) {
			snprintf( tmp, TMP_SIZE, _( " %d(%c)" ),
			          getRpgItem()->getDefense( i ),
			          RpgItem::getDamageTypeLetter( i ) );
			strcat( tooltip, tmp );
		}
	}
	if ( getLevel() > 1 ) {
		snprintf( tmp, TMP_SIZE, "|%s:%d",
		          _( "Level" ), getLevel() );
		strcat( tooltip, tmp );
	}
	if ( getRpgItem()->getPotionPower() ) {
		snprintf( tmp, TMP_SIZE, "|%s:%d", _( "Power" ), getRpgItem()->getPotionPower() );
		strcat( tooltip, tmp );
	}
	if ( getRpgItem()->getMaxCharges() > 0 &&
	        ( !getRpgItem()->hasSpell() || getSpell() ) ) {
		snprintf( tmp, TMP_SIZE, "|%s:%d(%d)", _( "Charges" ), getCurrentCharges(), getRpgItem()->getMaxCharges() );
		strcat( tooltip, tmp );
		if ( getSpell() ) {
			snprintf( tmp, TMP_SIZE, "|%s:%s", _( "Spell" ), getSpell()->getDisplayName() );
			strcat( tooltip, tmp );
		}
	}
	if ( getRpgItem()->getPotionTime() > 0 ) {
		snprintf( tmp, TMP_SIZE, "|%s:%d", _( "Duration" ), getRpgItem()->getPotionTime() );
		strcat( tooltip, tmp );
	}
}

bool Item::isFullyIdentified() {

	bool id = true;

	if ( magicLevel > -1 ) {
		if ( DEBUG_ITEM_ID || isIdentified() ) {
			id = true;
		} else {
			id = false;
		}
	}

	return id;
}

Texture Item::getContainerTexture() {
	if( !containerTexture.isSpecified() ) {
		containerTexture.load( getRpgItem()->getContainerTexture() );
	}
	return containerTexture;
}
