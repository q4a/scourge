
/***************************************************************************
                          statemodexpirationevent.h  -  description
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

#ifndef STATE_MOD_EXPIRATION_EVENT_H
#define STATE_MOD_EXPIRATION_EVENT_H

#include "../constants.h"
#include "../rpg/rpgitem.h"
#include "../creature.h"
#include "event.h"


/**
  *@author Gabor Torok
  */
  
class Creature;
class Session;
class RpgItem;
  
class StateModExpirationEvent : public Event  {

private:
  Creature *creature;
  int stateMod;
  Session *session;

public:

  void execute();    
  
  StateModExpirationEvent(Date currentDate, Date timeOut, Creature *c, int stateMod, Session *session, int nbExecutionsToDo);
  StateModExpirationEvent();
  virtual ~StateModExpirationEvent();  
  
};

#endif
