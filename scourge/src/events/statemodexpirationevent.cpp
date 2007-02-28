
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
  if(creature->getStateMod(Constants::dead)) return;
  if(stateMod == Constants::poisoned) {
    // apply poison damage
    if(creature->getStateMod(Constants::poisoned)) {
      Creature *tmp = creature->getTargetCreature();
      creature->setTargetCreature(creature);
      char message[80];
      sprintf(message, _( "%s suffers poison damage!" ), creature->getName());
      session->getMap()->addDescription(message, 0.05f, 1.0f, 0.05f);

			char tmp2[255];
			sprintf( tmp2, _( "%s poison." ), 
							 Constants::getMessage( Constants::CAUSE_OF_DEATH ) );
			creature->getTargetCreature()->setPendingCauseOfDeath( tmp2 );

      creature->getBattle()->dealDamage(4.0f*rand()/RAND_MAX, 
                                        //4, 
                                        Constants::EFFECT_GREEN, true);
      creature->setTargetCreature(tmp);
    }
  }
}

void StateModExpirationEvent::executeBeforeDelete() {
  // Don't need this event anymore    
  scheduleDeleteEvent();        
  
  if( creature->getStateMod(Constants::dead) ||
      !( creature->getStateMod( stateMod ) ) ) return;
  
  creature->setStateMod(stateMod, false);
  
  char msg[255];
  sprintf( msg, _( "%1$s is not %2$s any more." ), 
					 creature->getName(), 
					 _( Constants::STATE_DISPLAY_NAMES[stateMod] ) );
  session->getMap()->addDescription(msg, 0.2f, 1, 1);
  creature->startEffect(Constants::EFFECT_GREEN, (Constants::DAMAGE_DURATION * 4));
}

bool StateModExpirationEvent::doesReferenceCreature( Creature *creature ) {
	return( this->creature == creature ? true : false );
}

