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
  if(!strcasecmp(creature->getActionSpell()->getName(), "Healing touch")) {
	castHealingTouch(scourge, creature, power);
  } else {
	// default
	cerr << "*** ERROR: Implement spell " << creature->getActionSpell()->getName() << endl;
  }
}



void SpellCaster::castHealingTouch(Scourge *scourge, Creature *creature, int power) {
  if(power + creature->getTargetCreature()->getHp() > creature->getTargetCreature()->getMaxHp()) 
	power = creature->getTargetCreature()->getMaxHp() - creature->getTargetCreature()->getHp();
  creature->getTargetCreature()->setHp((int)(creature->getTargetCreature()->getHp() + power));
  char msg[200];
  sprintf(msg, "%s heals %d points.", creature->getTargetCreature()->getName(), power);
  scourge->getMap()->addDescription(msg, 0.2f, 1, 1);
  creature->getTargetCreature()->startEffect(Constants::EFFECT_SWIRL, (Constants::DAMAGE_DURATION * 4));
}
