/***************************************************************************
                          potionexpirationevent.cpp  -  description
                             -------------------
    begin                : Thu Apr 8 2004
    copyright            : (C) 2004 by Gabor Torok
    email                : cctorok@yahoo.com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "potionexpirationevent.h"

PotionExpirationEvent::PotionExpirationEvent(Date currentDate, Date timeOut, 
											 Creature *c, RpgItem *item, 
											 Scourge * scourge, int nbExecutionsToDo):
  Event(currentDate, timeOut, nbExecutionsToDo) {
    this->creature = c;
	this->item = item;
    this->scourge = scourge;
}

PotionExpirationEvent::~PotionExpirationEvent(){
}

void PotionExpirationEvent::execute() {

  // Don't need this event anymore    
  scheduleDeleteEvent();        

  if(creature->getStateMod(Constants::dead)) return;

  char msg[255];
  int skill = item->getPotionSkill();
  if(skill < 0) {
	switch(-skill - 2) {
	case Constants::HP:
	  // no-op
	  return;
	case Constants::AC:
	  creature->setBonusArmor(creature->getBonusArmor() - item->getAction());
	  sprintf(msg, "%s feels vulnerable...", creature->getName(), item->getAction());
	  scourge->getMap()->addDescription(msg, 0.2f, 1, 1);
	  creature->startEffect(Constants::EFFECT_SWIRL, (Constants::DAMAGE_DURATION * 2));
	  return;
	default:
	  cerr << "Implement me! (other potion skill boost)" << endl;
	  return;
	}
  } else {
	switch(skill) {
	default:
	  cerr << "Implement me! (other regular skill boost)" << endl;
	  return;
	}
  }
}

