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
#include "rpg/rpgitem.h"

Creature *Party::lastPlayer = NULL;

#define RANDOM_PARTY 1

Party::Party(Session *session) {
  this->session = session;

  startRound = true;
  calendar = Calendar::getInstance();

  partySize = 0;

  loadedCount = 0;
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
  if(loadedCount) {
    cerr << "*** Using loaded party!" << endl;
    for(int i = 0; i < loadedCount; i++) {
      party[i] = loadedParty[i];
    }
    partySize = loadedCount;
    loadedCount = 0;
  } else {
    cerr << "*** Creating new party!" << endl;
//    Creature *pc[MAX_PARTY_SIZE];
//    int pcCount;
    createHardCodedParty(session, party, &partySize);  
  }
  player = party[0];
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
      //setPlayer(getParty(i));
      setPlayer(i);
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

  // play selection sound
  if(lastPlayer != player) {
    lastPlayer = player;
    if(!player->getStateMod(Constants::dead))
      session->playSound(player->getCharacter()->getRandomSound(Constants::SOUND_TYPE_SELECT));
  }
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
void Party::createHardCodedParty(Session *session, Creature **pc, int *partySize) {
  int pcCount = 4;

#ifdef RANDOM_PARTY

  int level = 1;
  char names[4][80] = { "Alamont", "Barlett", "Corinus", "Dialante" };

  for(int i = 0; i < pcCount; i++) {
    // Get a good mix of classes (with different graphics)
    Character *c;
    switch(i) {
    case 0:
    c = Character::getCharacterByName("Knight"); // always knight: it has sound... ;-)
    break;
    case 1:
    if(0 == (int)(2.0f * rand()/RAND_MAX))
      c = Character::getCharacterByName("Tinkerer");
    else
      c = Character::getCharacterByName("Assassin");
    break;
    case 2:
    if(0 == (int)(2.0f * rand()/RAND_MAX))
      c = Character::getCharacterByName("Conjurer");
    else
      c = Character::getCharacterByName("Summoner");
    break;
    default:
    if(0 == (int)(2.0f * rand()/RAND_MAX))
      c = Character::getCharacterByName("Naturalist");
    else
      c = Character::getCharacterByName("Monk");
    }
    pc[i] = new Creature(session, c, strdup(names[i]));
    pc[i]->setLevel(level); 
    pc[i]->setExp(0);
    pc[i]->setHp();
    pc[i]->setMp();
    pc[i]->setHunger((int)(5.0f * rand()/RAND_MAX) + 5);
    pc[i]->setThirst((int)(5.0f * rand()/RAND_MAX) + 5); 

    // compute starting skill levels
    for(int skill = 0; skill < Constants::SKILL_COUNT; skill++) {
      int n = pc[i]->getCharacter()->getMinSkillLevel(skill) + level * (int)(10.0 * rand()/RAND_MAX);
      // basic skills
      if(skill < Constants::SWORD_WEAPON) n = 20 + level * (int)(10.0 * rand()/RAND_MAX);
      if(n > 99) n = 99;
      if(n > pc[i]->getCharacter()->getMaxSkillLevel(skill)) 
        n = pc[i]->getCharacter()->getMaxSkillLevel(skill);
      pc[i]->setSkill(skill, n);
    }

    // add a weapon anyone can wield
    int n = (int)(3.0f * rand()/RAND_MAX);
    switch(n) {
    case 0: pc[i]->addInventory(session->newItem(RpgItem::getItemByName("Smallbow"))); break;
    case 1: pc[i]->addInventory(session->newItem(RpgItem::getItemByName("Short sword"))); break;
    case 2: pc[i]->addInventory(session->newItem(RpgItem::getItemByName("Dagger"))); break;
    }
    pc[i]->equipInventory(0);

    // add some armor
    if(0 == (int)(4.0f * rand()/RAND_MAX)) {
      pc[i]->addInventory(session->newItem(RpgItem::getItemByName("Horned helmet")));
      pc[i]->equipInventory(1);
    }

    // some potions
    if(0 == (int)(4.0f * rand()/RAND_MAX))
      pc[i]->addInventory(session->newItem(RpgItem::getItemByName("Minor health potion")));  
    if(0 == (int)(4.0f * rand()/RAND_MAX))
      pc[i]->addInventory(session->newItem(RpgItem::getItemByName("Minor magic potion")));  
    if(0 == (int)(4.0f * rand()/RAND_MAX))
      pc[i]->addInventory(session->newItem(RpgItem::getItemByName("Liquid armor")));  

    // some food
    for(int t = 0; t < (int)(6.0f * rand()/RAND_MAX); t++) {
      if(0 == (int)(4.0f * rand()/RAND_MAX))
        pc[i]->addInventory(session->newItem(RpgItem::getItemByName("Apple")));
      if(0 == (int)(4.0f * rand()/RAND_MAX))
        pc[i]->addInventory(session->newItem(RpgItem::getItemByName("Bread")));
      if(0 == (int)(4.0f * rand()/RAND_MAX))
        pc[i]->addInventory(session->newItem(RpgItem::getItemByName("Mushroom")));
      if(0 == (int)(4.0f * rand()/RAND_MAX))
        pc[i]->addInventory(session->newItem(RpgItem::getItemByName("Big egg")));
      if(0 == (int)(4.0f * rand()/RAND_MAX))
        pc[i]->addInventory(session->newItem(RpgItem::getItemByName("Mutton meat")));
    }

    // some spells
    if(pc[i]->getMaxMp() > 0) {
      // useful spells
      pc[i]->addSpell(Spell::getSpellByName("Flame of Azun"));
      pc[i]->addSpell(Spell::getSpellByName("Ole Taffy's purty colors"));
      // attack spell
      if(0 == (int)(2.0f * rand()/RAND_MAX))
        pc[i]->addSpell(Spell::getSpellByName("Silent knives"));
      else
        pc[i]->addSpell(Spell::getSpellByName("Stinging light"));
      // defensive spell
      if(0 == (int)(2.0f * rand()/RAND_MAX))
        pc[i]->addSpell(Spell::getSpellByName("Lesser healing touch"));
      else
        pc[i]->addSpell(Spell::getSpellByName("Body of stone"));
    }
  }
  
#else

  int level = 10;

  // FIXME: consider using newCreature here
  // the end of startMission would have to be modified to not delete the party
  // also in scourge, where-ever creatureCount is used to mean all monsters would have
  // to change (maybe that's a good thing too... same logic for party and monsters)
  pc[0] = new Creature(session, Character::getCharacterByName("Assassin"), strdup("Alamont"));
  pc[0]->setLevel(level); 
  pc[0]->setExp();
  pc[0]->setHp();
  pc[0]->setMp();
  pc[0]->setHunger(8);
  pc[0]->setThirst(7); 
  pc[0]->setStateMod(Constants::blessed, true);

  pc[1] = new Creature(session, Character::getCharacterByName("Knight"), strdup("Barlett"));
  pc[1]->setLevel(level); 
  pc[1]->setExp();
  pc[1]->setHp();
  pc[1]->setMp();
  pc[1]->setHunger(10);
  pc[1]->setThirst(9);
  pc[1]->setStateMod(Constants::drunk, true);
  pc[1]->setStateMod(Constants::cursed, true);      

  pc[2] = new Creature(session, Character::getCharacterByName("Summoner"), strdup("Corinus"));
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

  pc[3] = new Creature(session, Character::getCharacterByName("Naturalist"), strdup("Dialante"));    
  pc[3]->setLevel(level); 
  pc[3]->setExp();
  pc[3]->setHp();
  pc[3]->setMp();
  pc[3]->setHunger(10);
  pc[3]->setThirst(10);
  //pc[3]->setStateMod(Constants::possessed, true);          

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

  Item *item = session->newItem(RpgItem::getItemByName("Long sword"));
  item->enchant(Constants::LESSER_MAGIC_ITEM);
  pc[1]->addInventory(item);
  item = session->newItem(RpgItem::getItemByName("Long sword"));
  item->enchant(Constants::GREATER_MAGIC_ITEM);
  pc[1]->addInventory(item);
  item = session->newItem(RpgItem::getItemByName("Long sword"));
  item->enchant(Constants::CHAMPION_MAGIC_ITEM);
  pc[1]->addInventory(item);
  item = session->newItem(RpgItem::getItemByName("Long sword"));
  item->enchant(Constants::DIVINE_MAGIC_ITEM);
  pc[1]->addInventory(item);

  item = session->newItem(RpgItem::getItemByName("Horned helmet"));
  item->enchant(Constants::LESSER_MAGIC_ITEM);
  pc[1]->addInventory(item);
  item = session->newItem(RpgItem::getItemByName("Horned helmet"));
  item->enchant(Constants::GREATER_MAGIC_ITEM);
  pc[1]->addInventory(item);
  item = session->newItem(RpgItem::getItemByName("Horned helmet"));
  item->enchant(Constants::CHAMPION_MAGIC_ITEM);
  pc[1]->addInventory(item);
  item = session->newItem(RpgItem::getItemByName("Horned helmet"));
  item->enchant(Constants::DIVINE_MAGIC_ITEM);
  pc[1]->addInventory(item);


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
      RpgItem *rpgItem = RpgItem::getItemByName("Scroll");
      Item *scroll = session->newItem(rpgItem);
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
  pc[3]->setMp(500);
 
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
  pc[3]->addSpell(Spell::getSpellByName("Bless group"));
  pc[3]->addSpell(Spell::getSpellByName("Invisibility"));
  pc[3]->addSpell(Spell::getSpellByName("Poison of ignorance"));
  pc[3]->addSpell(Spell::getSpellByName("Transmute poison"));
  pc[3]->addSpell(Spell::getSpellByName("Cursed ways"));
  pc[3]->addSpell(Spell::getSpellByName("Remove curse"));
  pc[3]->addSpell(Spell::getSpellByName("Enthrall fiend"));
  pc[3]->addSpell(Spell::getSpellByName("Break from possession"));
#endif

  *partySize = pcCount;  
}

void Party::setParty(int count, Creature **creatures) {
  loadedCount = count;
  for(int i = 0; i < count; i++) loadedParty[i] = creatures[i];
}

/** 
	Return the closest live player within the given radius or null if none can be found.
*/
Creature *Party::getClosestPlayer(int x, int y, int w, int h, int radius) {
  float minDist = 0;
  Creature *p = NULL;
  for(int i = 0; i < getPartySize(); i++) {
	if(!party[i]->getStateMod(Constants::dead) &&
       !party[i]->getStateMod(Constants::possessed)) {
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

void Party::savePlayerSettings() {
  savedPlayer = player;
  savedPlayerOnly = player_only;
}

void Party::restorePlayerSettings() {
  if(savedPlayer->getStateMod(Constants::dead)) setFirstLivePlayer();
  else if(player != savedPlayer) {
    for(int i = 0; i < getPartySize(); i++) {
      if(party[i] == savedPlayer) {
        setPlayer(i);
        break;
      }
    }
  }
  if(savedPlayerOnly != player_only) togglePlayerOnly();
}

