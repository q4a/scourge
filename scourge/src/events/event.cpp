/***************************************************************************
                          event.cpp  -  description
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

#include "event.h"

int Event::id=0;
Event::Event(){
}

// This event will occur nbExecutionsTD time every tmOut from currentDate.
Event::Event(Date currentDate, Date tmOut, long nbExecutionsToDo){    
    this->nbExecutions = 0;
    this->nbExecutionsToDo = nbExecutionsToDo;
    this->timeOut = tmOut;   
    this->eventDate = tmOut;    
    eventDate.addDate(currentDate);  
    id++;      
}

// This event will occur only one time at the given date
Event::Event(Date eventDate){
    this->nbExecutions = 0;
    this->nbExecutionsToDo = 1;
    this->timeOut.setDate(0,0,0,0,0,0);   
    this->eventDate = eventDate; 
    id++;             
}

Event::~Event(){
}
