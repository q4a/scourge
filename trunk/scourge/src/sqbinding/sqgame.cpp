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
#include "../rpg/rpg.h"
#include "../session.h"
#include "../creature.h"
#include "../date.h"
#include "../render/renderlib.h"
#include "../test/combattest.h"

using namespace std;

const char *SqGame::className = "ScourgeGame";
ScriptClassMemberDecl SqGame::members[] = {
  { "void", "_typeof", SqGame::_squirrel_typeof, 1, 0, "" },
  { "void", "constructor", SqGame::_constructor, 0, 0, "" },
  { "string", "getVersion", SqGame::_getVersion, 0, 0, "Get the game's version." },
  { "string", "getRootDir", SqGame::_getRootDir, 0, 0, "Get the game's data directory." },
  { "int", "getPartySize", SqGame::_getPartySize, 0, 0, "Get the number of party members." },
  { "Creature", "getPartyMember", SqGame::_getPartyMember, 0, 0, "Get one of the party member's creature objects. The first param is the index of the party member." },
  { "Creature", "getPlayer", SqGame::_getPlayer, 0, 0, "Get the currently selected character." },
  { "int", "getSkillCount", SqGame::_getSkillCount, 0, 0, "Get the number of skills in the game." },
  { "string", "getSkillName", SqGame::_getSkillName, 0, 0, "Get the given skill's name. The first param is the index of the skill." },
  { "string", "getSkillDisplayName", SqGame::_getSkillDisplayName, 0, 0, "Get the given skill's name. The first param is the index of the skill." },
  { "Mission", "getMission", SqGame::_getMission, 0, 0, "Get the current mission object." },
  { "int", "getStateModCount", SqGame::_getStateModCount, 0, 0, "Return the number of state modifiers in the game." },
  { "string", "getStateModName", SqGame::_getStateModName, 2, "xn", "Get the given state mod's name. The first param is the index of the state mod." },
  { "string", "getStateModDisplayName", SqGame::_getStateModDisplayName, 2, "xn", "Get the given state mod's name. The first param is the index of the state mod." },
  { "int", "getStateModByName", SqGame::_getStateModByName, 2, "xs", "Get the index of the state given in the param to this function." },
  { "string", "getDateString", SqGame::_getDateString, 0, 0, "Get the current game date. It is returned in the game's date format: (yyyy/m/d/h/m/s)" },
  { "bool", "isADayLater", SqGame::_isADayLater, 2, "xs", "Is the given date a day later than the current game date? The first parameter is a date in game date format. (yyyy/m/d/h/m/s)" },
  { "string", "getValue", SqGame::_getValue, 2, "xs", "Get the value associated with a given key from the value map. The first parameter is the key." },
  { "void", "setValue", SqGame::_setValue, 3, "xss", "Add a new or set an existing key and its value in the value map. The first parameter is the key, the second is its value." },
  { "void", "eraseValue", SqGame::_eraseValue, 2, "xs", "Remove a key and its value from the value map. The first parameter is the key to be removed." },
  { "void", "printMessage", SqGame::_printMessage, 2, "xs", "Print a message in the scourge message window. The resulting message will always be displayed in a lovely shade of purple." },
  { "void", "reloadNuts", SqGame::_reloadNuts, 0, 0, "Reload all currently used squirrel (.nut) files. The game engine will also do this for you automatically every 5 game minutes." },
  { "void", "documentSOM", SqGame::_documentSOM, 2, "xs", "Produce this documentation. The first argument is the location where the html files will be placed." },
  { "void", "runTests", SqGame::_runTests, 2, "xs", "Run internal tests of the rpg combat engine. Results are saved in path given as param to runTests()." },
  { "void", "showTextMessage", SqGame::_showTextMessage, 2, "xs", "show a scrollable text message dialog." },
  { "string", "getDeityLocation", SqGame::_getDeityLocation, 4, "xnnn", "Get the deity whose presense is bound to this location (like an altar). Results the name of the deity." },    
	{ "void", "endConversation", SqGame::_endConversation, 0, 0, "Close the conversation dialog." },
	{ "string", "getTranslatedString", SqGame::_getTranslatedString, 0, 0, "Get the translated version of this string. Calls GNU gettext." },
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

int SqGame::_getPlayer( HSQUIRRELVM vm ) {
  for( int i = 0; i < SqBinding::sessionRef->getParty()->getPartySize(); i++ ) {
    if( SqBinding::sessionRef->getParty()->getParty(i) == 
        SqBinding::sessionRef->getParty()->getPlayer() ) {
      sq_pushobject( vm, SqBinding::binding->refParty[ i ] );
      return 1;
    }
  }
  return sq_throwerror( vm, _SC( "Player is not set." ) );
}

int SqGame::_getMission( HSQUIRRELVM vm ) {
  sq_pushobject( vm, SqBinding::binding->refMission );
  return 1;
}

int SqGame::_getSkillCount( HSQUIRRELVM vm ) {
  sq_pushinteger( vm, Skill::SKILL_COUNT );
  return 1;
}

int SqGame::_getSkillName( HSQUIRRELVM vm ) {
  int index;
  if( SQ_FAILED( sq_getinteger( vm, 2, &index ) ) ) {
    return sq_throwerror( vm, _SC( "Can't get index in getSkillName." ) );
  }
  if( index < 0 || index >= Skill::SKILL_COUNT  ) {
    return sq_throwerror( vm, _SC( "Party index is out of range." ) );
  }

  sq_pushstring( vm, _SC( Skill::skills[ index ]->getName() ), -1 );
  return 1;
}

int SqGame::_getSkillDisplayName( HSQUIRRELVM vm ) {
  int index;
  if( SQ_FAILED( sq_getinteger( vm, 2, &index ) ) ) {
    return sq_throwerror( vm, _SC( "Can't get index in getSkillName." ) );
  }
  if( index < 0 || index >= Skill::SKILL_COUNT  ) {
    return sq_throwerror( vm, _SC( "Party index is out of range." ) );
  }

  sq_pushstring( vm, _SC( Skill::skills[ index ]->getDisplayName() ), -1 );
  return 1;
}

int SqGame::_getStateModCount( HSQUIRRELVM vm ) {
  sq_pushinteger( vm, StateMod::STATE_MOD_COUNT );
  return 1;
}

int SqGame::_getStateModName( HSQUIRRELVM vm ) {
  int index;
  if( SQ_FAILED( sq_getinteger( vm, 2, &index ) ) ) {
    return sq_throwerror( vm, _SC( "Can't get index in getSkillName." ) );
  }
  if( index < 0 || index >= StateMod::STATE_MOD_COUNT  ) {
    return sq_throwerror( vm, _SC( "Party index is out of range." ) );
  }

  sq_pushstring( vm, _SC( StateMod::stateMods[ index ]->getName() ), -1 );
  return 1;
}

int SqGame::_getStateModDisplayName( HSQUIRRELVM vm ) {
  int index;
  if( SQ_FAILED( sq_getinteger( vm, 2, &index ) ) ) {
    return sq_throwerror( vm, _SC( "Can't get index in getSkillName." ) );
  }
  if( index < 0 || index >= StateMod::STATE_MOD_COUNT  ) {
    return sq_throwerror( vm, _SC( "Party index is out of range." ) );
  }

  sq_pushstring( vm, _SC( StateMod::stateMods[ index ]->getDisplayName() ), -1 );
  return 1;
}

int SqGame::_getStateModByName( HSQUIRRELVM vm ) {
  GET_STRING( stateModName, 40 );
  int n = StateMod::getStateModByName( stateModName )->getIndex();
  if( n < 0 ) return sq_throwerror( vm, _SC( "No state mod by that name." ) );
  sq_pushinteger( vm, n );
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
  if( !strlen( path ) ) strcpy( path, "/home/gabor/sourceforge/scourge/som" );
  SqBinding::binding->documentSOM( path );
  return 0;
}

int SqGame::_runTests( HSQUIRRELVM vm ) {
  GET_STRING( path, 255 )
  if( !strlen( path ) ) strcpy( path, "/home/gabor/sourceforge/scourge/api/tests" );
  CombatTest::executeTests( SqBinding::sessionRef, path );
  return 0;
}

int SqGame::_showTextMessage( HSQUIRRELVM vm ) {
  GET_STRING( message, 3000 )
  SqBinding::sessionRef->getGameAdapter()->showTextMessage( message );
  return 0;
}

int SqGame::_getDeityLocation( HSQUIRRELVM vm ) {
  GET_INT(z)
  GET_INT(y)
  GET_INT(x)

  cerr << "SqGame::_getDeityLocation, x=" << x << " y=" << y << " z=" << z << endl;

  Location *pos = SqBinding::sessionRef->getMap()->getLocation( x, y, z );
  if( pos ) {
    char *deity = SqBinding::sessionRef->getGameAdapter()->getDeityLocation( pos );
    if( deity ) {
      sq_pushstring( vm, _SC( deity ), -1 );
      return 1;
    } else {
      return sq_throwerror( vm, _SC( "No deity bound to this location." ) );
    }
  } else {
    return sq_throwerror( vm, _SC( "Location not found." ) );
  }
}

int SqGame::_endConversation( HSQUIRRELVM vm ) {
	SqBinding::sessionRef->getGameAdapter()->endConversation();
	return 0;
}

int SqGame::_getTranslatedString( HSQUIRRELVM vm ) {
	GET_STRING( s, 3000 )
	sq_pushstring( vm, _( s ), -1 );
  return 1;
}

