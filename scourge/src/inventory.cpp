

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
						  100, 50, 420, 505, 
						  strdup("Party Information"), 
						  scourge->getShapePalette()->getGuiTexture() );
	player1Button  = mainWin->createButton( 0, 30, 105, 60, scourge->getParty(0)->getName(), true);
	player2Button  = mainWin->createButton( 0, 60, 105, 90, scourge->getParty(1)->getName(), true);
	player3Button  = mainWin->createButton( 0, 90, 105, 120, scourge->getParty(2)->getName(), true );
	player4Button  = mainWin->createButton( 0, 120, 105, 150, scourge->getParty(3)->getName(), true );
	inventoryButton = mainWin->createButton( 105,0, 210, 30, strdup("Inventory"), true);
	skillsButton   = mainWin->createButton( 210,0, 315, 30, strdup("Skills"), true);
	spellsButton   = mainWin->createButton( 315,0, 420, 30, strdup("Spells"), true);
	cards = new CardContainer(mainWin);

	// inventory page	
	cards->createLabel(115, 280, strdup("Inventory:"), INVENTORY, Constants::RED_COLOR);	
	inventoryWeightLabel = cards->createLabel(190, 280, NULL, INVENTORY);
	cards->createLabel(115, 45, strdup("Equipped Items:"), INVENTORY, Constants::RED_COLOR);
    
	for(int i = 0; i < Character::INVENTORY_COUNT; i++) {
	  Item *item = scourge->getParty(selected)->getEquippedInventory(i);
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
	equipButton    = mainWin->createButton( 0, yy, 105, yy + 30, strdup("Don/Doff"), INVENTORY);
	yy+=30;
	fixButton      = mainWin->createButton( 0, yy, 105, yy + 30, strdup("Fix Item"), INVENTORY);
	yy+=30;
	removeCurseButton = mainWin->createButton( 0, yy, 105, yy + 30, strdup("Remove Curse"), INVENTORY );
	yy+=30;
	combineButton  = mainWin->createButton( 0, yy, 105, yy + 30, strdup("Combine Item"), INVENTORY );
	yy+=30;
	enchantButton  = mainWin->createButton( 0, yy, 105, yy + 30, strdup("Enchant Item"), INVENTORY );
	yy+=30;
	identifyButton = mainWin->createButton( 0, yy, 105, yy + 30, strdup("Identify Item"), INVENTORY );
	yy+=30;
	openButton     = mainWin->createButton( 0, yy, 105, yy + 30, 
							 Constants::getMessage(Constants::OPEN_CONTAINER_LABEL), INVENTORY );	
    yy+=30;
    eatDrinkButton = mainWin->createButton( 0, yy, 105, yy + 30, strdup("Eat/Drink"), INVENTORY );
	
    // character info
	cards->createLabel(115, 45, strdup("Character Information"), CHARACTER, Constants::RED_COLOR);	
	nameAndClassLabel = cards->createLabel(115, 60, NULL, CHARACTER);
	levelLabel     = cards->createLabel(115, 75, NULL, CHARACTER);
	expLabel       = cards->createLabel(115, 90, NULL, CHARACTER);
	hpLabel        = cards->createLabel(115, 105, NULL, CHARACTER);
	thirstLabel    = cards->createLabel(115, 120, NULL, CHARACTER);
	hungerLabel    = cards->createLabel(220, 120, NULL, CHARACTER);
	
	cards->createLabel(115, 135, strdup("Current State:"), CHARACTER, Constants::RED_COLOR);
	stateList = new ScrollingList(115, 140, 290, 70);
	cards->addWidget(stateList, CHARACTER);

	cards->createLabel(115, 225, strdup("Skills:"), CHARACTER, Constants::RED_COLOR);
	skillList = new ScrollingList(115, 230, 290, 220);
	cards->addWidget(skillList, CHARACTER);

	// spellbook
	cards->createLabel(115, 45, strdup("Spellbook"), SPELL, Constants::RED_COLOR);

	setSelectedPlayerAndMode(0, INVENTORY);
}

Inventory::~Inventory() {
}

bool Inventory::handleEvent(Widget *widget, SDL_Event *event) {
	if(widget == mainWin->closeButton) mainWin->setVisible(false);
	else if(widget == player1Button) setSelectedPlayerAndMode(0, selectedMode);
	else if(widget == player2Button) setSelectedPlayerAndMode(1, selectedMode);
	else if(widget == player3Button) setSelectedPlayerAndMode(2, selectedMode);
	else if(widget == player4Button) setSelectedPlayerAndMode(3, selectedMode);
	else if(widget == inventoryButton) setSelectedPlayerAndMode(selected, INVENTORY);
	else if(widget == skillsButton)	setSelectedPlayerAndMode(selected, CHARACTER);
	else if(widget == spellsButton)	setSelectedPlayerAndMode(selected, SPELL);
	else if(widget == openButton) {
		int itemIndex = invList->getSelectedLine();  
		if(itemIndex > -1) {
			Item *item = scourge->getParty(selected)->getInventory(itemIndex);
			if(item->getRpgItem()->getType() == RpgItem::CONTAINER) {
				scourge->openContainerGui(item);
			}
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
	} else if(widget == eatDrinkButton) {
	   int itemIndex = invList->getSelectedLine();  
	   if(itemIndex > -1 && 
			 scourge->getParty(selected)->getInventoryCount() > itemIndex) {
			if(scourge->getParty(selected)->eatDrink(itemIndex)){
                scourge->getParty(selected)->removeInventory(itemIndex);                
			}
			// refresh screen
            setSelectedPlayerAndMode(selected, INVENTORY);
		}					   	   	   	      	
  
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
  
  // show only the ui elements belonging to the current mode
  cards->setActiveCard(selectedMode);   

  // arrange the gui
  Creature * selectedP = scourge->getParty(selected);
  switch(selectedMode) {
  case CHARACTER:       	
    sprintf(nameAndClassStr, "%s, %s", selectedP->getName(), selectedP->getCharacter()->getName());
	nameAndClassLabel->setText(nameAndClassStr);	
	sprintf(levelStr, "Level: %d", selectedP->getLevel());
	levelLabel->setText(levelStr);
	sprintf(expStr, "Exp: %u", selectedP->getExp());
	expLabel->setText(expStr);
	sprintf(hpStr, "HP: %d / %d", selectedP->getHp(), selectedP->getCharacter()->getStartingHp());
	hpLabel->setText(hpStr);
	sprintf(thirstStr, "Thirst : %d / 10", selectedP->getThirst());
	thirstLabel->setText(thirstStr);
	sprintf(hungerStr, "Hunger : %d / 10", selectedP->getHunger());
	hungerLabel->setText(hungerStr);
	stateCount = 0;
    for(int t = 0; t < Constants::STATE_MOD_COUNT; t++) {
      if(selectedP->getStateMod(t)) {
        sprintf(stateLine[stateCount++], "%s", Constants::STATE_NAMES[t]);
      }
    }
	stateList->setLines(stateCount, (const char**)stateLine);
    for(int t = 0; t < Constants::SKILL_COUNT; t++) {
	  sprintf(skillLine[t], "%d - %s", 
			  selectedP->getSkill(t), 
			  Constants::SKILL_NAMES[t]);
    }
	skillList->setLines(Constants::SKILL_COUNT, (const char**)skillLine);
	break;
	
  case INVENTORY:
    sprintf(inventoryWeightStr, " (Total : %2.2fkg / %2.2fkg)", 
            selectedP->getInventoryWeight(), selectedP->getMaxInventoryWeight());     
    inventoryWeightLabel->setText(inventoryWeightStr);
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
	break;
  case LOG:
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

	cerr << "z=" << mainWin->getZ() << endl;
	glTranslatef( mainWin->getX(), mainWin->getY() + Window::TOP_HEIGHT, mainWin->getZ() + 200 );
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

void Inventory::receive(Widget *widget) {
  if(scourge->getMovingItem()) {
	if(scourge->getParty(selected)->addInventory(scourge->getMovingItem())) {
	  // message: the player accepted the item
	  char message[120];
	  sprintf(message, "%s picks up %s.", 
			  scourge->getParty(selected)->getName(),
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
	scourge->getMap()->addDescription(message);
	setSelectedPlayerAndMode(selected, INVENTORY);
  }
}

void Inventory::refresh() {
  setSelectedPlayerAndMode(selected, INVENTORY);
}
