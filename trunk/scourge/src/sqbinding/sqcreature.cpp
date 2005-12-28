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
  { "int", "getSkillPercent", SqCreature::_getSkillPercent, SQ_MATCHTYPEMASKSTRING, "xn", "A 0-100 percentage value for the given skill index. See <a href=\"ScourgeGame.html\">ScourgeGame</a>.getSkillCount() and <a href=\"ScourgeGame.html\">ScourgeGame</a>.getSkillName()." },
  { "int", "getSkillByNamePercent", SqCreature::_getSkillByNamePercent, SQ_MATCHTYPEMASKSTRING, "xs", "Same as getSkill() but instead of an index, the skill is referenced by name. See <a href=\"ScourgeGame.html\">ScourgeGame</a>.getSkillCount() and <a href=\"ScourgeGame.html\">ScourgeGame</a>.getSkillName()." },
  { "int", "getStateMod", SqCreature::_getStateMod, SQ_MATCHTYPEMASKSTRING, "xn", "Returns a boolean value if the state-mod is in effect for this character. See <a href=\"ScourgeGame.html\">ScourgeGame</a>.getStateModCount() and <a href=\"ScourgeGame.html\">ScourgeGame</a>.getStateModName()." },
  { "int", "getProtectedStateMod", SqCreature::_getProtectedStateMod, SQ_MATCHTYPEMASKSTRING, "xn", "Returns a boolean value indicating if the character is protected from the given state mod. See <a href=\"ScourgeGame.html\">ScourgeGame</a>.getStateModCount() and <a href=\"ScourgeGame.html\">ScourgeGame</a>.getStateModName()." },
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

  // character methods
  { "bool", "isOfClass", SqCreature::_isOfClass, SQ_MATCHTYPEMASKSTRING, "xs", "Returns a boolean if the character is of the character class given in the argument. This function is slow because it does a string compare on the class's name." },  
  { "string", "getDeity", SqCreature::_getDeity, 0, 0, "Return the character's chosen deity's name." },

  { "void", "startConversation", SqCreature::_startConversation, 0, 0, "Start a conversation with this creature." },

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

int SqCreature::_getLevel( HSQUIRRELVM vm ) {
  GET_OBJECT(Creature*)
  sq_pushinteger( vm, _SC( object->getLevel() ) );
  return 1;
}

int SqCreature::_getExpOfNextLevel( HSQUIRRELVM vm ) {
  GET_OBJECT(Creature*)
  sq_pushinteger( vm, _SC( object->getExpOfNextLevel() ) );
  return 1;
}

int SqCreature::_getExp( HSQUIRRELVM vm ) {
  GET_OBJECT(Creature*)
  sq_pushinteger( vm, _SC( object->getExp() ) );
  return 1;
}

int SqCreature::_getMoney( HSQUIRRELVM vm ) {
  GET_OBJECT(Creature*)
  sq_pushinteger( vm, _SC( object->getMoney() ) );
  return 1;
}

int SqCreature::_getHp( HSQUIRRELVM vm ) {
  GET_OBJECT(Creature*)
  sq_pushinteger( vm, _SC( object->getHp() ) );
  return 1;
}

int SqCreature::_getStartingHp( HSQUIRRELVM vm ) {
  GET_OBJECT(Creature*)
  sq_pushinteger( vm, _SC( object->getStartingHp() ) );
  return 1;
}

int SqCreature::_getMaxHp( HSQUIRRELVM vm ) {
  GET_OBJECT(Creature*)
  sq_pushinteger( vm, _SC( object->getMaxHp() ) );
  return 1;
}

int SqCreature::_getMp( HSQUIRRELVM vm ) {
  GET_OBJECT(Creature*)
  sq_pushinteger( vm, _SC( object->getMp() ) );
  return 1;
}

int SqCreature::_getStartingMp( HSQUIRRELVM vm ) {
  GET_OBJECT(Creature*)
  sq_pushinteger( vm, _SC( object->getStartingMp() ) );
  return 1;
}

int SqCreature::_getMaxMp( HSQUIRRELVM vm ) {
  GET_OBJECT(Creature*)
  sq_pushinteger( vm, _SC( object->getMaxMp() ) );
  return 1;
}

int SqCreature::_getThirst( HSQUIRRELVM vm ) {
  GET_OBJECT(Creature*)
  sq_pushinteger( vm, _SC( object->getThirst() ) );
  return 1;
}

int SqCreature::_getHunger( HSQUIRRELVM vm ) {
  GET_OBJECT(Creature*)
  sq_pushinteger( vm, _SC( object->getHunger() ) );
  return 1;
}

int SqCreature::_getSkill( HSQUIRRELVM vm ) {
  GET_INT( index )
  GET_OBJECT( Creature* )
  sq_pushinteger( vm, _SC( object->getSkill( index ) ) );
  return 1;
}

int SqCreature::_getSkillByName( HSQUIRRELVM vm ) {
  GET_STRING( name, 80 )
  GET_OBJECT( Creature* )
  sq_pushinteger( vm, _SC( object->getSkill( Constants::getSkillByName( (char*)name ) ) ) );
  return 1;
}

int SqCreature::_getSkillPercent( HSQUIRRELVM vm ) {
  GET_INT( index )
  GET_OBJECT( Creature* )
  sq_pushinteger( vm, _SC( object->getLevelAdjustedSkill( index ) ) );
  return 1;
}

int SqCreature::_getSkillByNamePercent( HSQUIRRELVM vm ) {
  GET_STRING( name, 80 )
  GET_OBJECT( Creature* )
  sq_pushinteger( vm, _SC( object->getLevelAdjustedSkill( Constants::getSkillByName( (char*)name ) ) ) );
  return 1;
}

int SqCreature::_getStateMod( HSQUIRRELVM vm ) {
  GET_INT( index )
  GET_OBJECT(Creature*)
  sq_pushinteger( vm, _SC( object->getStateMod( index ) ) );
  return 1;
}

int SqCreature::_getProtectedStateMod( HSQUIRRELVM vm ) {
  GET_INT( index )
  GET_OBJECT(Creature*)
    cerr << "FIXME: getProtectedStateMod() need index." << endl;
  sq_pushinteger( vm, _SC( object->getProtectedStateMod( index ) ) );
  return 1;
}


int SqCreature::_getArmor( HSQUIRRELVM vm ) {
  GET_OBJECT(Creature*)
  sq_pushfloat( vm, _SC( object->getACPercent() ) );
  return 1;
}

int SqCreature::_isOfClass( HSQUIRRELVM vm ) {
  GET_STRING( name, 80 )
  GET_OBJECT( Creature* )
  SQBool b = ( object->getCharacter() && 
               !strcmp( object->getCharacter()->getName(), name ) );
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
  object->setSkill( Constants::getSkillByName( (char*)name ), n );
  return 0;
}

int SqCreature::_setStateMod( HSQUIRRELVM vm ) {
  GET_BOOL( setting )
  GET_INT( mod )
  GET_OBJECT( Creature* )

  cerr << "Setting: mod=" << Constants::STATE_NAMES[ mod ] << " to " << setting << endl;

  char msg[200];
  bool protectiveItem = false;
  if( !Constants::isStateModTransitionWanted( mod, setting ) ) {
    // check for magic item state mod protections
    protectiveItem = object->getProtectedStateMod(mod);
    if(protectiveItem && 0 == (int)(2.0f * rand()/RAND_MAX)) {
      sprintf(msg, "%s resists the spell with magic item!", 
              object->getName());
      SqBinding::sessionRef->getMap()->addDescription(msg, 1, 0.15f, 1);    
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
    sprintf( msg, "%s is %s.", object->getName(), Constants::STATE_NAMES[ mod ] );
  } else {
    sprintf(msg, "%s is not %s any more.", object->getName(), Constants::STATE_NAMES[ mod ] );
  }
  SqBinding::sessionRef->getMap()->addDescription(msg, 1, 0.15f, 1);
  
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
    Date d( 0, 1, 0, 0, 0, 0 ); 
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
