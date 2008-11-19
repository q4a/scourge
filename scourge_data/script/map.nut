// =============================================================
// This (poorly named) squirrel file contains the code to handle interaction with map objects.
// It is also the file that handles the story-related special events.
//

// Some definitions. Maybe this should live in its own file.
MD2_STAND <- 0;
MD2_RUN <- 1;
MD2_ATTACK <- 2;
MD2_PAIN1 <- 3;
MD2_PAIN2 <- 4;
MD2_PAIN3 <- 5;
MD2_JUMP <- 6;
MD2_FLIP <- 7;
MD2_SALUTE <- 8;    
MD2_TAUNT <- 9;
MD2_WAVE <- 10;
MD2_POINT <- 11;
MD2_CRSTAND <- 12;
MD2_CRWALK <- 13;
MD2_CRATTACK <- 14;
MD2_CRPAIN <- 15;
MD2_CRDEATH <- 16;    
MD2_DEATH1 <- 17;
MD2_DEATH2 <- 18;
MD2_DEATH3 <- 19;

// weather constants
WEATHER_CLEAR <- 0
WEATHER_RAIN <- 1
WEATHER_THUNDER <- 2
WEATHER_FOG <- 4

// Called when entering map (before pressing ok)
function enterMap( mapName ) {
  print( "Welcome to S.C.O.U.R.G.E.: Heroes of Lesser Renown\n" );
  // print( "v" + scourgeGame.getVersion() + "\n" );
	print( "v" + scourgeGame.getVersion() + "\n" );
  print( "You are on the " + mapName + " map.\n" );
	print( "Chapter=" + scourgeGame.getMission().getChapter() + " Depth=" + scourgeGame.getMission().getDungeonDepth() + "\n" );
	
	if( scourgeGame.getMission().isStoryLineMission() && !scourgeGame.getMission().isReplayMap() ) {
		switch( scourgeGame.getMission().getChapter() ) {
		case 5: initChapter6(); break;
		case 7: initChapter8(); break;
		case 8: initChapter9(); break;
		case 9: initChapter10(); break;
		case 10: initChapter11(); break;
		case 11: initChapter12(); break;
		}
	}
}

// Called when entering map (after pressing ok)
function mapStarting( mapName ) {
	if( mapName == "hq" ) {
		startHqMovie();
	}
}

// Called when exiting map
function exitMap( mapName ) {
  print( "Ending level.\n" );
}

function testMyco() {
  i <- 0;
  for( i = 0; i < scourgeGame.getMission().getCreatureCount(); i++ ) {
    if( scourgeGame.getMission().getCreature(i).getMonsterType() == "Mycotharsius the Mad" ) {
      scourgeGame.getMission().getCreature(i).startConversation();
      break;
    }
  }
}

// Called when a creature dies (before it turns into a "corpse" container.)
function creatureDeath( creature ) {
  if( creature.getMonsterType() == "Mycotharsius the Mad" ) {
    // Myco. starts a conversation before he dies, revealing much...
		scourgeGame.getMission().setCompleted( true );
    creature.startConversation( creature );
  } else if( creature.getMonsterType() == "Mothrazu Pain Incarnate" ) {
		scourgeGame.getMission().setCompleted( true );
		scourgeGame.showTextMessage( _( "The creature once known as Mothrazu sinks to one knee... Her breath comes in sharp gasps, as she growls:||\"I curse you humans...You may have defeated me but by Amod's grace, I still have one task left...\"||She reaches into a pouch and throws something small and silvery into the grove of trees at the center of the room.||\"You will never find it... Karzul's gift will mature and continue where I failed. May the plants be merciful to your malevolent souls... Amod! I am close...\"||With that Mothrazu passes from this world." ) );
	} else if( creature.getMonsterType() == "Karzul Agmordexu" ) {
		startGameEndMovie();
	}
  return true;
}

function decideAction( creature ) {
	if( creature.getMonsterType() == "Karzul Agmordexu" ) {
		if( creature.getHp().tofloat() <= 150 ) {
			// karzul can save 6 times
			key <- encodeKeyForCreature( creature, "Karzul.save" );
		  value <- scourgeGame.getValue( key );
		  if( value == null ) value = "0";
		  
		  numValue <- value.tointeger();
		  print( "numValue=" + numValue.tostring() + "\n" );
		  
		  if( numValue < 10 ) {
		  	print( "healing!\n" );
		  	scourgeGame.setValue( key, ( numValue + 1 ).tostring() );
			
				// heal self via the conduit
				scourgeGame.printMessage( _( "Karzul uses his infernal conduit and heals his wounds!" ) );
				creature.setHp( creature.getHp() + ( rand() * 50.0 / RAND_MAX ).tointeger() );
				scourgeGame.getMission().setMapEffect( creature.getX(), creature.getY(), 3, // map location 
				                                       "EFFECT_TELEPORT",  												// effect 
				                                       4, 4, 																	// base size
				                                       0,																			// delay
				                                       false,																	// forever 
				                                       0, 0, 0, 														// offset
				                                       1.0, 0.05, 0.25 														// color
																							);
				
				i <- 0;
				minion_count <- 0;
				minion_type <- "Cerxezu Demon Lord";
				max_minion_count <- 3;
				minion <- null;
				dead_mod <- scourgeGame.getStateModByName( "dead" );
				for( i = 0; i < scourgeGame.getMission().getCreatureCount(); i++ ) {
						minion = scourgeGame.getMission().getCreature( i );
						if( minion.getSummoner() == creature && !minion.getStateMod( dead_mod ) ) {
							minion_count++;
						}
				}
				
				print( "minions=" + minion_count.tostring() + "\n" );
				if( minion_count < max_minion_count ) {
					print( "summoning!\n" );
					// fixme: should count number of helpers
					scourgeGame.printMessage( _( "Karzul commands his minions to surge forth and destroy his attackers!" ) );
					for( i = 0; i < max_minion_count - minion_count; i++ ) {
						minion = creature.summon( minion_type, 304, 433, 351, 464 );
						print( "new minion at: " + minion.getX().tostring() + "," + minion.getY().tostring() + "\n");
					}
				}
				return true;
		  }
		}
	}
	return false;
}

// return true if the click was handled from squirrel
function useShape( x, y, z ) {
	//print( "useShape:: x=" + x + " y=" + y + " z=" + z + " chapter=" + scourgeGame.getMission().getChapter() + " depth=" + scourgeGame.getMission().getDungeonDepth() + "\n" );
	shape <- scourgeGame.getMission().getShape( x, y, z );
	if( shape != null ) {
		print( "Shape used: " + shape + "\n" );
		print( "Depth: " + scourgeGame.getMission().getDungeonDepth() + "\n" );
		if( scourgeGame.getMission().getShape( x, y, 6 ) == "RED_TELEPORTER" ) {
			if( scourgeGame.getMission().getDungeonDepth() == 0 ) {
				print( "...going down..." );
				scourgeGame.getMission().descendDungeon( x, y, z );
			} else {
				print( "...going up..." );
				scourgeGame.getMission().ascendDungeon( x, y, z );
			}
			return true;
		} else if( shape == "TREE-EMERIL-TRUNK" || shape == "TREE-EMERIL-TOP" ) {
			handleTreeOfEmeril();
			return true;
		} else if( shape == "EW_DOOR" &&
				x == 351 && y == 396 &&
				scourgeGame.getMission().getDungeonDepth() == 0 && 
				scourgeGame.getMission().getChapter() == 11 ) {
			startChapter12Movie();
			return false;
		}
	}
	return false;
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
    scourgeGame.printMessage( format( _( "The mighty lord %s reaches towards you," ), deity ) );
    if( !incrementDailyCount( player, "deity.help.lastDateUsed", 3 ) ) {
      scourgeGame.playSound("pool-canthelp");
      scourgeGame.printMessage( _( "...alas, the deity can aid you no more today." ) );
    } else {
      helped = true;
      player.setHp( player.getHp() + 
                    ( rand() * 
                      ( 2.0 * player.getLevel().tofloat() ) / 
                      RAND_MAX ).tointeger() + 1 );
      scourgeGame.playSound("pool-helped");
      scourgeGame.printMessage( _( "...and fills your spirit with energy." ) );
    }
  } else if( oppositeDeity == playersDeity ) {
    scourgeGame.printMessage( format( _( "Your presence angers the mighty lord %s!" ), deity ) );
    player.takeDamage( ( rand() * 
                         ( 2.0 * player.getLevel().tofloat() ) / 
                         RAND_MAX ) + 1.0 );
    scourgeGame.playSound("pool-punished");
    scourgeGame.printMessage( _( "...the deity's wrath scours your flesh!" ) );
  } else {
    scourgeGame.playSound("pool-ignored");
    scourgeGame.printMessage( format( _( "The mighty lord %s ignores you." ), deity ) );
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
    equippedItem <- attacker.getTargetCreature().getEquippedItem( 4 );
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
    item <- caster.getTargetCreature().getEquippedItem( 4 );
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
		scourgeGame.printMessage( format( _( "%s conjures up magic with blinding speed!" ), caster.getName() ) );
	}
}

// if item is null, it's a bare-handed attack
function waitHandlerItem( attacker, item ) {
//	print( "waitHandlerItem: " + attacker.getName() + " wait=" + turnWait + "\n" );
}

// =============================================================
// Conversation methods
//

/** 
 * Called when conversing with someone.
 * @param creature creature the name or type of the creature (npc) you are talking to
 * @param question the first keyphrase in the list that was clicked (Look at the maps/*.cfg files for examples)
 * @return the answer string or null for no special handling
 */
function converse( creature, question ) {
  print( "Creature=" + creature.getName() + " question:" + question + "\n" );

  // Orhithales will not give you information until the armor has been recovered
  if( creature.getName() == "Orhithales Grimwise" &&
      !( scourgeGame.getMission().isCompleted() ) &&
      question == "SQ_DRAGONSCALE_ARMOR" ) {
    return _( "Yes, yes... quite so... carry on!" );
  } else if( creature.getName() == "Sabien Gartu" ) {
		if( !( scourgeGame.getMission().isCompleted() ) &&
			scourgeGame.getMission().getChapter() != 7 &&
			question == "SQ_LOCKED_DOOR" ) {	
			scourgeGame.getMission().setCompleted();
		} else if( scourgeGame.getMission().getChapter() == 7 ) {
		  if( question == "SQ_AFTER" ) {
        // remove map block at 294,223 to make door accessible
        scourgeGame.getMission().removeMapPosition( 294, 223, 0 );
      }
    }
  } else if( creature.getName() == "Karzul Agmordexu" &&
             !( scourgeGame.getMission().isCompleted() ) &&
             question == "SQ_SERVES" ) {
    scourgeGame.getMission().setCompleted();
  }	else if( creature.getName() == "Positive Energy of Hornaxx" ) {
		if( question == "SQ_UNAMOIN" ) {
			storePartyLevel();
		} else if( question == "SQ_TOKENS" ) {
			assignAntimagicItems();
		}
	} else if( creature.getName() == "Mothrazu" &&
						 question == "SQ_DOOM" ) {
		mothrazuTransforms( creature );
	} else if( creature.getName() == "Spawn of Arcanex" ) {
		if( !( scourgeGame.getMission().isCompleted() ) && 
				question == "SQ_KARZUL" ) {
			scourgeGame.getMission().setCompleted();
		}
	} else if( creature.getName() == "Nightlord" && question == "ready" ) {
		teleportPartyToSurface();
	}
  return null;
}

function teleportPartyToSurface() {
	// conversation ends
	scourgeGame.endConversation();
	
	// summon help. todo: use this value once on the surface
	scourgeGame.setValue( "guardians_summoned", "true" );
	
	// nightwards depart
	
	// ascend!
	scourgeGame.ascendToSurface();
}

function mothrazuTransforms( creature ) {
	scourgeGame.endConversation();
	replacement <- scourgeGame.getMission().
		replaceCreature( creature, 
										 "Mothrazu Pain Incarnate" );
	// statemod: 0,1,2,3,4 and 11 (blessed, empowered, enraged, ac_protected, magic_protected and invisible)
	replacement.setStateMod( 0, true );
	replacement.setStateMod( 1, true );
	replacement.setStateMod( 2, true );
	replacement.setStateMod( 3, true );
	replacement.setStateMod( 4, true );
	replacement.setStateMod( 11, true );
}

// =============================================================
// Specific item handling
function useBrandOfIconoclast( creature, item ) {
  if( creature.getTargetCreature() != null &&
      creature.getTargetCreature().getMaxMp() > 0 ) {
    scourgeGame.printMessage( _( "...Brand of Iconoclast growls in a low tone..." ) );
    delta <- ( damage / 100.0 ) * 10.0;
    damage += delta;
    if( delta.tointeger() > 0 ) {
      scourgeGame.printMessage( format( _( "...damage is increased by %d pts!" ), delta.tointeger() ) );
    }
  }
}

// spell damage reduction when wearing the Cloak of Vengeance
function spellDamageHandlerCloakHateVen( caster, spell ) {
  if( spell.getDeity() == "Unamoin" && damage > 0 ) {
    
    delta <- ( damage / 100.0 ) * 15.0;
    damage += delta;
    if( delta.tointeger() > 0 ) {
      scourgeGame.printMessage( format( _( "...damage is reduced by %d pts! (Cloak of Hateful Vengeance)" ), delta.tointeger() ) );
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
      scourgeGame.printMessage( format( _( "...damage is reduced by %d pts! (Cloak of Safe Passage)" ), delta.tointeger() ) );
    }
  }
}

// slow method, oh well
function findCreatureByType( name ) {
	i <- 0;
	creature <- null;
	for( i = 0; i < scourgeGame.getMission().getCreatureCount(); i++ ) {
		creature = scourgeGame.getMission().getCreature( i );
		if( creature.getMonsterType() == name ) {
			return creature;
		}
	}
	return null;
}

function initChapter6() {
	// lock the door
	if( scourgeGame.getMission().getDungeonDepth() == 3 ) {
		scourgeGame.getMission().setDoorLocked( 292, 209, 0, true );
	}
}

function initChapter8() {
	// modify sabien's intro text
	i <- 0;
	if( scourgeGame.getMission().getDungeonDepth() == 0 ) {

		// unlock the door
		scourgeGame.getMission().setDoorLocked( 292, 209, 0, false );

		for( i = 0; i < scourgeGame.getMission().getCreatureCount(); i++ ) {
			if( scourgeGame.getMission().getCreature( i ).getName() == "Sabien Gartu" ) {
				scourgeGame.getMission().getCreature( i ).setIntro( "SQ_CHAPTER_8" );
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

		scourgeGame.showTextMessage( _( "You find yourself on a barren plain. The air is thick with moisture and very still. The bleak ground appears to emit a cancerous odor like the smell of death on a roadside cadaver. A sudden rush of despair hangs over you, clouding your senses... Is this the end? The tortured screams of unseen thousands drifts from beyond the nearest door. On this molten landscape filled with hopelessness and horror, even moving around seems to require an extra effort." ) );
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
					scourgeGame.getMission().getCreature( i ).setIntro( "SQ_RETURN" );
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
				return "SQ_RETURN_TRUE";
			} else {
				return "SQ_RETURN_FALSE";
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
		scourgeGame.showTextMessage( _( "The ancient walls appear to cave inward as if pushed on by the very roots of the forest from above. A metallic smell cuts the air and the noises echoing from the deep chill your bones. ||A grinding sound wells from beneath your feet as if the wheels of a great machine are turning. Could this be the contdown to Mothrazu's infernal plans? ||And as if that wasn't freeky enough, frequent quakes shake the dungeon..." ) );
	}

	// poison party members without antimagic binding armor
	i <- 0;
	found <- false;
	equippedItem <- null;
	t <- 0;
	for( i = 0; i < scourgeGame.getPartySize(); i++ ) {
		found = false;
		for( t = 0; t < antimagicItemLocations.len(); t++ ) {
			equippedItem = scourgeGame.getPartyMember( i ).getEquippedItem( antimagicItemLocations[ t ] );
			if( equippedItem != null ) {
				if( equippedItem.getName() == antimagicItems[ t ] ) {
					found = true;
					break;
				}
			}
		}
		if( !found ) {
			scourgeGame.printMessage( format( _( "%s feels very ill suddenly." ), 
																				scourgeGame.getPartyMember( i ).getName() ) );
			scourgeGame.getPartyMember( i ).setStateMod( 6, true );
		}
	}

	// start the earthquakes
	scourgeGame.getMission().setQuakesEnabled( true );
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
				scourgeGame.getPartyMember( i ).addToBackpackByName( antimagicItems[ t++ ] );
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

function _( s ) {
	r <- scourgeGame.getTranslatedString( s );
	return r;
}

function noop( s ) {
	return s;
}

function initChapter11() {
	if( scourgeGame.getMission().getDungeonDepth() == 0 ) {
		scourgeGame.showTextMessage( _( "A baleful wind blows bringing the scent of slow vegetative decay. Off in the distance you glance a mighty tree, standing in the center of a barren plane of ashen stone.||No other plants dare intrude upon its territory and no animal life stirs near its accursed branches.||Above, clouds darken the sky as if to signal the long awaited fulfillment of a pending doom." ) );
		
		// Add creature for spawn of arcanex (need in order to converse w. tree)
		// This has to be done each time the map is entered b/c npc-s aren't saved with the map.
		//print( "Adding creature: count=" + scourgeGame.getMission().getCreatureCount() );
		scourgeGame.getMission().addCreature( 5, 5, 0, "Spawn of Arcanex" );
		scourgeGame.getMission().setMapConfig( "/maps/tree" );
	}
}

function initChapter12() {
	if( scourgeGame.getMission().getDungeonDepth() == 0 ) {
				
		scourgeGame.getMission().setMapEffect( 312, 446, 1, // map location 
		                                       "EFFECT_SMOKE",  												// effect 
		                                       1, 1, 																	// base size
		                                       0,																			// delay
		                                       true,																	// forever 
		                                       0, 0, 0, 														// offset
		                                       0.2, 0.2, 0.5 														// color
																					);	
		scourgeGame.getMission().setMapEffect( 317, 438, 1, // map location 
				                                       "EFFECT_SMOKE",  												// effect 
				                                       1, 1, 																	// base size
				                                       0,																			// delay
				                                       true,																	// forever 
				                                       0, 0, 0, 														// offset
				                                       0.2, 0.2, 0.5 														// color
																							);
		scourgeGame.getMission().setMapEffect( 319, 455, 1, // map location 
				                                       "EFFECT_SMOKE",  												// effect 
				                                       1, 1, 																	// base size
				                                       0,																			// delay
				                                       true,																	// forever 
				                                       0, 0, 0, 														// offset
				                                       0.2, 0.2, 0.5 														// color
																							);
		scourgeGame.getMission().setMapEffect( 331, 455, 1, // map location 
				                                       "EFFECT_SMOKE",  												// effect 
				                                       1, 1, 																	// base size
				                                       0,																			// delay
				                                       true,																	// forever 
				                                       0, 0, 0, 														// offset
				                                       0.2, 0.2, 0.5 														// color
																							);
		scourgeGame.getMission().setMapEffect( 330, 438, 1, // map location 
				                                       "EFFECT_SMOKE",  												// effect 
				                                       1, 1, 																	// base size
				                                       0,																			// delay
				                                       true,																	// forever 
				                                       0, 0, 0, 														// offset
				                                       0.2, 0.2, 0.5 														// color
																							);
		
		
		// reset some creatures
		karzul <- findCreatureByType( "Karzul Agmordexu" );
		if( karzul != null ) {
			while( karzul != null ) {
				karzul.remove();
				karzul = findCreatureByType( "Karzul Agmordexu" );
			}
			karzul <- scourgeGame.getMission().addCreature( 320, 449, 0, "Karzul Agmordexu" );
			karzul.setNpc( false ); // karzul attacks			
	
			if( scourgeGame.getValue( "guardians_summoned" ) == "true" ) {
				norill <- findCreatureByType( "Norill" );
				while( norill != null ) {
					norill.remove();
					norill = findCreatureByType( "Norill" );
				}
				scourgeGame.getMission().addCreature( 338, 454, 0, "Norill" );
				
				sarrez <- findCreatureByType( "Sarrez" );
				while( sarrez != null ) {
					sarrez.remove();
					sarrez = findCreatureByType( "Sarrez" );
				}
				scourgeGame.getMission().addCreature( 305,454, 0, "Sarrez" );
				
				rolan <- findCreatureByType( "Rolan" );
				while( rolan != null ) {
					rolan.remove();
					rolan = findCreatureByType( "Rolan" );
				}			
				scourgeGame.getMission().addCreature( 332,416, 0, "Rolan" );
				
				scourgeGame.setWeather( WEATHER_RAIN | WEATHER_THUNDER );
	
				scourgeGame.showTextMessage( _( "Under the pale sky like the blackened husk of a burned tree stands the demon. He calmly awaits your charge fully aware of its futility.||But a red streak lines the mountain tops to the east... is it dawn? Or is it a sign of hope from the gods? Looking around you notice you are no longer alone on the charred battlefield. Three tall warriors in archaic clothing appear as if from nowhere. The tallest one - their leader? - silently bows his head to you.||After this briefest of courtesies, he turns to face Karzul with a steely resolve." ) );
			}
		}
	} else if( scourgeGame.getMission().getDungeonDepth() == 2 ) {
		//print( "chapter 12 map\n" );
		scourgeGame.getMission().setMapEffect( 296, 264, 2, // map location 
		                                       "EFFECT_SMOKE",  												// effect 
		                                       3, 3, 																	// base size
		                                       0,																			// delay
		                                       true,																	// forever 
		                                       0, 0, 0, 														// offset
		                                       0.3, 0.7, 0.3 														// color
																					);
		scourgeGame.getMission().setMapEffect( 326, 264, 2, // map location 
				                                       "EFFECT_SMOKE",  												// effect 
				                                       3, 3, 																	// base size
				                                       0,																			// delay
				                                       true,																	// forever 
				                                       0, 0, 0, 														// offset
				                                       0.7, 0.3, 0.3 														// color
																							);
		scourgeGame.getMission().setMapEffect( 311, 271, 2, // map location 
						                                       "EFFECT_SMOKE",  												// effect 
						                                       3, 3, 																	// base size
						                                       0,																			// delay
						                                       true,																	// forever 
						                                       0, 0, 0, 														// offset
						                                       0.3, 0.3, 0.7 														// color
																									);
		scourgeGame.getMission().setMapEffect( 296, 278, 2, // map location 
						                                       "EFFECT_SMOKE",  												// effect 
						                                       3, 3, 																	// base size
						                                       0,																			// delay
						                                       true,																	// forever 
						                                       0, 0, 0, 														// offset
						                                       0.3, 0.7, 0.7 														// color
																									);
		scourgeGame.getMission().setMapEffect( 326, 279, 2, // map location 
						                                       "EFFECT_SMOKE",  												// effect 
						                                       3, 3, 																	// base size
						                                       0,																			// delay
						                                       true,																	// forever 
						                                       0, 0, 0, 														// offset
						                                       0.7, 0.3, 0.7 														// color
																									);

		// the nightwards depart
		if( scourgeGame.getValue( "guardians_summoned" ) == "true" ) {
			creature <- null;
			i <- 0;
			for( i = 0; i < scourgeGame.getMission().getCreatureCount(); i++ ) {
				creature = scourgeGame.getMission().getCreature( i );
				if( creature.isNpc() ) {
					creature.setVisible( false );
				}
			}
		}
		
	}
}

function outdoorMapCompleted( mapName ) {
	print( "Customizing map: " + mapName + "\n" );
	if( scourgeGame.getMission().getChapter() == 10 && scourgeGame.getMission().isStoryLineMission() ) {
	
		// level the area in the center
		x <- 0;
		y <- 0;
		for( x = 250; x < 350; x++ ) {
			for( y = 250; y < 350; y++ ) {
				scourgeGame.getMission().setHeightMap( x, y, 3.0 );
				scourgeGame.getMission().removeMapPosition( x, y, 0 );
				// color the ground
			}
		}
		
		// add the tree of emeril
		scourgeGame.getMission().setMapPosition( 312, 288, 0, "TREE-EMERIL-TRUNK" );
		scourgeGame.getMission().setMapPosition( 300, 300, 12, "TREE-EMERIL-TOP" );
		
		// add decoration around tree

		// link to cfg file (contains conversations)


	}
}

function handleTreeOfEmeril() {
	hasCronostar <- false;
	i <- 0;
	t <- 0;
	item <- null;
	print( "Looking for cronostar...\n" );
	for( i = 0; hasCronostar == false && i < scourgeGame.getPartySize(); i++ ) {
		print( "\tparty member=" + i + "...\n" );
		for( t = 0; t < scourgeGame.getPartyMember( i ).getBackpackContentsCount(); t++ ) {
			item = scourgeGame.getPartyMember( i ).getBackpackItem( t );
			if( item.getName() == "Cronostar" ) {
				hasCronostar = true;
				break;	
			}
		}
	}
	print( "found it? " + ( hasCronostar ? "true" : "false" ) + "\n" );
	if( hasCronostar ) {
		for( i = 0; i < scourgeGame.getMission().getCreatureCount(); i++ ) {
			print( "\tcreature=" + scourgeGame.getMission().getCreature(i).getMonsterType() + "\n" );
			if( scourgeGame.getMission().getCreature(i).getMonsterType() == "Spawn of Arcanex" ) {
				print( "\t\tstarting conversation!\n" );
				scourgeGame.getMission().getCreature(i).startConversation();
				break;
			}
		}
	}
}
