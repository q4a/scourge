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

#define DEBUG_BATTLE
#define GOD_MODE 0

enum {
  NO_ACTION = 0,
  LOITER,
  ATTACK,
  MOVE,
  NO_TARGET,
  WAIT,
};

char *actionName[] = { "NO ACTION", "LOITER", "ATTACK", "MOVE", "NO_TARGET", "WAIT" };

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
  this->dist = creature->getDistanceToTargetCreature();
}

Battle::~Battle() {
}

void Battle::setupBattles(Scourge *scourge, Battle *battle[], int count, vector<Battle *> *turns) {

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

	  if(!battle[i]->creature->getTargetCreature()) {
		// remove creatures with no target creature from this round
		action = NO_TARGET;
	  } else if(battle[i]->creature->getTargetCreature()->getStateMod(Constants::dead)) {
		// if someone already killed this target, skip it
		battle[i]->creature->setTargetCreature(NULL);
		if(battle[i]->creature->isMonster()) {
		  battle[i]->creature->setMotion(Constants::MOTION_LOITER);
		  battle[i]->creature->setDistanceRange(0, 0);

		  action = LOITER;
		} else {
		  // FIXME: should help out another party member
		}
	  } else {

		GLint t = SDL_GetTicks();

		// get the best weapon given the distance from the target
		if(battle[i]->creature->getAction() > -1) {
		  battle[i]->initAction();
		} else {
		  battle[i]->selectBestItem();
		}
		
		// check the creature's initiative
		if(!battle[i]->initiativeCheck || 
		   battle[i]->creatureInitiative < initiative) {
		  
		  // remember that it passed (creature can attack next time if timing (below) is good)
		  battle[i]->initiativeCheck = true;
		  creatureFound = true;
		  
		  // this is to slow down battle, depending on your weapon
		  if(battle[i]->itemSpeed < t - battle[i]->creature->getLastTurn()) {
			
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
		  } else {
			// add a waiting term
			battle[i]->empty = true;
			action = WAIT;
		  }
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

void Battle::projectileHitTurn(Scourge *scourge, Projectile *proj, Creature *target) {
  // configure a turn
  Creature *oldTarget = proj->getCreature()->getTargetCreature();
  proj->getCreature()->setTargetCreature(target);
  Battle *battle = new Battle(scourge, proj->getCreature());
  battle->projectileHit = true;
  battle->initItem(proj->getItem());

  // play it
  battle->fightTurn();
  
  delete battle;
  proj->getCreature()->setTargetCreature(oldTarget);
}

void Battle::fightTurn() {

  // waiting to attack?
  if(isEmpty()) return;

  // target killed?
  if(!creature->getTargetCreature()) return;

  // too far? then keep following the target
  // also check if too close when using ranged weapons
  if((dist > Constants::MIN_DISTANCE && !item && !creature->getActionSpell()) ||  
	 !creature->isInRange()) { 
	creature->setSelXY(creature->getTargetCreature()->getX(),
					   creature->getTargetCreature()->getY(),
					   true);
	return;
  }

  // handle action: eat/drink items
  if(creature->getAction() == Constants::ACTION_EAT_DRINK) {
	// is it still in the inventory?
	int index = creature->findInInventory(creature->getActionItem());
	if(index > -1) {
	  if(creature->eatDrink(creature->getActionItem())){
		creature->removeInventory(index);
	  }
	}
	// cancel action
	creature->setTargetCreature(NULL);
	creature->setAction(-1);
	return;
  }
 
  // print a message and show an effect
  if(creature->getActionSpell()) {
	sprintf(message, "%s casts %s!", 
			creature->getName(), 
			creature->getActionSpell()->getName());
	scourge->getMap()->addDescription(message, 1, 0.15f, 1);
	((MD2Shape*)(creature->getShape()))->setAttackEffect(true);
  } else if(item) {
	sprintf(message, "%s attacks %s with %s! (I:%d,S:%d)", 
			creature->getName(), 
			creature->getTargetCreature()->getName(),
			item->getRpgItem()->getName(),
			creatureInitiative, itemSpeed);
	scourge->getMap()->addDescription(message);
	((MD2Shape*)(creature->getShape()))->setAttackEffect(true);
  } else if(dist <= Constants::MIN_DISTANCE) {
	sprintf(message, "%s attacks %s with bare hands! (I:%d,S:%d)", 
			creature->getName(), 
			creature->getTargetCreature()->getName(),
			creatureInitiative, itemSpeed);
	scourge->getMap()->addDescription(message);
	((MD2Shape*)(creature->getShape()))->setAttackEffect(true);
  }
 
  // FIXME: what about multiple targets for area spells?
  // the target creature gets really upset...
  // this is also an optimization for fps
  if(creature->getTargetCreature()->isMonster() && 
	 !creature->getTargetCreature()->getTargetCreature()) {
	// try to attack the nearest player
	Creature *p = scourge->getParty()->getClosestPlayer(creature->getTargetCreature()->getX(), 
														creature->getTargetCreature()->getY(), 
														creature->getTargetCreature()->getShape()->getWidth(),
														creature->getTargetCreature()->getShape()->getDepth(),
														20);
	// if that's not possible, go for the attacker
	if(!p) p = creature;
	sprintf(message, "...%s is enraged and attacks %s", 
			creature->getTargetCreature()->getName(), 
			creature->getName());
	scourge->getMap()->addDescription(message);	
	creature->getTargetCreature()->setMotion(Constants::MOTION_MOVE_TOWARDS);
	creature->getTargetCreature()->setTargetCreature(p);
  }
  
  // if this is a ranged weapon launch a projectile
  if(!projectileHit && item && item->getRpgItem()->isRangedWeapon()) {
	sprintf(message, "...%s shoots a projectile", creature->getName());
	scourge->getMap()->addDescription(message);	
	if(!Projectile::addProjectile(creature, creature->getTargetCreature(), item, 
								  scourge->getShapePalette()->findShapeByName("ARROW"),
								  creature->getMaxProjectileCount(item))) {
	  // max number of projectiles in the air
	}
  } else if(creature->getActionSpell()) {
	castSpell();
  } else {
	hitWithItem();
  }
}

void Battle::castSpell() {
  cerr << "FIXME: implement Battle::castSpell()" << endl;
  cerr << "\tdynamic effects (flame, explode, etc.), saving throws, multiple targets, projectile hits, mana usage, etc." << endl;
  cerr << "\tcall Battle::dealDamage() for each target." << endl;

  
  // cancel action
  creature->setTargetCreature(NULL);
  creature->setAction(-1);  
}

void Battle::hitWithItem() {
  // take a swing
  int tohit = creature->getToHit(item);
  int ac = creature->getTargetCreature()->getSkillModifiedArmor();
  sprintf(message, "...%s defends with armor=%d", creature->getTargetCreature()->getName(), ac);
  scourge->getMap()->addDescription(message);
  sprintf(message, "...toHit=%d vs. AC=%d", tohit, ac);
  scourge->getMap()->addDescription(message);
  if(tohit > ac) {
	// deal out the damage
	dealDamage(creature->getDamage(item));
  } else {
	// missed
	sprintf(message, "...and misses! (toHit=%d vs. AC=%d)", tohit, ac);
	scourge->getMap()->addDescription(message);
  }
}

void Battle::dealDamage(int damage) {
  if(damage) {	
	sprintf(message, "...and hits! for %d points of damage", damage);
	scourge->getMap()->addDescription(message, 1.0f, 0.5f, 0.5f);
	
	// target creature death
	if(creature->getTargetCreature()->takeDamage(damage)) {				  
	  creature->getShape()->setCurrentAnimation((int)MD2_TAUNT);  
	  sprintf(message, "...%s is killed!", creature->getTargetCreature()->getName());
	  scourge->getMap()->addDescription(message, 1.0f, 0.5f, 0.5f);
	  
	  if(creature->getTargetCreature()->isMonster() || !GOD_MODE)
		scourge->creatureDeath(creature->getTargetCreature());
	  
	  // add exp. points and money
	  if(!creature->isMonster()) {
		
		
		// FIXME: try to move to party.cpp
		for(int i = 0; i < scourge->getParty()->getPartySize(); i++) {
		  bool b = scourge->getParty()->getParty(i)->getStateMod(Constants::leveled);
		  if(!scourge->getParty()->getParty(i)->getStateMod(Constants::dead)) {
			int n = scourge->getParty()->getParty(i)->addExperience(creature->getTargetCreature());
			if(n > 0) {
			  sprintf(message, "%s gains %d experience points.", scourge->getParty()->getParty(i)->getName(), n);
			  scourge->getMap()->addDescription(message);
			  if(!b && scourge->getParty()->getParty(i)->getStateMod(Constants::leveled)) {
				sprintf(message, "%s gains a level!", scourge->getParty()->getParty(i)->getName());
				scourge->getMap()->addDescription(message, 1.0f, 0.5f, 0.5f);
			  }
			}
			
			n = scourge->getParty()->getParty(i)->addMoney(creature->getTargetCreature());
			if(n > 0) {
			  sprintf(message, "%s finds %d coins!", scourge->getParty()->getParty(i)->getName(), n);
			  scourge->getMap()->addDescription(message);
			}
		  }
		}
		// end of FIXME
		
		// see if this is a mission objective
		if(scourge->getCurrentMission() && 
		   creature->getTargetCreature()->getMonster() &&
		   scourge->getCurrentMission()->monsterSlain(creature->getTargetCreature()->getMonster())) {
		  scourge->missionCompleted();
		}
	  }
	}
  } else {
	sprintf(message, "...and hits! but causes no damage");
	scourge->getMap()->addDescription(message);
  }
}

void Battle::initAction() {
  if(creature->getAction() == -1) return;
  
  float range = 0;
  switch(creature->getAction()) {
  case Constants::ACTION_CAST_SPELL:
	range = creature->getActionSpell()->getDistance();
	itemSpeed = creature->getActionSpell()->getLevel() * Constants::HAND_WEAPON_SPEED * 2;
	creatureInitiative = creature->getInitiative(NULL, creature->getActionSpell());
	break;
  case Constants::ACTION_EAT_DRINK:
	range = creature->getActionItem()->getRpgItem()->getDistance();
	itemSpeed = creature->getActionItem()->getRpgItem()->getSpeed();
	creatureInitiative = creature->getInitiative(creature->getActionItem(), NULL);
	break;
  default:
	cerr << "*** Error: unhandled action: " << creature->getAction() << endl;
  }
  // set up the range
  creature->setDistanceRange(0, Constants::MIN_DISTANCE);
  if(range >= 8) {
	creature->setDistanceRange(range * 0.5f, range);
  }
}

void Battle::selectBestItem() {
  if(item) return;
  Item *i = creature->getBestWeapon(dist);

  // set up distance range for ranged weapons
  creature->setDistanceRange(0, Constants::MIN_DISTANCE);
  if(i) {
	float range = i->getRpgItem()->getDistance();
	if(range >= 8) {
	  creature->setDistanceRange(range * 0.5f, range);
	}
  }

  initItem(i);
}

void Battle::initItem(Item *item) {
  this->item = item;
  
  // (!item) is a bare-hands attack		
  itemSpeed = (item ? item->getRpgItem()->getSpeed() : Constants::HAND_WEAPON_SPEED) * 
	(scourge->getUserConfiguration()->getGameSpeedTicks() + 80);
  //	(scourge->getUserConfiguration()->getGameSpeedTicks() + 80);

  creatureInitiative = creature->getInitiative(item);
}

