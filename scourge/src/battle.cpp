/***************************************************************************
                          battle.cpp  -  description
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

#include "battle.h"
#include "render/renderlib.h"
#include "rpg/rpglib.h"
#include "item.h"
#include "projectile.h"
#include "creature.h"
#include "shapepalette.h"
#include "debug.h"
#include "sqbinding/sqbinding.h"
#include "pathmanager.h"

using namespace std;

#define MIN_FUMBLE_RANGE 4.0f

#define IS_AUTO_CONTROL( creature ) ( ( creature->isMonster() || creature->getStateMod( StateMod::possessed ) ) )

bool Battle::debugBattle = DEBUG_BATTLE;

char *Battle::sound[] = {
  "sound/weapon-swish/handheld/sw1.wav",
  "sound/weapon-swish/handheld/sw2.wav",
  "sound/weapon-swish/handheld/sw3.wav",
  
  "sound/weapon-swish/bows/swb2.wav",
  "sound/weapon-swish/bows/swb3.wav",

  "sound/potion/pd1.wav"
};

int Battle::handheldSwishSoundStart = 0;
int Battle::handheldSwishSoundCount = 3;
int Battle::bowSwishSoundStart = 3;
int Battle::bowSwishSoundCount = 2;
int Battle::potionSoundStart = 5;
int Battle::potionSoundCount = 1;

enum {
  NO_ACTION = 0,
  LOITER,
  ATTACK,
  MOVE,
  NO_TARGET,
  WAIT,
};

Battle::Battle() {
  this->creature = NULL;
}

Battle::Battle(Session *session, Creature *creature) {
  this->session = session;
  this->creature = creature;
  this->item = NULL;
  this->creatureInitiative = 0;
  this->initiativeCheck = false;
  this->speed = 0;
  this->dist = creature->getDistanceToTarget();
  this->spell = NULL;
  this->empty = false;

  this->needsReset = true;
  this->nextTurn = 0;
  this->weaponWait = 0;
  this->startingAp = this->ap = 0;
  if(debugBattle) cerr << "*** constructor, creature=" << creature->getName() << endl;
}

Battle::~Battle() {
}

void Battle::reset( bool keepPaused, bool keepAP ) {
  if(debugBattle) cerr << "*** reset: creature=" << creature->getName() << endl;
  this->steps = 0;
  if( !keepAP ) this->startingAp = this->ap = toint( creature->getMaxAP() );
  this->projectileHit = false;
  if( !keepPaused ) this->paused = false;
  this->weaponWait = 0;
  this->range = 0.0f;
  creature->getShape()->setCurrentAnimation(static_cast<int>(MD2_STAND));
  ((AnimatedShape*)(creature->getShape()))->setAttackEffect(false);
}

void Battle::setupBattles(Session *session, Battle *battle[], int count, vector<Battle *> *turns) {
  // for now put all battles into the vector
  // need to order by initiative
  for(int i = 0; i < count; i++) {
    // reset for the first time
    // (to avoid whack skill numbers for un-initialized creatures.)
    if(battle[i]->needsReset) {
	  if( debugBattle ) cerr << "*** Reset 1" << endl;
      battle[i]->reset();
      battle[i]->needsReset = false;
    }
    bool handled = false;
    for( vector<Battle*>::iterator e = turns->begin(); e != turns->end(); ++e ) {
      Battle *b = *e;
      if( b->getCreature()->getInitiative() > battle[i]->getCreature()->getInitiative() ) {
        turns->insert( e, battle[i] );
        handled = true;
        break;
      }
    }
    if( !handled ) turns->push_back(battle[i]);
  }
  
  /*
  cerr << "Battle order:" << endl;
  for( vector<Battle*>::iterator e = turns->begin(); e != turns->end(); ++e ) {
    Battle *b = *e;
    cerr << "\t" << b->getCreature()->getName() << 
      " INIT:" << b->getCreature()->getInitiative() << 
      " speed:" << b->getCreature()->getSkill( Constants::SPEED ) <<
      endl;
  }
  cerr << "-----------------------------------" << endl;
  */
}                 

bool Battle::fightTurn() {
  if(debugBattle) cerr << "TURN: creature=" << creature->getName() << 
    " ap=" << ap << 
    " wait=" << weaponWait << 
    " nextTurn=" << nextTurn << 
		" hasTarget=" << creature->hasTarget() <<
		" validTarget=" << creature->isTargetValid() <<
		" action=" << creature->getAction() <<
		" motion=" << creature->getMotion() <<
		" animation=" << ((AnimatedShape*)creature->getShape())->getCurrentAnimation() <<
		" attacking=" << ((AnimatedShape*)creature->getShape())->getAttackEffect() << endl;

  // are we alive?
  if(!creature || creature->getStateMod(StateMod::dead) ||
     (creature->getAction() == Constants::ACTION_NO_ACTION &&
      !IS_AUTO_CONTROL( creature ) && !getAvailablePartyTarget()) ) {
	if( debugBattle ) cerr << "*** Reset 2" << endl;
    reset();
    return true;
  }

  // done with this creature's turn
  if(ap <= 0) {
    if(weaponWait > 0) {
      nextTurn = weaponWait;
      if(debugBattle) cerr << "Carries over into next turn." << endl;
    }
    // in TB battle, wait for the animations to end before ending turn
    int a;
    if( session->getPreferences()->isBattleTurnBased()) {
      a =((AnimatedShape*)(creature->getShape()))->getCurrentAnimation();
      if( !( a == MD2_STAND || a == MD2_RUN )) {
        return false;
      }
      if( creature->getTargetCreature() && 
          !creature->getTargetCreature()->getStateMod( StateMod::dead ) ) {
        a =((AnimatedShape*)(creature->getTargetCreature()->getShape()))->getCurrentAnimation();
        if( !( a == MD2_STAND || a == MD2_RUN )) {
          return false;
        }
      }
    }
    if( debugBattle ) cerr << "*** Reset 3" << endl;
    reset();
    return true;
  }

	// wait for projectile to finish
	if( creature->getProjectiles() &&
			!creature->getProjectiles()->empty() ) return false;

	// wait for the animation to finish
	int a = ((AnimatedShape*)creature->getShape())->getCurrentAnimation();
	if( !( a == MD2_STAND || a == MD2_RUN )) return false;
	

  // waiting to cast a spell?
  if( creature->getAction() == Constants::ACTION_CAST_SPELL || 
      creature->getAction() == Constants::ACTION_SPECIAL ) {
    creature->startEffect(Constants::EFFECT_CAST_SPELL, 
                          Constants::DAMAGE_DURATION * 4);
  }

  if(pauseBeforePlayerTurn()) return false;

  // in TB combat, light the nearest player
  if( IS_AUTO_CONTROL( creature ) && session->getPreferences()->isBattleTurnBased() ) {
    Creature *p = session->getParty()->getClosestPlayer( toint( creature->getX() ), 
                                                         toint( creature->getY() ), 
                                                         creature->getShape()->getWidth(),
                                                         creature->getShape()->getDepth(),
                                                         20 );
    if( p ) {
      for( int i = 0; i < session->getParty()->getPartySize(); i++ ) {
        if( p == session->getParty()->getParty(i) &&
            p != session->getParty()->getPlayer() ) {
          session->getParty()->setPlayer( i );
        }
      }
    }
  }

  initTurnStep( true );

  if(creature->getAction() == Constants::ACTION_EAT_DRINK) {
    executeEatDrinkAction();
  }

  if(creature->hasTarget() && 
     creature->isTargetValid()) {    
    if(dist < range) {
      executeAction();
    } else {
      stepCloserToTarget();
    }    
  } else {
    moveCreature();
  }
  // not done yet with creature's turn
  return false;
}

void Battle::endTurn() {
  ap = 0;
  session->getParty()->toggleRound(false);
  //paused = false;
}

bool Battle::pauseBeforePlayerTurn() {
  // go to single-player mode
  if (session->getPreferences()->isBattleTurnBased() &&
      !session->getParty()->isPlayerOnly()) {
    session->getParty()->togglePlayerOnly(true);
  }

  // pause if this is a player's first step
  if (!steps && 
      !paused &&
      session->getPreferences()->isBattleTurnBased()) {
    if(!IS_AUTO_CONTROL( creature )) {
      if(debugBattle) cerr << "Pausing for round start. Turn: " << creature->getName() << endl;

      // center on player
      for (int i = 0; i < session->getParty()->getPartySize(); i++) {
        if (session->getParty()->getParty(i) == creature &&
            !creature->getStateMod(StateMod::possessed)) {
          session->getParty()->setPlayer(i);
          break;
        }
      }
      // FIXME: only center if not on-screen
      session->getMap()->refresh();
      session->getMap()->center( toint(creature->getX()), toint(creature->getY()), true);

      // pause the game
      if( getAvailablePartyTarget() ) {
        session->getParty()->toggleRound(true);
        paused = true;
        return true;
      }
    } else {
      // FIXME: only center if not on-screen
      session->getMap()->refresh();
      session->getMap()->center( toint(creature->getX()), toint(creature->getY()), true);
    }
  }
  steps++;
  paused = false;
  return false;
}

int Battle::calculateRange( Item *item ) {
  int range;
  if( creature->getActionSkill() ) {
    //range = MIN_DISTANCE;
    range = creature->getActionSkill()->getDistance();
  } else if(creature->getActionSpell()) {
    //range = MIN_DISTANCE;
    range = creature->getActionSpell()->getDistance();
  } else {
    range = static_cast<int>(MIN_DISTANCE);
    if(item) range = item->getRange();
  }
  return range;
}

int Battle::getAdjustedWait( int originalWait ) {
	getSession()->getSquirrel()->setGlobalVariable( "turnWait", originalWait );
	if( creature->getActionSkill() ) {
		getSession()->getSquirrel()->callSkillEvent( creature, 
																								 creature->getActionSkill()->getName(), 
																								 "waitHandlerSkill" );
	} else if( creature->getActionSpell() ) {
		getSession()->getSquirrel()->callSpellEvent( creature, 
																								 creature->getActionSpell(), 
																								 "waitHandlerSpell" );
	} else {
		getSession()->getSquirrel()->callItemEvent( creature, 
																								item, 
																								"waitHandlerItem" );
	}
	int newWait = static_cast<int>(getSession()->getSquirrel()->getGlobalVariable( "turnWait" ));
	//if( originalWait != newWait ) {
		//cerr << "turnWait was " << originalWait << " and now is " << newWait << endl;
	//}
	return newWait;
}																										 	

void Battle::initTurnStep( bool callScript ) {
  dist = creature->getDistanceToTarget();

  // select the best weapon only once
  if(weaponWait <= 0) {
    if(debugBattle) cerr << "*** initTurnStep, creature=" << creature->getName() << " wait=" << weaponWait << " nextTurn=" << nextTurn << endl;

    if( creature->getActionSkill() ) {
      range = calculateRange();
      if(nextTurn > 0) weaponWait = nextTurn;
      else weaponWait = getAdjustedWait( creature->getActionSkill()->getSpeed() );
      nextTurn = 0;
      if(debugBattle) cerr << "\tUsing capability: " << creature->getActionSkill()->getDisplayName() << endl;
    } else if(creature->getActionSpell()) {
      range = calculateRange();
      if(nextTurn > 0) weaponWait = nextTurn;
      else weaponWait = getAdjustedWait( creature->getActionSpell()->getSpeed() );
      nextTurn = 0;
      if(debugBattle) cerr << "\tUsing spell: " << creature->getActionSpell()->getDisplayName() << endl;
    } else {
      item = creature->getBestWeapon(dist, callScript );      
      range = calculateRange( item );
      if(item) {
        if(debugBattle) cerr << "\tUsing item: " << item->getRpgItem()->getName() << " ap=" << ap << endl;
      } else {
        if(debugBattle) cerr << "\tUsing bare hands." << endl;
      }
      // How many steps to wait before being able to use the weapon.
      weaponWait = getAdjustedWait( toint( creature->getWeaponAPCost( item ) ) );
      // Make turn-based mode a little snappier
      //if( session->getPreferences()->isBattleTurnBased() ) {
      //  weaponWait /= 2;
      //}
    }
    if(nextTurn > 0) weaponWait = nextTurn;
    nextTurn = 0;
    if(debugBattle) cerr << "\tname=" << creature->getName() << " Distance=" << dist << " range=" << range << " wait=" << weaponWait << endl;
  
  }
}

void Battle::executeAction() {
  // try to heal someone
  if( !creature->getActionSpell() ) {
    if( creature->castHealingSpell() ) {
      if(debugBattle) cerr << "Stopping executeAction: Creature: " << creature->getName() << 
        " will heal " << creature->getTargetCreature()->getName() << endl;
      weaponWait = nextTurn = 0;
      return;
    }
  }

  ap--;
  if(weaponWait > 0) {
    weaponWait--;
    if(weaponWait > 0) return;
  }

  // don't carry to next turn
  nextTurn = 0;

  // attack
  if(debugBattle) cerr << "\t\t *** Attacking." << endl;
  if( creature->getActionSkill() ) {
    useSkill();
  } else if(creature->getActionSpell()) {
    // casting a spell for the first time
    castSpell();
  } else if(item && item->getRpgItem()->isRangedWeapon()) {
      launchProjectile();
  } else {
    hitWithItem();
  }
  creature->getShape()->setCurrentAnimation( static_cast<int>(MD2_ATTACK), true );
  ((AnimatedShape*)(creature->getShape()))->setAngle(creature->getTargetAngle());

	// pause after each hit
	steps = 0;
}

void Battle::stepCloserToTarget() {
  // try to heal someone
  if( !creature->getActionSpell() ) {
    if( creature->castHealingSpell() ) {
      if(debugBattle) cerr << "Stopping stepCloserToTarget: Creature: " << creature->getName() << 
        " will heal " << creature->getTargetCreature()->getName() << endl;
      weaponWait = nextTurn = 0;
      return;
    }
  }

  // out of range: take 1 step closer

  // re-select the best weapon
  weaponWait = nextTurn = 0;

  // Set the movement mode; otherwise character won't move
  creature->getShape()->setCurrentAnimation(static_cast<int>(MD2_RUN));

  if(debugBattle) cerr << "\t\tTaking a step." << endl;
  if(creature->getTargetCreature()) {
    if(debugBattle) cerr << "\t\t\tto target creature: " << creature->getTargetCreature()->getName() << endl;

    // has the target creature moved?
    int tx = toint(creature->getTargetCreature()->getX());
    int tw = creature->getTargetCreature()->getShape()->getWidth();
    int ty = toint(creature->getTargetCreature()->getY());
    int th = creature->getTargetCreature()->getShape()->getDepth();
    if( !( creature->getSelX() >= tx && creature->getSelX() < tx + tw &&
           creature->getSelY() <= ty && creature->getSelY() > ty - th ) ) {
/*
    int tx = toint(creature->getTargetCreature()->getX() + 
                   creature->getTargetCreature()->getShape()->getWidth() / 2);
    int ty = toint(creature->getTargetCreature()->getY() - 
                   creature->getTargetCreature()->getShape()->getDepth() / 2);
    if(!(creature->getSelX() == tx && creature->getSelY() == ty)) {
*/    

      // Try to move to the target creature.
      // For monsters, if this is not possible, select a new target.
      if( !creature->setSelCreature( creature->getTargetCreature(), range, false) &&
          IS_AUTO_CONTROL( creature ) ) {
        creature->cancelTarget();
        creature->decideMonsterAction();
        ap--;
        if(debugBattle) 
          cerr << "*** " << creature->getName() << 
          " selecting new target" << endl;
        return;
      }
    }
  } else if(!(creature->getSelX() == creature->getTargetX() &&
              creature->getSelY() == creature->getTargetY())) {
    if(debugBattle) cerr << "\t\t\tto target location: " << creature->getTargetX() <<
      creature->getTargetY() << endl;
     // if we are here, it means:
     //  a) we have no target creature
     //  b) we are going somewhere
     //  c) the place we are going is not showing a "Selected" symbol

     // this can happen when casting Dori's Tumblers in a battle with no creatures
  }

  // wait for animation to end
  int a =((AnimatedShape*)(creature->getShape()))->getCurrentAnimation();
  if( !( a == MD2_STAND || a == MD2_RUN )) {
    if(debugBattle) cerr << "\t\t\tWaiting for animation to end." << endl;
    return;
  }

  GLfloat oldX = creature->getX();
  GLfloat oldY = creature->getY();
  bool moved = ( creature->moveToLocator() == NULL );
  if( !( toint(oldX) == toint(creature->getX()) &&
         toint(oldY) == toint(creature->getY()) ) ) {
    ap--;
  }
  //if( !moved ) moveCreature();
  if( !moved ) {
    if( debugBattle ) {
      cerr << "*** Warning: not moving closer to target and not in range. " <<
        " x,y=" << creature->getX() << "," << creature->getY() <<
        " selX,selY=" << creature->getSelX() << "," << creature->getSelY() <<
        " dist=" << dist << " range=" << range << 
        " item=" << ( item ? item->getRpgItem()->getName() : "none" ) <<
        " creature=" << creature->getName() << 
        " target=" << ( creature->getTargetCreature() ? 
                        creature->getTargetCreature()->getName() : 
                        ( creature->getTargetItem() ? creature->getTargetItem()->getRpgItem()->getName() : 
                          ( creature->getTargetX() > -1 ? "location" : "no-target" ))) << endl;
      if( creature->getTargetCreature() ) {
        cerr << "Target creature=" << creature->getTargetCreature()->getX() <<
          "," << creature->getTargetCreature()->getY() << endl;
      }
    }
      
    // guess a new path (once in a while)
    if( 1 == Util::dice( 4 ) ) {
      if(creature->getTargetCreature()) //new path to the creature, if we have one targetted
        creature->setSelCreature( creature->getTargetCreature(), range,  false);
      else //new path to our selected location
        creature->setSelXY( creature->getSelX(), creature->getSelY(), false );
    }

    if( IS_AUTO_CONTROL( creature ) ) {      
      ap--;  
    } else {
      if( session->getPreferences()->isBattleTurnBased() ) {
        if( getAvailablePartyTarget() ) session->getParty()->toggleRound(true);
      } else {
        ap--;
      }
    }
  }
}

bool Battle::moveCreature() {

  // Set the movement mode; otherwise character won't move
  if(creature->anyMovesLeft()) {
    creature->getShape()->setCurrentAnimation(static_cast<int>(MD2_RUN));
  } else {
    creature->getShape()->setCurrentAnimation(static_cast<int>(MD2_STAND));
  }

  // take 1 step closer
  if(debugBattle) cerr << "\t\tTaking a non-battle step." << endl;

  GLfloat oldX = creature->getX();
  GLfloat oldY = creature->getY();
  if( IS_AUTO_CONTROL( creature ) ) {
    session->getGameAdapter()->moveMonster(creature);
    if( !( toint(oldX) == toint(creature->getX()) &&
           toint(oldY) == toint(creature->getY()) ) ) {
      ap--;
    } 
    
    if( oldX == creature->getX() &&
        oldY == creature->getY() ) {
      if( debugBattle ) cerr << "*** WARNING: monster not moving." << endl;
      ap--;
    }
    return false;
  } else {
  
		// in RT mode if the player has no target, make party follow player 
		// (ie. they are running away)
  	bool playerHasTarget = 
			( session->getPreferences()->isBattleTurnBased() ||
				session->getParty()->getPlayer()->hasTarget() &&
				session->getParty()->getPlayer()->isTargetValid() ? true : false );
		if( playerHasTarget ) {  	

	    // someone killed our target, try to pick another one
	    if(creature->hasTarget() && !creature->isTargetValid()) {
	      //cerr << "*** Character has invalid target, selecting new one." << endl;
	      if( selectNewTarget() ) return true;
	    }

	    // wait for animation to end
	    int a =((AnimatedShape*)(creature->getShape()))->getCurrentAnimation();
	    if( !( a == MD2_STAND || a == MD2_RUN )) return false;

	    if(creature->getSelX() != -1) {
	      bool moved = ( creature->moveToLocator() == NULL );
	      if( !( toint(oldX) == toint(creature->getX()) &&
	             toint(oldY) == toint(creature->getY()) ) ) {
	        ap--;
	      }
	      
	      if( !moved ) {
	        if( debugBattle ) {
	          cerr << "*** WARNING: character not moving." << 
	            " x,y=" << creature->getX() << "," << creature->getY() <<
	            " selX,selY=" << creature->getSelX() << "," << creature->getSelY() <<
	            " dist=" << dist << " range=" << range << 
	            " item=" << ( item ? item->getRpgItem()->getName() : "none" ) <<
	            " creature=" << creature->getName() << 
	            " target=" << ( creature->getTargetCreature() ? 
	                            creature->getTargetCreature()->getName() : 
	                            ( creature->getTargetItem() ? creature->getTargetItem()->getRpgItem()->getName() : 
	                              ( creature->getTargetX() > -1 ? "location" : "no-target" ))) << endl;
	          if( creature->getTargetCreature() ) {
	            cerr << "Target creature=" << creature->getTargetCreature()->getX() <<
	              "," << creature->getTargetCreature()->getY() << endl;
	          }
	        }

	        // guess a new path
	        if( 1 == Util::dice( 4 ) ) {
	          if(creature->getTargetCreature()) //new path to the creature, if we have one targetted
                    creature->setSelCreature( creature->getTargetCreature(), range, false);
                  else //new path to our selected location
                    creature->setSelXY( creature->getSelX(), creature->getSelY(), false );
	        }
	        if( session->getPreferences()->isBattleTurnBased() ) {          
	          if( getAvailablePartyTarget() ) session->getParty()->toggleRound(true);
	        } else {
	          // is the below line needed?
	          ap--;
	        }
	      }
	    } else {
	      //cerr << "*** Character has no location, selecting new target." << endl;
	      return selectNewTarget();
	    }
		} else {
			// don't select a new target
			// cerr << "*** Character just moving after player." << endl;
			creature->follow( session->getParty()->getPlayer() );
			// actually take a step
			creature->moveToLocator();
			if( creature == session->getParty()->getPlayer() ) {
				session->getMap()->center( toint( creature->getX() ), 
																	 toint( creature->getY() ) );
			}
		}
  }
  return false;
}

Creature *Battle::getAvailablePartyTarget() {
  for( int i = 0; i < session->getParty()->getPartySize(); i++) {
    bool visible = ( session->getMap()->isLocationVisible(toint(session->getParty()->getParty(i)->getX()), 
                                                          toint(session->getParty()->getParty(i)->getY())) &&
                     session->getMap()->isLocationInLight(toint(session->getParty()->getParty(i)->getX()), 
                                                          toint(session->getParty()->getParty(i)->getY()),
                                                          session->getParty()->getParty(i)->getShape()));
    if( visible && !session->getParty()->getParty(i)->getStateMod(StateMod::dead) ) {
      Creature *c = session->getParty()->getParty(i)->getBattle()->getAvailableTarget();
      if( c ) return c;
    }
  }
  return NULL;
}

Creature *Battle::getAvailableTarget() {
  if( creature->isMonster() ) {
    cerr << "*** Error: Battle::getAvailableTarget() should only be called for characters!" << endl;
    return NULL;
  }
  Creature *target;
  if(creature->getStateMod(StateMod::possessed)) {
    target = session->getParty()->getClosestPlayer(toint(creature->getX()), 
                                                   toint(creature->getY()), 
                                                   creature->getShape()->getWidth(),
                                                   creature->getShape()->getDepth(),
                                                   20);
  } else {
    target = session->getClosestVisibleMonster(toint(creature->getX()), 
                                               toint(creature->getY()), 
                                               creature->getShape()->getWidth(),
                                               creature->getShape()->getDepth(),
                                               20);
  }
  return target;
}

bool Battle::selectNewTarget() {
  // select a new target
  if( IS_AUTO_CONTROL( creature ) ) {    
    cerr << "*** Error Battle::selectNewTarget should not be called for monsters." << endl;
    return false;
  } else {
    // select a new target
    Creature *target = getAvailableTarget();
    if (target) {
      if(debugBattle) cerr << "\tSelected new target: " << target->getName() << ", with range " << range << endl;
      creature->setTargetCreature(target, true, range);
    } else {
      creature->setTargetCreature(NULL);
      if(debugBattle) cerr << "\t\tCan't find new target." << endl;
    }

    // let the player override in TB mode
    if( target && session->getPreferences()->isBattleTurnBased() ) {
      if( getAvailablePartyTarget() ) session->getParty()->toggleRound(true);
    }

    return ( target ? true : false );
  }
}

void Battle::useSkill() {
  snprintf(message, MESSAGE_SIZE, _( "%1$s uses capability %2$s!" ), 
          creature->getName(), 
          creature->getActionSkill()->getDisplayName());
  if ( creature->getCharacter() ) {
    session->getGameAdapter()->writeLogMessage( message, Constants::MSGTYPE_PLAYERMAGIC );
  } else {
    session->getGameAdapter()->writeLogMessage( message, Constants::MSGTYPE_NPCMAGIC );
  }
  ((AnimatedShape*)(creature->getShape()))->setAttackEffect(true);

  char *err = 
    creature->useSpecialSkill( creature->getActionSkill(), 
                               true );
  if( err ) {
    session->getGameAdapter()->writeLogMessage( err, Constants::MSGTYPE_FAILURE );
    //showMessageDialog( err );
  }

  // cancel action
  creature->cancelTarget();
  // also cancel path
  if( !IS_AUTO_CONTROL( creature ) ) creature->setSelXY( -1, -1 );
}

void Battle::castSpell( bool alwaysSucceeds ) {
  int casterLevel;
  // use up some MP (not when using a scroll)
  if(!creature->getActionItem()) {
    int n = creature->getMp() - creature->getActionSpell()->getMp();
    if(n < 0) n = 0;
    creature->setMp( n );
    casterLevel = creature->getLevel();
  } else {    
    casterLevel = creature->getActionItem()->getLevel();
    // try to destroy the scroll or use up a charge
    int itemIndex = creature->findInInventory(creature->getActionItem());
    if(itemIndex > -1) {
      if( creature->getActionItem()->getRpgItem()->getMaxCharges() > 0 ) {
				if( creature->getActionItem()->getCurrentCharges() <= 0 ) {
					snprintf(message,MESSAGE_SIZE, _( "Your %s is out of charges." ), creature->getActionItem()->getItemName() );
					session->getGameAdapter()->writeLogMessage( message );
					creature->cancelTarget();
					// also cancel path
					if( !IS_AUTO_CONTROL( creature ) ) creature->setSelXY( -1, -1 );
					return;
				} else {
					creature->getActionItem()->setCurrentCharges( 
						creature->getActionItem()->getCurrentCharges() - 1 );
					snprintf(message,MESSAGE_SIZE, _( "Your %s feels lighter." ), creature->getActionItem()->getItemName() );
					session->getGameAdapter()->writeLogMessage( message );
				}
      } else {
        creature->removeInventory(itemIndex);
        snprintf(message,MESSAGE_SIZE, _( "%s crumbles into dust." ), creature->getActionItem()->getItemName());
	session->getGameAdapter()->writeLogMessage( message );
      }
      if(!session->getGameAdapter()->isHeadless()) 
        session->getGameAdapter()->refreshInventoryUI();
    } else {
      // scroll was removed from inventory before casting
      snprintf( message,MESSAGE_SIZE, _( "Couldn't find scroll, cancelled spell." ) );
      session->getGameAdapter()->writeLogMessage( message, Constants::MSGTYPE_FAILURE );
      creature->cancelTarget();
      // also cancel path
      if( !IS_AUTO_CONTROL( creature ) ) creature->setSelXY( -1, -1 );
      return;
    }
  }

  snprintf(message,MESSAGE_SIZE, _( "%1$s casts %2$s!" ), 
          creature->getName(), 
          creature->getActionSpell()->getDisplayName());
    if ( creature->getCharacter() ) {
      session->getGameAdapter()->writeLogMessage( message, Constants::MSGTYPE_PLAYERMAGIC );
    } else {
      session->getGameAdapter()->writeLogMessage( message, Constants::MSGTYPE_NPCMAGIC );
    }
  ((AnimatedShape*)(creature->getShape()))->setAttackEffect(true);

  // spell succeeds?
  // FIXME: use stats like IQ here to modify spell success rate...
  SpellCaster *sc = new SpellCaster( this, creature->getActionSpell(), false, casterLevel );

  /* 
    apply state_mods:
    blessed, 
	  empowered, 
  	enraged, 
  	ac_protected, 
  	magic_protected, 
    
  	drunk, 
    
  	poisoned, 
  	cursed, 
  	possessed, 
  	blinded, 
  	charmed, 
  	changed,
  	overloaded,
  */
  float delta = 0.0f;
  if(creature->getStateMod(StateMod::blessed)) {
    delta += Util::roll( 0.0f, 10.0f );
  }
  if(creature->getStateMod(StateMod::empowered)) {
    delta += Util::roll( 5.0f, 15.0f );
  }
  if(creature->getStateMod(StateMod::enraged)) {
    delta -= Util::roll( 0.0f, 10.0f );
  }
  if(creature->getStateMod(StateMod::drunk)) {
    delta += Util::roll( -7.0f, 7.0f );
  }
  if(creature->getStateMod(StateMod::cursed)) {
    delta += Util::roll( 7.0f, 15.0f );
  }
  if(creature->getStateMod(StateMod::blinded)) {
    delta -= Util::roll( 0.0f, 10.0f );
  }
  if(creature->getStateMod(StateMod::overloaded)) {
    delta -= Util::roll( 0.0f, 8.0f );
  }
  if(creature->getStateMod(StateMod::invisible)) {
    delta += Util::roll( 5.0f, 10.0f );
  }

	// Like with max cth, max skill is closer to the skill to avoid a lot of misses
	int skill = creature->getSkill( creature->getActionSpell()->getSchool()->getSkill() );
	int maxSkill = skill * 2;
	if( maxSkill > 100 ) maxSkill = 100;
	if( maxSkill < 40 ) maxSkill = 40;

	bool failed = false;
	if( !alwaysSucceeds && !projectileHit ) {
	  if( Util::dice( maxSkill ) > skill + delta ) {
			snprintf( message,MESSAGE_SIZE, _( "...%s needs more practice." ), creature->getName() );
	        session->getGameAdapter()->writeLogMessage( message, Constants::MSGTYPE_FAILURE );
		failed = true;
	  } else if( Util::dice( 100 ) + delta < creature->getActionSpell()->getFailureRate() ) {
	        session->getGameAdapter()->writeLogMessage( _( "...the magic fails inexplicably!" ), Constants::MSGTYPE_FAILURE );
		failed = true;
	  }
	}
	
	if( failed ) {
	  sc->spellFailed();
	} else {
	  
	  // get exp for casting the spell
	  if( !IS_AUTO_CONTROL( creature ) ) {
		creature->addExperienceWithMessage( creature->getActionSpell()->getExp() );
	  }
	  
	  sc->spellSucceeded();
	}
	delete sc;
	
	// cancel action
	creature->cancelTarget();
	// also cancel path
	if( !IS_AUTO_CONTROL( creature ) ) creature->setSelXY( -1, -1 );
	
}

void Battle::launchProjectile() {
  if(!Projectile::addProjectile(creature, creature->getTargetCreature(), item, 
                                new ShapeProjectileRenderer( session->getShapePalette()->findShapeByName("ARROW") ),
                                creature->getMaxProjectileCount(item))) {
    snprintf(message,MESSAGE_SIZE, _( "...%s has finished firing a volley" ), creature->getName());
	if ( creature->getCharacter() ) {
	  session->getGameAdapter()->writeLogMessage( message, Constants::MSGTYPE_PLAYERBATTLE );
	} else {
	  session->getGameAdapter()->writeLogMessage( message, Constants::MSGTYPE_NPCBATTLE );
	}
  } else {
    snprintf(message,MESSAGE_SIZE, _( "...%s shoots a projectile" ), creature->getName());
	if ( creature->getCharacter() ) {
	  session->getGameAdapter()->writeLogMessage( message, Constants::MSGTYPE_PLAYERBATTLE );
	} else {
	  session->getGameAdapter()->writeLogMessage( message, Constants::MSGTYPE_NPCBATTLE );
	}
  }

  if( creature->isMonster() && 
     0 == Util::dice( session->getPreferences()->getSoundFreq() ) ) {
    int panning = session->getMap()->getPanningFromMapXY( creature->getX(), creature->getY() );
    session->playSound(creature->getMonster()->getRandomSound(Constants::SOUND_TYPE_ATTACK), panning);
  }

  int panning = session->getMap()->getPanningFromMapXY( creature->getX(), creature->getY() );
  session->playSound( getRandomSound(bowSwishSoundStart, bowSwishSoundCount), panning );
}

void Battle::projectileHitTurn(Session *session, Projectile *proj, Creature *target) {
  if(debugBattle) cerr << "*** Projectile hit (target): creature=" << proj->getCreature()->getName() << endl;  
  Creature *oldTarget = ((Creature*)(proj->getCreature()))->getTargetCreature();
  ((Creature*)(proj->getCreature()))->setTargetCreature(target);
  Battle *battle = ((Creature*)(proj->getCreature()))->getBattle();
  battle->projectileHit = true;
  if(proj->getItem()) {
    //battle->initItem(proj->getItem());
    battle->item = ((Item*)(proj->getItem()));
    battle->hitWithItem();
  } else if(proj->getSpell()) {
//    battle->spell = proj->getSpell();
//    battle->castSpell();
        
    SpellCaster *sc = new SpellCaster( battle, 
                                       proj->getSpell(), 
                                       true, 
                                       proj->getCasterLevel() );
    
    sc->spellSucceeded();
    delete sc;
  }
  battle->projectileHit = false;
  battle->spell = NULL;
  ((Creature*)(proj->getCreature()))->cancelTarget();
  ((Creature*)(proj->getCreature()))->setTargetCreature(oldTarget);
  // also cancel path
  if( !IS_AUTO_CONTROL( ((Creature*)(proj->getCreature())) ) ) 
    ((Creature*)(proj->getCreature()))->setSelXY( -1, -1 );
  if(debugBattle) cerr << "*** Projectile hit ends." << endl;
}

void Battle::projectileHitTurn(Session *session, Projectile *proj, int x, int y) {
  if(debugBattle) cerr << "*** Projectile hit (x,y): creature=" << proj->getCreature()->getName() << endl;
  // configure a turn
  ((Creature*)(proj->getCreature()))->setTargetLocation(x, y, 0);
  Battle *battle = ((Creature*)(proj->getCreature()))->getBattle();
  battle->projectileHit = true;
  if(proj->getItem()) {
    battle->item = ((Item*)(proj->getItem()));
    battle->hitWithItem();
  } else if(proj->getSpell() && 
            proj->getSpell()->isLocationTargetAllowed()) {
    
    SpellCaster *sc = new SpellCaster( battle, 
                                       proj->getSpell(), 
                                       true, 
                                       proj->getCasterLevel() ); 
    
    sc->spellSucceeded();
    delete sc;
  }
  battle->projectileHit = false;
  battle->spell = NULL;
  ((Creature*)(proj->getCreature()))->cancelTarget();
  if( !IS_AUTO_CONTROL( ((Creature*)(proj->getCreature())) ) ) 
    ((Creature*)(proj->getCreature()))->setSelXY( -1, -1 );
  if(debugBattle) cerr << "*** Projectile hit ends." << endl;
}



/**
 * Message to user about battle turn.
 */
void Battle::prepareToHitMessage() {
  if(item) {
    if( session->getPreferences()->getCombatInfoDetail() > 0 ) {
      snprintf(message,MESSAGE_SIZE, _( "%1$s attacks %2$s with %3$s!" ), 
              creature->getName(), 
              creature->getTargetCreature()->getName(),
              item->getItemName() );
    } else {
      snprintf( message,MESSAGE_SIZE, _( "%1$s attacks %2$s with %3$s!" ), 
               creature->getName(), 
               creature->getTargetCreature()->getName(),
               item->getItemName() );
    }
    if ( creature->getCharacter() ) {
      session->getGameAdapter()->writeLogMessage( message, Constants::MSGTYPE_PLAYERBATTLE );
    } else {
      session->getGameAdapter()->writeLogMessage( message, Constants::MSGTYPE_NPCBATTLE );
    }
    ((AnimatedShape*)(creature->getShape()))->setAttackEffect(true);

    // play item sound
    if( creature->isMonster() && 
       0 == Util::dice( session->getPreferences()->getSoundFreq() ) ) {
      int panning = session->getMap()->getPanningFromMapXY( creature->getX(), creature->getY() );
      session->playSound(creature->getMonster()->getRandomSound(Constants::SOUND_TYPE_ATTACK), panning);
    }
    int panning = session->getMap()->getPanningFromMapXY( creature->getX(), creature->getY() );
    session->playSound( getRandomSound(handheldSwishSoundStart, handheldSwishSoundCount), panning );

  } else {
    snprintf( message,MESSAGE_SIZE, _( "%1$s attacks %2$s with bare hands!" ), 
             creature->getName(), 
             creature->getTargetCreature()->getName() );
	if ( creature->getCharacter() ) {
	  session->getGameAdapter()->writeLogMessage( message, Constants::MSGTYPE_PLAYERBATTLE );
	} else {
	  session->getGameAdapter()->writeLogMessage( message, Constants::MSGTYPE_NPCBATTLE );
	}
    ((AnimatedShape*)(creature->getShape()))->setAttackEffect(true);
  }
}

/**
 * Special actions for very low attack roll. If the attack roll
 * is below 10% of the max and the attack range is greater than 4pts,
 * roll to see if there is a fumble.
 */
bool Battle::handleLowAttackRoll( float attack, float min, float max ) {
  if( max - min >= MIN_FUMBLE_RANGE && 
      creature->getTargetCreature() &&
      attack - min < ( ( ( max - min ) / 100.0f ) * 5.0f ) ) {
    if( 0 == Util::dice( 3 ) ) {
      Creature *tmpTarget;
      if( IS_AUTO_CONTROL( creature ) ) {
        tmpTarget = session->
          getClosestVisibleMonster( toint(creature->getX()), 
                                    toint(creature->getY()), 
                                    creature->getShape()->getWidth(),
                                    creature->getShape()->getDepth(),
                                    20 );
      } else {
        tmpTarget = session->getParty()->
          getClosestPlayer( toint(creature->getX()), 
                            toint(creature->getY()), 
                            creature->getShape()->getWidth(),
                            creature->getShape()->getDepth(),
                            20 );
      }
      if( tmpTarget ) {
        // play item sound
	int panning = session->getMap()->getPanningFromMapXY( creature->getX(), creature->getY() );
        if(item) session->playSound(item->getRandomSound(), panning);
        snprintf( message,MESSAGE_SIZE, _( "...fumble: hits %s instead!" ), tmpTarget->getName() );
        session->getGameAdapter()->writeLogMessage( message, Constants::MSGTYPE_FAILURE );
        Creature *oldTarget = creature->getTargetCreature();
        creature->setTargetCreature( tmpTarget );

				char tmp[255];
				snprintf( tmp,255, 
					( creature->getSex() == Constants::SEX_MALE ? _( "%s his own fumbling hands" ) : _( "%s her fumbling hands" ) ), 
					Constants::getMessage( Constants::CAUSE_OF_DEATH ) 
				);
				creature->setPendingCauseOfDeath( tmp );

        dealDamage( Util::roll( 0.5f * MIN_FUMBLE_RANGE, 1.5f * MIN_FUMBLE_RANGE ) );
        creature->setTargetCreature( oldTarget );
        return true;
      }
    }
  }
  return false;
}

void Battle::applyHighAttackRoll( float *damage, float attack, float min, float max ) {
  float percent = ( attack - min ) / ( ( max - min ) / 100.0f );
  // special actions for very high tohits
  if( max - min >= MIN_FUMBLE_RANGE && 
      percent > 95 ) {
    int mul = Util::dice( 8 );
    switch( mul ) {
    case 2:
    strcpy(message, _( "...precise hit: double damage!" ) );
	if ( creature->getCharacter() ) {
	  session->getGameAdapter()->writeLogMessage( message, Constants::MSGTYPE_NPCDAMAGE );
	} else {
	  session->getGameAdapter()->writeLogMessage( message, Constants::MSGTYPE_PLAYERDAMAGE );
	}
    (*damage) *= mul;
    break;

    case 3:
    strcpy(message, _( "...precise hit: triple damage!" ) );
	if ( creature->getCharacter() ) {
	  session->getGameAdapter()->writeLogMessage( message, Constants::MSGTYPE_NPCDAMAGE );
	} else {
	  session->getGameAdapter()->writeLogMessage( message, Constants::MSGTYPE_PLAYERDAMAGE );
	}
    (*damage) *= mul;
    break;

    case 4:
    strcpy(message, _( "...precise hit: quad damage!" ) );
	if ( creature->getCharacter() ) {
	  session->getGameAdapter()->writeLogMessage( message, Constants::MSGTYPE_NPCDAMAGE );
	} else {
	  session->getGameAdapter()->writeLogMessage( message, Constants::MSGTYPE_PLAYERDAMAGE );
	}
    (*damage) *= mul;
    break;

    case 0:
    if( percent >= 98 ) {
      strcpy(message, _( "...precise hit: instant kill!" ) );
	if ( creature->getCharacter() ) {
	  session->getGameAdapter()->writeLogMessage( message, Constants::MSGTYPE_NPCDEATH );
	} else {
	  session->getGameAdapter()->writeLogMessage( message, Constants::MSGTYPE_PLAYERDEATH );
	}
      (*damage) = 10000;
    }
    break;

    default:
    break;
    }
  }
}

void Battle::applyMagicItemDamage( float *damage ) {
  // magical weapons
  if( item && item->isMagicItem() ) {
    int mul = 1;
    if( item->getMonsterType() && creature->getTargetCreature() && 
        !strcmp( item->getMonsterType(), 
                 creature->getTargetCreature()->getModelName() ) ) {
      mul = item->getDamageMultiplier();
    } else if( !item->getMonsterType() ) {
      mul = item->getDamageMultiplier();
    }
    if( mul < 1 ) mul = 1;
    if( mul == 2 ) {
      strcpy( message, _( "...double damage!" ) );
	if ( creature->getCharacter() ) {
	  session->getGameAdapter()->writeLogMessage( message, Constants::MSGTYPE_NPCDAMAGE );
	} else {
	  session->getGameAdapter()->writeLogMessage( message, Constants::MSGTYPE_PLAYERDAMAGE );
	}
    } else if( mul == 3 ) {
      strcpy( message, _( "...triple damage!" ) );
	if ( creature->getCharacter() ) {
	  session->getGameAdapter()->writeLogMessage( message, Constants::MSGTYPE_NPCDAMAGE );
	} else {
	  session->getGameAdapter()->writeLogMessage( message, Constants::MSGTYPE_PLAYERDAMAGE );
	}
    } else if( mul == 4 ) {
      strcpy( message, _( "...quad damage!" ) );
	if ( creature->getCharacter() ) {
	  session->getGameAdapter()->writeLogMessage( message, Constants::MSGTYPE_NPCDAMAGE );
	} else {
	  session->getGameAdapter()->writeLogMessage( message, Constants::MSGTYPE_PLAYERDAMAGE );
	}
    } else if( mul > 4 ) {
      snprintf( message,MESSAGE_SIZE, _( "...%d-times damage!" ), mul );
	if ( creature->getCharacter() ) {
	  session->getGameAdapter()->writeLogMessage( message, Constants::MSGTYPE_NPCDAMAGE );
	} else {
	  session->getGameAdapter()->writeLogMessage( message, Constants::MSGTYPE_PLAYERDAMAGE );
	}
    }
    (*damage) *= mul;
  }
}

float Battle::applyMagicItemSpellDamage() {
  // magical damage
  if( item && 
      item->isMagicItem() && 
      item->getSchool() &&
      creature->getTargetCreature() && 
      creature->isTargetValid() ) {
    
    // roll for the spell damage
    float damage = creature->rollMagicDamagePercent( item );

    // check for resistance
    int resistance = 
      creature->getTargetCreature()->
      getSkill( item->getSchool()->getResistSkill() );

    // reduce the magic attack by the resistance %.
    damage -= (damage / 100.0f) * static_cast<float>(resistance);
    if( damage < 0 ) damage = 0;

    char msg[ MESSAGE_SIZE ];
    snprintf( msg, MESSAGE_SIZE, _( "...%1$s attacks %2$s with %3$s magic." ), 
             creature->getName(), 
             creature->getTargetCreature()->getName(),
             item->getSchool()->getShortName() );
	if ( creature->getCharacter() ) {
	  session->getGameAdapter()->writeLogMessage( message, Constants::MSGTYPE_PLAYERMAGIC );
	} else {
	  session->getGameAdapter()->writeLogMessage( message, Constants::MSGTYPE_NPCMAGIC );
	}
    if( resistance > 0 ) {
      snprintf( msg, MESSAGE_SIZE, _( "%s resists the magic with %d." ), 
               creature->getTargetCreature()->getName(),
               resistance );
	if ( creature->getTargetCreature()->getCharacter() ) {
	  session->getGameAdapter()->writeLogMessage( message, Constants::MSGTYPE_PLAYERBATTLE );
	} else {
	  session->getGameAdapter()->writeLogMessage( message, Constants::MSGTYPE_NPCBATTLE );
	}
    }
    
    return damage;
  }
  return -1.0f;
}

void Battle::hitWithItem() {
  prepareToHitMessage();

	float cth, skill;
	creature->getCth( item, &cth, &skill );

	if( cth <= skill ) {
		float dodge = creature->getTargetCreature()->getDodge( creature, item );
		if( cth <= skill - dodge ) {
			// a hit?
	
			// Shield/weapon parry
			Item *parryItem = NULL;
			float parry = creature->getTargetCreature()->getParry( &parryItem );
			if( parry > cth ) {
				snprintf( message, MESSAGE_SIZE, _( "...%1$s blocks attack with %2$s!" ), 
								 creature->getTargetCreature()->getName(),
								 parryItem->getName() );
				if ( creature->getTargetCreature()->getCharacter() ) {
				  session->getGameAdapter()->writeLogMessage( message, Constants::MSGTYPE_PLAYERBATTLE );
				} else {
				  session->getGameAdapter()->writeLogMessage( message, Constants::MSGTYPE_NPCBATTLE );
				}
			} else {
				// a hit!
			
				// calculate attack value
				float max, min;
				float attack = creature->getAttack( item, &max, &min, true );
				float delta = creature->getAttackerStateModPercent();
				float extra = ( attack / 100.0f ) * delta;
				attack += extra;
	
				// cursed items
				if( item && item->isCursed() ) {
				        session->getGameAdapter()->writeLogMessage( _( "...Using cursed item!" ), Constants::MSGTYPE_FAILURE );
					attack -= ( attack / 3.0f );
				}
	
				snprintf( message, MESSAGE_SIZE, _( "...%s attacks for %d points." ), 
								 creature->getName(), toint( attack ) );
					if ( creature->getCharacter() ) {
					  session->getGameAdapter()->writeLogMessage( message, Constants::MSGTYPE_PLAYERBATTLE );
					} else {
					  session->getGameAdapter()->writeLogMessage( message, Constants::MSGTYPE_NPCBATTLE );
					}
				if( session->getPreferences()->getCombatInfoDetail() > 0 ) {
					snprintf(message, MESSAGE_SIZE, _( "...DAM:%.2f-%.2f extra:%.2f" ),
									min, max, extra );
					if ( creature->getCharacter() ) {
					  session->getGameAdapter()->writeLogMessage( message, Constants::MSGTYPE_PLAYERBATTLE );
					} else {
					  session->getGameAdapter()->writeLogMessage( message, Constants::MSGTYPE_NPCBATTLE );
					}
				}
	
				// very low attack rolls (fumble)
				if( handleLowAttackRoll( attack, min, max ) ) return;
				
				// get the armor
				float armor, dodgePenalty;
				creature->getTargetCreature()->
					getArmor( &armor, &dodgePenalty, 
										item ? item->getRpgItem()->getDamageType() : 0,
										item );
				if( toint( armor ) > 0 ) {
					snprintf( message, MESSAGE_SIZE, _( "...%s's armor blocks %d points" ), 
									 creature->getTargetCreature()->getName(), toint( armor ) );
					if ( creature->getTargetCreature()->getCharacter() ) {
					  session->getGameAdapter()->writeLogMessage( message, Constants::MSGTYPE_PLAYERBATTLE );
					} else {
					  session->getGameAdapter()->writeLogMessage( message, Constants::MSGTYPE_NPCBATTLE );
					}
				}
								
				float damage = ( armor > attack ? 0 : attack - armor );
				if( damage > 0 ) {
					// play item sound
					int panning = session->getMap()->getPanningFromMapXY( creature->getX(), creature->getY() );
					if( item ) session->playSound( item->getRandomSound(), panning );
		
					applyMagicItemDamage( &damage );
		
					applyHighAttackRoll( &damage, attack, min, max );	
				}
		
				// item attack event handler
				if( item ) {
					getSession()->getSquirrel()->setGlobalVariable( "damage", damage );
					getSession()->getSquirrel()->callItemEvent( creature, item, "damageHandler" );
					damage = getSession()->getSquirrel()->getGlobalVariable( "damage" );
				}

				char tmp[255];
				strcpy( tmp, Constants::getMessage( Constants::CAUSE_OF_DEATH ) );
				strcat( tmp, " " );
				if( creature->isMonster() ) {
					strcat( tmp, getAn( creature->getMonster()->getType() ) );
					strcat( tmp, " " );
					strcat( tmp, creature->getMonster()->getType() );
				} else {
					strcat( tmp, creature->getName() );
				}
				if( item ) {
					strcat( tmp, _( " weilding " ) );
					strcat( tmp, getAn( item->getName() ) );
					strcat( tmp, " " );
					strcat( tmp, item->getName() );
				} else {
					strcat( tmp, _( " in a flurry of blows" ) );
				}
				creature->getTargetCreature()->setPendingCauseOfDeath( tmp );
		
				dealDamage( damage );
				
				//lordtoran's testing cheat
				//if ( !creature->isMonster() ) dealDamage( 9999 );

				if( damage > 0 ) {
					// apply extra spell-like damage of magic items
					float spellDamage = applyMagicItemSpellDamage();
					if( spellDamage > -1 ) {

						strcpy( tmp, Constants::getMessage( Constants::CAUSE_OF_DEATH ) );
						strcat( tmp, _( " magical damage by " ) );
						if( creature->isMonster() ) {
							strcat( tmp, getAn( creature->getMonster()->getType() ) );
							strcat( tmp, " " );
							strcat( tmp, creature->getMonster()->getType() );
						} else {
							strcat( tmp, creature->getName() );
						}
						creature->getTargetCreature()->setPendingCauseOfDeath( tmp );

						dealDamage( damage, Constants::EFFECT_GREEN, true );
					}
				}
			}
		} else {
			// a miss
			snprintf(message, MESSAGE_SIZE, _( "...%s dodges the attack." ), 
							creature->getTargetCreature()->getName() );
				if ( creature->getTargetCreature()->getCharacter() ) {
				  session->getGameAdapter()->writeLogMessage( message, Constants::MSGTYPE_PLAYERBATTLE );
				} else {
				  session->getGameAdapter()->writeLogMessage( message, Constants::MSGTYPE_NPCBATTLE );
				}
		}
	} else {
		// a miss
		snprintf(message, MESSAGE_SIZE, _( "...%s misses the target." ), creature->getName() );
        	session->getGameAdapter()->writeLogMessage( message, Constants::MSGTYPE_FAILURE );
	}
}

void Battle::dealDamage( float damage, int effect, bool magical, GLuint delay ) {

	// monsters fight back
	if( creature->getTargetCreature() && IS_AUTO_CONTROL( creature->getTargetCreature() ) &&
			!creature->getTargetCreature()->hasTarget() ) {
		if(debugBattle)
			cerr << creature->getTargetCreature()->getName() << " fights back!" << endl;
		creature->getTargetCreature()->setTargetCreature( creature );
	}

	if( toint( damage ) > 0 ) {
		// also affects spell attacks
		float delta = creature->getDefenderStateModPercent(magical);
		float extra = (static_cast<float>(damage) / 100.0f) * delta;
		damage += extra;

		snprintf(message, MESSAGE_SIZE, _( "...%s hits for %d points of damage" ), creature->getName(), toint( damage ) );
			if ( creature->getTargetCreature()->getCharacter() ) {
			  session->getGameAdapter()->writeLogMessage( message, Constants::MSGTYPE_PLAYERDAMAGE );
			} else {
			  session->getGameAdapter()->writeLogMessage( message, Constants::MSGTYPE_NPCDAMAGE );
			}

		// play hit sound
		if(damage > 0) {
			if(creature->getTargetCreature()->isMonster()) {
				char const*soundname = creature->getTargetCreature()->getMonster()->getRandomSound(Constants::SOUND_TYPE_HIT);
					if (soundname) {
						int panning = session->getMap()->getPanningFromMapXY( creature->getTargetCreature()->getX(), creature->getTargetCreature()->getY() );
						session->playSound(soundname, panning);
					}
			} else {
				//session->playSound(creature->getTargetCreature()->getCharacter()->getRandomSound(Constants::SOUND_TYPE_HIT));
				int panning = session->getMap()->getPanningFromMapXY( creature->getTargetCreature()->getX(), creature->getTargetCreature()->getY() );
				creature->getTargetCreature()->playCharacterSound( GameAdapter::HIT_SOUND, panning );
			}
		}

    /*
			Note: in case of a creature hitting itself (like a spell fumble)
			creature->getTargetCreature() will return null after creature death.
			Having a ptr to the original is needed.
		*/
		Creature *tc = creature->getTargetCreature(); 

		// target creature death
		if( tc->takeDamage( toint( damage ), effect, delay ) ) {
      // only in RT mode... otherwise in TB mode character won't move
			if( !session->getPreferences()->isBattleTurnBased() )
				creature->getShape()->setCurrentAnimation(static_cast<int>(MD2_TAUNT)); 

			snprintf(message, MESSAGE_SIZE, _( "...%s is killed!" ), tc->getName());
				if ( creature->getCharacter() ) {
				  session->getGameAdapter()->writeLogMessage( message, Constants::MSGTYPE_PLAYERDEATH );
				} else {
				  session->getGameAdapter()->writeLogMessage( message, Constants::MSGTYPE_NPCDEATH );
				}
			if( session->getParty()->isPartyMember( tc ) ) 
				session->getGameAdapter()->writeLogMessage( tc->getCauseOfDeath(), Constants::MSGTYPE_PLAYERDEATH );

			// add exp. points and money
			if( !IS_AUTO_CONTROL( creature ) ) {

				// FIXME: try to move to party.cpp
				for(int i = 0; i < session->getParty()->getPartySize(); i++) {
					// Add the exp for the killed creature
					session->getParty()->getParty( i )->addExperience( tc );
					if( tc->isBoss() ) {
						// Bosses give double the experience
						session->getParty()->getParty( i )->addExperience( tc );
					}
					if(!session->getParty()->getParty(i)->getStateMod(StateMod::dead)) {
						int n = session->getParty()->getParty(i)->addMoney( tc );
						if(n > 0) {
							snprintf(message, MESSAGE_SIZE, _( "%s finds %d coins!" ), session->getParty()->getParty(i)->getName(), n);
							session->getGameAdapter()->writeLogMessage( message );
						}
					}
				}
				// end of FIXME

				if( tc->isBoss() ) {
					session->getGameAdapter()->writeLogMessage( _( "You have defeated the dungeon boss!" ), Constants::MSGTYPE_MISSION );
					session->getGameAdapter()->writeLogMessage( _( "...the news spreads quickly and his minions cower before you." ), Constants::MSGTYPE_MISSION );
				}

				// see if this is a mission objective
				if(session->getCurrentMission() && tc->getMonster() && session->getCurrentMission()->creatureSlain( tc )) {
					session->getGameAdapter()->missionCompleted();
				}
			}
		}
	} else {
		snprintf(message, MESSAGE_SIZE, _( "...no damaged caused." ) );
			if ( creature->getCharacter() ) {
			  session->getGameAdapter()->writeLogMessage( message, Constants::MSGTYPE_PLAYERBATTLE );
			} else {
			  session->getGameAdapter()->writeLogMessage( message, Constants::MSGTYPE_NPCBATTLE );
			}
	}
}

/*
void Battle::initItem(Item *item) {
  this->item = item;

  // (!item) is a bare-hands attack		
  speed = (item ? item->getSpeed() : Constants::HAND_WEAPON_SPEED) * 
          (session->getPreferences()->getGameSpeedTicks() + 80);
  //	(scourge->getPreferences()->getGameSpeedTicks() + 80);

  creatureInitiative = creature->getInitiative(item);
}
*/

void Battle::executeEatDrinkAction() {
  // is it still in the inventory?
  int index = creature->findInInventory(creature->getActionItem());
  if(index > -1) {
    int panning = session->getMap()->getPanningFromMapXY( creature->getX(), creature->getY() );
    session->playSound( getRandomSound(potionSoundStart, potionSoundCount), panning );
    if(creature->eatDrink(creature->getActionItem())){
      creature->removeInventory(index);
      if(!session->getGameAdapter()->isHeadless()) 
        session->getGameAdapter()->refreshInventoryUI();
    }
  }
  // cancel action
  creature->cancelTarget();
  if( !IS_AUTO_CONTROL( creature ) ) creature->setSelXY( -1, -1 );
}

void Battle::invalidate() {
  if(debugBattle) cerr << "*** invalidate: creature=" << getCreature()->getName() << endl;
  nextTurn = weaponWait = 0;
}

char *Battle::getRandomSound(int start, int count) {
  if(count)
    return sound[ start + Util::dice( count ) ];
  else return NULL;
}

bool Battle::describeAttack( Creature *target, char *buff, size_t buffSize, Color *color, bool includeActions ) {

  initTurnStep();

  // info for the player?
  if( includeActions && session->getParty()->getPlayer() == creature ) {
    if( creature->getAction() == Constants::ACTION_CAST_SPELL ) {
      snprintf( buff, buffSize, "%s: %d", 
               creature->getActionSpell()->getDisplayName(), 
               ( nextTurn > 0 ? nextTurn : weaponWait ) );
      color->r = 0.6f;
      color->g = 0.1f;
      color->b = 0.6f;
      return true;
    } else if( creature->getAction() == Constants::ACTION_EAT_DRINK ) {
      snprintf( buff, buffSize, "%s: %d", 
               creature->getActionItem()->getRpgItem()->getDisplayName(),
               ( nextTurn > 0 ? nextTurn : weaponWait ) );
      color->r = 0;
      color->g = 0.6f;
      color->b = 0;
      return true;
    } else if( creature->getAction() == Constants::ACTION_SPECIAL ) {
      snprintf( buff, buffSize, "%s: %d", 
               creature->getActionSkill()->getDisplayName(), 
               ( nextTurn > 0 ? nextTurn : weaponWait ) );
      color->r = 0;
      color->g = 0.3f;
      color->b = 0.6f;
      return true;
    }
  }

  // info for the target
  bool sameTarget = ( creature->getTargetCreature() == target );
  if( !target || !creature->canAttack( target ) ) return false;

  if( sameTarget ) {
    if( creature->getAction() == Constants::ACTION_CAST_SPELL ) {
      snprintf( buff, buffSize, _( "Targeted" ) );
      color->r = 0.6f;
      color->g = 0.1f;
      color->b = 0.6f;
      return true;
    } else if( creature->getAction() == Constants::ACTION_SPECIAL ) {
      snprintf( buff, buffSize, _( "Targeted" ) );
      color->r = 0;
      color->g = 0.3f;
      color->b = 0.6f;
      return true;
    }
  }
  
  Creature *tmp = NULL;
  if( !sameTarget ) {
    tmp = creature->getTargetCreature();
    creature->setTargetCreature( target );
  }
  float dist = creature->getDistanceToTarget();
  Item *item = creature->getBestWeapon(dist);
  if( !sameTarget ) creature->setTargetCreature( tmp );
  
  // out of range
  if( ( !item || item->getRange() <= dist ) && dist > MIN_DISTANCE ) {
    if( sameTarget ) {
      color->r = 0.5f;
      color->g = 0.2f;
      color->b = 0;
      //does the path get us in range?
      if( (!item && creature->getPathManager()->isPathToTargetCreature()) || (item && creature->getPathManager()->isPathTowardTargetCreature(item->getRange())) ) {
        snprintf( buff, buffSize, _( "Out of Range. Move: %d" ), static_cast<int>( creature->getPathManager()->getPathRemainingSize() ) );
      } else {
        snprintf( buff, buffSize, _( "Out of Range" ) );
      }
    } else {
      color->r = 0.3f;
      color->g = 0.3f;
      color->b = 0.3f;
      // could still be in range for spell or skill
      snprintf( buff, buffSize, ( range >= dist ? _( "In Range" ) : _( "Out of Range" ) ) );
    }
    return true;
  }
  
  // How many steps to wait before being able to use the weapon.
  int weaponWait = toint( creature->getWeaponAPCost( item ) );
  //if( session->getPreferences()->isBattleTurnBased() ) {
//    weaponWait /= 2;
//  }
  
	snprintf( buff, buffSize, "%s: %d", 
				 ( item ? item->getRpgItem()->getDisplayName() : _( "Bare Hands" ) ), 
				 ( sameTarget && nextTurn > 0 ? nextTurn : weaponWait ) );
	color->r = 0.6f;
	color->g = 0;
	color->b = 0;
  return true;
}

bool Battle::isInRangeOfTarget() {
	// FIXME: should also work with target item, location
	if( !creature->getTargetCreature() ) return false;
	float dist = creature->getDistanceToTarget();
	Item *item = creature->getBestWeapon( dist );
	return( ( !item && dist <= MIN_DISTANCE ) ||
					( item && item->getRange() >= dist ) );
}
