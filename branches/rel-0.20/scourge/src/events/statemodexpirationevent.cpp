
/***************************************************************************
                          statemodexpirationevent.cpp  -  description
                             -------------------
    begin                : Thu Apr 8 2004
    copyright            : (C) 2004 by Gabor Torok
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

#include "statemodexpirationevent.h"
#include "../render/renderlib.h"
#include "../creature.h"
#include "../session.h"

StateModExpirationEvent::StateModExpirationEvent(Date currentDate, Date timeOut, Creature *c, 
                                                 int stateMod, Session *session, int nbExecutionsToDo) :
    Event(currentDate, timeOut, nbExecutionsToDo) 
{
  this->creature = c;
  this->stateMod = stateMod;
  this->session = session;
  }

StateModExpirationEvent::~StateModExpirationEvent(){
}

void StateModExpirationEvent::execute() {
  if(creature->getStateMod(StateMod::dead)) return;
  if(stateMod == StateMod::poisoned) {
    // apply poison damage
    if(creature->getStateMod( StateMod::poisoned )) {
      Creature *tmp = creature->getTargetCreature();
      creature->setTargetCreature(creature);
      char message[80];
      snprintf(message, 80, _( "%s suffers poison damage!" ), creature->getName());
        if ( creature->getCharacter() ) {
          session->getGameAdapter()->writeLogMessage(message, Constants::MSGTYPE_PLAYERDAMAGE);
        } else {
          session->getGameAdapter()->writeLogMessage(message, Constants::MSGTYPE_NPCDAMAGE);
        }

			char tmp2[255];
			snprintf( tmp2, 255, _( "%s poison." ), 
							 Constants::getMessage( Constants::CAUSE_OF_DEATH ) );
			creature->getTargetCreature()->setPendingCauseOfDeath( tmp2 );

      creature->getBattle()->dealDamage( Util::roll( 0.0f, 4.0f ), 
                                        //4, 
                                        Constants::EFFECT_GREEN, true);
      creature->setTargetCreature(tmp);
    }
  }
}

void StateModExpirationEvent::executeBeforeDelete() {
  // Don't need this event anymore    
  scheduleDeleteEvent();        
  
  if( creature->getStateMod(StateMod::dead) ||
      !( creature->getStateMod( stateMod ) ) ) return;
  
  creature->setStateMod(stateMod, false);
  
  char msg[255];
	snprintf( msg, 255, StateMod::stateMods[ stateMod ]->getUnsetState(), creature->getName() );
  session->getGameAdapter()->writeLogMessage(msg, Constants::MSGTYPE_STATS);
  creature->startEffect(Constants::EFFECT_GREEN, (Constants::DAMAGE_DURATION * 4));
}

bool StateModExpirationEvent::doesReferenceCreature( Creature *creature ) {
	return( this->creature == creature );
}

