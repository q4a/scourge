/***************************************************************************
                          party.cpp  -  description
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

Party::Party(Session *session) {
  this->session = session;

  startRound = true;
  calendar = Calendar::getInstance();

  partySize = 0;
}

Party::~Party() {
  delete calendar;
  deleteParty();
}

void Party::deleteParty() {
  for(int i = 0; i < getPartySize(); i++) {
    if(party[i]) {
      delete party[i];
      party[i] = NULL;
    }
  }  
}

void Party::reset() {
  // Init the party; hard code for now                 
  // This will be replaced by a call to the character builder which either
  // loads or creates the party.
  deleteParty();
  Creature **pc;
  int pcCount;
  createHardCodedParty(session, &pc, &pcCount);
  party[0] = player = pc[0];
  party[1] = pc[1];
  party[2] = pc[2];
  party[3] = pc[3];
  partySize = pcCount;
  session->getGameAdapter()->resetPartyUI();
}

void Party::resetMultiplayer(Creature *c) {
  deleteParty();
  party[0] = player = c;
  party[1] = party[2] = party[3] = NULL;
  partySize = 1;
#ifdef HAVE_SDL_NET
  // upload your character to the server
  session->getClient()->sendCharacter(player->save());
#endif
  session->getGameAdapter()->resetPartyUI();
}

// set player to be the first non-dead character
void Party::setFirstLivePlayer() {
  for(int i = 0; i < getPartySize(); i++) {
    if(!party[i]->getStateMod(Constants::dead)) {
      setPlayer(getParty(i));
      break;
    }
  }
}

void Party::startPartyOnMission() {
  // Start calendar and add thirst & hunger event scheduling
  calendar->reset(false);

  player_only = false;
  partyDead = false;
  
  setFirstLivePlayer();
  setFormation(Constants::DIAMOND_FORMATION - Constants::DIAMOND_FORMATION);
  getPlayer()->cancelTarget();
  
  // init the rest of the party
  for(int i = 1; i < getPartySize(); i++) {
	getParty(i)->setNext(getPlayer(), i);
	getParty(i)->cancelTarget();
  }
}

void Party::setPartyMotion(int motion) {
  for(int i = 0; i < getPartySize(); i++) {
	if(party[i] != player) party[i]->setMotion(motion);
  }
}

void Party::followTargets() {
  for(int i = 0; i < getPartySize(); i++) {
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
	for(int t = 0; t < getPartySize(); t++) {
		if(party[t] == player) {
			n = t;
			break;
		}
	}			
	// switch to next player
	n++; if(n >= getPartySize()) n = 0;
	for(int t = 0; t < getPartySize(); t++) {
		if(!party[n]->getStateMod(Constants::dead)) {
			setPlayer(n);
			break;
		}
		n++; if(n >= getPartySize()) n = 0;
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
  for(int i = 0; i < getPartySize(); i++) {
    if(i != n) party[i]->setNextDontMove(player, count++);
  }

  //  move = 0;
  session->getMap()->refresh();
  session->getMap()->center(getPlayer()->getX(), getPlayer()->getY());
  
  session->getMap()->center(player->getX(), player->getY(), true);
  session->getGameAdapter()->refreshInventoryUI(n);  
  session->getGameAdapter()->setPlayerUI(n);
}


void Party::forceStopRound() {
  startRound = false;
  toggleRound();
}

void Party::toggleRound(bool test) {
  if(startRound == test) toggleRound();
}

void Party::toggleRound() {
  startRound = (startRound ? false : true);
  if(startRound){
    session->getMap()->addDescription(Constants::getMessage(Constants::REAL_TIME_MODE), 0.5f, 0.5f, 1.0f);
  }
  else{
    session->getMap()->addDescription(Constants::getMessage(Constants::TURN_MODE), 0.5f, 0.5f, 1.0f);    
  }
   
  // Freeze / unfreeze calendar
  calendar->setPause(!startRound); 
  
  // Freeze / unfreeze animations
  for(int i = 0; i < getPartySize(); i++){
    getParty(i)->getShape()->setPauseAnimation(!startRound);
  }  
  for(int i = 0; i < session->getCreatureCount(); i++){
    session->getCreature(i)->getShape()->setPauseAnimation(!startRound);
  } 

  session->getGameAdapter()->toggleRoundUI(startRound);  
}

void Party::setTargetCreature(Creature *creature) { 
  if(player_only) {
    player->setTargetCreature(creature);
  } else {
    for(int i = 0; i < getPartySize(); i++) {
      party[i]->setTargetCreature(creature); 
    }
  }
}

void Party::movePlayers() {   
  if(player_only) {	
	// move everyone
	for(int i = 0; i < getPartySize(); i++) {
	  if(!party[i]->getStateMod(Constants::dead)) {
		party[i]->moveToLocator(session->getMap());
	  }
	}
	// center on player
	session->getMap()->center(player->getX(), player->getY());
  } else {
	// In group mode:
	
	// move the leader
	if(!player->getStateMod(Constants::dead)) {
	  player->moveToLocator(session->getMap());
	  session->getMap()->center(player->getX(), player->getY());
	}
	
	// others follow the player
	for(int t = 0; t < getPartySize(); t++) {
	  if(!party[t]->getStateMod(Constants::dead) && party[t] != player) {
		party[t]->moveToLocator(session->getMap());
	  }
	}	
  }
}

int Party::getTotalLevel() {
  int totalLevel = 0;
  for(int i = 0; i < getPartySize(); i++) 
	totalLevel += getParty(i)->getLevel();
  return totalLevel;
}

/**
   Create a party programmatically until the party editor is made.
 */
void Party::createHardCodedParty(Session *session, Creature ***party, int *partySize) {
  int pcCount = 4;
  Creature **pc = (Creature**)malloc(sizeof(Creature*) * pcCount);

  int level = 6;

  // FIXME: consider using newCreature here
  // the end of startMission would have to be modified to not delete the party
  // also in scourge, where-ever creatureCount is used to mean all monsters would have
  // to change (maybe that's a good thing too... same logic for party and monsters)
  pc[0] = new Creature(session, Character::getCharacterByName("Assassin"), "Alamont");
  pc[0]->setLevel(level); 
  pc[0]->setExp();
  pc[0]->setHp();
  pc[0]->setMp();
  pc[0]->setHunger(8);
  pc[0]->setThirst(7); 
  pc[0]->setStateMod(Constants::blessed, true);
  pc[0]->setStateMod(Constants::poisoned, true);

  pc[1] = new Creature(session, Character::getCharacterByName("Knight"), "Barlett");
  pc[1]->setLevel(level); 
  pc[1]->setExp();
  pc[1]->setHp();
  pc[1]->setMp();
  pc[1]->setHunger(10);
  pc[1]->setThirst(9);
  pc[1]->setStateMod(Constants::drunk, true);
  pc[1]->setStateMod(Constants::cursed, true);      

  pc[2] = new Creature(session, Character::getCharacterByName("Summoner"), "Corinus");
  pc[2]->setLevel(level); 
  pc[2]->setExp();
  pc[2]->setHp();
  pc[2]->setMp();
  pc[2]->setHunger(3);
  pc[2]->setThirst(2);
  pc[2]->setStateMod(Constants::ac_protected, true);
  pc[2]->setStateMod(Constants::magic_protected, true);
  pc[2]->setStateMod(Constants::cursed, true);
  //  for(int i = 0; i < Constants::STATE_MOD_COUNT; i++) 
  //  	if(i != Constants::dead) pc[2]->setStateMod(i, true);

  pc[3] = new Creature(session, Character::getCharacterByName("Naturalist"), "Dialante");    
  pc[3]->setLevel(level); 
  pc[3]->setExp();
  pc[3]->setHp();
  pc[3]->setMp();
  pc[3]->setHunger(10);
  pc[3]->setThirst(10);
  pc[3]->setStateMod(Constants::possessed, true);          
  
  // compute starting skill levels
  for(int i = 0; i < pcCount; i++) {
	for(int skill = 0; skill < Constants::SKILL_COUNT; skill++) {
	  int n = pc[i]->getCharacter()->getMinSkillLevel(skill) + level * (int)(10.0 * rand()/RAND_MAX);
    if(n > 99) n = 99;
    if(n > pc[i]->getCharacter()->getMaxSkillLevel(skill)) 
      n = pc[i]->getCharacter()->getMaxSkillLevel(skill);
	  pc[i]->setSkill(skill, n);
	}
  }

  // add some items
  pc[0]->addInventory(session->newItem(RpgItem::getItemByName("Bastard sword")));
  pc[0]->addInventory(session->newItem(RpgItem::getItemByName("Horned helmet")));
  pc[0]->addInventory(session->newItem(RpgItem::getItemByName("Dagger")));
  pc[0]->addInventory(session->newItem(RpgItem::getItemByName("Minor health potion")));  
  pc[0]->addInventory(session->newItem(RpgItem::getItemByName("Minor health potion")));  
  pc[0]->addInventory(session->newItem(RpgItem::getItemByName("Liquid armor")));  
  pc[0]->addInventory(session->newItem(RpgItem::getItemByName("Potion of Speed")));
  pc[0]->addInventory(session->newItem(RpgItem::getItemByName("Potion of Coordination")));
  pc[0]->addInventory(session->newItem(RpgItem::getItemByName("Potion of Power")));
  pc[0]->addInventory(session->newItem(RpgItem::getItemByName("Potion of IQ")));
  pc[0]->addInventory(session->newItem(RpgItem::getItemByName("Potion of Leadership")));
  pc[0]->addInventory(session->newItem(RpgItem::getItemByName("Potion of Luck")));
  pc[0]->addInventory(session->newItem(RpgItem::getItemByName("Potion of Piety")));
  pc[0]->addInventory(session->newItem(RpgItem::getItemByName("Potion of Lore")));
  pc[0]->addInventory(session->newItem(RpgItem::getItemByName("Minor magic potion")));  
  pc[0]->addInventory(session->newItem(RpgItem::getItemByName("Minor magic potion")));  

  pc[1]->addInventory(session->newItem(RpgItem::getItemByName("Smallbow")));
  pc[1]->addInventory(session->newItem(RpgItem::getItemByName("Apple")));
  pc[1]->addInventory(session->newItem(RpgItem::getItemByName("Bread")));
  pc[1]->addInventory(session->newItem(RpgItem::getItemByName("Mushroom")));
  pc[1]->addInventory(session->newItem(RpgItem::getItemByName("Big egg")));
  pc[1]->addInventory(session->newItem(RpgItem::getItemByName("Mutton meat")));
  pc[1]->addInventory(session->newItem(RpgItem::getItemByName("Minor health potion")));  
  pc[1]->addInventory(session->newItem(RpgItem::getItemByName("Minor magic potion")));  
  pc[1]->addInventory(session->newItem(RpgItem::getItemByName("Liquid armor")));  

  pc[2]->addInventory(session->newItem(RpgItem::getItemByName("Dagger")));
  pc[2]->addInventory(session->newItem(RpgItem::getItemByName("Smallbow")));
  pc[2]->addInventory(session->newItem(RpgItem::getItemByName("Long sword")));
  pc[2]->addInventory(session->newItem(RpgItem::getItemByName("Wine barrel")));
  pc[2]->addInventory(session->newItem(RpgItem::getItemByName("Mutton meat")));
  pc[2]->addInventory(session->newItem(RpgItem::getItemByName("Minor health potion")));  
  pc[2]->addInventory(session->newItem(RpgItem::getItemByName("Minor health potion")));  
  pc[2]->addInventory(session->newItem(RpgItem::getItemByName("Minor magic potion")));  
  pc[2]->addInventory(session->newItem(RpgItem::getItemByName("Minor magic potion")));  
  pc[2]->addInventory(session->newItem(RpgItem::getItemByName("Liquid armor")));   

  // add some scrolls
  for(int i = 0; i < 10; i++) {
    Spell *spell = MagicSchool::getRandomSpell(1);
    if(spell) {
      Item *scroll = session->newItem(RpgItem::getItemByName("Scroll"));
      scroll->setSpell(spell);
      pc[2]->addInventory(scroll);  
    }
  }
  pc[2]->setMp(50);

  pc[3]->addInventory(session->newItem(RpgItem::getItemByName("Dagger")));
  pc[3]->addInventory(session->newItem(RpgItem::getItemByName("Great sword")));
  pc[3]->addInventory(session->newItem(RpgItem::getItemByName("Battleaxe")));
  pc[3]->addInventory(session->newItem(RpgItem::getItemByName("Throwing axe")));  
  pc[3]->addInventory(session->newItem(RpgItem::getItemByName("Minor health potion")));  
  pc[3]->addInventory(session->newItem(RpgItem::getItemByName("Minor health potion")));  
  pc[3]->addInventory(session->newItem(RpgItem::getItemByName("Minor health potion")));  
  pc[3]->addInventory(session->newItem(RpgItem::getItemByName("Minor magic potion")));  
  pc[3]->addInventory(session->newItem(RpgItem::getItemByName("Minor magic potion")));  
  pc[3]->addInventory(session->newItem(RpgItem::getItemByName("Liquid armor")));  
  
  // equip weapons
  pc[0]->equipInventory(0);
  pc[0]->equipInventory(1);
  pc[1]->equipInventory(0);
  pc[2]->equipInventory(0);
  pc[2]->equipInventory(1);
  pc[3]->equipInventory(0);

  // add some spells
  pc[2]->addSpell(Spell::getSpellByName("Flame of Azun"));
  pc[2]->addSpell(Spell::getSpellByName("Ole Taffy's purty colors"));
  pc[2]->addSpell(Spell::getSpellByName("Silent knives"));
  pc[2]->addSpell(Spell::getSpellByName("Stinging light"));
  pc[2]->addSpell(Spell::getSpellByName("Burning stare"));

  pc[3]->addSpell(Spell::getSpellByName("Lesser healing touch"));
  pc[3]->addSpell(Spell::getSpellByName("Body of stone"));
				  
  *party = pc;
  *partySize = pcCount;  
}

/** 
	Return the closest live player within the given radius or null if none can be found.
*/
Creature *Party::getClosestPlayer(int x, int y, int w, int h, int radius) {
  float minDist = 0;
  Creature *p = NULL;
  for(int i = 0; i < getPartySize(); i++) {
	if(!party[i]->getStateMod(Constants::dead)) {
	  float dist = Constants::distance(x, y, w, h,
									   party[i]->getX(),
									   party[i]->getY(),
									   party[i]->getShape()->getWidth(),
									   party[i]->getShape()->getDepth());
	  if(dist <= (float)radius &&
		 (!p || dist < minDist)) {
		p = party[i];
		minDist = dist;
	  }
	}
  }
  return p;
}

void Party::startEffect(int effect_type, int duration) {
  for(int i = 0; i < getPartySize(); i++) {
	if(!party[i]->getStateMod(Constants::dead)) {
	  party[i]->startEffect(effect_type, duration);
	}
  }
}

void Party::setFormation(int formation) {
  this->formation = formation;
  for(int i = 0; i < getPartySize(); i++) {
    getParty(i)->setFormation(formation);
  }
  player_only = false;
  startRound = true;
  session->getGameAdapter()->setFormationUI(formation, !isPlayerOnly());
}

void Party::togglePlayerOnly(bool keepTargets) {
  player_only = (player_only ? false : true);
  // in group mode everyone hunts the same creature
  if(!player_only && !keepTargets) {
    for(int i = 0; i < getPartySize(); i++) {
      if(party[i] != player) 
        party[i]->setTargetCreature(player->getTargetCreature());
    }
  }
  if(player_only)
    session->getMap()->addDescription(Constants::getMessage(Constants::SINGLE_MODE), 0.5f, 0.5f, 1.0f);
  else
    session->getMap()->addDescription(Constants::getMessage(Constants::GROUP_MODE), 0.5f, 0.5f, 1.0f);
  session->getGameAdapter()->togglePlayerOnlyUI(!isPlayerOnly());
}

