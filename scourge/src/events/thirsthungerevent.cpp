/***************************************************************************
                          thirsthungerevent.cpp  -  description
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

#include "thirsthungerevent.h"
#include "../render/renderlib.h"

ThirstHungerEvent::ThirstHungerEvent(Date currentDate, Date timeOut, Creature *c, Scourge * scourge, int nbExecutionsToDo):
Event(currentDate, timeOut, nbExecutionsToDo)
{
    this->creature = c;
    this->scourge = scourge;
}

void ThirstHungerEvent::execute(){
    int thirst, hunger;
    char buff[255];  
    
    if(creature -> getStateMod(Constants::dead)){
        // Don't need this event anymore    
        scheduleDeleteEvent();        
        return;
    }
    
    thirst = creature->getThirst();
    hunger = creature->getHunger();
    
    thirst--; hunger--; 
    creature->setThirst(thirst);
    creature->setHunger(hunger); 
                 
    if(thirst == 5){
        sprintf(buff, "%s is thirsty.", creature->getName());     
        scourge->getMap()->addDescription(buff, 1.0f, 0.5f, 0.5f);            
    }
    else if(thirst == 3){
        sprintf(buff, "%s is really thirsty.", creature->getName());     
        scourge->getMap()->addDescription(buff, 1.0f, 0.5f, 0.5f);   
    }
    else if(thirst == 2){
    
        sprintf(buff, "%s is beginning to dehydrate!", creature->getName());     
        scourge->getMap()->addDescription(buff, 1.0f, 0.5f, 0.5f); 
        // FIXME add state dehydrated or weak?
        // creature->setModState(dehydrated);    
    }
    else if(thirst == 1){        
        sprintf(buff, "%s is totally dehydrated!", creature->getName());     
        scourge->getMap()->addDescription(buff, 1.0f, 0.5f, 0.5f); 
        // FIXME add state nearlyDead -> can't walk fast ...?
        // creature->setModState(nearly_dead);
    }
    else if( thirst == 0 ){
        sprintf(buff, "%s dies from lack of water.", creature->getName());     
        scourge->getMap()->addDescription(buff, 1.0f, 0.5f, 0.5f);
        scourge->getSession()->creatureDeath(creature);
        scheduleDeleteEvent();
        return; // no need to go further        
    }        
    
    if(hunger == 5){
        sprintf(buff, "%s is hungry.", creature->getName());     
        scourge->getMap()->addDescription(buff, 1.0f, 0.5f, 0.5f);   
    }    
    else if(hunger == 3){
        sprintf(buff, "%s is really hungry.", creature->getName());     
        scourge->getMap()->addDescription(buff, 1.0f, 0.5f, 0.5f);   
    }
    else if(hunger == 2){
    
        sprintf(buff, "%s is starving!", creature->getName());     
        scourge->getMap()->addDescription(buff, 1.0f, 0.5f, 0.5f); 
        // FIXME add state starving ? or weak?
        // creature->setModState(starving);    
    }
    else if(hunger == 1){        
        sprintf(buff, "%s feels really weak!", creature->getName());     
        scourge->getMap()->addDescription(buff, 1.0f, 0.5f, 0.5f); 
        // FIXME add state nearlyDead -> can't walk fast ...?
        // creature->setModState(nearly_dead);
    } 
    else if( hunger == 0 ){
        sprintf(buff, "%s dies from starvation.", creature->getName());     
        scourge->getMap()->addDescription(buff, 1.0f, 0.5f, 0.5f);
        scourge->getSession()->creatureDeath(creature); 
        scheduleDeleteEvent();       
    }                  
}

ThirstHungerEvent::~ThirstHungerEvent(){
}
