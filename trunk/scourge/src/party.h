/***************************************************************************
                          party.h  -  description
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

#include <iostream>
#include <string>
#include "constants.h"
#include "map.h"
#include "calendar.h"
#include "rpg/character.h"
#include "rpg/monster.h"
#include "rpg/rpgitem.h"
#include "events/thirsthungerevent.h"

using namespace std;

class Session;

class Party {
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

 public:

  Party(Session *session);
  virtual ~Party();

  void reset();
  void resetMultiplayer(Creature *c);

  void deleteParty();

  inline Calendar *getCalendar() { return calendar; } 

  inline Creature *getPlayer() { return player; }

  void setPlayer(int n);
  inline void setPlayer(Creature *c) { player = c; }

  void setPartyMotion(int motion);
  
  void setFormation(int formation);

  inline int getFormation() { return formation; }

  inline Creature *getParty(int i) { return party[i]; } 

  // move the party
  void movePlayers();

  // returns false if the switch could not be made,
  // because the entire party is dead (the mission failed)
  bool switchToNextLivePartyMember();

  void togglePlayerOnly();

  void forceStopRound();
  void toggleRound(bool test);
  void toggleRound();
  inline bool isRealTimeMode() { return startRound; }

  void startPartyOnMission();

  void followTargets();

  void setTargetCreature(Creature *creature);
  inline bool isPartyDead() { return partyDead; }
  inline bool isPlayerOnly() { return player_only; }
  inline int getPartySize() { return partySize; }
  
  int getTotalLevel();

  /** 
	  Return the closest live player within the given radius or null if none can be found.
  */
  Creature *getClosestPlayer(int x, int y, int w, int h, int radius);

  void startEffect(int effect_type, int duration=Constants::DAMAGE_DURATION);

  static void createHardCodedParty(Session *session, Creature ***party, int *partySize);

protected:
  void resetPartyUI();
};

#endif
