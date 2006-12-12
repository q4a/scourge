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
#include "../board.h"

// is this ok? maybe we want to go thru the GameAdapter.
#include "../render/map.h"
#include "../render/shape.h"
#include "../render/location.h"

using namespace std;

const char *SqMission::className = "Mission";
ScriptClassMemberDecl SqMission::members[] = {
  { "void", "_typeof", SqMission::_squirrel_typeof, 1, 0, "" },
  { "void", "constructor", SqMission::_constructor, 0, 0, "" },
  { "bool", "isCompleted", SqMission::_isCompleted, 0, 0, "" },
  { "void", "setCompleted", SqMission::_setCompleted, 0, 0, "Mark the current mission as completed." },
  { "int", "getCreatureCount", SqMission::_getCreatureCount, 0, 0, "Get the number of monsters and npc-s on this level." },
  { "Creature", "getCreature", SqMission::_getCreature, 0, 0, "Return a reference to a monster or npc on this level. These references are only valid while on this map." },
  { "int", "getItemCount", SqMission::_getItemCount, 0, 0, "Get the number of items on this level." },
  { "Item", "getItem", SqMission::_getItem, 0, 0, "Returns a reference to an item on this level. These references are only valid while on this map." },
  { "Item", "getCurrentWeapon", SqMission::_getCurrentWeapon, 0, 0, "Get the item currently used to attack the player. (or null if by hands or spell.)" },
	{ "int", "getChapter", SqMission::_getChapter, 0, 0, "Get the current storyline chapter." },
	{ "void", "removeMapPosition", SqMission::_removeMapPosition, 0, 0, "Remove the shape at this map position." },
	{ "String", "getShape", SqMission::_getShape, 0, 0, "Get the name of a shape at this position." },
	{ "int", "getDungeonDepth", SqMission::_getDungeonDepth, 0, 0, "Get the current depth." },
	{ "void", "descendDungeon", SqMission::_descendDungeon, 0, 0, "Travel one dungeon level lower." },
	{ "void", "ascendDungeon", SqMission::_ascendDungeon, 0, 0, "Travel one dungeon level higher." },
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

int SqMission::_getCurrentWeapon( HSQUIRRELVM vm ) {
  if( SqBinding::getCurrentWeapon() ) {
    sq_pushobject( vm, *( SqBinding::binding->getItemRef( SqBinding::getCurrentWeapon() ) ) );
  } else {
    sq_pushnull( vm );
  }
  return 1;
}

int SqMission::_isCompleted( HSQUIRRELVM vm ) {
  sq_pushbool( vm, ( SqBinding::sessionRef->getCurrentMission() && 
                     SqBinding::sessionRef->getCurrentMission()->isCompleted() ? 
                     true : false ) );
  return 1;
}

int SqMission::_setCompleted( HSQUIRRELVM vm ) {
  SqBinding::sessionRef->getGameAdapter()->completeCurrentMission();
  return 0;
}

int SqMission::_getChapter( HSQUIRRELVM vm ) {
	sq_pushinteger( vm, SqBinding::sessionRef->getBoard()->getStorylineIndex() );
  return 1;
}

int SqMission::_removeMapPosition( HSQUIRRELVM vm ) {
	GET_INT( z )
	GET_INT( y )
	GET_INT( x )
	SqBinding::sessionRef->getMap()->removeLocation( x, y, z );
	return 0;
}

int SqMission::_getShape( HSQUIRRELVM vm ) {
	GET_INT( z )
	GET_INT( y )
	GET_INT( x )
	Location *pos = SqBinding::sessionRef->getMap()->getLocation( x, y, z );
	if( pos && pos->shape ) {
		sq_pushstring( vm, pos->shape->getName(), -1 );
	} else {
		sq_pushnull( vm );
	}
	return 1;
}

int SqMission::_getDungeonDepth( HSQUIRRELVM vm ) {
	sq_pushinteger( vm, SqBinding::sessionRef->getGameAdapter()->getCurrentDepth() );
	return 1;
}

int SqMission::_descendDungeon( HSQUIRRELVM vm ) {
	GET_INT( z )
	GET_INT( y )
	GET_INT( x )
	SqBinding::sessionRef->getGameAdapter()->descendDungeon( 
		SqBinding::sessionRef->getMap()->getLocation( x, y, z ) );
	return 0;
}

int SqMission::_ascendDungeon( HSQUIRRELVM vm ) {
	GET_INT( z )
	GET_INT( y )
	GET_INT( x )
	SqBinding::sessionRef->getGameAdapter()->ascendDungeon( 
		SqBinding::sessionRef->getMap()->getLocation( x, y, z ) );
	return 0;
}