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

SpellCaster::SpellCaster(Battle *battle, Spell *spell, bool projectileHit) {
  this->battle = battle;
  this->spell = spell;
  this->projectileHit = projectileHit;

  Creature *creature = battle->getCreature();
  // calculate spell's power
  // power=[0-25]
  power = (float)creature->getSkill(spell->getSchool()->getSkill()) / 4.0f;
  // power=[0-45]
  power += (float)creature->getSkill(Constants::IQ) / 5.0f;
  // power=[0-450]
  power *= creature->getLevel();
  // power=[0-500]
  power += ((float)creature->getSkill(Constants::LUCK) / 2.0f);
}

SpellCaster::~SpellCaster() {
}

void SpellCaster::spellFailed() {
  if(!spell) return;

//  cerr << "FAILED: " << spell->getName() << " power=" << power << endl;

  // put code here for spells with something spectacular when they fail...
  // (fouled fireball decimates party, etc.)

  // default is to print patronizing message...
  battle->getSession()->getMap()->addDescription(Constants::getMessage(Constants::SPELL_FAILED_MESSAGE), 1, 0.15f, 1);
}

void SpellCaster::spellSucceeded() {
  if(!spell) return;

//  cerr << "SUCCEEDED: " << spell->getName() << " power=" << power << endl;
  if(!strcasecmp(spell->getName(), "Lesser healing touch") ||
     !strcasecmp(spell->getName(), "Greater healing touch") ||
     !strcasecmp(spell->getName(), "Divine healing touch")) {
    increaseHP();
  } else if(!strcasecmp(spell->getName(), "Body of stone")) {
    increaseAC();
  } else if(!strcasecmp(spell->getName(), "Burning stare") ||
            !strcasecmp(spell->getName(), "Silent knives")) {
    if(projectileHit) {
      causeDamage();
    } else {
      launchProjectile(1);
    }
  } else if(!strcasecmp(spell->getName(), "Stinging light")) {
    if(projectileHit) {
      causeDamage();
    } else {
      launchProjectile(0);
    }
  } else if(!strcasecmp(spell->getName(), "Flame of Azun")) {
    if(projectileHit) {
      setStateMod(Constants::blinded);
    } else {
      launchProjectile(1, false);
    }
  } else {
    // default
    cerr << "*** ERROR: Implement spell " << spell->getName() << endl;
  }
}





void SpellCaster::increaseHP() {
  Creature *creature = battle->getCreature();

  int n = spell->getAction();
  n += (int)((((float)n / 100.0f) * power) * rand()/RAND_MAX);

  if(n + creature->getTargetCreature()->getHp() > creature->getTargetCreature()->getMaxHp())
    n = creature->getTargetCreature()->getMaxHp() - creature->getTargetCreature()->getHp();
  creature->getTargetCreature()->setHp((int)(creature->getTargetCreature()->getHp() + n));
  char msg[200];
  sprintf(msg, "%s heals %d points.", creature->getTargetCreature()->getName(), n);
  battle->getSession()->getMap()->addDescription(msg, 0.2f, 1, 1);
  creature->getTargetCreature()->startEffect(Constants::EFFECT_SWIRL, (Constants::DAMAGE_DURATION * 4));
}

void SpellCaster::increaseAC() {
  Creature *creature = battle->getCreature();
  int n = spell->getAction();
  n += (int)((((float)n / 100.0f) * power) * rand()/RAND_MAX);

  int timeInMin = 2 * creature->getLevel();

  //  cerr << "increaseAC: points=" << n << " time=" << timeInMin << " min-s." << endl;

  creature->getTargetCreature()->setBonusArmor(creature->getTargetCreature()->getBonusArmor() + n);
  char msg[200];
  sprintf(msg, "%s feels impervious to damage!", creature->getTargetCreature()->getName());
  battle->getSession()->getMap()->addDescription(msg, 0.2f, 1, 1);
  creature->getTargetCreature()->startEffect(Constants::EFFECT_SWIRL, (Constants::DAMAGE_DURATION * 4));

  // add calendar event to remove armor bonus
  // (format : sec, min, hours, days, months, years)
  Date d(0, timeInMin, 0, 0, 0, 0); 
  Event *e = new PotionExpirationEvent(battle->getSession()->getParty()->getCalendar()->getCurrentDate(), 
                                       d, creature->getTargetCreature(), 
                                       Constants::getPotionSkillByName("AC"), n, 
                                       battle->getSession(), 1);
  battle->getSession()->getParty()->getCalendar()->scheduleEvent((Event*)e);   // It's important to cast!!		

}

void SpellCaster::launchProjectile(int count, bool stopOnImpact) {
  Creature *creature = battle->getCreature();

  // maxcount for spells means number of projectiles
  // (for missiles it means how many can be in the air at once.)
  int n = count;
  if(n == 0) {
    n = creature->getLevel();
    if( n < 1 ) n = 1;
//    cerr << "launching " << n << " spell projectiles!" << endl;
  }

  // FIXME: projectile shape should be configurable per spell
  Projectile *p;
  if(creature->getTargetCreature()) {
    p = Projectile::addProjectile(creature, creature->getTargetCreature(), spell, 
                                  battle->getSession()->getShapePalette()->findShapeByName("SPELL_FIREBALL"),
                                  n, stopOnImpact);
  } else {
    int x, y, z;
    creature->getTargetLocation(&x, &y, &z);
    p = Projectile::addProjectile(creature, x, y, 1, 1, spell, 
                                  battle->getSession()->getShapePalette()->findShapeByName("SPELL_FIREBALL"),
                                  n, stopOnImpact);
  }
  if(!p) {
    // max number of projectiles in the air
    // FIXME: do something... 
  }
}

void SpellCaster::causeDamage() {
  Creature *creature = battle->getCreature();

  // roll for the spell damage
  int damage = 0;
  for(int i = 0; i < creature->getLevel(); i++) {
    damage += (int)((float)spell->getAction() * rand()/RAND_MAX);
  }

  char msg[200];
  sprintf(msg, "%s attacks %s with %s.", 
          creature->getName(), 
          creature->getTargetCreature()->getName(),
          spell->getName());
  battle->getSession()->getMap()->addDescription(msg, 1, 0.15f, 1);

  // cause damage, kill creature, gain levels, etc.
  battle->dealDamage(damage, 
                     spell->getAction() * creature->getLevel(), 
                     spell->getEffect());
}

void SpellCaster::setStateMod(int mod) {
  Creature *targets[100];
  int radius = battle->getCreature()->getLevel() * 2;
  if(radius > 15) radius = 15;

  // show radius effect
  battle->getSession()->getMap()->startEffect(battle->getCreature()->getTargetX(),
                                              battle->getCreature()->getTargetY(),
                                              1, Constants::EFFECT_RING, (Constants::DAMAGE_DURATION * 4),
                                              radius, radius);

  int targetCount = battle->getSession()->getMap()->getCreaturesInArea(battle->getCreature()->getTargetX(),
                                                                       battle->getCreature()->getTargetY(),
                                                                       radius,
                                                                       targets);
  for(int i = 0; i < targetCount; i++) {
    Creature *creature = targets[i];
    if(battle->getCreature()->isMonster() == creature->isMonster()) continue;
    creature->startEffect(Constants::EFFECT_GREEN, (Constants::DAMAGE_DURATION * 4));  
    // FIXME: should extend expiration event somehow if condition already exists
    if(creature->getStateMod(mod)) continue;    
    creature->setStateMod(mod, true);

    char msg[200];
    sprintf(msg, "%s is %s.", creature->getName(), Constants::STATE_NAMES[mod]);
    battle->getSession()->getMap()->addDescription(msg, 1, 0.15f, 1);
    
    // add calendar event to remove condition            
    // (format : sec, min, hours, days, months, years)
    Calendar *cal = battle->getSession()->getParty()->getCalendar();
    int timeInMin = 2 * battle->getCreature()->getLevel();
//    cerr << Constants::STATE_NAMES[mod] << " will expire in " << timeInMin << " minutes." << endl;
    Date d(0, timeInMin, 0, 0, 0, 0); 
    Event *e = new StateModExpirationEvent(cal->getCurrentDate(), 
                                           d, creature, mod, battle->getSession(), 1);
    cal->scheduleEvent((Event*)e);   // It's important to cast!!		
  }
}                                     

