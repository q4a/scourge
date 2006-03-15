// =============================================================
// This squirrel file contains the code to handle interaction with map objects
//

// Called when entering map                    
function enterMap( mapName ) {
  print( "Welcome to S.C.O.U.R.G.E.: Heroes of Lesser Renown\n" );
  print( "v" + scourgeGame.getVersion() + "\n" );
  print( "You are on the " + mapName + " map.\n" );
}

// Called when exiting map
function exitMap( mapName ) {
  print( "Ending level.\n" );
}

function testMyco() {
  i <- 0;
  for( i = 0; i < scourgeGame.getMission().getCreatureCount(); i++ ) {
    if( scourgeGame.getMission().getCreature(i).getName() == "Mycotharsius the Mad" ) {
      scourgeGame.getMission().getCreature(i).startConversation();
      break;
    }
  }
}

// Called when a creature dies (before it turns into a "corpse" container.)
function creatureDeath( creature ) {
  if( creature.getName() == "Mycotharsius the Mad" ) {
    // Myco. starts a conversation before he dies, revealing much...
    creature.startConversation( creature );
  }
  return true;
}

// deities from spells.txt
deities <- [ 
  "Unamoin",    // Confrontation
  "Soroughoz",  // Ambush Trickery and Deceit
  "Xelphate", // History and Lore
  "Garzul-Meg-Aonrod", // Life and Death,
  "Minirmuir", // Divine Awareness
  "Amod-Rheinur" // Nature
];


// Different actions based on pool's and player's deity
// Some combination does good, some does bad stuff...
function usePool( x, y, z ) {
  print( "Clicked on x=" + x + " y=" + y + " z=" + z + "\n" );
  deity <- scourgeGame.getDeityLocation( x, y, z );
  print( "Deity=" + deity + "\n" );

  player <- scourgeGame.getPlayer();
  playersDeity <- player.getDeity();
  print( "player's deity is: " + playersDeity + "\n" );

  i <- 0;
  deityIndex <- -1;
  oppositeDeity <- null;
  for( i = 0; i < deities.len(); i++ ) {
    if( deity == deities[i] ) {
      // a cheap way of calc. the opposite deity.
      // only works for an even numbered pantheon. :-)
      oppositeDeity = deities[ deities.len() - 1 - i ];
      deityIndex = i;
      break;
    }
  }
  if( deityIndex == -1 ) {
    // ERROR: something changed in spells.txt
    print( "*** Error: unknonw deity: " + deity + " did something change in data/world/spells.txt?" );
    return;
  }

  print( "Opposite deity=" + oppositeDeity );

  helped <- false;
  if( deity == playersDeity ) {
    scourgeGame.printMessage( "The mighty lord " + deity + " reaches towards you," );
    if( !incrementDailyCount( player, "deity.help.lastDateUsed", 3 ) ) {
      scourgeGame.printMessage( "...alas, the deity can aid you no more today." );
    } else {
      helped = true;
      player.setHp( player.getHp() + 
                    ( rand() * 
                      ( 2.0 * player.getLevel().tofloat() ) / 
                      RAND_MAX ).tointeger() + 1 );
      scourgeGame.printMessage( "...and fills your spirit with energy." );
    }
  } else if( oppositeDeity == playersDeity ) {
    scourgeGame.printMessage( "Your presence angers the mighty lord " + deity + "!" );
    player.takeDamage( ( rand() * 
                         ( 2.0 * player.getLevel().tofloat() ) / 
                         RAND_MAX ) + 1.0 );
    scourgeGame.printMessage( "...the deity's wrath scours your flesh!" );
  } else {
    scourgeGame.printMessage( "The mighty lord " + deity + " ignores you." );
  }

  // deity specific actions
  if( helped ) {
    mods <- [];
    modIndex <- 0;
    switch( deityIndex ) {
    case 0: // unamoin, confrontation
    player.setStateMod( scourgeGame.getStateModByName( "empowered" ), true );
    player.setStateMod( scourgeGame.getStateModByName( "ac_protected" ), true );
    break;
    case 1: // Soroughoz, ambush, trickery and deceit
    player.setStateMod( scourgeGame.getStateModByName( "blessed" ), true );
    player.setStateMod( scourgeGame.getStateModByName( "magic_protected" ), true );
    break;
    case 2: // Xelphate, history & lore
    mods = [ "drunk", "poisoned", "cursed" ];
    for( i = 0; i < mods.len(); i++ ) {  
      modIndex = scourgeGame.getStateModByName( mods[i] );
      if( player.getStateMod( modIndex ) ) player.setStateMod( modIndex, false );
    }
    break;
    case 3: // Garzul-Meg-Aonrod, life-death
    mods = [ "possessed", "blinded", "paralysed" ];
    for( i = 0; i < mods.len(); i++ ) {  
      modIndex = scourgeGame.getStateModByName( mods[i] );
      if( player.getStateMod( modIndex ) ) player.setStateMod( modIndex, false );
    }
    break;
    case 4: // Minirmuir, Divine Awareness
    player.setMp( player.getMp() + 
                  ( rand() * 
                    ( 2.0 * player.getLevel().tofloat() ) / 
                    RAND_MAX ).tointeger() + 1 );
    break;
    case 5: // Amod-Rheinur, nature
    player.setThirst( 10 );
    player.setHunger( 10 );
    break;
    }
  }

  // FIXME: sacrifizing money and items

}

// =============================================================
// Item events: they're only called for party members
//
function equipItem( creature, item ) {
  //print( "ITEM EVENT: equipItem, creature: " + creature.getName() + " item: " + item.getName() + "\n" );
}

function doffItem( creature, item ) {
  //print( "ITEM EVENT: doffItem, creature: " + creature.getName() + " item: " + item.getName() + "\n" );
}

function startBattleWithItem( creature, item ) {
  //print( "ITEM EVENT: startBattleWithItem, creature: " + creature.getName() + " item: " + item.getName() + "\n" );
}

// assume the global var "damage" is defined as for skills
function useItemInAttack( creature, item ) {
  if( item.getName() == "Brand of Iconoclast" && 
      creature.getTargetCreature() != null &&
      creature.getTargetCreature().getMaxMp() > 0 ) {
    scourgeGame.printMessage( "...Brand of Iconoclast growls in a low tone..." );
    delta <- ( damage / 100.0 ) * 10.0;
    damage += delta;
    if( delta.tointeger() > 0 ) {
      scourgeGame.printMessage( "...damage is increased by " + delta.tointeger() +
                                "pts!" );
    }
  }
}

// assume the global var "armor" is defined as for skills
function useItemInDefense( creature, item ) {
}


// =============================================================
// Conversation methods
//

// called when conversing with someone
// return the answer string or null for no special handling
function converse( creature, question ) {
//  print( "Creature=" + creature.getName() + " question:" + question + "\n" );

  // Orhithales will not give you information until the armor has been recovered
  if( creature.getName() == "Orhithales Grimwise" &&
      !( scourgeGame.getMission().isCompleted() ) &&
      ( question == "dragonscale" ||
        question == "armor" ) ) {
    return "Yes, yes... quite so... carry on!";
  } else if( creature.getName() == "Sabien Gartu" &&
             !( scourgeGame.getMission().isCompleted() ) &&
             ( question == "door" ||
               question == "seal" ||
               question == "sealed" ||
               question == "lock" ||
               question == "locked" ) ) {
    scourgeGame.getMission().setCompleted();
  }

  return null;
}

