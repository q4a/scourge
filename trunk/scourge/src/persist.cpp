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
#include "render/renderlib.h"
#include "io/file.h"

using namespace std;

LocationInfo *Persist::createLocationInfo( Uint16 x, Uint16 y, Uint16 z ) {
  LocationInfo *info = (LocationInfo*)malloc( sizeof( LocationInfo ) );

  info->x = x;
  info->y = y;
  info->z = z;
  
  info->item = NULL;
  info->creature = NULL;
  info->shape_name[0] = 0;
  info->floor_shape_name[0] = 0;

  info->locked = 0;
  info->key_x = 0;
  info->key_y = 0;
  info->key_z = 0;

  return info;
}

void Persist::saveMap( File *file, MapInfo *info ) {
  file->write( &(info->version) );
  file->write( &(info->start_x) );
  file->write( &(info->start_y) );
  file->write( &(info->grid_x) );
  file->write( &(info->grid_y) );
  file->write( info->theme_name, 255 );
  file->write( &(info->pos_count) );
  for( int i = 0; i < (int)info->pos_count; i++ ) {
    file->write( &(info->pos[i]->x) );
    file->write( &(info->pos[i]->y) );
    file->write( &(info->pos[i]->z) );
    if( strlen( (char*)(info->pos[i]->floor_shape_name) ) ) {
      file->write( info->pos[i]->floor_shape_name, 255 );
    } else {
      file->write( info->pos[i]->floor_shape_name );
    }
    if( strlen( (char*)(info->pos[i]->shape_name) ) ) {
      file->write( info->pos[i]->shape_name, 255 );
    } else {
      file->write( info->pos[i]->shape_name );
    }
    
    Uint8 hasItem = ( info->pos[i]->item ? 1 : 0 );
    file->write( &(hasItem) );
    if( hasItem ) saveItem( file, info->pos[i]->item );

    Uint8 hasCreature = ( info->pos[i]->creature ? 1 : 0 );
    file->write( &(hasCreature) );
    if( hasCreature ) saveCreature( file, info->pos[i]->creature );

    file->write( &(info->pos[i]->locked) );
    file->write( &(info->pos[i]->key_x) );
    file->write( &(info->pos[i]->key_y) );
    file->write( &(info->pos[i]->key_z) );
  }
}

// FIXME: reuse this in loadmap
void Persist::loadMapHeader( File *file, Uint16 *gridX, Uint16 *gridY ) {
  Uint32 i32;
  Uint16 i16;
  file->read( &i32 );
  file->read( &i16 );
  file->read( &i16 );
  file->read( gridX );
  file->read( gridY );
}

MapInfo *Persist::loadMap( File *file ) {
  MapInfo *info = (MapInfo*)malloc(sizeof(MapInfo));
  file->read( &(info->version) );
  if( info->version < PERSIST_VERSION ) {
    cerr << "*** Warning: loading older map file: v" << info->version << 
      " vs. v" << PERSIST_VERSION << endl;
  }
  file->read( &(info->start_x) );
  file->read( &(info->start_y) );
  file->read( &(info->grid_x) );
  file->read( &(info->grid_y) );
  file->read( info->theme_name, 255 );
  file->read( &(info->pos_count) );
  for( int i = 0; i < (int)info->pos_count; i++ ) {
    info->pos[i] = (LocationInfo*)malloc(sizeof(LocationInfo));
    file->read( &(info->pos[i]->x) );
    file->read( &(info->pos[i]->y) );
    file->read( &(info->pos[i]->z) );
    
    file->read( info->pos[i]->floor_shape_name );
    if( info->pos[i]->floor_shape_name[0] ) {
      file->read( info->pos[i]->floor_shape_name + 1, 254 );
    }

    file->read( info->pos[i]->shape_name );
    if( info->pos[i]->shape_name[0] ) {
      file->read( info->pos[i]->shape_name + 1, 254 );
    }
    
    Uint8 hasItem;
    file->read( &(hasItem) );
    if( hasItem ) info->pos[i]->item = loadItem( file );
    else info->pos[i]->item = NULL;

    Uint8 hasCreature;
    file->read( &(hasCreature) );
    if( hasCreature ) info->pos[i]->creature = loadCreature( file );
    else info->pos[i]->creature = NULL;

    file->read( &(info->pos[i]->locked) );
    file->read( &(info->pos[i]->key_x) );
    file->read( &(info->pos[i]->key_y) );
    file->read( &(info->pos[i]->key_z) );
  }
  return info;
}

void Persist::deleteMapInfo( MapInfo *info ) {
  for( int i = 0; i < (int)info->pos_count; i++ ) {
    if( info->pos[i]->item ) free( info->pos[i]->item );
    if( info->pos[i]->creature ) free( info->pos[i]->creature );
    free( info->pos[i] );
  }
  free( info );
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

void Persist::saveCreature( File *file, CreatureInfo *info ) {
  file->write( &(info->version) );
  file->write( info->name, 255 );
  file->write( info->character_name, 255 );
  file->write( &info->character_model_info_index );  
  file->write( &info->deityIndex );  
  file->write( info->monster_name, 255 );
  file->write( &(info->hp) );
  file->write( &(info->mp) );
  file->write( &(info->exp) );
  file->write( &(info->level) );
  file->write( &(info->money) );
  file->write( &(info->stateMod) );
  file->write( &(info->protStateMod) );
  file->write( &(info->x) );
  file->write( &(info->y) );
  file->write( &(info->z) );
  file->write( &(info->dir) );
  file->write( &(info->speed) );
  file->write( &(info->motion) );
  file->write( &(info->armor) );
  file->write( &(info->bonusArmor) );
  file->write( &(info->thirst) );
  file->write( &(info->hunger) );
  file->write( &(info->availableSkillPoints) );
  file->write( info->skills, Constants::SKILL_COUNT );
  file->write( info->skillMod, Constants::SKILL_COUNT );
  file->write( info->skillBonus, Constants::SKILL_COUNT );
	file->write( info->skillsUsed, Constants::SKILL_COUNT );
  file->write( &info->portraitTextureIndex );
  file->write( &(info->inventory_count) );
  for(int i = 0; i < (int)info->inventory_count; i++) {
    saveItem( file, info->inventory[i] );
  }
  file->write( info->equipped, Constants::INVENTORY_COUNT );
  file->write( &(info->spell_count) );
  for(int i = 0; i < (int)info->spell_count; i++) {
    file->write( info->spell_name[i], 255 );
  }
  for(int i = 0; i < 12; i++ ) {
    file->write( info->quick_spell[i], 255 );
  }
}

CreatureInfo *Persist::loadCreature( File *file ) {
  CreatureInfo *info = (CreatureInfo*)malloc(sizeof(CreatureInfo));
  file->read( &(info->version) );
  file->read( info->name, 255 );
  file->read( info->character_name, 255 );
  file->read( &info->character_model_info_index );
  file->read( &info->deityIndex );
  file->read( info->monster_name, 255 );
  file->read( &(info->hp) );
  file->read( &(info->mp) );
  file->read( &(info->exp) );
  file->read( &(info->level) );
  file->read( &(info->money) );
  file->read( &(info->stateMod) );
  file->read( &(info->protStateMod) );
  file->read( &(info->x) );
  file->read( &(info->y) );
  file->read( &(info->z) );
  file->read( &(info->dir) );
  file->read( &(info->speed) );
  file->read( &(info->motion) );
  file->read( &(info->armor) );
  file->read( &(info->bonusArmor) );
  file->read( &(info->thirst) );
  file->read( &(info->hunger) );
  file->read( &(info->availableSkillPoints) );
	if( info->version == 8 ||
      info->version == 7 ) {
    // no POLE_WEAPON skill in v8.
    file->read( info->skills, Constants::SKILL_COUNT - 1 );
    file->read( info->skillMod, Constants::SKILL_COUNT - 1 );
    file->read( info->skillBonus, Constants::SKILL_COUNT - 1 );
    for( int i = Constants::SKILL_COUNT - 1; i > Constants::POLE_WEAPON; i-- ) {
      info->skills[ i ] = info->skills[ i - 1];
      info->skillMod[ i ] = info->skillMod[ i - 1];
      info->skillBonus[ i ] = info->skillBonus[ i - 1];			
    }
    info->skills[ Constants::POLE_WEAPON ] = MAX_SKILL / 2;
    info->skillMod[ Constants::POLE_WEAPON ] = 0;
    info->skillBonus[ Constants::POLE_WEAPON ] = 0;
		for( int i = 0; i < Constants::SKILL_COUNT; i++ ) {
			info->skillsUsed[ i ] = 0;		
		}
  } else {
    file->read( info->skills, Constants::SKILL_COUNT );
    file->read( info->skillMod, Constants::SKILL_COUNT );
    file->read( info->skillBonus, Constants::SKILL_COUNT );
		if( info->version < 12 ) {
			for( int i = 0; i < Constants::SKILL_COUNT; i++ ) {
				info->skillsUsed[ i ] = 0;		
			}
		} else {
			file->read( info->skillsUsed, Constants::SKILL_COUNT );
		}
  }
  file->read( &info->portraitTextureIndex );
  file->read( &(info->inventory_count) );
  for(int i = 0; i < (int)info->inventory_count; i++) {
    info->inventory[i] = loadItem( file );
  }
  if( info->version <= 9 ) {
    // no INVENTORY_GLOVE before v10
    file->read( info->equipped, 14 );
    info->equipped[ 14 ] = MAX_INVENTORY_SIZE;
  } else {
    file->read( info->equipped, Constants::INVENTORY_COUNT );
  }
  file->read( &(info->spell_count) );
  for(int i = 0; i < (int)info->spell_count; i++) {
    file->read( info->spell_name[i], 255 );
  }
  for(int i = 0; i < 12; i++ ) {
    file->read( info->quick_spell[i], 255 );
  }
  return info;
}

void Persist::saveItem( File *file, ItemInfo *info ) {
  file->write( &(info->version) );
  file->write( &(info->level) );
  file->write( info->rpgItem_name, 255 );
  file->write( info->shape_name, 255 );
  file->write( &(info->blocking) );
  file->write( &(info->currentCharges) );
  file->write( &(info->weight) );
  file->write( &(info->quality) );
  file->write( &(info->price) );
  file->write( &(info->speed) );
  file->write( &(info->distance) );
  file->write( &(info->maxCharges) );
  file->write( &(info->duration) );
  file->write( info->spell_name, 255 );
  file->write( &(info->containedItemCount) );
  for(int i = 0; i < (int)info->containedItemCount; i++) {
    saveItem( file, info->containedItems[i] );
  }

  file->write( &(info->bonus) );
  file->write( &(info->damageMultiplier) );
  file->write( &(info->cursed) );
  file->write( &(info->magicLevel) );
  file->write( info->monster_type, 255 );
  file->write( info->magic_school_name, 255 );
  saveDice( file, info->magicDamage );
  for(int i = 0; i < Constants::STATE_MOD_COUNT; i++) {
    file->write( &(info->stateMod[i]) );
  }
  for(int i = 0; i < Constants::SKILL_COUNT; i++) {
    file->write( &(info->skillBonus[i]) );
  }
}

ItemInfo *Persist::loadItem( File *file ) {
  ItemInfo *info = (ItemInfo*)malloc(sizeof(ItemInfo));
  file->read( &(info->version) );
  file->read( &(info->level) );
  file->read( info->rpgItem_name, 255 );
  file->read( info->shape_name, 255 );
  file->read( &(info->blocking) );
  file->read( &(info->currentCharges) );
  file->read( &(info->weight) );
  file->read( &(info->quality) );
  file->read( &(info->price) );
  if( info->version == 8 ||
      info->version == 7 ) {
    // read 'action' value for v8
    Uint32 tmp;
    file->read( &tmp );
  }
  file->read( &(info->speed) );
  file->read( &(info->distance) );
  file->read( &(info->maxCharges) );
  file->read( &(info->duration) );
  file->read( info->spell_name, 255 );
  file->read( &(info->containedItemCount) );
  for(int i = 0; i < (int)info->containedItemCount; i++) {
    info->containedItems[i] = loadItem( file );
  }

  file->read( &(info->bonus) );
  file->read( &(info->damageMultiplier) );
  file->read( &(info->cursed) );
  file->read( &(info->magicLevel) );
  file->read( info->monster_type, 255 );
  file->read( info->magic_school_name, 255 );
  info->magicDamage = loadDice( file );
  for(int i = 0; i < Constants::STATE_MOD_COUNT; i++) {
    file->read( &(info->stateMod[i]) );
  }
  if( info->version == 8 ||
      info->version == 7 ) {
    // no POLE_WEAPON in v8.
    for(int i = 0; i < Constants::SKILL_COUNT - 1; i++) {
      file->read( &(info->skillBonus[i]) );
    }
    for( int i = Constants::SKILL_COUNT - 1; 
         i > Constants::POLE_WEAPON + 1; 
         i-- ) {
      info->skillBonus[i] = info->skillBonus[i - 1];
    }
    info->skillBonus[ Constants::POLE_WEAPON ] = 0;
  } else {
    for(int i = 0; i < Constants::SKILL_COUNT; i++) {
      file->read( &(info->skillBonus[i]) );
    }
  }
  return info;
}

void Persist::saveDice( File *file, DiceInfo *info ) {
  file->write( &(info->version) );
  file->write( &(info->count) );
  file->write( &(info->sides) );
  file->write( &(info->mod) );
}

DiceInfo *Persist::loadDice( File *file ) {
  DiceInfo *info = (DiceInfo*)malloc(sizeof(DiceInfo));
  file->read( &(info->version) );
  file->read( &(info->count) );
  file->read( &(info->sides) );
  file->read( &(info->mod) );
  return info;
}

