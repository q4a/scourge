// =============================================================
// This squirrel file contains the code to handle special capabilities
//



// **********************************************
// Superior toughness
function prereqSuperTough( creature ) {
  // print( "In prereqSuperTough. Creature=" + creature.getName() + ".\n" );

  // FIXME: this is inefficient. Instead, expose Character class
  // bindings so we can say something like: 
  // creature.getClass() == scourgeGame.characterClass
  return( creature.getLevel() >= 3 &&
          ( creature.isOfRootClass( "Man-at-arms" ) ||
						creature.isOfRootClass( "Cutpurse" ) ) );
}

function actionSuperTough( creature ) {
  causeDefense( 5, "Superior toughness" );
}




// **********************************************
// Missile defense
function prereqMissileDefense( creature ) {
  return( creature.getLevel() >= 3 &&
          creature.getSkillByName( "RANGED_WEAPON" ) >= 20 );
}

function actionMissileDefense( creature ) {
  //print( "In actionMissileDefense. Creature=" + creature.getName() + ".\n" );

  // FIXME: only if missile damage
  //print( "armor=" + armor + " type=" + typeof( armor ) + "\n" );
  
  // the weapon this player is being attacked with
  weapon <- scourgeGame.getMission().getCurrentWeapon();

  // is it a ranged weapon?
  if( weapon != null ) {
  	print( "*** weapon=" + weapon.getName() + " ranged=" + weapon.isRanged().tostring() + "\n" );
    if( weapon.isRanged() ) {
      causeDefense( 25, "Dodging missiles" );
    }
  }
}


// **********************************************
// War-rage
function prereqWarrage( creature ) {
  return( creature.getSkillByName( "MELEE_WEAPON" ) >= 30 ||
  		  creature.getSkillByName( "LARGE_WEAPON" ) >= 30 ||
          creature.getSkillByName( "HAND_TO_HAND_COMBAT" ) >= 30 );
}

function actionWarrage( creature ) {

  print( "damage=" + damage + " type=" + typeof( damage ) + "\n" );

  // the weapon this player is using
  weapon <- scourgeGame.getMission().getCurrentWeapon();

  // check that the weapon's skill is more than 90%
  skillCheck <- false;
  if( weapon != null ) {
    print( "*** weapon=" + weapon.getName() + "\n" );
    skillCheck = ( !( weapon.isRanged() ) && 
                   creature.getSkill( weapon.getDamageSkill() ) >= 30 );
  } else {
    print( "*** hand-to-hand combat.\n" );
    skillCheck = ( creature.getSkillByName( "HAND_TO_HAND_COMBAT" ) >= 30 );
  }

  print( "*** skillCheck=" + skillCheck.tostring() + "\n" );

  if( skillCheck ) {
    causeDamage( 15, "War rage" );
  }
}


// **********************************************
// Killer Blow
function prereqKillerblow( creature ) {
	return( creature.isOfRootClass( "Barbarian" ) );
}

function actionKillerblow( creature ) {
	causeDamage( 10, "The terrible blow" );
}


// **********************************************
// Natural healing
function prereqNaturalHealing( creature ) {
  // print( "In prereqNaturalHealing. Creature=" + creature.getName() + ".\n" );

  // at least 25pts of nature magic
  return( creature.getSkillByName( "NATURE_MAGIC" ) >= 25 );
}

function actionNaturalHealing( creature ) {
  if( !incrementDailyCount( creature, "NaturalHealing.lastDateUsed", 2 ) ) {
    scourgeGame.printMessage( "...but already used this capability twice today." );
    return;
  }

  // Heal some points
  amount <- ( rand() * 
              ( 5.0 * creature.getLevel().tofloat() ) / 
              RAND_MAX ).tointeger();
  if( creature.getHp() + amount > creature.getMaxHp() ) 
    amount = creature.getMaxHp() - creature.getHp();
  
  if( amount == 1 ) {
    scourgeGame.printMessage( "...and heals 1 hit point." );
  } else {
    scourgeGame.printMessage( "...and heals " + amount + " hit points." );
  }
  creature.setHp( creature.getHp() + amount );
}


// **********************************************
// Healing Aura
function prereqHealingAura( creature ) {
  return( ( creature.isOfRootClass( "Warrior" ) && creature.getLevel() >= 20 ) ||
					( creature.isOfRootClass( "Scholar" ) && creature.getLevel() >= 20 ) ||
					( creature.isOfRootClass( "Healer" ) && creature.getLevel() >= 10 ) );
}

function actionHealingAura( creature ) {
	n <- creature.getLevel() / 7 + 1;
	i <- 0;	
	hp <- 0;
	healed <- false;
	for( i = 0; i < scourgeGame.getPartySize(); i++ ) {
		hp = scourgeGame.getPartyMember( i ).getHp() + n;
		if( hp > scourgeGame.getPartyMember( i ).getMaxHp() ) 
			hp = scourgeGame.getPartyMember( i ).getMaxHp();
		if( hp != scourgeGame.getPartyMember( i ).getHp() ) healed = true;
		scourgeGame.getPartyMember( i ).setHp( hp );
  }
	if( healed ) 
		scourgeGame.printMessage( creature.getName() + "'s holy innocence sooths your pains." );
}


// **********************************************
// Speedy Casting
function prereqSpeedyCast( creature ) {
	return( creature.isCharacter() &&
					creature.getSkillByName( "LIFE_AND_DEATH_MAGIC" ) >= 75 ||
					creature.getSkillByName( "DECEIT_MAGIC" ) >= 75 ||
					creature.getSkillByName( "NATURE_MAGIC" ) >= 75 ||
					creature.getSkillByName( "AWARENESS_MAGIC" ) >= 75 ||
					creature.getSkillByName( "HISTORY_MAGIC" ) >= 75 ||
          creature.getSkillByName( "CONFRONTATION_MAGIC" ) >= 75 );
}

function actionSpeedyCast( creature ) {
	// the action is handled via the waitHandlerSpell function in map.nut.
}


// **********************************************
// Vitality Transfer
function prereqVitality( creature ) {
  return( creature.getSkillByName( "AWARENESS_MAGIC" ) >= 25 &&
          creature.getLevel() >= 5 );
}

function actionVitality( creature ) {

  if( !incrementDailyCount( creature, "VitalityTransfer.lastDateUsed", 3 ) ) {
    scourgeGame.printMessage( "...but already used this capability three times today." );
    return;
  }

  i <- 0;
  aveHp <- 0.0;
  for( i = 0; i < scourgeGame.getPartySize(); i++ ) {
    aveHp += scourgeGame.getPartyMember( i ).getHp().tofloat();
  }
  aveHp /= scourgeGame.getPartySize().tofloat();
  for( i = 0; i < scourgeGame.getPartySize(); i++ ) {
    scourgeGame.getPartyMember( i ).setHp( aveHp.tointeger() );
  }

  scourgeGame.printMessage( "...the divine healing affects all party members." );
}


// **********************************************
// Terminal Energy
function prereqTerminal( creature ) {
  return( creature.isOfRootClass( "Magician" ) ||
          creature.isOfRootClass( "Scholar" ) ||
          creature.isOfRootClass( "Healer" ) );
}

function actionTerminal( creature ) {
  if( creature.getHp().tofloat() / creature.getMaxHp().tofloat() < 0.1 ) {
    if( creature.getMp() < creature.getMaxMp() ) {
      n <- ( creature.getMaxMp().tofloat() / 20.0 ).tointeger();
      creature.setMp( creature.getMp() + n );
      scourgeGame.printMessage( "...nearing death, " + creature.getName() + " gains " + n + " MP (Terminal Energy)." );
    }
  }
}



// **********************************************
// Mystic Defense
function prereqMystic( creature ) {
  return( creature.getSkillByName( "HISTORY_MAGIC" ) >= 25 );
}

function actionMystic( creature ) {
  causeDefense( 15, "Chanelling mystic energies" );
}


// **********************************************
// Small Arms Mastery
function prereqSmallArmMastery( creature ) {
  return( creature.isOfRootClass( "Cutpurse" ) );
}

function actionSmallArmMastery( creature ) {
  weapon <- scourgeGame.getMission().getCurrentWeapon();
  
  // is it a ranged weapon?
  if( weapon != null ) {

    isSmall <- weapon.hasTag( "SMALL" );

    print( "*** weapon=" + weapon.getName() + " is small=" + isSmall.tostring() + "\n" );
    
    if( isSmall ) {
      causeDamage( 10, "Small Arms Mastery" );
    }
  }
}



// **********************************************
// Neutralize Poison
function prereqNeutPoison( creature ) {
  return( creature.isOfRootClass( "Cutpurse" ) &&
          creature.getLevel() >= 10 );
}

function actionNeutPoison( creature ) {
  if( creature.getStateMod( 6 ) ) {
    scourgeGame.printMessage( "This character is not currently poisoned." );
    return;
  }

  if( !incrementDailyCount( creature, "NeutPoison.lastDateUsed", 3 ) ) {
    scourgeGame.printMessage( "You already used this capability thrice today." );
    return;
  }

  creature.setStateMod( 6, false );
}



// **********************************************
// Arcane Stance
function prereqArcaneStance( creature ) {
    return( creature.isOfRootClass( "Scholar" ) );
}

function actionArcaneStance( creature ) {
  causeDefense( 5, creature.getName() + " uses an Arcane Stance which" );
}


// **********************************************
// Bow Mastery
function prereqBowMastery( creature ) {
  return( creature.isOfRootClass( "Scholar" ) &&
          creature.getLevel() >= 10 );
}

function actionBowMastery( creature ) {
  weapon <- scourgeGame.getMission().getCurrentWeapon();

  // is it a ranged weapon?
  if( weapon != null ) {
    print( "*** weapon=" + weapon.getName() + " is ranged=" + 
           weapon.isRanged().tostring() + "\n" );  
    if( weapon.isRanged() ) {
      causeDamage( 10, "masterful bow handling" );
    }
  }
}





// =============================================================
// Utility functions
//

function causeDamage( percent, message ) {
  delta <- ( damage / 100.0 ) * percent;
  damage += delta;
  if( delta.tointeger() > 0 ) {
    scourgeGame.printMessage( "..." + message + " causes " + 
                              delta.tointeger() + " bonus damage!" );
  }
}

function causeDefense( percent, message ) {
  delta <- ( armor / 100.0 ) * percent;
  armor += delta;
  if( delta.tointeger() > 0 ) {
    scourgeGame.printMessage( "..." + message + " causes " + 
                              delta.tointeger() + " armor bonus!" );
  }
}

/**
 * Returns true if the daily count for 'key' can be incremented.
 * Returns false if the value of 'key' is already at least 'maxCount'.
 */
function incrementDailyCount( creature, key, maxCount ) {
    /*
    If the code below looks complicated, it's because I want to avoid 
    storing an infinite number of values. So, the scheme looks like this:
        
    creature_name."NaturalHealing.lastDateUsed" -> lastDateUsed
    lastDateUsed -> count
    
    where:
    lastDateUsed is a Date::getShortString() formatted date
    count is an integer in string format
  */

  // How many times have we used this capability today?
  mainKey <- encodeKeyForCreature( creature, key );
  lastDateUsed <- scourgeGame.getValue( mainKey );
  today <- scourgeGame.getDateString();
  todayKey <- encodeKeyForCreature( creature, today );
  if( lastDateUsed == null ) {
    print( "First use of this capability ever.\n" );

    // and save today's count of 1
    scourgeGame.setValue( mainKey, today )
    scourgeGame.setValue( todayKey, "1" );
  } else {
    print( "lastDateUsed=" + lastDateUsed + "\n" );
    lastDateUsedKey <- encodeKeyForCreature( creature, lastDateUsed );
    
    // is now a day later?
    if( scourgeGame.isADayLater( lastDateUsed ) ) {
      print( "Last use was more than a day ago.\n" );
      // if so, erase the count
      scourgeGame.eraseValue( lastDateUsedKey );
      
      // and save today's count of 1
      scourgeGame.setValue( mainKey, today )
      scourgeGame.setValue( todayKey, "1" );
    } else {
      print( "Last use was less than a day ago.\n" );
      // otherwise get the count
      count <- scourgeGame.getValue( lastDateUsedKey );
      if( count == null ) count = 0;
      else count = count.tointeger();
      print( "count=" + count + "\n" );
  
      if( count.tointeger() >= maxCount ) {
        // Used it too many times.
        print( "Reached max count: count=" + count + " max=" + maxCount + ".\n" );
        return false;
      } else {
        // increment the count
        count = count + 1;

        // and save it
        scourgeGame.setValue( lastDateUsedKey, count.tostring() );
      }
    }
  }
  return true;
}

/**
 * Return the "key" encoded for "creature".
 * These methods ensure that keys are unique per creature.
 */
function encodeKeyForCreature( creature, key ) {
  return creature.getName() + "." + key;
}

/**
 * Returns only the key portion of an encoded key as generated by
 * encodeKeyForCreature.
 * These methods ensure that keys are unique per creature.
 */
function decodeKeyForCreature( creature, key ) {
  delim <- key.find( "." );
  if( delim == null ) return key;
  return key.slice( delim );
}

