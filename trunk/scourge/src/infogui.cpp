/***************************************************************************
                          infogui.cpp  -  description
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

#include "infogui.h"

InfoGui::InfoGui(Scourge *scourge) {
  this->scourge = scourge;

  int width = 220;
  int height = 350;

  int x = (scourge->getSDLHandler()->getScreen()->w - width) / 2;
  int y = (scourge->getSDLHandler()->getScreen()->h - height) / 2;

  win = scourge->createWindow( x, y, width, height, Constants::getMessage(Constants::INFO_GUI_TITLE) );
  int bx = width / 2 - 52;
  int by = height - (30 + Window::BOTTOM_HEIGHT + Window::TOP_HEIGHT);
  openButton = new Button( bx, by, bx + 105, by + 25, 
                           scourge->getShapePalette()->getHighlightTexture(), 
                           Constants::getMessage(Constants::CLOSE_LABEL) );
  win->addWidget((Widget*)openButton);

  win->createLabel(10, 10, strdup("Name:"), Constants::RED_COLOR);
  strcpy(name, "");
  nameLabel = new Label(10, 25, description, 35);
  win->addWidget(nameLabel);

  win->createLabel(10, 80, strdup("Detailed Description:"), Constants::RED_COLOR);
  strcpy(description, "");
  label = new Label(10, 95, description, 35);
  win->addWidget(label);
}

InfoGui::~InfoGui() {
  delete win;
}

void InfoGui::setItem(Item *item, int level) { 
  this->item = item; 
  this->setInfoDetailLevel( level );
}

void InfoGui::setInfoDetailLevel(int level) { 
  infoDetailLevel = level; 
  describe(); 
}

bool InfoGui::handleEvent(Widget *widget, SDL_Event *event) {
  if(widget == win->closeButton) {
    win->setVisible(false);
    return true;
  } else if(widget == openButton) {
    win->setVisible(false);
  }
  return false;
}

void InfoGui::drawWidget(Widget *w) {
}

void InfoGui::describe() {
  // describe item
  if(!item) return;
  item->getDetailedDescription(name);
  nameLabel->setText(name);

  // detailed description
  strcpy(description, item->getRpgItem()->getLongDesc());
  strcat(description, "|");

  char tmp[1000];
  if(item->getMagicAttrib()) {
    if(infoDetailLevel > (int)(100.0f * rand()/RAND_MAX)) {
      sprintf(tmp, "|%d bonus to %s.", 
              item->getMagicAttrib()->getBonus(),
              (item->getRpgItem()->isWeapon() ? "attack and damage" : "armor points"));
      strcat(description, tmp);
    } 
    if(item->getMagicAttrib()->getSchool() && 
       infoDetailLevel > (int)(100.0f * rand()/RAND_MAX)) {
      if(item->getRpgItem()->isWeapon()) {
        sprintf(tmp, "|extra damage of %s %s magic.", 
                item->getMagicAttrib()->describeMagicDamage(),
                item->getMagicAttrib()->getSchool()->getName());
      } else {
        sprintf(tmp, "|extra %d pts of %s magic resistance.", 
                item->getMagicAttrib()->getMagicResistance(),
                item->getMagicAttrib()->getSchool()->getName());
      }
      strcat(description, tmp);
    } 
    if(infoDetailLevel > (int)(100.0f * rand()/RAND_MAX)) {
      strcpy(tmp, "|Sets state mods:");
      bool found = false;
      for(int i = 0; i < Constants::STATE_MOD_COUNT; i++) {
        if(item->getMagicAttrib()->isStateModSet(i)) {
          strcat(tmp, " ");
          strcat(tmp, Constants::STATE_NAMES[i]);
          found = true;
        }
      }
      if(found) strcat(description, tmp);
    } 
    if(infoDetailLevel > (int)(100.0f * rand()/RAND_MAX)) {
      strcpy(tmp, "|Protects from state mods:");
      bool found = false;
      for(int i = 0; i < Constants::STATE_MOD_COUNT; i++) {
        if(item->getMagicAttrib()->isStateModProtected(i)) {
          strcat(tmp, " ");
          strcat(tmp, Constants::STATE_NAMES[i]);
          found = true;
        }
      }
      if(found) strcat(description, tmp);
    } 
    if(infoDetailLevel > (int)(100.0f * rand()/RAND_MAX)) {
      bool found = false;
      map<int,int> *skillBonusMap = item->getMagicAttrib()->getSkillBonusMap();
      for(map<int, int>::iterator i=skillBonusMap->begin(); i!=skillBonusMap->end(); ++i) {
        int skill = i->first;
        int bonus = i->second;
        sprintf(tmp, " %s+%d", Constants::SKILL_NAMES[skill], bonus);
        found = true;
      }
      if(found) {
        strcat(description, "|Bonuses to skills:");
        strcat(description, tmp);
      }
    } 
  } else if(item->getRpgItem()->getType() == RpgItem::SCROLL) {
    strcat(description, "|");
    strcat(description, item->getSpell()->getNotes());
  }

  label->setText(description);
}

