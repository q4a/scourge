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

enum {
  NO_ACTION = 0,
  LOITER,
  ATTACK,
  MOVE,
  NO_TARGET
};

char *actionName[] = { "NO ACTION", "LOITER", "ATTACK", "MOVE", "NO_TARGET" };

Battle::Battle() {
  this->creature = NULL;
}

Battle::Battle(Scourge *scourge, Creature *creature) {
  this->scourge = scourge;
  this->creature = creature;
  this->item = NULL;
  this->creatureInitiative = 0;
  this->initiativeCheck = false;
  this->itemSpeed = 0;
  this->dist = 0.0f;
  this->dist = Util::distance(creature->getX(),  creature->getY(), 
							  creature->getShape()->getWidth(), creature->getShape()->getDepth(),
							  creature->getTargetCreature()->getX(), creature->getTargetCreature()->getY(),
							  creature->getTargetCreature()->getShape()->getWidth(), creature->getTargetCreature()->getShape()->getDepth());
}

Battle::~Battle() {
}

void Battle::setupBattles(Scourge *scourge, Battle *battle[], int count, vector<Battle *> *turns) {

  if(count == 0) return;

  bool battleStarted = false;
  int battleCount = count;  
  int initiative = -10;
  char message[200];
  int turn = 0;

#ifdef DEBUG_BATTLE
  cerr << "==================================================" << endl;
  cerr << "battleCount=" << battleCount << endl;
  for(int i = 0; i < battleCount; i++) {
	cerr << "\t(" << battle[i]->creature->getName() << 
	  "->" << battle[i]->creature->getTargetCreature()->getName() << 
	  ")" << endl;
  }
#endif

  // this is O(n^2) unfortunately... maybe we could use a linked list or something here
  while(battleCount > 0) {
	bool creatureFound = false;

	for(int i = 0; i < battleCount; i++) {
	  int action = NO_ACTION;

	  if(!battle[i]->creature->getTargetCreature()) {
		// remove creatures with no target creature from this round
		action = NO_TARGET;
	  } else if(battle[i]->creature->getTargetCreature()->getStateMod(Constants::dead)) {
		// if someone already killed this target, skip it
		battle[i]->creature->setTargetCreature(NULL);
		if(battle[i]->creature->isMonster()) {
		  battle[i]->creature->setMotion(Constants::MOTION_LOITER);

		  action = LOITER;
		} else {
		  // FIXME: should help out another party member
		}
	  } else {

		GLint t = SDL_GetTicks();

		// get the best weapon given the distance from the target
		battle[i]->selectBestItem();

		// FIXME: clean this up
		// creature won't fight if too far from the action (!item is a bare-hands attack)
		if(1 || battle[i]->item || battle[i]->dist <= 1.0f) {

		  // check the creature's initiative
		  if(!battle[i]->initiativeCheck || 
			 battle[i]->creatureInitiative < initiative) {

			// remember that it passed (creature can attack next time if timing (below) is good)
			battle[i]->initiativeCheck = true;
			creatureFound = true;
			
			// FIXME: need to represent itemSpeed (later spell speed) with empty battle turns
			// is it time to act? (this is to slow down battle, 
			// depending on your weapon)
			//			if(battle[i]->itemSpeed < t - battle[i]->creature->getLastTurn()) {
			  
			  // remember the last active turn
			  battle[i]->creature->setLastTurn(t);
			  
			  if(!battleStarted) {
				scourge->getMap()->addDescription("A round of battle begins...", 1, 1, 1);
				battleStarted = true;
			  }
			  
			  // fight a turn of battle
			  //			  battle[i]->fight();
			  turns->push_back(battle[i]);
			  
			  action = ATTACK;
			  //}
		  }
		  /*
		} else {
		  // out of range, keep following the target
		  battle[i]->creature->setSelXY(battle[i]->creature->getTargetCreature()->getX(),
										battle[i]->creature->getTargetCreature()->getY(),
										true);
		  action = MOVE;
		  */
		}
	  }

	  if(action != NO_ACTION) {
		
#ifdef DEBUG_BATTLE
		cerr << "Turn: " << turn << " " << actionName[action] << ", " <<
		  battle[i]->creature->getName() << "(" << battle[i]->creatureInitiative << ")" <<
		  "->" << (battle[i]->creature->getTargetCreature() ? battle[i]->creature->getTargetCreature()->getName() : "<no target>") <<
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
}

void Battle::fightTurn() {

  // target killed?
  if(!creature->getTargetCreature()) return;

  // too far?
  if(!(dist <= 1.0f || item)) {
	creature->setSelXY(creature->getTargetCreature()->getX(),
					   creature->getTargetCreature()->getY(),
					   true);
	return;
  }

  if(item) {
	sprintf(message, "%s attacks %s with %s! (I:%d,S:%d)", 
			creature->getName(), 
			creature->getTargetCreature()->getName(),
			item->getRpgItem()->getName(),
			creatureInitiative, itemSpeed);
	scourge->getMap()->addDescription(message);
	((MD2Shape*)(creature->getShape()))->setAttackEffect(true);
  } else if(dist <= 1.0f) {
	sprintf(message, "%s attacks %s with bare hands! (I:%d,S:%d)", 
			creature->getName(), 
			creature->getTargetCreature()->getName(),
			creatureInitiative, itemSpeed);
	scourge->getMap()->addDescription(message);
	((MD2Shape*)(creature->getShape()))->setAttackEffect(true);
  }
  
  // the target creature gets really upset...
  // this is also an optimization for fps
  if(creature->getTargetCreature()->isMonster() && 
	 !creature->getTargetCreature()->getTargetCreature()) {
	creature->getTargetCreature()->setMotion(Constants::MOTION_MOVE_TOWARDS);
	creature->getTargetCreature()->setTargetCreature(creature);
  }
  
  // if this is a ranged weapon launch a projectile
  if(item && item->getRpgItem()->isRangedWeapon()) {
	sprintf(message, "...%s shoots a projectile", creature->getName());
	scourge->getMap()->addDescription(message);
	/*
	scourge->newProjectile(creature->getTargetCreature()->getX(),
						   creature->getTargetCreature()->getY(),
						   item);
	*/
  } else {
	hitWithItem();
  }
}

void Battle::hitWithItem() {
  // take a swing
  int tohit = creature->getToHit(item);
  int ac = creature->getTargetCreature()->getSkillModifiedArmor();
  sprintf(message, "...%s defends with armor=%d", creature->getTargetCreature()->getName(), ac);
  scourge->getMap()->addDescription(message);
  if(tohit > ac) {
	
	// deal out the damage
	int damage = creature->getDamage(item);
	if(damage) {
	  sprintf(message, "...and hits! (toHit=%d vs. AC=%d) for %d points of damage", 
			  tohit, ac, damage);
	  scourge->getMap()->addDescription(message, 1.0f, 0.5f, 0.5f);
	  
	  // target creature death
	  if(creature->getTargetCreature()->takeDamage(damage)) {				  
		creature->getShape()->setCurrentAnimation((int)MD2_TAUNT);  
		sprintf(message, "...%s is killed!", creature->getTargetCreature()->getName());
		scourge->getMap()->addDescription(message, 1.0f, 0.5f, 0.5f);
		scourge->creatureDeath(creature->getTargetCreature());
		
		// add exp. points and money
		if(!creature->isMonster()) {
		  for(int i = 0; i < 4; i++) {
			bool b = scourge->getParty(i)->getStateMod(Constants::leveled);
			if(!scourge->getParty(i)->getStateMod(Constants::dead)) {
			  int n = scourge->getParty(i)->addExperience(creature->getTargetCreature());
			  if(n > 0) {
				sprintf(message, "%s gains %d experience points.", scourge->getParty(i)->getName(), n);
				scourge->getMap()->addDescription(message);
				if(!b && scourge->getParty(i)->getStateMod(Constants::leveled)) {
				  sprintf(message, "%s gains a level!", scourge->getParty(i)->getName());
				  scourge->getMap()->addDescription(message, 1.0f, 0.5f, 0.5f);
				}
			  }
			  
			  n = scourge->getParty(i)->addMoney(creature->getTargetCreature());
			  if(n > 0) {
				sprintf(message, "%s finds %d coins!", scourge->getParty(i)->getName(), n);
				scourge->getMap()->addDescription(message);
			  }
			}
		  }
		}
	  }
	} else {
	  sprintf(message, "...and hits! (toHit=%d vs. AC=%d) but causes no damage", 
			  tohit, ac);
	  scourge->getMap()->addDescription(message);
	}
  } else {
	// missed
	sprintf(message, "...and misses! (toHit=%d vs. AC=%d)", tohit, ac);
	scourge->getMap()->addDescription(message);
  }
}

void Battle::selectBestItem() {
  if(item) return;

  item = creature->getBestWeapon(dist);  
  
  // (!item) is a bare-hands attack		
  itemSpeed = (item ? item->getRpgItem()->getSpeed() : Constants::HAND_WEAPON_SPEED) * 
	(scourge->getUserConfiguration()->getGameSpeedTicks() + 80);
  //	(scourge->getUserConfiguration()->getGameSpeedTicks() + 80);

  creatureInitiative = creature->getInitiative(item);
}

