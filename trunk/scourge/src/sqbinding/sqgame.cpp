/***************************************************************************
                          sqgame.cpp  -  description
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
#include "sqgame.h"
#include "../session.h"
#include "../creature.h"
#include "../date.h"
#include "../render/renderlib.h"
#include "../test/combattest.h"

const char *SqGame::className = "ScourgeGame";
ScriptClassMemberDecl SqGame::members[] = {
  { "void", "_typeof", SqGame::_squirrel_typeof, 1, 0, "" },
  { "void", "constructor", SqGame::_constructor, 0, 0, "" },
  { "string", "getVersion", SqGame::_getVersion, 0, 0, "Get the game's version." },
  { "string", "getRootDir", SqGame::_getRootDir, 0, 0, "Get the game's data directory." },
  { "int", "getPartySize", SqGame::_getPartySize, 0, 0, "Get the number of party members." },
  { "Creature", "getPartyMember", SqGame::_getPartyMember, 0, 0, "Get one of the party member's creature objects. The first param is the index of the party member." },
  { "int", "getSkillCount", SqGame::_getSkillCount, 0, 0, "Get the number of skills in the game." },
  { "string", "getSkillName", SqGame::_getSkillName, 0, 0, "Get the given skill's name. The first param is the index of the skill." },
  { "Mission", "getMission", SqGame::_getMission, 0, 0, "Get the current mission object." },
  { "int", "getStateModCount", SqGame::_getStateModCount, 0, 0, "Return the number of state modifiers in the game." },
  { "string", "getStateModName", SqGame::_getStateModName, 2, "xn", "Get the given state mod's name. The first param is the index of the state mod." },
  { "string", "getDateString", SqGame::_getDateString, 0, 0, "Get the current game date. It is returned in the game's date format: (yyyy/m/d/h/m/s)" },
  { "bool", "isADayLater", SqGame::_isADayLater, 2, "xs", "Is the given date a day later than the current game date? The first parameter is a date in game date format. (yyyy/m/d/h/m/s)" },
  { "string", "getValue", SqGame::_getValue, 2, "xs", "Get the value associated with a given key from the value map. The first parameter is the key." },
  { "void", "setValue", SqGame::_setValue, 3, "xss", "Add a new or set an existing key and its value in the value map. The first parameter is the key, the second is its value." },
  { "void", "eraseValue", SqGame::_eraseValue, 2, "xs", "Remove a key and its value from the value map. The first parameter is the key to be removed." },
  { "void", "printMessage", SqGame::_printMessage, 2, "xs", "Print a message in the scourge message window. The resulting message will always be displayed in a lovely shade of purple." },
  { "void", "reloadNuts", SqGame::_reloadNuts, 0, 0, "Reload all currently used squirrel (.nut) files. The game engine will also do this for you automatically every 5 game minutes." },
  { "void", "documentSOM", SqGame::_documentSOM, 2, "xs", "Produce this documentation. The first argument is the location where the html files will be placed." },
  { "void", "runTests", SqGame::_runTests, 2, "xs", "Run internal tests of the rpg combat engine. Results are saved in path given as param to runTests()." },
  { 0,0,0,0,0 } // terminator
};
SquirrelClassDecl SqGame::classDecl = { SqGame::className, 0, members, 
  "The root of the SOM. At the start of the game, a global variable named scourgeGame\
  is created. All other scourge classes are referenced from this object." };

SqGame::SqGame() {
}

SqGame::~SqGame() {
}

// ===========================================================================
// Static callback methods to ScourgeGame squirrel object member functions.
int SqGame::_squirrel_typeof( HSQUIRRELVM vm ) {
  sq_pushstring( vm, SqGame::className, -1 );
  return 1; // 1 value is returned
}

int SqGame::_constructor( HSQUIRRELVM vm ) {
  return 0; // no values returned
}

int SqGame::_getVersion( HSQUIRRELVM vm ) {
  sq_pushstring( vm, _SC( SCOURGE_VERSION ), -1 );
  return 1;
}

int SqGame::_getRootDir( HSQUIRRELVM vm ) {
  sq_pushstring( vm, _SC( rootDir ), -1 );
  return 1;
}

int SqGame::_getPartySize( HSQUIRRELVM vm ) {
  sq_pushinteger( vm, SqBinding::sessionRef->getParty()->getPartySize() );
  return 1;
}

int SqGame::_getPartyMember( HSQUIRRELVM vm ) {
  int partyIndex;
  if( SQ_FAILED( sq_getinteger( vm, 2, &partyIndex ) ) ) {
    return sq_throwerror( vm, _SC( "Can't get party index in _getPartyMember." ) );
  }
  if( partyIndex < 0 || partyIndex > SqBinding::sessionRef->getParty()->getPartySize() ) {
    return sq_throwerror( vm, _SC( "Party index is out of range." ) );
  }

  sq_pushobject( vm, SqBinding::binding->refParty[ partyIndex ] );
  return 1;
}

int SqGame::_getMission( HSQUIRRELVM vm ) {
  sq_pushobject( vm, SqBinding::binding->refMission );
  return 1;
}

int SqGame::_getSkillCount( HSQUIRRELVM vm ) {
  sq_pushinteger( vm, Constants::SKILL_COUNT );
  return 1;
}

int SqGame::_getSkillName( HSQUIRRELVM vm ) {
  int index;
  if( SQ_FAILED( sq_getinteger( vm, 2, &index ) ) ) {
    return sq_throwerror( vm, _SC( "Can't get index in getSkillName." ) );
  }
  if( index < 0 || index >= Constants::SKILL_COUNT  ) {
    return sq_throwerror( vm, _SC( "Party index is out of range." ) );
  }

  sq_pushstring( vm, _SC( Constants::SKILL_NAMES[ index ] ), -1 );
  return 1;
}

int SqGame::_getStateModCount( HSQUIRRELVM vm ) {
  sq_pushinteger( vm, Constants::STATE_MOD_COUNT );
  return 1;
}

int SqGame::_getStateModName( HSQUIRRELVM vm ) {
  int index;
  if( SQ_FAILED( sq_getinteger( vm, 2, &index ) ) ) {
    return sq_throwerror( vm, _SC( "Can't get index in getSkillName." ) );
  }
  if( index < 0 || index >= Constants::STATE_MOD_COUNT  ) {
    return sq_throwerror( vm, _SC( "Party index is out of range." ) );
  }

  sq_pushstring( vm, _SC( Constants::STATE_NAMES[ index ] ), -1 );
  return 1;
}

int SqGame::_getDateString( HSQUIRRELVM vm ) {
  sq_pushstring( vm, _SC( SqBinding::sessionRef->getParty()->
                          getCalendar()->getCurrentDate().
                          getShortString() ), 
                 -1 );
  return 1;
}

int SqGame::_isADayLater( HSQUIRRELVM vm ) {
  GET_STRING( dateShortString, 80 )
  Date *d = new Date( dateShortString );
  sq_pushbool( vm, ( SqBinding::sessionRef->getParty()->
                     getCalendar()->getCurrentDate().
                     isADayLater( *d ) ? 1 : 0 ) );
  delete d;
  return 1;
}

int SqGame::_getValue( HSQUIRRELVM vm ) {
  GET_STRING( key, 80 )
  sq_pushstring( vm, _SC( SqBinding::binding->getValue( key ) ), -1 );
  return 1;
}

int SqGame::_setValue( HSQUIRRELVM vm ) {
  GET_STRING( value, 80 );
  GET_STRING( key, 80 )
  SqBinding::binding->setValue( key, value );
  return 0;
}

int SqGame::_eraseValue( HSQUIRRELVM vm ) {
  GET_STRING( key, 80 )
  SqBinding::binding->eraseValue( key );
  return 0;
}

int SqGame::_printMessage( HSQUIRRELVM vm ) {
  GET_STRING( message, 80 )
  SqBinding::sessionRef->getMap()->addDescription( message, 1, 0, 1 );
  return 0;
}

int SqGame::_reloadNuts( HSQUIRRELVM vm ) {
  SqBinding::binding->reloadScripts();
  return 0;
}

int SqGame::_documentSOM( HSQUIRRELVM vm ) {
  GET_STRING( path, 255 )
  if( !strlen( path ) ) strcpy( path, "/home/gabor/sourceforge/scourge/api/som" );
  SqBinding::binding->documentSOM( path );
  return 0;
}

int SqGame::_runTests( HSQUIRRELVM vm ) {
  GET_STRING( path, 255 )
  if( !strlen( path ) ) strcpy( path, "/home/gabor/sourceforge/scourge/api/tests" );
  CombatTest::executeTests( SqBinding::sessionRef, path );
  return 0;
}

