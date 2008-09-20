/***************************************************************************
       potionexpirationevent.cpp  -  Potion expiration event
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
#include "../rpg/rpglib.h"
#include "../render/renderlib.h"
#include "../item.h"
#include "../creature.h"
#include "../session.h"

using namespace std;

PotionExpirationEvent::PotionExpirationEvent( Date currentDate, 
																							Date timeOut, 
																							Creature *c, 
																							Item *item, 
																							Session *session, 
																							int nbExecutionsToDo ) : Event( currentDate, timeOut, nbExecutionsToDo ) {
  this->creature = c;
	this->potionSkill = item->getRpgItem()->getPotionSkill();
	this->amount = item->getRpgItem()->getPotionPower();
	this->session = session;
}		

PotionExpirationEvent::PotionExpirationEvent( Date currentDate, 
																							Date timeOut, 
																							Creature *c, 
																							int potionSkill, 
																							int amount, 
																							Session *session, 
																							int nbExecutionsToDo ) : Event( currentDate, timeOut, nbExecutionsToDo ) {
  this->creature = c;
	this->potionSkill = potionSkill;
	this->amount = amount;
	this->session = session;
}

PotionExpirationEvent::~PotionExpirationEvent(){
}

void PotionExpirationEvent::execute() {

  // Don't need this event anymore    
  scheduleDeleteEvent();        

  if(creature->getStateMod(StateMod::dead)) return;
  enum { MSG_SIZE = 255 };
  char msg[ MSG_SIZE ];
  if(potionSkill < 0) {
	switch(-potionSkill - 2) {
	case Constants::HP:
	case Constants::MP:
	  // no-op
	  return;
	case Constants::AC:
	  creature->setBonusArmor(creature->getBonusArmor() - amount);
	  snprintf(msg, MSG_SIZE, _( "%s feels vulnerable..." ), creature->getName());
	  session->getGameAdapter()->writeLogMessage(msg, Constants::MSGTYPE_STATS);
	  creature->startEffect(Constants::EFFECT_SWIRL, (Constants::DAMAGE_DURATION * 4));
	  return;
	default:
	  cerr << "Implement me! (other potion skill boost)" << endl;
	  return;
	}
  } else {
	creature->setSkillBonus(potionSkill, 
							creature->getSkillBonus(potionSkill) - 
							amount);
	//	recalcAggregateValues();
	snprintf(msg, MSG_SIZE,  _( "%s feels a loss of contentment." ), creature->getName());
	session->getGameAdapter()->writeLogMessage(msg, Constants::MSGTYPE_STATS);
	creature->startEffect(Constants::EFFECT_SWIRL, (Constants::DAMAGE_DURATION * 4));
  }
}

bool PotionExpirationEvent::doesReferenceCreature( Creature *creature ) {
	return( this->creature == creature );
}

