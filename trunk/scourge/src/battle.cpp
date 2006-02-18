/***************************************************************************
                          battle.cpp  -  description
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

#include "battle.h"
#include "render/renderlib.h"
#include "rpg/rpglib.h"
#include "item.h"
#include "projectile.h"
#include "creature.h"
#include "shapepalette.h"

using namespace std;

#define WEAPON_WAIT_MUL 9
#define MIN_FUMBLE_RANGE 4.0f

bool Battle::debugBattle = false;

char *Battle::sound[] = {
  "sound/weapon-swish/handheld/sw1.wav",
  "sound/weapon-swish/handheld/sw2.wav",
  "sound/weapon-swish/handheld/sw3.wav",
  
  "sound/weapon-swish/bows/swb2.wav",
  "sound/weapon-swish/bows/swb3.wav",

  "sound/potion/pd1.wav"
};

int Battle::handheldSwishSoundStart = 0;
int Battle::handheldSwishSoundCount = 3;
int Battle::bowSwishSoundStart = 3;
int Battle::bowSwishSoundCount = 2;
int Battle::potionSoundStart = 5;
int Battle::potionSoundCount = 1;

enum {
  NO_ACTION = 0,
  LOITER,
  ATTACK,
  MOVE,
  NO_TARGET,
  WAIT,
};

char *actionName[] = { "NO ACTION", "LOITER", "ATTACK", "MOVE", "NO_TARGET", "WAIT"};

Battle::Battle() {
  this->creature = NULL;
}

Battle::Battle(Session *session, Creature *creature) {
  this->session = session;
  this->creature = creature;
  this->item = NULL;
  this->creatureInitiative = 0;
  this->initiativeCheck = false;
  this->speed = 0;
  this->dist = creature->getDistanceToTarget();
  this->spell = NULL;
  this->empty = false;

  this->needsReset = true;
  this->nextTurn = 0;
  this->weaponWait = 0;
  this->startingAp = this->ap = 0;
  if(debugBattle) cerr << "*** constructor, creature=" << creature->getName() << endl;
}

Battle::~Battle() {
}

void Battle::reset() {
  if(debugBattle) cerr << "*** reset: creature=" << creature->getName() << endl;
  this->steps = 0;
  this->startingAp = this->ap = toint( creature->getMaxAP() );
  this->projectileHit = false;
  this->paused = false;
  this->weaponWait = 0;
  this->range = 0.0f;
  creature->getShape()->setCurrentAnimation((int)MD2_STAND);
  ((MD2Shape*)(creature->getShape()))->setAttackEffect(false);
}

void Battle::setupBattles(Session *session, Battle *battle[], int count, vector<Battle *> *turns) {
  // for now put all battles into the vector
  // need to order by initiative
  for(int i = 0; i < count; i++) {
    // reset for the first time
    // (to avoid whack skill numbers for un-initialized creatures.)
    if(battle[i]->needsReset) {
	  if( debugBattle ) cerr << "*** Reset 1" << endl;
      battle[i]->reset();
      battle[i]->needsReset = false;
    }
    bool handled = false;
    for( vector<Battle*>::iterator e = turns->begin(); e != turns->end(); ++e ) {
      Battle *b = *e;
      if( b->getCreature()->getInitiative() < battle[i]->getCreature()->getInitiative() ) {
        turns->insert( e, battle[i] );
        handled = true;
        break;
      }
    }
    if( !handled ) turns->push_back(battle[i]);
  }
  
  /*
  cerr << "Battle order:" << endl;
  for( vector<Battle*>::iterator e = turns->begin(); e != turns->end(); ++e ) {
    Battle *b = *e;
    cerr << "\t" << b->getCreature()->getName() << 
      " INIT:" << b->getCreature()->getInitiative() << 
      " speed:" << b->getCreature()->getSkill( Constants::SPEED ) <<
      endl;
  }
  cerr << "-----------------------------------" << endl;
  */
}                 

bool Battle::fightTurn() {

  if(debugBattle) cerr << "TURN: creature=" << creature->getName() << 
    " ap=" << ap << 
    " wait=" << weaponWait << 
    " nextTurn=" << nextTurn << endl;

  // are we alive?
  if(!creature || creature->getStateMod(Constants::dead) ||
     (creature->getAction() == Constants::ACTION_NO_ACTION &&
      !creature->isMonster() && !getAvailablePartyTarget()) ) {
	if( debugBattle ) cerr << "*** Reset 2" << endl;
    reset();
    return true;
  }

  // done with this creature's turn
  if(ap <= 0) {
    if(weaponWait > 0) {
      nextTurn = weaponWait;
      if(debugBattle) cerr << "Carries over into next turn." << endl;
    }
    // in TB battle, wait for the animations to end before ending turn
    int a;
    if( session->getPreferences()->isBattleTurnBased()) {
      a =((MD2Shape*)(creature->getShape()))->getCurrentAnimation();
      if( !( a == MD2_STAND || a == MD2_RUN )) {
        return false;
      }
      if( creature->getTargetCreature() && 
          !creature->getTargetCreature()->getStateMod( Constants::dead ) ) {
        a =((MD2Shape*)(creature->getTargetCreature()->getShape()))->getCurrentAnimation();
        if( !( a == MD2_STAND || a == MD2_RUN )) {
          return false;
        }
      }
    }
    if( debugBattle ) cerr << "*** Reset 3" << endl;
    reset();
    return true;
  }

  // waiting to cast a spell?
  if( creature->getAction() == Constants::ACTION_CAST_SPELL || 
      creature->getAction() == Constants::ACTION_SPECIAL ) {
    creature->startEffect(Constants::EFFECT_CAST_SPELL, 
                          Constants::DAMAGE_DURATION * 4);
  }

  if(pauseBeforePlayerTurn()) return false;

  initTurnStep();

  if(creature->getAction() == Constants::ACTION_EAT_DRINK) {
    executeEatDrinkAction();
  }

  if(creature->hasTarget() && 
     creature->isTargetValid()) {    
    if(dist < range) {
      executeAction();
    } else {
      stepCloserToTarget();
    }    
  } else {
    moveCreature();
  }
  // not done yet with creature's turn
  return false;
}

void Battle::endTurn() {
  ap = 0;
  session->getParty()->toggleRound(false);
  //paused = false;
}

bool Battle::pauseBeforePlayerTurn() {
  // go to single-player mode
  if (session->getPreferences()->isBattleTurnBased() &&
      !session->getParty()->isPlayerOnly()) {
    session->getParty()->togglePlayerOnly(true);
  }

  // pause if this is a player's first step
  if (!steps && 
      !paused &&
      session->getPreferences()->isBattleTurnBased()) {
    if(!creature->isMonster()) {
      if(debugBattle) cerr << "Pausing for round start. Turn: " << creature->getName() << endl;

      // center on player
      for (int i = 0; i < session->getParty()->getPartySize(); i++) {
        if (session->getParty()->getParty(i) == creature &&
            !creature->getStateMod(Constants::possessed)) {
          session->getParty()->setPlayer(i);
          break;
        }
      }
      // FIXME: only center if not on-screen
      session->getMap()->refresh();
      session->getMap()->center( toint(creature->getX()), toint(creature->getY()), true);

      // pause the game
      if( getAvailablePartyTarget() ) {
        session->getParty()->toggleRound(true);
        paused = true;
        return true;
      }
    } else {
      // FIXME: only center if not on-screen
      session->getMap()->refresh();
      session->getMap()->center( toint(creature->getX()), toint(creature->getY()), true);
    }
  }
  steps++;
  paused = false;
  return false;
}

int Battle::calculateRange( Item *item ) {
  int range;
  if( creature->getActionSkill() ) {
    //range = MIN_DISTANCE;
    range = creature->getActionSkill()->getDistance();
  } else if(creature->getActionSpell()) {
    //range = MIN_DISTANCE;
    range = creature->getActionSpell()->getDistance();
  } else {
    range = (int)MIN_DISTANCE;
    if(item) range = item->getDistance();
  }
  return range;
}

void Battle::initTurnStep() {
  dist = creature->getDistanceToTarget();

  // select the best weapon only once
  if(weaponWait <= 0) {
    if(debugBattle) cerr << "*** initTurnStep, creature=" << creature->getName() << " wait=" << weaponWait << " nextTurn=" << nextTurn << endl;

    if( creature->getActionSkill() ) {
      range = calculateRange();
      if(nextTurn > 0) weaponWait = nextTurn;
      else weaponWait = creature->getActionSkill()->getSpeed() * WEAPON_WAIT_MUL;
      nextTurn = 0;
      if(debugBattle) cerr << "\tUsing capability: " << creature->getActionSkill()->getName() << endl;
    } else if(creature->getActionSpell()) {
      range = calculateRange();
      if(nextTurn > 0) weaponWait = nextTurn;
      else weaponWait = creature->getActionSpell()->getSpeed() * WEAPON_WAIT_MUL;
      nextTurn = 0;
      if(debugBattle) cerr << "\tUsing spell: " << creature->getActionSpell()->getName() << endl;
    } else {
      item = creature->getBestWeapon(dist);
      range = calculateRange( item );
      if(item) {
        if(debugBattle) cerr << "\tUsing item: " << item->getRpgItem()->getName() << " ap=" << ap << endl;
      } else {
        if(debugBattle) cerr << "\tUsing bare hands." << endl;
      }
      // How many steps to wait before being able to use the weapon.
      weaponWait = getWeaponSpeed( item );
      // Make turn-based mode a little snappier
      if( session->getPreferences()->isBattleTurnBased() ) {
        weaponWait /= 2;
      }
    }
    if(nextTurn > 0) weaponWait = nextTurn;
    nextTurn = 0;
    if(debugBattle) cerr << "\tname=" << creature->getName() << " Distance=" << dist << " range=" << range << " wait=" << weaponWait << endl;
  }
}

void Battle::executeAction() {
  // try to heal someone
  if( !creature->getActionSpell() ) {
    if( creature->castHealingSpell() ) {
      if(debugBattle) cerr << "Stopping executeAction: Creature: " << creature->getName() << 
        " will heal " << creature->getTargetCreature()->getName() << endl;
      weaponWait = nextTurn = 0;
      return;
    }
  }

  ap--;
  if(weaponWait > 0) {
    weaponWait--;
    if(weaponWait > 0) return;
  }

  // don't carry to next turn
  nextTurn = 0;

  // attack
  if(debugBattle) cerr << "\t\t *** Attacking." << endl;
  if( creature->getActionSkill() ) {
    useSkill();
  } else if(creature->getActionSpell()) {
    // casting a spell for the first time
    castSpell();
  } else if(item && item->getRpgItem()->isRangedWeapon()) {
      launchProjectile();
  } else {
    hitWithItem();
  }
  creature->getShape()->setCurrentAnimation((int)MD2_ATTACK);	  
  ((MD2Shape*)(creature->getShape()))->setAngle(creature->getTargetAngle());
}

void Battle::stepCloserToTarget() {
  // try to heal someone
  if( !creature->getActionSpell() ) {
    if( creature->castHealingSpell() ) {
      if(debugBattle) cerr << "Stopping stepCloserToTarget: Creature: " << creature->getName() << 
        " will heal " << creature->getTargetCreature()->getName() << endl;
      weaponWait = nextTurn = 0;
      return;
    }
  }

  // out of range: take 1 step closer

  // re-select the best weapon
  weaponWait = nextTurn = 0;

  // Set the movement mode; otherwise character won't move
  creature->getShape()->setCurrentAnimation((int)MD2_RUN);

  if(debugBattle) cerr << "\t\tTaking a step." << endl;
  if(creature->getTargetCreature()) {
    if(debugBattle) cerr << "\t\t\tto target creature: " << creature->getTargetCreature()->getName() << endl;

    // has the target creature moved?
    int tx = toint(creature->getTargetCreature()->getX());
    int tw = creature->getTargetCreature()->getShape()->getWidth();
    int ty = toint(creature->getTargetCreature()->getY());
    int th = creature->getTargetCreature()->getShape()->getDepth();
    if( !( creature->getSelX() >= tx && creature->getSelX() < tx + tw &&
           creature->getSelY() <= ty && creature->getSelY() > ty - th ) ) {
/*
    int tx = toint(creature->getTargetCreature()->getX() + 
                   creature->getTargetCreature()->getShape()->getWidth() / 2);
    int ty = toint(creature->getTargetCreature()->getY() - 
                   creature->getTargetCreature()->getShape()->getDepth() / 2);
    if(!(creature->getSelX() == tx && creature->getSelY() == ty)) {
*/    

      // Try to move to the target creature.
      // For monsters, if this is not possible, select a new target (once in a while).
      if( !creature->setSelXY( tx, ty, false ) &&
          creature->isMonster() &&
          1 == (int)( 4.0f * rand() / RAND_MAX ) ) {
        creature->cancelTarget();
        creature->decideMonsterAction();
        ap--;
        if(debugBattle) 
          cerr << "*** " << creature->getName() << 
          " selecting new target" << endl;
        return;
      }
    }
  } else if(!(creature->getSelX() == creature->getTargetX() &&
              creature->getSelY() == creature->getTargetY())) {
    if(debugBattle) cerr << "\t\t\tto target location: " << creature->getTargetX() <<
      creature->getTargetY() << endl;
    creature->setSelXY(creature->getTargetX(), creature->getTargetY(), false );
  }

  // wait for animation to end
  int a =((MD2Shape*)(creature->getShape()))->getCurrentAnimation();
  if( !( a == MD2_STAND || a == MD2_RUN )) {
    if(debugBattle) cerr << "\t\t\tWaiting for animation to end." << endl;
    return;
  }

  GLfloat oldX = creature->getX();
  GLfloat oldY = creature->getY();
  bool moved = creature->moveToLocator(session->getMap());
  if( !( toint(oldX) == toint(creature->getX()) &&
         toint(oldY) == toint(creature->getY()) ) ) {
    ap--;
  }
  //if( !moved ) moveCreature();
  if( !moved ) {
    if( debugBattle ) {
      cerr << "*** Warning: not moving closer to target and not in range. " <<
        " x,y=" << creature->getX() << "," << creature->getY() <<
        " selX,selY=" << creature->getSelX() << "," << creature->getSelY() <<
        " dist=" << dist << " range=" << range << 
        " item=" << ( item ? item->getRpgItem()->getName() : "none" ) <<
        " creature=" << creature->getName() << 
        " target=" << ( creature->getTargetCreature() ? 
                        creature->getTargetCreature()->getName() : 
                        ( creature->getTargetItem() ? creature->getTargetItem()->getRpgItem()->getName() : 
                          ( creature->getTargetX() > -1 ? "location" : "no-target" ))) << endl;
      if( creature->getTargetCreature() ) {
        cerr << "Target creature=" << creature->getTargetCreature()->getX() <<
          "," << creature->getTargetCreature()->getY() << endl;
      }
    }
      
    // guess a new path
    creature->setSelXY( creature->getSelX(), creature->getSelY(), false );

    if( creature->isMonster() ) {      
      ap--;  
    } else {
      if( session->getPreferences()->isBattleTurnBased() ) {
        if( getAvailablePartyTarget() ) session->getParty()->toggleRound(true);
      } else {
        ap--;
      }
    }
  }
}

bool Battle::moveCreature() {

  // Set the movement mode; otherwise character won't move
  if(creature->anyMovesLeft()) {
    creature->getShape()->setCurrentAnimation((int)MD2_RUN);
  } else {
    creature->getShape()->setCurrentAnimation((int)MD2_STAND);
  }

  // take 1 step closer
  if(debugBattle) cerr << "\t\tTaking a non-battle step." << endl;

  GLfloat oldX = creature->getX();
  GLfloat oldY = creature->getY();
  if(creature->isMonster()) {
    session->getGameAdapter()->moveMonster(creature);
    if( !( toint(oldX) == toint(creature->getX()) &&
           toint(oldY) == toint(creature->getY()) ) ) {
      ap--;
    } 
    
    if( oldX == creature->getX() &&
        oldY == creature->getY() ) {
      if( debugBattle ) cerr << "*** WARNING: monster not moving." << endl;
      ap--;
    }
    return false;
  } else {

    // someone killed our target, try to pick another one
    if(creature->hasTarget() && !creature->isTargetValid()) {
      //cerr << "*** Character has invalid target, selecting new one." << endl;
      if( selectNewTarget() ) return true;
    }

    // wait for animation to end
    int a =((MD2Shape*)(creature->getShape()))->getCurrentAnimation();
    if( !( a == MD2_STAND || a == MD2_RUN )) return false;

    if(creature->getSelX() != -1) {
      bool moved = creature->moveToLocator(session->getMap());
      if( !( toint(oldX) == toint(creature->getX()) &&
             toint(oldY) == toint(creature->getY()) ) ) {
        ap--;
      }
      
      if( !moved ) {
        if( debugBattle ) {
          cerr << "*** WARNING: character not moving." << 
            " x,y=" << creature->getX() << "," << creature->getY() <<
            " selX,selY=" << creature->getSelX() << "," << creature->getSelY() <<
            " dist=" << dist << " range=" << range << 
            " item=" << ( item ? item->getRpgItem()->getName() : "none" ) <<
            " creature=" << creature->getName() << 
            " target=" << ( creature->getTargetCreature() ? 
                            creature->getTargetCreature()->getName() : 
                            ( creature->getTargetItem() ? creature->getTargetItem()->getRpgItem()->getName() : 
                              ( creature->getTargetX() > -1 ? "location" : "no-target" ))) << endl;
          if( creature->getTargetCreature() ) {
            cerr << "Target creature=" << creature->getTargetCreature()->getX() <<
              "," << creature->getTargetCreature()->getY() << endl;
          }
        }

        // guess a new path
        creature->setSelXY( creature->getSelX(), creature->getSelY(), false );
        if( session->getPreferences()->isBattleTurnBased() ) {          
          if( getAvailablePartyTarget() ) session->getParty()->toggleRound(true);
        } else {
          // is the below line needed?
          ap--;
        }
      }
    } else {
      //cerr << "*** Character has no location, selecting new target." << endl;
      return selectNewTarget();
    }
  }
  return false;
}

Creature *Battle::getAvailablePartyTarget() {
  for( int i = 0; i < session->getParty()->getPartySize(); i++) {
    bool visible = ( session->getMap()->isLocationVisible(toint(session->getParty()->getParty(i)->getX()), 
                                                          toint(session->getParty()->getParty(i)->getY())) &&
                     session->getMap()->isLocationInLight(toint(session->getParty()->getParty(i)->getX()), 
                                                          toint(session->getParty()->getParty(i)->getY()),
                                                          session->getParty()->getParty(i)->getShape()));
    if( visible && !session->getParty()->getParty(i)->getStateMod(Constants::dead) ) {
      Creature *c = session->getParty()->getParty(i)->getBattle()->getAvailableTarget();
      if( c ) return c;
    }
  }
  return NULL;
}

Creature *Battle::getAvailableTarget() {
  if( creature->isMonster() ) {
    cerr << "*** Error: Battle::getAvailableTarget() should only be called for characters!" << endl;
    return NULL;
  }
  Creature *target;
  if(creature->getStateMod(Constants::possessed)) {
    target = session->getParty()->getClosestPlayer(toint(creature->getX()), 
                                                   toint(creature->getY()), 
                                                   creature->getShape()->getWidth(),
                                                   creature->getShape()->getDepth(),
                                                   20);
  } else {
    target = session->getClosestVisibleMonster(toint(creature->getX()), 
                                               toint(creature->getY()), 
                                               creature->getShape()->getWidth(),
                                               creature->getShape()->getDepth(),
                                               20);
  }
  return target;
}

bool Battle::selectNewTarget() {
  // select a new target
  if (creature->isMonster()) {    
    cerr << "*** Error Battle::selectNewTarget should not be called for monsters." << endl;
    return false;
  } else {
    // select a new target
    Creature *target = getAvailableTarget();
    if (target) {
      if(debugBattle) cerr << "\tSelected new target: " << target->getName() << endl;
      creature->setTargetCreature(target, true);
    } else {
      creature->setTargetCreature(NULL);
      if(debugBattle) cerr << "\t\tCan't find new target." << endl;
    }

    // let the player override in TB mode
    if( target && session->getPreferences()->isBattleTurnBased() ) {
      if( getAvailablePartyTarget() ) session->getParty()->toggleRound(true);
    }

    return ( target ? true : false );
  }
}

void Battle::useSkill() {
  sprintf(message, "%s uses capability %s!", 
          creature->getName(), 
          creature->getActionSkill()->getName());
  session->getMap()->addDescription(message, 1, 0.15f, 1);
  ((MD2Shape*)(creature->getShape()))->setAttackEffect(true);

  char *err = 
    creature->useSpecialSkill( creature->getActionSkill(), 
                               true );
  if( err ) {
    session->getMap()->addDescription( err, 1, 0, 0 );
    //showMessageDialog( err );
  }

  // cancel action
  creature->cancelTarget();
  // also cancel path
  if( !( creature->isMonster() ) ) creature->setSelXY( -1, -1 );
}

void Battle::castSpell( bool alwaysSucceeds ) {
  // use up some MP (not when using a scroll)
  if(!creature->getActionItem()) {
    int n = creature->getMp() - creature->getActionSpell()->getMp();
    if(n < 0) n = 0;
    creature->setMp( n );
  } else {
    // try to destroy the scroll
    int itemIndex = creature->findInInventory(creature->getActionItem());
    if(itemIndex > -1) {
      creature->removeInventory(itemIndex);
      sprintf(message, "%s crumbles into dust.", creature->getActionItem()->getItemName());
      session->getMap()->addDescription(message);
      if(!session->getGameAdapter()->isHeadless()) 
        session->getGameAdapter()->refreshInventoryUI();
    } else {
      // scroll was removed from inventory before casting
      sprintf(message, "Couldn't find scroll, cancelled spell.");
      session->getMap()->addDescription(message);
      creature->cancelTarget();
      // also cancel path
      if( !( creature->isMonster() ) ) creature->setSelXY( -1, -1 );
      return;
    }
  }

  sprintf(message, "%s casts %s!", 
          creature->getName(), 
          creature->getActionSpell()->getName());
  session->getMap()->addDescription(message, 1, 0.15f, 1);
  ((MD2Shape*)(creature->getShape()))->setAttackEffect(true);

  // spell succeeds?
  // FIXME: use stats like IQ here to modify spell success rate...
  SpellCaster *sc = new SpellCaster(this, creature->getActionSpell(), false);

  /* 
    apply state_mods:
    blessed, 
	  empowered, 
  	enraged, 
  	ac_protected, 
  	magic_protected, 
    
  	drunk, 
    
  	poisoned, 
  	cursed, 
  	possessed, 
  	blinded, 
  	charmed, 
  	changed,
  	overloaded,
  */
  float delta = 0.0f;
  if(creature->getStateMod(Constants::blessed)) {
    delta += (10.0f * rand()/RAND_MAX);
  }
  if(creature->getStateMod(Constants::empowered)) {
    delta += (10.0f * rand()/RAND_MAX) + 5;
  }
  if(creature->getStateMod(Constants::enraged)) {
    delta -= (10.0f * rand()/RAND_MAX);
  }
  if(creature->getStateMod(Constants::drunk)) {
    delta += (14.0f * rand()/RAND_MAX) - 7;
  }
  if(creature->getStateMod(Constants::cursed)) {
    delta += ((8.0f * rand()/RAND_MAX) + 7);
  }
  if(creature->getStateMod(Constants::blinded)) {
    delta -= (10.0f * rand()/RAND_MAX);
  }
  if(creature->getStateMod(Constants::overloaded)) {
    delta -= (8.0f * rand()/RAND_MAX);
  }
  if(creature->getStateMod(Constants::invisible)) {
    delta += (5.0f * rand()/RAND_MAX) + 5;
  }


  if( !alwaysSucceeds &&
      !projectileHit && 
      (int)((100.0f * rand() / RAND_MAX) + delta) < creature->getActionSpell()->getFailureRate() ) {
    sc->spellFailed();
  } else {

    // get exp for casting the spell
    if(!creature->isMonster()) {
      int level = creature->getLevel();
      if(!creature->getStateMod(Constants::dead)) {
        int n = creature->addExperience(creature->getActionSpell()->getExp());
        if(n > 0) {
          sprintf(message, "%s gains %d experience points.", creature->getName(), n);
          session->getMap()->addDescription(message);
          if( level != creature->getLevel() ) {
            sprintf(message, "%s gains a level!", creature->getName());
            session->getMap()->addDescription(message, 1.0f, 0.5f, 0.5f);
          }
        }
      }
    }

    sc->spellSucceeded();
  }
  delete sc;

  // cancel action
  creature->cancelTarget();
  // also cancel path
  if( !( creature->isMonster() ) ) creature->setSelXY( -1, -1 );

}

void Battle::launchProjectile() {
  sprintf(message, "...%s shoots a projectile", creature->getName());
  session->getMap()->addDescription(message); 
  if(!Projectile::addProjectile(creature, creature->getTargetCreature(), item, 
                                session->getShapePalette()->findShapeByName("ARROW"),
                                creature->getMaxProjectileCount(item))) {
    // max number of projectiles in the air
    // FIXME: do something... 
    // (like print message: can't launch projectile due to use of fixed-sized array in code?)
  }
  if(creature->isMonster() && 
     0 == (int)((float)(session->getPreferences()->getSoundFreq()) * rand()/RAND_MAX)) {
    session->playSound(creature->getMonster()->getRandomSound(Constants::SOUND_TYPE_ATTACK));
  }
  session->playSound( getRandomSound(bowSwishSoundStart, bowSwishSoundCount) );
}

void Battle::projectileHitTurn(Session *session, Projectile *proj, Creature *target) {
  if(debugBattle) cerr << "*** Projectile hit (target): creature=" << proj->getCreature()->getName() << endl;  
  Creature *oldTarget = ((Creature*)(proj->getCreature()))->getTargetCreature();
  ((Creature*)(proj->getCreature()))->setTargetCreature(target);
  Battle *battle = ((Creature*)(proj->getCreature()))->getBattle();
  battle->projectileHit = true;
  if(proj->getItem()) {
    //battle->initItem(proj->getItem());
    battle->item = ((Item*)(proj->getItem()));
    battle->hitWithItem();
  } else if(proj->getSpell()) {
//    battle->spell = proj->getSpell();
//    battle->castSpell();
    SpellCaster *sc = new SpellCaster(battle, proj->getSpell(), true); 
    sc->spellSucceeded();
    delete sc;
  }
  battle->projectileHit = false;
  battle->spell = NULL;
  ((Creature*)(proj->getCreature()))->cancelTarget();
  ((Creature*)(proj->getCreature()))->setTargetCreature(oldTarget);
  // also cancel path
  if( !( ((Creature*)(proj->getCreature()))->isMonster() ) ) 
    ((Creature*)(proj->getCreature()))->setSelXY( -1, -1 );
  if(debugBattle) cerr << "*** Projectile hit ends." << endl;
}

void Battle::projectileHitTurn(Session *session, Projectile *proj, int x, int y) {
  if(debugBattle) cerr << "*** Projectile hit (x,y): creature=" << proj->getCreature()->getName() << endl;
  // configure a turn
  ((Creature*)(proj->getCreature()))->setTargetLocation(x, y, 0);
  Battle *battle = ((Creature*)(proj->getCreature()))->getBattle();
  battle->projectileHit = true;
  if(proj->getItem()) {
    battle->item = ((Item*)(proj->getItem()));
    battle->hitWithItem();
  } else if(proj->getSpell() && 
            proj->getSpell()->isLocationTargetAllowed()) {
    SpellCaster *sc = new SpellCaster(battle, proj->getSpell(), true); 
    sc->spellSucceeded();
    delete sc;
  }
  battle->projectileHit = false;
  battle->spell = NULL;
  ((Creature*)(proj->getCreature()))->cancelTarget();
  if( !( ((Creature*)(proj->getCreature()))->isMonster() ) ) 
    ((Creature*)(proj->getCreature()))->setSelXY( -1, -1 );
  if(debugBattle) cerr << "*** Projectile hit ends." << endl;
}



/**
 * Message to user about battle turn.
 */
void Battle::prepareToHitMessage() {
  if(item) {
    if( session->getPreferences()->getCombatInfoDetail() > 0 ) {
      sprintf(message, "%s(%d) attacks %s(%d) with %s(%d)!", 
              creature->getName(), 
              creature->getLevel(),
              creature->getTargetCreature()->getName(),
              creature->getTargetCreature()->getLevel(),
              item->getItemName(),
              item->getLevel() );
    } else {
      sprintf( message, "%s attacks %s with %s!", 
               creature->getName(), 
               creature->getTargetCreature()->getName(),
               item->getItemName() );
    }
      session->getMap()->addDescription(message);
    ((MD2Shape*)(creature->getShape()))->setAttackEffect(true);

    // play item sound
    if(creature->isMonster() && 
       0 == (int)((float)(session->getPreferences()->getSoundFreq()) * rand()/RAND_MAX)) {
      session->playSound(creature->getMonster()->getRandomSound(Constants::SOUND_TYPE_ATTACK));
    }
    session->playSound( getRandomSound(handheldSwishSoundStart, handheldSwishSoundCount) );

  } else {
    sprintf( message, "%s attacks %s with bare hands!", 
             creature->getName(), 
             creature->getTargetCreature()->getName() );
    session->getMap()->addDescription(message);
    ((MD2Shape*)(creature->getShape()))->setAttackEffect(true);
  }
}

/**
 * Special actions for very low attack roll. If the attack roll
 * is below 10% of the max and the attack range is greater than 4pts,
 * roll to see if there is a fumble.
 */
bool Battle::handleLowAttackRoll( float attack, float min, float max ) {
  if( max - min >= MIN_FUMBLE_RANGE && 
      creature->getTargetCreature() &&
      attack - min < ( ( ( max - min ) / 100.0f ) * 10.0f ) ) {
    if( 0 == (int)( 3.0f * rand() / RAND_MAX ) ) {
      Creature *tmpTarget;
      if( creature->isMonster() || creature->getStateMod( Constants::possessed ) ) {
        tmpTarget = session->
          getClosestVisibleMonster( toint(creature->getX()), 
                                    toint(creature->getY()), 
                                    creature->getShape()->getWidth(),
                                    creature->getShape()->getDepth(),
                                    20 );
      } else {
        tmpTarget = session->getParty()->
          getClosestPlayer( toint(creature->getX()), 
                            toint(creature->getY()), 
                            creature->getShape()->getWidth(),
                            creature->getShape()->getDepth(),
                            20 );
      }
      if( tmpTarget ) {
        // play item sound
        if(item) session->playSound(item->getRandomSound());
        sprintf( message, "...fumble: hits %s instead!", tmpTarget->getName() );
        session->getMap()->addDescription( message );
        Creature *oldTarget = creature->getTargetCreature();
        creature->setTargetCreature( tmpTarget );
        dealDamage( ( MIN_FUMBLE_RANGE * rand() / RAND_MAX ) + 
                    ( MIN_FUMBLE_RANGE / 2.0f ) );
        creature->setTargetCreature( oldTarget );
        return true;
      }
    }
  }
  return false;
}

void Battle::applyHighAttackRoll( float *damage, float attack, float min, float max ) {
  float percent = ( attack - min ) / ( ( max - min ) / 100.0f );
  // special actions for very high tohits
  if( max - min >= MIN_FUMBLE_RANGE && 
      percent > 95 ) {
    int mul = (int)( 8.0f * rand()/RAND_MAX );
    switch( mul ) {
    case 2:
    strcpy(message, "...precise hit: double damage!");
    session->getMap()->addDescription(message);
    (*damage) *= mul;
    break;

    case 3:
    strcpy(message, "...precise hit: triple damage!");
    session->getMap()->addDescription(message);
    (*damage) *= mul;
    break;

    case 4:
    strcpy(message, "...precise hit: quad damage!");
    session->getMap()->addDescription(message);
    (*damage) *= mul;
    break;

    case 0:
    if( percent >= 98 ) {
      strcpy(message, "...precise hit: instant kill!");
      session->getMap()->addDescription(message);
      (*damage) = 1000;
    }
    break;

    default:
    break;
    }
  }
}

void Battle::applyMagicItemDamage( float *damage ) {
  // magical weapons
  if( item && item->isMagicItem() ) {
    int mul = 1;
    if( item->getMonsterType() && creature->getTargetCreature() && 
        !strcmp( item->getMonsterType(), 
                 creature->getTargetCreature()->getModelName() ) ) {
      mul = item->getDamageMultiplier();
    } else if( !item->getMonsterType() ) {
      mul = item->getDamageMultiplier();
    }
    if( mul < 1 ) mul = 1;
    if( mul == 2 ) {
      strcpy( message, "...double damage!" );
      session->getMap()->addDescription( message );
    } else if( mul == 3 ) {
      strcpy( message, "...triple damage!" );
      session->getMap()->addDescription( message );
    } else if( mul == 4 ) {
      strcpy( message, "...quad damage!" );
      session->getMap()->addDescription( message );
    } else if( mul > 4 ) {
      sprintf( message, "...%d-times damage!", mul );
      session->getMap()->addDescription( message );
    }
    (*damage) *= mul;
  }
}

float Battle::applyMagicItemSpellDamage() {
  // magical damage
  if( item && 
      item->isMagicItem() && 
      item->getSchool() &&
      creature->getTargetCreature() && 
      creature->isTargetValid() ) {
    
    // roll for the spell damage
    float damage = creature->rollMagicDamagePercent( item );

    // check for resistance
    int resistance = 
      creature->getTargetCreature()->
      getSkill( item->getSchool()->getResistSkill() );

    // reduce the magic attack by the resistance %.
    damage -= (damage / 100.0f) * (float)resistance;
    if( damage < 0 ) damage = 0;

    char msg[200];
    sprintf( msg, "...%s attacks %s with %s magic.", 
             creature->getName(), 
             creature->getTargetCreature()->getName(),
             item->getSchool()->getShortName() );
    getSession()->getMap()->addDescription( msg, 1, 0.15f, 1 );
    if( resistance > 0 ) {
      sprintf( msg, "%s resists the magic with %d.", 
               creature->getTargetCreature()->getName(),
               resistance );
      getSession()->getMap()->addDescription( msg, 1, 0.15f, 1 );
    }
    
    return damage;
  }
  return -1.0f;
}

void Battle::hitWithItem() {
  prepareToHitMessage();


  float total, max, min, skill, itemLevel;
  float attack = 
    creature->getAttackPercent( item, 
                                &max, 
                                &min,
                                &skill, 
                                &itemLevel );
  float delta = creature->getAttackerStateModPercent();
  float extra = ( attack / 100.0f ) * delta;
  attack += extra;

  sprintf(message, "...%s attacks for %.2f points.", 
          creature->getName(), attack );
  session->getMap()->addDescription(message);
  if( session->getPreferences()->getCombatInfoDetail() > 0 ) {
    sprintf(message, "...(MI:%.2f,MA:%.2f,SK:%.2f,IL:%.2f,EX:%.2f)",
            min, max, skill, itemLevel, extra );
    session->getMap()->addDescription(message);
  }

  // cursed items
  if( item && item->isCursed() ) {
    session->getMap()->addDescription("...Using cursed item!");
    attack -= ( attack / 3.0f );
  }

  // very low attack rolls
  if( handleLowAttackRoll( attack, min, max ) ) return;


  float ac = creature->getTargetCreature()->
    getACPercent( &total, &skill, attack, item );

  sprintf(message, "...%s blocks %.2f points", 
          creature->getTargetCreature()->getName(), ac);
  session->getMap()->addDescription(message);
  if( session->getPreferences()->getCombatInfoDetail() > 0 ) {
    sprintf(message, "...(TO:%.2f,SK:%.2f)",
            total, skill );
    session->getMap()->addDescription(message);
  }


  float damage = ( ac > attack ? 0 : attack - ac );
  if( damage > 0 ) {
    // play item sound
    if( item ) session->playSound( item->getRandomSound() );

    applyMagicItemDamage( &damage );

    applyHighAttackRoll( &damage, attack, min, max );

  }

  dealDamage( damage );

  if( damage > 0 ) {
    // apply extra spell-like damage of magic items
    float spellDamage = applyMagicItemSpellDamage();
    if( spellDamage > -1 ) {
      dealDamage( damage, Constants::EFFECT_GREEN, true );
    }
  }
}

void Battle::dealDamage( float damage, int effect, bool magical, GLuint delay ) {
  if( toint( damage ) > 0 ) {

    // also affects spell attacks
    float delta = creature->getDefenderStateModPercent(magical);
    float extra = ((float)damage / 100.0f) * delta;
    sprintf(message, "...and hits for %.2f(%.2f) points of damage", damage, extra );
    session->getMap()->addDescription(message, 1.0f, 0.5f, 0.5f);
    damage += extra;

    // play hit sound
    if(damage > 0) {
      if(creature->getTargetCreature()->isMonster()) {
        session->playSound(creature->getTargetCreature()->getMonster()->getRandomSound(Constants::SOUND_TYPE_HIT));
      } else {
        session->playSound(creature->getTargetCreature()->getCharacter()->getRandomSound(Constants::SOUND_TYPE_HIT));
      }
    }

    // target creature death
    if(creature->getTargetCreature()->takeDamage( toint( damage ), effect, delay)) {
      // only in RT mode... otherwise in TB mode character won't move
      if( !session->getPreferences()->isBattleTurnBased() )
        creature->getShape()->setCurrentAnimation((int)MD2_TAUNT); 

      sprintf(message, "...%s is killed!", creature->getTargetCreature()->getName());
      session->getMap()->addDescription(message, 1.0f, 0.5f, 0.5f);

      // add exp. points and money
      if(!creature->isMonster()) {


        // FIXME: try to move to party.cpp
        for(int i = 0; i < session->getParty()->getPartySize(); i++) {
          int level = session->getParty()->getParty(i)->getLevel();
          if(!session->getParty()->getParty(i)->getStateMod(Constants::dead)) {
            int n = session->getParty()->getParty(i)->addExperience(creature->getTargetCreature());
            if(n > 0) {
              sprintf(message, "%s gains %d experience points.", session->getParty()->getParty(i)->getName(), n);
              session->getMap()->addDescription(message);
              if( level != session->getParty()->getParty(i)->getLevel() ) {
                sprintf(message, "%s gains a level!", session->getParty()->getParty(i)->getName());
                session->getMap()->addDescription(message, 1.0f, 0.5f, 0.5f);
              }
            }

            n = session->getParty()->getParty(i)->addMoney(creature->getTargetCreature());
            if(n > 0) {
              sprintf(message, "%s finds %d coins!", session->getParty()->getParty(i)->getName(), n);
              session->getMap()->addDescription(message);
            }
          }
        }
        // end of FIXME

        // see if this is a mission objective
        if(session->getCurrentMission() && 
           creature->getTargetCreature()->getMonster() &&
           session->getCurrentMission()->creatureSlain(creature->getTargetCreature())) {
          session->getGameAdapter()->missionCompleted();
        }
      }
    }
  } else {
    sprintf(message, "...and causes no damage");
    session->getMap()->addDescription(message);
  }
}

/*
void Battle::initItem(Item *item) {
  this->item = item;

  // (!item) is a bare-hands attack		
  speed = (item ? item->getSpeed() : Constants::HAND_WEAPON_SPEED) * 
          (session->getPreferences()->getGameSpeedTicks() + 80);
  //	(scourge->getPreferences()->getGameSpeedTicks() + 80);

  creatureInitiative = creature->getInitiative(item);
}
*/

void Battle::executeEatDrinkAction() {
  // is it still in the inventory?
  int index = creature->findInInventory(creature->getActionItem());
  if(index > -1) {
    session->playSound( getRandomSound(potionSoundStart, potionSoundCount) );
    if(creature->eatDrink(creature->getActionItem())){
      creature->removeInventory(index);
      if(!session->getGameAdapter()->isHeadless()) 
        session->getGameAdapter()->refreshInventoryUI();
    }
  }
  // cancel action
  creature->cancelTarget();
  if( !( creature->isMonster() ) ) creature->setSelXY( -1, -1 );
}

void Battle::invalidate() {
  if(debugBattle) cerr << "*** invalidate: creature=" << getCreature()->getName() << endl;
  nextTurn = weaponWait = 0;
}

char *Battle::getRandomSound(int start, int count) {
  if(count)
    return sound[start + (int)((float)(count) * rand()/RAND_MAX)];
  else return NULL;
}

int Battle::getWeaponSpeed( Item *item ) {
  return ( item ? 
           item->getSpeed() : 
           Constants::HAND_WEAPON_SPEED ) * 
    WEAPON_WAIT_MUL;
}

bool Battle::describeAttack( Creature *target, char *buff, Color *color, bool includeActions ) {

  initTurnStep();

  // info for the player?
  if( includeActions && session->getParty()->getPlayer() == creature ) {
    if( creature->getAction() == Constants::ACTION_CAST_SPELL ) {
      sprintf( buff, "%s: %d", 
               creature->getActionSpell()->getName(), 
               ( nextTurn > 0 ? nextTurn : weaponWait ) );
      color->r = 0.6f;
      color->g = 0.1f;
      color->b = 0.6f;
      return true;
    } else if( creature->getAction() == Constants::ACTION_EAT_DRINK ) {
      sprintf( buff, "%s: %d", 
               creature->getActionItem()->getRpgItem()->getName(),
               ( nextTurn > 0 ? nextTurn : weaponWait ) );
      color->r = 0;
      color->g = 0.6f;
      color->b = 0;
      return true;
    } else if( creature->getAction() == Constants::ACTION_SPECIAL ) {
      sprintf( buff, "%s: %d", 
               creature->getActionSkill()->getName(), 
               ( nextTurn > 0 ? nextTurn : weaponWait ) );
      color->r = 0;
      color->g = 0.3f;
      color->b = 0.6f;
      return true;
    }
  }

  // info for the target
  bool sameTarget = ( creature->getTargetCreature() == target );
  if( !target || !creature->canAttack( target ) ) return false;
  
  Creature *tmp = NULL;
  if( !sameTarget ) {
    tmp = creature->getTargetCreature();
    creature->setTargetCreature( target );
  }
  float dist = creature->getDistanceToTarget();
  Item *item = creature->getBestWeapon(dist);
  if( !sameTarget ) creature->setTargetCreature( tmp );
  
  // out of range
  if( !item && dist > MIN_DISTANCE ) {
    if( sameTarget ) {
      if( creature->getAction() == Constants::ACTION_CAST_SPELL ) {
        sprintf( buff, "Targeted" );
        color->r = 0.6f;
        color->g = 0.1f;
        color->b = 0.6f;
      } else if( creature->getAction() == Constants::ACTION_SPECIAL ) {
        sprintf( buff, "Targeted" );
        color->r = 0;
        color->g = 0.3f;
        color->b = 0.6f;
      } else {
        color->r = 0.5f;
        color->g = 0.2f;
        color->b = 0;
        if( creature->isPathToTargetCreature() ) {
          sprintf( buff, "Out of Range. Move: %d", creature->getPath()->size() );
        } else {
          sprintf( buff, "Out of Range" );
        }
      }
    } else {
      color->r = 0.3f;
      color->g = 0.3f;
      color->b = 0.3f;
      // could still be in range for spell or skill
      sprintf( buff, ( range >= dist ? "In Range" : "Out of Range" ) );
    }
    return true;
  }
  
  // How many steps to wait before being able to use the weapon.
  int weaponWait = getWeaponSpeed( item );
  if( session->getPreferences()->isBattleTurnBased() ) {
    weaponWait /= 2;
  }
  
  sprintf( buff, "%s: %d", 
           ( item ? item->getRpgItem()->getName() : "Bare Hands" ), 
           ( sameTarget && nextTurn > 0 ? nextTurn : weaponWait ) );
   color->r = 0.6f;
   color->g = 0;
   color->b = 0;
  return true;
}

