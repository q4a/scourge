/***************************************************************************
                          party.cpp  -  Party class
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

#include "common/constants.h"
#include "party.h"
#include "render/renderlib.h"
#include "rpg/rpglib.h"
#include "item.h"
#include "creature.h"
#include "shapepalette.h"
#include "sqbinding/sqbinding.h"

using namespace std;

Creature *Party::lastPlayer = NULL;

//#define RANDOM_PARTY 1

//#define TEST_POSSESSION 1

#define PARTY_FOLLOW_INTERVAL 1500

Party::Party( Session *session ) {
	this->session = session;

	startRound = true;
	calendar = Calendar::getInstance();

	partySize = 0;

	loadedCount = 0;
	storylineIndex = 0;
}

Party::~Party() {
	// its static instance: delete calendar;
	deleteParty();
}

void Party::deleteParty() {
	partySet.clear();
	for ( int i = 0; i < getPartySize(); i++ ) {
		delete party[i];
		party[i] = NULL;
	}
	partySize = 0;
	player = NULL;
	lastPlayer = NULL;
}

void Party::reset() {
	maxSkills.clear();
	deleteParty();
	if ( loadedCount ) {
		for ( int i = 0; i < loadedCount; i++ ) {
			party[i] = loadedParty[i];
		}
		partySize = loadedCount;
		loadedCount = 0;
		session->getBoard()->setStorylineIndex( storylineIndex );
		storylineIndex = 0;
	} else {
		//createHardCodedParty(session, party, &partySize);
		session->getGameAdapter()->createParty( party, &partySize );
	}
	player = party[0];
	for ( int i = 0; i < partySize; i++ ) {
		partySet.insert( party[i] );
	}

#ifdef TEST_POSSESSION
	cerr << "****************************" << endl;
	cerr << "****************************" << endl;
	cerr << "Warning: possession testing is turned on." << endl;
	cerr << "****************************" << endl;
	cerr << "****************************" << endl;
	if ( partySize > 2 && party[2] ) party[2]->setStateMod( StateMod::possessed, true );
	if ( partySize > 3 && party[3] ) party[3]->setStateMod( StateMod::possessed, true );
#endif

	recomputeMaxSkills();
	if ( !session->getGameAdapter()->isHeadless() )
		session->getGameAdapter()->resetPartyUI();
}

void Party::resetMultiplayer( Creature *c ) {
	maxSkills.clear();
	deleteParty();
	party[0] = player = c;
	party[1] = party[2] = party[3] = NULL;
	partySize = 1;
	partySet.insert( c );
#ifdef HAVE_SDL_NET
	// upload your character to the server
	session->getClient()->sendCharacter( player->save() );
#endif
	session->getParty()->recomputeMaxSkills();
	if ( !session->getGameAdapter()->isHeadless() )
		session->getGameAdapter()->resetPartyUI();
}

// set player to be the first non-dead character
void Party::setFirstLivePlayer() {
	for ( int i = 0; i < getPartySize(); i++ ) {
		if ( !party[i]->getStateMod( StateMod::dead ) ) {
			//setPlayer(getParty(i));
			setPlayer( i );
			break;
		}
	}
}

int Party::getFirstLivePlayer() {
	for ( int i = 0; i < getPartySize(); i++ ) {
		if ( !party[i]->getStateMod( StateMod::dead ) ) {
			return i;
		}
	}
	return -1;
}

void Party::startPartyOnMission() {
	// Start calendar and add thirst & hunger event scheduling
	calendar->reset( false );

	player_only = false;
	partyDead = false;

	setFirstLivePlayer();
	setFormation( Creature::DIAMOND_FORMATION );
	getPlayer()->cancelTarget();

	// init the rest of the party
	for ( int i = 1; i < getPartySize(); i++ ) {
		//TODO: this is a formation thing. Replace with FormationPathManagers for the party
		//getParty(i)->setNext(getPlayer(), i);
		getParty( i )->cancelTarget();
		getParty( i )->resetSecretDoorAttempts();
		getParty( i )->resetTrapFindAttempts();
	}
}

void Party::setPartyMotion( int motion ) {
	for ( int i = 0; i < getPartySize(); i++ ) {
		if ( party[i] != player ) party[i]->setMotion( motion );
	}
}

/// Switches to the next alive party member.

/// returns false if the switch could not be made,
/// because the entire party is dead (the mission failed)

bool Party::switchToNextLivePartyMember() {
	Creature *oldPlayer = player;
	// find the player's index
	int n = getPlayerIndex();
	// switch to next player
	n++; if ( n >= getPartySize() ) n = 0;
	for ( int t = 0; t < getPartySize(); t++ ) {
		if ( !party[n]->getStateMod( StateMod::dead ) ) {
			setPlayer( n );
			break;
		}
		n++; if ( n >= getPartySize() ) n = 0;
	}
	bool res = ( oldPlayer != player );
	if ( !res ) partyDead = true;
	return res;
}

bool Party::nextPartyMember() {
	Creature *oldPlayer = player;
	int n = getPlayerIndex();
	n++;
	if ( n >= getPartySize() ) n = 0;
	setPlayer( n );
	return( oldPlayer != player );
}

bool Party::previousPartyMember() {
	Creature *oldPlayer = player;
	int n = getPlayerIndex();
	n--;
	if ( n < 0 ) n = getPartySize() - 1;
	setPlayer( n );
	return( oldPlayer != player );
}

int Party::getPlayerIndex() {
	for ( int t = 0; t < getPartySize(); t++ ) {
		if ( party[ t ] == player ) {
			return t;
		}
	}
	return -1;
}

void Party::setPlayer( int n, bool updateui ) {
	if ( n >= getPartySize() ) return;
	player = party[n];
	//player->setNextDontMove(NULL, 0);
	// init the rest of the party
// int count = 1;
	//for(int i = 0; i < getPartySize(); i++) {
	//if(i != n) party[i]->setNextDontMove(player, count++);
// }

	if ( updateui ) {
		//  move = 0;
		session->getMap()->refresh();
		session->getMap()->center( toint( player->getX() ), toint( player->getY() ), true );
		if ( !session->getGameAdapter()->isHeadless() ) {
			session->getGameAdapter()->refreshBackpackUI( n );
			session->getGameAdapter()->setPlayerUI( n );
		}

		// play selection sound
		if ( lastPlayer != player ) {
			if ( lastPlayer && !player->getStateMod( StateMod::dead ) ) {
				//session->playSound(player->getCharacter()->getRandomSound(Constants::SOUND_TYPE_SELECT));
				int panning = session->getMap()->getPanningFromMapXY( player->getX(), player->getY() );
				player->playCharacterSound( GameAdapter::SELECT_SOUND, panning );
			}
			lastPlayer = player;
		}
	}
}


void Party::forceStopRound() {
	startRound = false;
	toggleRound();
}

void Party::toggleRound( bool test ) {
	if ( startRound == test ) toggleRound();
}

void Party::toggleRound() {
	startRound = ( startRound ? false : true );
	if ( startRound ) {
		session->getGameAdapter()->writeLogMessage( Constants::getMessage( Constants::REAL_TIME_MODE ), Constants::MSGTYPE_SYSTEM );
	} else {
		session->getGameAdapter()->writeLogMessage( Constants::getMessage( Constants::TURN_MODE ), Constants::MSGTYPE_SYSTEM );
	}

	// Freeze / unfreeze calendar
	calendar->setPause( !startRound );

	// Freeze / unfreeze animations
	for ( int i = 0; i < getPartySize(); i++ ) {
		getParty( i )->getShape()->setPauseAnimation( !startRound );
	}
	for ( int i = 0; i < session->getCreatureCount(); i++ ) {
		session->getCreature( i )->getShape()->setPauseAnimation( !startRound );
	}

	if ( !session->getGameAdapter()->isHeadless() )
		session->getGameAdapter()->toggleRoundUI( startRound );
}

/**
 * Sets the target creature for the party.
 * As far as finding a path toward the target is concerned, this assumes that each party member is
 * using their preferred weapon. This path will be recomputed during battle anyway if this is not true.
 **/
void Party::setTargetCreature( Creature *creature ) {
	float range;
	if ( player_only ) {
		range = MIN_DISTANCE;
		if ( player->getPreferredWeapon() > -1 &&
		        player->getEquippedItem( player->getPreferredWeapon() ) ) {
			range = player->getEquippedItem( player->getPreferredWeapon() )->getRange();
		}
		player->setTargetCreature( creature, true, range );
	} else {
		for ( int i = 0; i < getPartySize(); i++ ) {
			range = MIN_DISTANCE;
			if ( party[i]->getPreferredWeapon() > -1 &&
			        party[i]->getEquippedItem( party[i]->getPreferredWeapon() ) ) {
				range = party[i]->getEquippedItem( party[i]->getPreferredWeapon() )->getRange();
			}
			party[i]->setTargetCreature( creature, true, range );
		}
	}
}

/**
 * Move the party someplace.
 * If in group mode and the selected player can't move to the desired
 * location, pick someone else from the group who can make the move.
 */
bool Party::setSelXY( Uint16 mapx, Uint16 mapy, bool cancelIfNotPossible ) {
	if ( isPlayerOnly() ) {
		getPlayer()->cancelTarget();
	} else {
		for ( int i = 0; i < getPartySize(); i++ ) {
			if ( !getParty( i )->getStateMod( StateMod::dead ) ) {
				getParty( i )->cancelTarget();
			}
		}
	}
	// Try to move the current player
	return getPlayer()->setSelXY( mapx, mapy, cancelIfNotPossible );
}

bool Party::isPartyInRange() {
	bool canMove = true;
	for ( int t = 0; t < getPartySize(); t++ ) {
		if ( !party[t]->getStateMod( StateMod::dead ) &&
		        party[t] != player ) {
			if ( party[t]->getDistanceToSel() > player->getDistanceToSel() &&
			        party[t]->getDistance( player ) > 7 ) {
				canMove = false;
				break;
			}
		}
	}
	return canMove;
}

void Party::movePlayers() {
	if ( player_only ) {
		// move everyone
		for ( int i = 0; i < getPartySize(); i++ ) {
			if ( !party[i]->getStateMod( StateMod::dead ) ) {
				party[i]->moveToLocator();
			}
		}
		// center on player
		session->getMap()->center( toint( player->getX() ), toint( player->getY() ) );
	} else { // In group mode:

		// move the leader
		if ( !player->getStateMod( StateMod::dead ) ) {
			//if( isPartyInRange() ) {
			player->moveToLocator();
			session->getMap()->center( toint( player->getX() ), toint( player->getY() ) );
		}

		// others follow the player
		for ( int t = 0; t < getPartySize(); t++ ) {
			if ( !party[t]->getStateMod( StateMod::dead ) && party[t] != player ) {
				// If the non-leader is done moving try to follow again.
				// This will be a no-op in follow() if we're close enough.
				if ( !getParty( t )->anyMovesLeft() ) {
					getParty( t )->follow( player );
				}
				// actually take a step
				party[t]->moveToLocator();
			}
		}
	}
}

int Party::getTotalLevel() {
	int totalLevel = 0;
	for ( int i = 0; i < getPartySize(); i++ )
		totalLevel += getParty( i )->getLevel();
	return totalLevel;
}

/**
   Create a party programmatically until the party editor is made.
 */
void Party::createHardCodedParty( Session *session, Creature **pc, int *partySize ) {
	int pcCount = 4;
	int level = 10;

	// FIXME: consider using newCreature here
	// the end of startMission would have to be modified to not delete the party
	// also in scourge, where-ever creatureCount is used to mean all monsters would have
	// to change (maybe that's a good thing too... same logic for party and monsters)
	pc[0] = new Creature( session,
	                      Character::getRandomCharacter(),
	                      "Alamont",
	                      Constants::SEX_MALE,
	                      0 );
	pc[0]->setLevel( level );
	pc[0]->setExp();
	pc[0]->setHp();
	pc[0]->setMp();
	pc[0]->setHunger( 8 );
	pc[0]->setThirst( 7 );
	pc[0]->setStateMod( StateMod::blessed, true );

	pc[1] = new Creature( session,
	                      Character::getRandomCharacter(),
	                      "Barlett",
	                      Constants::SEX_MALE,
	                      0 );
	pc[1]->setLevel( level );
	pc[1]->setExp();
	pc[1]->setHp();
	pc[1]->setMp();
	pc[1]->setHunger( 10 );
	pc[1]->setThirst( 9 );
	pc[1]->setStateMod( StateMod::drunk, true );
	pc[1]->setStateMod( StateMod::cursed, true );

	pc[2] = new Creature( session,
	                      Character::getRandomCharacter(),
	                      "Corinus",
	                      Constants::SEX_FEMALE,
	                      0 );
	pc[2]->setLevel( level );
	pc[2]->setExp();
	pc[2]->setHp();
	pc[2]->setMp();
	pc[2]->setHunger( 3 );
	pc[2]->setThirst( 2 );
	pc[2]->setStateMod( StateMod::ac_protected, true );
	pc[2]->setStateMod( StateMod::magic_protected, true );
	pc[2]->setStateMod( StateMod::cursed, true );
	//  for(int i = 0; i < Constants::STATE_MOD_COUNT; i++)
	//   if(i != Constants::dead) pc[2]->setStateMod(i, true);

	pc[3] = new Creature( session,
	                      Character::getRandomCharacter(),
	                      "Dialante",
	                      Constants::SEX_FEMALE,
	                      0 );
	pc[3]->setLevel( level );
	pc[3]->setExp();
	pc[3]->setHp();
	pc[3]->setMp();
	pc[3]->setHunger( 10 );
	pc[3]->setThirst( 10 );
	//pc[3]->setStateMod(StateMod::possessed, true);

	// compute starting skill levels
	for ( int i = 0; i < pcCount; i++ ) {
		for ( int skill = 0; skill < Skill::SKILL_COUNT; skill++ ) {
			int n = level * Util::dice( 10 );
			if ( n > 99 ) n = 99;
			int maxSkill = pc[i]->getCharacter()->getSkill( skill );
			if ( maxSkill >= 0 && n > maxSkill ) n = maxSkill;
			pc[i]->setSkill( skill, n );
		}
	}

	// add some items
	pc[0]->addToBackpack( session->newItem( RpgItem::getItemByName( "Bastard sword" ), level ) );
	pc[0]->addToBackpack( session->newItem( RpgItem::getItemByName( "Horned helmet" ), level ) );
	pc[0]->addToBackpack( session->newItem( RpgItem::getItemByName( "Dagger" ), level ) );
	pc[0]->addToBackpack( session->newItem( RpgItem::getItemByName( "Health potion" ), level ) );
	pc[0]->addToBackpack( session->newItem( RpgItem::getItemByName( "Health potion" ), level ) );
	pc[0]->addToBackpack( session->newItem( RpgItem::getItemByName( "Liquid armor" ), level ) );
	pc[0]->addToBackpack( session->newItem( RpgItem::getItemByName( "Potion of Speed" ), level ) );
	pc[0]->addToBackpack( session->newItem( RpgItem::getItemByName( "Potion of Coordination" ), level ) );
	pc[0]->addToBackpack( session->newItem( RpgItem::getItemByName( "Potion of Power" ), level ) );
	pc[0]->addToBackpack( session->newItem( RpgItem::getItemByName( "Potion of IQ" ), level ) );
	pc[0]->addToBackpack( session->newItem( RpgItem::getItemByName( "Potion of Leadership" ), level ) );
	pc[0]->addToBackpack( session->newItem( RpgItem::getItemByName( "Potion of Luck" ), level ) );
	pc[0]->addToBackpack( session->newItem( RpgItem::getItemByName( "Potion of Piety" ), level ) );
	pc[0]->addToBackpack( session->newItem( RpgItem::getItemByName( "Potion of Lore" ), level ) );
	pc[0]->addToBackpack( session->newItem( RpgItem::getItemByName( "Magic potion" ), level ) );
	pc[0]->addToBackpack( session->newItem( RpgItem::getItemByName( "Magic potion" ), level ) );

	pc[1]->addToBackpack( session->newItem( RpgItem::getItemByName( "Smallbow" ), level ) );
	pc[1]->addToBackpack( session->newItem( RpgItem::getItemByName( "Apple" ), level ) );
	pc[1]->addToBackpack( session->newItem( RpgItem::getItemByName( "Bread" ), level ) );
	pc[1]->addToBackpack( session->newItem( RpgItem::getItemByName( "Mushroom" ), level ) );
	pc[1]->addToBackpack( session->newItem( RpgItem::getItemByName( "Big egg" ), level ) );
	pc[1]->addToBackpack( session->newItem( RpgItem::getItemByName( "Mutton meat" ), level ) );
	pc[1]->addToBackpack( session->newItem( RpgItem::getItemByName( "Health potion" ), level ) );
	pc[1]->addToBackpack( session->newItem( RpgItem::getItemByName( "Magic potion" ), level ) );
	pc[1]->addToBackpack( session->newItem( RpgItem::getItemByName( "Liquid armor" ), level ) );

	pc[2]->addToBackpack( session->newItem( RpgItem::getItemByName( "Dagger" ), level ) );
	pc[2]->addToBackpack( session->newItem( RpgItem::getItemByName( "Smallbow" ), level ) );
	pc[2]->addToBackpack( session->newItem( RpgItem::getItemByName( "Long sword" ), level ) );
	pc[2]->addToBackpack( session->newItem( RpgItem::getItemByName( "Wine barrel" ), level ) );
	pc[2]->addToBackpack( session->newItem( RpgItem::getItemByName( "Mutton meat" ), level ) );
	pc[2]->addToBackpack( session->newItem( RpgItem::getItemByName( "Health potion" ), level ) );
	pc[2]->addToBackpack( session->newItem( RpgItem::getItemByName( "Health potion" ), level ) );
	pc[2]->addToBackpack( session->newItem( RpgItem::getItemByName( "Magic potion" ), level ) );
	pc[2]->addToBackpack( session->newItem( RpgItem::getItemByName( "Magic potion" ), level ) );
	pc[2]->addToBackpack( session->newItem( RpgItem::getItemByName( "Liquid armor" ), level ) );

	// add some scrolls
	for ( int i = 0; i < 10; i++ ) {
		Spell *spell = MagicSchool::getRandomSpell( 1 );
		if ( spell ) {
			RpgItem *rpgItem = RpgItem::getItemByName( "Scroll" );
			Item *scroll = session->newItem( rpgItem, level );
			scroll->setSpell( spell );
			pc[2]->addToBackpack( scroll );
		}
	}
	pc[2]->setMp( 50 );
	pc[3]->addToBackpack( session->newItem( RpgItem::getItemByName( "Dagger" ), level ) );
	pc[3]->addToBackpack( session->newItem( RpgItem::getItemByName( "Great sword" ), level ) );
	pc[3]->addToBackpack( session->newItem( RpgItem::getItemByName( "Battleaxe" ), level ) );
	pc[3]->addToBackpack( session->newItem( RpgItem::getItemByName( "Throwing axe" ), level ) );
	pc[3]->addToBackpack( session->newItem( RpgItem::getItemByName( "Health potion" ), level ) );
	pc[3]->addToBackpack( session->newItem( RpgItem::getItemByName( "Health potion" ), level ) );
	pc[3]->addToBackpack( session->newItem( RpgItem::getItemByName( "Health potion" ), level ) );
	pc[3]->addToBackpack( session->newItem( RpgItem::getItemByName( "Magic potion" ), level ) );
	pc[3]->addToBackpack( session->newItem( RpgItem::getItemByName( "Magic potion" ), level ) );
	pc[3]->addToBackpack( session->newItem( RpgItem::getItemByName( "Liquid armor" ), level ) );
	pc[3]->setMp( 500 );

	// equip weapons
	pc[0]->equipFromBackpack( 0 );
	pc[0]->equipFromBackpack( 1 );
	pc[1]->equipFromBackpack( 0 );
	pc[2]->equipFromBackpack( 0 );
	pc[2]->equipFromBackpack( 1 );
	pc[3]->equipFromBackpack( 0 );

	// add some spells
	pc[2]->addSpell( Spell::getSpellByName( "Flame of Azun" ) );
	pc[2]->addSpell( Spell::getSpellByName( "Ole Taffy's purty colors" ) );
	pc[2]->addSpell( Spell::getSpellByName( "Silent knives" ) );
	pc[2]->addSpell( Spell::getSpellByName( "Stinging light" ) );
	pc[2]->addSpell( Spell::getSpellByName( "Burning stare" ) );

	pc[3]->addSpell( Spell::getSpellByName( "Lesser healing touch" ) );
	pc[3]->addSpell( Spell::getSpellByName( "Body of stone" ) );
	pc[3]->addSpell( Spell::getSpellByName( "Bless group" ) );
	pc[3]->addSpell( Spell::getSpellByName( "Invisibility" ) );
	pc[3]->addSpell( Spell::getSpellByName( "Poison of ignorance" ) );
	pc[3]->addSpell( Spell::getSpellByName( "Transmute poison" ) );
	pc[3]->addSpell( Spell::getSpellByName( "Cursed ways" ) );
	pc[3]->addSpell( Spell::getSpellByName( "Remove curse" ) );
	pc[3]->addSpell( Spell::getSpellByName( "Enthrall fiend" ) );
	pc[3]->addSpell( Spell::getSpellByName( "Break from possession" ) );

	// assign random portraits
	for ( int i = 0; i < pcCount; i++ ) {
		pc[i]->setPortraitTextureIndex( Util::dice( session->getShapePalette()->getPortraitCount( pc[i]->getSex() ) ) );
	}

	*partySize = pcCount;
}

void Party::setParty( int count, Creature **creatures, int storylineIndex ) {
	loadedCount = count;
	for ( int i = 0; i < count; i++ ) loadedParty[i] = creatures[i];
	this->storylineIndex = storylineIndex;
}

/// Return the closest live player within the given radius or null if none can be found.

Creature *Party::getClosestPlayer( int x, int y, int w, int h, int radius ) {
	float minDist = 0;
	Creature *p = NULL;
	for ( int i = 0; i < getPartySize(); i++ ) {
		if ( !party[i]->getStateMod( StateMod::dead ) && !party[i]->getStateMod( StateMod::possessed ) ) {
			float dist = Constants::distance( x, y, w, h, party[i]->getX(), party[i]->getY(), party[i]->getShape()->getWidth(),
			             party[i]->getShape()->getDepth() );
			if ( dist <= static_cast<float>( radius ) && ( !p || dist < minDist ) ) {
				p = party[i];
				minDist = dist;
			}
		}
	}
	return p;
}

void Party::startEffect( int effect_type, int duration ) {
	for ( int i = 0; i < getPartySize(); i++ ) {
		if ( !party[i]->getStateMod( StateMod::dead ) ) {
			party[i]->startEffect( effect_type, duration );
		}
	}
}

void Party::setFormation( int formation ) {
	this->formation = formation;
	for ( int i = 0; i < getPartySize(); i++ ) {
		getParty( i )->setFormation( formation );
	}
	player_only = false;
	startRound = true;
	if ( !session->getGameAdapter()->isHeadless() )
		session->getGameAdapter()->setFormationUI( formation, !isPlayerOnly() );
}

void Party::togglePlayerOnly( bool keepTargets ) {
	player_only = ( player_only ? false : true );
	// in group mode everyone hunts the same creature
	if ( !player_only && !keepTargets ) {
		for ( int i = 0; i < getPartySize(); i++ ) {
			if ( party[i] != player )
				party[i]->setTargetCreature( player->getTargetCreature() );
		}
	}
	if ( player_only )
		session->getGameAdapter()->writeLogMessage( Constants::getMessage( Constants::SINGLE_MODE ), Constants::MSGTYPE_SYSTEM );
	else
		session->getGameAdapter()->writeLogMessage( Constants::getMessage( Constants::GROUP_MODE ), Constants::MSGTYPE_SYSTEM );
	if ( !session->getGameAdapter()->isHeadless() )
		session->getGameAdapter()->togglePlayerOnlyUI( !isPlayerOnly() );
}

void Party::savePlayerSettings() {
	savedPlayer = player;
	savedPlayerOnly = player_only;
}

void Party::restorePlayerSettings() {
	if ( savedPlayer->getStateMod( StateMod::dead ) ) setFirstLivePlayer();
	else if ( player != savedPlayer ) {
		for ( int i = 0; i < getPartySize(); i++ ) {
			if ( party[i] == savedPlayer ) {
				setPlayer( i );
				break;
			}
		}
	}
	if ( savedPlayerOnly != player_only ) togglePlayerOnly();
}

bool Party::isEquipped( Item *item ) {
	for ( int i = 0; i < getPartySize(); i++ ) {
		if ( getParty( i )->isEquipped( item ) ) return true;
	}
	return false;
}

void Party::regainMp() {
	for ( int i = 0; i < getPartySize(); i++ ) {
		if ( !getParty( i )->getStateMod( StateMod::dead ) &&
		        getParty( i )->getStartingMp() > 0 &&
		        getParty( i )->getMp() < getParty( i )->getMaxMp() ) {
			getParty( i )->setMp( getParty( i )->getMp() + 1 );
		}
	}
}

void Party::applyRecurringSpecialSkills() {
	for ( int i = 0; i < getPartySize(); i++ ) {
		if ( !getParty( i )->getStateMod( StateMod::dead ) ) {
			getParty( i )->applyRecurringSpecialSkills();
		}
	}
}

void Party::recomputeMaxSkills() {
	maxSkills.clear();
	for ( int skill = 0; skill < Skill::SKILL_COUNT; skill++ ) {
		int maxValue = 0;
		Creature *maxPC = NULL;
		for ( int i = 0; i < getPartySize(); i++ ) {
			int value = getParty( i )->getSkill( skill );
			if ( value > 0 && ( !maxPC || maxValue < value ) ) {
				maxPC = getParty( i );
				maxValue = value;
			}
		}
		maxSkills.push_back(maxPC);
	}
}

int Party::getAverageLevel() {
	int sum = 0;
	for ( int i = 0; i < getPartySize(); i++ ) {
		sum += getParty( i )->getLevel();
	}

	if ( sum )
		return static_cast<int>( sum / static_cast<float>( getPartySize() ) );
	else
		return 0;
}

void Party::hire( Creature *creature ) {
	// Remove from the session list
	assert( session->removeCreatureRef( creature, partySize ) );

	// add to party
	party[ partySize++ ] = creature;
	partySet.insert( creature );

	session->getSquirrel()->partyChanged();
}

void Party::dismiss( int index ) {
	session->addCreatureRef( party[ index ], index );
	for ( int i = index; i < partySize - 1; i++ ) {
		party[ i ] = party[ i + 1 ];
	}
	party[ --partySize ] = NULL;
	partySet.clear();
	for ( int i = 0; i < partySize; i++ ) {
		partySet.insert( party[i] );
	}
	session->getSquirrel()->partyChanged();
}

void Party::rollPerception() {
	for ( int i = 0; i < getPartySize(); i++ ) {
		getParty( i )->rollPerception();
	}
}
