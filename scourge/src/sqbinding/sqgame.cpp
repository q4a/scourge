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

using namespace std;

const char *SqGame::className = "ScourgeGame";
ScriptClassMemberDecl SqGame::members[] = {
  { "_typeof", SqGame::_squirrel_typeof, 1, 0 },
  { "constructor", SqGame::_constructor, 0, 0 },
  { "getVersion", SqGame::_getVersion, 0, 0 },
  { "getRootDir", SqGame::_getRootDir, 0, 0 },
  { "getPartySize", SqGame::_getPartySize, 0, 0 },
  { "getPartyMember", SqGame::_getPartyMember, 0, 0 },
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

void printArgs( HSQUIRRELVM v ) {
  SQInteger nargs = sq_gettop(v); //number of arguments
  for(SQInteger n=1;n<=nargs;n++) {
    printf("arg %d is ",n);
    switch(sq_gettype(v,n)) {
    case OT_NULL:
    printf("null");        
    break;
    case OT_INTEGER:
    printf("integer");
    break;
    case OT_FLOAT:
    printf("float");
    break;
    case OT_STRING:
    printf("string");
    break;    
    case OT_TABLE:
    printf("table");
    break;
    case OT_ARRAY:
    printf("array");
    break;
    case OT_USERDATA:
    printf("userdata");
    break;
    case OT_CLOSURE:        
    printf("closure(function)");    
    break;
    case OT_NATIVECLOSURE:
    printf("native closure(C function)");
    break;
    case OT_GENERATOR:
    printf("generator");
    break;
    case OT_USERPOINTER:
    printf("userpointer");
    break;
    case OT_INSTANCE:
    printf( "object instance" );
    break;
    default:
    cerr << "invalid param" << endl; //throws an exception
    }
  }
  printf("\n");
}

int SqGame::_getPartyMember( HSQUIRRELVM vm ) {
  printArgs( vm );

  int partyIndex;
  if( SQ_FAILED( sq_getinteger( vm, 2, &partyIndex ) ) ) {
    cerr << "*** Error: Can't get party index in _getPartyMember. Using 0 instead." << endl;
    partyIndex = 0;
  }
  if( partyIndex < 0 || partyIndex > SqBinding::sessionRef->getParty()->getPartySize() ) {
    cerr << "*** Error: party index of " << partyIndex << " is out of range. Using 0 instead." << endl;
    partyIndex = 0;
  }
  if( DEBUG_SQUIRREL ) cerr << "partyIndex=" << partyIndex << endl;

  sq_pushobject( vm, SqBinding::binding->refParty[ partyIndex ] );
  return 1;
}
