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


Calendar *Calendar::getInstance(){
    if(!Calendar::instance){
        Calendar::instance = new Calendar();
    }
    return Calendar::instance;
}

Calendar::Calendar(){
    //currentDate = new Date();    
    timeMultiplicator = 6;
    lastTick = 0;
    timeFrozen = false;    
}

void Calendar::reset(){    
    lastTick = 0;
    scheduledEvents.clear();
    currentDate.reset();
    timeFrozen = false; 
}

void Calendar::scheduleEvent(Event *e){
    if(scheduledEvents.size() < MAX_SCHEDULED_EVENTS){
          scheduledEvents.push_back(e);          
    }
}

void Calendar::setPause(bool mustPause){
    if(!mustPause){        
        // game is unfreezed, so starting time has changed
        lastTick = SDL_GetTicks();
    }
    timeFrozen = mustPause;             
}

bool Calendar::update(int gameSpeed){
    Event * e;            
    
    // no time update if time is frozen
    if(timeFrozen) return false;
      
    GLint t = SDL_GetTicks();
        
    if(t - lastTick >= 1000){               
        lastTick = t;
        switch(gameSpeed){
            case 0 : setTimeMultiplicator(60); break;
            case 1 : setTimeMultiplicator(15); break;
            case 2 : setTimeMultiplicator(6); break;
            case 3 : setTimeMultiplicator(2); break;
            case 4 : setTimeMultiplicator(1); break;
        }
        currentDate.addSeconds(timeMultiplicator);                       
    }        
    
    // look for scheduled events
    if(CALENDAR_DEBUG) cout << "nbScheduled events: " <<  scheduledEvents.size() << endl;
    for(int i = 0 ; i < (int)scheduledEvents.size(); i++){ 
        if(CALENDAR_DEBUG){      
            currentDate.print();        
            cout << " >= ";
            scheduledEvents[i]->getEventDate().print(); 
            cout << " ? ";        
        }  
        // eventDate >= currentDate ?
        if( !(currentDate.isInferiorTo(scheduledEvents[i]->getEventDate())) ){
            if(CALENDAR_DEBUG) cout<< " Yes " << endl;            
            scheduledEvents[i]->execute();                                                           
            
            // remove this event as it has been executed
            e = scheduledEvents[i];                                    
            scheduledEvents[i] = scheduledEvents[scheduledEvents.size()-1]; 
            scheduledEvents[scheduledEvents.size()-1] = e;
            scheduledEvents.pop_back(); 
            e -> increaseNbExecutions();
            if(CALENDAR_DEBUG) cout << "NbExecutions : " << e->getNbExecutions() << "/" << e->getNbExecutionsToDo() << endl;
            
            // and re-adds it if needed
            if(CALENDAR_DEBUG) cout << " readd ? ";
            if(    e->getNbExecutionsToDo() == Event::INFINITE_EXECUTIONS
                || e->getNbExecutions() < e->getNbExecutionsToDo()){
                if(CALENDAR_DEBUG) cout << " Yes" << endl;
                Date d;
                d = e->getEventDate();
                d.addDate(e->getTimeOut());                
                e -> setEventDate(d);
                scheduleEvent(e);
            }
            else{
                // Don't need this event anymore
                if(CALENDAR_DEBUG) cout << " No, deleting this event." << endl;
                delete e;                                 
                if(CALENDAR_DEBUG) cout << " Ok, event deleted." << endl;
            }            
        }    
        else{
            if(CALENDAR_DEBUG) cout << " No" << endl; 
        } 
    }
    return true;
}

void Calendar::setTimeMultiplicator(int t){
    if(t > 0 && t <= 60){
        timeMultiplicator = t;
    }    
}

Calendar::~Calendar(){
    Calendar::instance = NULL;
    //if(currentDate) delete currentDate;
}


