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

#define GUI_WIDTH 500
#define GUI_HEIGHT 165

Party::Party(Scourge *scourge) {
  this->scourge = scourge;

  lastEffectOn = false;

  startRound = true;
  mainWin = NULL;
  calendar = Calendar::getInstance();

  for(int i = 0; i < getPartySize(); i++) {
	party[i] = NULL;
  }
  createUI();
  reset();
}

Party::~Party() {
  delete calendar;
  for(int i = 0; i < getPartySize(); i++) {
	if(party[i]) {
	  delete party[i];
	}
  }
}

void Party::reset() {
  // Init the party; hard code for now
  // This will be replaced by a call to the character builder which either
  // loads or creates the party.
  for(int i = 0; i < getPartySize(); i++) {
	if(party[i]) {
	  delete party[i];
	}
  }
  Creature **pc = createHardCodedParty();
  party[0] = player = pc[0];
  party[1] = pc[1];
  party[2] = pc[2];
  party[3] = pc[3];
  
  player1Button->getLabel()->setText(party[0]->getName());
  player2Button->getLabel()->setText(party[1]->getName());
  player3Button->getLabel()->setText(party[2]->getName());
  player4Button->getLabel()->setText(party[3]->getName());

  Event *e;  
  Date d(0, 0, 6, 0, 0, 0); // 6 hours (format : sec, min, hours, days, months, years)
  for(int i = 0; i < 4 ; i++){
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
}

/**
   Setting the formation ends single-step mode (back to group mode).
 */
void Party::setFormation(int formation) {
  this->formation = formation;
  for(int i = 0; i < 4; i++) {
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
	for(int i = 0; i < 4; i++) {
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
  pc[2]->setLevel(3); 
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

  pc[1]->addInventory(scourge->newItem(RpgItem::getItemByName("Smallbow")));
  pc[1]->addInventory(scourge->newItem(RpgItem::getItemByName("Apple")));
  pc[1]->addInventory(scourge->newItem(RpgItem::getItemByName("Bread")));
  pc[1]->addInventory(scourge->newItem(RpgItem::getItemByName("Mushroom")));
  pc[1]->addInventory(scourge->newItem(RpgItem::getItemByName("Big egg")));
  pc[1]->addInventory(scourge->newItem(RpgItem::getItemByName("Mutton meat")));
  pc[1]->addInventory(scourge->newItem(RpgItem::getItemByName("Minor health potion")));  
  pc[1]->addInventory(scourge->newItem(RpgItem::getItemByName("Liquid armor")));  

  pc[2]->addInventory(scourge->newItem(RpgItem::getItemByName("Long sword")));
  pc[2]->addInventory(scourge->newItem(RpgItem::getItemByName("Wine barrel")));
  pc[2]->addInventory(scourge->newItem(RpgItem::getItemByName("Mutton meat")));
  pc[2]->addInventory(scourge->newItem(RpgItem::getItemByName("Minor health potion")));  
  pc[2]->addInventory(scourge->newItem(RpgItem::getItemByName("Minor health potion")));  
  pc[2]->addInventory(scourge->newItem(RpgItem::getItemByName("Liquid armor")));  

  pc[3]->addInventory(scourge->newItem(RpgItem::getItemByName("Great sword")));
  pc[3]->addInventory(scourge->newItem(RpgItem::getItemByName("Battleaxe")));
  pc[3]->addInventory(scourge->newItem(RpgItem::getItemByName("Throwing axe")));  
  pc[3]->addInventory(scourge->newItem(RpgItem::getItemByName("Minor health potion")));  
  pc[3]->addInventory(scourge->newItem(RpgItem::getItemByName("Minor health potion")));  
  pc[3]->addInventory(scourge->newItem(RpgItem::getItemByName("Minor health potion")));  
  pc[3]->addInventory(scourge->newItem(RpgItem::getItemByName("Liquid armor")));  

  // equip weapons
  pc[0]->equipInventory(0);
  pc[1]->equipInventory(0);
  pc[2]->equipInventory(0);
  pc[3]->equipInventory(0);

  return pc;
}

void Party::createUI() {
  char version[100];
  sprintf(version, "S.C.O.U.R.G.E. version %7.2f", SCOURGE_VERSION);
  mainWin = new Window( scourge->getSDLHandler(),
						scourge->getSDLHandler()->getScreen()->w - GUI_WIDTH, 
						scourge->getSDLHandler()->getScreen()->h - GUI_HEIGHT, 
						GUI_WIDTH, GUI_HEIGHT, 
						strdup(version), 
						scourge->getShapePalette()->getGuiTexture(), false );
//  int gx = sdlHandler->getScreen()->w - GUI_WIDTH;
//  int gy = sdlHandler->getScreen()->h - GUI_HEIGHT;
  inventoryButton = new Button( 0, 0, 120, 25, strdup("Party Info") );
  mainWin->addWidget((Widget*)inventoryButton);
  optionsButton = new Button( 0, 25,  60, 50, strdup("Options") );
  mainWin->addWidget((Widget*)optionsButton);
  quitButton = new Button( 60, 25,  120, 50, strdup("Quit") );
  mainWin->addWidget((Widget*)quitButton);
  roundButton = new Button( 0, 50,  120, 75, strdup("Real-Time") );
  roundButton->setToggle(true);
  roundButton->setSelected(true);
  mainWin->addWidget((Widget*)roundButton);
  
  int lowerRowHeight = 20;
  diamondButton = new Button( 0, 75,  20, 75 + lowerRowHeight, strdup("f1") );
  diamondButton->setToggle(true);
  mainWin->addWidget((Widget*)diamondButton);  
  staggeredButton = new Button( 20, 75,  40, 75 + lowerRowHeight, strdup("f2") );
  staggeredButton->setToggle(true);
  mainWin->addWidget((Widget*)staggeredButton);
  squareButton = new Button( 40, 75,  60, 75 + lowerRowHeight, strdup("f3") );
  squareButton->setToggle(true);
  mainWin->addWidget((Widget*)squareButton);
  rowButton = new Button( 60, 75,  80, 75 + lowerRowHeight, strdup("f4") );
  rowButton->setToggle(true);
  mainWin->addWidget((Widget*)rowButton);
  scoutButton = new Button( 80, 75, 100, 75 + lowerRowHeight, strdup("f5") );
  scoutButton->setToggle(true);
  mainWin->addWidget((Widget*)scoutButton);
  crossButton = new Button( 100, 75,  120, 75 + lowerRowHeight, strdup("f6") );
  crossButton->setToggle(true);
  mainWin->addWidget((Widget*)crossButton);

  groupButton = new Button( 0, 75 + lowerRowHeight,  20, 75 + (lowerRowHeight * 2), strdup("G") );
  groupButton->setToggle(true);
  groupButton->setSelected(true);
  mainWin->addWidget((Widget*)groupButton);
  calendarButton = new Button( 20, 75 + lowerRowHeight, 120, 75 + (lowerRowHeight * 2), 
							   strdup(calendar->getCurrentDate().getDateString()));      
  //calendarButton->setLabelPosition(Button::CENTER);
  mainWin->addWidget((Widget*)calendarButton);    


  int playerButtonWidth = (GUI_WIDTH - 120) / 4;
  int playerButtonHeight = 20;  
  player1Button = new Button( 120 + playerButtonWidth * 0, 0,  120 + playerButtonWidth * 1, playerButtonHeight );
  player1Button->setToggle(true);
  mainWin->addWidget((Widget*)player1Button);
  player2Button = new Button( 120 + playerButtonWidth * 1, 0,  120 + playerButtonWidth * 2, playerButtonHeight );
  player2Button->setToggle(true);
  mainWin->addWidget((Widget*)player2Button);
  player3Button = new Button( 120 + playerButtonWidth * 2, 0,  120 + playerButtonWidth * 3, playerButtonHeight );
  player3Button->setToggle(true);
  mainWin->addWidget((Widget*)player3Button);
  player4Button = new Button( 120 + playerButtonWidth * 3, 0,  120 + playerButtonWidth * 4, playerButtonHeight );
  player4Button->setToggle(true);
  mainWin->addWidget((Widget*)player4Button);

  for(int i = 0; i < getPartySize(); i++) {
	playerInfo[i] = new Canvas( 120 + playerButtonWidth * i, playerButtonHeight,  
								120 + playerButtonWidth * (i + 1), GUI_HEIGHT - 25, 
								this );
	mainWin->addWidget(playerInfo[i]);
  }
}

void Party::drawWidget(Widget *w) {
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
  char msg[80];
  w->applyColor();
  sprintf(msg, "%d/%d", p->getHp(), (p->getCharacter()->getStartingHp() * p->getLevel()));
  scourge->getSDLHandler()->texPrint(3, 10, msg);
  sprintf(msg, "hp:");
  scourge->getSDLHandler()->texPrint(3, 20, msg);
  drawBar(22, 18, ((GUI_WIDTH - 120) / 4) - 24,  
		  (float)p->getHp(), (float)(p->getCharacter()->getStartingHp() * p->getLevel()));

  // FIXME: instead of ac, show magic pts.?
  // ac
  w->applyColor();
  sprintf(msg, "%d/%d", p->getSkillModifiedArmor(), p->getArmor());
  scourge->getSDLHandler()->texPrint(3, 35, msg);
  sprintf(msg, "ac:");
  scourge->getSDLHandler()->texPrint(3, 45, msg);
  drawBar(22, 43, ((GUI_WIDTH - 120) / 4) - 24,  
		  (float)p->getSkillModifiedArmor(), (float)p->getArmor());

  // exp
  w->applyColor();
  sprintf(msg, "%d (%d)", p->getExp(), p->getLevel());
  scourge->getSDLHandler()->texPrint(3, 60, msg);
  sprintf(msg, "ex:");
  scourge->getSDLHandler()->texPrint(3, 70, msg);
  drawBar(22, 68, ((GUI_WIDTH - 120) / 4) - 24,  
		  (float)p->getExp(), (float)p->getExpOfNextLevel(),
		  0.45f, 0.65f, 1.0f, false);

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

void Party::drawBar(int x, int y, float barLength, float value, float maxValue,
					float red, float green, float blue, float gradient) {
  float percent = (maxValue == 0 ? 0 : (value >= maxValue ? 100.0f : value / (maxValue / 100.0f)));
  float length = barLength * (percent / 100.0f);
  
  glPushMatrix();
  glTranslatef( x, y, 0 );
  glLineWidth(6.0f);
  
  //  glColor3f( 0.2f, 0.2f, 0.2f );
  glColor3f( 1, 0.75f, 0.45f );
  glBegin( GL_LINES );
  glVertex3f( 0, 0, 0 );
  glVertex3f( barLength, 0, 0 );
  glEnd();
  
  // default args so I don't have to recompile party.h
  if(red == -1) {
	red = 0.5f;
	green = 1.0f;
	blue = 0.5f;
  }
  if(!gradient || percent > 40.0f) {	
	glColor3f( red, green, blue );
  } else if(percent > 25.0f) {
	glColor3f( 1.0f, 1.0f, 0.5f );
  } else {
	glColor3f( 1.0f, 0.5f, 0.5f );
  }
  glBegin( GL_LINES );
  glVertex3f( 0, 0, 0 );
  glVertex3f( length, 0, 0 );
  glEnd();
  
  glLineWidth(1.0f);
  
  glPopMatrix();
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
