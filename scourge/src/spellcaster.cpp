/***************************************************************************
                          spellcaster.cpp  -  description
                             -------------------
    begin                : Sat May 3 2003
    copyright            : (C) 2003 by Gabor Torok
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

#include "spellcaster.h"

/*
  In the future we can employ something more sophisticated than these if structures...
 */

void SpellCaster::spellFailed(Scourge *scourge, Creature *creature, int power) {
  if(!creature->getActionSpell()) return;
  cerr << "FAILED: " << creature->getActionSpell()->getName() << " power=" << power << endl;
  
  // put code here for spells with something spectacular when they fail...
  // (fouled fireball decimates party, etc.)

  // default is to print patronizing message...
  scourge->getMap()->addDescription(Constants::getMessage(Constants::SPELL_FAILED_MESSAGE), 1, 0.15f, 1);
}

void SpellCaster::spellSucceeded(Scourge *scourge, Creature *creature, int power) {
  if(!creature->getActionSpell()) return;
  cerr << "SUCCEEDED: " << creature->getActionSpell()->getName() << " power=" << power << endl;
  if(!strcasecmp(creature->getActionSpell()->getName(), "Lesser healing touch") ||
	 !strcasecmp(creature->getActionSpell()->getName(), "Greater healing touch") ||
	 !strcasecmp(creature->getActionSpell()->getName(), "Divine healing touch")) {
	increaseHP(scourge, creature, power);
  } else if(!strcasecmp(creature->getActionSpell()->getName(), "Body of stone")) {
	increaseAC(scourge, creature, power);
  } else {
	// default
	cerr << "*** ERROR: Implement spell " << creature->getActionSpell()->getName() << endl;
  }
}



void SpellCaster::increaseHP(Scourge *scourge, Creature *creature, int power) {
  int n = creature->getActionSpell()->getAction();
  n += (int)((((float)n / 100.0f) * (float)power) * rand()/RAND_MAX);

  if(n + creature->getTargetCreature()->getHp() > creature->getTargetCreature()->getMaxHp()) 
	n = creature->getTargetCreature()->getMaxHp() - creature->getTargetCreature()->getHp();
  creature->getTargetCreature()->setHp((int)(creature->getTargetCreature()->getHp() + n));
  char msg[200];
  sprintf(msg, "%s heals %d points.", creature->getTargetCreature()->getName(), n);
  scourge->getMap()->addDescription(msg, 0.2f, 1, 1);
  creature->getTargetCreature()->startEffect(Constants::EFFECT_SWIRL, (Constants::DAMAGE_DURATION * 4));
}

void SpellCaster::increaseAC(Scourge *scourge, Creature *creature, int power) {
  int n = creature->getActionSpell()->getAction();
  n += (int)((((float)n / 100.0f) * (float)power) * rand()/RAND_MAX);

  int timeInMin = 2 * creature->getLevel();

  //  cerr << "increaseAC: points=" << n << " time=" << timeInMin << " min-s." << endl;

  creature->getTargetCreature()->setBonusArmor(creature->getTargetCreature()->getBonusArmor() + n);
  char msg[200];
  sprintf(msg, "%s feels impervious to damage!", creature->getTargetCreature()->getName());
  scourge->getMap()->addDescription(msg, 0.2f, 1, 1);
  creature->getTargetCreature()->startEffect(Constants::EFFECT_SWIRL, (Constants::DAMAGE_DURATION * 4));
  
  // add calendar event to remove armor bonus
  // (format : sec, min, hours, days, months, years)
  Date d(0, timeInMin, 0, 0, 0, 0); 
  Event *e = new PotionExpirationEvent(scourge->getParty()->getCalendar()->getCurrentDate(), 
									   d, creature->getTargetCreature(), 
									   Constants::getPotionSkillByName("AC"), n, 
									   scourge, 1);
  scourge->getParty()->getCalendar()->scheduleEvent((Event*)e);   // It's important to cast!!		

}
