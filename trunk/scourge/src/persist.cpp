/***************************************************************************
                          persist.cpp  -  description
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
#include "persist.h"
#include "session.h"

#define SAVE_FILE "savegame.dat"

bool Persist::doesSaveGameExist(Session *session) {
  char path[300];
  get_file_name( path, 300, SAVE_FILE );
  FILE *fp = fopen( path, "rb" );
  bool ret = false;
  if(fp) {
    ret = true;
    fclose( fp );
  }
  return ret;
}

bool Persist::saveGame(Session *session) {
  session->getUserConfiguration()->createConfigDir();
  char path[300];
  get_file_name( path, 300, SAVE_FILE );
  FILE *fp = fopen( path, "wb" );
  if(!fp) {
    cerr << "Error creating savegame file! path=" << path << endl;
    return false;
  }
  Uint32 n = PERSIST_VERSION;
  fwrite( &n, sizeof(Uint32), 1, fp );
  n = session->getBoard()->getStorylineIndex();
  fwrite( &n, sizeof(Uint32), 1, fp );
  n = session->getParty()->getPartySize();
  fwrite( &n, sizeof(Uint32), 1, fp );
  for(int i = 0; i < session->getParty()->getPartySize(); i++) {
    CreatureInfo *info = session->getParty()->getParty(i)->save();
    Persist::saveCreature(fp, info);
    deleteCreatureInfo( info );
  }
  fclose( fp );

  return true;
}

bool Persist::loadGame(Session *session) {
  char path[300];
  get_file_name( path, 300, SAVE_FILE );
  FILE *fp = fopen( path, "rb" );
  if(!fp) {
    return false;
  }

  Uint32 n = PERSIST_VERSION;
  fread( &n, sizeof(Uint32), 1, fp );
  if( n != PERSIST_VERSION ) {
    cerr << "Savegame file is old: ignoring data in file." << endl;
  } else {
    Uint32 storylineIndex;
    fread( &storylineIndex, sizeof(Uint32), 1, fp );
    fread( &n, sizeof(Uint32), 1, fp );
    Creature *pc[MAX_PARTY_SIZE];
    for(int i = 0; i < (int)n; i++) {
      CreatureInfo *info = Persist::loadCreature( fp );
      pc[i] = session->getParty()->getParty(i)->load( session, info );
      deleteCreatureInfo( info );
    }

    // set the new party
    session->getParty()->setParty( n, pc, storylineIndex );
  }
  fclose( fp );
  return true;
}

void Persist::deleteCreatureInfo( CreatureInfo *info ) {
  for(int i = 0; i < (int)info->inventory_count; i++) {
    deleteItemInfo( info->inventory[i] );
  }
  free( info );
}

void Persist::deleteItemInfo( ItemInfo *info ) {
  for(int i = 0; i < (int)info->containedItemCount; i++) {
    deleteItemInfo( info->containedItems[i] );
  }
  free( info );
}

void Persist::deleteDiceInfo( DiceInfo *info ) {
  free( info );
}

void Persist::saveCreature(FILE *fp, CreatureInfo *info) {
  fwrite( &(info->version), sizeof(Uint32), 1, fp );
  fwrite( info->name, sizeof(Uint8), 255, fp );
  fwrite( info->character_name, sizeof(Uint8), 255, fp );
  fwrite( info->monster_name, sizeof(Uint8), 255, fp );
  fwrite( &(info->hp), sizeof(Uint32), 1, fp );
  fwrite( &(info->mp), sizeof(Uint32), 1, fp );
  fwrite( &(info->exp), sizeof(Uint32), 1, fp );
  fwrite( &(info->level), sizeof(Uint32), 1, fp );
  fwrite( &(info->money), sizeof(Uint32), 1, fp );
  fwrite( &(info->stateMod), sizeof(Uint32), 1, fp );
  fwrite( &(info->protStateMod), sizeof(Uint32), 1, fp );
  fwrite( &(info->x), sizeof(Uint32), 1, fp );
  fwrite( &(info->y), sizeof(Uint32), 1, fp );
  fwrite( &(info->z), sizeof(Uint32), 1, fp );
  fwrite( &(info->dir), sizeof(Uint32), 1, fp );
  fwrite( &(info->speed), sizeof(Uint32), 1, fp );
  fwrite( &(info->motion), sizeof(Uint32), 1, fp );
  fwrite( &(info->armor), sizeof(Uint32), 1, fp );
  fwrite( &(info->bonusArmor), sizeof(Uint32), 1, fp );
  fwrite( &(info->thirst), sizeof(Uint32), 1, fp );
  fwrite( &(info->hunger), sizeof(Uint32), 1, fp );
  fwrite( &(info->availableSkillPoints), sizeof(Uint32), 1, fp );
  fwrite( info->skills, sizeof(Uint32), Constants::SKILL_COUNT, fp );
  fwrite( info->skillMod, sizeof(Uint32), Constants::SKILL_COUNT, fp );
  fwrite( info->skillBonus, sizeof(Uint32), Constants::SKILL_COUNT, fp );
  fwrite( &info->portraitTextureIndex, sizeof(Uint32), 1, fp );
  fwrite( &(info->inventory_count), sizeof(Uint32), 1, fp );
  for(int i = 0; i < (int)info->inventory_count; i++) {
    saveItem( fp, info->inventory[i] );
  }
  fwrite( info->equipped, sizeof(Uint32), Constants::INVENTORY_COUNT, fp );
  fwrite( &(info->spell_count), sizeof(Uint32), 1, fp );
  for(int i = 0; i < (int)info->spell_count; i++) {
    fwrite( info->spell_name[i], sizeof(Uint8), 255, fp );
  }
}

CreatureInfo *Persist::loadCreature(FILE *fp) {
  CreatureInfo *info = (CreatureInfo*)malloc(sizeof(CreatureInfo));
  fread( &(info->version), sizeof(Uint32), 1, fp );
  fread( info->name, sizeof(Uint8), 255, fp );
  fread( info->character_name, sizeof(Uint8), 255, fp );
  fread( info->monster_name, sizeof(Uint8), 255, fp );
  fread( &(info->hp), sizeof(Uint32), 1, fp );
  fread( &(info->mp), sizeof(Uint32), 1, fp );
  fread( &(info->exp), sizeof(Uint32), 1, fp );
  fread( &(info->level), sizeof(Uint32), 1, fp );
  fread( &(info->money), sizeof(Uint32), 1, fp );
  fread( &(info->stateMod), sizeof(Uint32), 1, fp );
  fread( &(info->protStateMod), sizeof(Uint32), 1, fp );
  fread( &(info->x), sizeof(Uint32), 1, fp );
  fread( &(info->y), sizeof(Uint32), 1, fp );
  fread( &(info->z), sizeof(Uint32), 1, fp );
  fread( &(info->dir), sizeof(Uint32), 1, fp );
  fread( &(info->speed), sizeof(Uint32), 1, fp );
  fread( &(info->motion), sizeof(Uint32), 1, fp );
  fread( &(info->armor), sizeof(Uint32), 1, fp );
  fread( &(info->bonusArmor), sizeof(Uint32), 1, fp );
  fread( &(info->thirst), sizeof(Uint32), 1, fp );
  fread( &(info->hunger), sizeof(Uint32), 1, fp );
  fread( &(info->availableSkillPoints), sizeof(Uint32), 1, fp );
  fread( info->skills, sizeof(Uint32), Constants::SKILL_COUNT, fp );
  fread( info->skillMod, sizeof(Uint32), Constants::SKILL_COUNT, fp );
  fread( info->skillBonus, sizeof(Uint32), Constants::SKILL_COUNT, fp );
  fread( &info->portraitTextureIndex, sizeof(Uint32), 1, fp );
  fread( &(info->inventory_count), sizeof(Uint32), 1, fp );
  for(int i = 0; i < (int)info->inventory_count; i++) {
    info->inventory[i] = loadItem( fp );
  }
  fread( info->equipped, sizeof(Uint32), Constants::INVENTORY_COUNT, fp );
  fread( &(info->spell_count), sizeof(Uint32), 1, fp );
  for(int i = 0; i < (int)info->spell_count; i++) {
    fread( info->spell_name[i], sizeof(Uint8), 255, fp );
  }
  return info;
}

void Persist::saveItem( FILE *fp, ItemInfo *info ) {
  fwrite( &(info->version), sizeof(Uint32), 1, fp );
  fwrite( &(info->level), sizeof(Uint32), 1, fp );
  fwrite( info->rpgItem_name, sizeof(Uint8), 255, fp );
  fwrite( info->shape_name, sizeof(Uint8), 255, fp );
  fwrite( &(info->blocking), sizeof(Uint32), 1, fp );
  fwrite( &(info->currentCharges), sizeof(Uint32), 1, fp );
  fwrite( &(info->weight), sizeof(Uint32), 1, fp );
  fwrite( &(info->quality), sizeof(Uint32), 1, fp );
  fwrite( &(info->price), sizeof(Uint32), 1, fp );
  fwrite( &(info->action), sizeof(Uint32), 1, fp );
  fwrite( &(info->speed), sizeof(Uint32), 1, fp );
  fwrite( &(info->distance), sizeof(Uint32), 1, fp );
  fwrite( &(info->maxCharges), sizeof(Uint32), 1, fp );
  fwrite( &(info->duration), sizeof(Uint32), 1, fp );
  fwrite( info->spell_name, sizeof(Uint8), 255, fp );
  fwrite( &(info->containedItemCount), sizeof(Uint32), 1, fp );
  for(int i = 0; i < (int)info->containedItemCount; i++) {
    saveItem( fp, info->containedItems[i] );
  }

  fwrite( &(info->bonus), sizeof(Uint32), 1, fp );
  fwrite( &(info->damageMultiplier), sizeof(Uint32), 1, fp );
  fwrite( &(info->cursed), sizeof(Uint32), 1, fp );
  fwrite( &(info->magicLevel), sizeof(Uint32), 1, fp );
  fwrite( info->monster_type, sizeof(Uint8), 255, fp );
  fwrite( info->magic_school_name, sizeof(Uint8), 255, fp );
  saveDice( fp, info->magicDamage );
  for(int i = 0; i < Constants::STATE_MOD_COUNT; i++) {
    fwrite( &(info->stateMod[i]), sizeof(Uint8), 1, fp );
  }
  for(int i = 0; i < Constants::SKILL_COUNT; i++) {
    fwrite( &(info->skillBonus[i]), sizeof(Uint8), 1, fp );
  }
}

ItemInfo *Persist::loadItem( FILE *fp ) {
  ItemInfo *info = (ItemInfo*)malloc(sizeof(ItemInfo));
  fread( &(info->version), sizeof(Uint32), 1, fp );
  fread( &(info->level), sizeof(Uint32), 1, fp );
  fread( info->rpgItem_name, sizeof(Uint8), 255, fp );
  fread( info->shape_name, sizeof(Uint8), 255, fp );
  fread( &(info->blocking), sizeof(Uint32), 1, fp );
  fread( &(info->currentCharges), sizeof(Uint32), 1, fp );
  fread( &(info->weight), sizeof(Uint32), 1, fp );
  fread( &(info->quality), sizeof(Uint32), 1, fp );
  fread( &(info->price), sizeof(Uint32), 1, fp );
  fread( &(info->action), sizeof(Uint32), 1, fp );
  fread( &(info->speed), sizeof(Uint32), 1, fp );
  fread( &(info->distance), sizeof(Uint32), 1, fp );
  fread( &(info->maxCharges), sizeof(Uint32), 1, fp );
  fread( &(info->duration), sizeof(Uint32), 1, fp );
  fread( info->spell_name, sizeof(Uint8), 255, fp );
  fread( &(info->containedItemCount), sizeof(Uint32), 1, fp );
  for(int i = 0; i < (int)info->containedItemCount; i++) {
    info->containedItems[i] = loadItem( fp );
  }

  fread( &(info->bonus), sizeof(Uint32), 1, fp );
  fread( &(info->damageMultiplier), sizeof(Uint32), 1, fp );
  fread( &(info->cursed), sizeof(Uint32), 1, fp );
  fread( &(info->magicLevel), sizeof(Uint32), 1, fp );
  fread( info->monster_type, sizeof(Uint8), 255, fp );
  fread( info->magic_school_name, sizeof(Uint8), 255, fp );
  info->magicDamage = loadDice( fp );
  for(int i = 0; i < Constants::STATE_MOD_COUNT; i++) {
    fread( &(info->stateMod[i]), sizeof(Uint8), 1, fp );
  }
  for(int i = 0; i < Constants::SKILL_COUNT; i++) {
    fread( &(info->skillBonus[i]), sizeof(Uint8), 1, fp );
  }

  return info;
}

void Persist::saveDice( FILE *fp, DiceInfo *info ) {
  fwrite( &(info->version), sizeof(Uint32), 1, fp );
  fwrite( &(info->count), sizeof(Uint32), 1, fp );
  fwrite( &(info->sides), sizeof(Uint32), 1, fp );
  fwrite( &(info->mod), sizeof(Uint32), 1, fp );
}

DiceInfo *Persist::loadDice( FILE *fp ) {
  DiceInfo *info = (DiceInfo*)malloc(sizeof(DiceInfo));
  fread( &(info->version), sizeof(Uint32), 1, fp );
  fread( &(info->count), sizeof(Uint32), 1, fp );
  fread( &(info->sides), sizeof(Uint32), 1, fp );
  fread( &(info->mod), sizeof(Uint32), 1, fp );
  return info;
}

