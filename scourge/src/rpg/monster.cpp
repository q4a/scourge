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
Monster *Monster::monsters[MAX_MONSTER_LEVEL][MAX_MONSTER_COUNT] =
  {
	{
	  new Monster("An Imp", 0, 4, Constants::BUGGERLING_INDEX),
	  new Monster("An Oozing Green Waddler", 0, 3, Constants::SLIME_INDEX),
	  new Monster("A Buggerling", 0, 3, Constants::BUGGERLING_INDEX),
	  NULL
	},
	{
	  new Monster("A Rabbid Rodent", 1, 4, Constants::BUGGERLING_INDEX),
	  new Monster("A Gray Slimy Waddler", 1, 5, Constants::SLIME_INDEX),
	  new Monster("A Fleshworm", 1, 3, Constants::BUGGERLING_INDEX),
	  NULL
	},
	{
	  new Monster("A Kobold", 2, 6, Constants::BUGGERLING_INDEX),
	  new Monster("A Dire Stench-Waddler", 2, 6, Constants::SLIME_INDEX),
	  new Monster("A Minor Spectre", 2, 4, Constants::BUGGERLING_INDEX),
	  NULL
	}
  };
  
Monster::Monster(char *type, int level, int hp, Uint8 shapeIndex) {
  this->type = type;
  this->level = level;
  this->hp = hp;
  this->shapeIndex = shapeIndex;
  sprintf(description, "FIXME: need a description");
}

Monster::~Monster() {
}

void Monster::initMonsters() {
  for(int i = 0; i < MAX_MONSTER_LEVEL; i++) {
	for(int count = 0; count < MAX_MONSTER_COUNT; count++) {
	  if(!monsters[i][count]) {
		monsterCount[i] = count;
		break;
	  }
	}
  }
}

Monster *Monster::getRandomMonster(int level) {
  return monsters[level][(int) ((float)monsterCount[level] * rand()/RAND_MAX)];
}
