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
#include "cavemaker.h"
#include "dungeongenerator.h"
#include "mondrian.h"
#include "outdoorgenerator.h"

using namespace std;

// factory method
TerrainGenerator *TerrainGenerator::getGenerator( Scourge *scourge, int depth ) {
	Mission *mission = scourge->getSession()->getCurrentMission();
	TerrainGenerator *dg;
	if( depth == 0 ) {
		dg = new OutdoorGenerator( scourge, 
															 mission->getLevel(), 
															 depth,
															 mission->getDepth(),
															 ( depth < mission->getDepth() - 1 ),
															 ( depth > 0 ),
															 mission );
	} else if( strstr( mission->getMapName(), "caves" ) ) {
		dg = new CaveMaker( scourge, 
												mission->getLevel(), 
												depth,
												mission->getDepth(),
												( depth < mission->getDepth() - 1 ),
												( depth > 0 ),
												mission );
	} else {
		if( Util::dice( 5 ) == 0 ) {
			dg = new MondrianGenerator( scourge, 
																	mission->getLevel(), 
																	depth,
																	mission->getDepth(),
																	( depth < mission->getDepth() - 1 ),
																	( depth > 0 ),
																	mission );
		} else {
			dg = new DungeonGenerator( scourge, 
																 mission->getLevel(), 
																 depth,
																 mission->getDepth(),
																 ( depth < mission->getDepth() - 1 ),
																 ( depth > 0 ),
																 mission );
		}
	}
	return dg;
}											

TerrainGenerator::TerrainGenerator( Scourge *scourge, 
                                    int level, 
                                    int depth, 
																		int maxDepth,
                                    bool stairsDown, 
                                    bool stairsUp, 
                                    Mission *mission, 
                                    int progressSteps ) {
  this->scourge = scourge;
  this->level = level;
  this->depth = depth;
  this->maxDepth = maxDepth;
  this->stairsUp = stairsUp;
  this->stairsDown = stairsDown;
  this->mission = mission;
  this->stairsUpX = this->stairsUpY = this->stairsDownX = this->stairsDownY = 0;

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
  progress->updateStatus( statusMessage );
  Uint32 now = SDL_GetTicks();
//  cerr << "+++ " << statusMessage << ". Previous task's time=" << (now - start) << endl;
  start = now;
}

bool TerrainGenerator::toMap( Map *map, ShapePalette *shapePal, bool goingUp, bool goingDown ) {	 

  // set the renderer helper for this type of map
  map->setMapRenderHelper( getMapRenderHelper() );

  start = SDL_GetTicks();
  generate( map, shapePal );

  updateStatus( _( "Assembling Dungeon Level" ) );
    
  // loop until successfully drawn nodes onto map
  int status = progress->getStatus();
  for( int i = 0; i < 5; i++ ) {
    if( drawNodesOnMap( map, shapePal, goingUp, goingDown ) ) return true;
    // reset the progress
    progress->setStatus( status );
  }
  return false;
}

bool TerrainGenerator::drawNodesOnMap( Map *map, ShapePalette *shapePal, bool goingUp, bool goingDown ) {
  bool ret = drawNodes( map, shapePal );
  if( !ret ) {
		cerr << "*** Error: failed in drawNodes!" << endl;
		goto cleanup;
	}

  updateStatus( _( "Compressing free space" ) );
  createFreeSpaceMap(map, shapePal);

	// add stairs first so party can be placed nearby
	updateStatus( _( "Adding gates" ) );
  if( !addStairs(map, shapePal) ) {
    ret = false;
		cerr << "*** Error: failed in addStairs!" << endl;
    goto cleanup;
  }

	// add stuff in order of importance
	updateStatus( _( "Adding party" ) );
  if( !addParty( map, shapePal, goingUp, goingDown ) ) {
    ret = false;
		cerr << "*** Error: failed in addParty!" << endl;
    goto cleanup;
  }

	// add a teleporters
  updateStatus( _( "Adding teleporters" ) );
  if( !addTeleporters(map, shapePal) ) {
    ret = false;
		cerr << "*** Error: failed in addTeleporters!" << endl;
    goto cleanup;
  }

	updateStatus( _( "Adding mission objectives" ) );
	addMissionObjectives(map, shapePal);




  updateStatus( _( "Adding traps" ) );
  addTraps( map, shapePal );

	updateStatus( _( "Adding shapes" ) );
  addShapes(map, shapePal);
  
  updateStatus( _( "Adding containers" ) );
  addContainers(map, shapePal);  
  
  updateStatus( _( "Locking doors and chests" ) );
  lockDoors(map, shapePal);
  
	if( scourge->getParty()->getPartySize() ) {
		updateStatus( _( "Calculating room values" ) );
		calculateRoomValues(map, shapePal);
		
		updateStatus( _( "Adding monsters" ) );
		addMonsters(map, shapePal);
		addHarmlessCreatures( map, shapePal );
	}

  updateStatus( _( "Adding items" ) );
  addItems(map, shapePal);
  
  updateStatus( _( "Adding furniture" ) );
  addFurniture(map, shapePal);
  
cleanup:
  updateStatus( _( "Cleaning up" ) );
  deleteFreeSpaceMap(map, shapePal);
  
  return ret;

}

void TerrainGenerator::addShapes(Map *map, ShapePalette *shapePal) {
	cerr << "**** Current theme: " << shapePal->getCurrentThemeName() << endl;
	for( int i = 1; i < shapePal->getShapeCount(); i++ ) {
		GLShape *shape = shapePal->getShape( i );
		if( ( !strlen( shape->getOccurs()->theme ) || 
					!strcmp( shape->getOccurs()->theme, shapePal->getCurrentThemeName() ) ) &&
				shape->getOccurs() && 
				shape->getOccurs()->max_count > 0 ) {
			cerr << "\t**** adding shape: " << shape->getName() << " max-count=" << shape->getOccurs()->max_count << endl;
			for( int t = 0; t < shape->getOccurs()->max_count; t++ ) {
				if( shape->getOccurs()->rooms_only ) {
					for( int r = 0; r < roomCount; r++ ) {
						addShapeInRoom( shape, r );
					}
				} else {
					int xpos, ypos;
					getRandomLocationSimple( map, shape, &xpos, &ypos );
					if( xpos > -1 && ypos > -1 ) {
						addItem( map, NULL, NULL, shape, xpos, ypos );
					}
				}
			}
		}
	}
}

void TerrainGenerator::addContainers(Map *map, ShapePalette *shapePal) {
  int x = 0;
  int y = 0;
  
  for( int i = 0; i < objectCount; i++ ) {
    RpgItem *rpgItem;
	if ( Util::dice( 2 ) ) {
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
    for(int i = 0; i < roomCount; i++) {
      Shape *shape = scourge->getShapePalette()->findShapeByName("GATE_UP");
      int x, y;
      bool fits = getLocationInRoom(map, i, shape, &x, &y);
			if(fits && !map->coversDoor(shape, x, y)) {
        addItem(map, NULL, NULL, shape, x, y);
				stairsUpX = x;
				stairsUpY = y;
        done = true;
        break;
      }
    }
    if(!done) {
			char name[255];
			getName( name );
			cerr << "Error: couldn't add up stairs." << name << endl;
			printMaze();
			return false;
    }
  }
  if(stairsDown) {
    bool done = false;
    for(int i = roomCount - 1; i >= 0; i--) {
      Shape *shape = scourge->getShapePalette()->findShapeByName("GATE_DOWN");
      int x, y;
      bool fits = getLocationInRoom(map, i, shape, &x, &y);
      if(fits && !map->coversDoor(shape, x, y)) {
        addItem(map, NULL, NULL, shape, x, y);
				stairsDownX = x;
				stairsDownY = y;
				done = true;
        break;
      }
    }
    if(!done) {
      char name[255];
			getName( name );
			cerr << "Error: couldn't add down stairs." << name << endl;
			printMaze();
			return false;
    }
  }

  return true;
}

void TerrainGenerator::addItems(Map *map, ShapePalette *shapePal) {
  // add the items
  for(int i = 0; i < objectCount; i++) {
    Item *item = scourge->getSession()->getGameAdapter()->createRandomItem( level, depth );
    if( item ) {
      int x, y;
      getRandomLocation(map, item->getShape(), &x, &y);
      addItem(map, NULL, item, NULL, x, y);
    }
  }

  // add some scrolls with spells
	int spellLevel = level / 5;
  for(int i = 0; i < objectCount / 4; i++) {
    Spell *spell = MagicSchool::getRandomSpell(spellLevel);
    if(!spell) {
      cerr << "Warning: no spells defined for level: " << spellLevel << endl;
      break;
    }
    Item *item = scourge->getSession()->
      newItem(RpgItem::getItemByName("Scroll"), level, spell);
    int x, y;
    getRandomLocation(map, item->getShape(), &x, &y);
    addItem(map, NULL, item, NULL, x, y);
  }

  // populate containers
  for(int i = 0; i < static_cast<int>(containers.size()); i++) {
    Item *item = containers[i];
    int cx = containerX[i];
    int cy = containerY[i];
    int roomIndex = getRoomIndex(cx, cy);
    int valueBonus = 0;
    if(roomIndex > -1) valueBonus = room[roomIndex].valueBonus;
		
		scourge->getSession()->getGameAdapter()->fillContainer( item, level, depth );    
  }
}

// put one mission object (or creature) starting from the deepest level. 
// if there are more objects (or creatures) than levels then put one
// at all levels and rest at deepest level

void TerrainGenerator::addMissionObjectives(Map *map, ShapePalette *shapePal) {
  if( mission && !mission->isCompleted() ) {
		int startIndex, endIndex;
		int items = mission->getItemCount();
		if ( depth + items >= maxDepth && depth < maxDepth ) { // there are items 
			if( depth == maxDepth - 1 && items > maxDepth ) { // there are multiple items
				startIndex = 0;
				endIndex = items - maxDepth + 1;
			} else if ( items <= maxDepth ) { 
				startIndex = maxDepth - depth - 1;
				endIndex = startIndex + 1;
			} else { 
				startIndex = items - depth - 1;
				endIndex = startIndex + 1;
			}
			//cerr << "*** Added mission items: from " << startIndex << " to " << endIndex << endl;
			// mission objects are on a pedestal
			// and they are blocking so creatures can't get them
			for ( int i = startIndex; i < endIndex; i++ ) {
				RpgItem *rpgItem = mission->getItem( i );
				Item *item = scourge->getSession()->newItem( rpgItem, mission->getLevel() );
				//mission->addItemInstance( item, rpgItem );
				item->setMissionObjectInfo( mission->getMissionId(), i );
				item->setBlocking(true); // don't let monsters pick this up
				Item *pedestal = scourge->getSession()->newItem(RpgItem::getItemByName("Pedestal"));
				int x, y;
				getRandomLocation(map, pedestal->getShape(), &x, &y);
				addItem(map, NULL, pedestal, NULL, x, y);
				addItem(map, NULL, item, NULL, 
								x + (pedestal->getShape()->getWidth()/2) - (item->getShape()->getWidth()/2), 
								y - (pedestal->getShape()->getDepth()/2) + (item->getShape()->getDepth()/2), 
								pedestal->getShape()->getHeight());
				//cerr << "*** Added mission item: " << item->getItemName() << " at: " << x << "," << y << endl;
			}
		}

		int creatures = mission->getCreatureCount();
		if ( depth + creatures >= maxDepth && depth < maxDepth ) { // there are creatures
			if( depth == maxDepth - 1 && creatures > maxDepth ) { // there are multiple creatures
				startIndex = 0;
				endIndex = creatures - maxDepth + 1;
			} else if ( creatures <= maxDepth ) { 
				startIndex = maxDepth - depth - 1;
				endIndex = startIndex + 1;
			} else { 
				startIndex = creatures - depth - 1;
				endIndex = startIndex + 1;
			}
			//cerr << "*** Added mission creatures: from " << startIndex << " to " << endIndex << endl;
			// add mission creatures
			for(int i = startIndex; i < endIndex; i++) {
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
	      //cerr << "*** Added mission monster: " << creature->getMonster()->getType() << endl;
			}
		}
  }
}

void TerrainGenerator::addMonsters(Map *levelMap, ShapePalette *shapePal) {
	bool dungeonBoss = false;
  // add monsters in every room
  if(monsters) {
    int totalLevel = static_cast<int>( static_cast<float>(scourge->getParty()->getTotalLevel()) * getMonsterLevelMod() );
    //fprintf(stderr, "creating monsters for total player level: %d\n", totalLevel);
    for(int i = 0; i < roomCount; i++) {
			// the first room should have less monsters than the rest (this is where the party starts.)
			int totalLevelUsed = ( i == 0 ? totalLevel / 2 : totalLevel );
      int areaCovered = 0;
      // don't crowd the rooms
      int roomAreaUsed = static_cast<int>(room[i].w * room[i].h * unitSide * 0.33f);
      int monsterLevelTotal = 0;
      bool badAssMonsters = 
				( i > 0 && 
					getUseBadassMonsters() && 
					( Util::dice( 10 - depth ) == 0 ) );
      while( areaCovered < roomAreaUsed && 
						 ( badAssMonsters || 
							 monsterLevelTotal < totalLevelUsed ) ) {
				bool boss = false;
        int monsterLevel = getBaseMonsterLevel();
        if( badAssMonsters ) {
					if( 0 == Util::dice( 5 ) ) {
						monsterLevel++;
					}          
					if( !dungeonBoss ) {
						boss = true;
						monsterLevel += depth;
					}
				}
        Monster *monster = Monster::getRandomMonster(monsterLevel);
        //fprintf(stderr, "Trying to add %s to room %d\n", monster->getType(), i);
        if(!monster) {
          cerr << "* Warning: no monsters defined for level: " << level << endl;
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
					if( boss ) {
						cerr << "+++ Adding boss monster! " << creature->getName() << " level=" << monsterLevel << " for depth=" << depth << endl;
						creature->setBoss( true );
						dungeonBoss = true;
					}
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
      Monster *monster = Monster::getRandomMonster(getBaseMonsterLevel());
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

		/*
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
		*/

    // add npc-s
    for(int i = 0; i < roomCount; i++) {
      int areaCovered = 0;
      // don't crowd the rooms
      int roomAreaUsed = static_cast<int>(room[i].w * room[i].h * unitSide * 0.33f);
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

void TerrainGenerator::addHarmlessCreatures(Map *levelMap, ShapePalette *shapePal) {
	// add a few harmless creatures
	for(int i = 0; i < objectCount * 2; i++) {
		Monster *monster = (Monster*)Monster::getRandomHarmless();
		if( !monster ) {
			cerr << "Warning: no harmless creatures defined." << endl;
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
}																															 	

void TerrainGenerator::addMagicPools( Map *map, ShapePalette *shapePal ) {
	// add some magic pools
  DisplayInfo di;
  for( int i = 0; i < roomCount; i++ ) {
    //???: if( 0 == static_cast<int>( 0.0f * rand() / RAND_MAX ) ) {
      MagicSchool *ms = MagicSchool::getRandomSchool();
      di.red = ms->getDeityRed();
      di.green = ms->getDeityGreen();
      di.blue = ms->getDeityBlue();
      Location *pos = addShapeInRoom( scourge->getShapePalette()->findShapeByName("POOL"), i, &di );
      if( pos ) {
        // store pos->deity in scourge
        scourge->addDeityLocation( pos, ms );
      }
    //}
  }
}

void TerrainGenerator::addFurniture(Map *map, ShapePalette *shapePal) {
  // add tables, chairs, etc.
  addItemsInEveryRoom(RpgItem::getItemByName("Table"), 1);
  addItemsInEveryRoom(RpgItem::getItemByName("Chair"), 2);  
  addMagicPools( map, shapePal );
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
  bool b = (teleportersAdded > 0);
	if( !b ) {
		char name[255];
		getName( name );
		cerr << "Error: couldn't add a teleporter." << name << endl;
		printMaze();
	}
	return b;
}

/**
 * Add the party somewhere near the middle of the first room.
 * See warning notes on this approach in findPlace() and loadMap() 
 * descriptions.
 */
bool TerrainGenerator::addParty( Map *map, ShapePalette *shapePal, bool goingUp, bool goingDown ) {
  int xx, yy;
  if( goingDown && stairsUpX > 0 ) {
  	cerr << "TERRAINGEN. Starting at up-stairs." << endl;
  	xx = stairsUpX;
  	yy = stairsUpY;
  } else if( goingUp && stairsDownX > 0 ) {
  	cerr << "TERRAINGEN. Starting at down-stairs." << endl;
  	xx = stairsDownX;
  	yy = stairsDownY;
  } else {
  	cerr << "TERRAINGEN. Starting in middle of room 0." << endl;
  	getPartyStartingLocation( &xx, &yy );
  }
  int nx, ny;
  for( int r = 0; r < scourge->getParty()->getPartySize(); r++ ) {
    if( !scourge->getParty()->getParty(r)->getStateMod( StateMod::dead ) ) {
      scourge->getParty()->getParty(r)->findPlace( xx, yy, &nx, &ny );
      if( nx == -1 && ny == -1 ) {
				char name[255];
				getName( name );
				cerr << "Error placing party. Type=" << name << endl;
				printMaze();
				return false;
			}
      xx = nx;
      yy = ny;
    }
  }
  return true;
}

void TerrainGenerator::getPartyStartingLocation( int *xx, int *yy ) {
	*xx = MAP_OFFSET + ( room[0].x + room[0].w / 2 ) * MAP_UNIT;
	*yy = MAP_OFFSET + ( room[0].y + room[0].h / 2 ) * MAP_UNIT;	
}

void TerrainGenerator::lockDoors(Map *map, ShapePalette *shapePal) {
  // lock some doors
  for(int i = 0; i < doorCount; i++) {
    Sint16 mapx = door[i][0];
    Sint16 mapy = door[i][1];
    lockLocation(map, mapx, mapy);
  }
  // lock some teleporters
  for(int i = 0; i < static_cast<int>(teleporterX.size()); i++) {
    lockLocation(map, teleporterX[i], teleporterY[i]);
  }

}

void TerrainGenerator::lockLocation(Map *map, int mapx, int mapy) {
  if( Util::dice( LOCKED_DOOR_RAND ) == 0 ) {
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
						int gx = ( fx + ffx ) / OUTDOORS_STEP;
						int gy = ( fy + ffy ) / OUTDOORS_STEP;
            if( !map->getLocation(fx + ffx, fy + ffy, 0) && 
								map->getGroundHeight( gx, gy ) < 10 ) {
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
    int n = static_cast<int>(Util::roll( 0, ffCount-1 ));
    x = ff[n * 2];
    y = ff[n * 2 + 1];

    // can it fit?
    bool fits = map->shapeFits(shape, x, y, 0);
    // doesn't fit? try again (could be inf. loop)
    if(fits && 
       !map->coversDoor(shape, x, y)) {

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

void TerrainGenerator::getRandomLocationSimple( Map *map, Shape *shape, 
                                                int *xpos, int *ypos ) {
  int x, y;
  for( int i = 0; i < 500; i++ ) {
    // get a random location
    int n = static_cast<int>(Util::roll( 0, ffCount-1 ));
    x = ff[ n * 2 ];
    y = ff[ n * 2 + 1 ];

    // can it fit?
    bool fits = map->shapeFits( shape, x, y, 0 );
    // doesn't fit? try again (could be inf. loop)
    if( fits ) {
      // return result
      *xpos = x;
      *ypos = y;
      return;
    }
  }
  *xpos = *ypos = -1;
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
      if(fits && !scourge->getMap()->coversDoor(shape, x, y)) {
        Item *item = scourge->getSession()->newItem(rpgItem);
        addItem(scourge->getMap(), NULL, item, NULL, x, y);
        break;
      }
    }
  }
}

bool TerrainGenerator::addShapeInARoom( Shape *shape ) {
	for(int tt = 0; tt < 5; tt++) { // 5 room tries
		int room = Util::dice( roomCount );	
    if( addShapeInRoom( shape, room ) ) return true;
	}
	return false;
}

Location *TerrainGenerator::addShapeInRoom( Shape *shape, int room, DisplayInfo *di ) {
  int x, y;
  for(int t = 0; t < 5; t++) { // 5 tries
    bool fits = getLocationInRoom(scourge->getMap(), room, shape, &x, &y);
    if(fits && !scourge->getMap()->coversDoor(shape, x, y)) {
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
	int pos = Util::dice( count );
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

void TerrainGenerator::addRugs( Map *map, ShapePalette *shapePal ) {
	//cerr << "*** Adding rugs" << endl;
	for(int roomIndex = 0; roomIndex < roomCount; roomIndex++) {
    int startx = room[roomIndex].x;
    //int endx = room[roomIndex].x + room[roomIndex].w - 1;
    int starty = room[roomIndex].y;
    //int endy = room[roomIndex].y + room[roomIndex].h - 1;

		int n = Util::dice( 5 );
		for( int i = 0; i < n; i++ ) {
			// pick a random location in the room
			int px = startx + Util::dice( room[roomIndex].w );
			int py = starty + Util::dice( room[roomIndex].h );
			if( !map->hasRugAtPosition( px, py ) ) {
			
				// pick an orientation
				bool isHorizontal = ( Util::dice( 2 ) == 0 );
	
				// save it
				Rug rug;
				rug.isHorizontal = isHorizontal;
				rug.texture = shapePal->getRandomRug();
				rug.angle = Util::roll( -15.0f, 15.0f );
	
//				cerr << "*** Adding rug (tex: " << rug.texture << ") at " << px << "," << py << 
//					" room: " << room[roomIndex].x << "," << room[roomIndex].y << 
//					"-" << ( room[roomIndex].w + room[roomIndex].x ) << "," <<
//					( room[roomIndex].h + room[roomIndex].y ) << endl;
				map->setRugPosition( px, py, &rug );
			}
		}
	}
}

void TerrainGenerator::addTraps( Map *map, ShapePalette *shapePal ) {
  GLShape *dummy = new GLShape( 0, 1, 1, 1, "dummy", 0, 0, 0 );
  int trapCount = Util::pickOne( 3, 7 );
  for( int n = 0; n < trapCount; n++ ) {
    int x, y;
    getRandomLocationSimple( map, dummy, &x, &y );

    int w = Util::pickOne( 4, 9 );
    int h = Util::pickOne( 4, 9 );
    map->addTrap( x, y, w, h );
  }
  delete dummy;
}

// ----------------------------------
// Room helper functions
void TerrainGenerator::addContainersInRooms( Map *map, ShapePalette *shapePal ) {
	int x = 0;
	int y = 0;
	RpgItem *rpgItem;
	// add the containers
	for(int i = 0; i < roomCount; i++) {
	  for(int pos = unitOffset; pos < room[i].h * unitSide; pos++) {
	    rpgItem = RpgItem::getRandomContainer();
	    if(rpgItem) {
	      // WEST side
	      x = (room[i].x * unitSide) + unitOffset + offset;
	      y = (room[i].y * unitSide) + pos + offset;
	      Shape *shape = scourge->getShapePalette()->getShape(rpgItem->getShapeIndex());
	      if(map->shapeFits(shape, x, y, 0) && 
	         !map->coversDoor(shape, x, y)) {
	        addItem(map, NULL, scourge->getSession()->newItem(rpgItem), NULL, x, y);
	      }
	    }
	    rpgItem = RpgItem::getRandomContainer();
	    if(rpgItem) {
	      // EAST side
	      x = ((room[i].x + room[i].w - 1) * unitSide) + unitSide - (unitOffset * 2) + offset;
	      Shape *shape = scourge->getShapePalette()->getShape(rpgItem->getShapeIndex());
	      if(map->shapeFits(shape, x, y, 0) && 
	         !map->coversDoor(shape, x, y)) {
	        addItem(map, NULL, scourge->getSession()->newItem(rpgItem), NULL, x, y);
	      }
	    }
	  }
	  for(int pos = unitOffset; pos < room[i].w * unitSide; pos++) {
	    rpgItem = RpgItem::getRandomContainerNS();
	    if(rpgItem) {
	      // NORTH side
	      x = (room[i].x * unitSide) + pos + offset;
	      y = (room[i].y * unitSide) + (unitOffset * 2) + offset;
	      Shape *shape = scourge->getShapePalette()->getShape(rpgItem->getShapeIndex());
	      if(map->shapeFits(shape, x, y, 0) && 
	         !map->coversDoor(shape, x, y)) {
	        addItem(map, NULL, scourge->getSession()->newItem(rpgItem), NULL, x, y);
	      }
	    }
	    rpgItem = RpgItem::getRandomContainerNS();
	    if(rpgItem) {
	      // SOUTH side
	      y = ((room[i].y + room[i].h - 1) * unitSide) + unitSide - unitOffset + offset;
	      Shape *shape = scourge->getShapePalette()->getShape(rpgItem->getShapeIndex());
	      if(map->shapeFits(shape, x, y, 0) && 
	         !map->coversDoor(shape, x, y)) {
	        addItem(map, NULL, scourge->getSession()->newItem(rpgItem), NULL, x, y);
	      }
	    }
	  }
	}
}