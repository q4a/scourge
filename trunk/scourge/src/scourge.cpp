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
  mainWin = NULL;
  movingX = movingY = movingZ = MAP_WIDTH + 1;
  movingItem = NULL;

  isInfoShowing = true;
  
  // Reads the user configuration from a file      
  userConfiguration = new UserConfiguration();
  userConfiguration->loadConfiguration();
  
  // Initialize the video mode
  sdlHandler = new SDLHandler();
  sdlHandler->setVideoMode(argc, argv);

  shapePal = sdlHandler->getShapePalette();

  // initialize the monsters
  Monster::initMonsters();

  // init the party; hard code for now
  Creature **pc = Creature::createHardCodedParty(this);
  party[0] = player = pc[0];
  party[1] = pc[1];
  party[2] = pc[2];
  party[3] = pc[3];

  player_only = false;
  inventory = new Inventory(this);

  createUI();
  
  // show the main menu
  mainMenu = new MainMenu(this);  
  optionsMenu = new OptionsMenu(this);

  while(true) {
	mainMenu->show();    
	sdlHandler->setHandlers((SDLEventHandler *)mainMenu, (SDLScreenView *)mainMenu);
	sdlHandler->mainLoop();
	mainMenu->hide();

    // evaluate results and start a missions
    fprintf(stderr, "value=%d\n", mainMenu->getValue());
    if(mainMenu->getValue() == NEW_GAME) {
      //charBuilder-
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
}

void Scourge::startMission() {
  // add gui
  mainWin->setVisible(true);

  // new item and creature references
  itemCount = creatureCount = 0;
  
  // create the map
  map = new Map(this);
  miniMap = new MiniMap(this); 
  
  // Initialize the map with a random dunegeon	
  dg = new DungeonGenerator(this, level);
  Sint16 startx, starty;
  dg->toMap(map, &startx, &starty, getShapePalette());

  // position the players
  player_only = false;
  setPlayer(getParty(0));
  getPlayer()->moveTo(startx, starty, 0);
  map->setCreature(startx, starty, 0, getPlayer()); 

  // init the rest of the party
  for(int i = 1; i < 4; i++) {
	getParty(i)->setNext(getPlayer(), i);
	map->setCreature(getParty(i)->getX(), 
					 getParty(i)->getY(), 
					 getParty(i)->getZ(), 
					 getParty(i));
  }
 
  // center map on the player
  map->center(startx, starty);
  
  // Must be called after MiniMap has been built by dg->toMap() !!! 
  miniMap->computeDrawValues();

  // set to receive events here
  sdlHandler->setHandlers((SDLEventHandler *)this, (SDLScreenView *)this);



  // run mission
  sdlHandler->mainLoop();



  // remove gui
  mainWin->setVisible(false);

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
	  //fprintf(stderr, "Freeing item: i=%d name=%s\n", i, items[i]->getRpgItem()->getName());
	  delete items[i];
	}
  }
  for(int i = 0; i < creatureCount; i++) {
	//fprintf(stderr, "Freeing creature: i=%d name=%s\n", i, creatures[i]->getCharacter()->getName());
	delete creatures[i];
  }
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

void Scourge::drawView(SDL_Surface *screen) {
  glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
  
  map->draw(screen);
  
  glDisable( GL_DEPTH_TEST );
  glDisable( GL_TEXTURE_2D );

  if(isInfoShowing) {
    map->initMapView();  
    for(int i = 0; i < 4; i++) {
	  if(map->getSelectedDropTarget() && 
		 map->getSelectedDropTarget()->creature == party[i]) {
		glColor4f(0, 1, 1, 0.5f);
	  } else if(player == party[i]) {
		glColor4f(0.0f, 1.0f, 0.0f, 0.5f);
	  } else {
		glColor4f(0.7f, 0.7f, 0.7f, 0.25f);
	  }
	  map->showCreatureInfo(party[i]);
    }
  }

  map->drawDescriptions();

  miniMap->draw(0, 400);

  if(inventory->isVisible()) inventory->drawInventory();

  glEnable( GL_DEPTH_TEST );
  glEnable( GL_TEXTURE_2D );      
}

void Scourge::setPlayer(int n) {
  player = party[n];
  player->setNextDontMove(NULL, 0);
  // init the rest of the party
  int count = 1;
  for(int i = 0; i < 4; i++) {
	if(i != n) party[i]->setNextDontMove(player, count++);
  }
  map->refresh();
}

bool Scourge::handleEvent(SDL_Event *event) {
  string ea;  

  if(inventory->isVisible()) {
	inventory->handleEvent(event);
	return false;
  }

  if(optionsMenu->isVisible()) {
	optionsMenu->handleEvent(event);
	return false;
  }


  switch(event->type) {
  case SDL_MOUSEMOTION:
    if(event->motion.x < 10) {
      map->setZRot(2.0f);
    } else if(event->motion.x >= sdlHandler->getScreen()->w - 10) {
      map->setZRot(-2.0f);
    } else {
      map->setZRot(0.0f);
    }
    if(event->motion.y < 10) {
      map->setYRot(2.0f);
    } else if(event->motion.y >= sdlHandler->getScreen()->h - 10) {
      map->setYRot(-2.0f);
    } else {
      map->setYRot(0.0f);
    }
    processGameMouseMove(event->motion.x, event->motion.y);
    break;
  case SDL_MOUSEBUTTONUP:
    if(event->button.button) {
	  processGameMouseClick(event->button.x, event->button.y, event->button.button);
    }
    break;
  case SDL_KEYDOWN:
  case SDL_KEYUP:
  
    if(event->key.keysym.sym == SDLK_ESCAPE){
        player->setSelXY(-1, -1);   // stop moving
        movingItem = NULL;          // stop moving items
        return true;
    }
    
    // xxx_yyy_stop means : "do this action when the corresponding key is up"
    ea = userConfiguration->getEngineAction(event);
    cout << "scourge EA reçue : '" << ea << "'" << endl;
    if(ea == "set_move_down"){        
        map->setMove(Constants::MOVE_DOWN);
    }
    else if(ea == "set_move_up"){
        map->setMove(Constants::MOVE_UP);
    }
    else if(ea == "set_move_right"){
        map->setMove(Constants::MOVE_RIGHT);
    }
    else if(ea == "set_move_left"){
        map->setMove(Constants::MOVE_LEFT);
    }
    else if(ea == "set_move_down_stop"){        
        map->removeMove(Constants::MOVE_DOWN);
    }
    else if(ea == "set_move_up_stop"){
        map->removeMove(Constants::MOVE_UP);
    }
    else if(ea == "set_move_right_stop"){
        map->removeMove(Constants::MOVE_RIGHT);
    }
    else if(ea == "set_move_left_stop"){
        map->removeMove(Constants::MOVE_LEFT);
    }            
    else if(ea == "set_player_0"){
        setPlayer(0);
    }
    else if(ea == "set_player_1"){
        setPlayer(1);
    }
    else if(ea == "set_player_2"){
        setPlayer(2);
    }
    else if(ea == "set_player_3"){
        setPlayer(3);
    }
    else if(ea == "set_player_only"){
        player_only = (player_only ? false : true);
    }    
    else if(ea == "blend_a"){
        blendA++; if(blendA >= 11) blendA = 0;
        fprintf(stderr, "blend: a=%d b=%d\n", blendA, blendB);
    }
    else if(ea == "blend_b"){    
        blendB++; if(blendB >= 11) blendB = 0;
        fprintf(stderr, "blend: a=%d b=%d\n", blendA, blendB);
    }
    else if(ea == "show_inventory"){
        inventory->show();        
    }
    else if(ea == "show_options_menu"){
        optionsMenu->show();
    }
    else if(ea == "use_item_stop"){
        useItem();
    }
    else if(ea == "set_next_formation_stop"){
        if(getFormation() < Creature::FORMATION_COUNT - 1) setFormation(getFormation() + 1);
    }   
    else if(ea == "set_x_rot_plus"){
        map->setXRot(1.0f);
    }
    else if(ea == "set_x_rot_minus"){
        map->setXRot(-1.0f);
    }
    else if(ea == "set_y_rot_plus"){
        map->setYRot(1.0f);
    }
    else if(ea == "set_y_rot_minus"){
        map->setYRot(-1.0f);
    }
    else if(ea == "set_z_rot_plus"){
        map->setZRot(1.0f);
    }
    else if(ea == "set_z_rot_minus"){
        map->setZRot(-1.0f);
    }    
    else if(ea == "set_x_rot_plus_stop"){
        map->setXRot(0.0f);
    }
    else if(ea == "set_x_rot_minus_stop"){
        map->setXRot(0.0f);
    }
    else if(ea == "set_y_rot_plus_stop"){
        map->setYRot(0.0f);
    }
    else if(ea == "set_y_rot_minus_stop"){
        map->setYRot(0.0f);
    }
    else if(ea == "set_z_rot_plus_stop"){
        map->setZRot(0.0f);
    }
    else if(ea == "set_z_rot_minus_stop"){
        map->setZRot(0.0f);
    }
    
    else if(ea == "add_x_pos_plus"){
        map->addXPos(10.0f);
    }
    else if(ea == "add_x_pos_minus"){
        map->addXPos(-10.0f);
    }
    else if(ea == "add_y_pos_plus"){
        map->addYPos(10.0f);
    }
    else if(ea == "add_y_pos_minus"){
        map->addYPos(-10.0f);
    }
    else if(ea == "add_z_pos_plus"){
        map->addZPos(10.0f);
    }
    else if(ea == "add_z_pos_minus"){
        map->addZPos(-10.0f);
    } 
    
    else if(ea == "minimap_zoom_in"){
        miniMap->zoomIn();
    }
    else if(ea == "minimap_zoom_out"){
        miniMap->zoomOut();
    }
    else if(ea == "minimap_toggle"){
        miniMap->toggle();
    }
    else if(ea == "set_zoom_in"){
        map->setZoomIn(true);
    }
    else if(ea == "set_zoom_out"){
        map->setZoomOut(true);
    }
    else if(ea == "set_zoom_in_stop"){
        map->setZoomIn(false);
    }
    else if(ea == "set_zoom_out_stop"){
        map->setZoomOut(false);
    }
      /*case SDL_KEYDOWN:
    switch(event->key.keysym.sym) {
    case SDLK_ESCAPE: 
	  player->setSelXY(-1, -1); // stop moving
	  movingItem = NULL; // stop moving items
	  return true;
	 
    //case SDLK_F10:
      //isInfoShowing = (isInfoShowing ? false : true);
      //gui->setWindowVisible(topWin, isInfoShowing);
      //break;
	  
    case SDLK_DOWN:
	  map->setMove(Constants::MOVE_DOWN);
      break;
    case SDLK_UP:
	  map->setMove(Constants::MOVE_UP);
      break;
    case SDLK_LEFT:
	  map->setMove(Constants::MOVE_LEFT);
      break;
    case SDLK_RIGHT:
	  map->setMove(Constants::MOVE_RIGHT);
      break;

	case SDLK_SPACE:
	  moveMonsters();
	  break;

	case SDLK_1:
	  setPlayer(0); break;
	case SDLK_2:
	  setPlayer(1); break;
	case SDLK_3:
	  setPlayer(2); break;
	case SDLK_4:
	  setPlayer(3); break;
	case SDLK_0:
	  player_only = (player_only ? false : true);
	  break;

    case SDLK_5:
        blendA++; if(blendA >= 11) blendA = 0;
        fprintf(stderr, "blend: a=%d b=%d\n", blendA, blendB);
        break;
    case SDLK_6:
        blendB++; if(blendB >= 11) blendB = 0;
        fprintf(stderr, "blend: a=%d b=%d\n", blendA, blendB);
        break;

    case SDLK_i:
        inventory->show();
        break;
    case SDLK_o:
        optionsMenu->show();
        break;
    case SDLK_q:
        map->setXRot(1.0f);
        break;
    case SDLK_w:
        map->setXRot(-1.0f);
        break;
    case SDLK_a:
        map->setYRot(1.0f);
        break;
    case SDLK_s:
        map->setYRot(-1.0f);
        break;
    case SDLK_z:
        map->setZRot(1.0f);
        break;
    case SDLK_x:
        map->setZRot(-1.0f);
        break;

    case SDLK_t:
        map->addXPos(10.0f);
        break;
    case SDLK_y:
        map->addXPos(-10.0f);
        break;        
    case SDLK_g:
        map->addYPos(10.0f);
        break;
    case SDLK_h:
        map->addYPos(-10.0f);
        break;        
    case SDLK_b:
        map->addZPos(10.0f);
        break;
    case SDLK_n:
        map->addZPos(-10.0f);
        break;   
    case SDLK_KP_PLUS:
        miniMap->zoomIn();
        break;
    case SDLK_KP_MINUS:
        miniMap->zoomOut();
        break;  
    case SDLK_l:
        miniMap->toggle(); 
        break;   
    case SDLK_LEFTBRACKET:
        map->setZoomOut(true);
        break;
    case SDLK_RIGHTBRACKET:
        map->setZoomIn(true);
        break;

    default: break;
    }
    break;
  case SDL_KEYUP:
    switch(event->key.keysym.sym) {    
    case SDLK_DOWN:
      map->removeMove(Constants::MOVE_DOWN);
      break;
    case SDLK_UP:
      map->removeMove(Constants::MOVE_UP);
      break;
    case SDLK_LEFT:
      map->removeMove(Constants::MOVE_LEFT);
      break;
    case SDLK_RIGHT:
      map->removeMove(Constants::MOVE_RIGHT);
      break;
	case SDLK_f:
	  if(getFormation() < Creature::FORMATION_COUNT - 1) setFormation(getFormation() + 1);
	  else setFormation(0);
	  break;
	case SDLK_u:
	  useItem();
      break;
    case SDLK_q:
        map->setXRot(0.0f);
        break;
    case SDLK_w:
        map->setXRot(0.0f);
        break;
    case SDLK_a:
        map->setYRot(0.0f);
        break;
    case SDLK_s:
        map->setYRot(0.0f);
        break;
    case SDLK_z:
        map->setZRot(0.0f);
        break;
    case SDLK_x:
        map->setZRot(0.0f);
        break;      
    case SDLK_LEFTBRACKET:
        map->setZoomOut(false);
        break;
    case SDLK_RIGHTBRACKET:
        map->setZoomIn(false);
        break;

    default: break;
    }*/
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
		// FIXME: add container handling code
		if(!(dropTarget && dropTarget->creature)) dropTarget = NULL;
	  }
	}
	map->setSelectedDropTarget(dropTarget);

	map->handleMouseMove(mapx, mapy, 0);
  }
}

void Scourge::processGameMouseClick(Uint16 x, Uint16 y, Uint8 button) {
  Uint16 mapx, mapy, mapz;
  if(button == SDL_BUTTON_LEFT) {
	getMapXYZAtScreenXY(x, y, &mapx, &mapy, &mapz);
	if(mapx > MAP_WIDTH) getMapXYAtScreenXY(x, y, &mapx, &mapy);
	if(useItem(mapx, mapy)) return;
	getMapXYAtScreenXY(x, y, &mapx, &mapy);
	player->setSelXY(mapx, mapy);
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
        *mapx = map->getX() + (Uint16)(((obj_x) * GLShape::DIV)) - 1;
        *mapy = map->getY() + (Uint16)(((obj_y) * GLShape::DIV)) + 2;
        //*mapz = (Uint16)0;
        //*mapz = (Uint16)(obj_z * GLShape::DIV);
    } else {
        //*mapx = *mapy = *mapz = MAP_WIDTH + 1;
        *mapx = *mapy = MAP_WIDTH + 1;
    }

    glPopMatrix();
}

void Scourge::getMapXYZAtScreenXY(Uint16 x, Uint16 y,
                                  Uint16 *mapx, Uint16 *mapy, Uint16 *mapz) {
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
    map->draw(sdlHandler->getScreen());
    map->selectMode = false;

    glFlush();    
    hits = glRenderMode(GL_RENDER);
	//    fprintf(stderr, "hits=%d\n", hits);
    if (hits > 0)												// If There Were More Than 0 Hits
	{
		int	choose = buffer[4];									// Make Our Selection The First Object
		int depth = buffer[1];									// Store How Far Away It Is
        char *s;

		for (int loop = 0; loop < hits; loop++)					// Loop Through All The Detected Hits
		{
            
            s = NULL;
			//            fprintf(stderr, "\tloop=%d 0=%u 1=%u 2=%u 3=%u 4=%u \n", loop, 
			//                    buffer[loop*5+0], buffer[loop*5+1], buffer[loop*5+2], 
			//                    buffer[loop*5+3],  buffer[loop*5+4]);
            if(buffer[loop*5+4] > 0) {
                decodeName(buffer[loop*5+4], mapx, mapy, mapz);
                if(*mapx < MAP_WIDTH) {
                    Location *pos = map->getPosition(*mapx, *mapy, *mapz);
                    if(pos) {
                        if(pos->shape && pos->shape->getName()) {
						  //                            fprintf(stderr, "\tname=%s\n", pos->shape->getName());
                        } else if(pos->item && pos->item->getShape() && pos->item->getShape()->getName()) {
						  //                            fprintf(stderr, "\tname=ITEM:%s\n", pos->item->getShape()->getName());
                        }
                    }
                }
            }
            
			// If This Object Is Closer To Us Than The One We Have Selected
			if (buffer[loop*5+1] < GLuint(depth))
			{
				choose = buffer[loop*5+4];						// Select The Closer Object
				depth = buffer[loop*5+1];						// Store How Far Away It Is
			}
		}
        
		//        fprintf(stderr, "\n\n*** choose=%u\n", choose);
        //if(choose > 0) {
            decodeName(choose, mapx, mapy, mapz);
            if(*mapx < MAP_WIDTH) {
                Location *pos = map->getPosition(*mapx, *mapy, *mapz);
                if(pos) {
                    if(pos->shape && pos->shape->getName()) {
					  //                        fprintf(stderr, "\tname=%s\n", pos->shape->getName());
                    } else if(pos->item && pos->item->getShape() && pos->item->getShape()->getName()) {
					  //                        fprintf(stderr, "\tname=ITEM:%s\n", pos->item->getShape()->getName());
                    }
                }
            }
        //}
    } else {
	  *mapx = *mapy = MAP_WIDTH + 1;
	}

 // Restore the projection matrix
 glMatrixMode(GL_PROJECTION);
 glPopMatrix();

 // Go back to modelview for normal rendering
 glMatrixMode(GL_MODELVIEW);
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

void Scourge::moveParty() {
  player->moveToLocator(map);
  map->center(player->getX(), player->getY());
  if(!player_only) {
	for(int i = 0; i < 4; i++) {
	  if(party[i] != player) party[i]->follow(map);
	}
  }
}

void Scourge::setPartyMotion(int motion) {
  for(int i = 0; i < 4; i++) {
	if(party[i] != player) party[i]->setMotion(motion);
  }
}

void Scourge::setFormation(int formation) {
	this->formation = formation;
	for(int i = 0; i < 4; i++) {
		party[i]->setFormation(formation);
	}
}

bool Scourge::useItem(int x, int y) {
  if(movingItem) {
	//	dropItem(x, y);
	dropItem(map->getSelX(), map->getSelY());
	return true;
  }
  
  Location *pos = map->getPosition(x, y, player->getZ());
  if(pos) {
	if(getItem(pos)) {  
	  return true;
	} else if(useDoor(pos)) {
	  map->updateLightMap();
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
        map->removeItem(pos->x, pos->y, pos->z);
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
  if(map->getSelectedDropTarget()) {
	char message[120];
	Creature *c = map->getSelectedDropTarget()->creature;
	if(c) {
	  c->addInventory(movingItem);
	  sprintf(message, "%s picks up %s.", 
			  c->getName(), 
			  movingItem->getRpgItem()->getName());
	  map->addDescription(strdup(message));
	  movingItem = NULL;
	  movingX = movingY = movingZ = MAP_WIDTH + 1;
	}
  } else if(!map->isBlocked(x, y, 0,
					 movingX, movingY, movingZ,
					 movingItem->getShape())) {
	map->setItem(x, y, 0, movingItem);
	//	movingItem->moveTo(x, y, 0);
	movingItem = NULL;
	movingX = movingY = movingZ = MAP_WIDTH + 1;
  }
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

bool Scourge::useItem() {
  for(int x = player->getX() - 2; 
	  x < player->getX() + player->getShape()->getWidth() + 2; 
	  x++) {
	for(int y = player->getY() + 2; 
		y > player->getY() - player->getShape()->getDepth() - 2; 
		y--) {
	  if(useItem(x, y)) return true;
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
  if(inventory->isVisible()) {
	inventory->handleEvent(widget, event);
	return false;
  }

  if(optionsMenu->isVisible()) {
	optionsMenu->handleEvent(widget, event);
	return false;
  }


  if(widget == inventoryButton) {
	if(inventory->isVisible()) inventory->hide();
	else inventory->show();
  } else if(widget == optionsButton) {
	if(optionsMenu->isVisible()) optionsMenu->hide();
	else optionsMenu->show();
  } else if(widget == quitButton) {
	return true;
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
	player_only = (player_only ? false : true);
  }
  return false;
}

void Scourge::createUI() {
  // create the ui
  char version[100];
  sprintf(version, "S.C.O.U.R.G.E. version %7.2f", SCOURGE_VERSION);
  mainWin = new Window( getSDLHandler(),
						sdlHandler->getScreen()->w - GUI_WIDTH, 
						sdlHandler->getScreen()->h - GUI_HEIGHT, 
						GUI_WIDTH, GUI_HEIGHT, 
						strdup(version), 
						getShapePalette()->getGuiTexture() );
  int gx = sdlHandler->getScreen()->w - GUI_WIDTH;
  int gy = sdlHandler->getScreen()->h - GUI_HEIGHT;
  inventoryButton = new Button( 0, 0, 100, 25, strdup("Party Info") );
  mainWin->addWidget((Widget*)inventoryButton);
  optionsButton = new Button( 0, 25,  100, 50, strdup("Options") );
  mainWin->addWidget((Widget*)optionsButton);
  quitButton = new Button( 0, 50,  100, 75, strdup("Quit") );
  mainWin->addWidget((Widget*)quitButton);

  diamondButton = new Button( 100, 0,  120, 20 );
  mainWin->addWidget((Widget*)diamondButton);
  staggeredButton = new Button( 120, 0,  140, 20 );
  mainWin->addWidget((Widget*)staggeredButton);
  squareButton = new Button( 140, 0,  160, 20 );
  mainWin->addWidget((Widget*)squareButton);
  rowButton = new Button( 160, 0,  180, 20 );
  mainWin->addWidget((Widget*)rowButton);
  scoutButton = new Button( 180, 0,  200, 20 );
  mainWin->addWidget((Widget*)scoutButton);
  crossButton = new Button( 200, 0,  220, 20 );
  mainWin->addWidget((Widget*)crossButton);

  player1Button = new Button( 100, 20,  124, 40 );
  mainWin->addWidget((Widget*)player1Button);
  player2Button = new Button( 124, 20,  148, 40 );
  mainWin->addWidget((Widget*)player2Button);
  player3Button = new Button( 148, 20,  172, 40 );
  mainWin->addWidget((Widget*)player3Button);
  player4Button = new Button( 172, 20,  196, 40 );
  mainWin->addWidget((Widget*)player4Button);
  groupButton = new Button( 196, 20,  220, 40 );
  mainWin->addWidget((Widget*)groupButton);
}

void Scourge::moveMonsters() {
  fprintf(stderr, "FIXME: only move visible creatures!\n");
  fprintf(stderr, "FIXME: cleanup Creature::move()!\n");
  for(int i = 0; i < creatureCount; i++) {
	moveMonster(creatures[i]);
  }
}

// map calls this for every monster visible
void Scourge::moveMonster(Creature *monster) {
  // for now just twitch around
  // FIXME: this needs to be a lot more intelligent!
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
