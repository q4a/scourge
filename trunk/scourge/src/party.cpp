/***************************************************************************
                          scourge.cpp  -  description
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

#include "party.h"

Party::Party(Scourge *scourge) {
  this->scourge = scourge;

  // Init the party; hard code for now
  // This will be replaced by a call to the character builder which either
  // loads or creates the party.
  for(int i = 0; i < 4; i++) {
	if(party[i]) {
	  delete party[i];
	}
  }
  Creature **pc = createHardCodedParty();
  party[0] = player = pc[0];
  party[1] = pc[1];
  party[2] = pc[2];
  party[3] = pc[3];	  	  

  Event *e;  
  Date d(0, 0, 6, 0, 0, 0); // 6 hours (format : sec, min, hours, days, months, years)
  for(int i = 0; i < 4 ; i++){
	e = new ThirstHungerEvent(scourge->getCalendar()->getCurrentDate(), d, party[i], scourge, Event::INFINITE_EXECUTIONS);
	scourge->getCalendar()->scheduleEvent((Event*)e);   // It's important to cast!!
  }
}

Party::~Party() {
}

void Party::startPartyOnMission() {
  player_only = false;
  partyDead = false;

  // set player to be the first non-dead character
  for(int i = 0; i < 4; i++) {
	if(!party[i]->getStateMod(Constants::dead)) {
	  setPlayer(getParty(i));
	  break;
	}
  }
  setFormation(Constants::DIAMOND_FORMATION - Constants::DIAMOND_FORMATION);
  getPlayer()->setTargetCreature(NULL);
  
  // init the rest of the party
  for(int i = 1; i < 4; i++) {
	getParty(i)->setNext(getPlayer(), i);
	getParty(i)->setTargetCreature(NULL);
  }
}

void Party::setPartyMotion(int motion) {
  for(int i = 0; i < 4; i++) {
	if(party[i] != player) party[i]->setMotion(motion);
  }
}

void Party::followTargets() {
  for(int i = 0; i < 4; i++) {
	if(!party[i]->getStateMod(Constants::dead) && 
	   party[i]->getTargetCreature()) {
	  party[i]->setSelXY(party[i]->getTargetCreature()->getX(),
						 party[i]->getTargetCreature()->getY());
	}
  }
}

bool Party::switchToNextLivePartyMember() {
	Creature *oldPlayer = player;
	// find the player's index
	int n = -1;
	for(int t = 0; t < 4; t++) {
		if(party[t] == player) {
			n = t;
			break;
		}
	}			
	// switch to next player
	n++; if(n >= 4) n = 0;
	for(int t = 0; t < 4; t++) {
		if(!party[n]->getStateMod(Constants::dead)) {
			setPlayer(n);
			break;
		}
		n++; if(n >= 4) n = 0;
	}
	bool res = (oldPlayer != player);
	if(!res) partyDead = true;
	return res;
}

void Party::setPlayer(int n) {
  player = party[n];
  player->setNextDontMove(NULL, 0);
  //  player->setSelXY(-1, -1); // don't move
  // init the rest of the party
  int count = 1;
  for(int i = 0; i < 4; i++) {
	if(i != n) party[i]->setNextDontMove(player, count++);
  }
}

void Party::setFormation(int formation) {
  this->formation = formation;
  for(int i = 0; i < 4; i++) {
	party[i]->setFormation(formation);
  }
  player_only = false;
}

void Party::togglePlayerOnly() {
  player_only = (player_only ? false : true);
  // in group mode everyone hunts the same creature
  if(!player_only) {
	for(int i = 0; i < 4; i++) {
	  if(party[i] != player) 
		party[i]->setTargetCreature(player->getTargetCreature());
	}
  }
  if(player_only)
	scourge->getMap()->addDescription(Constants::getMessage(Constants::SINGLE_MODE), 0.5f, 0.5f, 1.0f);
  else
	scourge->getMap()->addDescription(Constants::getMessage(Constants::GROUP_MODE), 0.5f, 0.5f, 1.0f);
}

void Party::setTargetCreature(Creature *creature) { 
  if(player_only) player->setTargetCreature(creature);
  else for(int i = 0; i < getPartySize(); i++) party[i]->setTargetCreature(creature); 
}

void Party::movePlayers() {   
  if(player_only) {	
	
	// how many party members are still alive?
	int sum = 0;
	for(int i = 0; i < getPartySize(); i++) 
	  if(!party[i]->getStateMod(Constants::dead)) sum++;
	
	// in single-step mode:
	for(int i = 0; i < sum; i++) {
	  
	  // move the current player
	  if(!player->getStateMod(Constants::dead)) {
		player->moveToLocator(scourge->getMap(), player_only);
		scourge->getMap()->center(player->getX(), player->getY());
	  }
	  
	  switchToNextLivePartyMember();
	}	
  } else {
	// In group mode:
	
	// move the leader
	if(!player->getStateMod(Constants::dead)) {
	  player->moveToLocator(scourge->getMap(), player_only);
	  scourge->getMap()->center(player->getX(), player->getY());
	}
	
	// in keyboard mode, don't move the selection
	//if(move) player->setSelXY(-1, -1);
	
	// others follow the player
	for(int t = 0; t < getPartySize(); t++) {
	  if(!party[t]->getStateMod(Constants::dead) && party[t] != player) {
		if(party[t]->getTargetCreature()) {
		  party[t]->moveToLocator(scourge->getMap(), player_only);
		} else {
		  party[t]->moveToLocator(scourge->getMap(), player_only);
		}
	  }
	}	
  }
}

int Party::getTotalLevel() {
  int totalLevel = 0;
  for(int i = 0; i < scourge->getParty()->getPartySize(); i++) 
	totalLevel += scourge->getParty()->getParty(i)->getLevel();
  return totalLevel;
}

/**
   Create a party programmatically until the party editor is made.
 */
Creature **Party::createHardCodedParty() {
  Creature **pc = (Creature**)malloc(sizeof(Creature*) * 4);

  // FIXME: consider using newCreature here
  // the end of startMission would have to be modified to not delete the party
  // also in scourge, where-ever creatureCount is used to mean all monsters would have
  // to change (maybe that's a good thing too... same logic for party and monsters)
  pc[0] = new Creature(scourge, Character::getCharacterByName("Assassin"), "Alamont");
  pc[0]->setLevel(1); 
  pc[0]->setExp(300);
  pc[0]->setHp();
  pc[0]->setHunger(8);
  pc[0]->setThirst(7); 
  pc[0]->setStateMod(Constants::blessed, true);
  pc[0]->setStateMod(Constants::poisoned, true);

  pc[1] = new Creature(scourge, Character::getCharacterByName("Knight"), "Barlett");
  pc[1]->setLevel(1); 
  pc[1]->setExp(200);
  pc[1]->setHp();
  pc[1]->setHunger(10);
  pc[1]->setThirst(9);
  pc[1]->setStateMod(Constants::drunk, true);
  pc[1]->setStateMod(Constants::cursed, true);      

  pc[2] = new Creature(scourge, Character::getCharacterByName("Summoner"), "Corinus");
  pc[2]->setLevel(1); 
  pc[2]->setExp(150);
  pc[2]->setHp();
  pc[2]->setHunger(3);
  pc[2]->setThirst(2);
  pc[2]->setStateMod(Constants::ac_protected, true);
  pc[2]->setStateMod(Constants::magic_protected, true);
  pc[2]->setStateMod(Constants::cursed, true);
  //  for(int i = 0; i < Constants::STATE_MOD_COUNT; i++) 
  //  	if(i != Constants::dead) pc[2]->setStateMod(i, true);

  pc[3] = new Creature(scourge, Character::getCharacterByName("Naturalist"), "Dialante");    
  pc[3]->setLevel(1); 
  pc[3]->setExp(400);
  pc[3]->setHp();
  pc[3]->setHunger(10);
  pc[3]->setThirst(10);
  pc[3]->setStateMod(Constants::possessed, true);          
  
  // compute starting skill levels
  for(int i = 0; i < 4; i++) {
	for(int skill = 0; skill < Constants::SKILL_COUNT; skill++) {
	  int n = pc[i]->getCharacter()->getMinSkillLevel(skill) + (int)(20.0 * rand()/RAND_MAX);
	  if(n > pc[i]->getCharacter()->getMaxSkillLevel(skill)) 
		n = pc[i]->getCharacter()->getMaxSkillLevel(skill);
	  pc[i]->setSkill(skill, n);
	}
  }

  // Compute the new maxInventoryWeight for each pc, according to its POWER skill
  for(int i = 0; i < 4; i++) {
	pc[i]->setMaxInventoryWeight(pc[i]->computeMaxInventoryWeight());
  }

  // add some items
  pc[0]->addInventory(scourge->newItem(RpgItem::getItemByName("Bastard sword")));
  pc[0]->addInventory(scourge->newItem(RpgItem::getItemByName("Horned helmet")));
  pc[0]->addInventory(scourge->newItem(RpgItem::getItemByName("Dagger")));
  pc[1]->addInventory(scourge->newItem(RpgItem::getItemByName("Smallbow")));
  pc[1]->addInventory(scourge->newItem(RpgItem::getItemByName("Apple")));
  pc[1]->addInventory(scourge->newItem(RpgItem::getItemByName("Apple")));
  pc[2]->addInventory(scourge->newItem(RpgItem::getItemByName("Long sword")));
  pc[2]->addInventory(scourge->newItem(RpgItem::getItemByName("Wine barrel")));
  pc[2]->addInventory(scourge->newItem(RpgItem::getItemByName("Mutton meat")));
  pc[3]->addInventory(scourge->newItem(RpgItem::getItemByName("Great sword")));
  pc[3]->addInventory(scourge->newItem(RpgItem::getItemByName("Battleaxe")));
  pc[3]->addInventory(scourge->newItem(RpgItem::getItemByName("Throwing axe")));  

  return pc;
}
