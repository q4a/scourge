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

//#define DEBUG_BATTLE
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
}

Battle::~Battle() {
}

void Battle::reset() {
  this->steps = 0;
  this->startingAp = this->ap = 10 + (creature->getSkill(Constants::COORDINATION) / 10);
  this->projectileHit = false;
  this->paused = false;
}

void Battle::setupBattles(Session *session, Battle *battle[], int count, vector<Battle *> *turns) {
  // for now put all battles into the vector
  for(int i = 0; i < count; i++) {
    // reset for the first time
    // (to avoid whack skill numbers for un-initialized creatures.)
    if(battle[i]->needsReset) {
      battle[i]->reset();
      battle[i]->needsReset = false;
    }
    battle[i]->initTurn();
    battle[i]->getCreature()->getShape()->setCurrentAnimation((int)MD2_STAND, true);
    turns->push_back(battle[i]);
  }



/*
  if(count == 0) return;
  bool battleStarted = false;
  int battleCount = count;  
  int initiative = -10;
//char message[200];
  int turn = 0;

#ifdef DEBUG_BATTLE
  cerr << "==================================================" << endl;
  cerr << "battleCount=" << battleCount << endl;
  for(int i = 0; i < battleCount; i++) {
    cerr << "\t(" << battle[i]->creature->getName() << 
    "->" << (battle[i]->creature->getTargetCreature() ? 
             battle[i]->creature->getTargetCreature()->getName() :
             "NULL") << 
    ")" << endl;
  }
#endif

// this is O(n^2) unfortunately... maybe we could use a linked list or something here
  while(battleCount > 0) {
    bool creatureFound = false; 

    for(int i = 0; i < battleCount; i++) {
      int action = NO_ACTION;

      battle[i]->empty = false;
      battle[i]->projectileHit = false;

      if(!battle[i]->creature->hasTarget()) {
        // remove creatures with no target creature from this round
        action = NO_TARGET;
      } else if(!battle[i]->creature->isTargetValid()) {
        // if someone already killed this target, skip it
        battle[i]->creature->cancelTarget();
        if(battle[i]->creature->isMonster()) {
          //battle[i]->creature->setMotion(Constants::MOTION_LOITER);     
          action = LOITER;
        } else {
          // if party member re-join the battle
          Creature *c = battle[i]->creature;
          c->setTargetCreature(scourge->getClosestVisibleMonster(c->getX(), 
                                                                 c->getY(), 
                                                                 c->getShape()->getWidth(),
                                                                 c->getShape()->getDepth(),
                                                                 20));
        }
      } else {
        GLint t = SDL_GetTicks();

        // get the best weapon given the distance from the target
        battle[i]->initTurn();

        // check the creature's initiative
        if(!battle[i]->initiativeCheck || 
           battle[i]->creatureInitiative < initiative) {

          // remember that it passed (creature can attack next time if timing (below) is good)
          battle[i]->initiativeCheck = true;
          creatureFound = true;

          // this is to slow down battle, depending on your weapon
          // getLastTurn is 0 when an action is decided to be executed
          // this way it's two trips through this method forcing a spell (e.g.)
          // to use the clock.
          bool initialActionTurn = (battle[i]->creature->getLastTurn() == 0);
#ifdef DEBUG_BATTLE                                                                   
          cerr << "speed=" << battle[i]->speed << " last=" << battle[i]->creature->getLastTurn() << 
          " diff=" << (t - battle[i]->creature->getLastTurn()) << endl;
#endif                                                                                        
          if(initialActionTurn ||
             battle[i]->speed < t - battle[i]->creature->getLastTurn()) {

            // remember the last active turn			
            battle[i]->creature->setLastTurn(t);

            if(!battleStarted) {
              scourge->getMap()->addDescription("A round of battle begins...", 1, 1, 1);
              battleStarted = true;
            }

            if(initialActionTurn) {
              // add a waiting term
              battle[i]->empty = true;
              action = WAIT;
              turns->push_back(battle[i]);
            } else {
              // save this turn
              turns->push_back(battle[i]);
              action = ATTACK;
            }
          } else {
            // add a waiting term
            battle[i]->empty = true;
            action = WAIT;
            turns->push_back(battle[i]);
          }
        }
      }

      if(action != NO_ACTION) {

#ifdef DEBUG_BATTLE                 
        cerr << "Turn: " << turn << " " << actionName[action] << ", " <<
        battle[i]->creature->getName() << "(" << battle[i]->creatureInitiative << ")" <<
        "->" << (battle[i]->creature->getTargetCreature() ? 
                 battle[i]->creature->getTargetCreature()->getName() : 
                 "<no target>") <<
        endl;
#endif                                                    

        // remove this battle from the turn
        Battle *tmp = battle[i];
        for(int t = i; t < battleCount - 1; t++) {
          battle[t] = battle[t + 1];
        }
        // move current one to the end of the list so we don't loose it
        battle[battleCount - 1] = tmp;
        battleCount--;
        i--;
        turn++;
      }
    }

    // if there are no creatures for this initiative slot, go to the next one
    if(!creatureFound) initiative++;
  }
  */
}                 

bool Battle::pauseBeforePlayerTurn() {
  // go to single-player mode
  if (session->getUserConfiguration()->isBattleTurnBased() &&
      !session->getParty()->isPlayerOnly()) {
    session->getParty()->togglePlayerOnly(true);
  }

  // pause if this is a player's first step
  if (!creature->isMonster() &&
      !steps && 
      !paused &&
      session->getUserConfiguration()->isBattleTurnBased()) {
    cerr << "Pausing for round start. Turn: " << creature->getName() << endl;

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
  }
  steps++;
  paused = false;
  return false;
}

bool Battle::fightTurn() {

  cerr << "TURN: creature=" << creature->getName() << " ap=" << ap << " coordination=" << creature->getSkill(Constants::COORDINATION) << endl;

  // are we alive?
  if(!creature || creature->getStateMod(Constants::dead)) return true;

  // done with this creature's turn
  if(ap <= 0) {
    reset();
    // This is weird... but it's the only way I could make it work
    // setting the anim. to anything else causes them to not run 
    // when moving after battle.
    creature->getShape()->setCurrentAnimation((int)MD2_STAND, true);
    ((MD2Shape*)(creature->getShape()))->setAttackEffect(false);
    return true;
  }

  if(pauseBeforePlayerTurn()) return false;

  dist = creature->getDistanceToTarget();
  item = creature->getBestWeapon(dist);

  if(item) {
    cerr << "\tUsing item: " << item->getRpgItem()->getName() << " ap=" << ap << endl;
  } else {
    cerr << "\tUsing bare hands." << endl;
  }

  float range = Constants::MIN_DISTANCE;
  if(item) range = item->getRpgItem()->getDistance();
  if(creature->getTargetCreature()) {
    range += (creature->getTargetCreature()->getShape()->getWidth() > creature->getTargetCreature()->getShape()->getDepth() ? 
              creature->getTargetCreature()->getShape()->getWidth() / 2 :
              creature->getTargetCreature()->getShape()->getDepth() / 2);
  }
  cerr << "\tDistance=" << dist << " range=" << range << endl;

  /**                 
    * How many steps to wait before being able to use the weapon.
    * 
    *   FIXME: When implementing for real, this depends on item/spell,etc. 
    * and skills like speed/proficency. Hard-coded for now.
  */
  int weaponWait = 3;

  if(creature->getTargetCreature()) {
    if(creature->isTargetValid()) {
      if(dist < range) {
        if(!projectileHit) {
          ap -= weaponWait;
          if(ap < 0) ap = 0;
        }
        // attack
        cerr << "\t\tAttacking." << endl;
        if(!projectileHit && item && item->getRpgItem()->isRangedWeapon()) {
          launchProjectile();
        } else {
          hitWithItem();
        }
        creature->getShape()->setCurrentAnimation((int)MD2_ATTACK, true);	  
        ((MD2Shape*)(creature->getShape()))->setAngle(creature->getTargetAngle());
      } else {
        // out of range: take 1 step closer
        creature->getShape()->setCurrentAnimation((int)MD2_RUN, true);
        cerr << "\t\tTaking a step." << endl;
        if(!(creature->getSelX() == creature->getTargetCreature()->getX() &&
             creature->getSelY() == creature->getTargetCreature()->getY())) {
          creature->setSelXY(creature->getTargetCreature()->getX(),
                             creature->getTargetCreature()->getY(),
                             true);
        }
        creature->moveToLocator(session->getMap());
        ap--;
      }
    } else {
      // select a new target
      // FIXME: this code should move to Creature
      Creature *target = NULL;
      if(creature->isMonster()) {
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
      if(target) {
        cerr << "\tSelected new target: " << target->getName() << endl;
        creature->setTargetCreature(target);
        creature->setSelXY(creature->getTargetCreature()->getX(),
                           creature->getTargetCreature()->getY(),
                           true);
        initTurn();
      } else {
        creature->setTargetCreature(NULL);
        cerr << "\t\tCan't find new target." << endl;
        return true;
      }
    }
  } else {
    creature->getShape()->setCurrentAnimation((int)MD2_RUN, true);
    
    // take 1 step closer
    cerr << "\t\tTaking a non-battle step." << endl;
    if(creature->isMonster()) {
      session->getGameAdapter()->moveMonster(creature);
    } else {
      creature->moveToLocator(session->getMap());
    }
    ap--;
  }

  // not done yet with creature's turn
  return false;

  /*

  // waiting to cast a spell?
  if(creature->getAction() == Constants::ACTION_CAST_SPELL) {
    creature->startEffect(Constants::EFFECT_CAST_SPELL, 
                          Constants::DAMAGE_DURATION * 4);
  }

  // waiting to attack?
  if(isEmpty()) return;

  // target killed?
  if(!creature->hasTarget()) return;

  // if there was no 'item' selected it's because we're too far from the target
  // (getBestWeapon returned NULL) and we're not casting a spell, follow the target
  //
  // If it's an empty-handed attack, follow the target
  //
  // If it's a ranged attack and we're not in range, follow the target (will move away from target)
  //
  // This sure is confusing code...
//  cerr << "projectileHit=" << projectileHit <<
//        " dist=" << dist <<
//        " creature->isInRange()=" << creature->isInRange() <<
//        " item=" << item <<
//        " creature->getActionSpell()=" << creature->getActionSpell() <<
//        " spell=" << spell << endl;
  if(!projectileHit &&
     ((dist > Constants::MIN_DISTANCE && !item && !creature->getActionSpell() && !spell) ||  
      !creature->isInRange())) { 
    //cerr << "\tfollowing..." << endl;
    creature->followTarget();
    return;
  }
  //cerr << "\tcontinuing" << endl;

  // handle action: eat/drink items
  if(creature->getAction() == Constants::ACTION_EAT_DRINK) {
    executeEatDrinkAction();
    return;
  }

  // the attacked target may get upset
  creature->decideMonsterAction();

  // handle the action
  if(!projectileHit && item && item->getRpgItem()->isRangedWeapon()) {
    launchProjectile();
  } else if(spell) {
    // a spell projectile hit
    SpellCaster *sc = new SpellCaster(this, spell, true); 
    sc->spellSucceeded();
    delete sc;
  } else if(creature->getActionSpell()) {
    // casting a spell for the first time
    castSpell();
  } else {
    hitWithItem();
  }
*/  
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
  
  Creature *oldTarget = proj->getCreature()->getTargetCreature();
  proj->getCreature()->setTargetCreature(target);
  Battle *battle = proj->getCreature()->getBattle();
  if(proj->getItem()) {
    battle->initItem(proj->getItem());
  } else if(proj->getSpell()) {
    battle->spell = proj->getSpell();
  }
  battle->projectileHit = true;
  battle->hitWithItem();
  battle->projectileHit = false;
  proj->getCreature()->cancelTarget();
  proj->getCreature()->setTargetCreature(oldTarget);




  /*
  // configure a turn
  Creature *oldTarget = proj->getCreature()->getTargetCreature();
  proj->getCreature()->setTargetCreature(target);
  Battle *battle = new Battle(session, proj->getCreature());
  battle->projectileHit = true;

  if(proj->getItem()) {
    battle->initItem(proj->getItem());
  } else if(proj->getSpell()) {
    battle->spell = proj->getSpell();
  }
  // play it
  battle->fightTurn();

  delete battle;
  proj->getCreature()->cancelTarget();
  proj->getCreature()->setTargetCreature(oldTarget);
  */
}

void Battle::projectileHitTurn(Session *session, Projectile *proj, int x, int y) {
  // configure a turn
  proj->getCreature()->setTargetLocation(x, y, 0);
  Battle *battle = new Battle(session, proj->getCreature());
  battle->projectileHit = true;

  if(proj->getItem()) {
    battle->initItem(proj->getItem());
  } else if(proj->getSpell()) {
    battle->spell = proj->getSpell();
  }
  // play it
  battle->fightTurn();
  
  delete battle;
  proj->getCreature()->cancelTarget();
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

void Battle::initTurn() {
/*
  float range = 0.0f;

  // select a weapon
  if(creature->getAction() == Constants::ACTION_NO_ACTION) {

    // already selected weapon?
    if(item) return;
    Item *i = creature->getBestWeapon(dist);  
    if(i) range = i->getRpgItem()->getDistance();
    // set up distance range for ranged weapons (do it here so it only happens when the action changes)
    creature->setDistanceRange(0, Constants::MIN_DISTANCE);
    if(range >= 8) {
      creature->setDistanceRange(Constants::MIN_DISTANCE, range);
    }
    initItem(i);
  } else {
    // or init action
    switch(creature->getAction()) {
    case Constants::ACTION_CAST_SPELL:
      range = creature->getActionSpell()->getDistance();
      speed = creature->getActionSpell()->getSpeed() * 
              (scourge->getUserConfiguration()->getGameSpeedTicks() + 80);
      creatureInitiative = creature->getInitiative(NULL, creature->getActionSpell());
      // set up distance range for ranged weapons (do it here so it only happens when the action changes)
      creature->setDistanceRange(0, Constants::MIN_DISTANCE);
      if(range >= 8) {
        creature->setDistanceRange(Constants::MIN_DISTANCE, range);
      }
      break;
    case Constants::ACTION_EAT_DRINK:
      range = creature->getActionItem()->getRpgItem()->getDistance();
      speed = creature->getActionItem()->getRpgItem()->getSpeed() *
              (scourge->getUserConfiguration()->getGameSpeedTicks() + 80);
      creatureInitiative = creature->getInitiative(creature->getActionItem(), NULL);
      // set up distance range for ranged weapons (do it here so it only happens when the action changes)
      creature->setDistanceRange(0, Constants::MIN_DISTANCE);
      if(range >= 8) {
        creature->setDistanceRange(Constants::MIN_DISTANCE, range);
      }
      break;
    default:
      cerr << "*** Error: unhandled action: " << creature->getAction() << endl;
    }
  }  
  */
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
