

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
    this->invText = (char**)malloc(Constants::SKILL_COUNT * sizeof(char*));
    for(int i = 0; i < Constants::SKILL_COUNT; i++) {
        this->invText[i] = (char*)malloc(120 * sizeof(char));
    }
    this->pcInvText = (char**)malloc(MAX_INVENTORY_SIZE * sizeof(char*));
    for(int i = 0; i < MAX_INVENTORY_SIZE; i++) {
        this->pcInvText[i] = (char*)malloc(120 * sizeof(char));
    }
    selected = selectedMode = 0;

	mainWin = new Window( scourge->getSDLHandler(),
						  100, 50, 525, 505, 
						  "Party Information", 
						  scourge->getShapePalette()->getGuiTexture() );
	player1Button = new Button( 0, 0, 105, 120, scourge->getParty(0)->getPC()->getName() );
	player1Button->setLabelPosition(Button::BOTTOM);	
	player1Button->setToggle(true);
	mainWin->addWidget((Widget*)player1Button);
	player2Button = new Button( 0, 120, 105, 240, scourge->getParty(1)->getPC()->getName() );
	player2Button->setLabelPosition(Button::BOTTOM);
	player2Button->setToggle(true);
	mainWin->addWidget((Widget*)player2Button);
	player3Button = new Button( 0, 240, 105, 360, scourge->getParty(2)->getPC()->getName() );
	player3Button->setLabelPosition(Button::BOTTOM);
	player3Button->setToggle(true);
	mainWin->addWidget((Widget*)player3Button);
	player4Button = new Button( 0,360, 105, 480, scourge->getParty(3)->getPC()->getName() );
	player4Button->setLabelPosition(Button::BOTTOM);
	player4Button->setToggle(true);
	mainWin->addWidget((Widget*)player4Button);
	inventoryButton = new Button( 105,0, 210, 30, "Inventory" );
	inventoryButton->setToggle(true);
	mainWin->addWidget((Widget*)inventoryButton);
	skillsButton = new Button( 210,0, 315, 30, "Skills" );
	skillsButton->setToggle(true);
	mainWin->addWidget((Widget*)skillsButton);
	spellsButton = new Button( 315,0, 420, 30, "Spells" );
	spellsButton->setToggle(true);
	mainWin->addWidget((Widget*)spellsButton);
	closeButton = new Button( 420,0, 525, 30, "Close" );
	mainWin->addWidget((Widget*)closeButton);

	cards = new CardContainer(mainWin);

	// inventory page
	Label *label = new Label(115, 270, "Inventory:");
	label->setColor( 0.8f, 0.2f, 0, 1 );
	cards->addWidget(label, INVENTORY);
	label = new Label(115, 45, "Equipped Items:");
	label->setColor( 0.8f, 0.2f, 0, 1 );
	cards->addWidget(label, INVENTORY);
	for(int i = 0; i < PlayerChar::INVENTORY_COUNT; i++) {
	  RpgItem *item = scourge->getParty(selected)->getPC()->getEquippedInventory(i);
	  invEquipLabel[i] = new Label(300, 60 + (i * 15), (char *)(item ? item->getName() : ""));
	  cards->addWidget(invEquipLabel[i], INVENTORY);
	}
	for(int i = 0; i < PlayerChar::INVENTORY_COUNT; i++) {
	  label = new Label(115, 60 + (i * 15), PlayerChar::inventory_location[i]);
	  cards->addWidget(label, INVENTORY);
	}
	/*
    for(int t = 0; t < scourge->getParty(selected)->getPC()->getInventoryCount(); t++) {
	  RpgItem *item = scourge->getParty(selected)->getPC()->getInventory(t);
	  int location = scourge->getParty(selected)->getPC()->getEquippedIndex(t);
	  sprintf(pcInvText[t], "%s (A:%d) (S:%d) (Q:%d) (W: %d) %s", 
			  (location > -1 ? "<equipped> " : "                "),
			  item->getAction(), item->getSpeed(), item->getQuality(), item->getWeight(),
			  item->getName());
    }
	for(int t = scourge->getParty(selected)->getPC()->getInventoryCount(); 
		t < MAX_INVENTORY_SIZE; t++) {
	  strcpy(pcInvText[t], "");
	}
    scourge->getGui()->drawScrollingList(itemList, Constants::SKILL_COUNT, (const char**)pcInvText);
	*/
	
	char name[80];
	for(int i = 0; i < 4; i++) {
	  sprintf(name, "to %s", scourge->getParty(i)->getPC()->getName());
	  invToButton[i] = new Button( 420, 35 + (i * 30), 520, 35 + (i * 30) + 25, name );
	  cards->addWidget( invToButton[i], INVENTORY );
	}
	equipButton = new Button( 420, 155, 520, 180, "Don/Doff" );
	cards->addWidget(equipButton, INVENTORY);
	dropButton = new Button( 420, 185, 520, 210, "Drop Item" );
	cards->addWidget(dropButton, INVENTORY);
	fixButton = new Button( 420, 215, 520, 240, "Fix Item" );
	cards->addWidget(fixButton, INVENTORY);
	removeCurseButton = new Button( 420, 245, 520, 270, "Remove Curse" );
	cards->addWidget(removeCurseButton, INVENTORY);
	combineButton = new Button( 420, 275, 520, 300, "Combine Item" );
	cards->addWidget(combineButton, INVENTORY);
	enchantButton = new Button( 420, 305, 520, 330, "Enchant Item" );
	cards->addWidget(enchantButton, INVENTORY);
	identifyButton = new Button( 420, 335, 520, 360, "Identify Item" );
	cards->addWidget(identifyButton, INVENTORY);


	label = new Label(115, 45, "Character Information");
	label->setColor( 0.8f, 0.2f, 0, 1 );
	cards->addWidget(label, CHARACTER);

	label = new Label(115, 45, "Spellbook");
	label->setColor( 0.8f, 0.2f, 0, 1 );
	cards->addWidget(label, SPELL);

	setSelectedPlayerAndMode(0, INVENTORY);
}

Inventory::~Inventory() {
}

void Inventory::drawView(SDL_Surface *screen) {
    glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
	//    scourge->getGui()->drawWindows();
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
  return false;
  }

bool Inventory::handleEvent(SDL_Event *event) {
    switch(event->type) {
    case SDL_MOUSEBUTTONUP:
        if(event->button.button) {
            if(processMouseClick(event->button.x, event->button.y, event->button.button)) {
                scourge->getGui()->popWindows();
                return true;
            }
        }
        break;     
    case SDL_KEYDOWN:
        switch(event->key.keysym.sym) {
        case SDLK_ESCAPE: 
		  //scourge->getGui()->popWindows();
		  mainWin->setVisible(false);
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
  /*
  scourge->getGui()->removeAllScrollingLists();
  scourge->getGui()->removeActiveRegion(Constants::MOVE_ITEM_TO_PLAYER_0);
  scourge->getGui()->removeActiveRegion(Constants::MOVE_ITEM_TO_PLAYER_1);
  scourge->getGui()->removeActiveRegion(Constants::MOVE_ITEM_TO_PLAYER_2);
  scourge->getGui()->removeActiveRegion(Constants::MOVE_ITEM_TO_PLAYER_3);
  scourge->getGui()->removeActiveRegion(Constants::EQUIP_ITEM);
  scourge->getGui()->removeActiveRegion(Constants::DROP_ITEM);
  scourge->getGui()->removeActiveRegion(Constants::FIX_ITEM);
  scourge->getGui()->removeActiveRegion(Constants::ENCHANT_ITEM);
  scourge->getGui()->removeActiveRegion(Constants::REMOVE_CURSE_ITEM);
  scourge->getGui()->removeActiveRegion(Constants::COMBINE_ITEM);
  scourge->getGui()->removeActiveRegion(Constants::IDENTIFY_ITEM);
  */
  int ypos = 20;
  int height = 30;
  int xpos = 670;
  int width = 100;
  switch(mode) {
  case CHARACTER:
	/*
    skillList = scourge->getGui()->   
	  addScrollingList(270, 34, 500, 400,
					   Constants::SKILL_LIST);
	*/
	break;
  case INVENTORY:
	/*
    itemList = scourge->getGui()->   
	  addScrollingList(120, 270, 650, 550,
					   Constants::ITEM_LIST);
	for(int i = 0; i < 4; i++) {
	  scourge->getGui()->addActiveRegion(xpos, ypos, xpos + width, ypos + height, 
										 Constants::MOVE_ITEM_TO_PLAYER_0 + i, this);
	  ypos += (height + 10);
	}
	scourge->getGui()->addActiveRegion(xpos, ypos, xpos + width, ypos + height, 
									   Constants::EQUIP_ITEM, this);
	ypos += (height + 10);
	scourge->getGui()->addActiveRegion(xpos, ypos, xpos + width, ypos + height, 
									   Constants::DROP_ITEM, this);
	ypos += (height + 10);
	scourge->getGui()->addActiveRegion(xpos, ypos, xpos + width, ypos + height, 
									   Constants::FIX_ITEM, this);
	ypos += (height + 10);
	scourge->getGui()->addActiveRegion(xpos, ypos, xpos + width, ypos + height, 
									   Constants::ENCHANT_ITEM, this);
	ypos += (height + 10);
	scourge->getGui()->addActiveRegion(xpos, ypos, xpos + width, ypos + height, 
									   Constants::REMOVE_CURSE_ITEM, this);
	ypos += (height + 10);
	scourge->getGui()->addActiveRegion(xpos, ypos, xpos + width, ypos + height, 
									   Constants::COMBINE_ITEM, this);
	ypos += (height + 10);
	scourge->getGui()->addActiveRegion(xpos, ypos, xpos + width, ypos + height, 
									   Constants::IDENTIFY_ITEM, this);
	ypos += (height + 10);
	*/
	break;
  case SPELL:
	break;
  case LOG:
	break;
  }
}

bool Inventory::processMouseClick(int x, int y, int button) {
    int region = scourge->getGui()->testActiveRegions(x, y);
    if(region == Constants::INV_PLAYER_0 || region == Constants::INV_PLAYER_1 ||
       region == Constants::INV_PLAYER_2 || region == Constants::INV_PLAYER_3) {
		setSelectedPlayerAndMode(region, selectedMode);
    } else if(region == Constants::INV_MODE_PROPERTIES || 
			  region == Constants::INV_MODE_INVENTORY ||
              region == Constants::INV_MODE_SPELLS || 
			  region == Constants::INV_MODE_LOG) {
		setSelectedPlayerAndMode(selected, region - Constants::INV_MODE_INVENTORY);
    } else if(region == Constants::DROP_ITEM) {
	  int itemIndex = scourge->getGui()->getLineSelected(Constants::ITEM_LIST);  
	  if(itemIndex > -1 && 
		 scourge->getParty(selected)->getPC()->getInventoryCount() > itemIndex) {
		RpgItem *item = scourge->getParty(selected)->getPC()->removeInventory(itemIndex);
		scourge->setMovingItem(item->getIndex(), 
							   scourge->getParty(selected)->getX(), 
							   scourge->getParty(selected)->getY(), 
							   scourge->getParty(selected)->getZ());
		char message[120];
		sprintf(message, "%s drops %s.", 
				scourge->getParty(selected)->getPC()->getName(),
				item->getName());
		scourge->getMap()->addDescription(strdup(message));
		return true;
	  }
    } else if(region == Constants::EQUIP_ITEM) {
	  int itemIndex = scourge->getGui()->getLineSelected(Constants::ITEM_LIST);  
	  if(itemIndex > -1 && 
		 scourge->getParty(selected)->getPC()->getInventoryCount() > itemIndex) {
		scourge->getParty(selected)->getPC()->equipInventory(itemIndex);
	  }
    } else if(region == Constants::ESCAPE) {
        return true;
    } else if(region >= Constants::MOVE_ITEM_TO_PLAYER_0 && 
			  region <= Constants::MOVE_ITEM_TO_PLAYER_3) {
	  int itemIndex = scourge->getGui()->getLineSelected(Constants::ITEM_LIST);  
	  if(itemIndex > -1 && 
		 scourge->getParty(selected)->getPC()->getInventoryCount() > itemIndex) {
		int index = region - Constants::MOVE_ITEM_TO_PLAYER_0;
		if(index != selected) {
		  scourge->getParty(index)->getPC()->
			addInventory(scourge->getParty(selected)->getPC()->removeInventory(itemIndex));
		}
	  }
	}
    return false;
}

void Inventory::drawInventory() {
    glColor4f(1.0f, 1.0f, 0.4f, 1.0f);
    drawParty();

	/*
	glColor4f(1.0f, 1.0f, 0.4f, 1.0f);
	scourge->getGui()->outlineActiveRegion(Constants::INV_PLAYER_0);
	scourge->getGui()->outlineActiveRegion(Constants::INV_PLAYER_1);
	scourge->getGui()->outlineActiveRegion(Constants::INV_PLAYER_2);
	scourge->getGui()->outlineActiveRegion(Constants::INV_PLAYER_3);

    glColor4f(1.0f, 1.0f, 0.4f, 1.0f);
    drawModeButtons();

	glDisable(GL_DEPTH_TEST);
	scourge->getGui()->outlineActiveRegion(Constants::INV_MODE_PROPERTIES, "Skills");
	scourge->getGui()->outlineActiveRegion(Constants::INV_MODE_INVENTORY, "Inventory");
	scourge->getGui()->outlineActiveRegion(Constants::INV_MODE_SPELLS, "Spells");
	scourge->getGui()->outlineActiveRegion(Constants::INV_MODE_LOG, "Accomplishments");
	scourge->getGui()->outlineActiveRegion(Constants::ESCAPE, "Back");
	glEnable(GL_DEPTH_TEST);
    
    switch(selectedMode) {
    case CHARACTER:
	  drawCharacterInfo(); break;
    case INVENTORY:
	  drawInventoryInfo(); break;
    case SPELL:
	  drawSpellInfo(); break;
    case LOG:
	  drawLogInfo(); break;
    }
	*/
}

void Inventory::drawParty() {
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

	/*	
	if(selected == i) {
	  y = 10 + i * h;
	  glColor4f(0.6f, 0.4f, 0.2f, 0.5f);
	  glBlendFunc(GL_SRC_ALPHA, GL_ONE);
	  glEnable(GL_BLEND);
	  glBegin (GL_QUADS);
	  glVertex3i (0, y - 10, 150);
	  glVertex3i (0, y + h - 11, 150);      
	  glVertex3i (105, y + h - 11, 150);
	  glVertex3i (105, y - 10, 150);
	  glEnd ();
	  glDisable(GL_BLEND);
	  glColor4f(1.0f, 1.0f, 0.4f, 1.0f);
	}
	*/
  }      
}

void Inventory::drawModeButtons() {
  for(int i = 0; i < 4; i++) {
	if(selectedMode == i) {
	  glColor4f(0.6f, 0.4f, 0.2f, 0.5f);
	  glBlendFunc(GL_SRC_ALPHA, GL_ONE);
	  glEnable(GL_BLEND);
	  glBegin (GL_QUADS);
	  glVertex3i ((i + 1) * 160, scourge->getSDLHandler()->getScreen()->h - 30, 10);
	  glVertex3i ((i + 1) * 160, scourge->getSDLHandler()->getScreen()->h, 10);      
	  glVertex3i ((i + 2) * 160, scourge->getSDLHandler()->getScreen()->h, 10);
	  glVertex3i ((i + 2) * 160, scourge->getSDLHandler()->getScreen()->h - 30, 10);
	  glEnd ();
	  glDisable(GL_BLEND);
	  glColor4f(1.0f, 1.0f, 0.4f, 1.0f);
	}
  }
}

void Inventory::drawCharacterInfo() {
    int xx = 110, yy = 0;
    int x = 10, y = 10;
    int i = selected;

    x += xx;
    y += yy;
    glColor4f(1.0f, 1.0f, 0.4f, 1.0f);
    scourge->getSDLHandler()->texPrint((float)x, (float)(y + 10), "%s", 
									   scourge->getParty(i)->getPC()->getName());
    glColor4f(1.0f, 0.6f, 0.4f, 1.0f);
    scourge->getSDLHandler()->texPrint((float)x, (float)(y + 24), "%s", 
									   scourge->getParty(i)->getPC()->getCharacter()->getName());
    scourge->getSDLHandler()->texPrint((float)x, (float)(y + 38), "Level: %d", 
									   scourge->getParty(i)->getPC()->getLevel());
    scourge->getSDLHandler()->texPrint((float)x, (float)(y + 52), "Exp: %u", 
									   scourge->getParty(i)->getPC()->getExp());
    scourge->getSDLHandler()->texPrint((float)x, (float)(y + 66), "HP: %d", 
									   scourge->getParty(i)->getPC()->getHp());


    y = yy + 100;
	//    x = xx + GUI_PLAYER_INFO_W / 2 + 10;
    glColor4f(1.0f, 1.0f, 0.4f, 1.0f);
    scourge->getSDLHandler()->texPrint((float)x, (float)(y), "Current State:");    
    glColor4f(1.0f, 0.6f, 0.4f, 1.0f);

    y += 14;
    for(int t = 0; t < Constants::STATE_MOD_COUNT; t++) {
      if(scourge->getParty(i)->getPC()->getStateMod(t)) {
        scourge->getSDLHandler()->texPrint((float)x, (float)(y), "%s", Constants::STATE_NAMES[t]);
        y += 14;
      }
    }

    glColor4f(1.0f, 1.0f, 0.4f, 1.0f);
    scourge->getSDLHandler()->texPrint(270, 20, "Skills:");    
    glColor4f(1.0f, 0.6f, 0.4f, 1.0f);
    for(int t = 0; t < Constants::SKILL_COUNT; t++) {
        sprintf(invText[t], "%d - %s", scourge->getParty(i)->getPC()->getSkill(t), Constants::SKILL_NAMES[t]);
    }
    scourge->getGui()->drawScrollingList(skillList, Constants::SKILL_COUNT, (const char**)invText);
}


void Inventory::drawInventoryInfo() {
    glColor4f(1.0f, 1.0f, 0.4f, 1.0f);
    scourge->getSDLHandler()->texPrint(120, 260, "Inventory:");    
    scourge->getSDLHandler()->texPrint(120, 15, "Equipped items:");
	for(int i = 0; i < PlayerChar::INVENTORY_COUNT; i++) {
	  RpgItem *item = scourge->getParty(selected)->getPC()->getEquippedInventory(i);
	  scourge->getSDLHandler()->
		texPrint(300, 30 + (i * 15), "%s", 
				 (item ? item->getName() : ""));
	}

    glColor4f(1.0f, 0.6f, 0.4f, 1.0f);
	for(int i = 0; i < PlayerChar::INVENTORY_COUNT; i++) {
	  scourge->getSDLHandler()->
		texPrint(160, 30 + (i * 15), "On %s:", 
				 PlayerChar::inventory_location[i]);
	}

    for(int t = 0; t < scourge->getParty(selected)->getPC()->getInventoryCount(); t++) {
	  RpgItem *item = scourge->getParty(selected)->getPC()->getInventory(t);
	  int location = scourge->getParty(selected)->getPC()->getEquippedIndex(t);
	  sprintf(pcInvText[t], "%s (A:%d) (S:%d) (Q:%d) (W: %d) %s", 
			  (location > -1 ? "<equipped> " : "                "),
			  item->getAction(), item->getSpeed(), item->getQuality(), item->getWeight(),
			  item->getName());
    }
	for(int t = scourge->getParty(selected)->getPC()->getInventoryCount(); 
		t < MAX_INVENTORY_SIZE; t++) {
	  strcpy(pcInvText[t], "");
	}
    scourge->getGui()->drawScrollingList(itemList, Constants::SKILL_COUNT, (const char**)pcInvText);
	char name[80];
	for(int i = 0; i < 4; i++) {
	  sprintf(name, "to %s", scourge->getParty(i)->getPC()->getName());
	  scourge->getGui()->outlineActiveRegion(Constants::MOVE_ITEM_TO_PLAYER_0 + i, name);
	}
	scourge->getGui()->outlineActiveRegion(Constants::EQUIP_ITEM, "Don/Doff");
	scourge->getGui()->outlineActiveRegion(Constants::DROP_ITEM, "Drop Item");
	scourge->getGui()->outlineActiveRegion(Constants::FIX_ITEM, "Fix Item");
	scourge->getGui()->outlineActiveRegion(Constants::REMOVE_CURSE_ITEM, "Remove Curse");
	scourge->getGui()->outlineActiveRegion(Constants::COMBINE_ITEM, "Combine Item");
	scourge->getGui()->outlineActiveRegion(Constants::ENCHANT_ITEM, "Enchant Item");
	scourge->getGui()->outlineActiveRegion(Constants::IDENTIFY_ITEM, "Identify Item");
}


void Inventory::drawSpellInfo() {
}
void Inventory::drawLogInfo() {
}


