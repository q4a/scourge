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

/**
  *@author Daroth-U
  */

typedef struct _date{
    unsigned char sec;      // 0 to 59
    unsigned char min;      // 0 to 59
    unsigned char hour;     // 0 to 23
    unsigned char dayOfWeek;        // 1 to 7
    unsigned char numDayMonth;      // 1 to 31 or 30 or 29
    unsigned char month;    // 1 to 12
    int year;               // -32000 to +32000 
}date;



class Calendar {
 private:  
  
  static Calendar *instance;   
  
  static unsigned char dayInMonth[13]; 
  static const char * monthName[13];
  static const char * dayName[8];
  
  date currentDate;
  char currentDateString[100];
  int timeMultiplicator;
  GLint lastTick; 
  bool timeFrozen; 
  
  void buildDateString();    
  
 public:
 
  static Calendar * getInstance();
  void update();
  void setPause(bool mustPause);
  void setTimeMultiplicator(int t);
  
  inline const char * getDayOfWeekString()    { return dayName[currentDate.dayOfWeek]; }
  inline const char * getMonthString()        { return monthName[currentDate.month]; }
  inline unsigned char getNumDayMonth() { return currentDate.numDayMonth; }
  inline int getYear()                  { return currentDate.year; }
  inline unsigned char getHour()        { return currentDate.hour; }
  inline unsigned char getMin()         { return currentDate.min; }
  inline unsigned char getSec()         { return currentDate.sec; }
  inline char * getDateString()         { return currentDateString; }
  
  
  Calendar();
  ~Calendar();
  

 protected:
  


};

#endif
