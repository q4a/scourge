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
#include "rpg/spell.h"
#include "effect.h"
#include "projectile.h"
#include "spellcaster.h"

using namespace std;

/**
  *@author Gabor Torok
  */

class Scourge;
class Creature;
class Item;
class Projectile;
class Spell;

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
  int speed;
  float dist;
  bool empty;
  bool projectileHit;
  Spell *spell;

 public:
  
  /**
	 This method sets up and creates battle turns (Battle objects) in order of initiative.
   */
  static void setupBattles(Scourge *scourge, Battle *battle[], int count, vector<Battle *> *turns);

  /**
	 Call these when a projectile weapon finally hits.
	 It sets up a turn and plays it.
  */
  static void projectileHitTurn(Scourge *scourge, Projectile *proj, Creature *target);
  static void projectileHitTurn(Scourge *scourge, Projectile *proj, int x, int y);

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

  void dealDamage(int damage, int effect=Constants::EFFECT_GLOW);

  inline Creature *getCreature() { return creature; }
  inline Scourge *getScourge() { return scourge; }

 protected:
  void launchProjectile();
  void initTurn();
  void hitWithItem();
  void initItem(Item *item);
  void castSpell();
  void executeEatDrinkAction();
};

#endif
