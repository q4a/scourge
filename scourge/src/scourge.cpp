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

#define GUI_TOP 475

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

Scourge::Scourge(int width, int height,
                 int bpp, bool fullscreen){

  movingX = movingY = movingZ = MAP_WIDTH + 1;
  movingItem = NULL;

  isInfoShowing = true;

	// Initialize the video mode
  cout << "Setting video mode: " << width << "x" << height << "," << bpp << endl;
  sdlHandler = new SDLHandler();
  sdlHandler->setVideoMode(width, height, bpp, fullscreen);

  shapePal = sdlHandler->getShapePalette();
  gui = new Gui(this);

   // init the party
	party[0] = player = new Creature(this, shapePal->getCreatureShape(ShapePalette::ROGUE_INDEX));
	party[1] = new Creature(this, shapePal->getCreatureShape(ShapePalette::FIGHTER_INDEX));
	party[2] = new Creature(this, shapePal->getCreatureShape(ShapePalette::CLERIC_INDEX));
	party[3] = new Creature(this, shapePal->getCreatureShape(ShapePalette::WIZARD_INDEX));      

  // hard code the party for now
  party[0]->setName("Alamont"); party[0]->setPortraitIndex(0);
  party[0]->setCharacter(Character::knight);
  party[0]->setLevel(1); party[0]->setExp(300);
  party[0]->rollAttributes();
  party[0]->setHp();
  party[0]->setStateMod(Creature::blessed, true);
  party[0]->setStateMod(Creature::poisoned, true);
  for(int i = 0; i < Creature::SKILL_COUNT; i++) {
      party[0]->setSkill(i, (int)(100.0 * rand()/RAND_MAX));
  }
  
  party[1]->setName("Barlett"); party[1]->setPortraitIndex(1);
  party[1]->setCharacter(Character::loremaster);
  party[1]->setLevel(1); party[1]->setExp(200);
  party[1]->rollAttributes();
  party[1]->setHp();
  party[1]->setStateMod(Creature::drunk, true);
  party[1]->setStateMod(Creature::cursed, true);      
  for(int i = 0; i < Creature::SKILL_COUNT; i++) {
      party[1]->setSkill(i, (int)(100.0 * rand()/RAND_MAX));
  }
  
  party[2]->setName("Corinus"); party[2]->setPortraitIndex(2);
  party[2]->setCharacter(Character::summoner);
  party[2]->setLevel(1); party[2]->setExp(150);
  party[2]->rollAttributes();
  party[2]->setHp();
  party[2]->setStateMod(Creature::ac_protected, true);
  party[2]->setStateMod(Creature::magic_protected, true);
  party[2]->setStateMod(Creature::cursed, true);        
  for(int i = 0; i < Creature::SKILL_COUNT; i++) {
      party[2]->setSkill(i, (int)(100.0 * rand()/RAND_MAX));
  }
  
  party[3]->setName("Dialante"); party[3]->setPortraitIndex(3);
  party[3]->setCharacter(Character::naturalist);
  party[3]->setLevel(1); party[3]->setExp(400);
  party[3]->rollAttributes();
  party[3]->setHp();
  party[3]->setStateMod(Creature::possessed, true);          
  for(int i = 0; i < Creature::SKILL_COUNT; i++) {
      party[3]->setSkill(i, (int)(100.0 * rand()/RAND_MAX));
  }

  inventory = new Inventory(this);
  
  // show the main menu
  mainMenu = new MainMenu(this);  

  while(true) {
    mainMenu->init();    
    sdlHandler->setHandlers((SDLEventHandler *)mainMenu, (SDLScreenView *)mainMenu);
    sdlHandler->mainLoop();
    mainMenu->destroy();

    // evaluate results and start a missions
    fprintf(stderr, "value=%d\n", mainMenu->getValue());
    if(mainMenu->getValue() == NEW_GAME) {
      //charBuilder-
      startMission();
    } else if(mainMenu->getValue() == QUIT) {
      sdlHandler->quit(0);
    }
  }

  delete mainMenu;
}

Scourge::~Scourge(){
}

void Scourge::startMission() {
  // add gui
  topWin = gui->addWindow(500, GUI_TOP, 300, sdlHandler->getScreen()->h - GUI_TOP, 
						  &Gui::drawDescriptions);
  gui->addActiveRegion(680, GUI_TOP,  800, GUI_TOP + 25, Constants::SHOW_INVENTORY, this);
  gui->addActiveRegion(680, GUI_TOP + 25, 800, GUI_TOP + 50, Constants::SHOW_OPTIONS, this);
  gui->addActiveRegion(680, GUI_TOP + 50, 800, GUI_TOP + 75, Constants::ESCAPE, this);
  gui->addActiveRegion(680, GUI_TOP + 75, 700, GUI_TOP + 100, Constants::DIAMOND_FORMATION, this);
  gui->addActiveRegion(700, GUI_TOP + 75, 720, GUI_TOP + 100, Constants::STAGGERED_FORMATION, this);
  gui->addActiveRegion(720, GUI_TOP + 75, 740, GUI_TOP + 100, Constants::SQUARE_FORMATION, this);
  gui->addActiveRegion(740, GUI_TOP + 75, 760, GUI_TOP + 100, Constants::ROW_FORMATION, this);
  gui->addActiveRegion(760, GUI_TOP + 75, 780, GUI_TOP + 100, Constants::SCOUT_FORMATION, this);
  gui->addActiveRegion(780, GUI_TOP + 75, 800, GUI_TOP + 100, Constants::CROSS_FORMATION, this);

  // create the map
  map = new Map(this);

  map->addDescription(Constants::getMessage(Constants::WELCOME));
  map->addDescription("----------------------------------");
  
	// Initialize the map with a random dunegeon	
	DungeonGenerator *dg = new DungeonGenerator(1);
	Sint16 startx, starty;
  dg->toMap(map, &startx, &starty, shapePal);

  // position the players
  player->moveTo(startx, starty, 0);
  map->setCreature(startx, starty, 0, player);

	// init the rest of the party
	for(int i = 1; i < 4; i++) {
		party[i]->setNext(party[0], i);
		//		map->setPosition(party[i]->getX(), party[i]->getY(), party[i]->getZ(), party[i]->getShape());
		map->setCreature(party[i]->getX(), party[i]->getY(), party[i]->getZ(), party[i]);
	}

	// center on the player
	map->center(startx, starty);

  sdlHandler->setHandlers((SDLEventHandler *)this, (SDLScreenView *)this);
  sdlHandler->mainLoop();

  // remove gui
  gui->removeWindow(topWin);
  delete map;
  delete dg;
}

void Scourge::drawView(SDL_Surface *screen) {
  glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
  
  map->draw(screen);
  
  glDisable( GL_LIGHTING );
  glDisable( GL_TEXTURE_2D );
  glDisable( GL_DEPTH_TEST );

  if(isInfoShowing) {
    map->initMapView();  
    for(int i = 0; i < 4; i++) {
      map->showCreatureInfo(party[i]);
    }
  }

  gui->drawWindows();

  glEnable( GL_DEPTH_TEST );
  glEnable( GL_LIGHTING );
  glEnable( GL_TEXTURE_2D );      
}

bool Scourge::handleEvent(SDL_Event *event) {
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
        int region = gui->testActiveRegions(event->button.x, event->button.y);
        if(region == Constants::SHOW_INVENTORY) {
            inventory->show();
        } else if(region == Constants::SHOW_OPTIONS) {
            // do something
        } else if(region == Constants::ESCAPE) {
            return true;
		} else if(region >= Constants::DIAMOND_FORMATION && region <= Constants::CROSS_FORMATION) {
		  setFormation(region - Constants::DIAMOND_FORMATION);
        } else {        
            processGameMouseClick(event->button.x, event->button.y, event->button.button);
        }
    }
    break;
  case SDL_KEYDOWN:
    switch(event->key.keysym.sym) {
    case SDLK_ESCAPE: 
	  party[0]->setSelXY(-1, -1); // stop moving
	  movingItem = NULL; // stop moving items
	  return true;
    case SDLK_F10:
      isInfoShowing = (isInfoShowing ? false : true);
      gui->setWindowVisible(topWin, isInfoShowing);
      break;
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

    case SDLK_F5:
        blendA++; if(blendA >= 11) blendA = 0;
        fprintf(stderr, "blend: a=%d b=%d\n", blendA, blendB);
        break;
    case SDLK_F6:
        blendB++; if(blendB >= 11) blendB = 0;
        fprintf(stderr, "blend: a=%d b=%d\n", blendA, blendB);
        break;

    case SDLK_i:
        inventory->show();
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
    case SDLK_2:
        map->setZoomOut(true);
        break;
    case SDLK_1:
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
    case SDLK_2:
        map->setZoomOut(false);
        break;
    case SDLK_1:
        map->setZoomIn(false);
        break;

    default: break;
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
	party[0]->setSelXY(mapx, mapy);      
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
    int viewport[4];

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
    party[0]->moveToLocator(map);
    map->center(party[0]->getX(), party[0]->getY());
	for(int i = 1; i < 4; i++) {
	  party[i]->follow(map);
	}
}

void Scourge::setPartyMotion(int motion) {
  for(int i = 1; i < 4; i++) {
		party[i]->setMotion(motion);
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
  
  Location *pos = map->getPosition(x, y, party[0]->getZ());
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
							getParty(0)->getX(),
							getParty(0)->getY(),
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

void Scourge::dropItem(int x, int y) {
    if(!map->isBlocked(x, y, 0,
                       movingX, movingY, movingZ,
                       movingItem->getShape())) {
        map->setItem(x, y, 0, movingItem);
        movingItem->moveTo(x, y, 0);
        movingItem = NULL;
        movingX = movingY = movingZ = MAP_WIDTH + 1;
    }
}

bool Scourge::useDoor(Location *pos) {
    Shape *newDoorShape = NULL;
    if(pos->shape == shapePal->getShape(ShapePalette::EW_DOOR_INDEX)) {
        newDoorShape = shapePal->getShape(ShapePalette::NS_DOOR_INDEX);
    } else if(pos->shape == shapePal->getShape(ShapePalette::NS_DOOR_INDEX)) {
        newDoorShape = shapePal->getShape(ShapePalette::EW_DOOR_INDEX);
    }
    if(newDoorShape) {
        // switch door
        Sint16 ox = pos->x;
        Sint16 oy = pos->y;
        Sint16 nx = pos->x;
        Sint16 ny = (pos->y - pos->shape->getDepth()) + newDoorShape->getDepth();
        Shape *oldDoorShape = map->removePosition(ox, oy, party[0]->getZ());
        if(!map->isBlocked(nx, ny, party[0]->getZ(),
                           ox, oy, party[0]->getZ(),
                           newDoorShape)) {
            map->setPosition(nx, ny, party[0]->getZ(), newDoorShape);
            return true;
        } else {
            // rollback
            map->setPosition(ox, oy, party[0]->getZ(), oldDoorShape);
        }
    }
    return false;
}

bool Scourge::useItem() {
  for(int x = party[0]->getX() - 2; 
	  x < party[0]->getX() + party[0]->getShape()->getWidth() + 2; 
	  x++) {
	for(int y = party[0]->getY() + 2; 
		y > party[0]->getY() - party[0]->getShape()->getDepth() - 2; 
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

void Scourge::drawTopWindow() {

  glPushMatrix();
  glLoadIdentity();

    glColor4f(1.0f, 1.0f, 0.4f, 1.0f);
    sdlHandler->texPrint(705, GUI_TOP + 15, "Party Info");
    sdlHandler->texPrint(705, GUI_TOP + 40, "Options");
    sdlHandler->texPrint(705, GUI_TOP + 65, "Quit");

    glColor3f(1.0f, 0.6f, 0.3f);
    glBegin(GL_LINES);
        glVertex2d(680, GUI_TOP);
        glVertex2d(680, 600);
        glVertex2d(680, GUI_TOP + 25);
        glVertex2d(800, GUI_TOP + 25);
        glVertex2d(680, GUI_TOP + 50);
        glVertex2d(800, GUI_TOP + 50);
        glVertex2d(680, GUI_TOP + 75);
        glVertex2d(800, GUI_TOP + 75);
		glVertex2d(680, GUI_TOP + 100);
        glVertex2d(800, GUI_TOP + 100);
        glVertex2d(700, GUI_TOP + 100);
        glVertex2d(700, GUI_TOP + 75);
        glVertex2d(720, GUI_TOP + 100);
        glVertex2d(720, GUI_TOP + 75);
        glVertex2d(740, GUI_TOP + 100);
        glVertex2d(740, GUI_TOP + 75);
        glVertex2d(760, GUI_TOP + 100);
        glVertex2d(760, GUI_TOP + 75);
        glVertex2d(780, GUI_TOP + 100);
        glVertex2d(780, GUI_TOP + 75);
    glEnd();
	/*
    // debug info
    sdlHandler->texPrint(450, 10, "rot: %f, %f, %f", map->getXRot(), map->getYRot(), map->getZRot());
    sdlHandler->texPrint(450, 30, "FPS: %g", sdlHandler->getFPS());
    sdlHandler->texPrint(450, 50, "map: (%d, %d)-(%d, %d)", map->getX(), map->getY(), 
                         (map->getX() + MAP_VIEW_WIDTH), (map->getY() + MAP_VIEW_DEPTH));
    sdlHandler->texPrint(450, 70, "sel: (%u, %u, %u)", 
                         map->getSelX(), map->getSelY(), map->getSelZ());
    sdlHandler->texPrint(450, 90, "zoom: %f", map->getZoom());
	*/
    map->drawDescriptions();

	glPopMatrix();
}


