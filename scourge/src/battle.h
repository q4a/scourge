/***************************************************************************
                          battle.h  -  description
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

#ifndef BATTLE_H
#define BATTLE_H

#include <iostream>
#include <string>
#include <vector>
#include "constants.h"
#include "scourge.h"
#include "map.h"
#include "creature.h"
#include "item.h"
#include "rpg/character.h"
#include "rpg/monster.h"
#include "effect.h"
#include "projectile.h"

using namespace std;

/**
  *@author Gabor Torok
  */

class Scourge;
class Creature;
class Item;
class Projectile;

/**
   This class represents a single battle turn.
 */
class Battle {
 private:
  Scourge *scourge;
  Creature *creature;
  Item* item;
  char message[200];
  int creatureInitiative;
  bool initiativeCheck;
  int itemSpeed;
  float dist;
  bool empty;
  bool projectileHit;

 public:
  
  /**
	 This method sets up and creates battle turns (Battle objects) in order of initiative.
   */
  static void setupBattles(Scourge *scourge, Battle *battle[], int count, vector<Battle *> *turns);

  /**
	 Call this when a projectile weapon finally hits.
	 It sets up a turn and plays it.
  */
  static void projectileHitTurn(Scourge *scourge, Projectile *proj, Creature *target);

  /**
	 A no-op turn of battle.
  */
  Battle();

  /**
	 A Battle is a round of battle between 'creature' and 'creature->getTargetCreature()'
   */
  Battle(Scourge *scourge, Creature *creature);
  ~Battle();

  inline bool isEmpty() { return empty; }
  void fightTurn();

 protected:
  void hitWithItem();
  void selectBestItem();
  void initItem(Item *item);
};

#endif
