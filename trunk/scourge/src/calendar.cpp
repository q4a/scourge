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

#include "calendar.h"

Calendar *Calendar::instance = NULL;

unsigned char Calendar::dayInMonth[13] = {
    0, 31, 29, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31
};

const char * Calendar::monthName[13] = {
    NULL, 
    "January", "February", 
    "March", "April", 
    "May", "June", 
    "July", "August", 
    "September", "October", 
    "November", "December"
};

const char * Calendar::dayName[8] = {
    NULL, "Monday", "Tuesday", "Wednesday", "Thursday", 
    "Friday", "Saturday", "Sunday"
};


Calendar *Calendar::getInstance(){
    if(!Calendar::instance){
        Calendar::instance = new Calendar();
    }
    return Calendar::instance;
}

Calendar::Calendar(){
    currentDate.sec = 0;
    currentDate.min = 0;
    currentDate.hour = 0;
    currentDate.dayOfWeek = 1;
    currentDate.numDayMonth = 1;
    currentDate.month = 1;
    currentDate.year = 1128;
    timeMultiplicator = 6;
    lastTick = 0;
    timeFrozen = false;
}

void Calendar::setPause(bool mustPause){
    if(!mustPause){        
        // game is unfreezed, so starting time has changed
        lastTick = SDL_GetTicks();
    }
    timeFrozen = mustPause;             
}

void Calendar::update(){
    bool changed = false;
    
    // no update if time is frozen
    if(timeFrozen) return;
      
    GLint t = SDL_GetTicks();
        
    if(t - lastTick >= 1000){       
        // One second (real time) has passed
        changed = true;
        lastTick = t;
        currentDate.sec += timeMultiplicator;        
        
        if(currentDate.sec > 59){            
            currentDate.sec -= 60;    
            currentDate.min ++;
                        
            if(currentDate.min > 59){                
                currentDate.min = 0;    
                currentDate.hour ++;              
                
                if(currentDate.hour > 23){                
                    currentDate.hour = 0;    
                    currentDate.dayOfWeek ++;                              
                    
                    if(currentDate.dayOfWeek > 7){                
                        currentDate.dayOfWeek = 1;    
                        currentDate.numDayMonth ++;
                        
                        if(currentDate.numDayMonth > dayInMonth[currentDate.month]){
                            currentDate.numDayMonth = 1;
                            currentDate.month ++;
                                                        
                            if(currentDate.month > 12){
                                currentDate.month = 1;
                                currentDate.year ++;
                            }                            
                        }
                    }              
                }
            }
        }  
    }
    
    // rebuild date string
    if (changed){
        buildDateString();    
    }
    
}

void Calendar::buildDateString(){
    char buff[10];
                
    buff[0] = monthName[currentDate.month][0];
    buff[1] = monthName[currentDate.month][1];
    buff[2] = monthName[currentDate.month][2];
    buff[3] = ' ';
    buff[4] = '\0';    
    
    sprintf(currentDateString, "%s %.2d %.2dh%.2d :%.2d", 
            buff, 
            (int)currentDate.numDayMonth,            
            (int)currentDate.hour,
            (int)currentDate.min,
            (int)currentDate.sec
    );
        
}

void Calendar::setTimeMultiplicator(int t){
    if(t > 0 && t < 60){
        timeMultiplicator = t;
    }    
}

Calendar::~Calendar(){
    Calendar::instance = NULL;
}


