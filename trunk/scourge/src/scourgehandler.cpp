/***************************************************************************
                          scourgehandler.cpp  -  description
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
#include "scourgehandler.h"
#include "containergui.h"
#include "inventory.h"
#include "optionsmenu.h"
#include "scourge.h"
#include "render/renderlib.h"
#include "rpg/rpglib.h"
#include "item.h"
#include "creature.h"
#include "projectile.h"
#include "session.h"
#include "tradedialog.h"
#include "healdialog.h"
#include "donatedialog.h"
#include "traindialog.h"
#include "sqbinding/sqbinding.h"
#include "storable.h"
#include "shapepalette.h"
#include "debug.h"
#include "gui/confirmdialog.h"
#include "gui/textdialog.h"
#include "pceditor.h"
#include "savegamedialog.h"

#define DRAG_START_TOLERANCE 5

ScourgeHandler::ScourgeHandler( Scourge *scourge ) {
  this->scourge = scourge;
  willStartDrag = false;
  willStartDragX = willStartDragY = 0;
}

ScourgeHandler::~ScourgeHandler() {
}

bool ScourgeHandler::handleEvent(SDL_Event *event) {
  int ea;

  for( int i = 0; i < scourge->getContainerGuiCount(); i++ ) {
    if( scourge->getContainerGui( i )->handleEvent( event ) ) {
      scourge->closeContainerGui( i );
    }
  }

  if( scourge->getInventory()->isVisible() && 
      scourge->getInventory()->handleEvent(event) ) {
    return false;
  }

  if( scourge->getOptionsMenu()->isVisible()) {
    scourge->getOptionsMenu()->handleEvent(event);
    //    return false;
  }

  //if(multiplayer->isVisible()) {
//    multiplayer->handleEvent(event);
  //return false;
  //}

  scourge->getMap()->handleEvent( event );

  int mx, my;
  switch(event->type) {
  case SDL_MOUSEMOTION:

    if( !scourge->getMap()->isMouseRotating() ) {

      mx = event->motion.x;
      my = event->motion.y;

      // start the item drag
      if( willStartDrag &&
          ( abs( mx - willStartDragX ) > DRAG_START_TOLERANCE ||
            abs( my - willStartDragY ) > DRAG_START_TOLERANCE ) ) {
        // click on an item
        Uint16 mapx = scourge->getMap()->getCursorMapX();
        Uint16 mapy = scourge->getMap()->getCursorMapY();
        Uint16 mapz = scourge->getMap()->getCursorMapZ();
        if(mapx > MAP_WIDTH) {
          scourge->getMap()->getMapXYAtScreenXY(willStartDragX, willStartDragY, &mapx, &mapy);
          mapz = 0;
        }
        scourge->startItemDrag(mapx, mapy, mapz);
        willStartDrag = false;
      }
    }
    break;
  case SDL_MOUSEBUTTONDOWN:
    if( event->button.button == SDL_BUTTON_LEFT ) {
      // will start to drag when the mouse has moved
      willStartDrag = true;
      willStartDragX = scourge->getSDLHandler()->mouseX;
      willStartDragY = scourge->getSDLHandler()->mouseY;
    }
    break;
  case SDL_MOUSEBUTTONUP:
    if(event->button.button) {
      processGameMouseClick(scourge->getSDLHandler()->mouseX, scourge->getSDLHandler()->mouseY, event->button.button);
      scourge->mouseClickWhileExiting();
    }
    break;
  }
  switch(event->type) {
  case SDL_KEYDOWN:

    // DEBUG ------------------------------------

#ifdef DEBUG_KEYS
    if(event->key.keysym.sym == SDLK_d) {
      scourge->getParty()->getPlayer()->takeDamage( 1000 );
      return false;
    } else if(event->key.keysym.sym == SDLK_l) {
			if( scourge->getParty()->getPlayer()->getLevel() < MAX_LEVEL ) {
				scourge->getParty()->getPlayer()->setLevel( scourge->getParty()->getPlayer()->getLevel() + 1 );
				scourge->updatePartyUI();
				scourge->refreshInventoryUI();
			}
      return false;
    } else if(event->key.keysym.sym == SDLK_e) {
      // add a day
      //scourge->getSession()->getParty()->getCalendar()->addADay();
			scourge->getSession()->getMap()->toggleLightMap();
    } else if(event->key.keysym.sym == SDLK_f) {
      scourge->getMap()->useFrustum = ( scourge->getMap()->useFrustum ? false : true );
      scourge->getMap()->refresh();
    } else if(event->key.keysym.sym == SDLK_r &&
              scourge->getSession()->getCurrentMission() &&
              !scourge->getSession()->getCurrentMission()->isCompleted()) {
      scourge->completeCurrentMission();
    } else if( event->key.keysym.sym == SDLK_t ) {
      scourge->teleport();
    } else if( event->key.keysym.sym == SDLK_y ) {
      scourge->getBoard()->setStorylineIndex( scourge->getBoard()->getStorylineIndex() + 1 );
      cerr << "Incremented storyline index to " << scourge->getBoard()->getStorylineIndex() << endl;
			// init the missions board (this deletes completed missions)
      scourge->getBoard()->initMissions();
    } else if( event->key.keysym.sym == SDLK_u ) {
      scourge->getBoard()->setStorylineIndex( scourge->getBoard()->getStorylineIndex() - 1 );
      cerr << "Decremented storyline index to " << scourge->getBoard()->getStorylineIndex() << endl;
			// init the missions board (this deletes completed missions)
      scourge->getBoard()->initMissions();
    } else if(event->key.keysym.sym == SDLK_p) {
      cerr << "EFFECT!" << endl;
//      scourge->getParty()->startEffect( Constants::EFFECT_CAST_SPELL, (Constants::DAMAGE_DURATION * 4));

      scourge->getParty()->getPlayer()->startEffect( (int)( (float)Constants::EFFECT_COUNT * rand() / RAND_MAX ),
                                                     (Constants::DAMAGE_DURATION * 4) );

//    } else if(event->key.keysym.sym == SDLK_m) {
      //Map::debugMd2Shapes = ( Map::debugMd2Shapes ? false : true );
      //return false;
    } else if(event->key.keysym.sym == SDLK_b) {
      Battle::debugBattle = ( Battle::debugBattle ? false : true );
      return false;
    } else if(event->key.keysym.sym == SDLK_s) {
      scourge->getSquirrelConsole()->setVisible( scourge->getSquirrelConsole()->isVisible() ? false : true );
    }
#endif

    // END OF DEBUG ------------------------------------

  case SDL_KEYUP:

    if(event->type == SDL_KEYUP && event->key.keysym.sym == SDLK_ESCAPE){
      if( scourge->getInventory()->inStoreSpellMode() ) {
        scourge->getInventory()->setStoreSpellMode( false );
        return false;
      } else if( scourge->getTargetSelectionFor() ) {
        // cancel target selection ( cross cursor )
        scourge->getTargetSelectionFor()->cancelTarget();
        scourge->getTargetSelectionFor()->getBattle()->reset();
        scourge->setTargetSelectionFor( NULL );
        return false;
      } else if( scourge->getExitConfirmationDialog()->isVisible() ) {
        scourge->closeExitConfirmationDialog();
      } else if( !Window::anyFloatingWindowsOpen() ) {
        scourge->showExitConfirmationDialog();
      }
      return false;
    } else if( event->type == SDL_KEYUP && event->key.keysym.sym == SDLK_BACKSPACE ) {
      SDLHandler::showDebugInfo = ( SDLHandler::showDebugInfo ? 0 : 1 );
    }

    // xxx_yyy_stop means : "do xxx_yyy action when the corresponding key is up"
    ea = scourge->getUserConfiguration()->getEngineAction(event);
    if(ea == SWITCH_COMBAT) {
      scourge->resetBattles();
      scourge->getUserConfiguration()->setBattleTurnBased( scourge->getUserConfiguration()->isBattleTurnBased() ? false : true );
      char message[80];
      sprintf( message, "Combat is now %s.", ( scourge->getUserConfiguration()->isBattleTurnBased() ?
                                               "Turn-based" :
                                               "Real-time" ) );
      scourge->getMap()->addDescription( message, 0, 1, 1 );
			scourge->getTBCombatWin()->setVisible( scourge->inTurnBasedCombat(), false );
    } else if(ea == SET_PLAYER_0){
      scourge->setPlayer(0);
    } else if(ea == SET_PLAYER_1){
      scourge->setPlayer(1);
    } else if(ea == SET_PLAYER_2){
      scourge->setPlayer(2);
    } else if(ea == SET_PLAYER_3){
      scourge->setPlayer(3);
    } else if(ea == SET_PLAYER_ONLY && !scourge->inTurnBasedCombat()) {
      scourge->getParty()->togglePlayerOnly();
    }
    //    else if(ea == BLEND_A){
    else if(event->type == SDL_KEYUP && event->key.keysym.sym == SDLK_6){
      scourge->blendA++; if( scourge->blendA >= 11 ) scourge->blendA = 0;
      fprintf( stderr, "blend: a=%d b=%d\n", scourge->blendA, scourge->blendB );
    }
    //    else if(ea == BLEND_B){
    else if(event->type == SDL_KEYUP && event->key.keysym.sym == SDLK_7){
      scourge->blendB++; if( scourge->blendB >= 11 ) scourge->blendB = 0;
      fprintf( stderr, "blend: a=%d b=%d\n", scourge->blendA, scourge->blendB );
    } else if(ea == SHOW_INVENTORY){
      scourge->toggleInventoryWindow();
    } else if(ea == SHOW_OPTIONS_MENU){
      scourge->toggleOptionsWindow();
    } else if(ea == SET_NEXT_FORMATION_STOP){
      if(scourge->getParty()->getFormation() < Creature::FORMATION_COUNT - 1) scourge->getParty()->setFormation(scourge->getParty()->getFormation() + 1);
      else scourge->getParty()->setFormation(Constants::DIAMOND_FORMATION - Constants::DIAMOND_FORMATION);
    } else if(ea == TOGGLE_MINIMAP ){
      scourge->getMiniMap()->setShowMiniMap( scourge->getMiniMap()->isMiniMapShown() ? false : true );
    } else if( ea == NEXT_WEAPON && scourge->getParty()->getPlayer() ) {
      if( scourge->getParty()->getPlayer()->nextPreferredWeapon() ) {
        // reset but don't pause again
        scourge->getParty()->getPlayer()->getBattle()->reset( true );
      }
      if( scourge->getInventory()->isVisible() ) scourge->getInventory()->refresh();
    } else if(ea == TOGGLE_MAP_CENTER){
      bool mc;
      mc = scourge->getUserConfiguration()->getAlwaysCenterMap();
      scourge->getUserConfiguration()->setAlwaysCenterMap(!mc);
    } else if(ea == INCREASE_GAME_SPEED){
      scourge->addGameSpeed(-1);
    } else if(ea == DECREASE_GAME_SPEED){
      scourge->addGameSpeed(1);
    } else if(ea == START_ROUND) {
      scourge->getParty()->toggleRound();
    } else if(ea == LAYOUT_1) {
      scourge->setUILayout(Constants::GUI_LAYOUT_ORIGINAL);
    } else if(ea == LAYOUT_2) {
      scourge->setUILayout(Constants::GUI_LAYOUT_BOTTOM);
//    } else if(ea == LAYOUT_3) {
//      setUILayout(Constants::GUI_LAYOUT_SIDE);
    } else if(ea == LAYOUT_4) {
      scourge->setUILayout(Constants::GUI_LAYOUT_INVENTORY);
    } else if( ea >= QUICK_SPELL_1 && ea <= QUICK_SPELL_12 ) {
      quickSpellAction( ea - QUICK_SPELL_1 );
    }
    break;
  default: break;
  }

  return false;
}
  
bool ScourgeHandler::handleEvent(Widget *widget, SDL_Event *event) {
  if( widget == Window::message_button && 
      scourge->isInfoDialogShowing() ) {
    scourge->getParty()->toggleRound( false );
    scourge->setInfoDialogShowing( false );
    scourge->getSession()->getSquirrel()->startLevel();
    scourge->evalSpecialSkills();
    scourge->getParty()->startEffect( Constants::EFFECT_TELEPORT, 
                                      ( Constants::DAMAGE_DURATION * 4 ) );

		// save the party the first time around
		//cerr << "Saving party" << endl;
		if( !scourge->getSession()->isMultiPlayerGame() && 
				!strlen( scourge->getSession()->getSavegameName() ) ) {
			if( !scourge->getSaveDialog()->createNewSaveGame() ) {
				scourge->showMessageDialog( "Error saving game!" );
			}
		}
  }

  for(int i = 0; i < scourge->getContainerGuiCount(); i++) {
    if( scourge->getContainerGui(i)->handleEvent( widget, event ) ) {
      scourge->closeContainerGui( i );
    }
  }

  if( scourge->getInventory()->isVisible() ) {
    scourge->getInventory()->handleEvent( widget, event );
  }

  if( scourge->getOptionsMenu()->isVisible() ) {
    scourge->getOptionsMenu()->handleEvent( widget, event );
  }

  if( scourge->getNetPlay()->getWindow()->isVisible() ) {
    scourge->getNetPlay()->handleEvent( widget, event );
  }

  //if(multiplayer->isVisible()) {
  //    multiplayer->handleEvent(widget, event);
  //return false;
  //}

  if( scourge->getInfoGui()->getWindow()->isVisible() ) {
    scourge->getInfoGui()->handleEvent( widget, event );
  }

  if( scourge->getConversationGui()->getWindow()->isVisible() ) {
    scourge->getConversationGui()->handleEvent( widget, event );
  }

  if( scourge->getTradeDialog()->getWindow()->isVisible() ) {
    scourge->getTradeDialog()->handleEvent( widget, event );
  }

  if( scourge->getHealDialog()->getWindow()->isVisible() ) {
    scourge->getHealDialog()->handleEvent( widget, event );
  }

  if( scourge->getDonateDialog()->getWindow()->isVisible() ) {
    scourge->getDonateDialog()->handleEvent( widget, event );
  }

  if( scourge->getTrainDialog()->getWindow()->isVisible() ) {
    scourge->getTrainDialog()->handleEvent( widget, event );
  }

  if( scourge->getPcEditor()->getWindow()->isVisible() ) {
    scourge->getPcEditor()->handleEvent( widget,event );
  }

  if( scourge->getSaveDialog()->getWindow()->isVisible() ) {
    scourge->getSaveDialog()->handleEvent( widget, event );
  }

  // FIXME: this is hacky...
  if( handlePartyEvent( widget, event ) ) return true;
  int n = handleBoardEvent( widget, event );
  if( n == Board::EVENT_HANDLED ) return false;
  else if( n == Board::EVENT_PLAY_MISSION ) {
    if( scourge->playSelectedMission() ) return true;
  }

  if( widget == scourge->getYesExitConfirm() ) {
    scourge->movePartyToGateAndEndMission();
    return true;
  } else if( widget == scourge->getNoExitConfirm() ) {
    scourge->closeExitConfirmationDialog();
    return false;
  } else if( widget == scourge->getSquirrelRun() ||
             widget == scourge->getSquirrelText() ) {
    scourge->runSquirrelConsole();
  } else if( widget == scourge->getSquirrelClear() ) {
    scourge->clearSquirrelConsole();
  } else if( widget == scourge->getHireHeroDialog()->win->closeButton ||
						 widget == scourge->getHireHeroDialog()->cancelButton ) {
		scourge->getHireHeroDialog()->setVisible( false );
	} else if( widget == scourge->getHireHeroDialog()->okButton ) {
		scourge->getSession()->getParty()->
			hire( (Creature*)scourge->getHireHeroDialog()->getObject() );
		scourge->getHireHeroDialog()->setVisible( false );

	} else if( widget == scourge->getConfirmUpload()->win->closeButton ||
						 widget == scourge->getConfirmUpload()->cancelButton ) {
		scourge->getConfirmUpload()->setVisible( false );
	} else if( widget == scourge->getConfirmUpload()->okButton ) {
		scourge->getConfirmUpload()->setVisible( false );
		scourge->uploadScore();
	} else if( widget == scourge->getDismissHeroDialog()->win->closeButton ||
						 widget == scourge->getDismissHeroDialog()->cancelButton ) {
		scourge->getDismissHeroDialog()->setVisible( false );
	} else if( widget == scourge->getDismissHeroDialog()->okButton ) {
		scourge->getSession()->getParty()->
			dismiss( scourge->getDismissHeroDialog()->getMode() );
		//scourge->getDismissButton( scourge->getSession()->getParty()->
															 //getPartySize() )->
			//setTexture( 0 );
		scourge->getDismissHeroDialog()->setVisible( false );
  } else if( widget == scourge->getTextDialog()->win->closeButton ||
						 widget == scourge->getTextDialog()->okButton ) {
		scourge->getTextDialog()->setVisible( false );		
	} else if( widget == scourge->getPcEditor()->getOkButton() ) {
    scourge->getSession()->getParty()->
      hire( scourge->getPcEditor()->getCreature() );		
		//scourge->getDismissButton( scourge->getSession()->getParty()->
															 //getPartySize() - 1 )->
			//setTexture( scourge->getShapePalette()->getDismissTexture() );
		scourge->getPcEditor()->getWindow()->setVisible( false );
  } else if( widget == scourge->getPcEditor()->getCancelButton() ) {
		scourge->getPcEditor()->getWindow()->setVisible( false );
  }

  return false;
}

void ScourgeHandler::processGameMouseClick(Uint16 x, Uint16 y, Uint8 button) {
  // don't drag if you haven't started yet
  willStartDrag = false;

  Uint16 mapx, mapy, mapz;
  //Creature *c = getTargetSelectionFor();
  if(button == SDL_BUTTON_LEFT) {

    mapx = scourge->getMap()->getCursorMapX();
    mapy = scourge->getMap()->getCursorMapY();
    mapz = scourge->getMap()->getCursorMapZ();

    scourge->selectDropTarget( mapx, mapy, mapz );

    // clicking on a creature
    if( handleCreatureClick( mapx, mapy, mapz ) ) return;

    // click on an item
    if( mapx > MAP_WIDTH ) {
      mapx = scourge->getMap()->getCursorFlatMapX();
      mapy = scourge->getMap()->getCursorFlatMapY();
      mapz = 0;
    }

    if( scourge->getTargetSelectionFor() ) {
      Location *pos = scourge->getMap()->getLocation(mapx, mapy, mapz);
			Location *itemPos = scourge->getMap()->getItemLocation( mapx, mapy );
      if( mapx < MAP_WIDTH && pos && pos->item ) {
        scourge->handleTargetSelectionOfItem( ((Item*)(pos->item)), pos->x, pos->y, pos->z );
        return;
      } else if( mapx < MAP_WIDTH && itemPos && itemPos->item ) {
        scourge->handleTargetSelectionOfItem( ((Item*)(itemPos->item)), itemPos->x, itemPos->y, itemPos->z );
        return;
      } 
    }

    if( scourge->useItem( mapx, mapy, mapz ) ) return;

    // click on the scourge->getMap()
    mapx = scourge->getMap()->getCursorFlatMapX();
    mapy = scourge->getMap()->getCursorFlatMapY();

    // make sure the selected action can target a location
    if( scourge->getTargetSelectionFor() ) {
      scourge->handleTargetSelectionOfLocation( mapx, mapy, mapz );
      return;
    }

    // Make party move to new location
    int xx = mapx - ( scourge->getParty()->getPlayer()->getShape()->getWidth() / 2);
    int yy = mapy + 1 + ( scourge->getParty()->getPlayer()->getShape()->getHeight() / 2);
    if( !scourge->getParty()->setSelXY( xx, yy ) ) {
      scourge->getSDLHandler()->setCursorMode( Constants::CURSOR_FORBIDDEN, true );
    }

    // start round
    if( scourge->inTurnBasedCombatPlayerTurn() ) {
      if( scourge->getSDLHandler()->isDoubleClick ) {
        scourge->getParty()->toggleRound( false );
      }
    }

  } else if( button == SDL_BUTTON_RIGHT ) {
    scourge->describeLocation( scourge->getMap()->getCursorMapX(), 
                               scourge->getMap()->getCursorMapY(), 
                               scourge->getMap()->getCursorMapZ());
  }
}

bool ScourgeHandler::handleCreatureClick( Uint16 mapx, Uint16 mapy, Uint16 mapz ) {
  if( !scourge->getMovingItem() && mapx < MAP_WIDTH ) {
    Location *loc = scourge->getMap()->getLocation( mapx, mapy, mapz );
    if( loc && loc->creature ) {
      if( scourge->getTargetSelectionFor() ) {
        scourge->handleTargetSelectionOfCreature( ((Creature*)loc->creature) );
        return true;
      } else if( loc->creature->isMonster() &&
                 ((Creature*)(loc->creature))->getMonster()->isNpc() ) {
        // start a conversation
        scourge->getConversationGui()->start( ((Creature*)(loc->creature)) );
        return true;
      } else if( loc->creature->isMonster() ||
                 loc->creature->getStateMod( Constants::possessed ) ) {
        // follow this creature
        scourge->getParty()->setTargetCreature( ((Creature*)(loc->creature)) );
        // show path
        if( scourge->inTurnBasedCombatPlayerTurn() ) {
          // start round
          if( scourge->getSDLHandler()->isDoubleClick ) {
            scourge->getParty()->toggleRound( false );
          }
        }
        return true;
      } else {
        // select player
        for(int i = 0; i < scourge->getParty()->getPartySize(); i++) {
          if(scourge->getParty()->getParty(i) == loc->creature) {
            scourge->setPlayer(i);
            return true;
          }
        }
				// otherwise it's a wandering hero
				scourge->handleWanderingHeroClick( (Creature*)(loc->creature) );
				return true;
      }
    }
  }
  return false;
}

bool ScourgeHandler::handlePartyEvent(Widget *widget, SDL_Event *event) {
  if( widget == scourge->getInventoryButton() ) {
    scourge->toggleInventoryWindow();
  } else if( widget == scourge->getEndTurnButton() &&
            scourge->inTurnBasedCombatPlayerTurn() ) {
    scourge->endCurrentBattle();    
  } else if( widget == scourge->getOptionsButton() ) {
    scourge->toggleOptionsWindow();
  } else if( widget == scourge->getQuitButton() ) {
    scourge->showExitConfirmationDialog();
  } else if( widget == scourge->getGroupButton() && !scourge->inTurnBasedCombat()) {
    scourge->getParty()->togglePlayerOnly();
  } else if( widget == scourge->getRoundButton() ) {
    scourge->getParty()->toggleRound();
	} else if( widget == scourge->getIOButton() ) {
		scourge->getSaveDialog()->show( true );
  } else {
    for( int t = 0; t < scourge->getParty()->getPartySize(); t++ ) {
      if( widget == scourge->getPlayerInfo( t ) ) {
        if( scourge->getTargetSelectionFor() &&
						( scourge->getTargetSelectionFor()->getAction() != Constants::ACTION_CAST_SPELL ||
							( scourge->getTargetSelectionFor()->getAction() == Constants::ACTION_CAST_SPELL &&
								scourge->getTargetSelectionFor()->getActionSpell() &&
								scourge->getTargetSelectionFor()->getActionSpell()->isCreatureTargetAllowed() ) ) ) {
          scourge->handleTargetSelectionOfCreature( scourge->getParty()->getParty( t ) );
        } else {
          scourge->setPlayer( t );
        }
      } else if( widget == scourge->getPlayerHpMp( t ) ) {
        if( !scourge->inTurnBasedCombat() ||
            ( scourge->inTurnBasedCombat() &&
              scourge->getCurrentBattle()->getCreature() == scourge->getParty()->getParty( t ) ) ) {
          scourge->getInventory()->showSkills();
          if( scourge->getParty()->getPlayer() != scourge->getParty()->getParty( t ) ) {
            scourge->getParty()->setPlayer( t );
            if( !scourge->getInventory()->isVisible() ) scourge->toggleInventoryWindow();
          } else {
            scourge->toggleInventoryWindow();
          }
        }
      } else if( widget == scourge->getPlayerWeapon( t ) ) {
        if( !scourge->inTurnBasedCombat() ||
            ( scourge->inTurnBasedCombat() &&
              scourge->getCurrentBattle()->getCreature() == scourge->getParty()->getParty( t ) ) ) {
          if( scourge->getParty()->getPlayer() != scourge->getParty()->getParty( t ) ) {
            scourge->getParty()->setPlayer( t );
          }
          if( scourge->getParty()->getPlayer()->nextPreferredWeapon() ) {
            // reset but don't pause again
            scourge->getParty()->getPlayer()->getBattle()->reset( true );
          }
          if( scourge->getInventory()->isVisible() ) scourge->getInventory()->refresh();
        }
      } else if( widget == scourge->getDismissButton( t ) ) {
				scourge->handleDismiss( t );
			}
    }
    for( int t = 0; t < 12; t++ ) {
      if( widget == scourge->getQuickSpell( t ) ) {
        quickSpellAction( t, event->button.button );
      }
    }
  }
  return false;
}

void ScourgeHandler::quickSpellAction( int index, int button ) {
  if( scourge->getInventory()->inStoreSpellMode() ) {
    scourge->getParty()->getPlayer()->setQuickSpell( index, scourge->getInventory()->getStorable() );
    scourge->getInventory()->setStoreSpellMode( false );
    //if( scourge->getInventory()->isVisible() ) scourge->toggleInventoryWindow();
  } else {
    Creature *creature = scourge->getParty()->getPlayer();
    Storable *storable = creature->getQuickSpell( index );
    if( storable ) {
      if( storable->getStorableType() == Storable::SPELL_STORABLE ) {
        if( button == SDL_BUTTON_RIGHT ) {
          cerr << "*** FIXME: describe spell." << endl;
        } else {
          scourge->executeQuickSpell( (Spell*)storable );
        }
      } else if( storable->getStorableType() == Storable::SPECIAL_STORABLE ) {
        if( button == SDL_BUTTON_RIGHT ) {
          cerr << "*** FIXME: describe capability." << endl;
        } else {
          scourge->executeSpecialSkill( (SpecialSkill*)storable );
        }
      } else if( storable->getStorableType() == Storable::ITEM_STORABLE ) {
        Item *item = (Item*)storable;
        if( button == SDL_BUTTON_RIGHT ) {
          scourge->getInfoGui()->setItem( item );
          if( !scourge->getInfoGui()->getWindow()->isVisible() ) scourge->getInfoGui()->getWindow()->setVisible( true );
        } else {
          scourge->executeItem( item );
          // remove scroll from quick slot after use
          if( item->getRpgItem()->isScroll() ) {
          	scourge->getParty()->getPlayer()->setQuickSpell( index, NULL );
          }
        }
      } else {
        cerr << "*** Error: unknown storable type: " << storable->getStorableType() << endl;
      }
    } else {
      scourge->getInventory()->showSpells();
      if( !scourge->getInventory()->isVisible() ) scourge->toggleInventoryWindow();
    }
  }
}

int ScourgeHandler::handleBoardEvent(Widget *widget, SDL_Event *event) {
  if( widget == scourge->getBoardWin()->closeButton ||
      widget == scourge->getCloseBoard() ) {
    scourge->getBoardWin()->setVisible( false );
    return Board::EVENT_HANDLED;
  } else if( widget == scourge->getMissionList() ) {
    scourge->updateBoard();
    return Board::EVENT_HANDLED;
  } else if( widget == scourge->getPlayMission() ) {
    int selected = scourge->getMissionList()->getSelectedLine();
    if(selected != -1 && selected < scourge->getBoard()->getMissionCount()) {
      return Board::EVENT_PLAY_MISSION;
    }
    return Board::EVENT_HANDLED;
  }
  return -1;
}

