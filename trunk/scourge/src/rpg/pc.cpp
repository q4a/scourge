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
