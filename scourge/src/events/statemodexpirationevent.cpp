
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

  // Don't need this event anymore    
  scheduleDeleteEvent();        

  if(creature->getStateMod(Constants::dead)) return;

  creature->setStateMod(stateMod, false);

  char msg[255];
  sprintf(msg, "%s is not %s any more.", 
          creature->getName(), 
          Constants::STATE_NAMES[stateMod]);
  session->getMap()->addDescription(msg, 0.2f, 1, 1);
  creature->startEffect(Constants::EFFECT_GREEN, (Constants::DAMAGE_DURATION * 4));
}

