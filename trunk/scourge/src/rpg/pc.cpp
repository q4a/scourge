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

char PlayerChar::inventory_location[][80] = {
  "head",
  "neck",
  "back",
  "chest",
  "left hand",
  "right hand",
  "belt",
  "legs",
  "feet",
  "ring1",
  "ring2",
  "ring3",
  "ring4",
  "ranged weapon"
};

PlayerChar::PlayerChar(char *name, Character *character) {
  this->name = name;
  this->character = character;
  this->stateMod = 0;
  this->level = 1;
  this->exp = 0;
  this->hp = 0;
  this->ac = 0;
  this->inventory_count = 0;
  for(int i = 0; i < INVENTORY_COUNT; i++) {
	equipped[i] = MAX_INVENTORY_SIZE;
  }
}

PlayerChar::~PlayerChar() {
}

RpgItem *PlayerChar::removeInventory(int index) { 
  RpgItem *item = NULL;
  if(index < inventory_count) {
	// drop item if carrying it
	doff(index);
	// drop from inventory
	item = inventory[index];
	for(int i = index; i < inventory_count - 1; i++) {
	  inventory[i] = inventory[i + 1];
	}
	inventory_count--;
	// adjust equipped indexes too
	for(int i = 0; i < INVENTORY_COUNT; i++) {
	  if(equipped[i] > index && equipped[i] < MAX_INVENTORY_SIZE) {
		equipped[i]--;
	  }
	}
  }
  return item;
}

void PlayerChar::equipInventory(int index) {
  // doff
  if(doff(index)) return;
  // don
  // FIXME: take into account: two-handed weapons, race/class modifiers, min skill req-s., etc.
  RpgItem *item = getInventory(index);
  for(int i = 0; i < INVENTORY_COUNT; i++) {
	// if the slot is empty and the item can be worn here
	if(item->getEquip() & ( 1 << i ) && 
	   equipped[i] == MAX_INVENTORY_SIZE) {
		equipped[i] = index;
		return;
	}
  }
}

int PlayerChar::doff(int index) {
  // doff
  for(int i = 0; i < INVENTORY_COUNT; i++) {
	if(equipped[i] == index) {
	  equipped[i] = MAX_INVENTORY_SIZE;
	  return 1;
	}
  }
  return 0;
}

/**
   Get item at equipped index. (What is at equipped location?)
 */
RpgItem *PlayerChar::getEquippedInventory(int index) {
  int n = equipped[index];
  if(n < MAX_INVENTORY_SIZE) {
	return getInventory(n);
  }
  return NULL;
}

/**
   Get equipped index of inventory index. (Where is the item worn?)
*/
int PlayerChar::getEquippedIndex(int index) {
  for(int i = 0; i < INVENTORY_COUNT; i++) {
	if(equipped[i] == index) return i;
  }
  return -1;
}

/**
   Create a party programmatically until the party editor is made.
 */
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
