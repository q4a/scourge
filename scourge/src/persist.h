/***************************************************************************
                          persist.h  -  description
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

#ifndef PERSIST_H
#define PERSIST_H

#include "constants.h"

#define PERSIST_VERSION 1

typedef struct _ItemInfo {
} ItemInfo;

typedef struct _CreatureInfo {
  Uint32 version;
  Uint8 name[255];
  Uint32 character_index;
  Uint32 monster_index;
  Uint32 hp, mp, exp, level, money, statemod, x, y, z, dir;
  Uint32 speed, motion, armor, bonusArmor, thirst, hunger;
  Uint32 availableSkillPoints;
  Uint32 skills[Constants::SKILL_COUNT];
  Uint32 skillMod[Constants::SKILL_COUNT];
  Uint32 skillBonus[Constants::SKILL_COUNT];

  // inventory
  Uint32 inventory_count;
  ItemInfo inventory[MAX_INVENTORY_SIZE];
  Uint32 equipped[Constants::INVENTORY_COUNT];

  // spells memorized ([school][spell]
  Uint8 spell_index[10][10];
} CreatureInfo;

#endif
