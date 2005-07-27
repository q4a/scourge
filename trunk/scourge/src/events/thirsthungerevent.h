/***************************************************************************
                          thirsthungerevent.h  -  description
                             -------------------
    begin                : Thu Apr 8 2004
    copyright            : (C) 2004 by Daroth-U
    email                : daroth-u@ifrance.com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef THIRST_HUNGER_EVENT_H
#define THIRST_HUNGER_EVENT_H

#include "../constants.h"
#include "../scourge.h"
#include "event.h"


/**
  *@author Daroth-U
  */
  
class Creature;
class Scourge;
  
class ThirstHungerEvent : public Event  {

private:
  Creature * creature;
  Scourge * scourge;          

public:

  void execute();    
  
  ThirstHungerEvent(Date currentDate, Date timeOut, Creature *c, Scourge *scourge, int nbExecutionsToDo);  
  ThirstHungerEvent();
  virtual ~ThirstHungerEvent();  
  
};

#endif
