/***************************************************************************
                          sqcreature.cpp  -  description
                             -------------------
    begin                : Sat Oct 8 2005
    copyright            : (C) 2005 by Gabor Torok
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
#include "sqcreature.h"
#include "../events/statemodexpirationevent.h"
#include "../render/map.h"
#include "../session.h"
#include "../creature.h"
#include "../rpg/rpglib.h"

using namespace std;

const char *SqCreature::className = "Creature";
ScriptClassMemberDecl SqCreature::members[] = {
  { "void", "_typeof", SqCreature::_squirrel_typeof, 1, 0, "" },
  { "void", "constructor", SqCreature::_constructor, 0, 0, "" },
  
  { "string", "getName", SqCreature::_getName, 0, 0, "" },
	{ "string", "getMonsterType", SqCreature::_getMonsterType, 0, 0, "Return the creature's monster type or empty string if it's not a monster." },
  { "int", "getLevel", SqCreature::_getLevel, 0, 0, "" },
  { "int", "getExpOfNextLevel", SqCreature::_getExpOfNextLevel, 0, 0, "How many experience points are needed for the character to gain the next level." },
  { "int", "getExp", SqCreature::_getExp, 0, 0, "" },
  { "int", "getMoney", SqCreature::_getMoney, 0, 0, "" },
  { "int", "getHp", SqCreature::_getHp, 0, 0, "" },
  { "int", "getStartingHp", SqCreature::_getStartingHp, 0, 0, "The hit-points the character gains when leveling up." },
  { "int", "getMaxHp", SqCreature::_getMaxHp, 0, 0, "The total amount of hit-points for this character. (If completely healed.)" },
  { "int", "getMp", SqCreature::_getMp, 0, 0, "" },
  { "int", "getStartingMp", SqCreature::_getStartingMp, 0, 0, "The magic-points this character gains when leveling up." },
  { "int", "getMaxMp", SqCreature::_getMaxMp, 0, 0, "The total amount of magic-points for this character. (If completely restored.)" },
  { "int", "getThirst", SqCreature::_getThirst, 0, 0, "A value between 0-10 indicating how thristy the character is. 0-most, 10-least thirsty." },
  { "int", "getHunger", SqCreature::_getHunger, 0, 0, "A value between 0-10 indicating how hungry the character is. 0-most, 10-least hungry." },
  { "int", "getSkill", SqCreature::_getSkill, SQ_MATCHTYPEMASKSTRING, "xn", "The point value for the given skill index. See <a href=\"ScourgeGame.html\">ScourgeGame</a>.getSkillCount() and <a href=\"ScourgeGame.html\">ScourgeGame</a>.getSkillName()." },
  { "int", "getSkillByName", SqCreature::_getSkillByName, SQ_MATCHTYPEMASKSTRING, "xs", "Same as getSkill() but instead of an index, the skill is referenced by name. See <a href=\"ScourgeGame.html\">ScourgeGame</a>.getSkillCount() and <a href=\"ScourgeGame.html\">ScourgeGame</a>.getSkillName()." },
  { "int", "getStateMod", SqCreature::_getStateMod, SQ_MATCHTYPEMASKSTRING, "xn", "Returns a boolean value if the state-mod is in effect for this character. See <a href=\"ScourgeGame.html\">ScourgeGame</a>.getStateModCount() and <a href=\"ScourgeGame.html\">ScourgeGame</a>.getStateModName()." },
  { "int", "getProtectedStateMod", SqCreature::_getProtectedStateMod, SQ_MATCHTYPEMASKSTRING, "xn", "Returns a boolean value indicating if the character is protected from the given state mod. See <a href=\"ScourgeGame.html\">ScourgeGame</a>.getStateModCount() and <a href=\"ScourgeGame.html\">ScourgeGame</a>.getStateModName()." },
	{ "int", "getSex", SqCreature::_getSex, 0, 0, "Returns the creature's sex: 0-male, 1-female." },
	{ "bool", "hasCapability", SqCreature::_hasCapability, 0, 0, "Does this creature currently able to use this special capability?" },
  { "float", "getArmor", SqCreature::_getArmor, 0, 0, "Return the armor value (sum of armor items worn modified by skills.)" },  

  { "void", "setLevel", SqCreature::_setLevel, SQ_MATCHTYPEMASKSTRING, "xn", "" },
  { "void", "setExp", SqCreature::_setExp, SQ_MATCHTYPEMASKSTRING, "xn", "" },
  { "void", "setMoney", SqCreature::_setMoney, SQ_MATCHTYPEMASKSTRING, "xn", "" },
  { "void", "setHp", SqCreature::_setHp, SQ_MATCHTYPEMASKSTRING, "xn", "" },
  { "void", "takeDamage", SqCreature::_takeDamage, SQ_MATCHTYPEMASKSTRING, "xf", "" },
  { "void", "setMp", SqCreature::_setMp, SQ_MATCHTYPEMASKSTRING, "xn", "" },
  { "void", "setThirst", SqCreature::_setThirst, SQ_MATCHTYPEMASKSTRING, "xn", "" },
  { "void", "setHunger", SqCreature::_setHunger, SQ_MATCHTYPEMASKSTRING, "xn", "" },
  { "void", "setSkill", SqCreature::_setSkill, SQ_MATCHTYPEMASKSTRING, "xnn", "" },
  { "void", "setSkillByName", SqCreature::_setSkillByName, SQ_MATCHTYPEMASKSTRING, "xsn", "" },
  { "void", "setStateMod", SqCreature::_setStateMod, SQ_MATCHTYPEMASKSTRING, "xnb", "" },
  { "void", "setProtectedStateMod", SqCreature::_setProtectedStateMod, SQ_MATCHTYPEMASKSTRING, "xnb", "" },
  { "void", "setSex", SqCreature::_setSex, SQ_MATCHTYPEMASKSTRING, "xn", "Set the creature's sex: 0-male, 1-female." },	

  // character methods
  { "bool", "isOfClass", SqCreature::_isOfClass, SQ_MATCHTYPEMASKSTRING, "xs", "Returns a boolean if the character is of the character class given in the argument. This function is slow because it does a string compare on the class's name." },  
	{ "bool", "isOfRootClass", SqCreature::_isOfRootClass, SQ_MATCHTYPEMASKSTRING, "xs", "Returns a boolean if the character's root class is the argument. This function is slow because it does a string compare on the class's name." },  
  { "string", "getDeity", SqCreature::_getDeity, 0, 0, "Return the character's chosen deity's name." },
  { "Creature", "getTargetCreature", SqCreature::_getTargetCreature, 0, 0, "Return the creature's target creature of NULL if there isn't one." },
  { "Item", "getItemAtLocation", SqCreature::_getItemAtLocation, SQ_MATCHTYPEMASKSTRING, "xn", "Return the item currently equipped at the specified location. (location is left-hand, right-hand, etc.)" },
	{ "int", "getInventoryCount", SqCreature::_getInventoryCount, 0, 0, "Return the number of inventory items in this character's backpack." },
	{ "Item", "getInventoryItem", SqCreature::_getInventoryItem, SQ_MATCHTYPEMASKSTRING, "xn", "Return the inventory item at this index in this character's backpack." },

  { "void", "startConversation", SqCreature::_startConversation, 0, 0, "Start a conversation with this creature." },
	{ "void", "startConversationAbout", SqCreature::_startConversationAbout, 0, 0, "Start a conversation with this creature about a specific topic." },
	{ "void", "setIntro", SqCreature::_setIntro, 0, 0, "Set this NPC's intro text to the text referenced by this keyphrase in the <map>.txt file." },
	{ "void", "addInventoryByName", SqCreature::_addInventoryByName, 0, 0, "Add a new item of this name to the creature's inventory." },

	{ "bool", "isCharacter", SqCreature::_isCharacter, 0, 0, "Is this creature a pc?" },
	{ "bool", "isMonster", SqCreature::_isMonster, 0, 0, "Is this creature a monster?" },
	{ "bool", "isNpc", SqCreature::_isNpc, 0, 0, "Is this creature an npc?" },

  { 0,0,0,0,0 } // terminator
};
SquirrelClassDecl SqCreature::classDecl = { SqCreature::className, 0, members, 
  "Information about a scourge creature (monster, player or npc.)" };

SqCreature::SqCreature() {
}

SqCreature::~SqCreature() {
}

// ===========================================================================
// Static callback methods to ScourgeGame squirrel object member functions.
int SqCreature::_squirrel_typeof( HSQUIRRELVM vm ) {
  sq_pushstring( vm, SqCreature::className, -1 );
  return 1; // 1 value is returned
}

int SqCreature::_constructor( HSQUIRRELVM vm ) {
  cerr << "in " << SqCreature::className << " constructor." << endl;
  return 0; // no values returned
}

// ===========================================================================
// Member methods
int SqCreature::_getName( HSQUIRRELVM vm ) {
  GET_OBJECT(Creature*)
  sq_pushstring( vm, _SC( object->getName() ), -1 );
  return 1;
}

int SqCreature::_getMonsterType( HSQUIRRELVM vm ) {
  GET_OBJECT(Creature*)
	if( object->isMonster() ) {
		sq_pushstring( vm, _SC( object->getMonster()->getType() ), -1 );
	} else {
		sq_pushstring( vm, _SC( "" ), -1 );
	}
  return 1;
}

int SqCreature::_getLevel( HSQUIRRELVM vm ) {
  GET_OBJECT(Creature*)
  sq_pushinteger( vm, object->getLevel() );
  return 1;
}

int SqCreature::_getExpOfNextLevel( HSQUIRRELVM vm ) {
  GET_OBJECT(Creature*)
  sq_pushinteger( vm, object->getExpOfNextLevel() );
  return 1;
}

int SqCreature::_getExp( HSQUIRRELVM vm ) {
  GET_OBJECT(Creature*)
  sq_pushinteger( vm, object->getExp() );
  return 1;
}

int SqCreature::_getMoney( HSQUIRRELVM vm ) {
  GET_OBJECT(Creature*)
  sq_pushinteger( vm, object->getMoney() );
  return 1;
}

int SqCreature::_getHp( HSQUIRRELVM vm ) {
  GET_OBJECT(Creature*)
  sq_pushinteger( vm, object->getHp() );
  return 1;
}

int SqCreature::_getSex( HSQUIRRELVM vm ) {
  GET_OBJECT(Creature*)
  sq_pushinteger( vm, object->getSex() );
  return 1;
}

int SqCreature::_hasCapability( HSQUIRRELVM vm ) {
	GET_STRING( name, 80 )
	GET_OBJECT(Creature*)
	SQBool b = object->hasCapability( name );
  sq_pushbool( vm, b );
  return 1;
}

int SqCreature::_getStartingHp( HSQUIRRELVM vm ) {
  GET_OBJECT(Creature*)
  sq_pushinteger( vm, object->getStartingHp() );
  return 1;
}

int SqCreature::_getMaxHp( HSQUIRRELVM vm ) {
  GET_OBJECT(Creature*)
  sq_pushinteger( vm, object->getMaxHp() );
  return 1;
}

int SqCreature::_getMp( HSQUIRRELVM vm ) {
  GET_OBJECT(Creature*)
  sq_pushinteger( vm, object->getMp() );
  return 1;
}

int SqCreature::_getStartingMp( HSQUIRRELVM vm ) {
  GET_OBJECT(Creature*)
  sq_pushinteger( vm, object->getStartingMp() );
  return 1;
}

int SqCreature::_getMaxMp( HSQUIRRELVM vm ) {
  GET_OBJECT(Creature*)
  sq_pushinteger( vm, object->getMaxMp() );
  return 1;
}

int SqCreature::_getThirst( HSQUIRRELVM vm ) {
  GET_OBJECT(Creature*)
  sq_pushinteger( vm, object->getThirst() );
  return 1;
}

int SqCreature::_getHunger( HSQUIRRELVM vm ) {
  GET_OBJECT(Creature*)
  sq_pushinteger( vm, object->getHunger() );
  return 1;
}

int SqCreature::_getSkill( HSQUIRRELVM vm ) {
  GET_INT( index )
  GET_OBJECT( Creature* )
  sq_pushinteger( vm, object->getSkill( index ) );
  return 1;
}

int SqCreature::_getSkillByName( HSQUIRRELVM vm ) {
  GET_STRING( name, 80 )
  GET_OBJECT( Creature* )

	Skill *skill = Skill::getSkillByName( (char*)name );
	if( !skill ) {
		return sq_throwerror( vm, "Skill by name could not be found." );
	}
  sq_pushinteger( vm, object->getSkill( skill->getIndex() ) );
  return 1;
}

int SqCreature::_getStateMod( HSQUIRRELVM vm ) {
  GET_INT( index )
  GET_OBJECT(Creature*)
  sq_pushinteger( vm, object->getStateMod( index ) );
  return 1;
}

int SqCreature::_getProtectedStateMod( HSQUIRRELVM vm ) {
  GET_INT( index )
  GET_OBJECT(Creature*)
    cerr << "FIXME: getProtectedStateMod() need index." << endl;
  sq_pushinteger( vm, object->getProtectedStateMod( index ) );
  return 1;
}


int SqCreature::_getArmor( HSQUIRRELVM vm ) {
	GET_INT( damageType )
  GET_OBJECT(Creature*)
	float armor, dodgePenalty;
  sq_pushfloat( vm, object->getArmor( &armor, &dodgePenalty, damageType ) );
  return 1;
}

int SqCreature::_isOfClass( HSQUIRRELVM vm ) {
  GET_STRING( name, 80 )
  GET_OBJECT( Creature* )
	SQBool b = false;
	if( !object->getCharacter() ) {
		//return sq_throwerror( vm, "Creature is not a PC." );
	} else {
		b = ( !strcmp( object->getCharacter()->getName(), name ) );
	}
  sq_pushbool( vm, b );
  return 1;
}

int SqCreature::_isOfRootClass( HSQUIRRELVM vm ) {
  GET_STRING( name, 80 )
  GET_OBJECT( Creature* )
	Character *c = object->getCharacter();
	SQBool b = false;
	if( !c ) {
		//return sq_throwerror( vm, "Creature is not a PC." );
	} else {
		while( c->getParent() ) c = c->getParent();
		b = ( !strcmp( c->getName(), name ) );
	}
  sq_pushbool( vm, b );
  return 1;
}

int SqCreature::_setLevel( HSQUIRRELVM vm ) {
  GET_INT( n );
  GET_OBJECT( Creature* )
  object->setLevel( n );
  return 0;
}

int SqCreature::_setExp( HSQUIRRELVM vm ) {
  GET_INT( n );
  GET_OBJECT( Creature* )
  object->setExp( n );
  return 0;
}

int SqCreature::_setMoney( HSQUIRRELVM vm ) {
  GET_INT( n );
  GET_OBJECT( Creature* )
  object->setMoney( n );
  return 0;
}

int SqCreature::_setHp( HSQUIRRELVM vm ) {
  GET_INT( n );
  GET_OBJECT( Creature* )

  // show an effect if there is a change
  if( n > object->getHp() ) {
    object->startEffect( Constants::EFFECT_SWIRL, ( Constants::DAMAGE_DURATION * 4 ) );
  } else if( n < object->getHp() ) {
    object->startEffect( Constants::EFFECT_GLOW, ( Constants::DAMAGE_DURATION * 4 ) );
  }
  object->setHp( n );
  return 0;
}

int SqCreature::_takeDamage( HSQUIRRELVM vm ) {
  GET_FLOAT( n );
  GET_OBJECT( Creature* )
  object->takeDamage( n );
  return 0;
}

int SqCreature::_setMp( HSQUIRRELVM vm ) {
  GET_INT( n );
  GET_OBJECT( Creature* )
  object->setMp( n );
  return 0;
}

int SqCreature::_setSex( HSQUIRRELVM vm ) {
  GET_INT( n );
  GET_OBJECT( Creature* )
  object->setSex( n );
  return 0;
}

int SqCreature::_setThirst( HSQUIRRELVM vm ) {
  GET_INT( n );
  GET_OBJECT( Creature* )
  object->setThirst( n );
  return 0;
}

int SqCreature::_setHunger( HSQUIRRELVM vm ) {
  GET_INT( n );
  GET_OBJECT( Creature* )
  object->setHunger( n );
  return 0;
}

int SqCreature::_setSkill( HSQUIRRELVM vm ) {
  GET_INT( index );
  GET_INT( n );
  GET_OBJECT( Creature* )
  object->setSkill( index, n );
  return 0;
}

int SqCreature::_setSkillByName( HSQUIRRELVM vm ) {
  GET_STRING( name, 80 );
  GET_INT( n );
  GET_OBJECT( Creature* )
	Skill *skill = Skill::getSkillByName( (char*)name );
	if( !skill ) {
		return sq_throwerror( vm, "Skill by name could not be found." );
	}
  object->setSkill( skill->getIndex(), n );
  return 0;
}

int SqCreature::_setStateMod( HSQUIRRELVM vm ) {
  GET_BOOL( setting )
  GET_INT( mod )
  GET_OBJECT( Creature* )

  cerr << "Setting: mod=" << StateMod::stateMods[ mod ]->getDisplayName() << " to " << setting << endl;

  char msg[200];
  bool protectiveItem = false;
  if( !StateMod::stateMods[ mod ]->isStateModTransitionWanted( setting ) ) {
  //if( !Constants::isStateModTransitionWanted( mod, setting ) ) {
    // check for magic item state mod protections
    protectiveItem = object->getProtectedStateMod(mod);
    if(protectiveItem && 0 == Util::dice( 2 )) {
      sprintf(msg, _( "%s resists the spell with magic item!" ), 
              object->getName());
      SqBinding::sessionRef->getGameAdapter()->addDescription(msg, 1, 0.15f, 1);    
      return 0;
    }
  }

  int timeInMin = 5 * object->getLevel();
  if(protectiveItem) timeInMin /= 2;
  object->startEffect( Constants::EFFECT_SWIRL, (Constants::DAMAGE_DURATION * 4) );

  // extend expiration event somehow if condition already exists
  Event *e = object->getStateModEvent(mod);
  if( object->getStateMod(mod) == setting ) {
    if( e ) {
      //        cerr << "Extending existing event." << endl;
      e->setNbExecutionsToDo( timeInMin );
    }
    return 0;
  }
  object->setStateMod( mod, setting );
    
  if(setting) {
    sprintf( msg, StateMod::stateMods[ mod ]->getSetState(), object->getName() );
  } else {
    sprintf( msg, StateMod::stateMods[ mod ]->getUnsetState(), object->getName() );
  }
  SqBinding::sessionRef->getGameAdapter()->addDescription(msg, 1, 0.15f, 1);
  
  // cancel existing event if any
  if( e ) {
//      cerr << "Cancelling existing event." << endl;
    SqBinding::sessionRef->getParty()->getCalendar()->cancelEvent( e );
  }

  if( setting ) {
    // add calendar event to remove condition            
    // (format : sec, min, hours, days, months, years)
    Calendar *cal = SqBinding::sessionRef->getParty()->getCalendar();
    //    cerr << Constants::STATE_NAMES[mod] << " will expire in " << timeInMin << " minutes." << endl;
    Date d( 0, 15, 0, 0, 0, 0 ); 
//      cerr << "Creating new event." << endl;
    e = new StateModExpirationEvent( cal->getCurrentDate(), d, object, mod, SqBinding::sessionRef, timeInMin);
    cal->scheduleEvent( (Event*)e );   // It's important to cast!!
    object->setStateModEvent( mod, e );
  }
  return 0;
}

int SqCreature::_setProtectedStateMod( HSQUIRRELVM vm ) {
  GET_BOOL( b )
  GET_INT( index )
  GET_OBJECT( Creature* )
  object->setProtectedStateMod( index, b );
  return 0;
}

int SqCreature::_getDeity( HSQUIRRELVM vm ) {
  GET_OBJECT(Creature*)

  int di = object->getDeityIndex();
  if( di < 0 ) {
    return sq_throwerror( vm, "Creature has no deity." );
  }
  sq_pushstring( vm, _SC( MagicSchool::getMagicSchool( di )->getDeity() ), -1 );
  return 1;
}

int SqCreature::_startConversation( HSQUIRRELVM vm ) {
  GET_OBJECT(Creature*)
  SqBinding::sessionRef->getGameAdapter()->startConversation( object );
  return 0;
}

int SqCreature::_startConversationAbout( HSQUIRRELVM vm ) {
	GET_STRING( message, 120 )
  GET_OBJECT(Creature*)
  SqBinding::sessionRef->getGameAdapter()->startConversation( object, message );
  return 0;
}

int SqCreature::_getTargetCreature( HSQUIRRELVM vm ) {
  GET_OBJECT(Creature*)
  if( object->getTargetCreature() ) {
    if( object->getTargetCreature()->isMonster() ) {
      sq_pushobject( vm, *(SqBinding::binding->creatureMap[object->getTargetCreature()]) );
    } else {
      bool found = false;
      for( int i = 0; i < SqBinding::sessionRef->getParty()->getPartySize(); i++ ) {
        if( object->getTargetCreature() == SqBinding::sessionRef->getParty()->getParty( i ) ) {
          sq_pushobject( vm, SqBinding::binding->refParty[ i ] );
          found = true;
          break;
        }
      }
      if( !found ) {
        cerr << "SqCreature::_getTargetCreature did not find party member: " << object->getTargetCreature()->getName() << endl;
        sq_pushnull( vm );
      }
    }
  } else {
    sq_pushnull( vm );
  }
  return 1;
}

int SqCreature::_getItemAtLocation( HSQUIRRELVM vm ) {
  GET_INT( location )
  GET_OBJECT(Creature*)
  Item *item = object->getItemAtLocation( location );
  if( item ) {
    sq_pushobject( vm, *(SqBinding::binding->itemMap[ item ]) );
  } else {
    sq_pushnull( vm );
  }
  return 1;
}

int SqCreature::_getInventoryItem( HSQUIRRELVM vm ) {
	GET_INT( index )
	GET_OBJECT(Creature*)
	Item *item = object->getInventory( index );
  if( item ) {
    sq_pushobject( vm, *(SqBinding::binding->itemMap[ item ]) );
  } else {
    sq_pushnull( vm );
  }
  return 1;
}

int SqCreature::_getInventoryCount( HSQUIRRELVM vm ) {
	GET_OBJECT(Creature*)
	sq_pushinteger( vm, object->getInventoryCount() );
  return 1;
}

int SqCreature::_setIntro( HSQUIRRELVM vm ) {
	GET_STRING( keyphrase, 80 )
  GET_OBJECT( Creature* )
	if( !Mission::setIntro( object, keyphrase ) ) {
		return sq_throwerror( vm, "Error trying to set intro text." );
	}
	return 0;
}

int SqCreature::_addInventoryByName( HSQUIRRELVM vm ) {
	GET_STRING( itemName, 255 )
  GET_OBJECT( Creature* )

	RpgItem *rpgItem = RpgItem::getItemByName( itemName );
	if( !rpgItem ) {
		cerr << "*** Can't find RpgItem for name=" << itemName << endl;
		return sq_throwerror( vm, "Can't find RpgItem by this name." );
	}
	object->addInventory( SqBinding::sessionRef->newItem( rpgItem ), true );
	return 0;
}

int SqCreature::_isCharacter( HSQUIRRELVM vm ) {
	GET_OBJECT( Creature* )
	SQBool b = ( object->isMonster() ? false : true );
	sq_pushbool( vm, b );
	return 1;
}

int SqCreature::_isMonster( HSQUIRRELVM vm ) {
	GET_OBJECT( Creature* )
	SQBool b = ( object->isMonster() ? true : false );
	sq_pushbool( vm, b );
	return 1;
}

int SqCreature::_isNpc( HSQUIRRELVM vm ) {
	GET_OBJECT( Creature* )
	SQBool b = ( object->isNpc() ? true : false );
	sq_pushbool( vm, b );
	return 1;
}

