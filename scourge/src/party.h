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
#include "scourge.h"
#include "map.h"
#include "calendar.h"
#include "rpg/character.h"
#include "rpg/monster.h"
#include "rpg/rpgitem.h"
#include "events/thirsthungerevent.h"

using namespace std;

class Party {
 private:
  
  // hard-coded for now
  static const int PARTY_SIZE = 4;
  
  Scourge *scourge;
  Creature *player;
  Creature *party[4];
  bool partyDead;
  bool player_only;
  int formation;

 public:
  Party(Scourge *scourge);
  virtual ~Party();

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

  void startPartyOnMission();

  void followTargets();

  void setTargetCreature(Creature *creature);
  inline bool isPartyDead() { return partyDead; }
  inline bool isPlayerOnly() { return player_only; }
  inline int getPartySize() { return PARTY_SIZE; }
  
  int getTotalLevel();

 protected:
  Creature **createHardCodedParty();

};

#endif
