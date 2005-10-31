/***************************************************************************
                          reloadevent.h  -  description
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

#ifndef RELOAD_EVENT_H
#define RELOAD_EVENT_H

#include "../constants.h"
#include "event.h"

/** 
 * @author Gabor Torok
 */

class Session;
  
class ReloadEvent : public Event  {

private:
  Session *session;

public:

  void execute();    
  
  ReloadEvent(Date currentDate, Date timeOut, int nbExecutionsToDo, Session *session);  
  ReloadEvent();
  virtual ~ReloadEvent();  
  
};

#endif

