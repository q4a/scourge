

/***************************************************************************
                          inventory.cpp  -  description
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

#include "inventory.h"

/**
  *@author Gabor Torok
  */
  
  
Inventory::Inventory(Scourge *scourge) {
    this->scourge = scourge;

	// allocate strings for list
    this->pcInvText = (char**)malloc(MAX_INVENTORY_SIZE * sizeof(char*));
    for(int i = 0; i < MAX_INVENTORY_SIZE; i++) {
        this->pcInvText[i] = (char*)malloc(120 * sizeof(char));
    }
	this->skillLine = (char**)malloc(Constants::SKILL_COUNT * sizeof(char*));
	for(int i = 0; i < Constants::SKILL_COUNT; i++) {
	  this->skillLine[i] = (char*)malloc(120 * sizeof(char));
	}
	this->stateLine = (char**)malloc(Constants::STATE_MOD_COUNT * sizeof(char*));
	this->icons = (GLuint*)malloc(Constants::STATE_MOD_COUNT * sizeof(GLuint));
	for(int i = 0; i < Constants::STATE_MOD_COUNT; i++) {
	  this->stateLine[i] = (char*)malloc(120 * sizeof(char));
	}
    this->objectiveText = (char**)malloc(MAX_INVENTORY_SIZE * sizeof(char*));
	this->missionColor = (Color*)malloc(MAX_INVENTORY_SIZE * sizeof(Color));
    for(int i = 0; i < MAX_INVENTORY_SIZE; i++) {
        this->objectiveText[i] = (char*)malloc(120 * sizeof(char));
    }
    this->schoolText = (char**)malloc(MAX_INVENTORY_SIZE * sizeof(char*));
    for(int i = 0; i < MAX_INVENTORY_SIZE; i++) {
        this->schoolText[i] = (char*)malloc(120 * sizeof(char));
    }
    this->spellText = (char**)malloc(MAX_INVENTORY_SIZE * sizeof(char*));
    for(int i = 0; i < MAX_INVENTORY_SIZE; i++) {
        this->spellText[i] = (char*)malloc(120 * sizeof(char));
    }
	selected = selectedMode = 0;

	// construct UI
	mainWin = new Window( scourge->getSDLHandler(),
						  100, 50, 525, 505, 
						  strdup("Party Information"), 
						  scourge->getShapePalette()->getGuiTexture() );
	player1Button  = mainWin->createButton( 0, 30, 105, 60, scourge->getParty()->getParty(0)->getName(), true);
	player2Button  = mainWin->createButton( 0, 60, 105, 90, scourge->getParty()->getParty(1)->getName(), true);
	player3Button  = mainWin->createButton( 0, 90, 105, 120, scourge->getParty()->getParty(2)->getName(), true );
	player4Button  = mainWin->createButton( 0, 120, 105, 150, scourge->getParty()->getParty(3)->getName(), true );
	inventoryButton = mainWin->createButton( 105,0, 210, 30, strdup("Inventory"), true);
	skillsButton   = mainWin->createButton( 210,0, 315, 30, strdup("Skills"), true);
	spellsButton   = mainWin->createButton( 315,0, 420, 30, strdup("Spells"), true);
	missionButton   = mainWin->createButton( 420,0, 525, 30, strdup("Mission"), true);
	cards = new CardContainer(mainWin);

	// inventory page	
	cards->createLabel(115, 280, strdup("Inventory:"), INVENTORY, Constants::RED_COLOR);	
	inventoryWeightLabel = cards->createLabel(190, 280, NULL, INVENTORY);
	coinsLabel = cards->createLabel(300, 45, NULL, INVENTORY);
	cards->createLabel(115, 45, strdup("Equipped Items:"), INVENTORY, Constants::RED_COLOR);
    
	for(int i = 0; i < Character::INVENTORY_COUNT; i++) {
	  Item *item = scourge->getParty()->getParty(selected)->getEquippedInventory(i);
	  invEquipLabel[i] = cards->createLabel(300, 60 + (i * 15), 
                               (char *) item ? item->getRpgItem()->getName() : (char*)NULL, 
                               INVENTORY);
	}
	for(int i = 0; i < Character::INVENTORY_COUNT; i++) {
	  cards->createLabel(115, 60 + (i *15), Character::inventory_location[i], INVENTORY);
	}
	invList = new ScrollingList(115, 285, 295, 175, this);
	cards->addWidget(invList, INVENTORY);
	cards->createLabel(115, 475, Constants::getMessage(Constants::EXPLAIN_DRAG_AND_DROP), INVENTORY);
	
	int yy = 160;
	equipButton    = cards->createButton( 0, yy, 105, yy + 30, strdup("Don/Doff"), INVENTORY);
	yy+=30;
	fixButton      = cards->createButton( 0, yy, 105, yy + 30, strdup("Fix Item"), INVENTORY);
	yy+=30;
	removeCurseButton = cards->createButton( 0, yy, 105, yy + 30, strdup("Remove Curse"), INVENTORY );
	yy+=30;
	combineButton  = cards->createButton( 0, yy, 105, yy + 30, strdup("Combine Item"), INVENTORY );
	yy+=30;
	enchantButton  = cards->createButton( 0, yy, 105, yy + 30, strdup("Enchant Item"), INVENTORY );
	yy+=30;
	identifyButton = cards->createButton( 0, yy, 105, yy + 30, strdup("Identify Item"), INVENTORY );
	yy+=30;
	openButton     = cards->createButton( 0, yy, 105, yy + 30, 
										  Constants::getMessage(Constants::OPEN_CONTAINER_LABEL), 
										  INVENTORY );	
	yy+=30;
	eatDrinkButton = cards->createButton( 0, yy, 105, yy + 30, strdup("Eat/Drink"), INVENTORY );

	// character info
	nameAndClassLabel = cards->createLabel(115, 45, NULL, CHARACTER, Constants::RED_COLOR);
	attrCanvas     = new Canvas( 115, 50, 405, 150, this );
	cards->addWidget( attrCanvas, CHARACTER );

	cards->createLabel(115, 165, strdup("Current State:"), CHARACTER, Constants::RED_COLOR);
	stateList = new ScrollingList(115, 170, 290, 70);
	cards->addWidget(stateList, CHARACTER);

	cards->createLabel(115, 255, strdup("Skills:"), CHARACTER, Constants::RED_COLOR);
	skillModLabel = cards->createLabel(220, 255, NULL, CHARACTER);
	skillList = new ScrollingList(115, 260, 290, 180);
	cards->addWidget(skillList, CHARACTER);
	skillAddButton = cards->createButton( 115, 445, 200, 475, strdup(" + "), CHARACTER);
	skillSubButton = cards->createButton( 320, 445, 405, 475, strdup(" - "), CHARACTER);
	levelUpButton = cards->createButton( 205, 445, 315, 475, strdup("Level Up"), CHARACTER);

	// spellbook
	cards->createLabel(115, 45, strdup("School of magic: (with provider deity)"), 
					   SPELL, Constants::RED_COLOR);
	schoolList = new ScrollingList(115, 50, 290, 100);
	cards->addWidget(schoolList, SPELL);
	cards->createLabel(115, 170, strdup("Spells memorized:"), SPELL, Constants::RED_COLOR);
	spellList = new ScrollingList(115, 175, 290, 150);
	cards->addWidget(spellList, SPELL);
	cards->createLabel(115, 345, strdup("Spell notes:"), SPELL, Constants::RED_COLOR);
	spellDescriptionLabel = new Label(115, 360, strdup(""), 58);
	cards->addWidget(spellDescriptionLabel, SPELL);
	castButton = cards->createButton( 0, 160, 105, 190, strdup("Cast"), SPELL);


	// mission
	cards->createLabel(115, 45, strdup("Current Mission"), MISSION, Constants::RED_COLOR);
	missionDescriptionLabel = new Label(115, 60, strdup(""), 70);
	cards->addWidget(missionDescriptionLabel, MISSION);
	cards->createLabel(115, 280, strdup("Mission Objectives"), MISSION, Constants::RED_COLOR);
	objectiveList = new ScrollingList(115, 285, 295, 175);
	cards->addWidget(objectiveList, MISSION);

	setSelectedPlayerAndMode(0, INVENTORY);
}

Inventory::~Inventory() {
}

void Inventory::drawWidget(Widget *w) {
  Creature *p = scourge->getParty()->getParty(selected);

  int y = 15;
  char s[80];
  sprintf(s, "Exp: %u(%u)", p->getExp(), p->getExpOfNextLevel());
  if(p->getStateMod(Constants::leveled)) {
	expLabel->setColor( 1.0f, 0.2f, 0.0f, 1.0f );
  } else {
	w->applyColor();
  }
  scourge->getSDLHandler()->texPrint(5, y, s);
  w->applyColor();
  sprintf(s, "HP: %d (%d)", p->getHp(), p->getMaxHp());
  scourge->getSDLHandler()->texPrint(5, y + 15, s);
  sprintf(s, "MP: %d (%d)", p->getMp(), p->getMaxMp());
  scourge->getSDLHandler()->texPrint(5, y + 30, s);
  sprintf(s, "AC: %d (%d)", p->getSkillModifiedArmor(), p->getArmor());
  scourge->getSDLHandler()->texPrint(5, y + 45, s);
  sprintf(s, "Thirst: %d (10)", p->getThirst());
  scourge->getSDLHandler()->texPrint(5, y + 60, s);
  sprintf(s, "Hunger: %d (10)", p->getHunger());
  scourge->getSDLHandler()->texPrint(5, y + 75, s);

  Util::drawBar( 160,  y - 3, 120, (float)p->getExp(), (float)p->getExpOfNextLevel(), 1.0f, 0.65f, 1.0f, false );
  Util::drawBar( 160, y + 12, 120, (float)p->getHp(), (float)p->getMaxHp() );
  Util::drawBar( 160, y + 27, 120, (float)p->getMp(), (float)p->getMaxMp(), 0.45f, 0.65f, 1.0f, false );
  Util::drawBar( 160, y + 42, 120, (float)p->getSkillModifiedArmor(), (float)p->getArmor(), 0.45f, 0.65f, 1.0f, false );
  Util::drawBar( 160, y + 57, 120, (float)p->getThirst(), 10.0f, 0.45f, 0.65f, 1.0f, false );
  Util::drawBar( 160, y + 72, 120, (float)p->getHunger(), 10.0f, 0.45f, 0.65f, 1.0f, false );
}

bool Inventory::handleEvent(Widget *widget, SDL_Event *event) {
  Creature *creature = scourge->getParty()->getParty(selected);
  char *error = NULL;
  if(widget == mainWin->closeButton) mainWin->setVisible(false);
  else if(widget == player1Button) setSelectedPlayerAndMode(0, selectedMode);
  else if(widget == player2Button) setSelectedPlayerAndMode(1, selectedMode);
  else if(widget == player3Button) setSelectedPlayerAndMode(2, selectedMode);
  else if(widget == player4Button) setSelectedPlayerAndMode(3, selectedMode);
  else if(widget == inventoryButton) setSelectedPlayerAndMode(selected, INVENTORY);
  else if(widget == skillsButton)	setSelectedPlayerAndMode(selected, CHARACTER);
  else if(widget == spellsButton)	setSelectedPlayerAndMode(selected, SPELL);
  else if(widget == missionButton)	setSelectedPlayerAndMode(selected, MISSION);
  else if(widget == openButton) {
	int itemIndex = invList->getSelectedLine();  
	if(itemIndex > -1) {
	  Item *item = scourge->getParty()->getParty(selected)->getInventory(itemIndex);
	  if(item->getRpgItem()->getType() == RpgItem::CONTAINER) {
		scourge->openContainerGui(item);
	  }
	}
  } else if(widget == equipButton) {
	int itemIndex = invList->getSelectedLine();  
	if(itemIndex > -1 && 
	   scourge->getParty()->getParty(selected)->getInventoryCount() > itemIndex) {
	  scourge->getParty()->getParty(selected)->equipInventory(itemIndex);
	  // recreate list strings
	  int oldLine = invList->getSelectedLine();
	  setSelectedPlayerAndMode(selected, selectedMode);
	  invList->setSelectedLine(oldLine);
	}
  } else if(widget == eatDrinkButton) {
	if(scourge->getParty()->getParty(selected)->getStateMod(Constants::dead)) {
	  scourge->showMessageDialog(Constants::getMessage(Constants::DEAD_CHARACTER_ERROR));
	} else {
	  int itemIndex = invList->getSelectedLine();  
	  if(itemIndex > -1 && 
		 creature->getInventoryCount() > itemIndex) {

		// this action will occur in the next battle round
		
		creature->setAction(Constants::ACTION_EAT_DRINK, 
							creature->getInventory(itemIndex),
							NULL);
		creature->setTargetCreature(creature);
		mainWin->setVisible(false);
		
		//		if(scourge->getParty()->getParty(selected)->eatDrink(itemIndex)){
		//		  scourge->getParty()->getParty(selected)->removeInventory(itemIndex);                
		//		}
		// refresh screen
		//setSelectedPlayerAndMode(selected, INVENTORY);
	  }
	}
  } else if(widget == skillAddButton) {
	if(scourge->getParty()->getParty(selected)->getStateMod(Constants::dead) || 
	   !scourge->getParty()->getParty(selected)->getStateMod(Constants::leveled)) {
	  error = Constants::getMessage(Constants::LEVEL_UP_ERROR);
	} else if(scourge->getParty()->getParty(selected)->getAvailableSkillPoints() <= 0) {
	  //	  error = Constants::getMessage(Constants::OUT_OF_POINTS_ERROR);
	} else {
	  int itemIndex = skillList->getSelectedLine();  
	  if(itemIndex <= -1) {
		error = Constants::getMessage(Constants::NO_SKILL_ERROR);
	  } else {
		scourge->getParty()->getParty(selected)->incSkillMod(itemIndex);
		// recreate list strings
		int oldLine = skillList->getSelectedLine();
		setSelectedPlayerAndMode(selected, selectedMode);
		skillList->setSelectedLine(oldLine);
	  }
	}
	if(error) {
	  cerr << error << endl;
	  scourge->showMessageDialog(error);
	}
  } else if(widget == skillSubButton) {
	if(scourge->getParty()->getParty(selected)->getStateMod(Constants::dead) || 
	   !scourge->getParty()->getParty(selected)->getStateMod(Constants::leveled)) {
	  error = Constants::getMessage(Constants::LEVEL_UP_ERROR);
	} else if(scourge->getParty()->getParty(selected)->getAvailableSkillPoints() == 
			  scourge->getParty()->getParty(selected)->getCharacter()->getSkillBonus()) {
	  //	  error = Constants::getMessage(Constants::OUT_OF_POINTS_ERROR);
	} else {
	  int itemIndex = skillList->getSelectedLine();  
	  if(itemIndex <= -1) {
		error = Constants::getMessage(Constants::NO_SKILL_ERROR);
	  } else {
		scourge->getParty()->getParty(selected)->decSkillMod(itemIndex);
		// recreate list strings
		int oldLine = skillList->getSelectedLine();
		setSelectedPlayerAndMode(selected, selectedMode);
		skillList->setSelectedLine(oldLine);
	  }
	}
	if(error) {
	  cerr << error << endl;
	  scourge->showMessageDialog(error);
	}
  } else if(widget == levelUpButton) {
	if(scourge->getParty()->getParty(selected)->getStateMod(Constants::dead) || 
	   !scourge->getParty()->getParty(selected)->getStateMod(Constants::leveled)) {
	  error = Constants::getMessage(Constants::LEVEL_UP_ERROR);
	} else {
	  scourge->getParty()->getParty(selected)->applySkillMod();
	  // recreate list strings
	  int oldLine = skillList->getSelectedLine();
	  setSelectedPlayerAndMode(selected, selectedMode);
	  skillList->setSelectedLine(oldLine);
	}
	if(error) {
	  cerr << error << endl;
	  scourge->showMessageDialog(error);
	}
  } else if(widget == schoolList) {
	int n = schoolList->getSelectedLine();
	if(n != -1 && n < MagicSchool::getMagicSchoolCount()) {
	  showMemorizedSpellsInSchool( scourge->getParty()->getParty(selected), 
								   MagicSchool::getMagicSchool(n));
	}
  } else if(widget == spellList) {
	Spell *spell = getSelectedSpell();
	if(spell) showSpellDescription(spell);
  } else if(widget == castButton) {
	Spell *spell = getSelectedSpell();
	if(spell) {
	  if(spell->getMp() > creature->getMp()) {
		scourge->showMessageDialog("Not enough Magic Points to cast this spell!");
	  } else {
		creature->setAction(Constants::ACTION_CAST_SPELL, 
							NULL,
							spell);
		scourge->setTargetSelectionFor(creature);
		mainWin->setVisible(false);
	  }
	}
  }
  return false;
}

void Inventory::moveItemTo(int playerIndex) {
  int itemIndex = invList->getSelectedLine();  
  if(itemIndex > -1 && 
	 scourge->getParty()->getParty(selected)->getInventoryCount() > itemIndex) {
	if(playerIndex != selected) {
	  scourge->getParty()->getParty(playerIndex)->
		addInventory(scourge->getParty()->getParty(selected)->removeInventory(itemIndex));
	  // recreate strings in list
	  setSelectedPlayerAndMode(selected, selectedMode);
	}
  }
}

bool Inventory::handleEvent(SDL_Event *event) {
    switch(event->type) {
    case SDL_MOUSEBUTTONUP:
        break;     
    case SDL_KEYUP:
        switch(event->key.keysym.sym) {
		  //        case SDLK_ESCAPE: 
		  //		  hide();
		  //		  return true;
        default: break;
        }
    default: break;
    }
    return false;
}

void Inventory::setSelectedPlayerAndMode(int player, int mode) {
  selected = player;
  selectedMode = mode;

  player1Button->setSelected(selected == 0);
  player2Button->setSelected(selected == 1);
  player3Button->setSelected(selected == 2);
  player4Button->setSelected(selected == 3);
  inventoryButton->setSelected(selectedMode == INVENTORY);
  skillsButton->setSelected(selectedMode == CHARACTER);
  spellsButton->setSelected(selectedMode == SPELL);
  missionButton->setSelected(selectedMode == MISSION);
  
  // show only the ui elements belonging to the current mode
  cards->setActiveCard(selectedMode);   

  // arrange the gui
  Creature * selectedP = scourge->getParty()->getParty(selected);
  switch(selectedMode) {
  case CHARACTER:       	
	sprintf(nameAndClassStr, "%s, %s (level %d)", selectedP->getName(), 
			selectedP->getCharacter()->getName(), selectedP->getLevel());
	nameAndClassLabel->setText(nameAndClassStr);	

	stateCount = 0;
    for(int t = 0; t < Constants::STATE_MOD_COUNT; t++) {
      if(selectedP->getStateMod(t)) {
        sprintf(stateLine[stateCount], "%s", Constants::STATE_NAMES[t]);
		icons[stateCount] = scourge->getShapePalette()->getStatModIcon(t);
		stateCount++;
      }
    }
	stateList->setLines(stateCount, (const char**)stateLine, 
						(const Color *)NULL, (stateCount ? (const GLuint*)icons : NULL));
    for(int t = 0; t < Constants::SKILL_COUNT; t++) {
	  sprintf(skillLine[t], "%d(%d) - %s", 
			  selectedP->getSkill(t), 
			  selectedP->getSkillMod(t), 
			  Constants::SKILL_NAMES[t]);
    }
	skillList->setLines(Constants::SKILL_COUNT, (const char**)skillLine);
	break;
	
  case INVENTORY:
	sprintf(inventoryWeightStr, " (Total : %2.2fkg / %2.2fkg)", 
					selectedP->getInventoryWeight(), selectedP->getMaxInventoryWeight());     
	inventoryWeightLabel->setText(inventoryWeightStr);
	sprintf(coinsStr, "Coins: %d", selectedP->getMoney());
	coinsLabel->setText(coinsStr);
	for(int t = 0; t < selectedP->getInventoryCount(); t++) {
		Item *item = selectedP->getInventory(t);
		int location = selectedP->getEquippedIndex(t);
		char s[100];
		item->getDetailedDescription(s);
		sprintf(pcInvText[t], "%s %s", (location > -1 ? " *" : "   "), s);
	}
	for(int t = selectedP->getInventoryCount(); 
			 t < MAX_INVENTORY_SIZE; t++) {
		strcpy(pcInvText[t], "");
	}
	invList->setLines(selectedP->getInventoryCount(), 
										(const char **)pcInvText);
	for(int i = 0; i < Character::INVENTORY_COUNT; i++) {
		Item *item = selectedP->getEquippedInventory(i);
		invEquipLabel[i]->setText((char *)(item ? item->getRpgItem()->getName() : NULL));
	}
	break;
	
  case SPELL:
	for(int t = 0; t < MagicSchool::getMagicSchoolCount(); t++) {
	  MagicSchool *school = MagicSchool::getMagicSchool(t);		
	  sprintf(schoolText[t], "%s (%s)", school->getName(), school->getDeity());
	  if(t == 0) {
		showMemorizedSpellsInSchool(scourge->getParty()->getParty(selected), school);
	  }
	}
	schoolList->setLines(MagicSchool::getMagicSchoolCount(), 
						 (const char**)schoolText);
	break;
  case LOG:
	break;
	case MISSION:
	  int objectiveCount = 0;
	  if(scourge->getCurrentMission()) {
		sprintf(missionText, "%s: %s", 
				scourge->getCurrentMission()->getName(), 
				scourge->getCurrentMission()->getStory());
		objectiveCount = 
		  scourge->getCurrentMission()->getObjective()->itemCount +
		  scourge->getCurrentMission()->getObjective()->monsterCount;		
		for(int t = 0; t < scourge->getCurrentMission()->getObjective()->itemCount; t++) {
		  sprintf(objectiveText[t], "Find %s. %s", 
				  scourge->getCurrentMission()->getObjective()->item[t]->getName(),
				  (scourge->getCurrentMission()->getObjective()->itemHandled[t] ? 
				   "(completed)" : "(not yet found)"));
		  if(scourge->getCurrentMission()->getObjective()->itemHandled[t]) {
			missionColor[t].r = 0.2f;
			missionColor[t].g = 0.7f;
			missionColor[t].b = 0.2f;
		  } else {
			missionColor[t].r = 0.7f;
			missionColor[t].g = 0.2f;
			missionColor[t].b = 0.2f;
		  }
		}
		int start = scourge->getCurrentMission()->getObjective()->itemCount;
		for(int t = 0; t < scourge->getCurrentMission()->getObjective()->monsterCount; t++) {
		  sprintf(objectiveText[start + t], "Vanquish %s. %s", 
				  scourge->getCurrentMission()->getObjective()->monster[t]->getType(),
				  (scourge->getCurrentMission()->getObjective()->monsterHandled[t] ? 
				   "(completed)" : "(not yet done)"));
		  if(scourge->getCurrentMission()->getObjective()->monsterHandled[t]) {
			missionColor[start + t].r = 0.2f;
			missionColor[start + t].g = 0.7f;
			missionColor[start + t].b = 0.2f;
		  } else {
			missionColor[start + t].r = 0.7f;
			missionColor[start + t].g = 0.2f;
			missionColor[start + t].b = 0.2f;
		  }
		}
		start += scourge->getCurrentMission()->getObjective()->monsterCount;
		for(int t = objectiveCount; t < MAX_INVENTORY_SIZE; t++) {
		  strcpy(objectiveText[t], "");
		}
	  } else {
		strcpy(missionText, "");
		for(int t = 0; t < MAX_INVENTORY_SIZE; t++) {
		  strcpy(objectiveText[t], "");
		}
	  }
	  missionDescriptionLabel->setText(missionText);
	  objectiveList->setLines(objectiveCount, 
							  (const char **)objectiveText,
							  missionColor);
	  break;
  }
}

// FIXME: this doesn't work: I can't get it to draw above the window
void Inventory::drawInventory() {
  // draw the characters on top of the UI
  glColor4f(1.0f, 1.0f, 0.4f, 1.0f);
  int h = 120;
//  int y;  
  for(int i = 0; i < 4; i++) {

	// why do I need these 2 lines? Otherwise the models go behind the window
	//	glDisable(GL_DEPTH_TEST);
	//glEnable(GL_DEPTH_TEST);

	glPushMatrix();
	glLoadIdentity();

//	cerr << "z=" << mainWin->getZ() << endl;
	glTranslatef( mainWin->getX(), mainWin->getY() + Window::TOP_HEIGHT, mainWin->getZ() + 200 );
	glTranslatef( 20, 10 + i * h + 90, 300);

	glRotatef(90, 1, 0, 0);
	glScalef(0.8, 0.8, 0.8);	
	glEnable( GL_TEXTURE_2D );
	glColor4f( 1, 1, 1, 1 );
	mainWin->scissorToWindow();
	scourge->getParty()->getParty(i)->draw();
	glDisable( GL_SCISSOR_TEST );
	glDisable( GL_TEXTURE_2D );
	glPopMatrix();
  }
}

void Inventory::receive(Widget *widget) {
  if(scourge->getMovingItem()) {
	if(scourge->getParty()->getParty(selected)->addInventory(scourge->getMovingItem())) {
	  // message: the player accepted the item
	  char message[120];
	  sprintf(message, "%s picks up %s.", 
			  scourge->getParty()->getParty(selected)->getName(),
			  scourge->getMovingItem()->getRpgItem()->getName());
	  scourge->getMap()->addDescription(message);
	  scourge->endItemDrag();
	  setSelectedPlayerAndMode(selected, INVENTORY);
	} else {
	  // message: the player's inventory is full
	}
  }
}

void Inventory::startDrag(Widget *widget) {
  dropItem();
}

void Inventory::dropItem() {
  int itemIndex = invList->getSelectedLine();  
  if(itemIndex > -1 && 
	 scourge->getParty()->getParty(selected)->getInventoryCount() > itemIndex) {
	Item *item = scourge->getParty()->getParty(selected)->removeInventory(itemIndex);
	scourge->setMovingItem(item, 
						   scourge->getParty()->getParty(selected)->getX(), 
						   scourge->getParty()->getParty(selected)->getY(), 
						   scourge->getParty()->getParty(selected)->getZ());
	char message[120];
	sprintf(message, "%s drops %s.", 
			scourge->getParty()->getParty(selected)->getName(),
			item->getRpgItem()->getName());
	scourge->getMap()->addDescription(message);
	setSelectedPlayerAndMode(selected, INVENTORY);
  }
}

void Inventory::refresh() {
  setSelectedPlayerAndMode(selected, INVENTORY);
}

void Inventory::show() { 
  mainWin->setVisible(true); 
  
  // find selected player. FIXME: this is inefficient
  int n = selected;
  for(int i = 0; i < scourge->getParty()->getPartySize(); i++) {
	if(scourge->getParty()->getPlayer() == scourge->getParty()->getParty(i)) {
	  n = i;
	  break;
	}
  }
  setSelectedPlayerAndMode(n, selectedMode); 
}

Spell *Inventory::getSelectedSpell() {
  Creature *creature = scourge->getParty()->getParty(selected);
  MagicSchool *school = NULL;
  int n = schoolList->getSelectedLine();
  if(n != -1 && n < MagicSchool::getMagicSchoolCount()) {
	school = MagicSchool::getMagicSchool(n);
  }
  if(!school) return NULL;

  n = spellList->getSelectedLine();
  if(n != -1 && n < spellList->getLineCount()) {
	int spellCount = 0;
	for(int r = 0; r < school->getSpellCount(); r++) {
	  Spell *spell = school->getSpell(r);
	  if(creature->isSpellMemorized(spell)) {
		if(n == spellCount) return spell;
		spellCount++;
	  }
	}
  }
  return NULL;
}

void Inventory::showMemorizedSpellsInSchool(Creature *creature, MagicSchool *school) {
  int spellCount = 0;
  for(int r = 0; r < school->getSpellCount(); r++) {
	Spell *spell = school->getSpell(r);
	if(creature->isSpellMemorized(spell)) {
	  spell->describe(spellText[spellCount]);
	  if(spellCount == 0) {
		showSpellDescription(spell);
	  }
	  spellCount++;
	}
  }
  if(spellCount == 0) spellDescriptionLabel->setText("");
  spellList->setLines(spellCount, 
					  (const char**)spellText);
}

void Inventory::showSpellDescription(Spell *spell) {
  spellDescriptionLabel->setText((char*)(spell->getNotes() ? spell->getNotes() : ""));
}
