

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
	for(int i = 0; i < Constants::STATE_MOD_COUNT; i++) {
	  this->stateLine[i] = (char*)malloc(120 * sizeof(char));
	}
	selected = selectedMode = 0;

	// construct UI
	mainWin = new Window( scourge->getSDLHandler(),
						  100, 50, 525, 505, 
						  strdup("Party Information"), 
						  scourge->getShapePalette()->getGuiTexture() );
	player1Button = new Button( 0, 0, 105, 120, scourge->getParty(0)->getName() );
	player1Button->setLabelPosition(Button::BOTTOM);	
	player1Button->setToggle(true);
	mainWin->addWidget((Widget*)player1Button);
	player2Button = new Button( 0, 120, 105, 240, scourge->getParty(1)->getName() );
	player2Button->setLabelPosition(Button::BOTTOM);
	player2Button->setToggle(true);
	mainWin->addWidget((Widget*)player2Button);
	player3Button = new Button( 0, 240, 105, 360, scourge->getParty(2)->getName() );
	player3Button->setLabelPosition(Button::BOTTOM);
	player3Button->setToggle(true);
	mainWin->addWidget((Widget*)player3Button);
	player4Button = new Button( 0,360, 105, 480, scourge->getParty(3)->getName() );
	player4Button->setLabelPosition(Button::BOTTOM);
	player4Button->setToggle(true);
	mainWin->addWidget((Widget*)player4Button);
	inventoryButton = new Button( 105,0, 210, 30, strdup("Inventory") );
	inventoryButton->setToggle(true);
	mainWin->addWidget((Widget*)inventoryButton);
	skillsButton = new Button( 210,0, 315, 30, strdup("Skills") );
	skillsButton->setToggle(true);
	mainWin->addWidget((Widget*)skillsButton);
	spellsButton = new Button( 315,0, 420, 30, strdup("Spells") );
	spellsButton->setToggle(true);
	mainWin->addWidget((Widget*)spellsButton);
	closeButton = new Button( 420,0, 525, 30, strdup("Close") );
	mainWin->addWidget((Widget*)closeButton);

	cards = new CardContainer(mainWin);

	// inventory page
	Label *label = new Label(115, 280, strdup("Inventory:"));
	label->setColor( 0.8f, 0.2f, 0, 1 );
	cards->addWidget(label, INVENTORY);
	label = new Label(115, 45, strdup("Equipped Items:"));
	label->setColor( 0.8f, 0.2f, 0, 1 );
	cards->addWidget(label, INVENTORY);
	for(int i = 0; i < Character::INVENTORY_COUNT; i++) {
	  Item *item = scourge->getParty(selected)->getEquippedInventory(i);
	  invEquipLabel[i] = new Label(300, 60 + (i * 15), 
								   (char *)(item ? item->getRpgItem()->getName() : (char*)NULL));
	  cards->addWidget(invEquipLabel[i], INVENTORY);
	}
	for(int i = 0; i < Character::INVENTORY_COUNT; i++) {
	  label = new Label(115, 60 + (i * 15), Character::inventory_location[i]);
	  cards->addWidget(label, INVENTORY);
	}
	invList = new ScrollingList(115, 285, 295, 175);
	cards->addWidget(invList, INVENTORY);
	for(int i = 0; i < 4; i++) {
	  invToButton[i] = new Button( 420, 35 + (i * 30), 520, 35 + (i * 30) + 25, 
								   scourge->getParty(i)->getName() );
	  cards->addWidget( invToButton[i], INVENTORY );
	}
	equipButton = new Button( 420, 155, 520, 180, strdup("Don/Doff") );
	cards->addWidget(equipButton, INVENTORY);
	dropButton = new Button( 420, 185, 520, 210, strdup("Drop Item") );
	cards->addWidget(dropButton, INVENTORY);
	fixButton = new Button( 420, 215, 520, 240, strdup("Fix Item") );
	cards->addWidget(fixButton, INVENTORY);
	removeCurseButton = new Button( 420, 245, 520, 270, strdup("Remove Curse") );
	cards->addWidget(removeCurseButton, INVENTORY);
	combineButton = new Button( 420, 275, 520, 300, strdup("Combine Item") );
	cards->addWidget(combineButton, INVENTORY);
	enchantButton = new Button( 420, 305, 520, 330, strdup("Enchant Item") );
	cards->addWidget(enchantButton, INVENTORY);
	identifyButton = new Button( 420, 335, 520, 360, strdup("Identify Item") );
	cards->addWidget(identifyButton, INVENTORY);

	// character info
	label = new Label(115, 45, strdup("Character Information"));
	label->setColor( 0.8f, 0.2f, 0, 1 );
	cards->addWidget(label, CHARACTER);
	nameLabel = new Label(115, 60);
	cards->addWidget(nameLabel, CHARACTER);
	classLabel = new Label(115, 75);
	cards->addWidget(classLabel, CHARACTER);
	levelLabel = new Label(115, 90);
	cards->addWidget(levelLabel, CHARACTER);
	expLabel = new Label(115, 105);
	cards->addWidget(expLabel, CHARACTER);
	hpLabel = new Label(115, 120);
	cards->addWidget(hpLabel, CHARACTER);
	label = new Label(115, 135, strdup("Current State:"));
	label->setColor( 0.8f, 0.2f, 0, 1 );
	cards->addWidget(label, CHARACTER);
	label = new Label(320, 45, strdup("Skills:"));
	label->setColor( 0.8f, 0.2f, 0, 1 );
	cards->addWidget(label, CHARACTER);
	stateList = new ScrollingList(115, 140, 150, 70);
	cards->addWidget(stateList, CHARACTER);
	skillList = new ScrollingList(320, 50, 190, 290);
	cards->addWidget(skillList, CHARACTER);

	// spellbook
	label = new Label(115, 45, strdup("Spellbook"));
	label->setColor( 0.8f, 0.2f, 0, 1 );
	cards->addWidget(label, SPELL);

	setSelectedPlayerAndMode(0, INVENTORY);
}

Inventory::~Inventory() {
}

bool Inventory::handleEvent(Widget *widget, SDL_Event *event) {
  if(widget == closeButton) mainWin->setVisible(false);
  else if(widget == player1Button) setSelectedPlayerAndMode(0, selectedMode);
  else if(widget == player2Button) setSelectedPlayerAndMode(1, selectedMode);
  else if(widget == player3Button) setSelectedPlayerAndMode(2, selectedMode);
  else if(widget == player4Button) setSelectedPlayerAndMode(3, selectedMode);
  else if(widget == inventoryButton) setSelectedPlayerAndMode(selected, INVENTORY);
  else if(widget == skillsButton) setSelectedPlayerAndMode(selected, CHARACTER);
  else if(widget == spellsButton) setSelectedPlayerAndMode(selected, SPELL);
  else if(widget == dropButton) {
	int itemIndex = invList->getSelectedLine();  
	if(itemIndex > -1 && 
	   scourge->getParty(selected)->getInventoryCount() > itemIndex) {
	  Item *item = scourge->getParty(selected)->removeInventory(itemIndex);
	  scourge->setMovingItem(item, 
							 scourge->getParty(selected)->getX(), 
							 scourge->getParty(selected)->getY(), 
							 scourge->getParty(selected)->getZ());
	  char message[120];
	  sprintf(message, "%s drops %s.", 
			  scourge->getParty(selected)->getName(),
			  item->getRpgItem()->getName());
	  scourge->getMap()->addDescription(strdup(message));	  
	  mainWin->setVisible(false);
	}
  } else if(widget == equipButton) {
	int itemIndex = invList->getSelectedLine();  
	if(itemIndex > -1 && 
	   scourge->getParty(selected)->getInventoryCount() > itemIndex) {
	  scourge->getParty(selected)->equipInventory(itemIndex);
	  // recreate list strings
	  int oldLine = invList->getSelectedLine();
	  setSelectedPlayerAndMode(selected, selectedMode);
	  invList->setSelectedLine(oldLine);
	}
  } else if(widget == invToButton[0]) {
	moveItemTo(0);
  } else if(widget == invToButton[1]) {
	moveItemTo(1);
  } else if(widget == invToButton[2]) {
	moveItemTo(2);
  } else if(widget == invToButton[3]) {
	moveItemTo(3);
  }
  return false;
}

void Inventory::moveItemTo(int playerIndex) {
  int itemIndex = invList->getSelectedLine();  
  if(itemIndex > -1 && 
	 scourge->getParty(selected)->getInventoryCount() > itemIndex) {
	if(playerIndex != selected) {
	  scourge->getParty(playerIndex)->
		addInventory(scourge->getParty(selected)->removeInventory(itemIndex));
	  // recreate strings in list
	  setSelectedPlayerAndMode(selected, selectedMode);
	}
  }
}

bool Inventory::handleEvent(SDL_Event *event) {
    switch(event->type) {
    case SDL_MOUSEBUTTONUP:
        break;     
    case SDL_KEYDOWN:
        switch(event->key.keysym.sym) {
        case SDLK_ESCAPE: 
		  hide();
		  return true;
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
  
  // show only the ui elements belonging to the current mode
  cards->setActiveCard(selectedMode);

  // arrange the gui
  switch(selectedMode) {
  case CHARACTER:
	nameLabel->setText(scourge->getParty(selected)->getName());
	classLabel->setText(scourge->getParty(selected)->getCharacter()->getName());
	sprintf(levelStr, "Level: %d", scourge->getParty(selected)->getLevel());
	levelLabel->setText(levelStr);
	sprintf(expStr, "Exp: %u", scourge->getParty(selected)->getExp());
	expLabel->setText(expStr);
	sprintf(hpStr, "HP: %d", scourge->getParty(selected)->getHp());
	hpLabel->setText(hpStr);
	stateCount = 0;
    for(int t = 0; t < Constants::STATE_MOD_COUNT; t++) {
      if(scourge->getParty(selected)->getStateMod(t)) {
        sprintf(stateLine[stateCount++], "%s", Constants::STATE_NAMES[t]);
      }
    }
	stateList->setLines(stateCount, (const char**)stateLine);
    for(int t = 0; t < Constants::SKILL_COUNT; t++) {
	  sprintf(skillLine[t], "%d - %s", 
			  scourge->getParty(selected)->getSkill(t), 
			  Constants::SKILL_NAMES[t]);
    }
	skillList->setLines(Constants::SKILL_COUNT, (const char**)skillLine);
	break;
  case INVENTORY:
    for(int t = 0; t < scourge->getParty(selected)->getInventoryCount(); t++) {
	  Item *item = scourge->getParty(selected)->getInventory(t);
	  int location = scourge->getParty(selected)->getEquippedIndex(t);
	  sprintf(pcInvText[t], "%s(A:%d,S:%d,Q:%d,W:%d) %s", 
			  (location > -1 ? " *" : "   "),
			  item->getRpgItem()->getAction(), item->getRpgItem()->getSpeed(), item->getRpgItem()->getQuality(), item->getRpgItem()->getWeight(),
			  item->getRpgItem()->getName());
    }
	for(int t = scourge->getParty(selected)->getInventoryCount(); 
		t < MAX_INVENTORY_SIZE; t++) {
	  strcpy(pcInvText[t], "");
	}
	invList->setLines(scourge->getParty(selected)->getInventoryCount(), 
					  (const char **)pcInvText);
	for(int i = 0; i < Character::INVENTORY_COUNT; i++) {
	  Item *item = scourge->getParty(selected)->getEquippedInventory(i);
	  invEquipLabel[i]->setText((char *)(item ? item->getRpgItem()->getName() : NULL));
	}
	break;
  case SPELL:
	break;
  case LOG:
	break;
  }
}

void Inventory::drawInventory() {
  // draw the characters on top of the UI
  glColor4f(1.0f, 1.0f, 0.4f, 1.0f);
  int h = 120;
  int y;  
  for(int i = 0; i < 4; i++) {

	// why do I need these 2 lines? Otherwise the models go behind the window
	glDisable(GL_DEPTH_TEST);
	glEnable(GL_DEPTH_TEST);

	glPushMatrix();
	glLoadIdentity();

	glTranslatef( mainWin->getX(), mainWin->getY() + Window::TOP_HEIGHT, mainWin->getZ() + 5 );
	glTranslatef( 20, 10 + i * h + 90, 300);

	glRotatef(90, 1, 0, 0);
	glScalef(0.8, 0.8, 0.8);	
	glEnable( GL_TEXTURE_2D );
	glColor4f( 1, 1, 1, 1 );
	mainWin->scissorToWindow();
	scourge->getParty(i)->draw();
	glDisable( GL_SCISSOR_TEST );
	glDisable( GL_TEXTURE_2D );
	glPopMatrix();

  }      
}

