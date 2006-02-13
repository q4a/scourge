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
#include "session.h"
#include "spellcaster.h"

/**
  *@author Gabor Torok
  */

class Session;
class Creature;
class Item;
class Projectile;
class Spell;

/**
   This class represents a single battle turn.
 */
class Battle {
 private:
  Session *session;
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
  int weaponWait;
  float range;

  int ap, startingAp;
  bool paused;
  int steps;
  bool needsReset;
  int nextTurn;

  // sounds
  static int handheldSwishSoundStart, handheldSwishSoundCount;
  static int bowSwishSoundStart, bowSwishSoundCount;
  static int potionSoundStart, potionSoundCount;
  static char *sound[];

 public:

   static bool debugBattle;

   inline int getAP() { return ap; }
   inline int decrAP() { return --ap; }
   inline int getStartingAP() { return startingAp; }
  
   void endTurn();

  /**
	 This method sets up and creates battle turns (Battle objects) in order of initiative.
   */
  static void setupBattles(Session *session, Battle *battle[], int count, std::vector<Battle *> *turns);

  /**
	 Call these when a projectile weapon finally hits.
	 It sets up a turn and plays it.
  */
  static void projectileHitTurn(Session *session, Projectile *proj, Creature *target);
  static void projectileHitTurn(Session *session, Projectile *proj, int x, int y);

  /**
	 A no-op turn of battle.
  */
  Battle();

  /**
	 A Battle is a round of battle between 'creature' and 'creature->getTargetCreature()'
   */
  Battle(Session *session, Creature *creature);
  ~Battle();

  void reset();
  Creature *getAvailableTarget();
  Creature *getAvailablePartyTarget();

  inline bool isEmpty() { return empty; }
  bool fightTurn();

  void dealDamage( float damage, int effect=Constants::EFFECT_GLOW, bool magical=false, GLuint delay=0 );

  inline Creature *getCreature() { return creature; }
  inline Session *getSession() { return session; }

  void invalidate();

  static inline int getSoundCount() { return handheldSwishSoundCount + bowSwishSoundCount + potionSoundCount; }
  static inline char *getSound(int index) { return sound[index]; }

  void castSpell( bool alwaysSucceeds = false );

  void useSkill();

  static int getWeaponSpeed( Item *item );

  int calculateRange( Item *item=NULL );

  bool describeAttack( Creature *target, char *buff, bool includeActions );

 protected:
  void launchProjectile();
  //void initTurn();
  void hitWithItem();
  float applyMagicItemSpellDamage();
  void applyMagicItemDamage( float *damage );
  void applyHighAttackRoll( float *damage, float attack, float min, float max );
  bool handleLowAttackRoll( float attack, float min, float max );
  void prepareToHitMessage();
  void initItem(Item *item);
  
  void executeEatDrinkAction();
  // return true if game paused
  bool pauseBeforePlayerTurn();
  void initTurnStep();
  void executeAction();
  void stepCloserToTarget();
  bool selectNewTarget();
  bool moveCreature();

  static char *getRandomSound(int start, int count);
};

#endif
