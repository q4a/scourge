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


void Calendar::scheduleEvent(Event *e){
    if(scheduledEvents.size() < MAX_SCHEDULED_EVENTS){
          scheduledEvents.push_back(e);
          cout << "scheduleEvent id : " << e->id << endl;
    }
}

void Calendar::setPause(bool mustPause){
    if(!mustPause){        
        // game is unfreezed, so starting time has changed
        lastTick = SDL_GetTicks();
    }
    timeFrozen = mustPause;             
}

bool Calendar::update(){
    Event * e;            
    
    // no update if time is frozen
    if(timeFrozen) return false;
      
    GLint t = SDL_GetTicks();
        
    if(t - lastTick >= 1000){               
        lastTick = t;
        currentDate.addSeconds(timeMultiplicator);                       
    }        
    
    // look for scheduled events
    cout << "nbScheduled events: " <<  scheduledEvents.size() << endl;
    for(int i = 0 ; i < scheduledEvents.size(); i++){
        cout << "test event: " <<  i  << endl;
        scheduledEvents[i]->getEventDate().print();
        cout << " < ";
         currentDate.print();
         cout << " ?" << endl;        
        if( (scheduledEvents[i]->getEventDate()).isInferiorTo(currentDate) ){
            cout<< "Oui : event " << i << " execution."<< endl;
            scheduledEvents[i]->execute();                                   
            
            // remove this event as it has been executed
            e = scheduledEvents[i];                        
            scheduledEvents[i] = scheduledEvents[scheduledEvents.size()]; 
            scheduledEvents[scheduledEvents.size()] = e;
            scheduledEvents.pop_back(); 
            e -> increaseNbExecutions();
            
            // and re-adds it if needed
            if(    e->getNbExecutionsToDo() == Event::INFINITE_EXECUTIONS
                || e->getNbExecutions() < e->getNbExecutionsToDo()){
                Date d;
                d = e->getEventDate();
                d.addDate(e->getTimeOut());
                e -> setEventDate(d);
                scheduleEvent(e);
            }
            else{
                // Don't need this event anymore
                cout << "before Delete!!!" << endl;
                delete e;                 
                cout << "after Delete!!!" << endl;
            }
            
        }     
    }
    return true;
}

void Calendar::setTimeMultiplicator(int t){
    if(t > 0 && t < 60){
        timeMultiplicator = t;
    }    
}

Calendar::~Calendar(){
    Calendar::instance = NULL;
    //if(currentDate) delete currentDate;
}


