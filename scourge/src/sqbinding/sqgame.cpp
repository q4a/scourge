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

const char *SqGame::className = "ScourgeGame";
ScriptClassMemberDecl SqGame::members[] = {
  { "_typeof", SqGame::_squirrel_typeof, 1, 0 },
  { "constructor", SqGame::_constructor, 0, 0 },
  { "getVersion", SqGame::_getVersion, 0, 0 },
  { "getRootDir", SqGame::_getRootDir, 0, 0 },
  { "getPartySize", SqGame::_getPartySize, 0, 0 },
  { "getPartyMember", SqGame::_getPartyMember, 0, 0 },
  { "getSkillCount", SqGame::_getSkillCount, 0, 0 },
  { "getSkillName", SqGame::_getSkillName, 0, 0 },
  { "getMission", SqGame::_getMission, 0, 0 },
  { "getStateModCount", SqGame::_getStateModCount, 0, 0 },
  { "getStateModName", SqGame::_getStateModName, 0, 0 },
  { 0,0,0,0 } // terminator
};
SquirrelClassDecl SqGame::classDecl = { SqGame::className, 0, members };

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

