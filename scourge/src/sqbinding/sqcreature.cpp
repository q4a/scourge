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
  SQUserPointer creature;
  if( SqBinding::getObjectValue( vm, CREATURE_ID_TOKEN, &creature ) ) {
    sq_pushstring( vm, _SC( ((Creature*)creature)->getName() ), -1 );
    return 1;
  } else {
    return sq_throwerror( vm, _SC( "Can't find userpointer." ) );
  }
}
