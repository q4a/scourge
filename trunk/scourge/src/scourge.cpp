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
  lastTick = lastProjectileTick = 0;
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

  // initialize the items
  Item::initItems(shapePal);
  // initialize the monsters (they use items)
  Monster::initMonsters();
  Character::initCharacters();

  // create the mission board
  board = new Board(this);

  // do this before the inventory and optionsdialog (so Z is less than of those)
  party = new Party(this);  
  createUI();

  //  move = 0;
  battleCount = 0;  
  inventory = NULL;
  containerGuiCount = 0;
  changingStory = false;  

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

	  party->reset();
	  party->getCalendar()->reset(true); // reset the time
	  board->reset();

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

Scourge::~Scourge(){
  delete mainMenu;
  delete optionsMenu;
  delete userConfiguration;
  delete board;
}

void Scourge::startMission() {
  
  while(true) {
	
	// add gui
	party->getWindow()->setVisible(true);
	messageWin->setVisible(true);
	
	// create the map
	map = new Map(this);
	miniMap = new MiniMap(this); 
	
	// ready the party
	party->startPartyOnMission();
	
	// position the players
	//	move = 0;
	battleCount = 0;
	containerGuiCount = 0;
	lastMapX = lastMapY = lastMapZ = lastX = lastY = -1;
	teleporting = false;
	changingStory = false;
	
	
	
	if(nextMission == -1) {
	  
	  currentMission = NULL;
	  missionWillAwardExpPoints = false;

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
	  currentMission = board->getMission(nextMission);
	  missionWillAwardExpPoints = (!currentMission->isCompleted());
	  cerr << "Starting mission: level="  << currentMission->getLevel() << 
		" stories=" << currentMission->getDungeonStoryCount() << 
		" current story=" << currentStory << endl;
	  dg = new DungeonGenerator(this, currentMission->getLevel(), 
								(currentStory < currentMission->getDungeonStoryCount() - 1), 
								(currentStory > 0),
								currentMission);
	  dg->toMap(map, getShapePalette());
	}
	
	// center map on the player
	map->center(party->getPlayer()->getX(), 
				party->getPlayer()->getY(),
				true);
	
	// Must be called after MiniMap has been built by dg->toMap() !!! 
	miniMap->computeDrawValues();
	
	// set to receive events here
	sdlHandler->setHandlers((SDLEventHandler *)this, (SDLScreenView *)this);
	
	// hack to unfreeze animations, etc.
	party->forceStopRound();

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
	party->getWindow()->setVisible(false);
	messageWin->setVisible(false);
	closeAllContainerGuis();
	if(inventory->isVisible()) inventory->hide();
	if(board->boardWin->isVisible()) board->boardWin->setVisible(false);
	
	// clean up after the mission
	delete map;
	delete miniMap;
	delete dg;
	
	// delete the items and creatures created for this mission
	// (except items in inventory) 
	for(int i = 0; i < itemCount; i++) {
	  bool inInventory = false;
	  for(int t = 0; t < party->getPartySize(); t++) {
		if(party->getParty(t)->isItemInInventory(items[i])) {
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
  party->getPlayer()->setSelXY(-1, -1);   // stop moving
  movingItem = NULL;          // stop moving items
  //	move = 0;  
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
  
  party->drawView();
    
  map->draw();

  glDisable( GL_DEPTH_TEST );
  glDisable( GL_TEXTURE_2D );
  
  if(isInfoShowing) {
    map->initMapView();  
    for(int i = 0; i < party->getPartySize(); i++) {
	  if(!party->getParty(i)->getStateMod(Constants::dead)) {
		map->showCreatureInfo(party->getParty(i), (party->getPlayer() == party->getParty(i)), 
							  (map->getSelectedDropTarget() && 
							   map->getSelectedDropTarget()->creature == party->getParty(i)),
							  !party->isPlayerOnly());
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
	  setMove(Constants::MOVE_LEFT);
	} else if(event->motion.x >= sdlHandler->getScreen()->w - 10) {
	  setMove(Constants::MOVE_RIGHT);
	} else {
	  removeMove(Constants::MOVE_LEFT | Constants::MOVE_RIGHT);
	  map->setYRot(0.0f);
	  map->setZRot(0.0f);
	}
	if(event->motion.y < 10) {
	  setMove(Constants::MOVE_UP);
	} else if(event->motion.y >= sdlHandler->getScreen()->h - 10) {
	  setMove(Constants::MOVE_DOWN);
	} else {
	  removeMove(Constants::MOVE_UP | Constants::MOVE_DOWN);
	  map->setYRot(0.0f);
	  map->setZRot(0.0f);
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
		party->toggleRound(true);
		exitConfirmationDialog->setVisible(true);
	  } else if(changingStory && !exitConfirmationDialog->isVisible()) {
		exitLabel->setText(Constants::getMessage(Constants::USE_GATE_LABEL));
		party->toggleRound(true);
		exitConfirmationDialog->setVisible(true);
	  }
    }
    break;
  case SDL_KEYDOWN:
  case SDL_KEYUP:

	if(event->key.keysym.sym == SDLK_m) {
	  missionCompleted();
	  return false;
	}

	/* this is here to test effects
	if(event->key.keysym.sym == SDLK_f) {
	  party->startEffect(Constants::EFFECT_FLAMES, (Constants::DAMAGE_DURATION * 4));
	  return false;
	}
	if(event->key.keysym.sym == SDLK_t) {
	  party->startEffect(Constants::EFFECT_TELEPORT, (Constants::DAMAGE_DURATION * 4));
	  return false;
	}
	if(event->key.keysym.sym == SDLK_g) {
	  party->startEffect(Constants::EFFECT_GLOW, (Constants::DAMAGE_DURATION * 4));
	  return false;
	}
	*/
  
    if(event->type == SDL_KEYUP && event->key.keysym.sym == SDLK_ESCAPE){
	  if(exitConfirmationDialog->isVisible()) {
		exitConfirmationDialog->setVisible(false);
	  } else {
		party->toggleRound(true);
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
	  map->setYRot(0.0f);
	  map->setYRot(0);
	  removeMove(Constants::MOVE_DOWN);
    }
    else if(ea == SET_MOVE_UP_STOP){
	  map->setYRot(0.0f);
	  map->setYRot(0);
	  removeMove(Constants::MOVE_UP);
    }
    else if(ea == SET_MOVE_RIGHT_STOP){
	  map->setYRot(0.0f);
	  map->setZRot(0);
	  removeMove(Constants::MOVE_RIGHT);
    }
    else if(ea == SET_MOVE_LEFT_STOP){
	  map->setYRot(0.0f);
	  map->setZRot(0);
	  removeMove(Constants::MOVE_LEFT);
    }            
    else if(ea == SET_PLAYER_0){
	  party->setPlayer(0);
    }
    else if(ea == SET_PLAYER_1){
	  party->setPlayer(1);
    }
    else if(ea == SET_PLAYER_2){
	  party->setPlayer(2);
    }
    else if(ea == SET_PLAYER_3){
	  party->setPlayer(3);
    }
    else if(ea == SET_PLAYER_ONLY){
	  party->togglePlayerOnly();
    }    
	//    else if(ea == BLEND_A){
    else if(event->type == SDL_KEYUP && event->key.keysym.sym == SDLK_6){
        blendA++; if(blendA >= 11) blendA = 0;
        fprintf(stderr, "blend: a=%d b=%d\n", blendA, blendB);
    }
	//    else if(ea == BLEND_B){    
    else if(event->type == SDL_KEYUP && event->key.keysym.sym == SDLK_7){    
        blendB++; if(blendB >= 11) blendB = 0;
        fprintf(stderr, "blend: a=%d b=%d\n", blendA, blendB);
    }
    else if(ea == SHOW_INVENTORY){
	  inventory->show(); 	  
    }
    else if(ea == SHOW_OPTIONS_MENU){
	  party->toggleRound(true);
	  optionsMenu->show();
    }
    else if(ea == USE_ITEM_STOP){
        useItem();        
    }
    else if(ea == SET_NEXT_FORMATION_STOP){
        if(party->getFormation() < Creature::FORMATION_COUNT - 1) party->setFormation(party->getFormation() + 1);
	else party->setFormation(Constants::DIAMOND_FORMATION - Constants::DIAMOND_FORMATION);
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
	  party->toggleRound();
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
			  party->setTargetCreature(loc->creature);
			  return;
			} else {
			  // select player
			  for(int i = 0; i < party->getPartySize(); i++) {
				if(party->getParty(i) == loc->creature) {
				  party->setPlayer(i);
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
		if(useItem(mapx, mapy, mapz)) return;
		
		// click on the map
		getMapXYAtScreenXY(x, y, &mapx, &mapy);


		// FIXME: try to move to party.cpp
		party->getPlayer()->setSelXY(mapx, mapy);
		if(party->isPlayerOnly()) {
		  party->getPlayer()->setTargetCreature(NULL);
		} else {
		  for(int i = 0; i < party->getPartySize(); i++) {
			if(!party->getParty(i)->getStateMod(Constants::dead)) {
			  party->getParty(i)->setTargetCreature(NULL);
			  if(party->getParty(i) != party->getPlayer()) party->getParty(i)->follow(map);
			}
		  }
		}
		// end of FIXME
		
		
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
  for(int x = party->getPlayer()->getX() - 2; 
	  x < party->getPlayer()->getX() + party->getPlayer()->getShape()->getWidth() + 2; 
	  x++) {
	for(int y = party->getPlayer()->getY() + 2; 
		y > party->getPlayer()->getY() - party->getPlayer()->getShape()->getDepth() - 2; 
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
	  if(map->isWallBetween(party->getPlayer()->getX(), 
							party->getPlayer()->getY(), 
							party->getPlayer()->getZ(), 
							x, y, z)) {
		map->addDescription(Constants::getMessage(Constants::ITEM_OUT_OF_REACH));
		return true;
	  } else {
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
	}
	return false;
}

bool Scourge::getItem(Location *pos) {
    if(pos->item) {
	  if(map->isWallBetween(pos->x, pos->y, pos->z, 
							party->getPlayer()->getX(),
							party->getPlayer()->getY(),
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
	   !map->isWallBetween(party->getPlayer()->getX(), 
						   party->getPlayer()->getY(), 
						   party->getPlayer()->getZ(), 
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
	if(!party->getParty(i)->getStateMod(Constants::dead)) {
	  if(pos->shape == shapePal->findShapeByName("GATE_UP")) {
		oldStory = currentStory;
		currentStory--;
		changingStory = true;
		return true;
	  } else if(pos->shape == shapePal->findShapeByName("GATE_DOWN")) {
		oldStory = currentStory;
		currentStory++;
		changingStory = true;
		return true;
	  }
	}
  }
  return false;
}

bool Scourge::useBoard(Location *pos) {
  if(pos->shape == shapePal->findShapeByName("BOARD")) {
	board->boardWin->setVisible(true);
	return true;
  }
  return false;
}

bool Scourge::useTeleporter(Location *pos) {
  if(pos->shape == shapePal->findShapeByName("TELEPORTER") ||
	 pos->shape == shapePal->findShapeByName("TELEPORTER_BASE")) {
	// able to teleport if any party member is alive
	for(int i = 0; i < 4; i++) {
	  if(!party->getParty(i)->getStateMod(Constants::dead)) {
		teleporting = true;
		return true;
	  }
	}
  }
  return false;
}

bool Scourge::useDoor(Location *pos) {
    Shape *newDoorShape = NULL;
    if(pos->shape == shapePal->findShapeByName("EW_DOOR")) {
        newDoorShape = shapePal->findShapeByName("NS_DOOR");
    } else if(pos->shape == shapePal->findShapeByName("NS_DOOR")) {
        newDoorShape = shapePal->findShapeByName("EW_DOOR");
    }
    if(newDoorShape) {
        // switch door
        Sint16 ox = pos->x;
        Sint16 oy = pos->y;
        Sint16 nx = pos->x;
        Sint16 ny = (pos->y - pos->shape->getDepth()) + newDoorShape->getDepth();
        Shape *oldDoorShape = map->removePosition(ox, oy, party->getPlayer()->getZ());
        if(!map->isBlocked(nx, ny, party->getPlayer()->getZ(),
                           ox, oy, party->getPlayer()->getZ(),
                           newDoorShape)) {
            map->setPosition(nx, ny, party->getPlayer()->getZ(), newDoorShape);
            return true;
        } else {
          // rollback
          map->setPosition(ox, oy, party->getPlayer()->getZ(), oldDoorShape);
          map->addDescription(Constants::getMessage(Constants::DOOR_BLOCKED));
          return true;
        }
    }
    return false;
}

void Scourge::toggleInventoryWindow() {
  if(inventory->isVisible()) inventory->hide();
  else inventory->show();
}

void Scourge::toggleOptionsWindow() {
  if(optionsMenu->isVisible()) optionsMenu->hide();
  else {
	party->toggleRound(true);
	optionsMenu->show();
  }
}

void Scourge::showExitConfirmationDialog() {
  party->toggleRound(true);
  exitConfirmationDialog->setVisible(true);
}

bool Scourge::handleEvent(Widget *widget, SDL_Event *event) {

  if(widget == Window::message_button && info_dialog_showing) {
	party->toggleRound(false);
	info_dialog_showing = false;
	party->startEffect(Constants::EFFECT_TELEPORT, (Constants::DAMAGE_DURATION * 4));
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

  // FIXME: this is hacky...
  if(party->handleEvent(widget, event)) return true;
  int n = board->handleEvent(widget, event);
  if(n == Board::EVENT_HANDLED) return false;
  else if(n == Board::EVENT_PLAY_MISSION) {
	int selected = board->getSelectedLine();
	if(selected != -1 && selected < board->getMissionCount()) {
	  nextMission = selected;
	  oldStory = currentStory = 0;
	  endMission();
	  return true;
	}
  }
  
  if(widget == yesExitConfirm) {
	exitLabel->setText(Constants::getMessage(Constants::EXIT_MISSION_LABEL));
	exitConfirmationDialog->setVisible(false);
	endMission();
	return true;
  } else if(widget == noExitConfirm) {
	teleporting = false;
	changingStory = false;
	currentStory = oldStory;
	exitLabel->setText(Constants::getMessage(Constants::EXIT_MISSION_LABEL));
	exitConfirmationDialog->setVisible(false);
	return false;
  }
  return false;
}

// create the ui
void Scourge::createUI() {
  messageWin = new Window( getSDLHandler(),
						   0, 0, 400, 120, 
						   strdup("Messages"), 
						   getShapePalette()->getGuiTexture(), false );
  messageWin->setBackground(0, 0, 0);
  messageList = new ScrollingList(0, 0, 400, 95);
  messageList->setSelectionColor( 0.15f, 0.15f, 0.3f );
  messageWin->addWidget(messageList);
  // this has to be after addWidget
  messageList->setBackground( 1, 0.75f, 0.45f );
  messageList->setSelectionColor( 0.25f, 0.25f, 0.25f );

  // FIXME: try to encapsulate this in a class...
  //  exitConfirmationDialog = NULL;
  int w = 250;
  int h = 120;
  exitConfirmationDialog = new Window(getSDLHandler(),
	(getSDLHandler()->getScreen()->w/2) - (w/2), 
	(getSDLHandler()->getScreen()->h/2) - (h/2), 
	 w, h,
	 strdup("Leave level?"), 
	 getShapePalette()->getGuiTexture(), false);
  yesExitConfirm = new Button( 40, 50, 110, 80, strdup("Yes") );
  exitConfirmationDialog->addWidget((Widget*)yesExitConfirm);
  noExitConfirm = new Button( 140, 50, 210, 80, strdup("No") );
  exitConfirmationDialog->addWidget((Widget*)noExitConfirm);
  exitLabel = new Label(20, 20, Constants::getMessage(Constants::EXIT_MISSION_LABEL));
  exitConfirmationDialog->addWidget((Widget*)exitLabel);
}

void Scourge::playRound() {
  // change animation if needed
  for(int i = 0; i < 4; i++) {
	if(((MD2Shape*)(party->getParty(i)->getShape()))->getAttackEffect()) {
	  party->getParty(i)->getShape()->setCurrentAnimation((int)MD2_ATTACK);	  
	  ((MD2Shape*)(party->getParty(i)->getShape()))->setAngle(party->getParty(i)->getTargetAngle());
	} else if(party->getParty(i)->anyMovesLeft())
	  party->getParty(i)->getShape()->setCurrentAnimation((int)MD2_RUN);
	else 
	  party->getParty(i)->getShape()->setCurrentAnimation((int)MD2_STAND);
  }

  // move the map
  if(move) map->move(move);

  // hound your targets
  party->followTargets();
  
  // round starts if:
  // -in group mode
  // -(or) the round was manually started
  GLint t = SDL_GetTicks();
  if(party->isRealTimeMode()) {
	if(lastProjectileTick == 0 || t - lastProjectileTick > userConfiguration->getGameSpeedTicks() / 10) {
	  lastProjectileTick = t;
	  
	  // move projectiles
	  Projectile::moveProjectiles();
	}

	if(lastTick == 0 || t - lastTick > userConfiguration->getGameSpeedTicks()) {
	  lastTick = t;

	  // move the party members
	  party->movePlayers();
	  
	  // move visible monsters
	  for(int i = 0; i < creatureCount; i++) {
		if(!creatures[i]->getStateMod(Constants::dead) && 
		   map->isLocationVisible(creatures[i]->getX(), creatures[i]->getY())) {
		  moveMonster(creatures[i]);
		}
	  }

	  // setup the current battle round
	  if(battleRound.size() == 0) {
		
		// set up for battle
		battleCount = 0;
		
		// attack targeted monster if close enough
		for(int i = 0; i < 4; i++) {
		  if(!party->getParty(i)->getStateMod(Constants::dead) && 
			 party->getParty(i)->getTargetCreature()) {								
			battle[battleCount++] = new Battle(this, party->getParty(i));
		  }
		}
		for(int i = 0; i < creatureCount; i++) {
		  if(!creatures[i]->getStateMod(Constants::dead) && 
			 map->isLocationVisible(creatures[i]->getX(), creatures[i]->getY()) &&
			 creatures[i]->getTargetCreature()) {
			battle[battleCount++] = new Battle(this, creatures[i]);
		  }
		}
		
		// fight one round of the epic battle
		if(battleCount > 0) {
		  Battle::setupBattles(this, battle, battleCount, &battleRound);
		  battleTurn = 0;
		}
	  }
	  
	  // fight a turn of the battle
	  if(battleRound.size() > 0) {
		if(battleTurn < battleRound.size()) {
		  Battle *battle = battleRound[battleTurn];
		  battle->fightTurn();
		  delete battle;
		  battleTurn++;
		} else {
		  battleRound.clear();
		}
	  }
	}
  }
}

void Scourge::creatureDeath(Creature *creature) {
  if(creature == party->getPlayer()) {
	party->switchToNextLivePartyMember();
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

void Scourge::addGameSpeed(int speedFactor){
  char msg[80];
  userConfiguration->setGameSpeedLevel(userConfiguration->getGameSpeedLevel() + speedFactor);
  sprintf(msg, "Speed set to %d\n", userConfiguration->getGameSpeedTicks());
  map->addDescription(msg);
}

void Scourge::moveMonster(Creature *monster) {  
  // set running animation (currently move or attack)
  if(((MD2Shape*)(monster->getShape()))->getAttackEffect()) {
	monster->getShape()->setCurrentAnimation((int)MD2_ATTACK);
	((MD2Shape*)(monster->getShape()))->setAngle(monster->getTargetAngle());
	// don't move when attacking
	return;
  } else {
	monster->getShape()->setCurrentAnimation((int)MD2_RUN);
  }

  if(monster->getMotion() == Constants::MOTION_LOITER) {
	// attack the closest player
	if((int)(20.0f * rand()/RAND_MAX) == 0) {
	  Creature *p = party->getClosestPlayer(monster->getX(), monster->getY(), 
											monster->getShape()->getWidth(),
											monster->getShape()->getDepth(),
											20);
	  if(p) {
		monster->setMotion(Constants::MOTION_MOVE_TOWARDS);
		monster->setTargetCreature(p);
		//monster->setDistanceRange(0, Constants::MIN_DISTANCE);
	  }
	} else {
	  // random (non-attack) monster movement
	  monster->setDistanceRange(0, Constants::MIN_DISTANCE);
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
	// monster gives up when low on hp or bored
	// FIXME: when low on hp, it should run away not loiter
	if(monster->getHp() < (int)((float)monster->getStartingHp() * 0.2f) ||
	   (int)(20.0f * rand()/RAND_MAX) == 0) {
	  monster->setMotion(Constants::MOTION_LOITER);
	  monster->setTargetCreature(NULL);
	} else {
	  monster->moveToLocator(map);
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
  party->toggleRound(true);
  Window::showMessageDialog(getSDLHandler(), 
							getSDLHandler()->getScreen()->w / 2 - 150,
							getSDLHandler()->getScreen()->h / 2 - 55,
							300, 110, Constants::messages[Constants::SCOURGE_DIALOG][0],
							getShapePalette()->getGuiTexture(),
							message);
}

Window *Scourge::createWoodWindow(int x, int y, int w, int h, char *title) {
  Window *win = new Window( getSDLHandler(), x, y, w, h, title, 
							getShapePalette()->getGuiWoodTexture(),
							true, Window::SIMPLE_WINDOW );
  win->setBackgroundTileHeight(96);
  win->setBorderColor( 0.5f, 0.2f, 0.1f );
  win->setColor( 0.8f, 0.8f, 0.7f, 1 );
  win->setBackground( 0.65, 0.30f, 0.20f, 0.15f );
  win->setSelectionColor(  0.25f, 0.35f, 0.6f );
  return win;
}

void Scourge::missionCompleted() {
  showMessageDialog("Congratulations, mission accomplished!");

  // Award exp. points for completing the mission
  if(currentMission && missionWillAwardExpPoints && 
	 currentMission->isCompleted()) {

	// only do this once
	missionWillAwardExpPoints = false;

	// how many points?
	int exp = (currentMission->getLevel() + 1) * 100;
	map->addDescription("For completing the mission", 0, 1, 1);
	char message[200];
	sprintf(message, "The party receives %d points.", exp);
	map->addDescription(message, 0, 1, 1);
	
	for(int i = 0; i < getParty()->getPartySize(); i++) {
	  bool b = getParty()->getParty(i)->getStateMod(Constants::leveled);
	  if(!getParty()->getParty(i)->getStateMod(Constants::dead)) {
		int n = getParty()->getParty(i)->addExperience(exp);
		if(n > 0) {
		  // sprintf(message, "%s gains %d experience points.", getParty()->getParty(i)->getName(), n);
		  // getMap()->addDescription(message);
		  if(!b && getParty()->getParty(i)->getStateMod(Constants::leveled)) {
			sprintf(message, "%s gains a level!", getParty()->getParty(i)->getName());
			getMap()->addDescription(message, 1.0f, 0.5f, 0.5f);
		  }
		}
	  }
	}
  }
}
