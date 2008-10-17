/***************************************************************************
                 spellcaster.cpp  -  Currently cast spell class
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
#include "common/constants.h"
#include "spellcaster.h"
#include "render/renderlib.h"
#include "item.h"
#include "creature.h"
#include "events/event.h"
#include "events/potionexpirationevent.h"
#include "events/statemodexpirationevent.h"
#include "calendar.h"
#include "rpg/rpglib.h"
#include "projectile.h"
#include "shapepalette.h"
#include "sqbinding/sqbinding.h"
#include "debug.h"

using namespace std;

/*
  In the future we can employ something more sophisticated than these if structures...
 */

SpellCaster::SpellCaster( Battle *battle, Spell *spell, bool projectileHit, int level ) {
	this->battle = battle;
	this->spell = spell;
	this->projectileHit = projectileHit;
	this->level = level;

	Creature *creature = battle->getCreature();

	// calculate spell's power (0-100)
	// Used only for HP/AC restore spells
	power = level +
	        creature->getSkill( Skill::LUCK ) +
	        Util::roll( 0.0f, 30.0f );
}

SpellCaster::~SpellCaster() {
}

void SpellCaster::spellFailed() {
	if ( !spell ) return;

	// print patronizing message...
	battle->getSession()->getGameAdapter()->writeLogMessage( Constants::getMessage( Constants::SPELL_FAILED_MESSAGE ), Constants::MSGTYPE_FAILURE );

	// put code here for spells with something spectacular when they fail...
	// (fouled fireball decimates party, etc.)
	if ( !strcasecmp( spell->getName(), "Burning stare" ) ||
	        !strcasecmp( spell->getName(), "Silent knives" ) ||
	        !strcasecmp( spell->getName(), "Stinging light" ) ||
	        !strcasecmp( spell->getName(), "Unholy Decimator" ) ||
	        !strcasecmp( spell->getName(), "Blast of Fury" ) ) {

		Creature *tmpTarget;
		if ( battle->getCreature()->isMonster() ||
		        battle->getCreature()->getStateMod( StateMod::possessed ) ) {
			tmpTarget = battle->getSession()->getClosestMonster( toint( battle->getCreature()->getX() ),
			            toint( battle->getCreature()->getY() ),
			            battle->getCreature()->getShape()->getWidth(),
			            battle->getCreature()->getShape()->getDepth(),
			            20 );
		} else {
			tmpTarget = battle->getSession()->getParty()->getClosestPlayer( toint( battle->getCreature()->getX() ),
			            toint( battle->getCreature()->getY() ),
			            battle->getCreature()->getShape()->getWidth(),
			            battle->getCreature()->getShape()->getDepth(),
			            20 );
		}
		if ( tmpTarget ) {
			char message[200];
			snprintf( message, 200, _( "...fumble: hits %s instead!" ), tmpTarget->getName() );
			battle->getSession()->getGameAdapter()->writeLogMessage( message, Constants::MSGTYPE_FAILURE );
			Creature *oldTarget = battle->getCreature()->getTargetCreature();
			battle->getCreature()->setTargetCreature( tmpTarget );

			causeDamage( true, 0, 0.5 );

			battle->getCreature()->setTargetCreature( oldTarget );
		}
	} else if ( !strcasecmp( spell->getName(), "Teleportation" ) ) {
		battle->getSession()->getGameAdapter()->teleport( false );
	}
}

void SpellCaster::spellSucceeded() {
	if ( !spell ) return;

	int panning = battle->getSession()->getMap()->getPanningFromMapXY( battle->getCreature()->getX(), battle->getCreature()->getY() );
	battle->getSession()->playSound( spell->getSound(), panning );

//  cerr << "SUCCEEDED: " << spell->getName() << " power=" << power << endl;
	if ( !strcasecmp( spell->getName(), "Lesser healing touch" ) ||
	        !strcasecmp( spell->getName(), "Greater healing touch" ) ||
	        !strcasecmp( spell->getName(), "Divine healing touch" ) ) {
		increaseHP();
	} else if ( !strcasecmp( spell->getName(), "Body of stone" ) ) {
		increaseAC();
	} else if ( !strcasecmp( spell->getName(), "Burning stare" ) ||
	            !strcasecmp( spell->getName(), "Silent knives" ) ) {
		if ( projectileHit ) {
			causeDamage();
		} else {
			launchProjectile( 1 );
		}
	} else if ( !strcasecmp( spell->getName(), "Stinging light" ) ) {
		if ( projectileHit ) {
			causeDamage( false );
		} else {
			launchProjectile( 0 );
		}
	} else if ( !strcasecmp( spell->getName(), "Invisibility" ) ) {
		setStateMod( StateMod::invisible );
	} else if ( !strcasecmp( spell->getName(), "Art of protection" ) ) {
		setStateMod( StateMod::magic_protected );
	} else if ( !strcasecmp( spell->getName(), "Divine Aggressor" ) ) {
		setStateMod( StateMod::enraged );
	} else if ( !strcasecmp( spell->getName(), "Protective clay" ) ) {
		setStateMod( StateMod::ac_protected );
	} else if ( !strcasecmp( spell->getName(), "Empower friend" ) ) {
		setStateMod( StateMod::empowered );
	} else if ( !strcasecmp( spell->getName(), "Bless group" ) ) {
		setStateMod( StateMod::blessed );
	} else if ( !strcasecmp( spell->getName(), "Flame of Azun" ) ) {
		if ( projectileHit ) {
			setStateMod( StateMod::blinded );
		} else {
			launchProjectile( 1, false );
		}
	} else if ( !strcasecmp( spell->getName(), "Poison of ignorance" ) ) {
		if ( projectileHit ) {
			setStateMod( StateMod::poisoned );
		} else {
			launchProjectile( 1, false );
		}
	} else if ( !strcasecmp( spell->getName(), "Transmute poison" ) ) {
		setStateMod( StateMod::poisoned, false );
	} else if ( !strcasecmp( spell->getName(), "Cursed ways" ) ) {
		if ( projectileHit ) {
			setStateMod( StateMod::cursed );
		} else {
			launchProjectile( 1, false );
		}
	} else if ( !strcasecmp( spell->getName(), "Remove curse" ) ) {
		setStateMod( StateMod::cursed, false );
	} else if ( !strcasecmp( spell->getName(), "Enthrall fiend" ) ) {
		setStateMod( StateMod::possessed );
	} else if ( !strcasecmp( spell->getName(), "Break from possession" ) ) {
		setStateMod( StateMod::possessed, false );
	} else if ( !strcasecmp( spell->getName(), "Ole Taffy's purty colors" ) ) {
		viewInfo();
	} else if ( !strcasecmp( spell->getName(), "Ring of Harm" ) ) {
		circleAttack();
	} else if ( !strcasecmp( spell->getName(), "Malice Storm" ) ) {
		hailAttack();
	} else if ( !strcasecmp( spell->getName(), "Blast of Fury" ) ) {
		if ( projectileHit ) {
			causeDamage();
		} else {
			Session *session = battle->getSession();
			launchProjectile( 1, true,
			                  new EffectProjectileRenderer( session->getMap(),
			                                                session->getPreferences(),
			                                                session->getShapePalette(),
			                                                spell->getEffect(),
			                                                3000 ) );
		}
	} else if ( !strcasecmp( spell->getName(), "Unholy Decimator" ) ) {
		causeDamage();
	} else if ( !strcasecmp( spell->getName(), "Recall to life" ) ) {
		resurrect();
	} else if ( !strcasecmp( spell->getName(), "Teleportation" ) ) {
		battle->getSession()->getGameAdapter()->teleport();
	} else if ( !strcasecmp( spell->getName(), "Dori's Tumblers" ) ) {
		openLocked();
	} else if ( !strcasecmp( spell->getName(), "Gust of wind" ) ) {
		windAttack();
	} else {
		// default
		cerr << "*** ERROR: Implement spell " << spell->getName() << endl;
	}
}



void SpellCaster::viewInfo() {
	Creature *creature = battle->getCreature();
	Item *item = creature->getTargetItem();
	creature->startEffect( spell->getEffect(), ( Constants::DAMAGE_DURATION * 4 ) );
	if ( item ) {
		if ( !battle->getSession()->getGameAdapter()->isHeadless() )
			battle->getSession()->getGameAdapter()->showItemInfoUI( item,
			    creature->getSkill( Skill::IDENTIFY_ITEM ) +
			    creature->getSkill( spell->getSchool()->getSkill() ) );
	} else {
		int sx, sy, sz;
		creature->getTargetLocation( &sx, &sy, &sz );
		if ( sx > 0 && sy > 0 ) {

			// move spell focus to selected area
			int spellEffectSize = 4;
			int radius = spell->getDistance();

			// show radius effect
			battle->getSession()->getMap()->startEffect( sx, sy, 1,
			    Constants::EFFECT_RING, ( Constants::DAMAGE_DURATION * 4 ),
			    radius, radius );

			// walk around the circle
			enum { MSG_SIZE = 200 };
			char msg[ MSG_SIZE ];
			DisplayInfo di;
			di.red = 0.1f;
			di.green = 0.1f;
			di.blue = 0.2f;
			for ( int r = spellEffectSize; r; r += spellEffectSize ) {
				if ( r > radius ) r = radius;
				for ( int angle = 0; angle < 360; angle += 10 ) {
					int x = toint( sx + ( static_cast<float>( r ) * Constants::cosFromAngle( static_cast<float>( angle ) ) ) );
					int y = toint( sy - ( static_cast<float>( r ) * Constants::sinFromAngle( static_cast<float>( angle ) ) ) );
					if ( x >= 0 && x < MAP_WIDTH && y >= 0 && y < MAP_DEPTH ) {
						battle->getSession()->getMap()->startEffect( x, y, 1, Constants::EFFECT_GREEN,
						    ( Constants::DAMAGE_DURATION * 4 ),
						    spellEffectSize, spellEffectSize,
						    ( GLuint )( r * 25 ), false, &di );

						// check for secret doors
						Location *pos = battle->getSession()->getMap()->getLocation( x, y, 0 );
						if ( pos ) {
							if ( battle->getSession()->getMap()->isSecretDoor( pos ) ) {
								if ( !battle->getSession()->getMap()->isSecretDoorDetected( pos ) ) {
									battle->getSession()->getMap()->setSecretDoorDetected( pos );
									snprintf( msg, MSG_SIZE, _( "%s discovers a secret door!" ), creature->getName() );
									battle->getSession()->getGameAdapter()->writeLogMessage( msg, Constants::MSGTYPE_MISSION );
								}
							}
						}

						// check for traps
						int trapIndex = battle->getSession()->getMap()->getTrapAtLoc( x, y );
						if ( trapIndex > -1 ) {
							Trap *trap = battle->getSession()->getMap()->getTrapLoc( trapIndex );
							if ( !trap->discovered ) {
								trap->discovered = true;
								snprintf( msg, MSG_SIZE, _( "%s discovers a trap!" ), creature->getName() );
								battle->getSession()->getGameAdapter()->writeLogMessage( msg, Constants::MSGTYPE_MISSION );
							}
						}
					}
				}

				if ( r >= radius ) break;
			}
		} else {
			cerr << "*** Warning: implement ole' taffy for creature targets!" << endl;
		}
	}
}

void SpellCaster::increaseHP() {
	Creature *creature = battle->getCreature();

	int n = spell->getAction();
	n += Util::dice( static_cast<int>( n * power / 100 ) );

	if ( n + creature->getTargetCreature()->getHp() > creature->getTargetCreature()->getMaxHp() )
		n = creature->getTargetCreature()->getMaxHp() - creature->getTargetCreature()->getHp();
	creature->getTargetCreature()->setHp( static_cast<int>( creature->getTargetCreature()->getHp() + n ) );
	char msg[200];
	snprintf( msg, 200, _( "%s heals %d points." ), creature->getTargetCreature()->getName(), n );
	battle->getSession()->getGameAdapter()->writeLogMessage( msg, Constants::MSGTYPE_STATS );
	if ( creature->getCharacter() ) {
		battle->getSession()->getGameAdapter()->writeLogMessage( msg, Constants::MSGTYPE_PLAYERMAGIC );
	} else {
		battle->getSession()->getGameAdapter()->writeLogMessage( msg, Constants::MSGTYPE_NPCMAGIC );
	}
	creature->getTargetCreature()->startEffect( Constants::EFFECT_SWIRL, ( Constants::DAMAGE_DURATION * 4 ) );
}

void SpellCaster::increaseAC() {
	Creature *creature = battle->getCreature();
	int n = spell->getAction();
	n += Util::dice( static_cast<int>( n * power / 100 ) );

	int timeInMin = 15 + ( level / 2 );

	//  cerr << "increaseAC: points=" << n << " time=" << timeInMin << " min-s." << endl;

	creature->getTargetCreature()->setBonusArmor( creature->getTargetCreature()->getBonusArmor() + n );
	char msg[200];
	snprintf( msg, 200, _( "%s feels impervious to damage!" ), creature->getTargetCreature()->getName() );
	battle->getSession()->getGameAdapter()->writeLogMessage( msg, Constants::MSGTYPE_STATS );
	creature->getTargetCreature()->startEffect( Constants::EFFECT_SWIRL, ( Constants::DAMAGE_DURATION * 4 ) );

	// add calendar event to remove armor bonus
	// (format : sec, min, hours, days, months, years)
	Date d( 0, timeInMin, 0, 0, 0, 0 );
	Event *e = new PotionExpirationEvent( battle->getSession()->getParty()->getCalendar()->getCurrentDate(),
	    d, creature->getTargetCreature(),
	    Constants::getPotionSkillByName( "AC" ), n,
	    battle->getSession(), 1 );
	battle->getSession()->getParty()->getCalendar()->scheduleEvent( ( Event* )e );   // It's important to cast!!

}

Projectile *SpellCaster::launchProjectile( int count, bool stopOnImpact, ProjectileRenderer *renderer ) {
	Creature *creature = battle->getCreature();

	// maxcount for spells means number of projectiles
	// (for missiles it means how many can be in the air at once.)
	int n = count;
	if ( n <= 0 ) {
		n = level;
		if ( n < 1 ) n = 1;
	}

	if ( !renderer ) {
		renderer =
		  new ShapeProjectileRenderer( battle->getSession()->
		                               getShapePalette()->
		                               findShapeByName( "SPELL_FIREBALL" ) );
	}

	Projectile *p;
	if ( creature->getTargetCreature() ) {
		p = Projectile::addProjectile( creature,
		    creature->getTargetCreature(),
		    spell,
		    renderer,
		    n, stopOnImpact );
	} else {
		int x, y, z;
		creature->getTargetLocation( &x, &y, &z );
		p = Projectile::addProjectile( creature,
		    x, y,
		    1, 1,
		    spell,
		    renderer,
		    n, stopOnImpact );
	}
	if ( !p ) {
		char msg[ 200 ];
		snprintf( msg, 200, _( "...%s has finished firing a volley" ), creature->getName() );
		if ( creature->getCharacter() ) {
			battle->getSession()->getGameAdapter()->writeLogMessage( msg, Constants::MSGTYPE_PLAYERMAGIC );
		} else {
			battle->getSession()->getGameAdapter()->writeLogMessage( msg, Constants::MSGTYPE_NPCMAGIC );
		}
	} else {
		p->setCasterLevel( level );
	}
	return p;
}

void SpellCaster::causeDamage( bool multiplyByLevel, GLuint delay, GLfloat mult ) {
	Creature *creature = battle->getCreature();

	// roll for the spell damage
	float damage = 0;
	for ( int i = 0; i <= ( level / 2 ); i++ ) {
		damage += spell->getAction();
		if ( !multiplyByLevel ) break;
	}
	damage *= mult;

	// give early-game spells a chance
	bool lowDamage = damage < 5;

	// dodge saves for half damage
	enum { MSG_SIZE = 200 };
	char msg[ MSG_SIZE ];
	float skill = Util::roll( 0.0f, creature->getSkill( spell->getSchool()->getSkill() ) );
	float dodge = Util::roll( 0.0f, creature->getTargetCreature()->getDodge( creature ) );
	if ( skill < dodge ) {
		damage /= 2.0f;
		snprintf( msg, MSG_SIZE, _( "%s dodges some of the damage." ),
		          creature->getTargetCreature()->getName() );
		if ( creature->getTargetCreature()->getCharacter() ) {
			battle->getSession()->getGameAdapter()->writeLogMessage( msg, Constants::MSGTYPE_PLAYERBATTLE );
		} else {
			battle->getSession()->getGameAdapter()->writeLogMessage( msg, Constants::MSGTYPE_NPCBATTLE );
		}
	}

	// check for resistance
	int resistance = creature->getTargetCreature()->getSkill( spell->getSchool()->getResistSkill() );
	if ( resistance > 0 && !lowDamage ) {
		damage -= ( ( static_cast<float>( damage ) / 150.0f ) * resistance );
	}

	snprintf( msg, MSG_SIZE, _( "%1$s attacks %2$s with %3$s." ),
	          creature->getName(),
	          creature->getTargetCreature()->getName(),
	          spell->getDisplayName() );
	if ( creature->getCharacter() ) {
		battle->getSession()->getGameAdapter()->writeLogMessage( msg, Constants::MSGTYPE_PLAYERMAGIC );
	} else {
		battle->getSession()->getGameAdapter()->writeLogMessage( msg, Constants::MSGTYPE_NPCMAGIC );
	}
	if ( resistance > 0 && !lowDamage ) {
		snprintf( msg, MSG_SIZE, _( "%s resists the spell with %d." ),
		          creature->getTargetCreature()->getName(),
		          resistance );
		if ( creature->getTargetCreature()->getCharacter() ) {
			battle->getSession()->getGameAdapter()->writeLogMessage( msg, Constants::MSGTYPE_PLAYERBATTLE );
		} else {
			battle->getSession()->getGameAdapter()->writeLogMessage( msg, Constants::MSGTYPE_NPCBATTLE );
		}
	}

	// script spell resistance
	if( battle->getCreature()->getCharacter() && SQUIRREL_ENABLED ) {
		battle->getSession()->getSquirrel()->setGlobalVariable( "damage", damage );
		battle->getSession()->getSquirrel()->callSpellEvent( creature, spell, "spellDamageHandler" );
		damage = battle->getSession()->getSquirrel()->getGlobalVariable( "damage" );
	}

	char tmp[255];
	snprintf( tmp, 255, _( "%s the %s spell" ),
	          Constants::getMessage( Constants::CAUSE_OF_DEATH ),
	          spell->getDisplayName() );
	creature->getTargetCreature()->setPendingCauseOfDeath( tmp );

	// cause damage, kill creature, gain levels, etc.
	battle->dealDamage( damage,
	                    spell->getEffect(),
	                    true, delay );
}

void SpellCaster::resurrect() {
	for ( int i = 0; i < battle->getSession()->getParty()->getPartySize(); i++ ) {
		if ( battle->getSession()->getParty()->getParty( i )->getStateMod( StateMod::dead ) ) {
			battle->getSession()->getParty()->getParty( i )->
			resurrect( toint( battle->getCreature()->getX() ),
			           toint( battle->getCreature()->getY() ) );
		}
	}
}

void SpellCaster::setStateMod( int mod, bool setting ) {
	Creature *targets[100];
	int targetCount = 0;

	if ( spell->isPartyTargetAllowed() ) {
		for ( int i = 0; i < battle->getSession()->getParty()->getPartySize(); i++ ) {
			if ( spell->isPartyTargetAllowed() ) {
				targets[targetCount++] = battle->getSession()->getParty()->getParty( i );
			}
		}
	} else if ( spell->getTargetType() == GROUP_TARGET ) {
		int radius = level * 2;
		if ( radius > 15 ) radius = 15;
		if ( radius < 2 ) radius = 2;
//    cerr << "radius=" << radius << endl;
		// show radius effect
		battle->getSession()->getMap()->startEffect( battle->getCreature()->getTargetX(),
		    battle->getCreature()->getTargetY(),
		    1,
		    Constants::EFFECT_RING,
		    ( Constants::DAMAGE_DURATION * 4 ),
		    radius, radius );

		targetCount =
		  battle->getSession()->getMap()->
		  getCreaturesInArea( battle->getCreature()->getTargetX(),
		                      battle->getCreature()->getTargetY(),
		                      radius,
		                      ( RenderedCreature** )targets );
//    cerr << "targetCount=" << targetCount << endl;
	} else {
		targets[targetCount++] = battle->getCreature()->getTargetCreature();
	}





	for ( int i = 0; i < targetCount; i++ ) {
		Creature *creature = targets[i];

		bool protectiveItem = false;
		if ( !StateMod::stateMods[ mod ]->isStateModTransitionWanted( setting ) ) {
			//if(!Constants::isStateModTransitionWanted(mod, setting)) {

			// bad effects should only happen to enemies
			if ( !battle->getCreature()->canAttack( creature ) ) continue;

			// roll for resistance
			enum { MSG_SIZE = 200 };
			char msg[ MSG_SIZE ];
			if ( Util::dice( 100 ) < creature->getSkill( spell->getSchool()->getResistSkill() ) ) {
				snprintf( msg, MSG_SIZE, _( "%s resists the spell! [%d]" ),
				          creature->getName(),
				          creature->getSkill( spell->getSchool()->getResistSkill() ) );
				if ( creature->getCharacter() ) {
					battle->getSession()->getGameAdapter()->writeLogMessage( msg, Constants::MSGTYPE_PLAYERBATTLE );
				} else {
					battle->getSession()->getGameAdapter()->writeLogMessage( msg, Constants::MSGTYPE_NPCBATTLE );
				}
				continue;
			}

			// check for magic item state mod protections
			protectiveItem = creature->getProtectedStateMod( mod );
			if ( protectiveItem && 0 == Util::dice( 2 ) ) {
				snprintf( msg, MSG_SIZE, _( "%s resists the spell with magic item!" ),
				          creature->getName() );
				if ( creature->getCharacter() ) {
					battle->getSession()->getGameAdapter()->writeLogMessage( msg, Constants::MSGTYPE_PLAYERBATTLE );
				} else {
					battle->getSession()->getGameAdapter()->writeLogMessage( msg, Constants::MSGTYPE_NPCBATTLE );
				}
				continue;
			}
		}

		if ( !strcasecmp( spell->getName(), "Remove curse" ) ) {
			// remove cursed items
			if ( creature->removeCursedItems() ) {
				battle->getSession()->getGameAdapter()->writeLogMessage( _( "Cursed items have been doffed." ) );
			}
		}

		int timeInMin = 5 * level;
		if ( protectiveItem ) timeInMin /= 2;
		creature->startEffect( spell->getEffect(), ( Constants::DAMAGE_DURATION * 4 ) );

		// extend expiration event somehow if condition already exists
		Event *e = creature->getStateModEvent( mod );
		if ( creature->getStateMod( mod ) == setting ) {
			if ( e ) {
//        cerr << "Extending existing event." << endl;
				e->setNbExecutionsToDo( timeInMin );
			}
			continue;
		}
		creature->setStateMod( mod, setting );
		enum { MSG_SIZE = 200 };
		char msg[ MSG_SIZE ];
		if ( setting ) {
			snprintf( msg, MSG_SIZE, StateMod::stateMods[mod]->getSetState(), creature->getName() );
		} else {
			snprintf( msg, MSG_SIZE, StateMod::stateMods[mod]->getUnsetState(), creature->getName() );
		}
		battle->getSession()->getGameAdapter()->writeLogMessage( msg, Constants::MSGTYPE_STATS );

		// cancel existing event if any
		if ( e ) {
//      cerr << "Cancelling existing event." << endl;
			battle->getSession()->getParty()->getCalendar()->cancelEvent( e );
		}

		if ( setting ) {
			// add calendar event to remove condition
			// (format : sec, min, hours, days, months, years)
			Calendar *cal = battle->getSession()->getParty()->getCalendar();
			//    cerr << Constants::STATE_NAMES[mod] << " will expire in " << timeInMin << " minutes." << endl;
			Date d( 0, 15, 0, 0, 0, 0 );
//      cerr << "Creating new event." << endl;
			e = new StateModExpirationEvent( cal->getCurrentDate(),
			    d, creature, mod, battle->getSession(),
			    timeInMin );
			cal->scheduleEvent( ( Event* )e );   // It's important to cast!!
			creature->setStateModEvent( mod, e );
		}
	}
}

void SpellCaster::openLocked() {
	Location *pos = battle->getSession()->getMap()->getLocation( battle->getCreature()->getTargetX(),
	                battle->getCreature()->getTargetY(),
	                battle->getCreature()->getTargetZ() );
	if ( pos ) {
		battle->getSession()->getGameAdapter()->useDoor( pos, true );
	}
}

void SpellCaster::windAttack() {

	int spellEffectSize = 4;
	float sx, sy;
	int radius = getRadius( spellEffectSize, &sx, &sy );

	// walk around the circle
	set<Creature*> seen;
	Creature *targets[100];
	int targetCount = 0;
	Creature *c = battle->getCreature()->getTargetCreature();
	DisplayInfo di;
	for ( int r = spellEffectSize; r; r += spellEffectSize ) {
		if ( r > radius ) r = radius;

		di.red = 0.1f;
		di.green = 1 - static_cast<float>( r ) / static_cast<float>( radius );
		di.blue = static_cast<float>( r ) / static_cast<float>( radius );

		for ( int angle = 0; angle < 360; angle += 10 ) {
			int x = toint( sx + ( static_cast<float>( r ) * Constants::cosFromAngle( static_cast<float>( angle ) ) ) );
			int y = toint( sy - ( static_cast<float>( r ) * Constants::sinFromAngle( static_cast<float>( angle ) ) ) );
			if ( x >= 0 && x < MAP_WIDTH && y >= 0 && y < MAP_DEPTH ) {
				//      Location *pos = battle->getSession()->getMap()->getLocation( x, y, 0 );
				battle->getSession()->getMap()->startEffect( x, y, 1, Constants::EFFECT_GREEN,
				    ( Constants::DAMAGE_DURATION * 4 ),
				    spellEffectSize, spellEffectSize,
				    ( GLuint )( r * 25 ), false, &di );
				targetCount = battle->getSession()->getMap()->getCreaturesInArea( x, y, spellEffectSize, ( RenderedCreature** )targets );
				for ( int i = 0; i < targetCount; i++ ) {
					if ( seen.find( targets[ i ] ) == seen.end() &&
					        battle->getCreature()->canAttack( targets[ i ] ) ) {
						seen.insert( targets[ i ] );
						battle->getCreature()->setTargetCreature( targets[ i ] );
						causeDamage( true );
						// knock the creature back
						int cx = toint( targets[ i ]->getX() );
						int cy = toint( targets[ i ]->getY() );
						int px = toint( cx + ( 2.0f * Constants::cosFromAngle( static_cast<float>( angle ) ) ) );
						int py = toint( cy - ( 2.0f * Constants::sinFromAngle( static_cast<float>( angle ) ) ) );
						if ( !( battle->getSession()->getMap()->
						        moveCreature( cx, cy, 0,
						                      px, py, 0,
						                      targets[ i ] ) ) ) {
							targets[ i ]->moveTo( px, py, 0 );
						}
					}
				}
			}
		}

		if ( r >= radius ) break;
	}
	battle->getCreature()->setTargetCreature( c );
}


void SpellCaster::circleAttack() {

	int spellEffectSize = 2;
	float sx, sy;
	int radius = getRadius( spellEffectSize, &sx, &sy );

	// walk around the circle
	Creature *targets[100];
	int targetCount = 0;
	Creature *c = battle->getCreature()->getTargetCreature();
	for ( int angle = 0; angle < 360; angle += 10 ) {
		int x = toint( sx + ( static_cast<float>( radius ) * Constants::cosFromAngle( static_cast<float>( angle ) ) ) );
		int y = toint( sy - ( static_cast<float>( radius ) * Constants::sinFromAngle( static_cast<float>( angle ) ) ) );
		if ( x >= 0 && x < MAP_WIDTH && y >= 0 && y < MAP_DEPTH ) {
//      Location *pos = battle->getSession()->getMap()->getLocation( x, y, 0 );
			battle->getSession()->getMap()->startEffect( x, y, 1, Constants::EFFECT_DUST,
			    ( Constants::DAMAGE_DURATION * 4 ),
			    spellEffectSize, spellEffectSize,
			    ( GLuint )( angle * 5 ) );
			targetCount = battle->getSession()->getMap()->getCreaturesInArea( x, y, spellEffectSize, ( RenderedCreature** )targets );
			for ( int i = 0; i < targetCount; i++ ) {
				if ( battle->getCreature()->canAttack( targets[ i ] ) ) {
					battle->getCreature()->setTargetCreature( targets[ i ] );
					causeDamage( true );
				}
			}
		}
	}
	battle->getCreature()->setTargetCreature( c );
}

void SpellCaster::hailAttack() {

	float sx, sy;
	int spellEffectSize = 2;
	int radius = getRadius( spellEffectSize, &sx, &sy );

	// pick random locations in the circle
	for ( int i = 0; i < radius * 2 + 2; i++ ) {
		int x = static_cast<int>( Util::roll( sx - radius, sx + radius ) );
		int y = static_cast<int>( Util::roll( sy - radius, sy + radius ) );

		if ( x >= 0 && x < MAP_WIDTH && y >= 0 && y < MAP_DEPTH ) {
			//      Location *pos = battle->getSession()->getMap()->getLocation( x, y, 0 );
			battle->getSession()->getMap()->startEffect( x, y, 1, Constants::EFFECT_HAIL,
			    ( Constants::DAMAGE_DURATION * 4 ),
			    1, 1,
			    ( GLuint )( i * 50 ) );
		}
	}

	// select targets
	Creature *targets[100];
	int targetCount = 0;
	Creature *c = battle->getCreature()->getTargetCreature();
	targetCount = battle->getSession()->getMap()->getCreaturesInArea( toint( battle->getCreature()->getX() ),
	              toint( battle->getCreature()->getY() ),
	              radius,
	              ( RenderedCreature** )targets );
	for ( int i = 0; i < targetCount; i++ ) {
		if ( battle->getCreature()->canAttack( targets[ i ] ) ) {
			battle->getCreature()->setTargetCreature( targets[ i ] );
			// FIXME: Spellcaster::causeDamage() must have first parameter of type bool not GLuint
			causeDamage( ( GLuint )( i * 50 ) );
		}
	}
	battle->getCreature()->setTargetCreature( c );
}

int SpellCaster::getRadius( int spellEffectSize, float *sx, float *sy ) {

	//cerr << "target x=" << battle->getCreature()->getTargetX() << "," << battle->getCreature()->getTargetY() << endl;

	int tw = 1;
	int td = 1;
	Location *pos = battle->getSession()->getMap()->getLocation( battle->getCreature()->getTargetX(),
	                battle->getCreature()->getTargetY(),
	                0 );
	if ( pos && pos->shape ) {
		tw = pos->shape->getWidth();
		td = pos->shape->getDepth();
	}

	int selectedRadius = static_cast<int>( Constants::distance( battle->getCreature()->getX(), battle->getCreature()->getY(),
	                     battle->getCreature()->getShape()->getWidth(),
	                     battle->getCreature()->getShape()->getDepth(),
	                     battle->getCreature()->getTargetX(), battle->getCreature()->getTargetY(),
	                     tw, td ) );
	selectedRadius += toint( battle->getCreature()->getShape()->getWidth() / 2.0f + tw / 2.0f + spellEffectSize );

	// cap the selected radius
	int radius = level * 2;
	if ( radius > 15 ) radius = 15;
	if ( radius < 2 ) radius = 2;

	//cerr << "selected radius=" << selectedRadius << " max radius=" << radius << endl;
	if ( selectedRadius < radius ) radius = selectedRadius;
	//cerr << "radius=" << radius << endl;

	*sx = battle->getCreature()->getX() + ( battle->getCreature()->getShape()->getWidth() / 2.0f );
	*sy = battle->getCreature()->getY() - ( battle->getCreature()->getShape()->getDepth() / 2.0f );

	// show radius effect
	battle->getSession()->getMap()->startEffect( toint( *sx ), toint( *sy ), 1,
	    Constants::EFFECT_RING, ( Constants::DAMAGE_DURATION * 4 ),
	    radius, radius );

	return radius;
}


