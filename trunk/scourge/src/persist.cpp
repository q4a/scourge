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
  
	// preset strings to all 0 for better compressability.
	memset( info->item_pos_name, 0, sizeof( info->item_pos_name ) );
	memset( info->item_name, 0, sizeof( info->item_name ) );
	memset( info->monster_name, 0, sizeof( info->monster_name ) );
	memset( info->shape_name, 0, sizeof( info->shape_name ) );
	memset( info->floor_shape_name, 0, sizeof( info->floor_shape_name ) );
	memset( info->magic_school_name, 0, sizeof( info->magic_school_name ) );

	info->item_pos = NULL;
	info->item = NULL;
	info->creature = NULL;

  return info;
}

RugInfo *Persist::createRugInfo( Uint16 cx, Uint16 cy ) {
	RugInfo *info = (RugInfo*)malloc( sizeof( RugInfo ) );
	info->cx = cx;
	info->cy = cy;
	info->angle = 0;
	info->isHorizontal = 0;
	info->texture = 0;
	return info;
}

LockedInfo *Persist::createLockedInfo( Uint32 key, Uint8 value ) {
	LockedInfo *info = (LockedInfo*)malloc( sizeof( LockedInfo ) );
	info->key = key;
	info->value = value;
	return info;
}

DoorInfo *Persist::createDoorInfo( Uint32 key, Uint32 value ) {
	DoorInfo *info = (DoorInfo*)malloc( sizeof( DoorInfo ) );
	info->key = key;
	info->value = value;
	return info;
}

void Persist::saveMap( File *file, MapInfo *info ) {
  file->write( &(info->version) );
	file->write( &(info->map_type) );
  file->write( &(info->start_x) );
  file->write( &(info->start_y) );
  file->write( &(info->grid_x) );
  file->write( &(info->grid_y) );
  file->write( info->theme_name, 255 );
	file->write( &(info->hasWater) );
	file->write( &(info->reference_type) );
	file->write( &(info->edited) );
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

		Uint8 hasItemPos = ( strlen( (char*)( info->pos[i]->item_pos_name ) ) || info->pos[i]->item_pos ? 1 : 0 );
    file->write( &(hasItemPos) );
    if( hasItemPos ) {
			if( info->reference_type == REF_TYPE_NAME ) {
				file->write( info->pos[i]->item_pos_name, 255 );
			} else {
				saveItem( file, info->pos[i]->item_pos );
			}
		}

    if( strlen( (char*)(info->pos[i]->shape_name) ) ) {
      file->write( info->pos[i]->shape_name, 255 );
    } else {
      file->write( info->pos[i]->shape_name );
    }
    
    Uint8 hasItem = ( strlen( (char*)( info->pos[i]->item_name ) ) || info->pos[i]->item ? 1 : 0 );
    file->write( &(hasItem) );
    if( hasItem ) {
			if( info->reference_type == REF_TYPE_NAME ) {
				file->write( info->pos[i]->item_name, 255 );
			} else {
				saveItem( file, info->pos[i]->item );
			}
		}

    Uint8 hasCreature = ( strlen( (char*)( info->pos[i]->monster_name ) ) || info->pos[i]->creature ? 1 : 0 );
    file->write( &(hasCreature) );
    if( hasCreature ) {
			if( info->reference_type == REF_TYPE_NAME ) {
				file->write( info->pos[i]->monster_name, 255 );
			} else {
				saveCreature( file, info->pos[i]->creature );
			}
		}

		Uint8 hasDeity = ( strlen( (char*)( info->pos[i]->magic_school_name ) ) ? 1 : 0 );
    file->write( &(hasDeity) );
    if( hasDeity ) {
			file->write( info->pos[i]->magic_school_name, 255 );
		}

  }
	file->write( &(info->rug_count) );
  for( int i = 0; i < (int)info->rug_count; i++ ) {
		file->write( &(info->rugPos[i]->cx) );
		file->write( &(info->rugPos[i]->cy) );
		file->write( &(info->rugPos[i]->texture) );
		file->write( &(info->rugPos[i]->isHorizontal) );
		file->write( &(info->rugPos[i]->angle) );
	}
	file->write( &(info->locked_count) );
  for( int i = 0; i < (int)info->locked_count; i++ ) {
		file->write( &(info->locked[i]->key) );
		file->write( &(info->locked[i]->value) );
	}
	file->write( &(info->door_count) );
  for( int i = 0; i < (int)info->door_count; i++ ) {
		file->write( &(info->door[i]->key) );
		file->write( &(info->door[i]->value) );
	}
	file->write( &(info->secret_count) );
  for( int i = 0; i < (int)info->secret_count; i++ ) {
		file->write( &(info->secret[i]->key) );
		file->write( &(info->secret[i]->value) );
	}
	for( int x = 0; x < MAP_WIDTH; x++ ) {
		for( int y = 0; y < MAP_DEPTH; y++ ) {
			file->write( &(info->fog_info.fog[x][y]) );
			//for( int i = 0; i < 4; i++ ) {
				//file->write( &(info->fog_info.players[x + (y * MAP_WIDTH)][i] ) );
			//}
		}
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
	if( info->version >= 24 ) {
		file->read( &(info->map_type) );
	} else {
		info->map_type = 1; // default to room-type: MapRenderHelper::ROOM_HELPER
	}
  file->read( &(info->start_x) );
  file->read( &(info->start_y) );
  file->read( &(info->grid_x) );
  file->read( &(info->grid_y) );
  file->read( info->theme_name, 255 );
	if( info->version >= 21 ) {
		file->read( &(info->hasWater) );
	} else {
		info->hasWater = 0;
	}
	if( info->version >= 27 ) {
		file->read( &(info->reference_type) );
	} else {
		info->reference_type = REF_TYPE_NAME;
	}
	if( info->version >= 32 ) {
		file->read( &(info->edited) );
	} else {
		info->edited = true;
	}
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
    
		strcpy( (char*)( info->pos[i]->item_pos_name ), "" );
		info->pos[i]->item_pos = NULL;
		if( info->version >= 19 ) {
			Uint8 hasItemPos;
			file->read( &(hasItemPos) );
			if( hasItemPos ) {
				if( info->reference_type == REF_TYPE_NAME ) {
					file->read( info->pos[i]->item_pos_name, 255 );
				} else {
					info->pos[i]->item_pos = loadItem( file );
				}
			}
		}
			
    file->read( info->pos[i]->shape_name );
    if( info->pos[i]->shape_name[0] ) {
      file->read( info->pos[i]->shape_name + 1, 254 );
    }
    
		strcpy( (char*)( info->pos[i]->item_name ), "" );
		info->pos[i]->item = NULL;
    Uint8 hasItem;
    file->read( &(hasItem) );
    if( hasItem ) {
			if( info->reference_type == REF_TYPE_NAME ) {
				file->read( info->pos[i]->item_name, 255 );
			} else {
				info->pos[i]->item = loadItem( file );
			}
		}

		strcpy( (char*)( info->pos[i]->monster_name ), "" );
		info->pos[i]->creature = NULL;
    Uint8 hasCreature;
    file->read( &(hasCreature) );
    if( hasCreature ) {
			if( info->reference_type == REF_TYPE_NAME ) {
				file->read( info->pos[i]->monster_name, 255 );
			} else {
				info->pos[i]->creature = loadCreature( file );
			}
		}

		if( info->version >= 26 ) {
			Uint8 hasDeity;
			file->read( &(hasDeity) );
			if( hasDeity ) {
				file->read( info->pos[i]->magic_school_name, 255 );
			} else strcpy( (char*)( info->pos[i]->magic_school_name ), "" );
		} else strcpy( (char*)( info->pos[i]->magic_school_name ), "" );

		if( info->version < 22 ) {
			// old door info (now unused)
			Uint8 locked;
			Uint16 key_x, key_y, key_z;

			file->read( &(locked) );
			file->read( &(key_x) );
			file->read( &(key_y) );
			file->read( &(key_z) );
		}
  }
	if( info->version >= 20 ) {
		file->read( &(info->rug_count) );
		for( int i = 0; i < (int)info->rug_count; i++ ) {
			info->rugPos[i] = (RugInfo*)malloc(sizeof(RugInfo));
			file->read( &(info->rugPos[i]->cx) );
			file->read( &(info->rugPos[i]->cy) );
			file->read( &(info->rugPos[i]->texture) );
			file->read( &(info->rugPos[i]->isHorizontal) );
			file->read( &(info->rugPos[i]->angle) );
		}
	} else {
		info->rug_count = 0;
	}
	if( info->version >= 22 ) {
		file->read( &(info->locked_count) );
		for( int i = 0; i < (int)info->locked_count; i++ ) {
			info->locked[i] = (LockedInfo*)malloc(sizeof(LockedInfo));
			file->read( &(info->locked[i]->key) );
			file->read( &(info->locked[i]->value) );
		}
		file->read( &(info->door_count) );
		for( int i = 0; i < (int)info->door_count; i++ ) {
			info->door[i] = (DoorInfo*)malloc(sizeof(DoorInfo));
			file->read( &(info->door[i]->key) );
			file->read( &(info->door[i]->value) );
		}
	} else {
		info->locked_count = info->door_count = 0;
	}
	if( info->version >= 23 ) {
		file->read( &(info->secret_count) );
		for( int i = 0; i < (int)info->secret_count; i++ ) {
			info->secret[i] = (LockedInfo*)malloc(sizeof(LockedInfo));
			file->read( &(info->secret[i]->key) );
			file->read( &(info->secret[i]->value) );
		}
	} else {
		info->secret_count = 0;
	}
	if( info->version >= 25 ) {
		for( int x = 0; x < MAP_WIDTH; x++ ) {
			for( int y = 0; y < MAP_DEPTH; y++ ) {
				file->read( &(info->fog_info.fog[x][y]) );
				//for( int i = 0; i < 4; i++ ) {
					//file->read( &(info->fog_info.players[x + (y * MAP_WIDTH)][i] ) );
				//}
			}
		}
	} else {
		for( int x = 0; x < MAP_WIDTH; x++ ) {
			for( int y = 0; y < MAP_DEPTH; y++ ) {
				info->fog_info.fog[x][y] = 0; // FOG_UNVISITED
				for( int i = 0; i < 4; i++ ) {
					info->fog_info.players[x + (y * MAP_WIDTH)][i] = 5; // no player
				}
			}
		}
	}
  return info;
}

void Persist::deleteMapInfo( MapInfo *info ) {
  for( int i = 0; i < (int)info->pos_count; i++ ) {
		if( info->pos[i]->item_pos ) free( info->pos[i]->item_pos );
		if( info->pos[i]->item ) free( info->pos[i]->item );
		if( info->pos[i]->creature ) free( info->pos[i]->creature );
    free( info->pos[i] );
  }
	for( int i = 0; i < (int)info->rug_count; i++ ) {
    free( info->rugPos[i] );
  }
	for( int i = 0; i < (int)info->locked_count; i++ ) {
    free( info->locked[i] );
  }
	for( int i = 0; i < (int)info->door_count; i++ ) {
    free( info->door[i] );
  }
	for( int i = 0; i < (int)info->secret_count; i++ ) {
    free( info->secret[i] );
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

void Persist::deleteMissionInfo( MissionInfo *info ) {
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
  file->write( &(info->sex) );  
  file->write( &(info->armor) );
  file->write( &(info->bonusArmor) );
  file->write( &(info->thirst) );
  file->write( &(info->hunger) );
  file->write( &(info->availableSkillPoints) );
	file->write( info->skills, Skill::SKILL_COUNT );
	file->write( info->skillMod, Skill::SKILL_COUNT );
	file->write( info->skillBonus, Skill::SKILL_COUNT );
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
  if( info->version >= 16 ) {		
		file->read( &(info->sex ) );
  } else {
		info->sex = Constants::SEX_MALE;
	}
  file->read( &(info->armor) );
  file->read( &(info->bonusArmor) );
  file->read( &(info->thirst) );
  file->read( &(info->hunger) );
  file->read( &(info->availableSkillPoints) );
	file->read( info->skills, Skill::SKILL_COUNT );
	file->read( info->skillMod, Skill::SKILL_COUNT );
	file->read( info->skillBonus, Skill::SKILL_COUNT );
  file->read( &info->portraitTextureIndex );
  file->read( &(info->inventory_count) );
  for(int i = 0; i < (int)info->inventory_count; i++) {
    info->inventory[i] = loadItem( file );
  }
	file->read( info->equipped, Constants::INVENTORY_COUNT );
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
	file->write( &(info->identifiedBits) );
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
  for(int i = 0; i < StateMod::STATE_MOD_COUNT; i++) {
    file->write( &(info->stateMod[i]) );
  }
  for(int i = 0; i < Skill::SKILL_COUNT; i++) {
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
	if( info->version >= 17 ) file->read( &( info->identifiedBits ) );
	else info->identifiedBits = 0;
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
  for(int i = 0; i < StateMod::STATE_MOD_COUNT; i++) {
    file->read( &(info->stateMod[i]) );
  }
	for(int i = 0; i < Skill::SKILL_COUNT; i++) {
		file->read( &(info->skillBonus[i]) );
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

void Persist::saveMission( File *file, MissionInfo *info ) {
  file->write( &(info->version) );
	file->write( &(info->level) );
	file->write( &(info->depth) );
	file->write( &(info->completed) );
	file->write( info->mapName, 80 );
	file->write( info->templateName, 80 );
	file->write( &(info->itemCount) );
	for( int i = 0; i < (int)info->itemCount; i++ ) {
		file->write( info->itemName[ i ], 255 );
		file->write( &(info->itemDone[ i ]) );
	}
	file->write( &(info->monsterCount) );
	for( int i = 0; i < (int)info->monsterCount; i++ ) {
		file->write( info->monsterName[ i ], 255 );
		file->write( &(info->monsterDone[ i ]) );
	}
}

MissionInfo *Persist::loadMission( File *file ) {
  MissionInfo *info = (MissionInfo*)malloc(sizeof(MissionInfo));
	file->read( &(info->version) );
	file->read( &(info->level) );
	file->read( &(info->depth) );
	if( info->version >= 29 ) {
		file->read( &(info->completed) );
	} else {
		info->completed = 0;
	}
	file->read( info->mapName, 80 );
	file->read( info->templateName, 80 );
	file->read( &(info->itemCount) );
	for( int i = 0; i < (int)info->itemCount; i++ ) {
		file->read( info->itemName[ i ], 255 );
		file->read( &(info->itemDone[ i ]) );
	}
	file->read( &(info->monsterCount) );
	for( int i = 0; i < (int)info->monsterCount; i++ ) {
		file->read( info->monsterName[ i ], 255 );
		file->read( &(info->monsterDone[ i ]) );
	}
  return info;
}

