/***************************************************************************
                          monster.cpp  -  description
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
#include "monster.h"

/**
  *@author Gabor Torok
  */

int Monster::monsterCount[MAX_MONSTER_LEVEL];
Monster *Monster::monsters[MAX_MONSTER_LEVEL][MAX_MONSTER_COUNT];
  
Monster::Monster(char *type, int level, int hp, Uint8 shapeIndex) {
  this->type = type;
  this->level = level;
  this->hp = hp;
  this->shapeIndex = shapeIndex;
  for(int i = 0; i < 10; i++) {
	weapon[i] = armor[i] = item[i] = NULL;
  }
  speed = 50;
  sprintf(description, "FIXME: need a description");
}

Monster::~Monster() {
}

void Monster::initMonsters() {
  // ###########################################
  Monster *m;
  int level = 0;
  monsterCount[level] = 0;
  m = monsters[level][monsterCount[0]++] = 
	new Monster("An Imp", 0, 4, Constants::BUGGERLING_INDEX);
  m->weapon[0] = RpgItem::items[RpgItem::SHORT_SWORD];
  m->weapon[1] = RpgItem::items[RpgItem::DAGGER];

  m = monsters[level][monsterCount[0]++] = 
	new Monster("An Oozing Green Waddler", 0, 3, Constants::SLIME_INDEX);
  m->weapon[0] = RpgItem::items[RpgItem::DAGGER];

  m = monsters[level][monsterCount[0]++] = 
	new Monster("A Buggerling", 0, 3, Constants::BUGGERLING_INDEX);
  m->weapon[0] = RpgItem::items[RpgItem::SHORT_SWORD];


  // ###########################################
  level = 1;
  monsterCount[level] = 0;
  m = monsters[level][monsterCount[1]++] =
	new Monster("A Rabbid Rodent", 1, 4, Constants::BUGGERLING_INDEX);
  m->weapon[0] = RpgItem::items[RpgItem::DAGGER];

  m = monsters[level][monsterCount[1]++] =
	new Monster("A Gray Slimy Waddler", 1, 5, Constants::SLIME_INDEX);
  m->weapon[0] = RpgItem::items[RpgItem::DAGGER];

  m = monsters[level][monsterCount[1]++] =
	new Monster("A Fleshworm", 1, 3, Constants::BUGGERLING_INDEX);
  m->weapon[0] = RpgItem::items[RpgItem::DAGGER];


  // ###########################################
  level = 2;
  monsterCount[level] = 0;
  m = monsters[level][monsterCount[1]++] =
	new Monster("A Kobold", 2, 6, Constants::BUGGERLING_INDEX);
  m->weapon[0] = RpgItem::items[RpgItem::DAGGER];

  m = monsters[level][monsterCount[1]++] =
	new Monster("A Dire Stench-Waddler", 2, 6, Constants::SLIME_INDEX);
  m->weapon[0] = RpgItem::items[RpgItem::DAGGER];

  m = monsters[level][monsterCount[1]++] =
	new Monster("A Minor Spectre", 2, 4, Constants::BUGGERLING_INDEX);
  m->weapon[0] = RpgItem::items[RpgItem::DAGGER];
}

Monster *Monster::getRandomMonster(int level) {
  return monsters[level][(int) ((float)monsterCount[level] * rand()/RAND_MAX)];
}
