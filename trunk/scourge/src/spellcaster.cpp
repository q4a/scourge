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

float SpellCaster::getPower(Creature *creature, Spell *spell) {
  // calculate spell's power
  // power=[0-25]
  float power = (float)creature->getSkill(spell->getSchool()->getSkill()) / 4.0f;
  // power=[0-45]
  power += (float)creature->getSkill(Constants::IQ) / 5.0f;
  // power=[0-450]
  power *= creature->getLevel();
  // power=[0-500]
  power += ((float)creature->getSkill(Constants::LUCK) / 2.0f);
  return power;
}

void SpellCaster::spellFailed(Scourge *scourge, Creature *creature, Spell *spell, bool projectileHit) {
  if(!spell) return;
  float power = getPower(creature, spell);

  cerr << "FAILED: " << spell->getName() << " power=" << power << endl;
  
  // put code here for spells with something spectacular when they fail...
  // (fouled fireball decimates party, etc.)

  // default is to print patronizing message...
  scourge->getMap()->addDescription(Constants::getMessage(Constants::SPELL_FAILED_MESSAGE), 1, 0.15f, 1);
}

void SpellCaster::spellSucceeded(Scourge *scourge, Creature *creature, Spell *spell, bool projectileHit) {
  if(!spell) return;
  float power = getPower(creature, spell);

  cerr << "SUCCEEDED: " << spell->getName() << " power=" << power << endl;
  if(!strcasecmp(spell->getName(), "Lesser healing touch") ||
	 !strcasecmp(spell->getName(), "Greater healing touch") ||
	 !strcasecmp(spell->getName(), "Divine healing touch")) {
	increaseHP(scourge, creature, spell, power);
  } else if(!strcasecmp(spell->getName(), "Body of stone")) {
	increaseAC(scourge, creature, spell, power);
  } else if(!strcasecmp(spell->getName(), "Stinging light")) {
	if(projectileHit) {
	  causeDamage(scourge, creature, spell, power);
	} else {
	  launchProjectile(scourge, creature, spell, power);
	}
  } else {
	// default
	cerr << "*** ERROR: Implement spell " << spell->getName() << endl;
  }
}





void SpellCaster::increaseHP(Scourge *scourge, Creature *creature, Spell *spell, float power) {
  int n = spell->getAction();
  n += (int)((((float)n / 100.0f) * power) * rand()/RAND_MAX);

  if(n + creature->getTargetCreature()->getHp() > creature->getTargetCreature()->getMaxHp()) 
	n = creature->getTargetCreature()->getMaxHp() - creature->getTargetCreature()->getHp();
  creature->getTargetCreature()->setHp((int)(creature->getTargetCreature()->getHp() + n));
  char msg[200];
  sprintf(msg, "%s heals %d points.", creature->getTargetCreature()->getName(), n);
  scourge->getMap()->addDescription(msg, 0.2f, 1, 1);
  creature->getTargetCreature()->startEffect(Constants::EFFECT_SWIRL, (Constants::DAMAGE_DURATION * 4));
}

void SpellCaster::increaseAC(Scourge *scourge, Creature *creature, Spell *spell, float power) {
  int n = spell->getAction();
  n += (int)((((float)n / 100.0f) * power) * rand()/RAND_MAX);

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

void SpellCaster::launchProjectile(Scourge *scourge, Creature *creature, Spell *spell, float power) {
  // FIXME: implement multiple projectiles... currently n is a number to how many can be in the air at once
  // this works for continuous fire (arrows) but not for magic-missile type action...
  // may have to implement parabolic (in 2d) flight for >1 missiles
  int n = creature->getLevel();
  if( n < 1 ) n = 1;
  cerr << "launching " << n << " spell projectiles!" << endl;

  // FIXME: shape should be configurable per spell
  if(!Projectile::addProjectile(creature, creature->getTargetCreature(), spell, 
								scourge->getShapePalette()->findShapeByName("ARROW"),
								n)) {
	// max number of projectiles in the air
	// FIXME: do something... 
	// (like print message: can't launch projectile due to use of fixed-sized array in code?)
  }
}

void SpellCaster::causeDamage(Scourge *scourge, Creature *creature, Spell *spell, float power) {
  cerr << "SpellCaster::causeDamage: Implement me!" << endl;
  // FIXME: effect should be configurable per spell
  creature->getTargetCreature()->startEffect(Constants::EFFECT_EXPLOSION, (Constants::DAMAGE_DURATION * 4));
}
