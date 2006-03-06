/***************************************************************************
                          terraingenerator.cpp  -  description
                             -------------------
    begin                : Thu Jan 15 2006
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
#include "terraingenerator.h"
#include "render/renderlib.h"
#include "rpg/rpglib.h"
#include "scourge.h"
#include "shapepalette.h"
#include "board.h"
#include "gui/progress.h"
#include "item.h"
#include "creature.h"

using namespace std;

TerrainGenerator::TerrainGenerator( Scourge *scourge, 
                                    int level, 
                                    int depth, 
                                    bool stairsDown, 
                                    bool stairsUp, 
                                    Mission *mission, 
                                    int progressSteps ) {
  this->scourge = scourge;
  this->level = level;
  this->depth = depth;
  this->stairsUp = stairsUp;
  this->stairsDown = stairsDown;
  this->mission = mission;

  progress = new Progress(scourge->getSDLHandler(), 
                          scourge->getSession()->getShapePalette()->getProgressTexture(),
                          scourge->getSession()->getShapePalette()->getProgressHighlightTexture(),
                          progressSteps, false, true );

  // reasonable defaults
  doorCount = 0;
  roomCount = 1;
  room[0].x = room[0].y = 0;
  room[0].w = ( MAP_WIDTH - ( 2 * MAP_OFFSET ) ) / MAP_UNIT;
  room[0].h = ( MAP_DEPTH - ( 2 * MAP_OFFSET ) ) / MAP_UNIT;
  room[0].valueBonus = 0;
  roomMaxWidth = 0;
  roomMaxHeight = 0;
  objectCount = 50;
  monsters = true;
}

TerrainGenerator::~TerrainGenerator() {
  delete progress;
}

void TerrainGenerator::updateStatus(const char *statusMessage) {
  progress->updateStatus(statusMessage);
  Uint32 now = SDL_GetTicks();
//  cerr << "+++ " << statusMessage << ". Previous task's time=" << (now - start) << endl;
  start = now;
}

void TerrainGenerator::toMap( Map *map, ShapePalette *shapePal ) {	 

  // set the renderer helper for this type of map
  map->setMapRenderHelper( getMapRenderHelper() );

  start = SDL_GetTicks();
  generate( map, shapePal );

  updateStatus(MESSAGE);
    
  // loop until successfully drawn nodes onto map
  int status = progress->getStatus();
  while( !drawNodesOnMap(map, shapePal) ) {
    // reset the progress
    progress->setStatus( status );
  }
}

bool TerrainGenerator::drawNodesOnMap(Map *map, ShapePalette *shapePal) {
  bool ret = drawNodes( map, shapePal );
  if( !ret ) goto cleanup;

  updateStatus("Compressing free space");
  createFreeSpaceMap(map, shapePal);
  
  updateStatus("Adding containers");
  addContainers(map, shapePal);  
  
  updateStatus("Adding gates");
  if( !addStairs(map, shapePal) ) {
    ret = false;
    goto cleanup;
  }

  updateStatus("Adding party");
  addParty(map, shapePal);
  
  // add a teleporters
  updateStatus("Adding teleporters");
  if( !addTeleporters(map, shapePal) ) {
    ret = false;
    goto cleanup;
  }
  
  updateStatus("Locking doors and chests");
  lockDoors(map, shapePal);
  
  updateStatus("Calculating room values");
  calculateRoomValues(map, shapePal);
  
  updateStatus("Adding mission objectives");
  addMissionObjectives(map, shapePal);
  
  updateStatus("Adding monsters");
  addMonsters(map, shapePal);
  
  updateStatus("Adding items");
  addItems(map, shapePal);
  
  updateStatus("Adding furniture");
  addFurniture(map, shapePal);
  
cleanup:
  updateStatus("Cleaning up");
  deleteFreeSpaceMap(map, shapePal);
  
  return ret;

}

void TerrainGenerator::addContainers(Map *map, ShapePalette *shapePal) {
  int x = 0;
  int y = 0;
  
  for( int i = 0; i < objectCount; i++ ) {
    RpgItem *rpgItem;
    if( 0 == (int)( 1.0f * rand() / RAND_MAX ) ) {
      rpgItem = RpgItem::getRandomContainer();
    } else {
      rpgItem = RpgItem::getRandomContainerNS();
    }
    if( rpgItem ) {
      Shape *shape = scourge->getShapePalette()->getShape(rpgItem->getShapeIndex());
      bool fits = getLocationInRoom(map, 0, shape, &x, &y);
      if( fits ) addItem( map, NULL, scourge->getSession()->newItem(rpgItem), NULL, x, y );
    }
  }
}

bool TerrainGenerator::addStairs(Map *map, ShapePalette *shapePal) {
  // add stairs for multi-level missions
  if(stairsUp) {
    bool done = false;
    for(int i = 0; i < 10; i++) {
      Shape *shape = scourge->getShapePalette()->findShapeByName("GATE_UP");
      int x, y;
      bool fits = getLocationInRoom(map, i, shape, &x, &y);
      if(fits && !coversDoor(map, scourge->getShapePalette(), shape, x, y)) {
        addItem(map, NULL, NULL, shape, x, y);
        done = true;
        break;
      }
    }
    if(!done) {
      cerr << "Error: couldn't add up stairs." << endl;
      return false;
    }
  }
  if(stairsDown) {
    bool done = false;
    for(int i = 0; i < 10; i++) {
      Shape *shape = scourge->getShapePalette()->findShapeByName("GATE_DOWN");
      int x, y;
      bool fits = getLocationInRoom(map, i, shape, &x, &y);
      if(fits && !coversDoor(map, scourge->getShapePalette(), shape, x, y)) {
        addItem(map, NULL, NULL, shape, x, y);
        done = true;
        break;
      }
    }
    if(!done) {
      cerr << "Error: couldn't add down stairs." << endl;
      return false;
    }
  }

  return true;
}

void TerrainGenerator::addItems(Map *map, ShapePalette *shapePal) {
  // add the items
  for(int i = 0; i < objectCount; i++) {
    Item *item = createRandomItem();
    if( item ) {
      int x, y;
      getRandomLocation(map, item->getShape(), &x, &y);
      addItem(map, NULL, item, NULL, x, y);
    }
  }

  // add some scrolls with spells
  for(int i = 0; i < objectCount / 4; i++) {
    Spell *spell = MagicSchool::getRandomSpell(level);
    if(!spell) {
      cerr << "Warning: no spells defined for level: " << level << endl;
      break;
    }
    Item *item = scourge->getSession()->
      newItem(RpgItem::getItemByName("Scroll"), level, spell);
    int x, y;
    getRandomLocation(map, item->getShape(), &x, &y);
    addItem(map, NULL, item, NULL, x, y);
  }

  // populate containers
  for(int i = 0; i < (int)containers.size(); i++) {
    Item *item = containers[i];
    int cx = containerX[i];
    int cy = containerY[i];
    int roomIndex = getRoomIndex(cx, cy);
    int valueBonus = 0;
    if(roomIndex > -1) valueBonus = room[roomIndex].valueBonus;

    // some items
    int n = (int)(3.0f * rand() / RAND_MAX);
    for(int i = 0; i < n; i++) {
      Item *containedItem = createRandomItem();
      if( containedItem ) {
        item->addContainedItem(containedItem, true);
      }
    }
    // some spells
    if(!((int)(25.0f * rand() / RAND_MAX))) {
      int n = (int)(2.0f * rand() / RAND_MAX) + 1;
      for(int i = 0; i < n; i++) {
        Spell *spell = MagicSchool::getRandomSpell(level);
        if(spell) {
          Item *scroll = scourge->getSession()->
            newItem(RpgItem::getItemByName("Scroll"), level, spell);
          item->addContainedItem(scroll, true);
        }
      }
    }
  }
}

Item *TerrainGenerator::createRandomItem() {
  // special items
  for( int i = 0; i < RpgItem::getSpecialCount(); i++ ) {
    RpgItem *rpgItem = RpgItem::getSpecial( i );
    if( rpgItem->getMinLevel() <= level &&
        rpgItem->getMinDepth() <= depth &&
        !( scourge->getSession()->getSpecialItem( rpgItem ) ) &&
        0 == (int)( (float)rpgItem->getRareness() * rand() / RAND_MAX ) ) {
      // set loading to true so the level is exact and the item is not enchanted
      Item *item = scourge->getSession()->newItem( rpgItem, level, NULL );
      return item;
    }
  }

  // general items
  RpgItem *rpgItem = RpgItem::getRandomItem( depth );
  if(!rpgItem) {
    cerr << "Warning: no items found." << endl;
    return NULL;
  }
  // Make a random level object
  return scourge->getSession()->newItem(rpgItem, level);
}

void TerrainGenerator::addMissionObjectives(Map *map, ShapePalette *shapePal) {
  if(mission && !mission->isCompleted() && !stairsDown) {

    // mission objects are on a pedestal
    // and they are blocking so creatures can't get them
    for(int i = 0; i < mission->getItemCount(); i++) {
      RpgItem *rpgItem = mission->getItem( i );
      Item *item = scourge->getSession()->newItem( rpgItem, mission->getLevel() );
      mission->addItemInstance( item, rpgItem );
      item->setBlocking(true); // don't let monsters pick this up
      Item *pedestal = scourge->getSession()->newItem(RpgItem::getItemByName("Pedestal"));
      int x, y;
      getRandomLocation(map, pedestal->getShape(), &x, &y);
      addItem(map, NULL, pedestal, NULL, x, y);
      addItem(map, NULL, item, NULL, 
              x + (pedestal->getShape()->getWidth()/2) - (item->getShape()->getWidth()/2), 
              y - (pedestal->getShape()->getDepth()/2) + (item->getShape()->getDepth()/2), 
              pedestal->getShape()->getHeight());
//      cerr << "*** Added mission item: " << item->getItemName() << " at: " << x << "," << y << endl;
    }

    // add mission creatures
    for(int i = 0; i < mission->getCreatureCount(); i++) {
      int x, y;
      Monster *monster = mission->getCreature( i );
      GLShape *shape = scourge->getSession()->getShapePalette()->
        getCreatureShape(monster->getModelName(), 
                         monster->getSkinName(), 
                         monster->getScale(),
						 monster);
      Creature *creature = scourge->getSession()->newCreature( monster, shape );
      mission->addCreatureInstanceMap( creature, monster );
      getRandomLocation(map, creature->getShape(), &x, &y);    
      addItem(map, creature, NULL, NULL, x, y);
      creature->moveTo(x, y, 0);
//      cerr << "*** Added mission monster: " << creature->getMonster()->getType() << endl;
    }
  }
}

void TerrainGenerator::addMonsters(Map *levelMap, ShapePalette *shapePal) {
  // add monsters in every room
  if(monsters) {
    int totalLevel = scourge->getParty()->getTotalLevel();
    //fprintf(stderr, "creating monsters for total player level: %d\n", totalLevel);
    for(int i = 0; i < roomCount; i++) {
      int areaCovered = 0;
      // don't crowd the rooms
      int roomAreaUsed = (int)(room[i].w * room[i].h * unitSide * 0.33f);
      int monsterLevelTotal = 0;
      bool badAssMonsters = ((int)((float)(10 - depth) * rand()/RAND_MAX) == 0);
      while(areaCovered < roomAreaUsed && (badAssMonsters || monsterLevelTotal < totalLevel)) {
        int monsterLevel = level;
        if(badAssMonsters && 0 == (int)(5.0f * rand()/RAND_MAX))
          monsterLevel++;
        Monster *monster = Monster::getRandomMonster(monsterLevel);
        //fprintf(stderr, "Trying to add %s to room %d\n", monster->getType(), i);
        if(!monster) {
          cerr << "Warning: no monsters defined for level: " << level << endl;
          break;
        }
        GLShape *shape = 
          scourge->getShapePalette()->getCreatureShape(monster->getModelName(), 
                                                       monster->getSkinName(), 
                                                       monster->getScale(),
													   monster);
        int x, y;
        bool fits = getLocationInRoom(levelMap, i, shape, &x, &y);

        if(fits) {
          //fprintf(stderr, "\tmonster fits at %d,%d.\n", x, y);
          Creature *creature = scourge->getSession()->newCreature(monster, shape);
          addItem(levelMap, creature, NULL, NULL, x, y);
          creature->moveTo(x, y, 0);
          areaCovered += (creature->getShape()->getWidth() * 
                          creature->getShape()->getDepth());
          monsterLevelTotal += creature->getMonster()->getLevel();
        } else {
          //fprintf(stderr, "\tmonster DOESN'T fit.\n");
          delete shape;
          break;
        }
      }

      if(monsterLevelTotal > totalLevel) {
        room[i].valueBonus++;
//        cerr << "Room " << i << " is guarded by badass monsters(tm). " << 
          //"Room's valueBonus=" << room[i].valueBonus << endl;
      }
    }

    // add a few misc. monsters in the corridors (use objectCount to approx. number of wandering monsters)
    for(int i = 0; i < objectCount * 2; i++) {
      Monster *monster = Monster::getRandomMonster(level);
      if(!monster) {
        cerr << "Warning: no monsters defined for level: " << level << endl;
        break;
      }
      GLShape *shape = 
        scourge->getShapePalette()->getCreatureShape(monster->getModelName(), 
                                                     monster->getSkinName(), 
                                                     monster->getScale(),
													 monster);
      Creature *creature = scourge->getSession()->newCreature(monster, shape);
      int x, y;                           
      getRandomLocation(levelMap, creature->getShape(), &x, &y);
      addItem(levelMap, creature, NULL, NULL, x, y);
      creature->moveTo(x, y, 0);
    }
  } else {

    // add positioned npcs
    for( map<string, Monster*>::iterator i=Monster::npcPos.begin(); 
         i != Monster::npcPos.end(); ++i ) {
      string key = i->first;
      Monster *npc = i->second;
      char tmp[80];
      strcpy( tmp, key.c_str() );
      int startX = atoi( strtok( tmp, "," ) );
      int startY = atoi( strtok( NULL, "," ) );
      
      //cerr << "Adding " << npc->getType() << " at " << startX << "," << startY << endl;
      GLShape *shape = 
        scourge->getShapePalette()->getCreatureShape( npc->getModelName(), 
                                                      npc->getSkinName(), 
                                                      npc->getScale(),
                                                      npc );
      Creature *creature = scourge->getSession()->newCreature( npc, shape );
      addItem( levelMap, creature, NULL, NULL, startX, startY );
      creature->moveTo( startX, startY, 0 );
    }

    // add npc-s
    for(int i = 0; i < roomCount; i++) {
      int areaCovered = 0;
      // don't crowd the rooms
      int roomAreaUsed = (int)(room[i].w * room[i].h * unitSide * 0.33f);
      while(areaCovered < roomAreaUsed) {
        Monster *monster = (Monster*)Monster::getRandomNpc();
        //fprintf(stderr, "Trying to add %s to room %d\n", monster->getType(), i);
        if(!monster) {
          cerr << "Warning: no npc found!" << endl;
          break;
        }
        GLShape *shape = 
          scourge->getShapePalette()->getCreatureShape(monster->getModelName(), 
                                                       monster->getSkinName(), 
                                                       monster->getScale(),
                                                       monster);
        int x, y;
        bool fits = getLocationInRoom(levelMap, i, shape, &x, &y);
        
        if(fits) {
          //fprintf(stderr, "\tmonster fits at %d,%d.\n", x, y);
          Creature *creature = scourge->getSession()->newCreature(monster, shape);
          addItem(levelMap, creature, NULL, NULL, x, y);
          creature->moveTo(x, y, 0);
          areaCovered += (creature->getShape()->getWidth() * 
                          creature->getShape()->getDepth());
        } else {
          //fprintf(stderr, "\tmonster DOESN'T fit.\n");
          delete shape;
          break;
        }
      }
    }
  }
}

void TerrainGenerator::addFurniture(Map *map, ShapePalette *shapePal) {
  // add tables, chairs, etc.
  addItemsInEveryRoom(RpgItem::getItemByName("Table"), 1);
  addItemsInEveryRoom(RpgItem::getItemByName("Chair"), 2);  

  // add some magic pools
  DisplayInfo di;
  for( int i = 0; i < roomCount; i++ ) {
    if( 0 == (int)( 0.0f * rand() / RAND_MAX ) ) {
      MagicSchool *ms = MagicSchool::getRandomSchool();
      di.red = ms->getDeityRed();
      di.green = ms->getDeityGreen();
      di.blue = ms->getDeityBlue();
      Location *pos = addShapeInRoom( scourge->getShapePalette()->findShapeByName("POOL"), i, &di );
      if( pos ) {
        // store pos->deity in scourge
        scourge->addDeityLocation( pos, ms );
      }
    }
  }
}

bool TerrainGenerator::addTeleporters(Map *map, ShapePalette *shapePal) {
  int teleportersAdded = 0;
  for(int teleporterCount = 0; teleporterCount < 3; teleporterCount++) {
    int x, y;
    getRandomLocation(map, scourge->getShapePalette()->findShapeByName("TELEPORTER"), &x, &y);
    if( x < MAP_WIDTH ) {
//      cerr << "teleporter at " << x << "," << y << endl;
      addItem(scourge->getMap(), NULL, NULL, 
              scourge->getShapePalette()->findShapeByName("TELEPORTER"), 
              x, y, 1);
      addItem(scourge->getMap(), NULL, NULL, 
              scourge->getShapePalette()->findShapeByName("TELEPORTER_BASE"), 
              x, y);
      teleportersAdded++;
    } else {
      cerr << "ERROR: couldn't add teleporter!!! #" << teleporterCount << endl;
    }
  }
  return (teleportersAdded > 0);
}

/**
 * Add the party somewhere near the middle of the first room.
 * See warning notes on this approach in findPlace() and loadMap() 
 * descriptions.
 */
void TerrainGenerator::addParty(Map *map, ShapePalette *shapePal) {
  int xx = MAP_OFFSET + ( room[0].x + room[0].w / 2 ) * MAP_UNIT;
  int yy = MAP_OFFSET + ( room[0].y + room[0].h / 2 ) * MAP_UNIT;
  int nx, ny;
  for( int r = 0; r < scourge->getParty()->getPartySize(); r++ ) {
    if( !scourge->getParty()->getParty(r)->getStateMod( Constants::dead ) ) {
      scourge->getParty()->getParty(r)->findPlace( xx, yy, &nx, &ny );
      xx = nx;
      yy = ny;
    }
  }

  /*
  int n = scourge->getParty()->getFirstLivePlayer();
  if( n > -1 ) {
    int x, y;
    bool fits;
    fits = 
      getLocationInRoom(map, 
                        0,
                        scourge->getParty()->getParty(n)->getShape(), 
                        &x, &y,
                        true);
    if(fits) {
      addItem(map, scourge->getParty()->getParty(n), NULL, NULL, x, y);
      scourge->getParty()->getParty(n)->moveTo(x, y, 0);
      scourge->getParty()->getParty(n)->setSelXY(-1,-1);

      // add the others nearby
      int xx = x;
      int yy = y;
      int nx, ny;
      for( int r = n + 1; r < scourge->getParty()->getPartySize(); r++ ) {
        if( !scourge->getParty()->getParty(r)->getStateMod( Constants::dead ) ) {
          scourge->getParty()->getParty(r)->findPlace( xx, yy, &nx, &ny );
          xx = nx;
          yy = ny;
        }
      }
    } else {
      cerr << "*** Error: Can't add party!!!" << endl;
    }
  }
  */
}

void TerrainGenerator::lockDoors(Map *map, ShapePalette *shapePal) {
  // lock some doors
  for(int i = 0; i < doorCount; i++) {
    Sint16 mapx = door[i][0];
    Sint16 mapy = door[i][1];
    lockLocation(map, mapx, mapy);
  }
  // lock some teleporters
  for(int i = 0; i < (int)teleporterX.size(); i++) {
    lockLocation(map, teleporterX[i], teleporterY[i]);
  }

}

void TerrainGenerator::lockLocation(Map *map, int mapx, int mapy) {
  if((int)(LOCKED_DOOR_RAND * rand() / RAND_MAX) == 0) {
    //cerr << "\t*** Locking door: " << mapx << "," << mapy << " roomIndex=" << getRoomIndex(mapx, mapy) << endl;
    // lock the door
    map->setLocked(mapx, mapy, 0, true);
    // find an accessible location for the switch
    int nx, ny;
    Shape *lever = scourge->getShapePalette()->findShapeByName("SWITCH_OFF");
    getRandomLocation(map, lever, &nx, &ny, true, 
                      toint(scourge->getParty()->getPlayer()->getX()), 
                      toint(scourge->getParty()->getPlayer()->getY()));
    if( nx < MAP_WIDTH ) {
//      Location *pos = map->getLocation(mapx, mapy, 0);
//      cerr << "*** Locking " << pos->shape->getName() << ": " << 
//        mapx << "," << mapy << " roomIndex=" << getRoomIndex(mapx, mapy) << 
//        " with lever at: " << nx << "," << ny << " roomIndex=" << getRoomIndex(nx, ny) << endl;
      // place the switch
      addItem(scourge->getMap(), NULL, NULL, lever, nx, ny, 0);
      // connect the switch and the door
      map->setKeyLocation(mapx, mapy, 0, nx, ny, 0);
    } else {
      //cerr << "\t\t*** Room not locked." << endl;
      // if none found, unlock the door
      map->removeLocked(mapx, mapy, 0);
    }
  }
}

void TerrainGenerator::calculateRoomValues(Map *map, ShapePalette *shapePal) {
  // see which rooms are locked
  map->configureAccessMap(toint(scourge->getParty()->getPlayer()->getX()), 
                          toint(scourge->getParty()->getPlayer()->getY()));
  for(int i = 0; i < roomCount; i++) {
    int x = offset + room[i].x * unitSide + room[i].w  * (unitSide / 2);
    int y = offset + room[i].y * unitSide + room[i].h * ( unitSide / 2);
    if(!map->isPositionAccessible(x, y)) {
      room[i].valueBonus++;
//      cerr << "\tRoom " << i << " is locked. valueBonus=" << room[i].valueBonus << endl;
    } 
  }
}

void TerrainGenerator::createFreeSpaceMap(Map *map, ShapePalette *shapePal) {
  // Collapse the free space and put objects in the available spots
  ff = (Sint16*)malloc( 2 * sizeof(Sint16) * MAP_WIDTH * MAP_DEPTH );
  if(!ff) {
    fprintf(stderr, "out of mem\n");
    exit(0);    
  }
  ffCount = 0;
  for(int fx = offset; fx < MAP_WIDTH; fx+=unitSide) {
    for(int fy = offset; fy < MAP_DEPTH; fy+=unitSide) {
      if(map->getFloorPosition(fx, fy + unitSide)) {
        for(int ffx = 0; ffx < unitSide; ffx++) {
          for(int ffy = unitSide; ffy > 0; ffy--) {
            if(!map->getLocation(fx + ffx, fy + ffy, 0)) {
              *(ff + ffCount * 2) = fx + ffx;
              *(ff + ffCount * 2 + 1) = fy + ffy;
              ffCount++;
            }
          }
        }
      }
    }
  } 
}

void TerrainGenerator::deleteFreeSpaceMap(Map *map, ShapePalette *shapePal) {
  // free empty space container
  free(ff);  
}

// =================================================================
// Utilities
// FIXME: low likelyhood of infinite loop
void TerrainGenerator::getRandomLocation(Map *map, Shape *shape, 
                                         int *xpos, int *ypos, 
                                         bool accessible, int fromX, int fromY) {

  if(accessible) {
    map->configureAccessMap(fromX, fromY);
  }

  int maxCount = 500; // max # of tries to find accessible location
  int count = 0;
  int x, y;
  while(1) {
    // get a random location
    int n = (int)((float)ffCount * rand()/RAND_MAX);
    x = ff[n * 2];
    y = ff[n * 2 + 1];

    // can it fit?
    bool fits = map->shapeFits(shape, x, y, 0);
    // doesn't fit? try again (could be inf. loop)
    if(fits && 
       !coversDoor(map, scourge->getShapePalette(), shape, x, y)) {

      // check if location is accessible
      if(accessible) {
        if(!map->isPositionAccessible(x, y)) {
          count++;
          if(count >= maxCount) {
            // we failed.
            *xpos = *ypos = MAP_WIDTH;
            return;
          }
          continue;
        }
      }



      // remove from ff list
      for(int i = n + 1; i < ffCount - 1; i++) {
        ff[i * 2] = ff[i * 2 + 2];
        ff[i * 2 + 1] = ff[ i * 2 + 3];
      }
      ffCount--;
      // return result
      *xpos = x;
      *ypos = y;
      return;
    }
  } 
}

void TerrainGenerator::addItemsInEveryRoom( RpgItem *rpgItem, int n ) {
  for(int i = 0; i < roomCount; i++) {
    addItemsInRoom(rpgItem, n, i );
  }
}

void TerrainGenerator::addItemsInRoom( RpgItem *rpgItem, int n, int room ) {
  int x, y;
  for(int r = 0; r < n; r++) {
    for(int t = 0; t < 5; t++) { // 5 tries
      Shape *shape = scourge->getShapePalette()->getShape(rpgItem->getShapeIndex());
      bool fits = getLocationInRoom(scourge->getMap(), room, shape, &x, &y);
      if(fits && !coversDoor(scourge->getMap(), scourge->getShapePalette(), shape, x, y)) {
        Item *item = scourge->getSession()->newItem(rpgItem);
        addItem(scourge->getMap(), NULL, item, NULL, x, y);
        break;
      }
    }
  }
}

bool TerrainGenerator::addShapeInARoom( Shape *shape ) {
	for(int tt = 0; tt < 5; tt++) { // 5 room tries
		int room = (int)(roomCount * rand() / RAND_MAX);	
    if( addShapeInRoom( shape, room ) ) return true;
	}
	return false;
}

Location *TerrainGenerator::addShapeInRoom( Shape *shape, int room, DisplayInfo *di ) {
  int x, y;
  for(int t = 0; t < 5; t++) { // 5 tries
    bool fits = getLocationInRoom(scourge->getMap(), room, shape, &x, &y);
    if(fits && !coversDoor(scourge->getMap(), scourge->getShapePalette(), shape, x, y)) {
      addItem(scourge->getMap(), NULL, NULL, shape, x, y, 0, di );
      return scourge->getMap()->getLocation( x, y, 0 );
    }
  }
  return NULL;
}

// return false if the creature won't fit in the room
bool TerrainGenerator::getLocationInRoom(Map *map, int roomIndex, Shape *shape, 
										 int *xpos, int *ypos,
										 bool startMiddle) {

  int startx = offset + room[roomIndex].x * unitSide + unitOffset;
  int endx = offset + (room[roomIndex].x + room[roomIndex].w) * unitSide;
  int starty = offset + room[roomIndex].y * unitSide + unitOffset;
  int endy = offset + (room[roomIndex].y + room[roomIndex].h) * unitSide;

  Sint16* fff = (Sint16*)malloc( 2 * sizeof(Sint16) * (endx - startx) * (endy - starty) );  

  int count = 0;
  for(int n = 0; n < ffCount; n++) {
	if(ff[n * 2] >= startx && ff[n * 2] < endx &&
	   ff[n * 2 + 1] >=starty && ff[n * 2 + 1] < endy) {
	  fff[count * 2] = ff[n * 2];
	  fff[count * 2 + 1] = ff[n * 2 + 1];
	  count++;
	}
  }

  bool fits = false;
  while(count > 0) {
	int pos = (int)((float)count * rand() / RAND_MAX);
	int x = fff[pos * 2];
	int y = fff[pos * 2 + 1];
	fits = map->shapeFits(shape, x, y, 0);
	if(fits) {
	  // find this location in ff list
	  for(int n = 0; n < ffCount; n++) {
		if(x == ff[n * 2] && y == ff[n * 2 + 1]) {
		  ff[n * 2] = ff[(ffCount - 1) * 2];
		  ff[n * 2 + 1] = ff[(ffCount - 1) * 2 + 1];
		  /*
		  // remove from ff list
		  for(int i = n + 1; i < ffCount - 1; i++) {
			ff[i * 2] = ff[i * 2 + 2];
			ff[i * 2 + 1] = ff[ i * 2 + 3];
		  }
		  */
		  ffCount--;
		  break;
		}
	  }
	  *xpos = x;
	  *ypos = y;
	  break;	  
	} else {
	  // "remove" this from fff (replace w. last element and decrement counter)
	  fff[pos * 2] = fff[(count - 1) * 2];
	  fff[pos * 2 + 1] = fff[(count - 1) * 2 + 1];
	  count--;
	}
  }

  free(fff);
  return fits;
}

// move this to Map class
bool TerrainGenerator::coversDoor(Map *map, ShapePalette *shapePal, 
                                  Shape *shape, int x, int y) {
  for(int ty = y - shape->getDepth() - 6; ty < y + 6; ty++) {
    for(int tx = x - 6; tx < x + shape->getWidth() + 6; tx++) {
      if(map->isDoor(tx, ty)) return true;
    }
  }
  return false;
}

bool TerrainGenerator::isAccessible(Map *map, int x, int y, int fromX, int fromY, int stepsTaken, int dir) {
  //cerr << "&&& isAccessible: x=" << x << " y=" << y << " fromX=" << fromX << " fromY=" << fromY << " dir=" << dir << endl;
  if(x == fromX && y == fromY) {
//    cerr << "&&& isAccessible is true in " << stepsTaken << " steps." << endl;
    return true;
  }
  if(stepsTaken > MAX_STEPS) {
    //cerr << "&&& isAccessible is false after " << stepsTaken << " steps." << endl;
    return false;
  }
  int ox = x;
  int oy = y;
  switch(dir) {
  case DIR_N: y--; break;
  case DIR_E: x++; break;
  case DIR_S: y++; break;  
  case DIR_W: x--; break;
  }
  bool failed = false;
  if(!(x < 0 || y < 0 || x >= MAP_WIDTH || y >= MAP_DEPTH)) {
    Location *pos = map->getLocation(x, y, 0);
    
    //if(pos) cerr << "\tpos: shape=" << pos->shape->getName() <<
    //" item=" << (pos->item ? pos->item->getRpgItem()->getName() : "null") <<
      //" creature=" << (pos->creature ? pos->creature->getName() : "null") << endl;

    // if it's not true that it's: empty space or has a creature or an item (movable things)
    // then change dir.
    if(!(!pos || pos->creature || pos->item)) {
      failed = true;
    }
  } else {
    failed = true;
  }
  if(failed) {
    dir++;
    if(dir >= DIR_COUNT) dir = DIR_N;
    x = ox;
    y = oy;
  }
  return isAccessible(map, x, y, fromX, fromY, stepsTaken + 1, dir);
}

void TerrainGenerator::addItem(Map *map, 
                               Creature *creature, 
                               Item *item, 
                               Shape *shape, 
                               int x, int y, int z, 
                               DisplayInfo *di) {
  if(creature) map->setCreature(x, y, z, creature);
  else if(item) map->setItem(x, y, z, item);
  else map->setPosition(x, y, z, shape, di);
  // remember the containers
  if(item && item->getRpgItem()->getType() == RpgItem::CONTAINER) {
    containers.push_back(item);
    containerX.push_back(x);
    containerY.push_back(y);
  }  
  // remember the teleporters
  if(shape == scourge->getShapePalette()->findShapeByName("TELEPORTER_BASE")) {
    teleporterX.push_back(x);
    teleporterY.push_back(y);
  }
}

int TerrainGenerator::getRoomIndex(int x, int y) {
  int rx = (x - offset) / unitSide;
  int ry = (y - offset) / unitSide;
  for(int i = 0; i < roomCount; i++) {
    if(rx >= room[i].x && rx < room[i].x + room[i].w &&
       ry >= room[i].y && ry < room[i].y + room[i].h)
      return i;
  }
  return -1;
}

