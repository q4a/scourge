/***************************************************************************
                          pc.h  -  description
                             -------------------
    begin                : Mon Jul 7 2003
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
#include "pc.h"

PlayerChar::PlayerChar(char *name, Character *character) {
  this->name = name;
  this->character = character;
  this->stateMod = 0;
  this->level = 1;
  this->exp = 0;
  this->hp = 0;
  this->ac = 0;
  this->inventory_count = 0;
}

PlayerChar::~PlayerChar() {
}

RpgItem *PlayerChar::removeInventory(int index) { 
  RpgItem *item = NULL;
  if(index < inventory_count) {
	item = inventory[index];
	for(int i = index; i < inventory_count - 1; i++) {
	  inventory[i] = inventory[i + 1];
	}
	inventory_count--;
  }
  return item;
}

PlayerChar **PlayerChar::createHardCodedParty() {
  //PlayerChar *pc = new PlayerChar[4];
  PlayerChar **pc = (PlayerChar**)malloc(sizeof(PlayerChar*) * 4);

  pc[0] = new PlayerChar("Alamont", Character::character_class[Character::knight]);
  pc[1] = new PlayerChar("Barlett", Character::character_class[Character::loremaster]);
  pc[2] = new PlayerChar("Corinus", Character::character_class[Character::summoner]);
  pc[3] = new PlayerChar("Dialante", Character::character_class[Character::naturalist]);

  pc[0]->setLevel(1); 
  pc[0]->setExp(300);
  pc[0]->setHp();
  pc[0]->setStateMod(Constants::blessed, true);
  pc[0]->setStateMod(Constants::poisoned, true);
  for(int i = 0; i < Constants::SKILL_COUNT; i++) {
      pc[0]->setSkill(i, (int)(100.0 * rand()/RAND_MAX));
  }
  pc[0]->addInventory(RpgItem::items[RpgItem::BASTARD_SWORD]);
  pc[0]->addInventory(RpgItem::items[RpgItem::DAGGER]);
  
  pc[1]->setLevel(1); 
  pc[1]->setExp(200);
  pc[1]->setHp();
  pc[1]->setStateMod(Constants::drunk, true);
  pc[1]->setStateMod(Constants::cursed, true);      
  for(int i = 0; i < Constants::SKILL_COUNT; i++) {
      pc[1]->setSkill(i, (int)(100.0 * rand()/RAND_MAX));
  }
  
  pc[2]->setLevel(1); 
  pc[2]->setExp(150);
  pc[2]->setHp();
  pc[2]->setStateMod(Constants::ac_protected, true);
  pc[2]->setStateMod(Constants::magic_protected, true);
  pc[2]->setStateMod(Constants::cursed, true);        
  for(int i = 0; i < Constants::SKILL_COUNT; i++) {
      pc[2]->setSkill(i, (int)(100.0 * rand()/RAND_MAX));
  }
  pc[2]->addInventory(RpgItem::items[RpgItem::LONG_SWORD]);
  
  pc[3]->setLevel(1); 
  pc[3]->setExp(400);
  pc[3]->setHp();
  pc[3]->setStateMod(Constants::possessed, true);          
  for(int i = 0; i < Constants::SKILL_COUNT; i++) {
      pc[3]->setSkill(i, (int)(100.0 * rand()/RAND_MAX));
  }
  pc[3]->addInventory(RpgItem::items[RpgItem::GREAT_SWORD]);
  pc[3]->addInventory(RpgItem::items[RpgItem::BATTLE_AXE]);
  pc[3]->addInventory(RpgItem::items[RpgItem::THROWING_AXE]);

  return pc;
}
