/***************************************************************************
                          calendar.cpp  -  description
                             -------------------
    begin                : Wed April 7 2004
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

#ifndef CALENDAR_H
#define CALENDAR_H

#include "constants.h"
#include <iostream>
#include <vector>    // STL for Vector
#include "date.h"
#include "events/event.h"

#define MAX_SCHEDULED_EVENTS 500

/**
  *@author Daroth-U
  */


class Event;
class Date;

class Calendar {
 private:  
  
  static Calendar *instance;   
  
  Date currentDate;
  int timeMultiplicator;
  GLint lastTick; 
  bool timeFrozen; 
  
  std::vector<Event*> scheduledEvents;  
  
 public:
 
  static Calendar * getInstance();
  bool update(int gameSpeed);
  void setPause(bool mustPause);
  void setTimeMultiplicator(int t);
  void scheduleEvent(Event *e);
  void reset(bool resetTime=true);
  void cancelEvent(Event *e);

  // return date by value to avoid modification by other classes
  inline Date getCurrentDate() { return currentDate; }
  inline void addADay() {
    currentDate.setDate( currentDate.getSec(), 
                         currentDate.getMin(), 
                         currentDate.getHour(), 
                         currentDate.getDay() + 1, 
                         currentDate.getMonth(), 
                         currentDate.getYear() );
  }
    
  Calendar();
  ~Calendar();
  

 protected:
  


};

#endif
