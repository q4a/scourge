

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
}

Inventory::~Inventory() {
}

void Inventory::show() {
    scourge->getGui()->pushWindows();
    createGui();
    scourge->getSDLHandler()->pushHandlers(this, this);
}

void Inventory::createGui() {
    win = scourge->getGui()->addWindow(1, 1,
                                       scourge->getSDLHandler()->getScreen()->w - 1,
                                       scourge->getSDLHandler()->getScreen()->h - 1,
                                       &Gui::drawInventory);
    // add some active regions
    scourge->getGui()->addActiveRegion(0, 0, 105, 120, Constants::INV_PLAYER_0, this);
    scourge->getGui()->addActiveRegion(0, 120, 105, 240, Constants::INV_PLAYER_1, this);
    scourge->getGui()->addActiveRegion(0, 240, 105, 360, Constants::INV_PLAYER_2, this);
    scourge->getGui()->addActiveRegion(0, 360, 105, 480, Constants::INV_PLAYER_3, this);

    scourge->getGui()->addActiveRegion(0, scourge->getSDLHandler()->getScreen()->h - 30, 
									   160, scourge->getSDLHandler()->getScreen()->h, 
									   Constants::ESCAPE, this);
    scourge->getGui()->addActiveRegion(320, scourge->getSDLHandler()->getScreen()->h - 30, 
									   480, scourge->getSDLHandler()->getScreen()->h, 
									   Constants::INV_MODE_PROPERTIES, this);
    scourge->getGui()->addActiveRegion(160, scourge->getSDLHandler()->getScreen()->h - 30, 
									   320, scourge->getSDLHandler()->getScreen()->h, 
									   Constants::INV_MODE_INVENTORY, this);
    scourge->getGui()->addActiveRegion(480, scourge->getSDLHandler()->getScreen()->h - 30, 
									   640, scourge->getSDLHandler()->getScreen()->h, 
									   Constants::INV_MODE_SPELLS, this);
    scourge->getGui()->addActiveRegion(640, scourge->getSDLHandler()->getScreen()->h - 30, 
									   800, scourge->getSDLHandler()->getScreen()->h, 
									   Constants::INV_MODE_LOG, this);

	setSelectedPlayerAndMode(0, INVENTORY);
}

void Inventory::drawView(SDL_Surface *screen) {
    glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
    scourge->getGui()->drawWindows();
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
            scourge->getGui()->popWindows();
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

  // arrange the gui
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
  int ypos = 50;
  int height = 30;
  int xpos = 530;
  int width = 100;
  switch(mode) {
  case CHARACTER:
    skillList = scourge->getGui()->   
	  addScrollingList(270, 34, 500, 400,
					   Constants::SKILL_LIST);
	break;
  case INVENTORY:
    itemList = scourge->getGui()->   
	  addScrollingList(120, 50, 510, 550,
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
}

void Inventory::drawParty() {
  int h = 120;
  int y;  
  for(int i = 0; i < 4; i++) {
	glPushMatrix();
	glLoadIdentity();
	glTranslatef( 20, 10 + i * h + 90, 100);
	glRotatef(90, 1, 0, 0);
	glScalef( 1.2f, 0, 0.8f );
	
	glDisable(GL_DEPTH_TEST);
	glColorMask(0,0,0,0);
	glEnable(GL_STENCIL_TEST);
	glStencilOp(GL_REPLACE, GL_REPLACE, GL_REPLACE);
	glStencilFunc(GL_ALWAYS, 1, 0xffffffff);
	scourge->getParty(i)->draw();
	glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
	glEnable(GL_DEPTH_TEST);
	
	glStencilFunc(GL_EQUAL, 1, 0xffffffff);  // draw if stencil=0
	// GL_INCR makes sure to only draw shadow once
	glStencilOp(GL_KEEP, GL_KEEP, GL_INCR);	
	
	glEnable( GL_BLEND );
	glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );
	glColor4f( 0.1f, 0, 0.15f, 0.3f );
	scourge->getParty(i)->draw();
	glDisable( GL_BLEND );
	glDisable(GL_STENCIL_TEST); 
	glPopMatrix();
	
	
	glPushMatrix();
	glLoadIdentity();
	glTranslatef( 24, 10 + i * h + 90, 200);
	glRotatef(90, 1, 0, 0);
	glScalef(0.7, 0.7, 0.7);
	
	glEnable( GL_TEXTURE_2D );
	glColor4f( 1, 1, 1, 1 );
	scourge->getParty(i)->draw();
	glDisable( GL_TEXTURE_2D );
	glPopMatrix();
	
	scourge->getSDLHandler()->
	  texPrint(10, 10 + i * h + 100, "%s", 
			   scourge->getParty(i)->getPC()->getName());
	
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
    scourge->getSDLHandler()->texPrint(120, 40, "Inventory:");    
    glColor4f(1.0f, 0.6f, 0.4f, 1.0f);
    for(int t = 0; t < scourge->getParty(selected)->getPC()->getInventoryCount(); t++) {
	  RpgItem *item = scourge->getParty(selected)->getPC()->getInventory(t);
	  sprintf(pcInvText[t], "(A:%d) (S:%d) (Q:%d) (W: %d) %s", 
			  item->getAction(), item->getSpeed(), item->getQuality(), item->getWeight(),
			  item->getName());
    }
	for(int t = scourge->getParty(selected)->getPC()->getInventoryCount(); 
		t < MAX_INVENTORY_SIZE; t++) {
	  sprintf(pcInvText[t], "");
	}
    scourge->getGui()->drawScrollingList(itemList, Constants::SKILL_COUNT, (const char**)pcInvText);
	char name[80];
	for(int i = 0; i < 4; i++) {
	  sprintf(name, "to %s", scourge->getParty(i)->getPC()->getName());
	  scourge->getGui()->outlineActiveRegion(Constants::MOVE_ITEM_TO_PLAYER_0 + i, name);
	}
	scourge->getGui()->outlineActiveRegion(Constants::EQUIP_ITEM, "Equip Item");
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


