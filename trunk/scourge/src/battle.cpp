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

#define GOD_MODE 0
#define MONSTER_IMORTALITY 0

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
  if(DEBUG_BATTLE) cerr << "*** constructor, creature=" << creature->getName() << endl;
}

Battle::~Battle() {
}

void Battle::reset() {
  if(DEBUG_BATTLE) cerr << "*** reset: creature=" << creature->getName() << endl;
  this->steps = 0;
  this->startingAp = this->ap = 10 + (creature->getSkill(Constants::COORDINATION) / 10);
  this->projectileHit = false;
  this->paused = false;
  this->weaponWait = 0;
  this->range = 0.0f;
  creature->getShape()->setCurrentAnimation((int)MD2_STAND, true);
  ((MD2Shape*)(creature->getShape()))->setAttackEffect(false);
}

void Battle::setupBattles(Session *session, Battle *battle[], int count, vector<Battle *> *turns) {
  // for now put all battles into the vector
  // FIXME: need to order by initiative
  for(int i = 0; i < count; i++) {
    // reset for the first time
    // (to avoid whack skill numbers for un-initialized creatures.)
    if(battle[i]->needsReset) {
      battle[i]->reset();
      battle[i]->needsReset = false;
    }
    //battle[i]->initTurn();
    battle[i]->getCreature()->getShape()->setCurrentAnimation((int)MD2_STAND, true);
    turns->push_back(battle[i]);
  }
}                 

bool Battle::fightTurn() {

  if(DEBUG_BATTLE) cerr << "TURN: creature=" << creature->getName() << 
    " ap=" << ap << 
    " wait=" << weaponWait << 
    " nextTurn=" << nextTurn << endl;

  // are we alive?
  if(!creature || creature->getStateMod(Constants::dead)) return true;

  // done with this creature's turn
  if(ap <= 0) {
    if(weaponWait > 0) {
      nextTurn = weaponWait;
      if(DEBUG_BATTLE) cerr << "Carries over into next turn." << endl;
    }
    reset();
    return true;
  }

  // waiting to cast a spell?
  if(creature->getAction() == Constants::ACTION_CAST_SPELL) {
    creature->startEffect(Constants::EFFECT_CAST_SPELL, 
                          Constants::DAMAGE_DURATION * 4);
  }

  if(pauseBeforePlayerTurn()) return false;

  initTurnStep();

  if(creature->getAction() == Constants::ACTION_EAT_DRINK) {
    executeEatDrinkAction();
  }

  if(creature->hasTarget()) {
    if(creature->isTargetValid()) {
      if(dist < range) {
        executeAction();
      } else {
        stepCloserToTarget();
      }
    } else {
      if(!selectNewTarget()) return true;
    }
  } else {
    moveCreature();
  }

  // not done yet with creature's turn
  return false;
}

bool Battle::pauseBeforePlayerTurn() {
  // go to single-player mode
  if (session->getUserConfiguration()->isBattleTurnBased() &&
      !session->getParty()->isPlayerOnly()) {
    session->getParty()->togglePlayerOnly(true);
  }

  // pause if this is a player's first step
  if (!steps && 
      !paused &&
      session->getUserConfiguration()->isBattleTurnBased()) {
    if(!creature->isMonster()) {
      if(DEBUG_BATTLE) cerr << "Pausing for round start. Turn: " << creature->getName() << endl;

      // center on player
      for (int i = 0; i < session->getParty()->getPartySize(); i++) {
        if (session->getParty()->getParty(i) == creature) {
          session->getParty()->setPlayer(i);
          break;
        }
      }
      // FIXME: only center if not on-screen
      session->getMap()->refresh();
      session->getMap()->center(creature->getX(), creature->getY(), true);

      // pause the game
      session->getParty()->toggleRound(true);
      paused = true;
      return true;
    } else {
      // FIXME: only center if not on-screen
      session->getMap()->refresh();
      session->getMap()->center(creature->getX(), creature->getY(), true);
    }
  }
  steps++;
  paused = false;
  return false;
}

void Battle::initTurnStep() {
  dist = creature->getDistanceToTarget();

  // select the best weapon only once
  if(weaponWait <= 0) {
    if(DEBUG_BATTLE) cerr << "*** initTurnStep, creature=" << creature->getName() << " wait=" << weaponWait << " nextTurn=" << nextTurn << endl;
    if(creature->getActionSpell()) {
      range = Constants::MIN_DISTANCE;
      range = creature->getActionSpell()->getDistance();
      if(creature->getTargetCreature()) {
        range += (creature->getTargetCreature()->getShape()->getWidth() > creature->getTargetCreature()->getShape()->getDepth() ? 
                  creature->getTargetCreature()->getShape()->getWidth() / 2 :
                  creature->getTargetCreature()->getShape()->getDepth() / 2);
      } else if(creature->getTargetItem()) {
        range += (creature->getTargetItem()->getShape()->getWidth() > creature->getTargetItem()->getShape()->getDepth() ? 
                  creature->getTargetItem()->getShape()->getWidth() / 2 :
                  creature->getTargetItem()->getShape()->getDepth() / 2);
      }
      if(nextTurn > 0) weaponWait = nextTurn;
      else weaponWait = creature->getActionSpell()->getSpeed();
      nextTurn = 0;
      if(DEBUG_BATTLE) cerr << "\tUsing spell: " << creature->getActionSpell()->getName() << endl;
    } else {
      item = creature->getBestWeapon(dist);
      range = Constants::MIN_DISTANCE;
      if(item) range = item->getRpgItem()->getDistance();
      if(creature->getTargetCreature()) {
        range += (creature->getTargetCreature()->getShape()->getWidth() > creature->getTargetCreature()->getShape()->getDepth() ? 
                  creature->getTargetCreature()->getShape()->getWidth() / 2 :
                  creature->getTargetCreature()->getShape()->getDepth() / 2);
      }
      if(item) {
        if(DEBUG_BATTLE) cerr << "\tUsing item: " << item->getRpgItem()->getName() << " ap=" << ap << endl;
      } else {
        if(DEBUG_BATTLE) cerr << "\tUsing bare hands." << endl;
      }
      // How many steps to wait before being able to use the weapon.
      weaponWait = (item ? item->getRpgItem()->getSpeed() : Constants::HAND_WEAPON_SPEED);
    }
    if(nextTurn > 0) weaponWait = nextTurn;
    nextTurn = 0;
    if(DEBUG_BATTLE) cerr << "\tDistance=" << dist << " range=" << range << " wait=" << weaponWait << endl;
  }
}

void Battle::executeAction() {
  ap--;
  if(weaponWait > 0) {
    weaponWait--;
    if(weaponWait > 0) return;
  }

  // attack
  if(DEBUG_BATTLE) cerr << "\t\t *** Attacking." << endl;
  if(creature->getActionSpell()) {
    // casting a spell for the first time
    castSpell();
  } else if(item && item->getRpgItem()->isRangedWeapon()) {
      launchProjectile();
  } else {
    hitWithItem();
  }
  creature->getShape()->setCurrentAnimation((int)MD2_ATTACK, true);	  
  ((MD2Shape*)(creature->getShape()))->setAngle(creature->getTargetAngle());
}

void Battle::stepCloserToTarget() {
  // out of range: take 1 step closer
  creature->getShape()->setCurrentAnimation((int)MD2_RUN, true);
  if(DEBUG_BATTLE) cerr << "\t\tTaking a step." << endl;
  if(creature->getTargetCreature()) {
    if(!(creature->getSelX() == creature->getTargetCreature()->getX() &&
         creature->getSelY() == creature->getTargetCreature()->getY())) {
      creature->setSelXY(creature->getTargetCreature()->getX(),
                         creature->getTargetCreature()->getY(),
                         true);
    }
  } else {
    if(!(creature->getSelX() == creature->getTargetX() &&
         creature->getSelY() == creature->getTargetY())) {
      creature->setSelXY(creature->getTargetX(),
                         creature->getTargetY(),
                         true);
    }
  }
  creature->moveToLocator(session->getMap());
  ap--;
}

bool Battle::selectNewTarget() {
// select a new target
  // FIXME: this code should move to Creature
  Creature *target = NULL;
  if (creature->isMonster()) {
    target = session->getParty()->getClosestPlayer(creature->getX(), 
                                                   creature->getY(), 
                                                   creature->getShape()->getWidth(),
                                                   creature->getShape()->getDepth(),
                                                   20);
  } else {
    target = session->getClosestVisibleMonster(creature->getX(), 
                                               creature->getY(), 
                                               creature->getShape()->getWidth(),
                                               creature->getShape()->getDepth(),
                                               20);
  }
  if (target) {
    if(DEBUG_BATTLE) cerr << "\tSelected new target: " << target->getName() << endl;
    creature->setTargetCreature(target);
    creature->setSelXY(creature->getTargetCreature()->getX(),
                       creature->getTargetCreature()->getY(),
                       true);
    //initTurn();
    return true;
  } else {
    creature->setTargetCreature(NULL);
    if(DEBUG_BATTLE) cerr << "\t\tCan't find new target." << endl;
    return false;
  }
}

void Battle::moveCreature() {
  creature->getShape()->setCurrentAnimation((int)MD2_RUN, true);

  // take 1 step closer
  if(DEBUG_BATTLE) cerr << "\t\tTaking a non-battle step." << endl;
  if(creature->isMonster()) {
    session->getGameAdapter()->moveMonster(creature);
  } else {
    creature->moveToLocator(session->getMap());
  }
  ap--;
}

void Battle::castSpell() {
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
      session->getGameAdapter()->refreshInventoryUI();
    } else {
      // scroll was removed from inventory before casting
      sprintf(message, "Couldn't find scroll, cancelled spell.");
      session->getMap()->addDescription(message);
      creature->cancelTarget();
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
  if(!projectileHit && 
     (int)(100.0f * rand() / RAND_MAX) < creature->getActionSpell()->getFailureRate()) {
    sc->spellFailed();
  } else {

    // get exp for casting the spell
    if(!creature->isMonster()) {
      bool b = creature->getStateMod(Constants::leveled);
      if(!creature->getStateMod(Constants::dead)) {
        int n = creature->addExperience(creature->getActionSpell()->getExp());
        if(n > 0) {
          sprintf(message, "%s gains %d experience points.", creature->getName(), n);
          session->getMap()->addDescription(message);
          if(!b && creature->getStateMod(Constants::leveled)) {
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
}

void Battle::projectileHitTurn(Session *session, Projectile *proj, Creature *target) {
  if(DEBUG_BATTLE) cerr << "*** Projectile hit (target): creature=" << proj->getCreature()->getName() << endl;  
  Creature *oldTarget = proj->getCreature()->getTargetCreature();
  proj->getCreature()->setTargetCreature(target);
  Battle *battle = proj->getCreature()->getBattle();
  battle->projectileHit = true;
  if(proj->getItem()) {
    //battle->initItem(proj->getItem());
    battle->item = proj->getItem();
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
  proj->getCreature()->cancelTarget();
  proj->getCreature()->setTargetCreature(oldTarget);
  if(DEBUG_BATTLE) cerr << "*** Projectile hit ends." << endl;
}

void Battle::projectileHitTurn(Session *session, Projectile *proj, int x, int y) {
  if(DEBUG_BATTLE) cerr << "*** Projectile hit (x,y): creature=" << proj->getCreature()->getName() << endl;
  // configure a turn
  proj->getCreature()->setTargetLocation(x, y, 0);
  Battle *battle = proj->getCreature()->getBattle();
  battle->projectileHit = true;
  if(proj->getItem()) {
    //battle->initItem(proj->getItem());
    battle->item = proj->getItem();
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
  proj->getCreature()->cancelTarget();
  if(DEBUG_BATTLE) cerr << "*** Projectile hit ends." << endl;
}

void Battle::hitWithItem() {

  if(item) {
    sprintf(message, "%s attacks %s with %s! (I:%d,S:%d)", 
            creature->getName(), 
            creature->getTargetCreature()->getName(),
            item->getItemName(),
            creatureInitiative, speed);
    session->getMap()->addDescription(message);
    ((MD2Shape*)(creature->getShape()))->setAttackEffect(true);
  } else if(dist <= Constants::MIN_DISTANCE) {
    sprintf(message, "%s attacks %s with bare hands! (I:%d,S:%d)", 
            creature->getName(), 
            creature->getTargetCreature()->getName(),
            creatureInitiative, speed);
    session->getMap()->addDescription(message);
    ((MD2Shape*)(creature->getShape()))->setAttackEffect(true);
  }

  // take a swing
  int maxToHit;
  int tohit = creature->getToHit(item, &maxToHit);
  int ac = creature->getTargetCreature()->getSkillModifiedArmor();
  sprintf(message, "...%s defends with armor=%d", creature->getTargetCreature()->getName(), ac);
  session->getMap()->addDescription(message);
  sprintf(message, "...toHit=%d (max=%d) vs. AC=%d", tohit, maxToHit, ac);
  session->getMap()->addDescription(message);
  if(tohit > ac) {
    // deal out the damage
    int maxDamage;
    int damage = creature->getDamage(item, &maxDamage);
    dealDamage(damage, maxDamage);
  } else {
    // missed
    sprintf(message, "...and misses! (toHit=%d vs. AC=%d)", tohit, ac);
    session->getMap()->addDescription(message);
  }
}

void Battle::dealDamage(int damage, int maxDamage, int effect) {
  if(damage) {  
    sprintf(message, "...and hits! for %d (max=%d) points of damage", damage, maxDamage);
    session->getMap()->addDescription(message, 1.0f, 0.5f, 0.5f);

    // target creature death
    if(creature->getTargetCreature()->takeDamage(damage, effect)) {         
      creature->getShape()->setCurrentAnimation((int)MD2_TAUNT, true);  
      sprintf(message, "...%s is killed!", creature->getTargetCreature()->getName());
      session->getMap()->addDescription(message, 1.0f, 0.5f, 0.5f);

      if((creature->getTargetCreature()->isMonster() && !MONSTER_IMORTALITY) || 
         !GOD_MODE)
        session->creatureDeath(creature->getTargetCreature());

      // add exp. points and money
      if(!creature->isMonster()) {


        // FIXME: try to move to party.cpp
        for(int i = 0; i < session->getParty()->getPartySize(); i++) {
          bool b = session->getParty()->getParty(i)->getStateMod(Constants::leveled);
          if(!session->getParty()->getParty(i)->getStateMod(Constants::dead)) {
            int n = session->getParty()->getParty(i)->addExperience(creature->getTargetCreature());
            if(n > 0) {
              sprintf(message, "%s gains %d experience points.", session->getParty()->getParty(i)->getName(), n);
              session->getMap()->addDescription(message);
              if(!b && session->getParty()->getParty(i)->getStateMod(Constants::leveled)) {
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
           session->getCurrentMission()->monsterSlain(creature->getTargetCreature()->getMonster())) {
          session->getGameAdapter()->missionCompleted();
        }
      }
    }
  } else {
    sprintf(message, "...and hits! but causes no damage");
    session->getMap()->addDescription(message);
  }
}

void Battle::initItem(Item *item) {
  this->item = item;

  // (!item) is a bare-hands attack		
  speed = (item ? item->getRpgItem()->getSpeed() : Constants::HAND_WEAPON_SPEED) * 
          (session->getUserConfiguration()->getGameSpeedTicks() + 80);
  //	(scourge->getUserConfiguration()->getGameSpeedTicks() + 80);

  creatureInitiative = creature->getInitiative(item);
}

void Battle::executeEatDrinkAction() {
  // is it still in the inventory?
  int index = creature->findInInventory(creature->getActionItem());
  if(index > -1) {
    if(creature->eatDrink(creature->getActionItem())){
      creature->removeInventory(index);
      session->getGameAdapter()->refreshInventoryUI();
    }
  }
  // cancel action
  creature->cancelTarget();
}

void Battle::invalidate() {
  if(DEBUG_BATTLE) cerr << "*** invalidate: creature=" << getCreature()->getName() << endl;
  nextTurn = weaponWait = 0;
}

