/***************************************************************************
                          scourge.cpp  -  description
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

#include "scourge.h"
#include "events/thirsthungerevent.h"

#define GUI_WIDTH 220
#define GUI_HEIGHT 125


// 2,3  2,6  3,6*  5,1+  6,3   8,3*


// good for debugging blending
int Scourge::blendA = 2;
int Scourge::blendB = 6;     // 3
int Scourge::blend[] = { 
    GL_ZERO, GL_ONE, GL_SRC_COLOR, GL_ONE_MINUS_SRC_COLOR, 
    GL_DST_COLOR, GL_ONE_MINUS_DST_COLOR, GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA,
    GL_DST_ALPHA, GL_ONE_MINUS_DST_ALPHA, GL_SRC_ALPHA_SATURATE
};

void Scourge::setBlendFunc() {
    glBlendFunc(blend[blendA], blend[blendB]);
}

Scourge::Scourge(int argc, char *argv[]){
  lastTick = 0;
  mainWin = NULL;
  messageWin = NULL;
  movingX = movingY = movingZ = MAP_WIDTH + 1;
  movingItem = NULL;
  nextMission = -1;
  // in HQ map
  inHq = true;
  
  isInfoShowing = true; // what is this?
  info_dialog_showing = false;

  // new item and creature references
  itemCount = creatureCount = 0;
  
  // Reads the user configuration from a file      
  userConfiguration = new UserConfiguration();  
  userConfiguration->loadConfiguration();    
  userConfiguration->parseCommandLine(argc, argv);  
   
  // Initialize the video mode
  sdlHandler = new SDLHandler(); 
  sdlHandler->setVideoMode(userConfiguration); 
  
  shapePal = sdlHandler->getShapePalette();  
  calendar = Calendar::getInstance();  

  // initialize the items
  initItems();
  // initialize the monsters (they use items)
  Monster::initMonsters();
  Character::initCharacters();

  // create the mission board
  board = new Board(this);

  // do this before the inventory and optionsdialog (so Z is less than of those)
  createUI();

  player_only = false;
  move = 0;
  startRound = true;
  battleCount = 0;  
  inventory = NULL;
  containerGuiCount = 0;
  changingStory = false;
  
  for(int i = 0; i < 4; i++) party[i] = NULL;

  // show the main menu
  mainMenu = new MainMenu(this);
  optionsMenu = new OptionsMenu(this);

  while(true) {
	mainMenu->show();    
	sdlHandler->setHandlers((SDLEventHandler *)mainMenu, (SDLScreenView *)mainMenu);
	sdlHandler->mainLoop();
	mainMenu->hide();

    // evaluate results and start a missions
    if(mainMenu->getValue() == NEW_GAME) {

	  // Init the party; hard code for now
	  // This will be replaced by a call to the character builder which either
	  // loads or creates the party.
	  for(int i = 0; i < 4; i++) {
		if(party[i]) {
		  delete party[i];
		}
	  }
	  Creature **pc = Creature::createHardCodedParty(this);
	  party[0] = player = pc[0];
	  party[1] = pc[1];
	  party[2] = pc[2];
	  party[3] = pc[3];	  	  

	  // inventory needs the party
	  if(!inventory) {
		inventory = new Inventory(this);
	  }

	  // always start in hq
	  nextMission = -1;
	  inHq = true;

      startMission();
	} else if(mainMenu->getValue() == OPTIONS) {
	  optionsMenu->show();
    } else if(mainMenu->getValue() == QUIT) {
      sdlHandler->quit(0);
    }
  }
}

void Scourge::initItems() {
  char errMessage[500];
  char s[200];
  sprintf(s, "data/world/items.txt");
  FILE *fp = fopen(s, "r");
  if(!fp) {        
	sprintf(errMessage, "Unable to find the file: %s!", s);
	cerr << errMessage << endl;
	exit(1);
  }

  int itemCount = 0;
  char name[255], type[255], shape[255], skill[255];
  char long_description[500], short_description[120];
  char line[255];
  int n = fgetc(fp);
  while(n != EOF) {
	if(n == 'I') {
	  // skip ':'
	  fgetc(fp);
	  // read the rest of the line
	  n = Constants::readLine(name, fp);
	  n = Constants::readLine(line, fp);
	  int level = atoi(strtok(line + 1, ","));
	  char *p = strtok(NULL, ",");
	  int action = 0;
	  int speed = 0;
	  int distance = 0;
	  int currentCharges = 0;
	  int maxCharges = 0;
	  if(p) {
		action = atoi(p);
		speed = atoi(strtok(NULL, ","));
		distance = atoi(strtok(NULL, ","));
		currentCharges = atoi(strtok(NULL, ","));
		maxCharges = atoi(strtok(NULL, ","));
	  }
	  n = Constants::readLine(line, fp);
	  strcpy(type, strtok(line + 1, ","));
	  float weight = strtof(strtok(NULL, ","), NULL);
	  int price = atoi(strtok(NULL, ","));

	  int inventory_location = 0;
	  int twohanded = 0;
	  strcpy(shape, "");
	  strcpy(skill, "");
	  p = strtok(NULL, ",");	  
	  if(p) {
		strcpy(shape, p);
		p = strtok(NULL, ",");
		if(p) {
		  inventory_location = atoi(p);
		  twohanded = atoi(strtok(NULL, ","));
		  strcpy(skill, strtok(NULL, ","));
		}
	  }
	  
	  n = Constants::readLine(line, fp);
	  strcpy(long_description, line + 1);
	  
	  n = Constants::readLine(line, fp);
	  strcpy(short_description, line + 1);
	  
	  // resolve strings
	  int type_index = RpgItem::getTypeByName(type);	  
	  int shape_index = shapePal->findShapeIndexByName(shape);
	  int skill_index = Constants::getSkillByName(skill);
	  if(skill_index < 0) {
		if(strlen(skill)) cerr << "*** WARNING: cannot find skill: " << skill << endl;
		skill_index = 0;
	  }
	  RpgItem::addItem(new RpgItem(itemCount++, strdup(name), level, type_index, 
								   weight, price, 100, 
								   action, speed, strdup(long_description), 
								   strdup(short_description), 
								   inventory_location, shape_index, 
								   twohanded, distance, skill_index, currentCharges, 
								   maxCharges));	  
	} else {
	  n = Constants::readLine(line, fp);
	}
  }
  fclose(fp);
}

Scourge::~Scourge(){
  delete mainMenu;
  delete optionsMenu;
  delete userConfiguration;
  delete calendar;
	for(int i = 0; i < Board::MAX_AVAILABLE_MISSION_COUNT; i++) {
		free(missionText[i]);
	}
	free(missionText);
	delete board;
}

void Scourge::startMission() {
  
  while(true) {
	
	// add gui
	mainWin->setVisible(true);
	messageWin->setVisible(true);
	
	// create the map
	map = new Map(this);
	miniMap = new MiniMap(this); 
	
	/*
	  cerr << "Before mission: " <<
	  " creatureCount=" << creatureCount << 
	  " itemCount=" << itemCount << endl;
	*/
	
	// Start calendar and add thirst & hunger event scheduling
	calendar->reset();
	Event *e;  
	Date d(0, 0, 6, 0, 0, 0); // 6 hours (format : sec, min, hours, days, months, years)
	for(int i = 0; i < 4 ; i++){
   	  e = new ThirstHungerEvent(calendar->getCurrentDate(), d, party[i], this, Event::INFINITE_EXECUTIONS);
   	  calendar->scheduleEvent((Event*)e);   // It's important to cast!!
	}
	
	// position the players
	player_only = false;
	move = 0;
	//startRound = true;
	battleCount = 0;
	partyDead = false;
	containerGuiCount = 0;
	lastMapX = lastMapY = lastMapZ = lastX = lastY = -1;
	teleporting = false;
	changingStory = false;
	
	// set player to be the first non-dead character
	for(int i = 0; i < 4; i++) {
	  if(!party[i]->getStateMod(Constants::dead)) {
		setPlayer(getParty(i));
		break;
	  }
	}
	setFormation(Constants::DIAMOND_FORMATION - Constants::DIAMOND_FORMATION);
	getPlayer()->setTargetCreature(NULL);
	
	// init the rest of the party
	for(int i = 1; i < 4; i++) {
	  getParty(i)->setNext(getPlayer(), i);
	  getParty(i)->setTargetCreature(NULL);
	}
	
	
	
	
	if(nextMission == -1) {
	  
	  // in HQ map
	  inHq = true;
	  
	  // init the missions board
	  board->initMissions();
	  
	  // display the HQ map
	  dg = new DungeonGenerator(this, 2, false, false); // level 2 is a big enough map for HQ_LOCATION... this is hacky
	  dg->toMap(map, getShapePalette(), DungeonGenerator::HQ_LOCATION);		
	} else {
	  // in HQ map
	  inHq = false;
	  
	  // Initialize the map with a random dunegeon	
	  const Mission *mission = board->getMission(nextMission);
	  cerr << "Starting mission: level="  << mission->level << 
		" stories=" << mission->dungeonStoryCount << 
		" current story=" << currentStory << endl;
	  dg = new DungeonGenerator(this, mission->level, 
								(currentStory < mission->dungeonStoryCount - 1), 
								(currentStory > 0));
	  dg->toMap(map, getShapePalette());
	}
	
	// center map on the player
	map->center(player->getX(), player->getY());
	
	// Must be called after MiniMap has been built by dg->toMap() !!! 
	miniMap->computeDrawValues();
	
	// set to receive events here
	sdlHandler->setHandlers((SDLEventHandler *)this, (SDLScreenView *)this);
	
	// hack to unfreeze animations, etc.
	startRound = false;
	toggleRound();

	// show an info dialog
	if(nextMission == -1) {
	  sprintf(infoMessage, "Welcome to the S.C.O.U.R.G.E. Head Quarters");
	} else {
	  sprintf(infoMessage, "Entering dungeon level %d", currentStory);
	}
	showMessageDialog(infoMessage);
	info_dialog_showing = true;
	
	
	// run mission
	sdlHandler->mainLoop();
	
	
	
	
	// remove gui
	mainWin->setVisible(false);
	messageWin->setVisible(false);
	closeAllContainerGuis();
	if(inventory->isVisible()) inventory->hide();
	if(boardWin->isVisible()) boardWin->setVisible(false);
	
	// clean up after the mission
	delete map;
	delete miniMap;
	delete dg;
	
	// delete the items and creatures created for this mission
	// (except items in inventory) 
	for(int i = 0; i < itemCount; i++) {
	  bool inInventory = false;
	  for(int t = 0; t < 4; t++) {
		if(getParty(t)->isItemInInventory(items[i])) {
		  inInventory = true;
		  break;
		}
	  }
	  if(!inInventory) {
		delete items[i];
		itemCount--;
		for(int t = i; t < itemCount; t++) {
		  items[t] = items[t + 1];
		}
		i--;
	  }
	}
	for(int i = 0; i < creatureCount; i++) {
	  delete creatures[i];
	}
	creatureCount = 0;
	/*
	  cerr << "After mission: " <<
	  " creatureCount=" << creatureCount << 
	  " itemCount=" << itemCount << endl;
	*/
	
	
	if(!changingStory) {
	  if(!inHq) {
		if(teleporting) {
		  // go back to HQ when coming from a mission	
		  nextMission = -1;
		} else {
		  break;
		}
	  } else if(nextMission == -1) {
		// if quiting in HQ, exit loop
		break;
	  }
	}
	
	
  }
}

void Scourge::endMission() {
	player->setSelXY(-1, -1);   // stop moving
	movingItem = NULL;          // stop moving items
	move = 0;
}

// items created for the mission
Item *Scourge::newItem(RpgItem *rpgItem) {
  items[itemCount++] = new Item(rpgItem);
  return items[itemCount - 1];
}

// creatures created for the mission
Creature *Scourge::newCreature(Character *character, char *name) {
  creatures[creatureCount++] = new Creature(this, character, name);
  return creatures[creatureCount - 1];
}

// creatures created for the mission
Creature *Scourge::newCreature(Monster *monster) {
  creatures[creatureCount++] = new Creature(this, monster);
  return creatures[creatureCount - 1];
}

void Scourge::drawView() {
  glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

  // make a move (player, monsters, etc.)
  playRound();
  
  // update current date variables and see if scheduled events have occured  
  if(calendar->update(userConfiguration->getGameSpeedLevel())){
    calendarButton->getLabel()->setTextCopy(calendar->getCurrentDate().getDateString());        
  }
  
  map->draw();

  glDisable( GL_DEPTH_TEST );
  glDisable( GL_TEXTURE_2D );
  
  if(isInfoShowing) {
    map->initMapView();  
    for(int i = 0; i < 4; i++) {
			if(!party[i]->getStateMod(Constants::dead)) {
				map->showCreatureInfo(party[i], (player == party[i]), 
															(map->getSelectedDropTarget() && 
															 map->getSelectedDropTarget()->creature == party[i]),
															!player_only);
			}
    }
	glDisable( GL_CULL_FACE );
  }

  map->drawDescriptions(messageList);

  miniMap->draw(0, 400);

  //  if(inventory->isVisible()) inventory->drawInventory();
  
  glEnable( GL_DEPTH_TEST );
  glEnable( GL_TEXTURE_2D );      
}

void Scourge::drawAfter() {
  map->drawDraggedItem();
}

bool Scourge::handleEvent(SDL_Event *event) {
  int ea;  

  if(changingStory) {
	endMission();
	return true;
  }

  if(containerGuiCount > 0) {
	for(int i = 0; i < containerGuiCount; i++) {
	  if(containerGui[i]->handleEvent(event)) {
		closeContainerGui(containerGui[i]);
	  }
	}
  }

  if(inventory->isVisible()) {
  	inventory->handleEvent(event);
	//	return false;
  }

  if(optionsMenu->isVisible()) {
	optionsMenu->handleEvent(event);
	return false;
  }


  switch(event->type) {
  case SDL_MOUSEMOTION:
    if(event->motion.x < 10) {
      map->setZRot(5.0f);
    } else if(event->motion.x >= sdlHandler->getScreen()->w - 10) {
      map->setZRot(-5.0f);
    } else {
      map->setZRot(0.0f);
    }
    if(event->motion.y < 10) {
      map->setYRot(5.0f);
    } else if(event->motion.y >= sdlHandler->getScreen()->h - 10) {
      map->setYRot(-5.0f);
    } else {
      map->setYRot(0.0f);
    }
    processGameMouseMove(event->motion.x, event->motion.y);
    break;
  case SDL_MOUSEBUTTONDOWN:
    if(event->button.button) {
	  processGameMouseDown(event->button.x, event->button.y, event->button.button);
    }
    break;	
  case SDL_MOUSEBUTTONUP:
    if(event->button.button) {
	  processGameMouseClick(event->button.x, event->button.y, event->button.button);
	  if(teleporting && !exitConfirmationDialog->isVisible()) {
		exitLabel->setText(Constants::getMessage(Constants::TELEPORT_TO_BASE_LABEL));
		if(startRound) toggleRound();

		exitConfirmationDialog->setVisible(true);
	  }
    }
    break;
  case SDL_KEYDOWN:
  case SDL_KEYUP:
  
    if(event->type == SDL_KEYUP && event->key.keysym.sym == SDLK_ESCAPE){
	  if(exitConfirmationDialog->isVisible()) {
		exitConfirmationDialog->setVisible(false);
	  } else {
		if(startRound) toggleRound();
		exitConfirmationDialog->setVisible(true);
	  }	  
	  return false;
	}
    
    // xxx_yyy_stop means : "do xxx_yyy action when the corresponding key is up"
    ea = userConfiguration->getEngineAction(event);    
    if(ea == SET_MOVE_DOWN){        
        setMove(Constants::MOVE_DOWN);
    }
    else if(ea == SET_MOVE_UP){
        setMove(Constants::MOVE_UP);
    }
    else if(ea == SET_MOVE_RIGHT){
        setMove(Constants::MOVE_RIGHT);
    }
    else if(ea == SET_MOVE_LEFT){
        setMove(Constants::MOVE_LEFT);
    }
    else if(ea == SET_MOVE_DOWN_STOP){        
        removeMove(Constants::MOVE_DOWN);
    }
    else if(ea == SET_MOVE_UP_STOP){
        removeMove(Constants::MOVE_UP);
    }
    else if(ea == SET_MOVE_RIGHT_STOP){
        removeMove(Constants::MOVE_RIGHT);
    }
    else if(ea == SET_MOVE_LEFT_STOP){
        removeMove(Constants::MOVE_LEFT);
    }            
    else if(ea == SET_PLAYER_0){
	  setPlayer(0);
    }
    else if(ea == SET_PLAYER_1){
	  setPlayer(1);
    }
    else if(ea == SET_PLAYER_2){
	  setPlayer(2);
    }
    else if(ea == SET_PLAYER_3){
	  setPlayer(3);
    }
    else if(ea == SET_PLAYER_ONLY){
	  togglePlayerOnly();
    }    
    else if(ea == BLEND_A){
        blendA++; if(blendA >= 11) blendA = 0;
        fprintf(stderr, "blend: a=%d b=%d\n", blendA, blendB);
    }
    else if(ea == BLEND_B){    
        blendB++; if(blendB >= 11) blendB = 0;
        fprintf(stderr, "blend: a=%d b=%d\n", blendA, blendB);
    }
    else if(ea == SHOW_INVENTORY){
	  //	  if(startRound) toggleRound();
	  inventory->show(); 	  
    }
    else if(ea == SHOW_OPTIONS_MENU){
	  if(startRound) toggleRound();
	  optionsMenu->show();
    }
    else if(ea == USE_ITEM_STOP){
        useItem();        
    }
    else if(ea == SET_NEXT_FORMATION_STOP){
        if(getFormation() < Creature::FORMATION_COUNT - 1) setFormation(getFormation() + 1);
	else setFormation(Constants::DIAMOND_FORMATION - Constants::DIAMOND_FORMATION);
    }   
    else if(ea == SET_X_ROT_PLUS){        
        map->setXRot(5.0f);
    }
    else if(ea == SET_X_ROT_MINUS){
        map->setXRot(-5.0f);
    }
    else if(ea == SET_Y_ROT_PLUS){
        map->setYRot(5.0f);
    }
    else if(ea == SET_Y_ROT_MINUS){
        map->setYRot(-5.0f);
    }
    else if(ea == SET_Z_ROT_PLUS){
        map->setZRot(5.0f);
    }
    else if(ea == SET_Z_ROT_MINUS){
        map->setZRot(-5.0f);
    }    
    else if(ea == SET_X_ROT_PLUS_STOP){
        map->setXRot(0.0f);
    }
    else if(ea == SET_X_ROT_MINUS_STOP){
        map->setXRot(0.0f);
    }
    else if(ea == SET_Y_ROT_PLUS_STOP){
        map->setYRot(0.0f);
    }
    else if(ea == SET_Y_ROT_MINUS_STOP){
        map->setYRot(0.0f);
    }
    else if(ea == SET_Z_ROT_PLUS_STOP){
        map->setZRot(0.0f);
    }
    else if(ea == SET_Z_ROT_MINUS_STOP){
        map->setZRot(0.0f);
    }    
    else if(ea == ADD_X_POS_PLUS){
        map->addXPos(10.0f);
    }
    else if(ea == ADD_X_POS_MINUS){
        map->addXPos(-10.0f);
    }
    else if(ea == ADD_Y_POS_PLUS){
        map->addYPos(10.0f);
    }
    else if(ea == ADD_Y_POS_MINUS){
        map->addYPos(-10.0f);
    }
    else if(ea == ADD_Z_POS_PLUS){
        map->addZPos(10.0f);
    }
    else if(ea == ADD_Z_POS_MINUS){
        map->addZPos(-10.0f);
    }     
    else if(ea == MINIMAP_ZOOM_IN){
        miniMap->zoomIn();
    }
    else if(ea == MINIMAP_ZOOM_OUT){
        miniMap->zoomOut();
    }
    else if(ea == MINIMAP_TOGGLE){
        miniMap->toggle();
    }
    else if(ea == SET_ZOOM_IN){
        map->setZoomIn(true);
    }
    else if(ea == SET_ZOOM_OUT){
        map->setZoomOut(true);
    }
    else if(ea == SET_ZOOM_IN_STOP){
        map->setZoomIn(false);
    }
    else if(ea == SET_ZOOM_OUT_STOP){
        map->setZoomOut(false);
    }
    else if(ea == TOGGLE_MAP_CENTER){
        bool mc;
        mc = userConfiguration->getAlwaysCenterMap();
        userConfiguration->setAlwaysCenterMap(!mc);
    }
    else if(ea == INCREASE_GAME_SPEED){
        addGameSpeed(-1);        
    }
    else if(ea == DECREASE_GAME_SPEED){
        addGameSpeed(1);        
    }    
	else if(ea == START_ROUND) {
	  toggleRound();
	}
    break;
  default: break;
  }

  return false;  
}

void Scourge::processGameMouseMove(Uint16 x, Uint16 y) {
  Uint16 mapx, mapy;
  getMapXYAtScreenXY(x, y, &mapx, &mapy);
  if(mapx < MAP_WIDTH) {

	// find the drop target
	Location *dropTarget = NULL;
	if(movingItem) {
	  Uint16 mapx, mapy, mapz;
	  getMapXYZAtScreenXY(x, y, &mapx, &mapy, &mapz);
	  if(mapx < MAP_WIDTH) {
		dropTarget = map->getLocation(mapx, mapy, mapz);
		if(!(dropTarget && 
			 (dropTarget->creature || 
			  (dropTarget->item && dropTarget->item->getRpgItem()->getType() == RpgItem::CONTAINER)) )) 
		  dropTarget = NULL;
	  }
	}
	map->setSelectedDropTarget(dropTarget);

	map->handleMouseMove(mapx, mapy, 0);
  }
}

void Scourge::processGameMouseDown(Uint16 x, Uint16 y, Uint8 button) {
  Uint16 mapx, mapy, mapz;
  if(button == SDL_BUTTON_LEFT) {
	// click on an item
	getMapXYZAtScreenXY(x, y, &mapx, &mapy, &mapz);
	if(mapx > MAP_WIDTH) {
	  getMapXYAtScreenXY(x, y, &mapx, &mapy);
	  mapz = 0;
	}
	if(startItemDrag(mapx, mapy, mapz)) return;
  }
}

void Scourge::processGameMouseClick(Uint16 x, Uint16 y, Uint8 button) {
	Uint16 mapx, mapy, mapz;
	if(button == SDL_BUTTON_LEFT) {
		getMapXYZAtScreenXY(x, y, &mapx, &mapy, &mapz);
		
		// clicking on a creature
		if(!movingItem && mapx < MAP_WIDTH) {
			Location *loc = map->getLocation(mapx, mapy, mapz);
			if(loc && loc->creature) {
				if(loc->creature->isMonster()) {
					// follow this creature
					player->setTargetCreature(loc->creature);
					if(!player_only) {
						for(int i = 0; i < 4; i++)
							party[i]->setTargetCreature(loc->creature);
					}
					return;
				} else {
					// select player
					for(int i = 0; i < 4; i++) {
						if(party[i] == loc->creature) {
							setPlayer(i);
							return;
						}
					}
				}
			}
		}
		
		// click on an item
		if(mapx > MAP_WIDTH) {
			getMapXYAtScreenXY(x, y, &mapx, &mapy);
			mapz = 0;
		}
		if(useItem(mapx, mapy, mapz))	return;
		
		// click on the map
		getMapXYAtScreenXY(x, y, &mapx, &mapy);
		player->setSelXY(mapx, mapy);
		if(player_only) {
			player->setTargetCreature(NULL);
		} else {
			for(int i = 0; i < 4; i++) {
				if(!party[i]->getStateMod(Constants::dead)) {
					party[i]->setTargetCreature(NULL);
					if(party[i] != player) party[i]->follow(map);
				}
			}
		}
		
		
	} else if(button == SDL_BUTTON_RIGHT) {
		getMapXYZAtScreenXY(x, y, &mapx, &mapy, &mapz);
		if(mapx < MAP_WIDTH) {    
			map->handleMouseClick(mapx, mapy, mapz, button);
		}
	}
}

void Scourge::getMapXYAtScreenXY(Uint16 x, Uint16 y,
                                 Uint16 *mapx, Uint16 *mapy) {
    glPushMatrix();

    // Initialize the scene w/o y rotation.
    //map->initMapView(false, true);
    map->initMapView(true);

    double obj_x, obj_y, obj_z;
    double win_x = (double)x;
    double win_y = (double)sdlHandler->getScreen()->h - y - 1;
    double win_z = 0.0;

    double projection[16];
    double modelview[16];
    GLint viewport[4];

    glGetDoublev(GL_PROJECTION_MATRIX, projection);
    glGetDoublev(GL_MODELVIEW_MATRIX, modelview);
    glGetIntegerv(GL_VIEWPORT, viewport);

    int res = gluUnProject(win_x, win_y, win_z,
                           modelview,
                           projection,
                           viewport,
                           &obj_x, &obj_y, &obj_z);
   
    if(res) {
        /*char buff[255];
        sprintf(buff, "avant: x %2.2f, y %2.2f, z %2.2f, getX %d, getY %d", obj_x, obj_y, obj_z, map->getX(), map->getY());
        map->addDescription(buff, 1.0f, 1.0f, 0.5f);  
        sprintf(buff, "avant op : yrot = %2.2f, sin(yrot) = %2.2f, obj_x * sin(yrot) = %2.2f", map->getYRot(), sin(map->getYRot()), obj_x * sin(map->getYRot()));
        map->addDescription(buff, 1.0f, 1.0f, 0.5f);
        float radians;
        radians = map->getYRot() * 3.1415926 / 180.0f;
        obj_x += (obj_x * sin(radians));
        obj_y += (obj_y * sin(radians));
        sprintf(buff, "avant 2: x %2.2f, y %2.2f, z %2.2f", obj_x, obj_y, obj_z);
        map->addDescription(buff, 1.0f, 1.0f, 0.5f);*/
        *mapx = map->getX() + (Uint16)(((obj_x) * GLShape::DIV)) - 1;
        *mapy = map->getY() + (Uint16)(((obj_y) * GLShape::DIV)) + 2;
        //*mapz = (Uint16)0;
        //*mapz = (Uint16)(obj_z * GLShape::DIV);
		map->debugX = *mapx;
		map->debugY = *mapy;
		map->debugZ = 0;
		//map->debgTextMouse= true;
		//map->debugZ = obj_z;
    } else {
        //*mapx = *mapy = *mapz = MAP_WIDTH + 1;
        *mapx = *mapy = MAP_WIDTH + 1;
    }
    glPopMatrix();
}

void Scourge::getMapXYZAtScreenXY(Uint16 x, Uint16 y,
                                  Uint16 *mapx, Uint16 *mapy, Uint16 *mapz) {

  // only do this if the mouse has moved some (optimization)
  if(abs(lastX - x) < POSITION_SAMPLE_DELTA && abs(lastY - y) < POSITION_SAMPLE_DELTA) {
	*mapx = lastMapX;
	*mapy = lastMapY;
	*mapz = lastMapZ;
	return;
  }

  GLuint buffer[512];
  GLint  hits, viewport[4];

  glGetIntegerv(GL_VIEWPORT, viewport);
  glSelectBuffer(512, buffer);
  glRenderMode(GL_SELECT);
  glInitNames();
  glPushName(0);

  glMatrixMode(GL_PROJECTION);
  glPushMatrix();
  glLoadIdentity();
  gluPickMatrix(x, viewport[3]-y, 1, 1, viewport);
  sdlHandler->setOrthoView();

  glMatrixMode(GL_MODELVIEW);
  map->selectMode = true;
  map->draw();
  map->selectMode = false;

  glFlush();    
  hits = glRenderMode(GL_RENDER);
  //cerr << "hits=" << hits << endl;
  if (hits > 0)	{					  // If There Were More Than 0 Hits
	int choose = buffer[4];					// Make Our Selection The First Object
	int depth = buffer[1];					// Store How Far Away It Is

	for (int loop = 0; loop < hits; loop++)	{		// Loop Through All The Detected Hits

	  //            fprintf(stderr, "\tloop=%d 0=%u 1=%u 2=%u 3=%u 4=%u \n", loop, 
	  //                    buffer[loop*5+0], buffer[loop*5+1], buffer[loop*5+2], 
	  //                    buffer[loop*5+3],  buffer[loop*5+4]);
	  if (buffer[loop*5+4] > 0) {
		decodeName(buffer[loop*5+4], mapx, mapy, mapz);
	  }

	  // If This Object Is Closer To Us Than The One We Have Selected
	  if (buffer[loop*5+1] < GLuint(depth))	{
		choose = buffer[loop*5+4];			  // Select The Closer Object
		depth = buffer[loop*5+1];			// Store How Far Away It Is
	  }
	}

	//cerr << "choose=" << choose << endl;
	decodeName(choose, mapx, mapy, mapz);
  } else {
	*mapx = *mapy = MAP_WIDTH + 1;
  }

  // Restore the projection matrix
  glMatrixMode(GL_PROJECTION);
  glPopMatrix();

  // Go back to modelview for normal rendering
  glMatrixMode(GL_MODELVIEW);

  lastMapX = *mapx;
  lastMapY = *mapy;
  lastMapZ = *mapz;
  lastX = x;
  lastY = y;
}

void Scourge::decodeName(int name, Uint16* mapx, Uint16* mapy, Uint16* mapz) {
    char *s;
    if(name > 0) {
        // decode the encoded map coordinates
        *mapz = name / (MAP_WIDTH * MAP_DEPTH);
        if(*mapz > 0)
            name %= (MAP_WIDTH * MAP_DEPTH);
        *mapx = name % MAP_WIDTH;
        *mapy = name / MAP_WIDTH;
        Location *pos = map->getPosition(*mapx, *mapy, 0);
        if(pos) {
            if(pos->shape) s = pos->shape->getName();
            else if(pos->item && pos->item->getShape()) {
                s = pos->item->getShape()->getName();
            }
        } else s = NULL;
		//        fprintf(stderr, "\tmap coordinates: pos null=%s shape null=%s item null=%s %u,%u,%u name=%s\n", 
		//                (pos ? "no" : "yes"), (pos && pos->shape ? "no" : "yes"), (pos && pos->item ? "no" : "yes"), *mapx, *mapy, *mapz, (s ? s : "NULL"));
    } else {
        *mapx = MAP_WIDTH + 1;
        *mapy = 0;
        *mapz = 0;
		//        fprintf(stderr, "\t---\n");
    }
}

void Scourge::setPartyMotion(int motion) {
  for(int i = 0; i < 4; i++) {
	if(party[i] != player) party[i]->setMotion(motion);
  }
}

void Scourge::startItemDragFromGui(Item *item) {
  movingX = -1;
  movingY = -1;
  movingZ = -1;
  movingItem = item;
}

bool Scourge::startItemDrag(int x, int y, int z) {
  if(movingItem) return false;
  Location *pos = map->getPosition(x, y, z);
  if(pos) {
	if(getItem(pos)) {  
	  dragStartTime = SDL_GetTicks();
	  return true;
	}
  }
  return false;
}

void Scourge::endItemDrag() {
  // item move is over
  movingItem = NULL;
  movingX = movingY = movingZ = MAP_WIDTH + 1;  
}

bool Scourge::useItem() {
  for(int x = player->getX() - 2; 
	  x < player->getX() + player->getShape()->getWidth() + 2; 
	  x++) {
	for(int y = player->getY() + 2; 
		y > player->getY() - player->getShape()->getDepth() - 2; 
		y--) {
	  if(useItem(x, y, 0)) return true;
	}
  }
  return false;
}

bool Scourge::useItem(int x, int y, int z) {
  if(movingItem) {
		// a quick click opens a container
		GLint delta = SDL_GetTicks() - dragStartTime;
		if(delta < ACTION_CLICK_TIME &&
			 movingItem->getRpgItem()->getType() == RpgItem::CONTAINER) {
			openContainerGui(movingItem);
		}
		dropItem(map->getSelX(), map->getSelY());
		return true;
	}
	
	Location *pos = map->getPosition(x, y, z);
	if(pos) {
		if(useDoor(pos)) {
			map->updateLightMap();
			return true;
		} else if(useGate(pos)) {
			return true;
		} else if(useBoard(pos)) {
			return true;
		} else if(useTeleporter(pos)) {
		  return true;
		}
	}
	return false;
}

bool Scourge::getItem(Location *pos) {
    if(pos->item) {
	  if(map->isWallBetween(pos->x, pos->y, pos->z, 
							getPlayer()->getX(),
							getPlayer()->getY(),
							0)) {
		map->addDescription(Constants::getMessage(Constants::ITEM_OUT_OF_REACH));
	  } else {
        movingX = pos->x;
        movingY = pos->y;
        movingZ = pos->z;
        movingItem = pos->item;		
		int x = pos->x;
		int y = pos->y;
		int z = pos->z;
        map->removeItem(pos->x, pos->y, pos->z);
		map->dropItemsAbove(x, y, z, movingItem);
		// draw the item as 'selected'
		map->setSelectedDropTarget(NULL);
		map->handleMouseMove(movingX, movingY, movingZ);
	  }
	  return true;
    }
    return false;
}

// drop an item from the inventory
void Scourge::setMovingItem(Item *item, int x, int y, int z) {
  movingX = x;
  movingY = y;
  movingZ = z;
  movingItem = item;
}

void Scourge::dropItem(int x, int y) {
  int z;
  bool replace = false;
  if(map->getSelectedDropTarget()) {
	char message[120];
	Creature *c = map->getSelectedDropTarget()->creature;
	if(c) {
	  c->addInventory(movingItem);
	  sprintf(message, "%s picks up %s.", 
			  c->getName(), 
			  movingItem->getRpgItem()->getName());
	  map->addDescription(message);
	  // if the inventory is open, update it
	  if(inventory->isVisible()) inventory->refresh();
	} else if(map->getSelectedDropTarget()->item && 
			  map->getSelectedDropTarget()->item->getRpgItem()->getType() == RpgItem::CONTAINER) {
	  map->getSelectedDropTarget()->item->addContainedItem(movingItem);
	  sprintf(message, "%s is placed in %s.", 
			  movingItem->getRpgItem()->getName(), 
			  map->getSelectedDropTarget()->item->getRpgItem()->getName());
	  map->addDescription(message);
	  // if this container's gui is open, update it
	  refreshContainerGui(map->getSelectedDropTarget()->item);
	} else {
	  replace = true;
	}
  } else {
	// see if it's blocked and get the value of z (stacking items)
	Location *pos = map->isBlocked(x, y, 0,
								   movingX, movingY, movingZ,
								   movingItem->getShape(), &z);
	if(!pos && 
	   !map->isWallBetween(getPlayer()->getX(), 
						   getPlayer()->getY(), 
						   getPlayer()->getZ(), 
						   x, y, z)) {
	  map->setItem(x, y, z, movingItem);
	} else {
	  replace = true;
	}
  }

  // failed to drop item; put it back to where we got it from
  if(replace) {
	if(movingX <= -1 || movingX >= MAP_WIDTH) {
	  // the item drag originated from the gui... what to do?
	  // for now don't end the drag
	  return;
	} else {
	  map->isBlocked(movingX, movingY, movingZ,
					 -1, -1, -1,
					 movingItem->getShape(), &z);
	  map->setItem(movingX, movingY, z, movingItem);
	}
  }
  endItemDrag();
}

bool Scourge::useGate(Location *pos) {
  for(int i = 0; i < 4; i++) {
	if(!party[i]->getStateMod(Constants::dead)) {
	  if(pos->shape == shapePal->getShape(Constants::STAIRS_UP_INDEX)) {
		currentStory--;
		changingStory = true;
		return true;
	  } else if(pos->shape == shapePal->getShape(Constants::STAIRS_DOWN_INDEX)) {
		currentStory++;
		changingStory = true;
		return true;
	  }
	}
  }
  return false;
}

bool Scourge::useBoard(Location *pos) {
	if(pos->shape == shapePal->getShape(Constants::BOARD_INDEX)) {
		for(int i = 0; i < board->getMissionCount(); i++) {
			strcpy(missionText[i], board->getMission(i)->name);
		}
		missionList->setLines(board->getMissionCount(), (const char**)missionText);
		if(board->getMissionCount())
			missionDescriptionLabel->setText((char*)(board->getMission(0)->story));
		boardWin->setVisible(true);
		return true;
	}
	return false;
}

bool Scourge::useTeleporter(Location *pos) {
  if(pos->shape == shapePal->getShape(Constants::TELEPORTER_INDEX) ||
	 pos->shape == shapePal->getShape(Constants::TELEPORTER_BASE_INDEX)) {
	// able to teleport if any party member is alive
	for(int i = 0; i < 4; i++) {
	  if(!party[i]->getStateMod(Constants::dead)) {
		teleporting = true;
		return true;
	  }
	}
  }
  return false;
}

bool Scourge::useDoor(Location *pos) {
    Shape *newDoorShape = NULL;
    if(pos->shape == shapePal->getShape(Constants::EW_DOOR_INDEX)) {
        newDoorShape = shapePal->getShape(Constants::NS_DOOR_INDEX);
    } else if(pos->shape == shapePal->getShape(Constants::NS_DOOR_INDEX)) {
        newDoorShape = shapePal->getShape(Constants::EW_DOOR_INDEX);
    }
    if(newDoorShape) {
        // switch door
        Sint16 ox = pos->x;
        Sint16 oy = pos->y;
        Sint16 nx = pos->x;
        Sint16 ny = (pos->y - pos->shape->getDepth()) + newDoorShape->getDepth();
        Shape *oldDoorShape = map->removePosition(ox, oy, player->getZ());
        if(!map->isBlocked(nx, ny, player->getZ(),
                           ox, oy, player->getZ(),
                           newDoorShape)) {
            map->setPosition(nx, ny, player->getZ(), newDoorShape);
            return true;
        } else {
          // rollback
          map->setPosition(ox, oy, player->getZ(), oldDoorShape);
          map->addDescription(Constants::getMessage(Constants::DOOR_BLOCKED));
          return true;
        }
    }
    return false;
}

Creature *Scourge::isPartyMember(Location *pos) {
  for(int i = 0; i < 4; i++) {
	  if(pos->x == party[i]->getX() &&
       pos->y == party[i]->getY() &&
       pos->z == party[i]->getZ()) return party[i];
  }
	return NULL;
}

bool Scourge::handleEvent(Widget *widget, SDL_Event *event) {

  if(widget == Window::message_button && info_dialog_showing) {
	if(!startRound) toggleRound();
	info_dialog_showing = false;
  }

  if(containerGuiCount > 0) {
	for(int i = 0; i < containerGuiCount; i++) {
	  if(containerGui[i]->handleEvent(widget, event)) {
		closeContainerGui(containerGui[i]);
	  }
	}
	//	return false;
  }

  if(inventory->isVisible()) {
	inventory->handleEvent(widget, event);
	//	return false;
  }

  if(optionsMenu->isVisible()) {
	optionsMenu->handleEvent(widget, event);
	return false;
  }


  if(widget == inventoryButton) {
	if(inventory->isVisible()) inventory->hide();
	else {
	  //	  if(startRound) toggleRound();
	  inventory->show();
	}
  } else if(widget == optionsButton) {
	if(optionsMenu->isVisible()) optionsMenu->hide();
	else {
	  if(startRound) toggleRound();
	  optionsMenu->show();
	}
  } else if(widget == yesExitConfirm) {
	exitLabel->setText(Constants::getMessage(Constants::EXIT_MISSION_LABEL));
	exitConfirmationDialog->setVisible(false);
	endMission();
	return true;
  } else if(widget == noExitConfirm) {
	teleporting = false;
	exitLabel->setText(Constants::getMessage(Constants::EXIT_MISSION_LABEL));
	exitConfirmationDialog->setVisible(false);
	return false;
  } else if(widget == quitButton) {
	if(startRound) toggleRound();
	exitConfirmationDialog->setVisible(true);
  } else if(widget == diamondButton) {
		setFormation(Constants::DIAMOND_FORMATION - Constants::DIAMOND_FORMATION);
  } else if(widget == staggeredButton) {
		setFormation(Constants::STAGGERED_FORMATION - Constants::DIAMOND_FORMATION);
  } else if(widget == squareButton) {
		setFormation(Constants::SQUARE_FORMATION - Constants::DIAMOND_FORMATION);
  } else if(widget == rowButton) {
		setFormation(Constants::ROW_FORMATION - Constants::DIAMOND_FORMATION);
  } else if(widget == scoutButton) {
		setFormation(Constants::SCOUT_FORMATION - Constants::DIAMOND_FORMATION);
  } else if(widget == crossButton) {
		setFormation(Constants::CROSS_FORMATION - Constants::DIAMOND_FORMATION);
  } else if(widget == player1Button) {
		setPlayer(Constants::PLAYER_1 - Constants::PLAYER_1);
  } else if(widget == player2Button) {
		setPlayer(Constants::PLAYER_2 - Constants::PLAYER_1);
  } else if(widget == player3Button) {
		setPlayer(Constants::PLAYER_3 - Constants::PLAYER_1);
  } else if(widget == player4Button) {
		setPlayer(Constants::PLAYER_4 - Constants::PLAYER_1);
  } else if(widget == groupButton) {
		togglePlayerOnly();
  } else if(widget == roundButton) {
		toggleRound();
  } else if(widget == boardWin->closeButton) {
		boardWin->setVisible(false);
	} else if(widget == missionList) {
		int selected = missionList->getSelectedLine();
		if(selected != -1 && selected < board->getMissionCount()) {
			missionDescriptionLabel->setText((char*)(board->getMission(selected)->story));
		}
	} else if(widget == playMission) {
		int selected = missionList->getSelectedLine();
		if(selected != -1 && selected < board->getMissionCount()) {
			nextMission = selected;
			currentStory = 0;
			endMission();
			return true;
		}
	}
  return false;
}

// create the ui
void Scourge::createUI() {
  char version[100];
  sprintf(version, "S.C.O.U.R.G.E. version %7.2f", SCOURGE_VERSION);
  mainWin = new Window( getSDLHandler(),
						sdlHandler->getScreen()->w - GUI_WIDTH, 
						sdlHandler->getScreen()->h - GUI_HEIGHT, 
						GUI_WIDTH, GUI_HEIGHT, 
						strdup(version), 
						getShapePalette()->getGuiTexture(), false );
//  int gx = sdlHandler->getScreen()->w - GUI_WIDTH;
//  int gy = sdlHandler->getScreen()->h - GUI_HEIGHT;
  inventoryButton = new Button( 0, 0, 100, 25, strdup("Party Info") );
  mainWin->addWidget((Widget*)inventoryButton);
  optionsButton = new Button( 0, 25,  100, 50, strdup("Options") );
  mainWin->addWidget((Widget*)optionsButton);
  quitButton = new Button( 0, 50,  100, 75, strdup("Quit") );
  mainWin->addWidget((Widget*)quitButton);
  roundButton = new Button( 0, 75,  100, 100, strdup("Real-Time") );
  roundButton->setToggle(true);
  roundButton->setSelected(true);
  mainWin->addWidget((Widget*)roundButton);    
  
  calendarButton = new Button( 100, 20, GUI_WIDTH, GUI_HEIGHT - 25, strdup(calendar->getCurrentDate().getDateString()));      
  //calendarButton->setLabelPosition(Button::CENTER);
  mainWin->addWidget((Widget*)calendarButton);    
 

  diamondButton = new Button( 100, 0,  120, 20 );
  diamondButton->setToggle(true);
  mainWin->addWidget((Widget*)diamondButton);  
  staggeredButton = new Button( 120, 0,  140, 20 );
  staggeredButton->setToggle(true);
  mainWin->addWidget((Widget*)staggeredButton);
  squareButton = new Button( 140, 0,  160, 20 );
  squareButton->setToggle(true);
  mainWin->addWidget((Widget*)squareButton);
  rowButton = new Button( 160, 0,  180, 20 );
  rowButton->setToggle(true);
  mainWin->addWidget((Widget*)rowButton);
  scoutButton = new Button( 180, 0,  200, 20 );
  scoutButton->setToggle(true);
  mainWin->addWidget((Widget*)scoutButton);
  crossButton = new Button( 200, 0,  220, 20 );
  crossButton->setToggle(true);
  mainWin->addWidget((Widget*)crossButton);

  player1Button = new Button( 100, 20,  124, 40, strdup("1") );
  player1Button->setToggle(true);
  mainWin->addWidget((Widget*)player1Button);  
  player2Button = new Button( 124, 20,  148, 40, strdup("2") );
  player2Button->setToggle(true);
  mainWin->addWidget((Widget*)player2Button);
  player3Button = new Button( 148, 20,  172, 40, strdup("3") );
  player3Button->setToggle(true);
  mainWin->addWidget((Widget*)player3Button);
  player4Button = new Button( 172, 20,  196, 40, strdup("4") );
  player4Button->setToggle(true);
  mainWin->addWidget((Widget*)player4Button);
  groupButton = new Button( 196, 20,  220, 40, strdup("G") );
  groupButton->setToggle(true);
  groupButton->setSelected(true);
  mainWin->addWidget((Widget*)groupButton);

  messageWin = new Window( getSDLHandler(),
						   0, 0, 400, 120, 
						   strdup("Messages"), 
						   getShapePalette()->getGuiTexture(), false );
  messageWin->setBackground(0, 0, 0);
  messageList = new ScrollingList(0, 0, 400, 95);
  messageList->setSelectionColor( 0.15f, 0.15f, 0.3f );
  messageWin->addWidget(messageList);

  // FIXME: try to encapsulate this in a class...
  //  exitConfirmationDialog = NULL;
  int w = 250;
  int h = 120;
  exitConfirmationDialog = new Window(getSDLHandler(),
	(getSDLHandler()->getScreen()->w/2) - (w/2), 
	(getSDLHandler()->getScreen()->h/2) - (h/2), 
	 w, h,
	 strdup("Exit mission?"), 
	 getShapePalette()->getGuiTexture(), false);
  yesExitConfirm = new Button( 40, 50, 110, 80, strdup("Yes") );
  exitConfirmationDialog->addWidget((Widget*)yesExitConfirm);
  noExitConfirm = new Button( 140, 50, 210, 80, strdup("No") );
  exitConfirmationDialog->addWidget((Widget*)noExitConfirm);
  exitLabel = new Label(20, 20, Constants::getMessage(Constants::EXIT_MISSION_LABEL));
  exitConfirmationDialog->addWidget((Widget*)exitLabel);

  boardWin = new Window( getSDLHandler(),
						 (sdlHandler->getScreen()->w - BOARD_GUI_WIDTH) / 2, 
						 (sdlHandler->getScreen()->h - BOARD_GUI_HEIGHT) / 2, 
						 BOARD_GUI_WIDTH, BOARD_GUI_HEIGHT, 
						 strdup("Available Missions"), 
						 getShapePalette()->getGuiTexture() );
  missionList = new ScrollingList(5, 40, BOARD_GUI_WIDTH - 10, 150);
  boardWin->addWidget(missionList);
  missionDescriptionLabel = new Label(5, 210, strdup(""), 70);
  boardWin->addWidget(missionDescriptionLabel);
  playMission = new Button(5, 5, 105, 35, Constants::getMessage(Constants::PLAY_MISSION_LABEL));
  boardWin->addWidget(playMission);
  missionText = (char**)malloc(Board::MAX_AVAILABLE_MISSION_COUNT * sizeof(char*));
  for(int i = 0; i < Board::MAX_AVAILABLE_MISSION_COUNT; i++) {
	missionText[i] = (char*)malloc(120 * sizeof(char));
  }
}


void Scourge::playRound() {
  // change animation if needed
  for(int i = 0; i < 4; i++) {
	if(((MD2Shape*)(party[i]->getShape()))->getAttackEffect()) {
	  party[i]->getShape()->setCurrentAnimation((int)MD2_ATTACK);	  
	} else if(party[i]->anyMovesLeft())
	  party[i]->getShape()->setCurrentAnimation((int)MD2_RUN);
	else 
	  party[i]->getShape()->setCurrentAnimation((int)MD2_STAND);
  }

  // move the player's selX,selY in a direction as specified by keystroke
  handleKeyboardMovement();

  // hound your targets
  for(int i = 0; i < 4; i++) {
	if(!party[i]->getStateMod(Constants::dead) && 
	   party[i]->getTargetCreature()) {
	  party[i]->setSelXY(party[i]->getTargetCreature()->getX(),
						 party[i]->getTargetCreature()->getY());
	}
  }
  
  // round starts if:
  // -in group mode
  // -(or) the round was manually started
  GLint t = SDL_GetTicks();
  if(startRound && 
	 (lastTick == 0 || t - lastTick > userConfiguration->getGameSpeedTicks())) {
	lastTick = t;
		
	// move the party members
	movePlayers();
	
	// move visible monsters
	for(int i = 0; i < creatureCount; i++) {
	  if(!creatures[i]->getStateMod(Constants::dead) && 
		 map->isLocationVisible(creatures[i]->getX(), creatures[i]->getY())) {
		moveMonster(creatures[i]);
	  }
	}

	// set up for battle
	battleCount = 0;

	// attack targeted monster if close enough
	for(int i = 0; i < 4; i++) {
	  if(!party[i]->getStateMod(Constants::dead) && 
			 party[i]->getTargetCreature()) {								
		battle[battleCount].creature = party[i];
		battleCount++;
	  }
	}
	for(int i = 0; i < creatureCount; i++) {
	  if(!creatures[i]->getStateMod(Constants::dead) && 
		 map->isLocationVisible(creatures[i]->getX(), creatures[i]->getY()) &&
		 creatures[i]->getTargetCreature()) {
		battle[battleCount].creature = creatures[i];
		battleCount++;
	  }
	}

	// fight one round of the epic battle
	if(battleCount > 0) fightBattle();
  }
}

// REFACTOR. Add rand() for a monster to give up fighting, implement damage special effects
// implement damage, death (esp. character death), level up, containers (corpse w. items)
void Scourge::fightBattle() {
  int initiative = -10;
  char message[200];
  bool fightingStarted = false;
  // this is O(n^2) unfortunately... maybe we could use a linked list or something here
  while(battleCount > 0) {
	bool creatureFound = false;
	for(int i = 0; i < battleCount; i++) {
	  Creature *creature = battle[i].creature;

	  // if someone already killed this target, skip it
	  if(creature->getTargetCreature()->getStateMod(Constants::dead)) {
		creature->setTargetCreature(NULL);
		if(creature->isMonster()) {
		  creature->setMotion(Constants::MOTION_LOITER);
		}
	  } else {
	  
		float dist = Util::distance(creature->getX(), 
									creature->getY(), 
									creature->getShape()->getWidth(),
									creature->getShape()->getDepth(),
									creature->getTargetCreature()->getX(),
									creature->getTargetCreature()->getY(),
									creature->getTargetCreature()->getShape()->getWidth(),
									creature->getTargetCreature()->getShape()->getDepth());
		// get the best weapon given the distance from the target
		Item *item = creature->getBestWeapon(dist);
		// creature won't fight if too far from the action 
		// (!item) is a bare-hands attack
		
		GLint t = SDL_GetTicks();
		int itemSpeed = (item ? item->getRpgItem()->getSpeed() : Constants::HAND_WEAPON_SPEED);
		if(item || dist <= 1.0f) {
		  if((itemSpeed * (userConfiguration->getGameSpeedTicks() + 80)) < t - creature->getLastTurn()) {
			// not time for this creature's turn yet
			int creatureInitiative = creature->getInitiative(item);
			if(creatureInitiative > initiative) continue;
			creatureFound = true;
			
			// remember the last active turn
			creature->setLastTurn(t);
			
			if(!fightingStarted) {
			  map->addDescription("A round of battle begins...", 1, 1, 1);
			  fightingStarted = true;
			}
			
			if(item) {
			  sprintf(message, "%s attacks %s with %s! (I:%d,S:%d)", 
					  creature->getName(), 
					  creature->getTargetCreature()->getName(),
					  item->getRpgItem()->getName(),
					  creatureInitiative, itemSpeed);
			  map->addDescription(message);
			  ((MD2Shape*)(creature->getShape()))->setAttackEffect(true);
			} else if(dist <= 1.0f) {
			  sprintf(message, "%s attacks %s with bare hands! (I:%d,S:%d)", 
					  creature->getName(), 
					  creature->getTargetCreature()->getName(),
					  creatureInitiative, itemSpeed);
			  map->addDescription(message);
			  ((MD2Shape*)(creature->getShape()))->setAttackEffect(true);
			}
			
			// the target creature gets really upset...
			// this is also an optimization for fps
			if(creature->getTargetCreature()->isMonster() && 
			   !creature->getTargetCreature()->getTargetCreature()) {
			  creature->getTargetCreature()->setMotion(Constants::MOTION_MOVE_TOWARDS);
			  creature->getTargetCreature()->setTargetCreature(creature);
			}
			
			// take a swing
			int tohit = creature->getToHit(item);
			int ac = creature->getTargetCreature()->getArmor();
			if(tohit > ac) {
			  
			  // deal out the damage
			  int damage = creature->getDamage(item);
			  if(damage) {
				sprintf(message, "...and hits! (toHit=%d vs. AC=%d) for %d points of damage", 
						tohit, ac, damage);
				map->addDescription(message, 1.0f, 0.5f, 0.5f);
				
				// target creature death
				if(creature->getTargetCreature()->takeDamage(damage)) {				  
				  creature->getShape()->setCurrentAnimation((int)MD2_TAUNT);  
				  sprintf(message, "...%s is killed!", creature->getTargetCreature()->getName());
				  map->addDescription(message, 1.0f, 0.5f, 0.5f);
				  creatureDeath(creature->getTargetCreature());

				  // add exp. points and money
				  if(!creature->isMonster()) {
					for(int i = 0; i < 4; i++) {
					  bool b = party[i]->getStateMod(Constants::leveled);
					  if(!party[i]->getStateMod(Constants::dead)) {
						int n = party[i]->addExperience(creature->getTargetCreature());
						if(n > 0) {
						  sprintf(message, "%s gains %d experience points.", party[i]->getName(), n);
						  map->addDescription(message);
						  if(!b && party[i]->getStateMod(Constants::leveled)) {
							sprintf(message, "%s gains a level!", party[i]->getName());
							map->addDescription(message, 1.0f, 0.5f, 0.5f);
						  }
						}

						n = party[i]->addMoney(creature->getTargetCreature());
						if(n > 0) {
						  sprintf(message, "%s finds %d coins!", party[i]->getName(), n);
						  map->addDescription(message);
						}
					  }
					}
				  }
				}
			  } else {
				sprintf(message, "...and hits! (toHit=%d vs. AC=%d) but causes no damage", 
						tohit, ac);
				map->addDescription(message);
			  }
			  
			} else {
			  // missed
			  sprintf(message, "...and misses! (toHit=%d vs. AC=%d)", tohit, ac);
			  map->addDescription(message);
			}
		  }
		} else {
		  // out of range
		  creature->setSelXY(creature->getTargetCreature()->getX(),
							 creature->getTargetCreature()->getY(),
							 true);
		}
	  }
	  // remove this creature from the turn
	  for(int t = i; t < battleCount - 1; t++) {
		battle[t].creature = battle[t + 1].creature;
	  }
	  battleCount--;
	  i--;
	}
	if(!creatureFound) initiative++;
  }
}

void Scourge::creatureDeath(Creature *creature) {
	if(creature == player) {
		if(!switchToNextLivePartyMember()) {
			partyDead = true;
		}
	}
  // remove from the map; the object will be cleaned up at the end of the mission
  map->removeCreature(creature->getX(), creature->getY(), creature->getZ());
  // add a container object instead
  //creature->getShape()->setCurrentAnimation(MD2_DEATH1);
  Item *item = newItem(RpgItem::getItemByName("Corpse"));
  // add creature's inventory to container
  map->setItem(creature->getX(), creature->getY(), creature->getZ(), item);
  int n = creature->getInventoryCount();
  for(int i = 0; i < n; i++) {
	item->addContainedItem(creature->removeInventory(0));
  }
  creature->setStateMod(Constants::dead, true);
}

// move the player in a direction as specified by keystroke
void Scourge::handleKeyboardMovement() {
  if(!move) return;

  // when moving w. the keyboard don't attack creatures
  for(int i = 0; i < 4; i++) party[i]->setTargetCreature(NULL);

  // decode keyboard movement
  if(move & Constants::MOVE_UP) {
	if(getPlayer()->getSelX() == -1) 
	  getPlayer()->setSelXY(getPlayer()->getX(), getPlayer()->getY() - 1);
	else
	  getPlayer()->setSelXY(getPlayer()->getSelX(), getPlayer()->getSelY() - 1);
  }
  if(move & Constants::MOVE_DOWN) {
	if(getPlayer()->getSelX() == -1) 
	  getPlayer()->setSelXY(getPlayer()->getX(), getPlayer()->getY() + 1);
	else
	  getPlayer()->setSelXY(getPlayer()->getSelX(), getPlayer()->getSelY() + 1);
  }
  if(move & Constants::MOVE_LEFT) {
	if(getPlayer()->getSelX() == -1) 
	  getPlayer()->setSelXY(getPlayer()->getX() - 1, getPlayer()->getY());
	else
	  getPlayer()->setSelXY(getPlayer()->getSelX() - 1, getPlayer()->getSelY());
  }
  if(move & Constants::MOVE_RIGHT) {
	if(getPlayer()->getSelX() == -1) 
	  getPlayer()->setSelXY(getPlayer()->getX() + 1, getPlayer()->getY());
	else
	  getPlayer()->setSelXY(getPlayer()->getSelX() + 1, getPlayer()->getSelY());
  }

  if(!player_only) {
    for(int i = 0; i < 4; i++) {
	  if(!party[i]->getStateMod(Constants::dead)) {
		party[i]->setTargetCreature(NULL);
		if(party[i] != player) party[i]->follow(map);
	  }
    }
  }
}

void Scourge::movePlayers() {   
  if(player_only) {	
	
	// how many party members are still alive?
	int sum = 0;
	for(int i = 0; i < 4; i++) 
	  if(!party[i]->getStateMod(Constants::dead)) sum++;
	
	// in single-step mode:
	for(int i = 0; i < sum; i++) {
	  
	  // move the current player
	  if(!player->getStateMod(Constants::dead)) {
		player->moveToLocator(map, player_only);
		map->center(player->getX(), player->getY());
	  }
	  
	  switchToNextLivePartyMember();
	}	
  } else {
	// In group mode:
	
	// move the leader
	if(!player->getStateMod(Constants::dead)) {
	  player->moveToLocator(map, player_only);
	  map->center(player->getX(), player->getY());
	}
	
	// in keyboard mode, don't move the selection
	if(move) player->setSelXY(-1, -1);
	
	// others follow the player
	for(int t = 0; t < 4; t++) {
	  if(!party[t]->getStateMod(Constants::dead) && party[t] != player) {
		if(party[t]->getTargetCreature()) {
		  party[t]->moveToLocator(map, player_only);
		} else {
		  party[t]->moveToLocator(map, player_only);
		}
	  }
	}	
  }
}

bool Scourge::switchToNextLivePartyMember() {
	Creature *oldPlayer = player;
	// find the player's index
	int n = -1;
	for(int t = 0; t < 4; t++) {
		if(party[t] == player) {
			n = t;
			break;
		}
	}			
	// switch to next player
	n++; if(n >= 4) n = 0;
	for(int t = 0; t < 4; t++) {
		if(!party[n]->getStateMod(Constants::dead)) {
			setPlayer(n);
			break;
		}
		n++; if(n >= 4) n = 0;
	}
	return(oldPlayer != player);
}

void Scourge::setPlayer(int n) {
  player = party[n];
  player->setNextDontMove(NULL, 0);
  move = 0;
  //  player->setSelXY(-1, -1); // don't move
  // init the rest of the party
  int count = 1;
  for(int i = 0; i < 4; i++) {
	if(i != n) party[i]->setNextDontMove(player, count++);
  }
  map->refresh();
  map->center(player->getX(), player->getY());
  player1Button->setSelected(false);
  player2Button->setSelected(false);
  player3Button->setSelected(false);
  player4Button->setSelected(false);
  switch(n) {
  case 0 : player1Button->setSelected(true); break;
  case 1 : player2Button->setSelected(true); break;
  case 2 : player3Button->setSelected(true); break;
  case 3 : player4Button->setSelected(true); break;
  }
}

/**
   Setting the formation ends single-step mode (back to group mode).
 */
void Scourge::setFormation(int formation) {
  this->formation = formation;
  for(int i = 0; i < 4; i++) {
	party[i]->setFormation(formation);
  }
  player_only = false;
  groupButton->setSelected(!player_only);
  startRound = true;
  roundButton->setSelected(startRound);


  diamondButton->setSelected(false);
  staggeredButton->setSelected(false);
  squareButton->setSelected(false);
  rowButton->setSelected(false);
  scoutButton->setSelected(false);
  crossButton->setSelected(false);
  switch(formation + Constants::DIAMOND_FORMATION) {
  case Constants::DIAMOND_FORMATION:
    diamondButton->setSelected(true); break;
  case Constants::STAGGERED_FORMATION:
    staggeredButton->setSelected(true); break;
  case Constants::SQUARE_FORMATION:
    squareButton->setSelected(true); break;
  case Constants::ROW_FORMATION:
    rowButton->setSelected(true); break;
  case Constants::SCOUT_FORMATION:
    scoutButton->setSelected(true); break;
  case Constants::CROSS_FORMATION:
    crossButton->setSelected(true); break;
  }

}

void Scourge::addGameSpeed(int speedFactor){
    char msg[80];
    userConfiguration->setGameSpeedLevel(userConfiguration->getGameSpeedLevel() + speedFactor);
    sprintf(msg, "Speed set to %d\n", userConfiguration->getGameSpeedTicks());
    map->addDescription(msg);
}

/**
   When pausing the round, we enter single-step mode.
 */
void Scourge::toggleRound() {
  int i ;
  startRound = (startRound ? false : true);
  if(startRound){
	map->addDescription(Constants::getMessage(Constants::REAL_TIME_MODE), 0.5f, 0.5f, 1.0f);
  }
  else{
	map->addDescription(Constants::getMessage(Constants::TURN_MODE), 0.5f, 0.5f, 1.0f);    
  }
  
  // Freeze / unfreeze animations
  for(i = 0; i < 4; i++){
    party[i]->getShape()->setPauseAnimation(!startRound);
  }  
  for(i = 0; i < creatureCount; i++){
    creatures[i]->getShape()->setPauseAnimation(!startRound);
  } 
  
  // Freeze / unfreeze calendar
  calendar->setPause(!startRound); 
  
  roundButton->setSelected(startRound);       
}

void Scourge::togglePlayerOnly() {
  player_only = (player_only ? false : true);
  // in group mode everyone hunts the same creature
  if(!player_only) {
	for(int i = 0; i < 4; i++) {
	  if(party[i] != player) 
		party[i]->setTargetCreature(player->getTargetCreature());
	}
  }
  if(player_only)
	map->addDescription(Constants::getMessage(Constants::SINGLE_MODE), 0.5f, 0.5f, 1.0f);
  else
	map->addDescription(Constants::getMessage(Constants::GROUP_MODE), 0.5f, 0.5f, 1.0f);
  groupButton->setSelected(!player_only);
}
  
void Scourge::moveMonster(Creature *monster) {  
  // set running animation (currently move or attack)
  if(((MD2Shape*)(monster->getShape()))->getAttackEffect()) {
	monster->getShape()->setCurrentAnimation((int)MD2_ATTACK);
	// don't move when attacking
	return;
  } else {
	monster->getShape()->setCurrentAnimation((int)MD2_RUN);
  }

  if(monster->getMotion() == Constants::MOTION_LOITER) {
	// attack a player
	if((int)(20.0f * rand()/RAND_MAX) == 0) {
	  int n = (int)(4.0f * rand()/RAND_MAX);
	  float dist = Util::distance(monster->getX(), 
								  monster->getY(), 
								  monster->getShape()->getWidth(),
								  monster->getShape()->getDepth(),
								  party[n]->getX(),
								  party[n]->getY(),
								  party[n]->getShape()->getWidth(),
								  party[n]->getShape()->getDepth());
	  if(dist < 20.0) {
		monster->setMotion(Constants::MOTION_MOVE_TOWARDS);
		monster->setTargetCreature(party[n]);
	  }
	} else {
	  // random (non-attack) monster movement
	  for(int i = 0; i < 4; i++) {
		int n = (int)(10.0f * rand()/RAND_MAX);
		if(n == 0 || !monster->move(monster->getDir(), map)) {
		  int dir = (int)(4.0f * rand()/RAND_MAX);
		  switch(dir) {
		  case 0: monster->setDir(Constants::MOVE_UP); break;
		  case 1: monster->setDir(Constants::MOVE_DOWN); break;
		  case 2: monster->setDir(Constants::MOVE_LEFT); break;
		  case 3: monster->setDir(Constants::MOVE_RIGHT); break;
		  }
		} else {
		  break;
		}
	  }
	}
  } else if(monster->getTargetCreature()) {
	// monster gives up
	if((int)(20.0f * rand()/RAND_MAX) == 0) {
	  monster->setMotion(Constants::MOTION_LOITER);
	  monster->setTargetCreature(NULL);
	} else {
	  // creature won't fight if too far from the action 
	  float dist = Util::distance(monster->getX(), 
								  monster->getY(), 
								  monster->getShape()->getWidth(),
								  monster->getShape()->getDepth(),
								  monster->getTargetCreature()->getX(),
								  monster->getTargetCreature()->getY(),
								  monster->getTargetCreature()->getShape()->getWidth(),
								  monster->getTargetCreature()->getShape()->getDepth());
	  Item *item = monster->getBestWeapon(dist);
	  if(item || dist <= 1.0) {
		monster->stopMoving(); // fps optimization
	  } else {
		monster->moveToLocator(map, false);
	  }
	}
  }
}

void Scourge::openContainerGui(Item *container) {
  // is it already open?
  for(int i = 0; i < containerGuiCount; i++) {
	if(containerGui[i]->getContainer() == container) {
	  containerGui[i]->getWindow()->toTop();
	  return;
	}
  }
  // open new window
  if(containerGuiCount < MAX_CONTAINER_GUI) {
	//	if(startRound) toggleRound();
	containerGui[containerGuiCount++] = new ContainerGui(this, container, 
														 10 + containerGuiCount * 15, 
														 10 + containerGuiCount * 15);
  }
}

void Scourge::closeContainerGui(ContainerGui *gui) {
  if(containerGuiCount <= 0) return;
  for(int i = 0; i < containerGuiCount; i++) {
	if(containerGui[i] == gui) {
	  for(int t = i; t < containerGuiCount - 1; t++) {
		containerGui[t] = containerGui[t + 1];
	  }
	  containerGuiCount--;
	  delete gui;
	  return;
	}
  }
}

void Scourge::closeAllContainerGuis() {
  for(int i = 0; i < containerGuiCount; i++) {
	delete containerGui[i];
  }
  containerGuiCount = 0;
}

void Scourge::refreshContainerGui(Item *container) {
  for(int i = 0; i < containerGuiCount; i++) {
	if(containerGui[i]->getContainer() == container) {
	  containerGui[i]->refresh();
	}
  }
}

void Scourge::showMessageDialog(char *message) {
  if(startRound) toggleRound();
  Window::showMessageDialog(getSDLHandler(), 
							getSDLHandler()->getScreen()->w / 2 - 150,
							getSDLHandler()->getScreen()->h / 2 - 55,
							300, 110, Constants::messages[Constants::SCOURGE_DIALOG][0],
							getShapePalette()->getGuiTexture(),
							message);
}
