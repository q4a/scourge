/***************************************************************************
                          potionexpirationevent.h  -  description
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

#ifndef POTION_EXPIRATION_EVENT_H
#define POTION_EXPIRATION_EVENT_H

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
  
class PotionExpirationEvent : public Event  {

private:
  Creature *creature;
  int potionSkill;
  int amount;
  Session *session;

public:

  void execute();    
  
  PotionExpirationEvent(Date currentDate, Date timeOut, Creature *c, RpgItem *item, Session *session, int nbExecutionsToDo);  
  PotionExpirationEvent(Date currentDate, Date timeOut, Creature *c, int potionSkill, int amount, Session *session, int nbExecutionsToDo);
  PotionExpirationEvent();
  virtual ~PotionExpirationEvent();  
  
};

#endif
