// =============================================================
// This squirrel file contains the code to handle interaction with map objects
//

// Called when entering map                    
function enterMap( mapName ) {
  print( "Welcome to S.C.O.U.R.G.E.: Heroes of Lesser Renown\n" );
  print( "v" + scourgeGame.getVersion() + "\n" );
  print( "You are on the " + mapName + " map.\n" );
	print( "Chapter=" + scourgeGame.getMission().getChapter() + " Depth=" + scourgeGame.getMission().getDungeonDepth() + "\n" );

	if( scourgeGame.getMission().getChapter() == 7 && mapName == "library3" ) {
		initChapter8();
	} else if( scourgeGame.getMission().getChapter() == 8 && mapName == "temple" ) {
		initChapter9();
	} else if( scourgeGame.getMission().getChapter() == 9 && mapName == "emeril" ) {
		initChapter10();
	}
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
		scourgeGame.getMission().setCompleted( true );
    creature.startConversation( creature );
  }
  return true;
}

// return true if the click was handled from squirrel
function useShape( x, y, z ) {
	print( "Shape used: " + scourgeGame.getMission().getShape( x, y, z ) + "\n" );
	print( "Depth: " + scourgeGame.getMission().getDungeonDepth() + "\n" );
	if( scourgeGame.getMission().getShape( x, y, 6 ) == "RED_TELEPORTER" ) {
		if( scourgeGame.getMission().getDungeonDepth() == 0 ) {
			print( "...going down..." );
			scourgeGame.getMission().descendDungeon( x, y, z );
		} else {
			print( "...going up..." );
			scourgeGame.getMission().ascendDungeon( x, y, z );
		}
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
    return false;
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

	return true;
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
  if( item.getName() == "Brand of Iconoclast" ) {
    useBrandOfIconoclast( creature, item );
  }
}

// assume the global var "armor" is defined as for skills
function useItemInDefense( creature, item ) {
  print( "useItemInDefense, item=" + item.getName() + "\n" ); 
}

// assume the global var "damage" exists and contains the current amount of 
// damage caused by this weapon to attacker.getTargetCreature()
// Change the value of damage to be more or less depending on items (etc)
// held/equipped by the attacker or the target.
// 
// Note: item can be null if attack is with bare hands
function damageHandler( attacker, item ) {
  if( attacker.getTargetCreature() != null ) {
    equippedItem <- attacker.getTargetCreature().getItemAtLocation( 4 );
    if( equippedItem != null ) {
      if( equippedItem.getName() == "Cloak of Safe Passage" ) damageHandlerCloakSafePass( attacker, item );
    }
  }
}

// assume the global var "spellDamage" exists and contains the current amount of 
// damage caused by this spell to caster.getTargetCreature()
// Change the value of spellDamage to be more or less depending on items (etc)
// held/equipped by the caster or the target.
function spellDamageHandler( caster, spell ) {
  if( caster.getTargetCreature() != null ) {
    item <- caster.getTargetCreature().getItemAtLocation( 4 );
    if( item != null ) {
      if( item.getName() == "Cloak of Hateful Vengeance" ) spellDamageHandlerCloakHateVen( caster, spell );
    }
  }
}

// The waitHandlerXYZ functions allow you to modify the 'wait' time of the
// item, spell or skill used in a combat turn.
// Assumem that a global variable named 'turnWait' exists. Change it to the
// desired new value, or leave as is if no change is needed.
function waitHandlerSkill( attacker, skillName ) {
//	print( "waitHandlerSkill: " + attacker.getName() + " wait=" + turnWait + "\n" );
}

function waitHandlerSpell( caster, spell ) {
//	print( "waitHandlerSpell: " + caster.getName() + " wait=" + turnWait + "\n" );
	if( caster.hasCapability( "Speedy Casting" ) == true ) {
		turnWait = turnWait * 0.75;
		scourgeGame.printMessage( caster.getName() + " conjures up magic with blinding speed!" );
	}
}

// if item is null, it's a bare-handed attack
function waitHandlerItem( attacker, item ) {
//	print( "waitHandlerItem: " + attacker.getName() + " wait=" + turnWait + "\n" );
}

// =============================================================
// Conversation methods
//

// called when conversing with someone
// return the answer string or null for no special handling
function converse( creature, question ) {
  print( "Creature=" + creature.getName() + " question:" + question + "\n" );

  // Orhithales will not give you information until the armor has been recovered
  if( creature.getName() == "Orhithales Grimwise" &&
      !( scourgeGame.getMission().isCompleted() ) &&
      ( question == "dragonscale" ||
        question == "armor" ) ) {
    return "Yes, yes... quite so... carry on!";
  } else if( creature.getName() == "Sabien Gartu" ) {
		if( !( scourgeGame.getMission().isCompleted() ) &&
				scourgeGame.getMission().getChapter() != 7 &&
				( question == "door" ||
					question == "seal" ||
					question == "sealed" ||
					question == "lock" ||
					question == "locked" ) ) {
			scourgeGame.getMission().setCompleted();
		} else if( scourgeGame.getMission().getChapter() == 7 ) {
      if( question == "after" || question == "stop" ) {
        // remove map block at 294,223 to make door accessible
        scourgeGame.getMission().removeMapPosition( 294, 223, 0 );
      }
    }
  } else if( creature.getName() == "Karzul Agmordexu" &&
             !( scourgeGame.getMission().isCompleted() ) &&
             ( question == "serves" || question == "gift" ) ) {
    scourgeGame.getMission().setCompleted();
  }	else if( creature.getName() == "Positive Energy of Hornaxx" ) {
		if( question.tolower() == "unamoin" ) {
			storePartyLevel();
		} else if( question.tolower() == "tokens" ) {
			assignAntimagicItems();
		}
	}
  return null;
}

// =============================================================
// Specific item handling
function useBrandOfIconoclast( creature, item ) {
  if( creature.getTargetCreature() != null &&
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

// spell damage reduction when wearing the Cloak of Vengeance
function spellDamageHandlerCloakHateVen( caster, spell ) {
  if( spell.getDeity() == "Unamoin" && damage > 0 ) {
    
    delta <- ( damage / 100.0 ) * 15.0;
    damage += delta;
    if( delta.tointeger() > 0 ) {
      scourgeGame.printMessage( "...damage is reduced by " + delta.tointeger() +
                                "pts! (Cloak of Hateful Vengeance)" );
    }

    incrementCloakHateVenUse( caster.getTargetCreature() );
  }
}

function incrementCloakHateVenUse( target ) {  
  key <- encodeKeyForCreature( target, "CloakOfHatefulVengeance" );
  value <- scourgeGame.getValue( key );
  if( value == null ) numValue <- 0;
  else numValue <- value.tointeger();
  numValue++;
  if( numValue >= 100 ) {
    // FIXME: do something... cloak of hateful vengeance was used 100 times
    scourgeGame.printMessage( "FIXME: do something! Cloak of Hateful Vengeance was used 100 times." );
  }
  print( "Cloak of hateful vengeance was used " + numValue + " times.\n" );
  scourgeGame.setValue( key, numValue.tostring() );
}

function damageHandlerCloakSafePass( attacker, item ) {
  if( damage > 0 ) {
    delta <- ( damage / 100.0 ) * 10.0;
    damage += delta;
    if( delta.tointeger() > 0 ) {
      scourgeGame.printMessage( "...damage is reduced by " + delta.tointeger() +
                                "pts! (Cloak of Safe Passage)" );
    }
  }
}

function initChapter8() {
	// modify sabien's intro text
	i <- 0;
	if( scourgeGame.getMission().getDungeonDepth() == 0 ) {
		for( i = 0; i < scourgeGame.getMission().getCreatureCount(); i++ ) {
			if( scourgeGame.getMission().getCreature( i ).getName() == "Sabien Gartu" ) {
				scourgeGame.getMission().getCreature( i ).setIntro( "_chapter8_" );
				break;
			}
		}
	} else if( scourgeGame.getMission().getDungeonDepth() == 1 ) {
		// everyone is cursed and poisoned
		i	<- 0;
		for( i = 0; i < scourgeGame.getPartySize(); i++ ) {
			// if not dead
			if( !( scourgeGame.getPartyMember( i ).getStateMod( 13 ) ) ) {
				// poisoned
				scourgeGame.getPartyMember( i ).setStateMod( 6, true );
				// cursed
				scourgeGame.getPartyMember( i ).setStateMod( 7, true );
			}
		}

		scourgeGame.showTextMessage( "You find yourself on a barren plain. The air is thick with moisture and very still. The bleak ground appears to emit a cancerous odor like the smell of death on a roadside cadaver. A sudden rush of despair hangs over you, clouding your senses... Is this the end? The tortured screams of unseen thousands drifts from beyond the nearest door. On this molten landscape filled with hopelessness and horror, even moving around seems to require an extra effort." );
	}
}

function initChapter9() {
	i <- 0;
	if( scourgeGame.getMission().getDungeonDepth() == 0 ) {
		
		s <- getChapter9Intro();
		if( s != null ) {
			for( i = 0; i < scourgeGame.getMission().getCreatureCount(); i++ ) {
				if( scourgeGame.getMission().getCreature( i ).getName() == "Positive Energy of Hornaxx" ) {
					scourgeGame.getMission().getCreature( i ).setIntro( s );
				} else if( scourgeGame.getMission().getCreature( i ).getName() == "Negative Energy of Hornaxx" ) {
					scourgeGame.getMission().getCreature( i ).setIntro( "_return_" );
				}
			}
		}	
	}
}

function getChapter9Intro() {
	i <- 0;
	for( i = 0; i < scourgeGame.getPartySize(); i++ ) {
		key <- encodeKeyForCreature( scourgeGame.getPartyMember( i ), "hornaxx" );
		value <- scourgeGame.getValue( key );
		if( value != null ) {
			numValue <- value.tointeger();
			if( scourgeGame.getPartyMember( i ).getLevel() > numValue ) {
				return "_return_true_";
			} else {
				return "_return_false_";
			}
		}
	}
	return null;
}

antimagicItems <- [ 
  "Boots of antimagic binding",
  "Antimagic foil hat",
  "Antimagic chestplate"
];

antimagicItemLocations <- [
	256, 1, 8
];

function initChapter10() {
	if( scourgeGame.getMission().getDungeonDepth() == 0 ) {
		scourgeGame.showTextMessage( "The ancient walls appear to cave inward as if pushed " +
																 "on by the very roots of the forest from above. " +
																 "A metallic smell cuts the air and the noises echoing " +
																 "from the deep chill your bones. " +
																 "||A grinding sound wells from beneath your feet as if the " +
																 "wheels of a great machine are turning. " + 
																 "Could this be the contdown to Mothrazu's infernal plans? " +
																 "||And as if that wasn't freeky enough, frequent quakes " +
																 "shake the dungeon..." );
	}

	// poison party members without antimagic binding armor
	i <- 0;
	found <- false;
	equippedItem <- null;
	t <- 0;
	for( i = 0; i < scourgeGame.getPartySize(); i++ ) {
		found = false;
		for( t = 0; t < antimagicItemLocations.len(); t++ ) {
			equippedItem = scourgeGame.getPartyMember( i ).getItemAtLocation( antimagicItemLocations[ t ] );
			if( equippedItem != null ) {
				if( equippedItem.getName() == antimagicItems[ t ] ) {
					found = true;
					break;
				}
			}
		}
		if( !found ) {
			scourgeGame.printMessage( 
				scourgeGame.getPartyMember( i ).getName() + 
				" feels very ill suddenly." );
			scourgeGame.getPartyMember( i ).setStateMod( 6, true );
		}
	}
}

function storePartyLevel() {
	i <- 0;
	for( i = 0; i < scourgeGame.getPartySize(); i++ ) {
		print( "storing for " + i + "\n" );
		key <- encodeKeyForCreature( scourgeGame.getPartyMember( i ), "hornaxx" );
		value <- scourgeGame.getValue( key );
		if( value == null ) {
			print( "Storing level for " + scourgeGame.getPartyMember( i ).getName() + "\n" );
			scourgeGame.setValue( key, scourgeGame.getPartyMember( i ).getLevel().tostring() );
		}
	}
}

function assignAntimagicItems() {
	i <- 0;
	t <- 0;
	b <- false;
	for( i = 0; i < scourgeGame.getPartySize(); i++ ) {
		key <- encodeKeyForCreature( scourgeGame.getPartyMember( i ), "hornaxx-antimagic" );
		value <- scourgeGame.getValue( key );
		if( value == null ) {
			// Assign items. 
			// The first player gets more more if not all party positions are filled in.
			n <- 1;
			if( i == 0 ) n = n + ( 4 - scourgeGame.getPartySize() );
			r <- 0;
			for( r = 0; r < n; r++ ) {
				scourgeGame.getPartyMember( i ).addInventoryByName( antimagicItems[ t++ ] );
				if( t >= antimagicItems.len() ) t = 0;
			}
			scourgeGame.setValue( key, "true" );
			b = true;
		}
	}
	if( b ) {
		scourgeGame.getMission().setCompleted( true );
	}
}
