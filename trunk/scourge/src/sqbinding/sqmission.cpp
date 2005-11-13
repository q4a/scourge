/***************************************************************************
                          sqmission.cpp  -  description
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
#include "sqmission.h"
#include "../session.h"
#include "../creature.h"
#include "../item.h"

using namespace std;

const char *SqMission::className = "Mission";
ScriptClassMemberDecl SqMission::members[] = {
  { "void", "_typeof", SqMission::_squirrel_typeof, 1, 0, "" },
  { "void", "constructor", SqMission::_constructor, 0, 0, "" },
  { "int", "getCreatureCount", SqMission::_getCreatureCount, 0, 0, "Get the number of monsters and npc-s on this level." },
  { "Creature", "getCreature", SqMission::_getCreature, 0, 0, "Return a reference to a monster or npc on this level. These references are only valid while on this map." },
  { "int", "getItemCount", SqMission::_getItemCount, 0, 0, "Get the number of items on this level." },
  { "Item", "getItem", SqMission::_getItem, 0, 0, "Returns a reference to an item on this level. These references are only valid while on this map." },
  { 0,0,0,0,0 } // terminator
};
SquirrelClassDecl SqMission::classDecl = { SqMission::className, 0, members,
  "Information about the currently used map." };

SqMission::SqMission() {
}

SqMission::~SqMission() {
}

// ===========================================================================
// Static callback methods to ScourgeGame squirrel object member functions.
int SqMission::_squirrel_typeof( HSQUIRRELVM vm ) {
  sq_pushstring( vm, SqMission::className, -1 );
  return 1; // 1 value is returned
}

int SqMission::_constructor( HSQUIRRELVM vm ) {
  return 0; // no values returned
}

int SqMission::_getCreatureCount( HSQUIRRELVM vm ) {
  sq_pushinteger( vm, SqBinding::sessionRef->getCreatureCount() );
  return 1;
}

int SqMission::_getCreature( HSQUIRRELVM vm ) {
  int index;
  if( SQ_FAILED( sq_getinteger( vm, 2, &index ) ) ) {
    return sq_throwerror( vm, _SC( "Can't get creature index in _getCreature." ) );
  }
  if( index < 0 || index > SqBinding::sessionRef->getCreatureCount() ) {
    return sq_throwerror( vm, _SC( "Creature index is out of range." ) );
  }

  sq_pushobject( vm, *(SqBinding::binding->refCreature[index]) );
  return 1;
}

int SqMission::_getItemCount( HSQUIRRELVM vm ) {
  sq_pushinteger( vm, SqBinding::sessionRef->getItemCount() );
  return 1;
}

int SqMission::_getItem( HSQUIRRELVM vm ) {
  int index;
  if( SQ_FAILED( sq_getinteger( vm, 2, &index ) ) ) {
    return sq_throwerror( vm, _SC( "Can't get item index in _getItem." ) );
  }
  if( index < 0 || index > SqBinding::sessionRef->getItemCount() ) {
    return sq_throwerror( vm, _SC( "Item index is out of range." ) );
  }

  sq_pushobject( vm, *(SqBinding::binding->refItem[ index ]) );
  return 1;
}

