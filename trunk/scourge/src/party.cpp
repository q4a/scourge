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

#define MAX_SIZE 0
#define MIN_SIZE 1

int Party::pcCount;
Creature **Party::pc;

Party::Party(Scourge *scourge) {
  this->scourge = scourge;

  lastEffectOn = false;

  startRound = true;
  mainWin = NULL;
  calendar = Calendar::getInstance();

  // do we need this?
  //for(int i = 0; i < 4; i++) {
//    party[i] = NULL;
//  }
  createUI();
  reset();
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
  party[0] = player = new Creature(pc[0]);
  party[1] = new Creature(pc[1]);
  party[2] = new Creature(pc[2]);
  party[3] = new Creature(pc[3]);
  partySize = 4;
  resetPartyUI();
}

void Party::resetMultiplayer(Creature *c) {
  deleteParty();
  party[0] = player = new Creature(c);
  party[1] = party[2] = party[3] = NULL;
  partySize = 1;
  resetPartyUI();
}

void Party::resetPartyUI() {
  player1Button->getLabel()->setText(party[0]->getName());
  player1Button->setSelected(true);
  if(getPartySize() > 1) {
    player2Button->getLabel()->setText(party[1]->getName());
    player2Button->setVisible(true);
  } else {
    player2Button->setVisible(false);
  }
  if(getPartySize() > 2) {
    player3Button->getLabel()->setText(party[2]->getName());
    player3Button->setVisible(true);
  } else {
    player3Button->setVisible(false);
  }
  if(getPartySize() > 3) {
    player4Button->getLabel()->setText(party[3]->getName());
    player4Button->setVisible(true);
  } else {
    player4Button->setVisible(false);
  }

  Event *e;  
  Date d(0, 0, 6, 0, 0, 0); // 6 hours (format : sec, min, hours, days, months, years)
  for(int i = 0; i < getPartySize() ; i++){
    e = new ThirstHungerEvent(calendar->getCurrentDate(), d, party[i], scourge, Event::INFINITE_EXECUTIONS);
    calendar->scheduleEvent((Event*)e);   // It's important to cast!!
  }
}

void Party::startPartyOnMission() {
  // Start calendar and add thirst & hunger event scheduling
  calendar->reset(false);

  player_only = false;
  partyDead = false;

  // set player to be the first non-dead character
  for(int i = 0; i < getPartySize(); i++) {
	if(!party[i]->getStateMod(Constants::dead)) {
	  setPlayer(getParty(i));
	  break;
	}
  }
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
  scourge->getMap()->refresh();
  scourge->getMap()->center(getPlayer()->getX(), getPlayer()->getY());
  player1Button->setSelected(false);
  player2Button->setSelected(false);
  player3Button->setSelected(false);
  player4Button->setSelected(false);
  switch(n) {
  case 0 : player1Button->setSelected(true); break;
  case 1 : player2Button->setSelected(true); break;
  case 2 : player3Button->setSelected(true); break;
  case 3 : player4Button->setSelected(true); break;
  }
  scourge->getMap()->center(player->getX(), player->getY(), true);
  scourge->getInventory()->refresh(n);
}

/**
   Setting the formation ends single-step mode (back to group mode).
 */
void Party::setFormation(int formation) {
  this->formation = formation;
  for(int i = 0; i < getPartySize(); i++) {
	party[i]->setFormation(formation);
  }
  player_only = false;

  groupButton->setSelected(!isPlayerOnly());
  startRound = true;
  roundButton->setSelected(startRound);
  diamondButton->setSelected(false);
  staggeredButton->setSelected(false);
  squareButton->setSelected(false);
  rowButton->setSelected(false);
  scoutButton->setSelected(false);
  crossButton->setSelected(false);
  switch(formation + Constants::DIAMOND_FORMATION) {
  case Constants::DIAMOND_FORMATION:
    diamondButton->setSelected(true); break;
  case Constants::STAGGERED_FORMATION:
    staggeredButton->setSelected(true); break;
  case Constants::SQUARE_FORMATION:
    squareButton->setSelected(true); break;
  case Constants::ROW_FORMATION:
    rowButton->setSelected(true); break;
  case Constants::SCOUT_FORMATION:
    scoutButton->setSelected(true); break;
  case Constants::CROSS_FORMATION:
    crossButton->setSelected(true); break;
  }
}

void Party::togglePlayerOnly() {
  player_only = (player_only ? false : true);
  // in group mode everyone hunts the same creature
  if(!player_only) {
	for(int i = 0; i < getPartySize(); i++) {
	  if(party[i] != player) 
		party[i]->setTargetCreature(player->getTargetCreature());
	}
  }
  if(player_only)
	scourge->getMap()->addDescription(Constants::getMessage(Constants::SINGLE_MODE), 0.5f, 0.5f, 1.0f);
  else
	scourge->getMap()->addDescription(Constants::getMessage(Constants::GROUP_MODE), 0.5f, 0.5f, 1.0f);
  groupButton->setSelected(!isPlayerOnly());
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
	scourge->getMap()->addDescription(Constants::getMessage(Constants::REAL_TIME_MODE), 0.5f, 0.5f, 1.0f);
  }
  else{
	scourge->getMap()->addDescription(Constants::getMessage(Constants::TURN_MODE), 0.5f, 0.5f, 1.0f);    
  }
   
  // Freeze / unfreeze calendar
  calendar->setPause(!startRound); 
  
  // Freeze / unfreeze animations
  for(int i = 0; i < getPartySize(); i++){
    getParty(i)->getShape()->setPauseAnimation(!startRound);
  }  
  for(int i = 0; i < scourge->getCreatureCount(); i++){
    scourge->getCreature(i)->getShape()->setPauseAnimation(!startRound);
  } 
  roundButton->setSelected(startRound);
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
		party[i]->moveToLocator(scourge->getMap());
	  }
	}
	// center on player
	scourge->getMap()->center(player->getX(), player->getY());
  } else {
	// In group mode:
	
	// move the leader
	if(!player->getStateMod(Constants::dead)) {
	  player->moveToLocator(scourge->getMap());
	  scourge->getMap()->center(player->getX(), player->getY());
	}
	
	// others follow the player
	for(int t = 0; t < getPartySize(); t++) {
	  if(!party[t]->getStateMod(Constants::dead) && party[t] != player) {
		party[t]->moveToLocator(scourge->getMap());
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
Creature **Party::createHardCodedParty(Scourge *scourge) {
  pcCount = 4;
  pc = (Creature**)malloc(sizeof(Creature*) * pcCount);

  int level = 6;

  // FIXME: consider using newCreature here
  // the end of startMission would have to be modified to not delete the party
  // also in scourge, where-ever creatureCount is used to mean all monsters would have
  // to change (maybe that's a good thing too... same logic for party and monsters)
  pc[0] = new Creature(scourge, Character::getCharacterByName("Assassin"), "Alamont");
  pc[0]->setLevel(level); 
  pc[0]->setExp();
  pc[0]->setHp();
  pc[0]->setMp();
  pc[0]->setHunger(8);
  pc[0]->setThirst(7); 
  pc[0]->setStateMod(Constants::blessed, true);
  pc[0]->setStateMod(Constants::poisoned, true);

  pc[1] = new Creature(scourge, Character::getCharacterByName("Knight"), "Barlett");
  pc[1]->setLevel(level); 
  pc[1]->setExp();
  pc[1]->setHp();
  pc[1]->setMp();
  pc[1]->setHunger(10);
  pc[1]->setThirst(9);
  pc[1]->setStateMod(Constants::drunk, true);
  pc[1]->setStateMod(Constants::cursed, true);      

  pc[2] = new Creature(scourge, Character::getCharacterByName("Summoner"), "Corinus");
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

  pc[3] = new Creature(scourge, Character::getCharacterByName("Naturalist"), "Dialante");    
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
  pc[0]->addInventory(scourge->newItem(RpgItem::getItemByName("Bastard sword")));
  pc[0]->addInventory(scourge->newItem(RpgItem::getItemByName("Horned helmet")));
  pc[0]->addInventory(scourge->newItem(RpgItem::getItemByName("Dagger")));
  pc[0]->addInventory(scourge->newItem(RpgItem::getItemByName("Minor health potion")));  
  pc[0]->addInventory(scourge->newItem(RpgItem::getItemByName("Minor health potion")));  
  pc[0]->addInventory(scourge->newItem(RpgItem::getItemByName("Liquid armor")));  
  pc[0]->addInventory(scourge->newItem(RpgItem::getItemByName("Potion of Speed")));
  pc[0]->addInventory(scourge->newItem(RpgItem::getItemByName("Potion of Coordination")));
  pc[0]->addInventory(scourge->newItem(RpgItem::getItemByName("Potion of Power")));
  pc[0]->addInventory(scourge->newItem(RpgItem::getItemByName("Potion of IQ")));
  pc[0]->addInventory(scourge->newItem(RpgItem::getItemByName("Potion of Leadership")));
  pc[0]->addInventory(scourge->newItem(RpgItem::getItemByName("Potion of Luck")));
  pc[0]->addInventory(scourge->newItem(RpgItem::getItemByName("Potion of Piety")));
  pc[0]->addInventory(scourge->newItem(RpgItem::getItemByName("Potion of Lore")));
  pc[0]->addInventory(scourge->newItem(RpgItem::getItemByName("Minor magic potion")));  
  pc[0]->addInventory(scourge->newItem(RpgItem::getItemByName("Minor magic potion")));  

  pc[1]->addInventory(scourge->newItem(RpgItem::getItemByName("Smallbow")));
  pc[1]->addInventory(scourge->newItem(RpgItem::getItemByName("Apple")));
  pc[1]->addInventory(scourge->newItem(RpgItem::getItemByName("Bread")));
  pc[1]->addInventory(scourge->newItem(RpgItem::getItemByName("Mushroom")));
  pc[1]->addInventory(scourge->newItem(RpgItem::getItemByName("Big egg")));
  pc[1]->addInventory(scourge->newItem(RpgItem::getItemByName("Mutton meat")));
  pc[1]->addInventory(scourge->newItem(RpgItem::getItemByName("Minor health potion")));  
  pc[1]->addInventory(scourge->newItem(RpgItem::getItemByName("Minor magic potion")));  
  pc[1]->addInventory(scourge->newItem(RpgItem::getItemByName("Liquid armor")));  

  pc[2]->addInventory(scourge->newItem(RpgItem::getItemByName("Dagger")));
  pc[2]->addInventory(scourge->newItem(RpgItem::getItemByName("Smallbow")));
  pc[2]->addInventory(scourge->newItem(RpgItem::getItemByName("Long sword")));
  pc[2]->addInventory(scourge->newItem(RpgItem::getItemByName("Wine barrel")));
  pc[2]->addInventory(scourge->newItem(RpgItem::getItemByName("Mutton meat")));
  pc[2]->addInventory(scourge->newItem(RpgItem::getItemByName("Minor health potion")));  
  pc[2]->addInventory(scourge->newItem(RpgItem::getItemByName("Minor health potion")));  
  pc[2]->addInventory(scourge->newItem(RpgItem::getItemByName("Minor magic potion")));  
  pc[2]->addInventory(scourge->newItem(RpgItem::getItemByName("Minor magic potion")));  
  pc[2]->addInventory(scourge->newItem(RpgItem::getItemByName("Liquid armor")));   

  // add some scrolls
  for(int i = 0; i < 10; i++) {
    Spell *spell = MagicSchool::getRandomSpell(1);
    if(spell) {
      Item *scroll = scourge->newItem(RpgItem::getItemByName("Scroll"));
      scroll->setSpell(spell);
      pc[2]->addInventory(scroll);  
    }
  }
  pc[2]->setMp(50);

  pc[3]->addInventory(scourge->newItem(RpgItem::getItemByName("Dagger")));
  pc[3]->addInventory(scourge->newItem(RpgItem::getItemByName("Great sword")));
  pc[3]->addInventory(scourge->newItem(RpgItem::getItemByName("Battleaxe")));
  pc[3]->addInventory(scourge->newItem(RpgItem::getItemByName("Throwing axe")));  
  pc[3]->addInventory(scourge->newItem(RpgItem::getItemByName("Minor health potion")));  
  pc[3]->addInventory(scourge->newItem(RpgItem::getItemByName("Minor health potion")));  
  pc[3]->addInventory(scourge->newItem(RpgItem::getItemByName("Minor health potion")));  
  pc[3]->addInventory(scourge->newItem(RpgItem::getItemByName("Minor magic potion")));  
  pc[3]->addInventory(scourge->newItem(RpgItem::getItemByName("Minor magic potion")));  
  pc[3]->addInventory(scourge->newItem(RpgItem::getItemByName("Liquid armor")));  
  
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
				  

  return pc;
}

void Party::createUI() {
  sprintf(version, "S.C.O.U.R.G.E. version %7.2f", SCOURGE_VERSION);
  sprintf(min_version, "S.C.O.U.R.G.E.");
  mainWin = new Window( scourge->getSDLHandler(),
						scourge->getSDLHandler()->getScreen()->w - Scourge::PARTY_GUI_WIDTH, 
						scourge->getSDLHandler()->getScreen()->h - Scourge::PARTY_GUI_HEIGHT, 
						Scourge::PARTY_GUI_WIDTH, Scourge::PARTY_GUI_HEIGHT, 
						version, 
						scourge->getShapePalette()->getGuiTexture(), false );
  cards = new CardContainer(mainWin);  

  inventoryButton = cards->createButton( 0, 0, 120, 25, strdup("Party Info"), MAX_SIZE );
  optionsButton = cards->createButton( 0, 25,  60, 50, strdup("Options"), MAX_SIZE );
  quitButton = cards->createButton( 60, 25,  120, 50, strdup("Quit"), MAX_SIZE );
  roundButton = cards->createButton( 0, 50,  120, 75, strdup("Real-Time"), MAX_SIZE );
  roundButton->setToggle(true);
  roundButton->setSelected(true);
  
  int lowerRowHeight = 20;
  diamondButton = cards->createButton( 0, 75,  20, 75 + lowerRowHeight, strdup("f1"), MAX_SIZE );
  diamondButton->setToggle(true);
  staggeredButton = cards->createButton( 20, 75,  40, 75 + lowerRowHeight, strdup("f2"), MAX_SIZE );
  staggeredButton->setToggle(true);
  squareButton = cards->createButton( 40, 75,  60, 75 + lowerRowHeight, strdup("f3"), MAX_SIZE );
  squareButton->setToggle(true);
  rowButton = cards->createButton( 60, 75,  80, 75 + lowerRowHeight, strdup("f4"), MAX_SIZE );
  rowButton->setToggle(true);
  scoutButton = cards->createButton( 80, 75, 100, 75 + lowerRowHeight, strdup("f5"), MAX_SIZE );
  scoutButton->setToggle(true);
  crossButton = cards->createButton( 100, 75,  120, 75 + lowerRowHeight, strdup("f6"), MAX_SIZE );
  crossButton->setToggle(true);

  groupButton = cards->createButton( 0, 75 + lowerRowHeight,  20, 75 + (lowerRowHeight * 2), strdup("G"), MAX_SIZE );
  groupButton->setToggle(true);
  groupButton->setSelected(true);
  calendarButton = cards->createButton( 20, 75 + lowerRowHeight, 120, 75 + (lowerRowHeight * 2), 
							   strdup(calendar->getCurrentDate().getDateString()), MAX_SIZE);      
  //calendarButton->setLabelPosition(Button::CENTER);

  minButton = cards->createButton( 0, 75 + (lowerRowHeight * 2), 20, 75 + (lowerRowHeight * 3), strdup("-"), MAX_SIZE );
  maxButton = cards->createButton( 0, 75 + (lowerRowHeight * 2), 20, 75 + (lowerRowHeight * 3), strdup("+"), MIN_SIZE );

  layoutButton1 = cards->createButton( 20, 75 + (lowerRowHeight * 2), 40, 75 + (lowerRowHeight * 3), strdup("L1"), MAX_SIZE );
  layoutButton2 = cards->createButton( 40, 75 + (lowerRowHeight * 2), 60, 75 + (lowerRowHeight * 3), strdup("L2"), MAX_SIZE );
  layoutButton3 = cards->createButton( 60, 75 + (lowerRowHeight * 2), 80, 75 + (lowerRowHeight * 3), strdup("L3"), MAX_SIZE );
  layoutButton4 = cards->createButton( 80, 75 + (lowerRowHeight * 2), 100, 75 + (lowerRowHeight * 3), strdup("L4"), MAX_SIZE );


  int playerButtonWidth = (Scourge::PARTY_GUI_WIDTH - 120) / 4;
  int playerButtonHeight = 20;  
  player1Button = cards->createButton( 120 + playerButtonWidth * 0, 0,  
									   120 + playerButtonWidth * 1, playerButtonHeight, NULL, MAX_SIZE );
  player1Button->setToggle(true);
  player2Button = cards->createButton( 120 + playerButtonWidth * 1, 0,  
									   120 + playerButtonWidth * 2, playerButtonHeight, NULL, MAX_SIZE );
  player2Button->setToggle(true);
  player3Button = cards->createButton( 120 + playerButtonWidth * 2, 0, 
									   120 + playerButtonWidth * 3, playerButtonHeight, NULL, MAX_SIZE );
  player3Button->setToggle(true);
  player4Button = cards->createButton( 120 + playerButtonWidth * 3, 0,  
									   120 + playerButtonWidth * 4, playerButtonHeight, NULL, MAX_SIZE );
  player4Button->setToggle(true);

  for(int i = 0; i < 4; i++) {
	playerInfo[i] = new Canvas( 120 + playerButtonWidth * i, playerButtonHeight,  
								120 + playerButtonWidth * (i + 1), Scourge::PARTY_GUI_HEIGHT - 25, 
								this );
	cards->addWidget( playerInfo[i], MAX_SIZE );
  }

  minPartyInfo = new Canvas( 0, 0, Scourge::PARTY_MIN_GUI_WIDTH, 75 + (lowerRowHeight * 2), this );
  cards->addWidget( minPartyInfo, MIN_SIZE );

  cards->setActiveCard( MAX_SIZE );   
}

void Party::drawWidget(Widget *w) {
  char msg[80];
  if(w == minPartyInfo) {
	for(int i = 0; i < getPartySize(); i++) {	  
	  // hp
	  if(getParty(i) == getPlayer()) {
		w->applyBorderColor();
		glBegin( GL_QUADS );
		glVertex3f( Scourge::PARTY_MIN_GUI_WIDTH, (i * 20), 0 );
		glVertex3f( 0, (i * 20), 0 );
		glVertex3f( 0, 20 + (i * 20), 0 );
		glVertex3f( Scourge::PARTY_MIN_GUI_WIDTH, 20 + (i * 20), 0 );
		glEnd();
	  }
	  //	  w->applyColor();
	  glColor4f( 0.8f, 0.2f, 0.0f, 1.0f );
	  sprintf(msg, "%c:", getParty(i)->getName()[0]);
	  scourge->getSDLHandler()->texPrint(0, 13 + (i * 20), msg);
	  Util::drawBar(15, 8 + (i * 20), Scourge::PARTY_MIN_GUI_WIDTH - 20,  
					(float)getParty(i)->getHp(), (float)getParty(i)->getMaxHp());
	  Util::drawBar(15, 14 + (i * 20), Scourge::PARTY_MIN_GUI_WIDTH - 20,  
					(float)getParty(i)->getMp(), (float)getParty(i)->getMaxMp(),
					0.45f, 0.65f, 1.0f, false);
	}
  } else {
	int selectedPlayerIndex = -1;
	for(int i = 0; i < getPartySize(); i++) {
	  if(playerInfo[i] == w) {
		selectedPlayerIndex = i;
		break;
	  }
	}
	if(selectedPlayerIndex == -1) {
	  cerr << "Warning: Unknown widget in Party::drawWidget." << endl;
	  return;
	}
	Creature *p = getParty(selectedPlayerIndex);
	
	// hp
	w->applyColor();
	sprintf(msg, "%d/%d", p->getHp(), p->getMaxHp());
	scourge->getSDLHandler()->texPrint(3, 10, msg);
	glColor4f( 0.8f, 0.2f, 0.0f, 1.0f );
	sprintf(msg, "hp:");
	scourge->getSDLHandler()->texPrint(3, 20, msg);
	Util::drawBar(22, 18, ((Scourge::PARTY_GUI_WIDTH - 120) / 4) - 24,  
				  (float)p->getHp(), (float)p->getMaxHp());
	
	// mp
	w->applyColor();
	sprintf(msg, "%d/%d", p->getMp(), p->getMaxMp());
	scourge->getSDLHandler()->texPrint(3, 35, msg);
	glColor4f( 0.8f, 0.2f, 0.0f, 1.0f );
	sprintf(msg, "mp:");
	scourge->getSDLHandler()->texPrint(3, 45, msg);
	Util::drawBar(22, 43, ((Scourge::PARTY_GUI_WIDTH - 120) / 4) - 24,  
				  (float)p->getMp(), (float)p->getMaxMp(),
				  0.45f, 0.65f, 1.0f, false);

	/*
	// ac
	w->applyColor();
	sprintf(msg, "%d/%d", p->getSkillModifiedArmor(), p->getArmor());
	scourge->getSDLHandler()->texPrint(3, 35, msg);
	glColor4f( 0.8f, 0.2f, 0.0f, 1.0f );
	sprintf(msg, "ac:");
	scourge->getSDLHandler()->texPrint(3, 45, msg);
	Util::drawBar(22, 43, ((GUI_WIDTH - 120) / 4) - 24,  
	(float)p->getSkillModifiedArmor(), (float)p->getArmor());
	*/

	// exp
	w->applyColor();
	sprintf(msg, "%d (%d)", p->getExp(), p->getLevel());
	scourge->getSDLHandler()->texPrint(3, 60, msg);
	glColor4f( 0.8f, 0.2f, 0.0f, 1.0f );
	sprintf(msg, "ex:");
	scourge->getSDLHandler()->texPrint(3, 70, msg);
	Util::drawBar(22, 68, ((Scourge::PARTY_GUI_WIDTH - 120) / 4) - 24,  
				  (float)p->getExp(), (float)p->getExpOfNextLevel(),
				  1.0f, 0.65f, 1.0f, false);
	
	// show stat mods
	glEnable(GL_TEXTURE_2D);
	int xp = 0;
	int yp = 0;
	float n = 12;
	int row = 5;
	int left = 5;
	int bottom = w->getHeight() - ((int)(3 * n + 1) + 4);
	for(int i = 0; i < Constants::STATE_MOD_COUNT; i++) {
	  GLuint icon = scourge->getShapePalette()->getStatModIcon(i);
	  if(p->getStateMod(i)) {
		glColor4f( 1.0f, 1.0f, 0.5f, 0.5f );
		if(icon) {
		  glBindTexture( GL_TEXTURE_2D, icon );
		}
	  } else {
		w->applyBorderColor();
		icon = 0;
	  }
	  
	  glPushMatrix();
	  glTranslatef( left + xp * (n + 1), bottom + (yp * (n + 1)), 0 );
	  glBegin( GL_QUADS );
	  glNormal3f( 0, 0, 1 );
	  if(icon) glTexCoord2f( 0, 0 );
	  glVertex3f( 0, 0, 0 );
	  if(icon) glTexCoord2f( 0, 1 );
	  glVertex3f( 0, n, 0 );
	  if(icon) glTexCoord2f( 1, 1 );
	  glVertex3f( n, n, 0 );
	  if(icon) glTexCoord2f( 1, 0 );
	  glVertex3f( n, 0, 0 );
	  glEnd();
	  glPopMatrix();
	  
	  xp++;
	  if(xp >= row) {
		xp = 0;
		yp++;
	  }
	}
	glDisable(GL_TEXTURE_2D);
  }
}

void Party::drawView() {
  // update current date variables and see if scheduled events have occured  
  if(calendar->update(scourge->getUserConfiguration()->getGameSpeedLevel())){
	calendarButton->getLabel()->setTextCopy(calendar->getCurrentDate().getDateString());        
  }
  // refresh map if any party member's effect is on
  bool effectOn = false;
  for(int i = 0; i < getPartySize(); i++) {
	if(!party[i]->getStateMod(Constants::dead) && party[i]->isEffectOn()) {
	  effectOn = true;
	  break;
	}
  }
  if(effectOn != lastEffectOn) {
	lastEffectOn = effectOn;
	scourge->getMap()->refresh();
  }
}

bool Party::handleEvent(Widget *widget, SDL_Event *event) {
  if(widget == inventoryButton) {
    scourge->toggleInventoryWindow();
  } else if(widget == optionsButton) {
    scourge->toggleOptionsWindow();
  } else if(widget == quitButton) {
    scourge->showExitConfirmationDialog();
  } else if(widget == diamondButton) {
    setFormation(Constants::DIAMOND_FORMATION - Constants::DIAMOND_FORMATION);
  } else if(widget == staggeredButton) {
    setFormation(Constants::STAGGERED_FORMATION - Constants::DIAMOND_FORMATION);
  } else if(widget == squareButton) {
    setFormation(Constants::SQUARE_FORMATION - Constants::DIAMOND_FORMATION);
  } else if(widget == rowButton) {
    setFormation(Constants::ROW_FORMATION - Constants::DIAMOND_FORMATION);
  } else if(widget == scoutButton) {
    setFormation(Constants::SCOUT_FORMATION - Constants::DIAMOND_FORMATION);
  } else if(widget == crossButton) {
    setFormation(Constants::CROSS_FORMATION - Constants::DIAMOND_FORMATION);
  } else if(widget == player1Button) {
    setPlayer(Constants::PLAYER_1 - Constants::PLAYER_1);
  } else if(widget == player2Button) {
    setPlayer(Constants::PLAYER_2 - Constants::PLAYER_1);
  } else if(widget == player3Button) {
    setPlayer(Constants::PLAYER_3 - Constants::PLAYER_1);
  } else if(widget == player4Button) {
    setPlayer(Constants::PLAYER_4 - Constants::PLAYER_1);
  } else if(widget == groupButton) {
    togglePlayerOnly();
  } else if(widget == roundButton) {
    toggleRound();
  } else if(widget == minButton) {
    cards->setActiveCard( MIN_SIZE );
    mainWin->resize( Scourge::PARTY_MIN_GUI_WIDTH, Scourge::PARTY_GUI_HEIGHT );
    oldX = mainWin->getX();
    mainWin->move( (oldX < (scourge->getSDLHandler()->getScreen()->w / 2) - (Scourge::PARTY_GUI_WIDTH / 2) ? 
                    0 : 
                    scourge->getSDLHandler()->getScreen()->w - Scourge::PARTY_MIN_GUI_WIDTH), mainWin->getY() );
    mainWin->setTitle( min_version );
  } else if(widget == maxButton) {
    cards->setActiveCard( MAX_SIZE );
    mainWin->move( oldX, mainWin->getY() );
    mainWin->resize( Scourge::PARTY_GUI_WIDTH, Scourge::PARTY_GUI_HEIGHT );
    mainWin->setTitle( version );
  } else if(widget == layoutButton1) {
    scourge->setUILayout(Constants::GUI_LAYOUT_ORIGINAL);
  } else if(widget == layoutButton2) {
    scourge->setUILayout(Constants::GUI_LAYOUT_BOTTOM);
  } else if(widget == layoutButton3) {
    scourge->setUILayout(Constants::GUI_LAYOUT_SIDE);
  } else if(widget == layoutButton4) {
    scourge->setUILayout(Constants::GUI_LAYOUT_INVENTORY);
  }
  return false;
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
