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

class Session;

#define PERSIST_VERSION 4

typedef struct _DiceInfo {
  Uint32 version;
  Uint32 count, sides, mod;
} DiceInfo;

typedef struct _ItemInfo {
  Uint32 version;
  Uint32 level;
  Uint8 rpgItem_name[255];
  Uint8 shape_name[255];
  Uint32 blocking, currentCharges, weight;
  Uint32 quality;
  Uint32 price;
  Uint32 action;
  Uint32 speed;
  Uint32 distance;
  Uint32 maxCharges;
  Uint32 duration;
  Uint8 spell_name[255];
  Uint32 containedItemCount;
  struct _ItemInfo *containedItems[MAX_CONTAINED_ITEMS];

  Uint32 bonus, damageMultiplier, cursed, magicLevel;
  Uint8 monster_type[255];
  Uint8 magic_school_name[255];
  DiceInfo *magicDamage;
  Uint8 stateMod[Constants::STATE_MOD_COUNT];
  Uint8 skillBonus[Constants::SKILL_COUNT];

} ItemInfo;
/*
typedef struct _ContainedItemInfo {
  Uint32 version;
  Uint32 containedItemCount;
  ItemInfo containedItems[MAX_CONTAINED_ITEMS];
} ContainedItemInfo;
*/

typedef struct _CreatureInfo {
  Uint32 version;
  Uint8 name[255];
  Uint8 character_name[255];
  Uint8 monster_name[255];
  Uint32 hp, mp, exp, level, money, stateMod, protStateMod, x, y, z, dir;
  Uint32 speed, motion, armor, bonusArmor, thirst, hunger;
  Uint32 availableSkillPoints;
  Uint32 skills[Constants::SKILL_COUNT];
  Uint32 skillMod[Constants::SKILL_COUNT];
  Uint32 skillBonus[Constants::SKILL_COUNT];

  // inventory
  Uint32 inventory_count;
  ItemInfo *inventory[MAX_INVENTORY_SIZE];
  //ContainedItemInfo containedItems[MAX_INVENTORY_SIZE];
  Uint32 equipped[Constants::INVENTORY_COUNT];

  // spells memorized ([school][spell]
  Uint32 spell_count;
  Uint8 spell_name[100][255];
} CreatureInfo;

class Persist {
public:
  static bool doesSaveGameExist(Session *session);
  static bool saveGame(Session *session);
  static bool loadGame(Session *session);

protected:
  static void saveCreature(FILE *fp, CreatureInfo *info);
  static CreatureInfo *loadCreature(FILE *fp);
  static void deleteCreatureInfo( CreatureInfo *info );

  static void saveItem(FILE *fp, ItemInfo *item);
  static ItemInfo *loadItem(FILE *fp);
  static void deleteItemInfo( ItemInfo *info );

  static void saveDice( FILE *fp, DiceInfo *info );
  static DiceInfo *loadDice(FILE *fp);
  static void deleteDiceInfo( DiceInfo *info );
};

#endif
