/***************************************************************************
                            party.h  -  Party class
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

#ifndef PARTY_H
#define PARTY_H
#pragma once

#include <iostream>
#include <string>
#include "calendar.h"
#include "events/thirsthungerevent.h"

class Session;
class Creature;
class Item;

/// Small utility class.

class CreatureGroupInfo {
public:
	CreatureGroupInfo() {
	}

	virtual ~CreatureGroupInfo() {
	}

	virtual Creature *getHighestSkillPC( int skill ) = 0;
};

/// A party of characters.

class Party : public CreatureGroupInfo {
private:

	Session *session;
	Creature *player;
	Creature *party[MAX_PARTY_SIZE];
	bool partyDead;
	bool player_only;
	int formation;
	Calendar * calendar;
	bool startRound;
	int partySize;
	Creature *savedPlayer;
	bool savedPlayerOnly;
	int storylineIndex;

	static Creature *lastPlayer;

	Creature *loadedParty[MAX_PARTY_SIZE];
	int loadedCount;
	std::vector<Creature*> maxSkills;
	std::set<Creature*> partySet;

public:

	Party( Session *session );
	virtual ~Party();

	inline Creature *getHighestSkillPC( int skill ) {
		return ( skill >= 0 && skill < (int)maxSkills.size() ? maxSkills[ skill ] : NULL );
	}
	void recomputeMaxSkills();

	inline bool isPartyMember( Creature *c ) {
		return( partySet.find( c ) != partySet.end() );
	}

	void regainMp();

	void applyRecurringSpecialSkills();

	inline int getStorylineIndex() {
		return storylineIndex;
	}

	void reset();
	void resetMultiplayer( Creature *c );

	void deleteParty();

	inline Calendar *getCalendar() {
		return calendar;
	}

	inline Creature *getPlayer() {
		return player;
	}

	void setPlayer( int n, bool updateui = true );

	void setPartyMotion( int motion );

	void setFormation( int formation );

	inline int getFormation() {
		return formation;
	}

	inline Creature *getParty( int i ) {
		return party[i];
	}

	// move the party
	bool setSelXY( Uint16 mapx, Uint16 mapy, bool cancelIfNotPossible = false );
	void movePlayers();

	bool switchToNextLivePartyMember();

	bool nextPartyMember();
	bool previousPartyMember();
	int getPlayerIndex();

	void togglePlayerOnly( bool keepTargets = false );

	void forceStopRound();
	void toggleRound( bool test );
	void toggleRound();
	inline bool isRealTimeMode() {
		return startRound;
	}

	void startPartyOnMission();

	void setFirstLivePlayer();
	int getFirstLivePlayer();

	void setTargetCreature( Creature *creature );
	inline bool isPartyDead() {
		return partyDead;
	}
	inline bool isPlayerOnly() {
		return player_only;
	}
	inline int getPartySize() {
		return partySize;
	}

	int getTotalLevel();

	Creature *getClosestPlayer( int x, int y, int w, int h, int radius );

	void startEffect( int effect_type, int duration = Constants::DAMAGE_DURATION );

	static void createHardCodedParty( Session *session, Creature **party, int *partySize );

	void savePlayerSettings();
	void restorePlayerSettings();

	void setParty( int count, Creature **creatures, int storylineIndex );

	bool isEquipped( Item *item );

	int getAverageLevel();

	void hire( Creature *creature );
	void dismiss( int index );

	void rollPerception();
protected:
	void resetPartyUI();
	bool isPartyInRange();
};

#endif
