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
#include "../session.h"
#include "../creature.h"

using namespace std;

const char *SqCreature::className = "Creature";
ScriptClassMemberDecl SqCreature::members[] = {
  { "_typeof", SqCreature::_squirrel_typeof, 1, 0 },
  { "constructor", SqCreature::_constructor, 0, 0 },
  { "getName", SqCreature::_getName, 0, 0 },
  { 0,0,0,0 } // terminator
};
SquirrelClassDecl SqCreature::classDecl = { SqCreature::className, 0, members };

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

int SqCreature::_getName( HSQUIRRELVM vm ) {
  cerr << "*** FIXME: name should come from creature object ref ('this' param)." << endl;


  // FIXME: retrieve the CREATURE_ID_TOKEN from the "this" object (param 1) and use it to find the Creature*.
  int index;
  int top = sq_gettop( vm );
  cerr << "_getName, 1, top=" << sq_gettop( vm ) << endl;
  //HSQOBJECT object;
  //sq_getstackobj( vm, 1, &object );
  sq_pushstring( vm, _SC( CREATURE_ID_TOKEN ), -1 );
  cerr << "_getName, 2, top=" << sq_gettop( vm ) << endl;
  if( SQ_FAILED( sq_get( vm, -2 ) ) ) {
    cerr << "Failed to get creature-token in _getName(). Using index 0 instead." << endl;
    index = 0;
  }
  cerr << "_getName, 3, top=" << sq_gettop( vm ) << endl;
  sq_getinteger( vm, -1, &index );
  sq_settop( vm, top );
  cerr << "_getName, 4, top=" << sq_gettop( vm ) << endl;
  cerr << "index=" << index << endl;




  /*
  Creature *creature = ( SqGame::partyToCreatureMap.find( object ) != 
                         SqGame::partyToCreatureMap.end() ? 
                         SqGame::partyToCreatureMap[ object ] :
                         NULL );
  sq_pushstring( vm, ( creature ? creature->getName() : "Null" ), -1 );
  return 1;
  */

  sq_pushstring( vm, _SC( "Alamont" ), -1 );
  return 1;

  return 0;
}
