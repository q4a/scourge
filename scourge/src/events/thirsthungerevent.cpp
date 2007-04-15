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
#include "../creature.h"

ThirstHungerEvent::ThirstHungerEvent(Date currentDate, Date timeOut, Creature *c, Scourge * scourge, int nbExecutionsToDo):
Event(currentDate, timeOut, nbExecutionsToDo)
{
    this->creature = c;
    this->scourge = scourge;
}

void ThirstHungerEvent::execute(){
    int thirst, hunger;
    char buff[255];  
    
    if(creature -> getStateMod(StateMod::dead)){
      // FIXME: needs testing. I commented it out in case the player is resurrected.
      // Don't need this event anymore          
      //scheduleDeleteEvent();        
        return;
    }
    
    thirst = creature->getThirst();
    hunger = creature->getHunger();
    
    thirst--; 
		if( thirst < 0 ) thirst = 0;
		hunger--; 
		if( hunger < 0 ) hunger = 0;
    creature->setThirst(thirst);
    creature->setHunger(hunger); 
                 
		int n;
    if(thirst == 5){
        sprintf(buff, _( "%s is thirsty." ), creature->getName());     
        scourge->addDescription(buff, 1.0f, 0.5f, 0.5f);            
    }
    else if(thirst == 3){
        sprintf(buff, _( "%s is really thirsty." ), creature->getName());     
        scourge->addDescription(buff, 1.0f, 0.5f, 0.5f);   
    }
    else if(thirst == 2){
    
        sprintf(buff, _( "%s is beginning to dehydrate!" ), creature->getName());     
        scourge->addDescription(buff, 1.0f, 0.5f, 0.5f); 
        // FIXME add state dehydrated or weak?
        // creature->setModState(dehydrated);    
    }
    else if(thirst == 1){        
        sprintf(buff, _( "%s is totally dehydrated!" ), creature->getName());     
        scourge->addDescription(buff, 1.0f, 0.5f, 0.5f); 
        // FIXME add state nearlyDead -> can't walk fast ...?
        // creature->setModState(nearly_dead);
    }
    else if( thirst == 0 ){
			n = (int)( 8.0f * rand() / RAND_MAX );
			sprintf(buff, _( "%1$s looses %2$d hit points from dehydration!" ), creature->getName(), n );     
			scourge->addDescription(buff, 1.0f, 0.5f, 0.5f); 
			creature->setHp( creature->getHp() - n );
			if( creature->getHp() <= 0 ) {
				creature->setCauseOfDeath( _( "Died of thirst" ) );
        scourge->getSession()->creatureDeath( creature );
        return;
			}
    }        
    
    if(hunger == 5){
        sprintf(buff, _( "%s is hungry." ), creature->getName());     
        scourge->addDescription(buff, 1.0f, 0.5f, 0.5f);   
    }    
    else if(hunger == 3){
        sprintf(buff, _( "%s is really hungry." ), creature->getName());     
        scourge->addDescription(buff, 1.0f, 0.5f, 0.5f);   
    }
    else if(hunger == 2){
    
        sprintf(buff, _( "%s is starving!" ), creature->getName());     
        scourge->addDescription(buff, 1.0f, 0.5f, 0.5f); 
        // FIXME add state starving ? or weak?
        // creature->setModState(starving);    
    }
    else if(hunger == 1){        
        sprintf(buff, _( "%s feels really weak!" ), creature->getName());     
        scourge->addDescription(buff, 1.0f, 0.5f, 0.5f); 
        // FIXME add state nearlyDead -> can't walk fast ...?
        // creature->setModState(nearly_dead);
    } 
    else if( hunger == 0 ){
			n = (int)( 8.0f * rand() / RAND_MAX );
			sprintf(buff, _( "%1$s looses %2$d hit points from hunger!" ), creature->getName(), n );     
			scourge->addDescription(buff, 1.0f, 0.5f, 0.5f); 
			creature->setHp( creature->getHp() - n );
			if( creature->getHp() <= 0 ) {
				creature->setCauseOfDeath( _( "Expired due to famine" ) );
        scourge->getSession()->creatureDeath( creature );
        return;
			}
    }                  
}

ThirstHungerEvent::~ThirstHungerEvent(){
}

bool ThirstHungerEvent::doesReferenceCreature( Creature *creature ) {
	return( this->creature == creature ? true : false );
}

