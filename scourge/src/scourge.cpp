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

#define MOUSE_ROT_DELTA 2

#define BATTLES_ENABLED 1

#define DRAG_START_TOLERANCE 5

#define INFO_INTERVAL 3000

#define DEBUG_KEYS 1

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

Scourge::Scourge(UserConfiguration *config) : GameAdapter(config) {
  lastTick = 0;
  messageWin = NULL;
  movingX = movingY = movingZ = MAP_WIDTH + 1;
  movingItem = NULL;
  needToCheckInfo = false;
  needToCheckDropLocation = true;
  nextMission = -1;
  teleportFailure = false;
  cursorMapX = cursorMapY = cursorMapZ = MAP_WIDTH + 1;
  // in HQ map
  inHq = true;
  //showPath = config->getAlwaysShowPath();

  layoutMode = Constants::GUI_LAYOUT_BOTTOM;
  
  isInfoShowing = true; // what is this?
  info_dialog_showing = false;

  // we're not in target selection mode
  targetSelectionFor = NULL;
  
  move = 0;
  battleCount = 0;  
  inventory = NULL;
  containerGuiCount = 0;
  changingStory = false;  

  targetWidth = 0.0f;
  targetWidthDelta = 0.05f / GLShape::DIV;
  lastTargetTick = SDL_GetTicks();

  lastEffectOn = false;
  resetBattles();

  turnProgress = new Progress(this, 10, false, false, false);
  mouseZoom = mouseRot = false;
  willStartDrag = false;
  willStartDragX = willStartDragY = 0;
}

void Scourge::initVideo(ShapePalette *shapePal) {
  this->shapePal = shapePal;

  // Initialize the video mode
  sdlHandler = new SDLHandler(shapePal); 
  sdlHandler->setVideoMode(userConfiguration); 
}

void Scourge::initUI() {
  // init UI themes
  GuiTheme::initThemes( shapePal );

  // for now pass map in
  this->levelMap = session->getMap();
  miniMap = new MiniMap(this); 

  // create the mission board
  this->board = session->getBoard();

  this->party = session->getParty();  
  createPartyUI();  
  createBoardUI();
  netPlay = new NetPlay(this);
  createUI();

  // show the main menu
  mainMenu = new MainMenu(this);
  optionsMenu = new OptionsMenu(this);
  multiplayer = new MultiplayerDialog(this);

  // load character, item sounds
  sdlHandler->getSound()->loadSounds(session->getUserConfiguration());
}

void Scourge::start() {
  this->quadric = gluNewQuadric();
  bool initMainMenu = true;
  while(true) {

    if(initMainMenu) {
      initMainMenu = false;
      mainMenu->show();    
      sdlHandler->getSound()->playMusicMenu();
    }

    sdlHandler->setHandlers((SDLEventHandler *)mainMenu, (SDLScreenView *)mainMenu);
    sdlHandler->mainLoop();

    // evaluate results and start a missions
    int value = mainMenu->getValue();
    if(value == NEW_GAME) {
      if(Persist::doesSaveGameExist( session )) {
        mainMenu->showNewGameConfirmationDialog();
      } else {
        mainMenu->showPartyEditor();
        //value = NEW_GAME_START;
      }
    }
      
    if(value == NEW_GAME_START ||
       value == MULTIPLAYER_START ||
       value == CONTINUE_GAME ) {
      mainMenu->hide();
      sdlHandler->getSound()->stopMusicMenu();
      
      initMainMenu = true;
      bool failed = false;
     
#ifdef HAVE_SDL_NET
      if(value == MULTIPLAYER_START) {
        if(!initMultiplayer()) continue;
      }
#endif  

      if(value == CONTINUE_GAME) {
        if(!Persist::loadGame( session )) {
          showMessageDialog( "Error loading game!" );
          failed = true;
        }
      }

      if( !failed ) {
        // do this to fix slowness in mainmenu the second time around
        glPushAttrib(GL_ENABLE_BIT);
        startMission();
        glPopAttrib();
      }
    } else if(value == OPTIONS) {
      toggleOptionsWindow();
    } else if(value == MULTIPLAYER) {
      multiplayer->show();
    } else if(value == QUIT) {
      sdlHandler->quit(0);
    }
  }
}

Scourge::~Scourge(){
  delete mainMenu;
  delete optionsMenu;
  delete multiplayer;
  delete miniMap;
  delete netPlay;
  delete infoGui;
  delete conversationGui;
}

void Scourge::startMission() {

  // set up some cross-mission objects
  bool resetParty = true;
  
  // always start in hq
  nextMission = -1;
  inHq = true;
  
  while(true) {

    // add gui
    mainWin->setVisible(true);
    messageWin->setVisible(true);
    if(session->isMultiPlayerGame()) netPlay->getWindow()->setVisible(true);

    // create the map
    levelMap->reset();

    // do this only once
    if(resetParty) {
      board->reset();
      if(session->isMultiPlayerGame()) {
        party->resetMultiplayer(multiplayer->getCreature());
      } else {
        party->reset();
      }
      party->getCalendar()->reset(true); // reset the time

      // inventory needs the party
      if(!inventory) {
        inventory = new Inventory(this);
      }

      resetParty = false;
    }

    miniMap->reset();
    miniMap->show();

    // ready the party
    party->startPartyOnMission();

    // save the party
    if(!session->isMultiPlayerGame()) {
      if(!Persist::saveGame(session)) {
        showMessageDialog( "Error saving game!" );
      }
    }

    // position the players
    move = 0;
    battleCount = 0;
    containerGuiCount = 0;
    lastMapX = lastMapY = lastMapZ = lastX = lastY = -1;
    teleporting = false;
    changingStory = false;
    mouseMoveScreen = true;
    targetSelectionFor = NULL;  

    if(nextMission == -1) {

      missionWillAwardExpPoints = false;

      // in HQ map
      inHq = true;

      // init the missions board
      board->initMissions();

      // display the HQ map
      getSession()->setCurrentMission(NULL);
      missionWillAwardExpPoints = false;
      dg = new DungeonGenerator(this, 2, 0, false, false); // level 2 is a big enough map for HQ_LOCATION... this is hacky
      dg->toMap(levelMap, getShapePalette(), DungeonGenerator::HQ_LOCATION);   
    } else {
      // in HQ map
      inHq = false;

      // Initialize the map with a random dunegeon	
      getSession()->setCurrentMission(board->getMission(nextMission));
      missionWillAwardExpPoints = (!getSession()->getCurrentMission()->isCompleted());
	  /*
      cerr << "Starting mission: level="  << getSession()->getCurrentMission()->getLevel() << 
      " depth=" << getSession()->getCurrentMission()->getDepth() << 
      " current story=" << currentStory << endl;
	  */
      dg = new DungeonGenerator(this, getSession()->getCurrentMission()->getLevel(), currentStory, 
                                (currentStory < getSession()->getCurrentMission()->getDepth() - 1), 
                                (currentStory > 0),
                                getSession()->getCurrentMission());
      dg->toMap(levelMap, getShapePalette());
    }
	
    // center map on the player
    levelMap->center(toint(party->getPlayer()->getX()), 
                toint(party->getPlayer()->getY()),
                true);

    // Must be called after MiniMap has been built by dg->toMap() !!! 
    //miniMap->computeDrawValues();

    // set to receive events here
    sdlHandler->setHandlers((SDLEventHandler *)this, (SDLScreenView *)this);

    // hack to unfreeze animations, etc.
    party->forceStopRound();

    // show an info dialog
    if(nextMission == -1) {
      sprintf(infoMessage, "Welcome to the S.C.O.U.R.G.E. Head Quarters");
    } else if( teleportFailure ) {
      teleportFailure = false;
      sprintf(infoMessage, "Teleport spell failed!! Entering level %d", ( currentStory + 1 ));
    } else {
      sprintf(infoMessage, "Entering dungeon level %d", ( currentStory + 1 ));
    }
    showMessageDialog(infoMessage);
    info_dialog_showing = true;

    // set the map view
    setUILayout();

    // start the haunting tunes
    if(inHq) sdlHandler->getSound()->playMusicMenu();
    else sdlHandler->getSound()->playMusicDungeon();

    // run mission
    sdlHandler->mainLoop();

    // stop the music
    sdlHandler->getSound()->stopMusicDungeon();

    // clean up after the mission
    resetInfos();

    // remove gui
    mainWin->setVisible(false);
    messageWin->setVisible(false);
    closeAllContainerGuis();
    if(inventory->isVisible()) {
      inventory->hide();
      inventoryButton->setSelected( false );
    }
    if(optionsMenu->isVisible()) {
      optionsMenu->hide();
      optionsButton->setSelected( false );
    }
    if(boardWin->isVisible()) boardWin->setVisible(false);
    miniMap->hide();
    netPlay->getWindow()->setVisible(false);
    infoGui->getWindow()->setVisible(false);
    conversationGui->getWindow()->setVisible( false );

    resetBattles();
    
    // delete active projectiles
    Projectile::resetProjectiles();

    // delete the mission level's item and monster instances
    if( session->getCurrentMission() ) 
      session->getCurrentMission()->deleteItemMonsterInstances();

    session->deleteCreaturesAndItems(true);

    // delete map
    delete dg; dg = NULL;

    //cerr << "Mission end: changingStory=" << changingStory << " inHQ=" << inHq << " teleporting=" << teleporting << " nextMission=" << nextMission << endl;
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
#ifdef HAVE_SDL_NET
  session->stopClientServer();
#endif
  session->deleteCreaturesAndItems(false);

  // delete the party (w/o deleting the party ui)
  party->deleteParty();
}

void Scourge::endMission() {
  for(int i = 0; i < party->getPartySize(); i++) {
    party->getParty(i)->setSelXY(-1, -1);   // stop moving
  }
  movingItem = NULL;          // stop moving items
  //	move = 0;  
}

void Scourge::drawView() {

  // make a move (player, monsters, etc.)
  playRound();

  updatePartyUI();

  if( getSDLHandler()->mouseIsMovingOverMap )
    getMapXYZAtScreenXY(getSDLHandler()->mouseX, getSDLHandler()->mouseY, 
                        &cursorMapX, &cursorMapY, &cursorMapZ);

  checkForDropTarget();
  checkForInfo();

  levelMap->draw();

  // cancel mouse-based map movement (middle button)
  if(mouseRot) {
    levelMap->setXRot(0);
    levelMap->setYRot(0);
    levelMap->setZRot(0);
  }
  if(mouseZoom) {
    mouseZoom = false;
    levelMap->setZoomIn(false);
    levelMap->setZoomOut(false);
  }

  // the boards outside the map
  drawOutsideMap();

  glDisable( GL_DEPTH_TEST );
  glDisable( GL_TEXTURE_2D );

  if(isInfoShowing) {
    levelMap->initMapView();

    // creatures first
    for(int i = 0; i < session->getCreatureCount(); i++) {
      if(!session->getCreature(i)->getStateMod(Constants::dead) && 
         levelMap->isLocationVisible(toint(session->getCreature(i)->getX()), 
                                toint(session->getCreature(i)->getY())) &&
         levelMap->isLocationInLight(toint(session->getCreature(i)->getX()), 
                                toint(session->getCreature(i)->getY()))) {
        showCreatureInfo(session->getCreature(i), false, false, false);
      }
    }
    // party next so red target circle shows over gray
    for(int i = 0; i < party->getPartySize(); i++) {
      if(!party->getParty(i)->getStateMod(Constants::dead)) {

        bool player = party->getPlayer() == party->getParty(i);
        if(session->getUserConfiguration()->isBattleTurnBased() && 
           party->isRealTimeMode() && 
           battleTurn < (int)battleRound.size()) {
          player = (party->getParty(i) == battleRound[battleTurn]->getCreature());
        }

        showCreatureInfo(party->getParty(i), 
                         player, 
                         (levelMap->getSelectedDropTarget() && 
                          levelMap->getSelectedDropTarget()->creature == party->getParty(i)),
                         !party->isPlayerOnly());
      }
    }

    drawInfos();

    glDisable( GL_CULL_FACE );
    glDisable( GL_SCISSOR_TEST );
  }

  levelMap->drawDescriptions(messageList);

  glEnable( GL_DEPTH_TEST );
  glEnable( GL_TEXTURE_2D );

  miniMap->buildTexture(0, 0);

  drawBorder();

  // Hack: A container window may have been closed by hitting the Esc. button.
  if(Window::windowWasClosed) {
    if(containerGuiCount > 0) {
      for(int i = 0; i < containerGuiCount; i++) {
        if(!containerGui[i]->getWindow()->isVisible()) {
          closeContainerGui(containerGui[i]);
        }
      }
    }
  }
}

void Scourge::drawOutsideMap() {
  // cover the area outside the map
  if(levelMap->getViewWidth() < sdlHandler->getScreen()->w || 
     levelMap->getViewHeight() < sdlHandler->getScreen()->h) {
    //glPushAttrib( GL_ENABLE_BIT );
    glDisable( GL_CULL_FACE );
    glDisable( GL_DEPTH_TEST );
    glEnable( GL_TEXTURE_2D );
    glPushMatrix();
    glColor3f( 1, 1, 1 );
    glBindTexture( GL_TEXTURE_2D, getShapePalette()->getGuiWoodTexture() );
    
    //    float TILE_W = 510 / 2.0f;
    float TILE_H = 270 / 2.0f; 
    
    glLoadIdentity();
    glTranslatef( levelMap->getViewWidth(), 0, 0 );
    glBegin( GL_QUADS );
    glTexCoord2f( 0, 0 );
    glVertex2i( 0, 0 );
    glTexCoord2f( 0, sdlHandler->getScreen()->h / TILE_H );
    glVertex2i( 0, sdlHandler->getScreen()->h );
    glTexCoord2f( 1, sdlHandler->getScreen()->h / TILE_H );
    glVertex2i( sdlHandler->getScreen()->w - levelMap->getViewWidth(), sdlHandler->getScreen()->h );
    glTexCoord2f( 1, 0 );
    glVertex2i( sdlHandler->getScreen()->w - levelMap->getViewWidth(), 0 );
    glEnd();
    
    glLoadIdentity();
    glTranslatef( 0, levelMap->getViewHeight(), 0 );
    glBegin( GL_QUADS );
    glTexCoord2f( 0, 0 );
    glVertex2i( 0, 0 );
    glTexCoord2f( 0, levelMap->getViewHeight() / TILE_H );
    glVertex2i( 0, levelMap->getViewHeight() );
    glTexCoord2f( 1, levelMap->getViewHeight() / TILE_H );
    glVertex2i( levelMap->getViewWidth(), levelMap->getViewHeight() );
    glTexCoord2f( 1, 0 );
    glVertex2i( levelMap->getViewWidth(), 0 );
    glEnd();
    
    glPopMatrix();
    //    glPopAttrib();
    glEnable( GL_CULL_FACE );
    glEnable( GL_DEPTH_TEST );
    glDisable( GL_TEXTURE_2D );
  }  
}

bool Scourge::inTurnBasedCombatPlayerTurn() {
  return (inTurnBasedCombat() &&
          !battleRound[battleTurn]->getCreature()->isMonster());
}         

void Scourge::drawAfter() {
  drawDraggedItem();

  // draw turn info
  if(inTurnBasedCombat()) {
    //glPushAttrib(GL_ENABLE_BIT);
    glPushMatrix();
    glLoadIdentity();
    glTranslatef( 20, 20, 0 );
    Creature *c = battleRound[battleTurn]->getCreature();
    char msg[80];
    sprintf(msg, "%s %d/%d", 
            c->getName(),
            c->getBattle()->getAP(),
            c->getBattle()->getStartingAP());
    turnProgress->updateStatus(msg, false, 
                               c->getBattle()->getAP(), 
                               c->getBattle()->getStartingAP(),
                               c->getProposedPath()->size() );
    glPopMatrix();
    //glPushAttrib(GL_ENABLE_BIT);
  }
}

void Scourge::showCreatureInfo(Creature *creature, bool player, bool selected, bool groupMode) {
  glPushMatrix();
  //showInfoAtMapPos(creature->getX(), creature->getY(), creature->getZ(), creature->getName());

  glEnable( GL_DEPTH_TEST );
  glDepthMask(GL_FALSE);
  glEnable( GL_BLEND );
  glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );
  glDisable( GL_CULL_FACE );

  // draw circle
  double w = ((double)(creature->getShape()->getWidth()) / 2.0f) / GLShape::DIV;
  double d = (((double)(creature->getShape()->getWidth()) / 2.0f) + 1.0f) / GLShape::DIV;
  double s = 0.35f / GLShape::DIV;

  float xpos2, ypos2, zpos2;

  Uint32 t = SDL_GetTicks();
  if(t - lastTargetTick > 45) {
    // initialize target width
    if(targetWidth == 0.0f) {
      targetWidth = s;
      targetWidthDelta *= -1.0f;
    }
    // targetwidth oscillation
    targetWidth += targetWidthDelta;
    if((targetWidthDelta < 0 && targetWidth < s) || 
       (targetWidthDelta > 0 && targetWidth >= s + (5 * targetWidthDelta)))
      targetWidthDelta *= -1.0f;

    lastTargetTick = t;
  }

  // show path
  if(player && 
     session->getUserConfiguration()->isBattleTurnBased() && 
     battleTurn < (int)battleRound.size() ) {
    for( int i = creature->getProposedPathIndex(); 
         i < (int)creature->getProposedPath()->size() && 
         i <= creature->getBattle()->getAP(); i++) {
      Location pos = (*(creature->getProposedPath()))[i];
      glColor4f(1, 0.4f, 0.0f, 0.5f);
      xpos2 = ((float)(pos.x - levelMap->getX()) / GLShape::DIV);
      ypos2 = ((float)(pos.y - levelMap->getY()) / GLShape::DIV);
      zpos2 = 0.0f / GLShape::DIV;  
      glPushMatrix();
      glTranslatef( xpos2 + w, ypos2 - w, zpos2 + 5);
      gluDisk(quadric, 0, 4, 12, 1);
      glPopMatrix();
    }
  }

  // Yellow for move creature target
  if(player && creature->getSelX() > -1 && 
     !creature->getTargetCreature() &&
     !(creature->getSelX() == toint(creature->getX()) && 
       creature->getSelY() == toint(creature->getY())) ) {
    // draw target
    glColor4f(1.0f, 0.75f, 0.0f, 0.5f);
    xpos2 = ((float)(creature->getSelX() - levelMap->getX()) / GLShape::DIV);
    ypos2 = ((float)(creature->getSelY() - levelMap->getY()) / GLShape::DIV);
    zpos2 = 0.0f / GLShape::DIV;  
    glPushMatrix();
    //glTranslatef( xpos2 + w, ypos2 - w * 2, zpos2 + 5);
    glTranslatef( xpos2 + w, ypos2 - w, zpos2 + 5);
    gluDisk(quadric, w - targetWidth, w, 12, 1);
    glPopMatrix();
  }

  // red for attack target
  if(player && creature->getTargetCreature()) {
    double tw = ((double)creature->getTargetCreature()->getShape()->getWidth() / 2.0f) / GLShape::DIV;
    double td = (((double)(creature->getTargetCreature()->getShape()->getWidth()) / 2.0f) + 1.0f) / GLShape::DIV;
    //double td = ((double)(creature->getTargetCreature()->getShape()->getDepth())) / GLShape::DIV;
    glColor4f(1.0f, 0.15f, 0.0f, 0.5f);
    xpos2 = ((float)(creature->getTargetCreature()->getX() - levelMap->getX()) / GLShape::DIV);
    ypos2 = ((float)(creature->getTargetCreature()->getY() - levelMap->getY()) / GLShape::DIV);
    zpos2 = 0.0f / GLShape::DIV;  
    glPushMatrix();
    //glTranslatef( xpos2 + tw, ypos2 - tw * 2, zpos2 + 5);
    glTranslatef( xpos2 + tw, ypos2 - td, zpos2 + 5);
    gluDisk(quadric, tw - targetWidth, tw, 12, 1);
    glPopMatrix();
  }

  xpos2 = (creature->getX() - (float)(levelMap->getX())) / GLShape::DIV;
  ypos2 = (creature->getY() - (float)(levelMap->getY())) / GLShape::DIV;
  zpos2 = creature->getZ() / GLShape::DIV;  

  if(creature->getAction() != Constants::ACTION_NO_ACTION) {
    glColor4f(0, 0.7, 1, 0.5f);
  } else if(selected) {
    glColor4f(0, 1, 1, 0.5f);
  } else if(player) {
    glColor4f(0.0f, 1.0f, 0.0f, 0.5f);
  } else if(creature->getMonster() && creature->getMonster()->isNpc()) {
    glColor4f(0.75f, 1.0f, 0.0f, 0.5f);
  } else {
    glColor4f(0.7f, 0.7f, 0.7f, 0.25f);
  }

  // draw state mods
  if(groupMode || player || creature->isMonster()) {
    glEnable(GL_TEXTURE_2D);
    int n = 16;
    //float x = 0.0f;
    //float y = 0.0f;
    int on = 0;
    for(int i = 0; i < Constants::STATE_MOD_COUNT; i++) {
      if(creature->getStateMod(i) && i != Constants::dead) {
        on++;
      }
    }
    int count = 0;
    for(int i = 0; i < Constants::STATE_MOD_COUNT; i++) {
      if(creature->getStateMod(i) && i != Constants::dead) {

        /* DEBUG
        glPushMatrix();
        glTranslatef( xpos2, ypos2 - ( w * 2.0f ) - ( 1.0f / GLShape::DIV ), zpos2 + 5);
        glColor4f( 1, 1, 1, 1 );
        glBegin( GL_LINES );
        glVertex3f( 0, 0, 0 );
        glVertex3f( 0, w*2.0f, 0 );
        glVertex3f( 0, w*2.0f, 0 );
        glVertex3f( w*2.0f, w*2.0f, 0 );
        glVertex3f( w*2.0f, w*2.0f, 0 );
        glVertex3f( w*2.0f, 0, 0 );
        glVertex3f( w*2.0f, 0, 0 );
        glVertex3f( 0, 0, 0 );
        glVertex3f( 0, 0, 0 );
        glVertex3f( w*2.0f, w*2.0f, 0 );
        glVertex3f( 0, w*2.0f, 0 );
        glVertex3f( w*2.0f, 0, 0 );
        glEnd();
        glPopMatrix();
        */

        glPushMatrix();
        glTranslatef( xpos2 + w, 
                      ypos2 - ( w * 2.0f ) - ( 1.0f / GLShape::DIV ) + w, 
                      zpos2 + 5);
        float angle = -(count * 30) - (levelMap->getZRot() + 180);
        
        glRotatef( angle, 0, 0, 1 );
        glTranslatef( w + 15, 0, 0 );
        glRotatef( (count * 30) + 180, 0, 0, 1 );
        glTranslatef( -7, -7, 0 );

        GLuint icon = getShapePalette()->getStatModIcon(i);
        if(icon) {
          glBindTexture( GL_TEXTURE_2D, icon );
        }
        glBegin( GL_QUADS );
        glNormal3f( 0, 0, 1 );
        if(icon) glTexCoord2f( 0, 0 );
        glVertex3f( 0, 0, 0 );
        if(icon) glTexCoord2f( 0, 1 );
        glVertex3f( 0, n, 0 );
        if(icon) glTexCoord2f( 1, 1 );
        glVertex3f( n, n, 0 );
        if(icon) glTexCoord2f( 1, 0 );
        glVertex3f( n, 0, 0 );
        glEnd();
        glPopMatrix();
        count++;
      }
    }
    glDisable(GL_TEXTURE_2D);
  }

  //glTranslatef( xpos2 + w, ypos2 - w * 2, zpos2 + 5);
  glTranslatef( xpos2 + w, ypos2 - d, zpos2 + 5);
  if(groupMode || player || creature->isMonster()) 
    gluDisk(quadric, w - s, w, 12, 1);

  glEnable( GL_CULL_FACE );
  glDisable( GL_BLEND );
  glDisable( GL_DEPTH_TEST );
  glDepthMask(GL_TRUE);

  // draw name
  //glTranslatef( 0, 0, 100);
  //getSDLHandler()->texPrint(0, 0, "%s", creature->getName());

  glPopMatrix();
}

void Scourge::drawDraggedItem() {
  if(getMovingItem()) {
    glPushMatrix();
    glLoadIdentity();	
    glTranslatef( getSDLHandler()->mouseX, getSDLHandler()->mouseY, 500);
    glRotatef( getMap()->getXRot(), 0.0f, 1.0f, 0.0f );  
    glRotatef( getMap()->getYRot(), 1.0f, 0.0f, 0.0f );  
    glRotatef( getMap()->getZRot(), 0.0f, 0.0f, 1.0f );

    DrawLater later;
    later.xpos = 0;
    later.ypos = 0;
    later.zpos = 0;
    later.shape = getMovingItem()->getShape();
    later.creature = NULL;
    later.item = getMovingItem();
    later.projectile = NULL;
    later.effect = NULL;
    later.name = 0;
    later.pos = NULL;

    levelMap->doDrawShape( &later );
    glPopMatrix();
  }


  /*
  float xpos2, ypos2, zpos2;  
  Shape *shape = NULL;  

  if(selX >= getX() && selX < getX() + MAP_VIEW_WIDTH &&
	 selY >= getY() && selY < getY() + MAP_VIEW_DEPTH &&
	 selZ < MAP_VIEW_HEIGHT &&
	 scourge->getMovingItem()) {

	shape = scourge->getMovingItem()->getShape();	
	int newz = selZ;
	Location *dropLoc = isBlocked(selX, selY, selZ, -1, -1, -1, shape, &newz);
	selZ = newz;

	// only let drop on other creatures and containers
	// unfortunately I have to call isWallBetween(), so objects aren't dragged behind walls
	// this makes moving items slow
	if(dropLoc || 
	   (oldLocatorSelX < MAP_WIDTH && 
		isWallBetween(selX, selY, selZ, 
					  oldLocatorSelX, 
					  oldLocatorSelY, 
					  oldLocatorSelZ))) {
	  selX = oldLocatorSelX;
	  selY = oldLocatorSelY;
	  selZ = oldLocatorSelZ;
	}
	
	xpos2 = ((float)(selX - getX()) / GLShape::DIV);
	ypos2 = (((float)(selY - getY() - 1) - (float)shape->getDepth()) / GLShape::DIV);
	zpos2 = (float)(selZ) / GLShape::DIV;
	
	doDrawShape(xpos2, ypos2, zpos2, shape, 0);

	oldLocatorSelX = selX;
	oldLocatorSelY = selY;
	oldLocatorSelZ = selZ;
  }
  */
}

void Scourge::drawBorder() {
  if(levelMap->getViewWidth() == getSDLHandler()->getScreen()->w && 
     levelMap->getViewHeight() == getSDLHandler()->getScreen()->h &&
     !getUserConfiguration()->getFrameOnFullScreen()) return;

  glPushMatrix();
  glLoadIdentity();

  // ok change: viewx, viewy always 0
  //glTranslatef(viewX, viewY, 100);
  glTranslatef(0, 0, 100);

  //  glDisable(GL_BLEND);
  glDisable(GL_DEPTH_TEST);

  // draw border
  glColor4f( 1, 1, 1, 1);

  int w = (getMap()->getViewWidth() == getSDLHandler()->getScreen()->w ? 
           getMap()->getViewWidth() : 
           getMap()->getViewWidth() - Window::SCREEN_GUTTER);
  int h = (getMap()->getViewHeight() == getSDLHandler()->getScreen()->h ? 
           getMap()->getViewHeight() : 
           getMap()->getViewHeight() - Window::SCREEN_GUTTER);
  float TILE_W = 20.0f;
  float TILE_H = 120.0f;

  glBindTexture( GL_TEXTURE_2D, getShapePalette()->getBorderTexture() );
  glBegin( GL_QUADS );
  // left
  glTexCoord2f (0.0f, 0.0f);
  glVertex2i (0, 0);
  glTexCoord2f (0.0f, h/TILE_H);
  glVertex2i (0, h);
  glTexCoord2f (TILE_W/TILE_W, h/TILE_H);
  glVertex2i ((int)TILE_W, h);
  glTexCoord2f (TILE_W/TILE_W, 0.0f);      
  glVertex2i ((int)TILE_W, 0);

  // right
  glTexCoord2f (TILE_W/TILE_W, 0.0f);
  glVertex2i (w - (int)TILE_W, 0);
  glTexCoord2f (TILE_W/TILE_W, h/TILE_H);
  glVertex2i (w - (int)TILE_W, h);
  glTexCoord2f (0.0f, h/TILE_H);
  glVertex2i (w, h);
  glTexCoord2f (0.0f, 0.0f);      
  glVertex2i (w, 0);
  glEnd();

  TILE_W = 120.0f;
  TILE_H = 20.0f;
  glBindTexture( GL_TEXTURE_2D, getShapePalette()->getBorder2Texture() );
  glBegin( GL_QUADS );
  // top
  glTexCoord2f (0.0f, 0.0f);
  glVertex2i (0, 0);
  glTexCoord2f (0.0f, TILE_H/TILE_H);
  glVertex2i (0, (int)TILE_H);
  glTexCoord2f (w/TILE_W, TILE_H/TILE_H);
  glVertex2i (w, (int)TILE_H);
  glTexCoord2f (w/TILE_W, 0.0f);      
  glVertex2i (w, 0);

  // bottom
  glTexCoord2f (w/TILE_W, TILE_H/TILE_H);
  glVertex2i (0, h - (int)TILE_H);
  glTexCoord2f (w/TILE_W, 0.0f);
  glVertex2i (0, h);
  glTexCoord2f (0.0f, 0.0f);
  glVertex2i (w, h);
  glTexCoord2f (0.0f, TILE_H/TILE_H);      
  glVertex2i (w, h - (int)TILE_H);
  glEnd();

  //int gw = 128;
  //int gh = 96;

  //int gw = 115;
  //int gh = 81;

  int gw = 240;
  int gh = 64;

  glEnable( GL_ALPHA_TEST );
  glAlphaFunc( GL_NOTEQUAL, 0 );
  glBindTexture( GL_TEXTURE_2D, getShapePalette()->getGargoyleTexture() );

  /*
  glPushMatrix();
  glLoadIdentity();
  glTranslatef(10, -5, 0);
  glRotatef(20, 0, 0, 1);
  glBegin( GL_QUADS );
  // top left
  glTexCoord2f (1, 0);
  glVertex2i (0, 0);
  glTexCoord2f (1, 1);
  glVertex2i (0, gh);
  glTexCoord2f (0, 1);
  glVertex2i (gw, gh);
  glTexCoord2f (0, 0);      
  glVertex2i (gw, 0);
  glEnd();
  glPopMatrix();

  // top right
  glPushMatrix();
  glLoadIdentity();
  glTranslatef(w - (gw + 7), 35, 0);
  glRotatef(-20, 0, 0, 1);
  glBegin( GL_QUADS );
  glTexCoord2f (0, 0);
  glVertex2i (0, 0);
  glTexCoord2f (0, 1);
  glVertex2i (0, gh);
  glTexCoord2f (1, 1);
  glVertex2i (gw, gh);
  glTexCoord2f (1, 0);      
  glVertex2i (gw, 0);
  glEnd();
  */

  glPushMatrix();
  glLoadIdentity();
  //glTranslatef(10, -5, 0);
  //glRotatef(20, 0, 0, 1);
  glBegin( GL_QUADS );
  // top left
  glTexCoord2f (0, 0);
  glVertex2i (0, 0);
  glTexCoord2f (0, 1);
  glVertex2i (0, gh);
  glTexCoord2f (1, 1);
  glVertex2i (gw, gh);
  glTexCoord2f (1, 0);      
  glVertex2i (gw, 0);
  glEnd();
  glPopMatrix();

  // top right
  glPushMatrix();
  glLoadIdentity();
  glTranslatef(w - gw, 0, 0);
  //glRotatef(-20, 0, 0, 1);
  glBegin( GL_QUADS );
  glTexCoord2f (1, 0);
  glVertex2i (0, 0);
  glTexCoord2f (1, 1);
  glVertex2i (0, gh);
  glTexCoord2f (0, 1);
  glVertex2i (gw, gh);
  glTexCoord2f (0, 0);      
  glVertex2i (gw, 0);
  glEnd();

  glPopMatrix();

  //glEnable( GL_TEXTURE_2D );
  glDisable( GL_ALPHA_TEST );
  glEnable(GL_DEPTH_TEST);
  glPopMatrix();
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
    //    return false;
  }

  //if(multiplayer->isVisible()) {
//    multiplayer->handleEvent(event);
  //return false;
  //}

  int mx, my;
  switch(event->type) {
  case SDL_MOUSEMOTION:
    if(mouseRot) {
      levelMap->setZRot(-event->motion.xrel * MOUSE_ROT_DELTA);
      levelMap->setYRot(-event->motion.yrel * MOUSE_ROT_DELTA);
    } else {
      //sdlHandler->applyMouseOffset(event->motion.x, event->motion.y, &mx, &my);
      mx = event->motion.x;
      my = event->motion.y;
      if(mx < 10) {
        mouseMoveScreen = true;
        setMove(Constants::MOVE_LEFT);
      } else if(mx >= sdlHandler->getScreen()->w - 10) {
        mouseMoveScreen = true;
        setMove(Constants::MOVE_RIGHT);
      } else if(my < 10) {
        mouseMoveScreen = true;
        setMove(Constants::MOVE_UP);
      } else if(my >= sdlHandler->getScreen()->h - 10) {
        mouseMoveScreen = true;
        setMove(Constants::MOVE_DOWN);
      } else {
        if(mouseMoveScreen) {
          mouseMoveScreen = false;
          removeMove(Constants::MOVE_LEFT | Constants::MOVE_RIGHT);
          removeMove(Constants::MOVE_UP | Constants::MOVE_DOWN);
          levelMap->setYRot(0.0f);
          levelMap->setZRot(0.0f);
        }
      }

      // start the item drag
      if(willStartDrag && 
         (abs(mx - willStartDragX) > DRAG_START_TOLERANCE ||
          abs(my - willStartDragY) > DRAG_START_TOLERANCE)) {
        // click on an item
        Uint16 mapx = cursorMapX;
        Uint16 mapy = cursorMapY;
        Uint16 mapz = cursorMapZ;
        if(mapx > MAP_WIDTH) {
          getMapXYAtScreenXY(willStartDragX, willStartDragY, &mapx, &mapy);
          mapz = 0;
        }
        startItemDrag(mapx, mapy, mapz);
        willStartDrag = false;
      }

      processGameMouseMove(mx, my);  
    }
    break;
  case SDL_MOUSEBUTTONDOWN:
    if(event->button.button) {
      processGameMouseDown(sdlHandler->mouseX, sdlHandler->mouseY, event->button.button);
    }
    break;  
  case SDL_MOUSEBUTTONUP:
    if(event->button.button) {
      processGameMouseClick(sdlHandler->mouseX, sdlHandler->mouseY, event->button.button);
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
  }
  switch(event->type) {
  case SDL_KEYDOWN:

    // DEBUG ------------------------------------

#ifdef DEBUG_KEYS
    if(event->key.keysym.sym == SDLK_l) {
      cerr << "Lightmap is now=" << getMap()->toggleLightMap() << endl;
      return false;
    } else if(event->key.keysym.sym == SDLK_f) {
      getMap()->useFrustum = ( getMap()->useFrustum ? false : true );
      getMap()->refresh();
    } else if(event->key.keysym.sym == SDLK_r && 
              getSession()->getCurrentMission() && 
              !getSession()->getCurrentMission()->isCompleted()) {
      getSession()->getCurrentMission()->setCompleted( true );
      if( getSession()->getCurrentMission()->isStoryLine() )
        board->storylineMissionCompleted( getSession()->getCurrentMission() );
      missionCompleted();
    } else if( event->key.keysym.sym == SDLK_t ) {
      teleport();
    } else if(event->key.keysym.sym == SDLK_u) {
      cerr << "EFFECT!" << endl;
//      party->startEffect( Constants::EFFECT_CAST_SPELL, (Constants::DAMAGE_DURATION * 4));

      levelMap->startEffect( toint(party->getPlayer()->getX()), 
                             toint(party->getPlayer()->getY()), 1, 
                             Constants::EFFECT_RING, (Constants::DAMAGE_DURATION * 4),
                             12, 12 );

    } else if(event->key.keysym.sym == SDLK_m) {
      Map::debugMd2Shapes = ( Map::debugMd2Shapes ? false : true );
      return false;
    } else if(event->key.keysym.sym == SDLK_b) {
      Battle::debugBattle = ( Battle::debugBattle ? false : true );
      return false;
    }
#endif

    // END OF DEBUG ------------------------------------

  case SDL_KEYUP:

    if(event->type == SDL_KEYUP && event->key.keysym.sym == SDLK_ESCAPE){
      if( inventory->inStoreSpellMode() ) {
        inventory->setStoreSpellMode( false );
        return false;
      } else if( getTargetSelectionFor() ) {
        // cancel target selection ( cross cursor )
        getTargetSelectionFor()->cancelTarget();
        setTargetSelectionFor( NULL );
        return false;
      } else if( exitConfirmationDialog->isVisible() ) {
        exitConfirmationDialog->setVisible(false);
      } else {
        party->toggleRound(true);
        exitConfirmationDialog->setVisible(true);
      }   
      return false;
    } else if( event->type == SDL_KEYUP && event->key.keysym.sym == SDLK_BACKSPACE ) {
      SDLHandler::showDebugInfo = ( SDLHandler::showDebugInfo ? 0 : 1 );
    }

    // xxx_yyy_stop means : "do xxx_yyy action when the corresponding key is up"
    ea = userConfiguration->getEngineAction(event);    
    if(ea == SWITCH_COMBAT) {
      getUserConfiguration()->setBattleTurnBased( getUserConfiguration()->isBattleTurnBased() ? false : true );
      char message[80];
      sprintf( message, "Combat is now %s.", ( getUserConfiguration()->isBattleTurnBased() ?
                                               "Turn-based" :
                                               "Real-time" ) );
      levelMap->addDescription( message, 0, 1, 1 );
    } else if(ea == SET_MOVE_DOWN){        
      setMove(Constants::MOVE_DOWN);
    } else if(ea == SET_MOVE_UP){
      setMove(Constants::MOVE_UP);
    } else if(ea == SET_MOVE_RIGHT){
      setMove(Constants::MOVE_RIGHT);
    } else if(ea == SET_MOVE_LEFT){
      setMove(Constants::MOVE_LEFT);
    } else if(ea == SET_MOVE_DOWN_STOP){        
      levelMap->setYRot(0.0f);
      levelMap->setYRot(0);
      removeMove(Constants::MOVE_DOWN);
    } else if(ea == SET_MOVE_UP_STOP){
      levelMap->setYRot(0.0f);
      levelMap->setYRot(0);
      removeMove(Constants::MOVE_UP);
    } else if(ea == SET_MOVE_RIGHT_STOP){
      levelMap->setYRot(0.0f);
      levelMap->setZRot(0);
      removeMove(Constants::MOVE_RIGHT);
    } else if(ea == SET_MOVE_LEFT_STOP){
      levelMap->setYRot(0.0f);
      levelMap->setZRot(0);
      removeMove(Constants::MOVE_LEFT);
    } else if(ea == SET_PLAYER_0){
      setPlayer(0);
    } else if(ea == SET_PLAYER_1){
      setPlayer(1);
    } else if(ea == SET_PLAYER_2){
      setPlayer(2);
    } else if(ea == SET_PLAYER_3){
      setPlayer(3);
    } else if(ea == SET_PLAYER_ONLY && !inTurnBasedCombat()) {
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
    } else if(ea == SHOW_INVENTORY){
      toggleInventoryWindow();
    } else if(ea == SHOW_OPTIONS_MENU){
      toggleOptionsWindow();
    } else if(ea == SET_NEXT_FORMATION_STOP){
      if(party->getFormation() < Creature::FORMATION_COUNT - 1) party->setFormation(party->getFormation() + 1);
      else party->setFormation(Constants::DIAMOND_FORMATION - Constants::DIAMOND_FORMATION);
    } else if(ea == MINIMAP_ZOOM_IN){
      miniMap->zoomIn();
    } else if(ea == MINIMAP_ZOOM_OUT){
      miniMap->zoomOut();
//    } else if(ea == MINIMAP_TOGGLE){
//      miniMap->toggle();
    } else if(ea == SET_ZOOM_IN){
      levelMap->setZoomIn(true);
    } else if(ea == SET_ZOOM_OUT){
      levelMap->setZoomOut(true);
    } else if(ea == SET_ZOOM_IN_STOP){
      levelMap->setZoomIn(false);
    } else if(ea == SET_ZOOM_OUT_STOP){
      levelMap->setZoomOut(false);
    } else if(ea == TOGGLE_MAP_CENTER){
      bool mc;
      mc = userConfiguration->getAlwaysCenterMap();
      userConfiguration->setAlwaysCenterMap(!mc);
    } else if(ea == INCREASE_GAME_SPEED){
      addGameSpeed(-1);        
    } else if(ea == DECREASE_GAME_SPEED){
      addGameSpeed(1);        
    } else if(ea == START_ROUND) {
      party->toggleRound();
    } else if(ea == LAYOUT_1) {
      setUILayout(Constants::GUI_LAYOUT_ORIGINAL);
    } else if(ea == LAYOUT_2) {
      setUILayout(Constants::GUI_LAYOUT_BOTTOM);
//    } else if(ea == LAYOUT_3) {
//      setUILayout(Constants::GUI_LAYOUT_SIDE);
    } else if(ea == LAYOUT_4) {
      setUILayout(Constants::GUI_LAYOUT_INVENTORY);
    } else if( ea >= QUICK_SPELL_1 && ea <= QUICK_SPELL_12 ) {
      quickSpellAction( ea - QUICK_SPELL_1 );
    }
    break;
  default: break;
  }

  return false;  
}

void Scourge::processGameMouseMove(Uint16 x, Uint16 y) {
  Uint16 mapx, mapy;
  getMapXYAtScreenXY(x, y, &mapx, &mapy);
  if(mapx < MAP_WIDTH) levelMap->handleMouseMove( mapx, mapy, 0 );
}

void Scourge::processGameMouseDown(Uint16 x, Uint16 y, Uint8 button) {
  if(button == SDL_BUTTON_LEFT) {
    // will start to drag when the mouse has moved
    willStartDrag = true;
    willStartDragX = x;
    willStartDragY = y;
  } if(button == SDL_BUTTON_MIDDLE) {
    mouseRot = true;
  } if(button == SDL_BUTTON_WHEELUP) {
    mouseZoom = true;
    levelMap->setZoomIn(false);
    levelMap->setZoomOut(true);
  } if(button == SDL_BUTTON_WHEELDOWN) {
    mouseZoom = true;
    levelMap->setZoomIn(true);
    levelMap->setZoomOut(false);
  }
}

void Scourge::processGameMouseClick(Uint16 x, Uint16 y, Uint8 button) {
  // don't drag if you haven't started yet
  willStartDrag = false;

  Uint16 mapx, mapy, mapz;
  //Creature *c = getTargetSelectionFor();
  if(button == SDL_BUTTON_MIDDLE) {
    mouseRot = false;
    levelMap->setXRot(0);
    levelMap->setYRot(0);
    levelMap->setZRot(0);
  } else if(button == SDL_BUTTON_LEFT) {

    mapx = cursorMapX;
    mapy = cursorMapY;
    mapz = cursorMapZ;

    // drop target?
    Location *dropTarget = NULL;
    if(movingItem) {
      if(mapx < MAP_WIDTH) {
        dropTarget = levelMap->getLocation(mapx, mapy, mapz);
        if(!(dropTarget && 
             (dropTarget->creature || 
              (dropTarget->item && 
               dropTarget->item->getRpgItem()->getType() == RpgItem::CONTAINER)))) {
          dropTarget = NULL;
        }      
      }
    }
    levelMap->setSelectedDropTarget(dropTarget);

    // clicking on a creature
    if(!movingItem && mapx < MAP_WIDTH) {
      Location *loc = levelMap->getLocation(mapx, mapy, mapz);
      if(loc && loc->creature) {
        if(getTargetSelectionFor()) {
          handleTargetSelectionOfCreature( loc->creature );
          return;
        } else if(loc->creature->isMonster()) {
          if( loc->creature->getMonster()->isNpc() ) {
            // start a conversation
            conversationGui->start( loc->creature );
          } else {
            // follow this creature
            party->setTargetCreature(loc->creature);
            // show path
            if( inTurnBasedCombatPlayerTurn() ) {
              battleRound[battleTurn]->getCreature()->findPath( mapx, mapy );
              // start round
              if( getSDLHandler()->isDoubleClick ) {
                party->toggleRound( false );
              }
            }
          }
          return;
        } else {
          // select player
          for(int i = 0; i < party->getPartySize(); i++) {
            if(party->getParty(i) == loc->creature) {
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

    if( getTargetSelectionFor() ) {
      Location *pos = levelMap->getLocation(mapx, mapy, mapz);
      if(mapx < MAP_WIDTH && pos && pos->item) {
        handleTargetSelectionOfItem( pos->item, pos->x, pos->y, pos->z );
        return;
      }
    }

    if(useItem(mapx, mapy, mapz)) return;

    // click on the levelMap
    getMapXYAtScreenXY(x, y, &mapx, &mapy);

    // make sure the selected action can target a location
    if( getTargetSelectionFor() ) {
      handleTargetSelectionOfLocation( mapx, mapy, mapz );
     return;
    }


    
    // Make party move to new location
    party->setSelXY( mapx, mapy );

    // start round
    if( inTurnBasedCombatPlayerTurn() ) {
      battleRound[battleTurn]->getCreature()->findPath( mapx, mapy );
      if( getSDLHandler()->isDoubleClick ) {
        party->toggleRound( false );
      }
    }

  } else if(button == SDL_BUTTON_RIGHT) {
    describeLocation(cursorMapX, cursorMapY, cursorMapZ);
  }
}

bool Scourge::handleTargetSelectionOfLocation( Uint16 mapx, Uint16 mapy, Uint16 mapz ) {
  bool ret = false;
  char msg[80];
  Creature *c = getTargetSelectionFor();
  if(c->getAction() == Constants::ACTION_CAST_SPELL &&
     c->getActionSpell() &&
     c->getActionSpell()->isLocationTargetAllowed()) {

    // assign this creature
    c->setTargetLocation(mapx, mapy, 0);
    char msg[80];
    sprintf(msg, "%s selected a target", c->getName());
    levelMap->addDescription(msg);       
    ret = true;
  } else {
    sprintf(msg, "%s cancelled a pending action.", c->getName());
    levelMap->addDescription(msg);
  }
  // turn off selection mode
  setTargetSelectionFor(NULL);
  return ret;
}

bool Scourge::handleTargetSelectionOfCreature( Creature *potentialTarget ) {
  bool ret = false;
  char msg[80];
  Creature *c = getTargetSelectionFor();
  // make sure the selected action can target a creature
  if(c->getAction() == Constants::ACTION_CAST_SPELL &&
     c->getActionSpell() &&
     c->getActionSpell()->isCreatureTargetAllowed()) {
    
    // assign this creature
    c->setTargetCreature( potentialTarget );
    char msg[80];
    sprintf(msg, "%s will target %s", c->getName(), c->getTargetCreature()->getName());
    levelMap->addDescription(msg);       
    ret = true;
  } else {
    sprintf(msg, "%s cancelled a pending action.", c->getName());
    levelMap->addDescription(msg);
  }
  // turn off selection mode
  setTargetSelectionFor(NULL);
  return ret;
}

bool Scourge::handleTargetSelectionOfItem( Item *item, int x, int y, int z ) {
  bool ret = false;
  char msg[80];  
  // make sure the selected action can target an item
  Creature *c = getTargetSelectionFor();
  if(c->getAction() == Constants::ACTION_CAST_SPELL &&
     c->getActionSpell() &&
     c->getActionSpell()->isItemTargetAllowed()) {
    
    // assign this creature
    c->setTargetItem( x, y, z, item );
    sprintf( msg, "%s targeted %s.", c->getName(), item->getRpgItem()->getName() );
    levelMap->addDescription( msg );
    ret = true;
  } else {
    sprintf( msg, "%s cancelled a pending action.", c->getName() );
    levelMap->addDescription( msg );
  }
  // turn off selection mode
  setTargetSelectionFor( NULL );
  return ret;
}

void Scourge::describeLocation(int mapx, int mapy, int mapz) {
  char s[300];
  if(mapx < MAP_WIDTH) {
    //fprintf(stderr, "\tclicked map coordinates: x=%u y=%u z=%u\n", mapx, mapy, mapz);
    Location *loc = levelMap->getPosition(mapx, mapy, mapz);
    if(loc) {
      char *description = NULL;
      Creature *creature = loc->creature;
      //fprintf(stderr, "\tcreature?%s\n", (creature ? "yes" : "no"));
      if(creature) {
        creature->getDetailedDescription(s);
        description = s;
      } else {
        Item *item = loc->item;
        //fprintf(stderr, "\titem?%s\n", (item ? "yes" : "no"));
        if( item ) {
          //item->getDetailedDescription(s, false);
          //description = s;
          infoGui->setItem( item, getParty()->getPlayer()->getSkill(Constants::IDENTIFY_ITEM_SKILL) );
          if(!infoGui->getWindow()->isVisible()) infoGui->getWindow()->setVisible( true );
        } else {
          Shape *shape = loc->shape;
          //fprintf(stderr, "\tshape?%s\n", (shape ? "yes" : "no"));
          if(shape) {
            description = session->getShapePalette()->getRandomDescription(shape->getDescriptionGroup());
          }        
        }
      }
      if(description) {
        levelMap->addDescription(description);
      }
    }
  }
} 

void Scourge::getMapXYAtScreenXY(Uint16 x, Uint16 y,
                                 Uint16 *mapx, Uint16 *mapy) {
  glPushMatrix();
  
  // Initialize the scene w/o y rotation.
  levelMap->initMapView(true);
  
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
  
  glDisable( GL_SCISSOR_TEST );
  
  if(res) {
    *mapx = levelMap->getX() + (Uint16)(((obj_x) * GLShape::DIV)) - 1;
    *mapy = levelMap->getY() + (Uint16)(((obj_y) * GLShape::DIV)) + 2;
    //*mapz = (Uint16)0;
    //*mapz = (Uint16)(obj_z * GLShape::DIV);
    levelMap->debugX = *mapx;
    levelMap->debugY = *mapy;
    levelMap->debugZ = 0;
  } else {
    //*mapx = *mapy = *mapz = MAP_WIDTH + 1;
    *mapx = *mapy = MAP_WIDTH + 1;
  }
  glPopMatrix();
}   

void Scourge::getMapXYZAtScreenXY(Uint16 x, Uint16 y,
                                  Uint16 *mapx, Uint16 *mapy, Uint16 *mapz) {
  /*
  // only do this if the mouse has moved some (optimization)
  if(abs(lastX - x) < POSITION_SAMPLE_DELTA && abs(lastY - y) < POSITION_SAMPLE_DELTA) {
    *mapx = lastMapX;
    *mapy = lastMapY;
    *mapz = lastMapZ;
    return;
  }
  */

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
  levelMap->selectMode = true;
  levelMap->draw();
  levelMap->selectMode = false;

  glFlush();    
  hits = glRenderMode(GL_RENDER);
  //cerr << "hits=" << hits << endl;
  if(hits > 0) {           // If There Were More Than 0 Hits
    int choose = buffer[4];         // Make Our Selection The First Object
    int depth = buffer[1];          // Store How Far Away It Is

    for(int loop = 0; loop < hits; loop++) {   // Loop Through All The Detected Hits

      //            fprintf(stderr, "\tloop=%d 0=%u 1=%u 2=%u 3=%u 4=%u \n", loop, 
      //                    buffer[loop*5+0], buffer[loop*5+1], buffer[loop*5+2], 
      //                    buffer[loop*5+3],  buffer[loop*5+4]);
      if(buffer[loop*5+4] > 0) {
        decodeName(buffer[loop*5+4], mapx, mapy, mapz);
      }

      // If This Object Is Closer To Us Than The One We Have Selected
      if(buffer[loop*5+1] < GLuint(depth)) {
        choose = buffer[loop*5+4];        // Select The Closer Object
        depth = buffer[loop*5+1];     // Store How Far Away It Is
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

  levelMap->debugX = *mapx;
  levelMap->debugY = *mapy;
  levelMap->debugZ = *mapz;
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
        Location *pos = levelMap->getPosition(*mapx, *mapy, 0);
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
  Location *pos = levelMap->getPosition(x, y, z);
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

bool Scourge::useItem(int x, int y, int z) {
  if (movingItem) {
    dropItem(levelMap->getSelX(), levelMap->getSelY());
    return true;
  }

  Location *pos = levelMap->getPosition(x, y, z);
  if (pos) {
    Shape *shape = (pos->item ? pos->item->getShape() : pos->shape);
    if (levelMap->isWallBetweenShapes(toint(party->getPlayer()->getX()), 
                                 toint(party->getPlayer()->getY()), 
                                 toint(party->getPlayer()->getZ()), 
                                 party->getPlayer()->getShape(),
                                 x, y, z,
                                 shape)) {
      levelMap->addDescription(Constants::getMessage(Constants::ITEM_OUT_OF_REACH));
      return true;
    } else {
      if (useLever(pos)) {
        return true;
      } else if (useDoor(pos)) {
        return true;
      } else if (useGate(pos)) {
        return true;
      } else if (useBoard(pos)) {
        return true;
      } else if (useTeleporter(pos)) {
        return true;
      } else if(pos && pos->item && pos->item->getRpgItem()->getType() == RpgItem::CONTAINER) {
        openContainerGui(pos->item);      
        return true;
      }
    }
  }
  return false;
}

bool Scourge::getItem(Location *pos) {
    if(pos->item) {
      if(levelMap->isWallBetween(pos->x, pos->y, pos->z, 
                            toint(party->getPlayer()->getX()),
                            toint(party->getPlayer()->getY()),
                            0)) {
		levelMap->addDescription(Constants::getMessage(Constants::ITEM_OUT_OF_REACH));
	  } else {
        movingX = pos->x;
        movingY = pos->y;
        movingZ = pos->z;
        movingItem = pos->item;		
		int x = pos->x;
		int y = pos->y;
		int z = pos->z;
        levelMap->removeItem(pos->x, pos->y, pos->z);
		levelMap->dropItemsAbove(x, y, z, movingItem);
		// draw the item as 'selected'
		levelMap->setSelectedDropTarget(NULL);
		levelMap->handleMouseMove(movingX, movingY, movingZ);
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

int Scourge::dropItem(int x, int y) {
  int z=-1;
  bool replace = false;
  if(levelMap->getSelectedDropTarget()) {
    char message[120];
    Creature *c = levelMap->getSelectedDropTarget()->creature;
    if(c) {
      if(c->addInventory(movingItem)) {
        sprintf(message, "%s picks up %s.", 
                c->getName(), 
                movingItem->getItemName());
        levelMap->addDescription(message);
        // if the inventory is open, update it
        if(inventory->isVisible()) inventory->refresh();
      } else {
        showMessageDialog("The item won't fit in that container!");
        replace = true;
      }
    } else if(levelMap->getSelectedDropTarget()->item && 
              levelMap->getSelectedDropTarget()->item->getRpgItem()->getType() == RpgItem::CONTAINER) {
      if(!levelMap->getSelectedDropTarget()->item->addContainedItem(movingItem)) {
        showMessageDialog("The item won't fit in that container!");
        replace = true;
      } else {
        sprintf(message, "%s is placed in %s.", 
                movingItem->getItemName(), 
                levelMap->getSelectedDropTarget()->item->getItemName());
        levelMap->addDescription(message);
        // if this container's gui is open, update it
        refreshContainerGui(levelMap->getSelectedDropTarget()->item);
      }
    } else {
      replace = true;
    }
    levelMap->setSelectedDropTarget(NULL);
  } else {
    // see if it's blocked and get the value of z (stacking items)
    Location *pos = levelMap->isBlocked(x, y, 0,
                                   movingX, movingY, movingZ,
                                   movingItem->getShape(), &z);
    if(!pos && 
       !levelMap->isWallBetween(toint(party->getPlayer()->getX()), 
                           toint(party->getPlayer()->getY()), 
                           toint(party->getPlayer()->getZ()), 
                           x, y, z)) {
      levelMap->setItem(x, y, z, movingItem);
    } else {
      replace = true;
    }
  }

  // failed to drop item; put it back to where we got it from
  if(replace) {
    if(movingX <= -1 || movingX >= MAP_WIDTH) {
      // the item drag originated from the gui... what to do?
      // for now don't end the drag
      return z;
    } else {
      levelMap->isBlocked(movingX, movingY, movingZ,
                     -1, -1, -1,
                     movingItem->getShape(), &z);
      levelMap->setItem(movingX, movingY, z, movingItem);
    }
  }
  endItemDrag();
  getSDLHandler()->getSound()->playSound(Window::DROP_SUCCESS);
  return z;
}

bool Scourge::useGate(Location *pos) {
  for(int i = 0; i < party->getPartySize(); i++) {
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
    boardWin->setVisible(true);
    return true;
  }
  return false;
}

bool Scourge::useTeleporter(Location *pos) {
  if(pos->shape == shapePal->findShapeByName("TELEPORTER") ||
     pos->shape == shapePal->findShapeByName("TELEPORTER_BASE")) {
    if(levelMap->isLocked(pos->x, pos->y, pos->z)) {
      levelMap->addDescription(Constants::getMessage(Constants::TELEPORTER_OFFLINE));
      return true;
    } else {
      // able to teleport if any party member is alive
      for(int i = 0; i < 4; i++) {
        if(!party->getParty(i)->getStateMod(Constants::dead)) {
          teleporting = true;
          return true;
        }
      }
    }
  }
  return false;
}

bool Scourge::useLever(Location *pos) {
  Shape *newShape = NULL;
  if(pos->shape == shapePal->findShapeByName("SWITCH_OFF")) {
    newShape = shapePal->findShapeByName("SWITCH_ON");
  } else if(pos->shape == shapePal->findShapeByName("SWITCH_ON")) {
    newShape = shapePal->findShapeByName("SWITCH_OFF");
  }
  if(newShape) {
    int keyX = pos->x;
    int keyY = pos->y;
    int keyZ = pos->z;
    int doorX, doorY, doorZ;
    levelMap->getDoorLocation(keyX, keyY, keyZ,
                         &doorX, &doorY, &doorZ);
    // flip the switch
    levelMap->setPosition(keyX, keyY, keyZ, newShape);
    // unlock the door
    levelMap->setLocked(doorX, doorY, doorZ, (levelMap->isLocked(doorX, doorY, doorZ) ? false : true));
    // show message, depending on distance from key to door
    float d = Constants::distance(keyX,  keyY, 1, 1, doorX, doorY, 1, 1);
    if(d < 20.0f) {
      levelMap->addDescription(Constants::getMessage(Constants::DOOR_OPENED_CLOSE));
    } else if(d >= 20.0f && d < 100.0f) {
      levelMap->addDescription(Constants::getMessage(Constants::DOOR_OPENED));
    } else {
      levelMap->addDescription(Constants::getMessage(Constants::DOOR_OPENED_FAR));
    }
    return true;
  }
  return false;
}

bool Scourge::useDoor(Location *pos) {
  Shape *newDoorShape = NULL;
  Shape *oldDoorShape = pos->shape;
  if(oldDoorShape == shapePal->findShapeByName("EW_DOOR")) {
	newDoorShape = shapePal->findShapeByName("NS_DOOR");
  } else if(oldDoorShape == shapePal->findShapeByName("NS_DOOR")) {
	newDoorShape = shapePal->findShapeByName("EW_DOOR");
  }
  if(newDoorShape) {
	int doorX = pos->x;
	int doorY = pos->y;
	int doorZ = pos->z;
    
	// see if the door is open or closed. This is done by checking the shape above the
	// door. If there's something there and the orientation (NS vs. EW) matches, the
	// door is closed. I know it's a hack.
	Location *above = levelMap->getLocation(doorX, 
											doorY, 
											doorZ + pos->shape->getHeight());
	//if(above && above->shape) cerr << "ABOVE: shape=" << above->shape->getName() << endl;
	//else cerr << "Nothing above!" << endl;
	bool closed = ((pos->shape == shapePal->findShapeByName("EW_DOOR") &&
					above && above->shape == shapePal->findShapeByName("EW_DOOR_TOP")) ||
				   (pos->shape == shapePal->findShapeByName("NS_DOOR") &&
					above && above->shape == shapePal->findShapeByName("NS_DOOR_TOP")) ?
				   true : false);
	//cerr << "DOOR is closed? " << closed << endl;
	if(closed && levelMap->isLocked(doorX, doorY, doorZ)) {
	  levelMap->addDescription(Constants::getMessage(Constants::DOOR_LOCKED));
	  return true;
	}
	
	// switch door
	Sint16 ox = pos->x;
	Sint16 oy = pos->y;
	Sint16 nx = pos->x;
	Sint16 ny = (pos->y - pos->shape->getDepth()) + newDoorShape->getDepth();
	
	Shape *oldDoorShape = levelMap->removePosition(ox, oy, toint(party->getPlayer()->getZ()));
	Location *blocker = levelMap->isBlocked(nx, ny, toint(party->getPlayer()->getZ()),
											ox, oy, toint(party->getPlayer()->getZ()),
											newDoorShape);
	if( !blocker ) {

	  // there is a chance that the door will be destroyed
	  if( 0 == (int)( 10.0f * rand()/RAND_MAX ) ) {
		destroyDoor( ox, oy, oldDoorShape );
		levelMap->updateLightMap();
	  } else {
		levelMap->setPosition(nx, ny, toint(party->getPlayer()->getZ()), newDoorShape);
		levelMap->updateLightMap();          
		levelMap->updateDoorLocation(doorX, doorY, doorZ,
									 nx, ny, toint(party->getPlayer()->getZ()));
	  }
	  return true;
	} else if( blocker->creature && blocker->creature->isMonster() ) {
	  // rollback if blocked by a player
	  levelMap->setPosition(ox, oy, toint(party->getPlayer()->getZ()), oldDoorShape);
	  levelMap->addDescription(Constants::getMessage(Constants::DOOR_BLOCKED));
	  return true;
	} else {
	  // Deeestroy!
	  destroyDoor( ox, oy, oldDoorShape );
	  levelMap->updateLightMap();
	  return true;
	}
  }
  return false;
}

void Scourge::destroyDoor( Sint16 ox, Sint16 oy, Shape *shape ) {
  levelMap->addDescription( "The door splinters into many, tiny pieces!", 0, 1, 1 );
  for( int i = 0; i < 8; i++ ) {
	int x = ox + (int)( (float)( shape->getWidth() ) * rand() / RAND_MAX );
	int y = oy - (int)( (float)( shape->getDepth() ) * rand() / RAND_MAX );
	int z = 2 + (int)(( ( (float)( shape->getHeight() ) / 2.0f ) - 2.0f ) * rand() / RAND_MAX );
//	cerr << "starting effects at: " << x << "," << y << "," << z << endl;
	levelMap->startEffect( x, y, z, Constants::EFFECT_DUST, 
						   (GLuint)( (float)Constants::DAMAGE_DURATION / 2.0f ), 2, 2,
						   (GLuint)((float)(i) / 4.0f * (float)Constants::DAMAGE_DURATION) );
  }
}

void Scourge::toggleInventoryWindow() {
  if(inventory->isVisible()) {
    //if(!inventory->getWindow()->isLocked()) inventory->hide();
    inventory->hide();
  } else {
    inventory->show();
  }
  inventoryButton->setSelected( inventory->isVisible() );
}

void Scourge::toggleOptionsWindow() {
  if(optionsMenu->isVisible()) {
    optionsMenu->hide();
  } else {
    //	party->toggleRound(true);
	optionsMenu->show();
  }
  optionsButton->setSelected( optionsMenu->isVisible() );
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
    //	return false;
  }
  
  if(netPlay->getWindow()->isVisible()) {
    netPlay->handleEvent(widget, event);
  }
  
  //if(multiplayer->isVisible()) {
  //    multiplayer->handleEvent(widget, event);
  //return false;
  //}
  
  if(infoGui->getWindow()->isVisible()) {
    infoGui->handleEvent(widget, event);
  }

  if( conversationGui->getWindow()->isVisible() ) {
    conversationGui->handleEvent( widget, event );
  }

  // FIXME: this is hacky...
  if(handlePartyEvent(widget, event)) return true;
  int n = handleBoardEvent(widget, event);
  if(n == Board::EVENT_HANDLED) return false;
  else if(n == Board::EVENT_PLAY_MISSION) {
    int selected = missionList->getSelectedLine();
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

  infoGui = new InfoGui( this );

  conversationGui = new ConversationGui( this );

  int width = 
    getSDLHandler()->getScreen()->w - 
    (PARTY_GUI_WIDTH + (Window::SCREEN_GUTTER * 2));
  messageWin = new Window( getSDLHandler(),
                           0, 0, width, PARTY_GUI_HEIGHT, 
                           "Messages", 
                           getShapePalette()->getGuiTexture(), false,
                           Window::BASIC_WINDOW,
                           getShapePalette()->getGuiTexture2() );
  messageWin->setBackground(0, 0, 0);
  messageList = new ScrollingList(0, 0, width, PARTY_GUI_HEIGHT - 25, getShapePalette()->getHighlightTexture());
  messageList->setSelectionColor( 0.15f, 0.15f, 0.3f );
  messageList->setCanGetFocus( false );
  messageWin->addWidget(messageList);
  // this has to be after addWidget
  messageList->setBackground( 1, 0.75f, 0.45f );
  messageList->setSelectionColor( 0.25f, 0.25f, 0.25f );

  // FIXME: try to encapsulate this in a class...
  //  exitConfirmationDialog = NULL;
  int w = 400;
  int h = 120;
  exitConfirmationDialog = new Window(getSDLHandler(),
                                      (getSDLHandler()->getScreen()->w/2) - (w/2), 
                                      (getSDLHandler()->getScreen()->h/2) - (h/2), 
                                      w, h,
                                      "Leave level?", 
                                      getShapePalette()->getGuiTexture(), false,
                                      Window::BASIC_WINDOW,
                                      getShapePalette()->getGuiTexture2());
  int mx = w / 2;
  yesExitConfirm = new Button( mx - 80, 50, mx - 10, 80, getShapePalette()->getHighlightTexture(), "Yes" );
  exitConfirmationDialog->addWidget((Widget*)yesExitConfirm);
  noExitConfirm = new Button( mx + 10, 50, mx + 80, 80, getShapePalette()->getHighlightTexture(), "No" );
  exitConfirmationDialog->addWidget((Widget*)noExitConfirm);
  exitLabel = new Label(20, 20, Constants::getMessage(Constants::EXIT_MISSION_LABEL));
  exitConfirmationDialog->addWidget((Widget*)exitLabel);
}

void Scourge::setUILayout(int mode) {
  layoutMode = mode;
  setUILayout();
}

void Scourge::setUILayout() {

  // reshape the levelMap
  int mapX = 0;
  int mapY = 0;
  int mapWidth = getSDLHandler()->getScreen()->w;
  int mapHeight = getSDLHandler()->getScreen()->h;

  // move the message gui
  int width = 
  getSDLHandler()->getScreen()->w -                 
  (PARTY_GUI_WIDTH + (Window::SCREEN_GUTTER * 2));

  mainWin->setVisible( false );
  messageWin->setVisible(false);
  miniMap->getWindow()->setVisible(false);
  switch(layoutMode) {
  case Constants::GUI_LAYOUT_ORIGINAL:
    messageList->resize(width, PARTY_GUI_HEIGHT - 25);
    messageWin->resize(width, PARTY_GUI_HEIGHT);
    messageWin->move(getSDLHandler()->getScreen()->w - width, 0);
    messageWin->setLocked(false);
    mainWin->setLocked(false);
    miniMap->getWindow()->setLocked(false);
    miniMap->resize(MINIMAP_WINDOW_WIDTH, MINIMAP_WINDOW_HEIGHT);
    miniMap->getWindow()->move(0, getSDLHandler()->getScreen()->h - (PARTY_GUI_HEIGHT + MINIMAP_WINDOW_HEIGHT + Window::SCREEN_GUTTER));
//  if(inventory->getWindow()->isLocked()) {
    inventory->hide();
//    inventory->getWindow()->setLocked(false);
//  }
    netPlay->getWindow()->move(0, getSDLHandler()->getScreen()->h - PARTY_GUI_HEIGHT - Window::SCREEN_GUTTER);
    netPlay->getWindow()->setLocked(false);
    break;

  case Constants::GUI_LAYOUT_BOTTOM:
    messageList->resize(width, PARTY_GUI_HEIGHT - 25);
    messageWin->resize(width, PARTY_GUI_HEIGHT);
    messageWin->move(0, getSDLHandler()->getScreen()->h - PARTY_GUI_HEIGHT - Window::SCREEN_GUTTER);
    mapHeight = getSDLHandler()->getScreen()->h - PARTY_GUI_HEIGHT - Window::SCREEN_GUTTER;
    messageWin->setLocked(true);
    mainWin->setLocked(true);
    miniMap->getWindow()->setLocked(true);
    miniMap->getWindow()->move(0,
                               getSDLHandler()->getScreen()->h - (PARTY_GUI_HEIGHT + MINIMAP_WINDOW_HEIGHT + Window::SCREEN_GUTTER + Window::SCREEN_GUTTER));
    miniMap->resize(MINIMAP_WINDOW_WIDTH, MINIMAP_WINDOW_HEIGHT);
//  if(inventory->getWindow()->isLocked()) {
    //inventory->getWindow()->setVisible(false);
    inventory->hide();
//    inventory->getWindow()->setLocked(false);
//  }
    netPlay->getWindow()->move(0, getSDLHandler()->getScreen()->h - PARTY_GUI_HEIGHT * 2 - Window::SCREEN_GUTTER);
    netPlay->getWindow()->setLocked(true);
    break;

  case Constants::GUI_LAYOUT_SIDE:
    messageList->resize(PARTY_GUI_WIDTH, getSDLHandler()->getScreen()->h - (PARTY_GUI_HEIGHT + Window::SCREEN_GUTTER * 2 + MINIMAP_WINDOW_HEIGHT + 25));
    messageWin->resize(PARTY_GUI_WIDTH, 
                       getSDLHandler()->getScreen()->h - (PARTY_GUI_HEIGHT + Window::SCREEN_GUTTER * 2 + MINIMAP_WINDOW_HEIGHT));
    messageWin->move(getSDLHandler()->getScreen()->w - PARTY_GUI_WIDTH,  MINIMAP_WINDOW_HEIGHT + Window::SCREEN_GUTTER);
    miniMap->getWindow()->setLocked(true);
    miniMap->getWindow()->move(getSDLHandler()->getScreen()->w - MINIMAP_WINDOW_WIDTH, 0);
    miniMap->resize(MINIMAP_WINDOW_WIDTH, MINIMAP_WINDOW_HEIGHT);
    //mapX = 400;
    mapX = 0;
    mapWidth = getSDLHandler()->getScreen()->w - PARTY_GUI_WIDTH;
    mapHeight = getSDLHandler()->getScreen()->h;
    messageWin->setLocked(true);
    mainWin->setLocked(true);
//  if(inventory->getWindow()->isLocked()) {
    inventory->hide();
//    inventory->getWindow()->setLocked(false);
//  }
    netPlay->getWindow()->move(0, getSDLHandler()->getScreen()->h - PARTY_GUI_HEIGHT);
    netPlay->getWindow()->setLocked(true);
    break;

  case Constants::GUI_LAYOUT_INVENTORY:
    messageList->resize(width, PARTY_GUI_HEIGHT - 25);
    messageWin->resize(width, PARTY_GUI_HEIGHT);
    messageWin->move(0, getSDLHandler()->getScreen()->h - PARTY_GUI_HEIGHT - Window::SCREEN_GUTTER);
    miniMap->getWindow()->setLocked(true);
    miniMap->getWindow()->move(getSDLHandler()->getScreen()->w - MINIMAP_WINDOW_WIDTH, 0);
    int h = getSDLHandler()->getScreen()->h - (PARTY_GUI_HEIGHT + INVENTORY_HEIGHT + Window::SCREEN_GUTTER * 2);
    miniMap->resize(MINIMAP_WINDOW_WIDTH, (h > MINIMAP_WINDOW_HEIGHT ? MINIMAP_WINDOW_HEIGHT : h));
    mapHeight = getSDLHandler()->getScreen()->h - PARTY_GUI_HEIGHT - Window::SCREEN_GUTTER;
    messageWin->setLocked(true);
    mainWin->setLocked(true);
    inventory->hide();
    //inventory->getWindow()->move(getSDLHandler()->getScreen()->w - INVENTORY_WIDTH, 
                                 //getSDLHandler()->getScreen()->h - (PARTY_GUI_HEIGHT + INVENTORY_HEIGHT + Window::SCREEN_GUTTER));
//  inventory->getWindow()->setLocked(true);
    inventory->show(false);
    //mapX = INVENTORY_WIDTH;
    mapX = 0;
    mapWidth = getSDLHandler()->getScreen()->w - INVENTORY_WIDTH;
    mapHeight = getSDLHandler()->getScreen()->h - PARTY_GUI_HEIGHT - Window::SCREEN_GUTTER;
    netPlay->getWindow()->move(0, getSDLHandler()->getScreen()->h - PARTY_GUI_HEIGHT * 2 - Window::SCREEN_GUTTER);
    netPlay->getWindow()->setLocked(true);
    break;
  }

  inventoryButton->setSelected( inventory->isVisible() );
  optionsButton->setSelected( optionsMenu->isVisible() );

  messageWin->setVisible(true, false);
  miniMap->getWindow()->setVisible(true, false);

  mainWin->move(getSDLHandler()->getScreen()->w - PARTY_GUI_WIDTH,
                getSDLHandler()->getScreen()->h - PARTY_GUI_HEIGHT);
  mainWin->setVisible( true, false );
  inventory->positionWindow();

  // FIXME: resize levelMap drawing area to remainder of screen.
  levelMap->setViewArea(mapX, mapY, mapWidth, mapHeight);
  if(getParty()->getPlayer()) {
    getMap()->center(toint(getParty()->getPlayer()->getX()),
                     toint(getParty()->getPlayer()->getY()), 
                     true);
  }
}

void Scourge::playRound() {                           
  // move the levelMap
  if(move) levelMap->move(move);

  if(targetSelectionFor) return;
 
  // is the game not paused?
  if(party->isRealTimeMode()) {
    
    Projectile::moveProjectiles(this);

    // fight battle turns
    bool fromBattle = false;
    if(battleRound.size() > 0) {
      fromBattle = fightCurrentBattleTurn();
    } 

    // create a new battle round
    if(battleRound.size() == 0 && !createBattleTurns()) {
      // not in battle
      if(fromBattle) {
        // go back to real-time, group-mode
        resetUIAfterBattle();
      }
      moveCreatures();
    }
  }
}

// fight a turn of the battle
bool Scourge::fightCurrentBattleTurn() {
  if(battleRound.size() > 0) {

    // end of battle if party has no-one to attack
    bool roundOver = false;
    int c = 0;
    for( int i = 0; i < party->getPartySize(); i++ ) {
      if( party->getParty(i)->getAction() == Constants::ACTION_NO_ACTION && 
          !party->getParty(i)->getBattle()->getAvailableTarget() ) {
        c++;
      }
    }
    if( c == party->getPartySize() ) {
      for( int i = 0; i < party->getPartySize(); i++ ) {
		if( Battle::debugBattle ) cerr << "*** Reset in scourge!" << endl;
        party->getParty(i)->getBattle()->reset();
      }
      roundOver = true;
    }

    if( !roundOver ) {
      if( getUserConfiguration()->isBattleTurnBased() ) {
        // TB: fight the current battle turn only
        Battle *battle = battleRound[battleTurn];
        if(battle->fightTurn()) {
          battleTurn++;
        }
        roundOver = ( battleTurn >= (int)battleRound.size() );
      } else {
        // RT: fight every battle turn
        for( int i = 0; i < (int)battleRound.size(); i++) {
          Battle *battle = battleRound[i];
          battle->fightTurn();
        }
        roundOver = true;
      }
    }

    if( roundOver ) {
      rtStartTurn = battleTurn = 0;
      if(battleRound.size()) battleRound.erase(battleRound.begin(), battleRound.end());
      
      if(Battle::debugBattle) cerr << "ROUND ENDS" << endl;
      if(Battle::debugBattle) cerr << "----------------------------------" << endl;
      return true;
    }
  }
  return false;
}

bool Scourge::createBattleTurns() {
  if(!BATTLES_ENABLED) return false;

  // set up battles
  battleCount = 0;

  // anybody doing anything?
  for( int i = 0; i < party->getPartySize(); i++ ) {
    if( !party->getParty(i)->getStateMod(Constants::dead) ) {
      // possessed creature attacks fellows...
      if(party->getParty(i)->getTargetCreature() && 
         party->getParty(i)->getStateMod(Constants::possessed)) {
        Creature *target = session->getParty()->getClosestPlayer(toint(party->getParty(i)->getX()), 
                                                                 toint(party->getParty(i)->getY()), 
                                                                 party->getParty(i)->getShape()->getWidth(),
                                                                 party->getParty(i)->getShape()->getDepth(),
                                                                 20);
        if (target) {
          party->getParty(i)->setTargetCreature(target);
        }
      }
      bool hasTarget = (party->getParty(i)->hasTarget() || 
                        party->getParty(i)->getAction() > -1);
      bool visible = ( levelMap->isLocationVisible(toint(party->getParty(i)->getX()), 
                                                   toint(party->getParty(i)->getY())) &&
                       levelMap->isLocationInLight(toint(party->getParty(i)->getX()), 
                                                   toint(party->getParty(i)->getY())));
      if( hasTarget ) {
        if( party->getParty(i)->isTargetValid() && visible ) {
          if( Battle::debugBattle ) cerr << "*** init party target" << endl;
          battle[battleCount++] = party->getParty(i)->getBattle();
        } else {
          party->getParty(i)->cancelTarget();
        }
      }
    }
  }
  for (int i = 0; i < session->getCreatureCount(); i++) {
    if (!session->getCreature(i)->getStateMod(Constants::dead) &&
        levelMap->isLocationVisible(toint(session->getCreature(i)->getX()), 
                                    toint(session->getCreature(i)->getY())) &&
        levelMap->isLocationInLight(toint(session->getCreature(i)->getX()), 
                                    toint(session->getCreature(i)->getY())) &&
        !(session->getCreature(i)->getMonster() && 
          session->getCreature(i)->getMonster()->isNpc())) {
      bool hasTarget = (session->getCreature(i)->getTargetCreature() ||
                        session->getCreature(i)->getAction() > -1);
      // Don't start a round if this creature is unreachable by party. Otherwise
      // this causes a lock-up.
      bool possible = ( session->getCreature(i)->getBattle()->getAvailablePartyTarget() != NULL );
      if( hasTarget ) {
        if( session->getCreature(i)->isTargetValid() ) {
          if( !possible ) {
            if( Battle::debugBattle ) 
              cerr << "*** not starting combat: possible is false." << endl;
            session->getCreature(i)->cancelTarget();
          } else battle[battleCount++] = session->getCreature(i)->getBattle();
        } else {
          session->getCreature(i)->cancelTarget();
        }
      }
    }
  }

  // if somebody is attacking (or casting a spell), enter battle mode
  // and add everyone present to the battle round.
  if (battleCount > 0) {

    // add other movement
    for (int i = 0; i < party->getPartySize(); i++) {
      bool visible = ( levelMap->isLocationVisible(toint(party->getParty(i)->getX()), 
                                              toint(party->getParty(i)->getY())) &&
                       levelMap->isLocationInLight(toint(party->getParty(i)->getX()), 
                                              toint(party->getParty(i)->getY())));
      if( visible && !party->getParty(i)->getStateMod(Constants::dead) ) {
        bool found = false;
        for( int t = 0; t < battleCount; t++ ) {
          if( battle[t] == party->getParty(i)->getBattle() ) {
            found = true;
            break;
          }
        }
        if( !found ) battle[battleCount++] = party->getParty(i)->getBattle();
      }
    }
    for (int i = 0; i < session->getCreatureCount(); i++) {
      if (!session->getCreature(i)->getStateMod(Constants::dead) &&
          levelMap->isLocationVisible(toint(session->getCreature(i)->getX()), 
                                 toint(session->getCreature(i)->getY())) &&
          levelMap->isLocationInLight(toint(session->getCreature(i)->getX()), 
                                 toint(session->getCreature(i)->getY()))) {
        bool hasTarget = (session->getCreature(i)->getTargetCreature() ||
                          session->getCreature(i)->getAction() > -1);
        if (!hasTarget || (hasTarget && !session->getCreature(i)->isTargetValid())) {
          bool found = false;
          for( int t = 0; t < battleCount; t++ ) {
            if( battle[t] == session->getCreature(i)->getBattle() ) {
              found = true;
              break;
            }
          }
          if( !found ) battle[battleCount++] = session->getCreature(i)->getBattle();
        }
      }
    }

    party->savePlayerSettings();

    // order the battle turns by initiative
    Battle::setupBattles(getSession(), battle, battleCount, &battleRound);
    rtStartTurn = battleTurn = 0;
    if(Battle::debugBattle) cerr << "++++++++++++++++++++++++++++++++++" << endl;
    if(Battle::debugBattle) cerr << "ROUND STARTS" << endl;

    if(getUserConfiguration()->isBattleTurnBased()) groupButton->setVisible(false);
    return true;
  } else {
    return false;
  }
}

void Scourge::resetUIAfterBattle() {
  toggleRoundUI(party->isRealTimeMode());
  //party->setFirstLivePlayer();
  party->restorePlayerSettings();
  if(getUserConfiguration()->isBattleTurnBased() && 
     party->isPlayerOnly()) party->togglePlayerOnly();
  groupButton->setVisible(true);
  for(int i = 0; i < party->getPartySize(); i++) {
    party->getParty(i)->cancelTarget();
    if(party->getParty(i)->anyMovesLeft()) {
      party->getParty(i)->getShape()->setCurrentAnimation((int)MD2_RUN, true);
    } else {
      party->getParty(i)->getShape()->setCurrentAnimation((int)MD2_STAND, true);
    }
  }
}

void Scourge::moveCreatures() {
  // change animation if needed
  for(int i = 0; i < party->getPartySize(); i++) {                            
    if(((MD2Shape*)(party->getParty(i)->getShape()))->getAttackEffect()) {
      party->getParty(i)->getShape()->setCurrentAnimation((int)MD2_ATTACK);	  
      ((MD2Shape*)(party->getParty(i)->getShape()))->setAngle(party->getParty(i)->getTargetAngle());
    } else if(party->getParty(i)->anyMovesLeft())
      party->getParty(i)->getShape()->setCurrentAnimation((int)MD2_RUN);
    else 
      party->getParty(i)->getShape()->setCurrentAnimation((int)MD2_STAND);
  }

  // move the party members
  party->movePlayers();

  // move visible monsters
  for(int i = 0; i < session->getCreatureCount(); i++) {
    if(!session->getCreature(i)->getStateMod(Constants::dead) && 
       levelMap->isLocationVisible(toint(session->getCreature(i)->getX()), 
                              toint(session->getCreature(i)->getY()))) {
      moveMonster(session->getCreature(i));
    }
  }
}

void Scourge::addGameSpeed(int speedFactor){
  char msg[80];
  userConfiguration->setGameSpeedLevel(userConfiguration->getGameSpeedLevel() + speedFactor);
  sprintf(msg, "Speed set to %d\n", userConfiguration->getGameSpeedTicks());
  levelMap->addDescription(msg);
}

//#define MONSTER_FLEE_IF_LOW_HP

void Scourge::moveMonster(Creature *monster) {
  // set running animation (currently move or attack)
  if(((MD2Shape*)(monster->getShape()))->getAttackEffect()) {
    //monster->getShape()->setCurrentAnimation((int)MD2_ATTACK);
    //((MD2Shape*)(monster->getShape()))->setAngle(monster->getTargetAngle());
    // don't move when attacking
    return;
  } else {
    //if(battleRound.size() > 0) monster->getShape()->setCurrentAnimation((int)MD2_RUN);
    monster->getShape()->setCurrentAnimation( monster->getMotion() == Constants::MOTION_LOITER ? 
                                              (int)MD2_RUN :
                                              (int)MD2_STAND );
  }

  if(monster->getMotion() == Constants::MOTION_LOITER) {
    // attack the closest player
    if( BATTLES_ENABLED && 
        (int)(20.0f * rand()/RAND_MAX) == 0) {
      monster->decideMonsterAction();
    } else {
      // random (non-attack) monster movement
      monster->move(monster->getDir(), levelMap);
    }
  } else if(monster->getMotion() == Constants::MOTION_STAND) {
    if( (int)(40.0f * rand()/RAND_MAX) == 0) {
      monster->setMotion(Constants::MOTION_LOITER);
    } else {
      monster->decideMonsterAction();
    }
  } else if(monster->hasTarget()) {
#ifdef MONSTER_FLEE_IF_LOW_HP
    // monster gives up when low on hp or bored
    // FIXME: when low on hp, it should run away not loiter
    if(monster->getAction() == Constants::ACTION_NO_ACTION &&
       monster->getHp() < (int)((float)monster->getStartingHp() * 0.2f)) {
      monster->setMotion(Constants::MOTION_LOITER);
      monster->cancelTarget();
    } else {
      monster->moveToLocator(levelMap);
    }
#endif
    // see if there's another target that's closer
    if(monster->getAction() == Constants::ACTION_NO_ACTION) {
      monster->decideMonsterAction();
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
							getSDLHandler()->getScreen()->w / 2 - 200,
							getSDLHandler()->getScreen()->h / 2 - 55,
							400, 110, Constants::messages[Constants::SCOURGE_DIALOG][0],
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

Window *Scourge::createWindow(int x, int y, int w, int h, char *title) {
  Window *win = new Window( getSDLHandler(), x, y, w, h, title, 
                            getShapePalette()->getGuiTexture(), 
                            true, Window::BASIC_WINDOW,
                            getShapePalette()->getGuiTexture2() );
  return win;
}

void Scourge::missionCompleted() {
  showMessageDialog("Congratulations, mission accomplished!");
  
  // Award exp. points for completing the mission
  if(getSession()->getCurrentMission() && missionWillAwardExpPoints && 
     getSession()->getCurrentMission()->isCompleted()) {
    
    // only do this once
    missionWillAwardExpPoints = false;
    
    // how many points?
    int exp = (getSession()->getCurrentMission()->getLevel() + 1) * 100;
    levelMap->addDescription("For completing the mission", 0, 1, 1);
    char message[200];
    sprintf(message, "The party receives %d points.", exp);
    levelMap->addDescription(message, 0, 1, 1);
    
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

#ifdef HAVE_SDL_NET
int Scourge::initMultiplayer() {
  int serverPort = 6666; // FIXME: make this more dynamic
  if(multiplayer->getValue() == MultiplayerDialog::START_SERVER) {
    session->startServer((GameStateHandler*)netPlay, serverPort);
    session->startClient((GameStateHandler*)netPlay, (CommandInterpreter*)netPlay, 
                         (char*)Constants::localhost, 
                         serverPort, 
                         (char*)Constants::adminUserName);
  } else {
    session->startClient((GameStateHandler*)netPlay, (CommandInterpreter*)netPlay,
                         multiplayer->getServerName(),
                         atoi(multiplayer->getServerPort()),
                         multiplayer->getUserName());
  }
  Progress *progress = new Progress(this, 10);
  progress->updateStatus("Connecting to server");
  if(!session->getClient()->login()) {
    cerr << Constants::getMessage(Constants::CLIENT_CANT_CONNECT_ERROR) << endl;
    showMessageDialog(Constants::getMessage(Constants::CLIENT_CANT_CONNECT_ERROR));
    delete progress;
    return 0;
  }
  progress->updateStatus("Connected!");
  SDL_Delay(3000);

  delete progress;

  return 1;
} 


#endif

int Scourge::getScreenWidth() {
  return getSDLHandler()->getScreen()->w;
}

int Scourge::getScreenHeight() {
  return getSDLHandler()->getScreen()->h;
}

void Scourge::fightProjectileHitTurn(Projectile *proj, Creature *creature) {
  Battle::projectileHitTurn(getSession(), proj, creature);
}

void Scourge::fightProjectileHitTurn(Projectile *proj, int x, int y) {
  Battle::projectileHitTurn(getSession(), proj, x, y);
}

void Scourge::createPartyUI() {
  sprintf(version, "S.C.O.U.R.G.E. version %s", SCOURGE_VERSION);
  sprintf(min_version, "S.C.O.U.R.G.E.");
  mainWin = new Window( getSDLHandler(),
                        getSDLHandler()->getScreen()->w - Scourge::PARTY_GUI_WIDTH, 
                        getSDLHandler()->getScreen()->h - Scourge::PARTY_GUI_HEIGHT, 
                        Scourge::PARTY_GUI_WIDTH, 
                        Scourge::PARTY_GUI_HEIGHT, 
                        version, false, Window::BASIC_WINDOW,
                        "default" );
  cards = new CardContainer(mainWin);  

  roundButton = cards->createButton( 5, 0, 90, 20, "Real-Time", 0 );
  endTurnButton = cards->createButton( 5, 20, 90, 40, "End Turn", 0 );
  groupButton = cards->createButton( 5, 40,  90, 60, "Group Mode", 0 );
  inventoryButton = cards->createButton( 5, 60, 90, 80, "Party Info", 0 );  
  optionsButton = cards->createButton( 5, 80,  90, 100, "Options", 0 );
  quitButton = cards->createButton( 5, 100,  90, 120, "Quit", 0 );
  groupButton->setToggle(true);
  groupButton->setSelected(true);
  roundButton->setToggle(true);
  roundButton->setSelected(true);
  inventoryButton->setToggle(true);
  inventoryButton->setSelected(false);
  optionsButton->setToggle(true);
  optionsButton->setSelected(false);

  int offsetX = 90;
  int playerButtonWidth = (Scourge::PARTY_GUI_WIDTH - offsetX) / 4;
  //int playerButtonHeight = 20;  
  int playerInfoHeight = 100;
  //int playerButtonY = playerInfoHeight;

  for(int i = 0; i < 4; i++) {
    playerInfo[i] = new Canvas( offsetX + playerButtonWidth * i, 20,  
                                offsetX + playerButtonWidth * (i + 1) - 25, 20 + playerInfoHeight, 
                                this, this );
    cards->addWidget( playerInfo[i], 0 );
    playerHpMp[i] = new Canvas( offsetX + playerButtonWidth * (i + 1) - 25, 20,  
                                offsetX + playerButtonWidth * (i + 1), 20 + playerInfoHeight, 
                                this, NULL, true );
    cards->addWidget( playerHpMp[i], 0 );
  }
  int quickButtonWidth = (int)((float)(Scourge::PARTY_GUI_WIDTH - offsetX - 20) / 12.0f);
  for( int i = 0; i < 12; i++ ) {
    int xx = offsetX + quickButtonWidth * i + ( i / 4 ) * 10;
    quickSpell[i] = new Canvas( xx, 0, xx + quickButtonWidth, 20, 
                                this, NULL, true );
    cards->addWidget( quickSpell[i], 0 );
  }    
  
  cards->setActiveCard( 0 );   
}

void Scourge::receive( Widget *widget ) {
  if( getMovingItem() ) {
    int selected = -1;
    for(int i = 0; i < getParty()->getPartySize(); i++) {
      if( widget == playerInfo[i] ) {
        selected = i;
        break;
      }
    }
    if( selected == -1 ) return;
    getParty()->setPlayer( selected );
    inventory->receive( widget );
  }
}     

void Scourge::drawWidget(Widget *w) {
  for(int i = 0; i < party->getPartySize(); i++) {
    if(playerInfo[i] == w) {
      drawPortrait( w, party->getParty( i ) );
      return;
    } else if( playerHpMp[i] == w ) {
      Creature *p = party->getParty( i );
      Util::drawBar( 10, 5, 90,
                     (float)p->getHp(), (float)p->getMaxHp(), 
                     -1, -1, -1, true, mainWin->getTheme(), 
                     Util::VERTICAL_LAYOUT );
      Util::drawBar( 17, 5, 90,
                     (float)p->getMp(), (float)p->getMaxMp(), 
                     0, 0, 1, false, mainWin->getTheme(), 
                     Util::VERTICAL_LAYOUT );
      return;
    }
  }
  for( int t = 0; t < 12; t++ ) {
    if( quickSpell[t] == w ) {
      quickSpell[t]->setGlowing( inventory->inStoreSpellMode() );
      for(int i = 0; i < party->getPartySize(); i++) {
        if( party->getParty( i ) == getParty()->getPlayer() ) {
          if( getParty()->getPlayer()->getQuickSpell( t ) ) {
			Spell *spell = getParty()->getPlayer()->getQuickSpell( t );
      w->setTooltip( spell->getName() );
			glEnable( GL_ALPHA_TEST );
			glAlphaFunc( GL_EQUAL, 0xff );
			glEnable(GL_TEXTURE_2D);
			glPushMatrix();
			//    glTranslatef( x, y, 0 );
			glBindTexture( GL_TEXTURE_2D, getShapePalette()->spellsTex[ spell->getIconTileX() ][ spell->getIconTileY() ] );
			glColor4f(1, 1, 1, 1);
			
			glBegin( GL_QUADS );
			glNormal3f( 0, 0, 1 );
			glTexCoord2f( 0, 0 );
			glVertex3f( 0, 0, 0 );
			glTexCoord2f( 0, 1 );
			glVertex3f( 0, w->getHeight(), 0 );
			glTexCoord2f( 1, 1 );
			glVertex3f( w->getWidth(), w->getHeight(), 0 );
			glTexCoord2f( 1, 0 );
			glVertex3f( w->getWidth(), 0, 0 );
			glEnd();
			glPopMatrix();
			
			glDisable( GL_ALPHA_TEST );
			glDisable(GL_TEXTURE_2D);
          }
          return;
        }
      }
    }
  }
  

  cerr << "Warning: Unknown widget in Party::drawWidget." << endl;
  return;
}

void Scourge::drawPortrait( Widget *w, Creature *p ) {
  glPushMatrix();
  glEnable( GL_TEXTURE_2D );
  glColor4f( 1, 1, 1, 1 );
  if( p->getStateMod( Constants::dead ) ) {
    glBindTexture( GL_TEXTURE_2D, getShapePalette()->getDeathPortraitTexture() );
  } else {
    glBindTexture( GL_TEXTURE_2D, getShapePalette()->getPortraitTexture( p->getPortraitTextureIndex() ) );
  }
  int portraitSize = ((Scourge::PARTY_GUI_WIDTH - 90) / 4);
  int offs = 15;
  glBegin( GL_QUADS );
  glNormal3f( 0, 0, 1 );
  glTexCoord2f( 0, 0 );
  glVertex2i( -offs, 0 );
  glTexCoord2f( 1, 0 );
  glVertex2i( portraitSize - offs, 0 );
  glTexCoord2f( 1, 1 );
  glVertex2i( portraitSize - offs, portraitSize );
  glTexCoord2f( 0, 1 );
  glVertex2i( -offs, portraitSize );
  glEnd();
  glDisable( GL_TEXTURE_2D );

  bool shade = false;
  if( p->getStateMod( Constants::possessed ) ) {
    glColor4f( 1.0f, 0, 0, 0.5f );
    shade = true;
  } else if( p->getStateMod( Constants::invisible ) ) {
    glColor4f( 0, 0.75f, 1.0f, 0.5f );
    shade = true;
  } else if( p->getStateMod( Constants::poisoned ) ) {
    glColor4f( 1, 0.75f, 0, 0.5f );
    shade = true;
  } else if( p->getStateMod( Constants::blinded ) ) {
    glColor4f( 1, 1, 1, 0.5f );
    shade = true;
  } else if( p->getStateMod( Constants::cursed ) ) {
    glColor4f( 0.75, 0, 0.75f, 0.5f );
    shade = true;
  }
  if( shade ) {
    glEnable( GL_BLEND );
    glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );
    glBegin( GL_QUADS );
    glVertex2i( 0, 0 );
    glVertex2i( portraitSize, 0 );
    glVertex2i( portraitSize, portraitSize );
    glVertex2i( 0, portraitSize );
    glEnd();
    glDisable( GL_BLEND );
  }

  glPopMatrix();
  
  glColor3f( 1, 1, 1 );
  getSDLHandler()->texPrint( 5, 12, "%s", p->getName() );

  // show stat mods
  glEnable(GL_TEXTURE_2D);
  glColor4f( 1.0f, 1.0f, 1.0f, 0.5f );
  int xp = 0;
  int yp = 1;
  float n = 12;
  int row = ( w->getWidth() / (int)n );
  for(int i = 0; i < Constants::STATE_MOD_COUNT; i++) {
    if(p->getStateMod(i)) {
      GLuint icon = getShapePalette()->getStatModIcon(i);
      if(icon) {
        glBindTexture( GL_TEXTURE_2D, icon );
      }

      glPushMatrix();
      glTranslatef( 5 + xp * (n + 1), 
                    w->getHeight() - (yp * (n + 1)), 
                    0 );
      glBegin( GL_QUADS );
      glNormal3f( 0, 0, 1 );
      if(icon) glTexCoord2f( 0, 0 );
      glVertex3f( 0, 0, 0 );
      if(icon) glTexCoord2f( 0, 1 );
      glVertex3f( 0, n, 0 );
      if(icon) glTexCoord2f( 1, 1 );
      glVertex3f( n, n, 0 );
      if(icon) glTexCoord2f( 1, 0 );
      glVertex3f( n, 0, 0 );
      glEnd();
      glPopMatrix();

      xp++;
      if(xp >= row) {
        xp = 0;
        yp++;
      }
    }
  }
  glDisable(GL_TEXTURE_2D);

  // draw selection border
  if( p == getParty()->getPlayer() ) {
    glEnable( GL_BLEND );
    glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );
    float lineWidth = 5.0f;
    glLineWidth( 5.0f );
    GuiTheme *theme = mainWin->getTheme();
    if( theme->getSelectionBackground() ) {
      glColor4f( theme->getSelectionBackground()->color.r,
                 theme->getSelectionBackground()->color.g,
                 theme->getSelectionBackground()->color.b,
                 0.5f );
    } else {
      mainWin->applySelectionColor();
    }
    glBegin( GL_LINE_LOOP );
    glVertex2f( lineWidth / 2.0f, lineWidth / 2.0f );
    glVertex2f( lineWidth / 2.0f, w->getHeight() - lineWidth / 2.0f );
    glVertex2f( w->getWidth() - lineWidth / 2.0f, w->getHeight() - lineWidth / 2.0f );
    glVertex2f( w->getWidth() - lineWidth / 2.0f, lineWidth / 2.0f );
    glEnd();
    glLineWidth( 1.0f );
    glDisable( GL_BLEND );
  }
}

void Scourge::resetPartyUI() {
  Event *e;  
  Date d(0, 0, 6, 0, 0, 0); // 6 hours (format : sec, min, hours, days, months, years)
  for(int i = 0; i < party->getPartySize() ; i++){
    e = new ThirstHungerEvent(party->getCalendar()->getCurrentDate(), d, party->getParty(i), this, Event::INFINITE_EXECUTIONS);
    party->getCalendar()->scheduleEvent((Event*)e);   // It's important to cast!!
  }
}

bool Scourge::handlePartyEvent(Widget *widget, SDL_Event *event) {
  if(widget == inventoryButton) {
    toggleInventoryWindow();
  } else if(widget == endTurnButton &&
            inTurnBasedCombatPlayerTurn()) {
    battleRound[battleTurn]->endTurn();
  } else if(widget == optionsButton) {
    toggleOptionsWindow();
  } else if(widget == quitButton) {
    showExitConfirmationDialog();
    /*
  } else if(widget == diamondButton) {
    party->setFormation(Constants::DIAMOND_FORMATION - Constants::DIAMOND_FORMATION);
  } else if(widget == staggeredButton) {
    party->setFormation(Constants::STAGGERED_FORMATION - Constants::DIAMOND_FORMATION);
  } else if(widget == squareButton) {
    party->setFormation(Constants::SQUARE_FORMATION - Constants::DIAMOND_FORMATION);
  } else if(widget == rowButton) {
    party->setFormation(Constants::ROW_FORMATION - Constants::DIAMOND_FORMATION);
  } else if(widget == scoutButton) {
    party->setFormation(Constants::SCOUT_FORMATION - Constants::DIAMOND_FORMATION);
  } else if(widget == crossButton) {
    party->setFormation(Constants::CROSS_FORMATION - Constants::DIAMOND_FORMATION);
    */
  } else if(widget == playerInfo[0] ) {
    if( getTargetSelectionFor() ) {
      handleTargetSelectionOfCreature( getParty()->getParty( 0 ) );
    } else {
      setPlayer(Constants::PLAYER_1 - Constants::PLAYER_1);
    }
  } else if(widget == playerInfo[1] ) {
    if( getTargetSelectionFor() ) {
      handleTargetSelectionOfCreature( getParty()->getParty( 1 ) );
    } else {
      setPlayer(Constants::PLAYER_2 - Constants::PLAYER_1);
    }
  } else if(widget == playerInfo[2] ) {
    if( getTargetSelectionFor() ) {
      handleTargetSelectionOfCreature( getParty()->getParty( 2 ) );
    } else {
      setPlayer(Constants::PLAYER_3 - Constants::PLAYER_1);
    }
  } else if(widget == playerInfo[3] ) {
    if( getTargetSelectionFor() ) {
      handleTargetSelectionOfCreature( getParty()->getParty( 3 ) );
    } else {
      setPlayer(Constants::PLAYER_4 - Constants::PLAYER_1);
    }
  } else if(widget == groupButton && !inTurnBasedCombat()) {
    party->togglePlayerOnly();
  } else if(widget == roundButton) {
    party->toggleRound();
  } else {
    for( int t = 0; t < 4; t++ ) {
      if( widget == playerHpMp[t] ) {
        inventory->showSkills();
        if( getParty()->getPlayer() != getParty()->getParty( t ) ) {
          getParty()->setPlayer( t );
          if( !inventory->isVisible() ) toggleInventoryWindow();
        } else {
          toggleInventoryWindow();
        }
      }
    }
    for( int t = 0; t < 12; t++ ) {
      if( widget == quickSpell[t] ) {
        quickSpellAction( t );
      }
    }
  }
  return false;
}

void Scourge::quickSpellAction( int index ) {
  if( inventory->inStoreSpellMode() ) {    
    getParty()->getPlayer()->setQuickSpell( index, inventory->getStoreSpell() );
    inventory->setStoreSpellMode( false );
    if( inventory->isVisible() ) toggleInventoryWindow();
  } else {
    Creature *creature = getParty()->getPlayer();
    Spell *spell = creature->getQuickSpell( index );
    if( spell ) {
      if(spell->getMp() > creature->getMp()) {
        showMessageDialog("Not enough Magic Points to cast this spell!");
      } else {
        creature->setAction(Constants::ACTION_CAST_SPELL, 
                            NULL,
                            spell);
        if(!spell->isPartyTargetAllowed()) {
          setTargetSelectionFor(creature);
        } else {
          creature->setTargetCreature(creature);
        }
      }
    } else {
      inventory->showSpells();
      if( !inventory->isVisible() ) toggleInventoryWindow();
    }
  }
}

void Scourge::refreshInventoryUI(int playerIndex) {
  getInventory()->refresh(playerIndex);
}

void Scourge::refreshInventoryUI() {
  getInventory()->refresh();
}

void Scourge::updatePartyUI() {
  // update current date variables and see if scheduled events have occured  
  /*
  if(party->getCalendar()->update(getUserConfiguration()->getGameSpeedLevel())){
    calendarButton->setLabel(party->getCalendar()->getCurrentDate().getDateString());        
  }
  */
  // refresh levelMap if any party member's effect is on
  bool effectOn = false;
  for(int i = 0; i < party->getPartySize(); i++) {
	if(!party->getParty(i)->getStateMod(Constants::dead) && party->getParty(i)->isEffectOn()) {
	  effectOn = true;
	  break;
	}
  }
  if(effectOn != lastEffectOn) {
	lastEffectOn = effectOn;
	getMap()->refresh();
  }
}

void Scourge::setPlayer(int n) {
  // don't change player in TB combat
  if(battleTurn < (int)battleRound.size() &&
     getUserConfiguration()->isBattleTurnBased()) return;
  party->setPlayer(n);
}

void Scourge::setPlayerUI(int index) {
}

void Scourge::toggleRoundUI(bool startRound) {
  if(battleTurn < (int)battleRound.size() &&
     getUserConfiguration()->isBattleTurnBased()) {
    if(!startRound && 
       !battleRound[battleTurn]->getCreature()->isMonster()) {
      roundButton->setLabel("Begin Turn");
      roundButton->setGlowing(true);
    } else {
      roundButton->setLabel("...in Turn...");
      roundButton->setGlowing(false);
    }
  } else {
    if(startRound) roundButton->setLabel("Real-Time      ");
    else roundButton->setLabel("Paused");
    roundButton->setGlowing(false);
  }
  roundButton->setSelected(startRound);
}

void Scourge::setFormationUI(int formation, bool playerOnly) {
  groupButton->setSelected(playerOnly);
  roundButton->setSelected(true);
  /*
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
  */
}

void Scourge::togglePlayerOnlyUI(bool playerOnly) {
  groupButton->setSelected(playerOnly);
}

  // initialization events
void Scourge::initStart(int statusCount, char *message) {
  progress = new Progress(this, statusCount, true, true);
  // Don't print text during startup. On windows this causes font corruption.
//  progress->updateStatus(message);
  progress->updateStatus(NULL);
}

void Scourge::initUpdate(char *message) {
  // Don't print text during startup. On windows this causes font corruption.  
//  progress->updateStatus(message);
  progress->updateStatus(NULL);  
}

void Scourge::initEnd() {
  delete progress;
}

void Scourge::createBoardUI() {
  // init gui
  /*
  boardWin = createWoodWindow((getSDLHandler()->getScreen()->w - BOARD_GUI_WIDTH) / 2, 
									   (getSDLHandler()->getScreen()->h - BOARD_GUI_HEIGHT) / 2, 
									   BOARD_GUI_WIDTH, BOARD_GUI_HEIGHT, 
									   "Available Missions");
*/                     
  boardWin = new Window( getSDLHandler(),
                        (getSDLHandler()->getScreen()->w - BOARD_GUI_WIDTH) / 2, 
                        (getSDLHandler()->getScreen()->h - BOARD_GUI_HEIGHT) / 2, 
                        BOARD_GUI_WIDTH, BOARD_GUI_HEIGHT, 
                        "Available Missions", true, Window::SIMPLE_WINDOW,
                        "wood" );
  missionList = new ScrollingList(5, 40, BOARD_GUI_WIDTH - 10, 150, getShapePalette()->getHighlightTexture());
  boardWin->addWidget(missionList);
  //missionDescriptionLabel = new Label(5, 210, "", 67);
  missionDescriptionLabel = new ScrollingLabel( 5, 210, 
                                                BOARD_GUI_WIDTH - 10, 
                                                BOARD_GUI_HEIGHT - Window::TOP_HEIGHT - Window::BOTTOM_HEIGHT - 210 - 10, "" );
  boardWin->addWidget(missionDescriptionLabel);
  playMission = new Button(5, 5, 105, 35, getShapePalette()->getHighlightTexture(), Constants::getMessage(Constants::PLAY_MISSION_LABEL));
  boardWin->addWidget(playMission);
  closeBoard = new Button(110, 5, 210, 35, getShapePalette()->getHighlightTexture(), Constants::getMessage(Constants::CLOSE_LABEL));
  boardWin->addWidget(closeBoard);
}

void Scourge::updateBoardUI(int count, const char **missionText, Color *missionColor) {
  missionList->setLines(count, missionText, missionColor);
}

int Scourge::handleBoardEvent(Widget *widget, SDL_Event *event) {
  if(widget == boardWin->closeButton || 
     widget == closeBoard) {
    boardWin->setVisible(false);
    return Board::EVENT_HANDLED;
  } else if(widget == missionList) {
    int selected = missionList->getSelectedLine();
    if(selected != -1 && selected < board->getMissionCount()) {
      missionDescriptionLabel->setText((char*)(board->getMission(selected)->getDescription()));
    }
    return Board::EVENT_HANDLED;
  } else if(widget == playMission) {
    int selected = missionList->getSelectedLine();
    if(selected != -1 && selected < board->getMissionCount()) {
      return Board::EVENT_PLAY_MISSION;
    }
    return Board::EVENT_HANDLED;
  }
  return -1;
}

void Scourge::setMissionDescriptionUI(char *s) {
  missionDescriptionLabel->setText(s);
}

void Scourge::removeBattle(Battle *b) {
  for(int i = 0; i < (int)battleRound.size(); i++) {
    Battle *battle = battleRound[i];
    if(battle == b) {
      delete battle;
      if(battleTurn > i) battleTurn--;
      for(int t = i; t < (int)battleRound.size() - 1; t++) {
        battle[t] = battle[t + 1];
      }
      return;
    }
  }
}

void Scourge::colorMiniMapPoint(int x, int y, Shape *shape, Location *pos) { 
  miniMap->colorMiniMapPoint(x, y, shape, pos); 
}

void Scourge::eraseMiniMapPoint(int x, int y) { 
  miniMap->eraseMiniMapPoint(x,y); 
}

void Scourge::resetBattles() {
  // delete battle references
  if(battleRound.size()) battleRound.erase(battleRound.begin(), battleRound.end());
  for(int i = 0; i < MAX_BATTLE_COUNT; i++) {
    battle[i] = NULL;
  }
  battleCount = 0;
  battleTurn = 0;
  inBattle = false;  
}  

void Scourge::checkForDropTarget() {
  // find the drop target
  if(movingItem) {

    // is the mouse moving?
    if(!getSDLHandler()->mouseIsMovingOverMap) {
      if(needToCheckDropLocation) {
        needToCheckDropLocation = false;

        // check location
        Location *dropTarget = NULL;
        Uint16 mapx = cursorMapX;
        Uint16 mapy = cursorMapY;
        Uint16 mapz = cursorMapZ;
        if(mapx < MAP_WIDTH) {
          dropTarget = levelMap->getLocation(mapx, mapy, mapz);
          if(!(dropTarget && 
               (dropTarget->creature || 
                (dropTarget->item && 
                 dropTarget->item->getRpgItem()->getType() == RpgItem::CONTAINER)))) {
            dropTarget = NULL;
          }      
        }
        levelMap->setSelectedDropTarget(dropTarget);  
      }
    } else {
      needToCheckDropLocation = true;
    }
  }  
}

void Scourge::showItemInfoUI(Item *item, int level) {
  infoGui->setItem( item, level );
  if(!infoGui->getWindow()->isVisible()) infoGui->getWindow()->setVisible( true );    
}

void Scourge::checkForInfo() {
  Uint16 mapx, mapy, mapz;

  // change cursor when over a hostile creature
  if( sdlHandler->getCursorMode() == SDLHandler::CURSOR_NORMAL || 
      sdlHandler->getCursorMode() == SDLHandler::CURSOR_ATTACK ||
      sdlHandler->getCursorMode() == SDLHandler::CURSOR_TALK ) {
    bool handled = false;
    mapx = cursorMapX;
    mapy = cursorMapY;
    mapz = cursorMapZ;
    if(mapx < MAP_WIDTH) {
      Location *pos = levelMap->getLocation(mapx, mapy, mapz);    
      if( pos && 
          pos->creature && 
          party->getPlayer()->canAttack( pos->creature ) ) {
        sdlHandler->setCursorMode( pos->creature->getMonster()->isNpc() ?
                                   SDLHandler::CURSOR_TALK :
                                   SDLHandler::CURSOR_ATTACK );
        handled = true;
      }
    }
    if( !handled ) sdlHandler->setCursorMode( SDLHandler::CURSOR_NORMAL );
  }  

  if( session->getUserConfiguration()->getTooltipEnabled() &&
      SDL_GetTicks() - getSDLHandler()->lastMouseMoveTime > 
      (Uint32)(session->getUserConfiguration()->getTooltipInterval() * 10) ) {
    if(needToCheckInfo) {
      needToCheckInfo = false;
      
      // check location
      Uint16 mapx = cursorMapX;
      Uint16 mapy = cursorMapY;
      Uint16 mapz = cursorMapZ;
      if(mapx < MAP_WIDTH) {
        Location *pos = levelMap->getLocation(mapx, mapy, mapz);
        if( pos ) {
          char s[300];
          void *obj = NULL;
          if( pos->creature ) {
            obj = pos->creature;
            pos->creature->getDetailedDescription(s);
          } else if( pos->item ) {
            obj = pos->item;
            pos->item->getDetailedDescription(s);
          } else if( pos->shape ) {
            obj = pos->shape;
            strcpy( s, session->getShapePalette()->getRandomDescription( pos->shape->getDescriptionGroup() ) );
          }
          if( obj ) {
            bool found = false;
            // Don't show info about the same object twice
            // FIXME: use lookup table
            for (map<InfoMessage *, Uint32>::iterator i=infos.begin(); i!=infos.end(); ++i) {
              InfoMessage *message = i->first;
              if( message->obj == obj || 
                  ( message->x == pos->x &&
                    message->y == pos->y &&
                    message->z == pos->z ) ) {
                found = true;
                break;
              }
            }
            if( !found ) {
              InfoMessage *message = 
                new InfoMessage( s, obj, pos->x, pos->y, 
                                 pos->z + pos->shape->getHeight() );
              infos[ message ] = SDL_GetTicks();
            }
          }
        }
      }
    } else {
      needToCheckInfo = true;
    }
  }
  // timeout descriptions
  Uint32 now = SDL_GetTicks();
  for (map<InfoMessage *, Uint32>::iterator i=infos.begin(); i!=infos.end(); ++i) {
    InfoMessage *message = i->first;
    Uint32 time = i->second;
    if( now - time > INFO_INTERVAL ) {
      delete message;
      infos.erase( i );
    }
  }
}

void Scourge::resetInfos() {
  for (map<InfoMessage *, Uint32>::iterator i=infos.begin(); i!=infos.end(); ++i) {
    InfoMessage *message = i->first;
    delete message;
    infos.erase( i );
  }
}

void Scourge::drawInfos() {
  float xpos2, ypos2, zpos2;
  for (map<InfoMessage *, Uint32>::iterator i=infos.begin(); i!=infos.end(); ++i) {

    InfoMessage *message = i->first;
    xpos2 = ((float)(message->x - levelMap->getX()) / GLShape::DIV);
    ypos2 = ((float)(message->y - levelMap->getY()) / GLShape::DIV);
    zpos2 = ((float)(message->z) / GLShape::DIV);

    getSDLHandler()->drawTooltip( xpos2, ypos2, zpos2, 
                                  -( levelMap->getZRot() ),
                                  -( levelMap->getYRot() ),
                                  message->message );
  }
}

void Scourge::createParty( Creature **pc, int *partySize ) { 
  mainMenu->createParty( pc, partySize ); 
}

void Scourge::teleport( bool toHQ ) {
  if( toHQ ) {
    if( !inHq ) {
      teleporting = true;
      exitLabel->setText(Constants::getMessage(Constants::TELEPORT_TO_BASE_LABEL));
      party->toggleRound(true);
      exitConfirmationDialog->setVisible(true);
    } else {
      this->showMessageDialog( "This spell has no effect here..." );
    }
  } else {
    // teleport to a random mission
    teleportFailure = true;
    changingStory = true;
    int selected = (int)( (float)( missionList->getLineCount() ) * rand() / RAND_MAX );
    nextMission = selected;
    oldStory = currentStory = 0;
    endMission();
    exitLabel->setText(Constants::getMessage(Constants::TELEPORT_TO_BASE_LABEL));
    party->toggleRound(true);
    exitConfirmationDialog->setVisible(true);
  }
}

