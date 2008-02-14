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
#include "../render/glshape.h"
#include "../render/location.h"
#include "../shapepalette.h"

using namespace std;

const char *SqMission::className = "Mission";
ScriptClassMemberDecl SqMission::members[] = {
  { "void", "_typeof", SqMission::_squirrel_typeof, 1, 0, "" },
  { "void", "constructor", SqMission::_constructor, 0, 0, "" },
  { "bool", "isCompleted", SqMission::_isCompleted, 0, 0, "" },
  { "void", "setCompleted", SqMission::_setCompleted, 0, 0, "Mark the current mission as completed." },
  { "int", "getCreatureCount", SqMission::_getCreatureCount, 0, 0, "Get the number of monsters and npc-s on this level." },
  { "Creature", "getCreature", SqMission::_getCreature, 0, 0, "Return a reference to a monster or npc on this level. These references are only valid while on this map." },
	{ "void", "replaceCreature", SqMission::_replaceCreature, 0, 0, "Replace the given creature with a new one of the given type on the map." },
	{ "Creature", "addCreature", SqMission::_addCreature, 0, 0, "Create a new creature from the given template. Return a reference to the monster or npc created. It is valid while on this map." },
  { "int", "getItemCount", SqMission::_getItemCount, 0, 0, "Get the number of items on this level." },
  { "Item", "getItem", SqMission::_getItem, 0, 0, "Returns a reference to an item on this level. These references are only valid while on this map." },
  { "Item", "getCurrentWeapon", SqMission::_getCurrentWeapon, 0, 0, "Get the item currently used to attack the player. (or null if by hands or spell.)" },
	{ "int", "getChapter", SqMission::_getChapter, 0, 0, "Get the current storyline chapter." },
	{ "void", "removeMapPosition", SqMission::_removeMapPosition, 0, 0, "Remove the shape at this map position." },
	{ "void", "setMapPosition", SqMission::_setMapPosition, 0, 0, "Set a shape at this map position. Shape is given by its name." },
	{ "float", "getHeightMap", SqMission::_getHeightMap, 0, 0, "Get the ground height (outdoors only) at this map position." },
	{ "void", "setHeightMap", SqMission::_setHeightMap, 0, 0, "Set the ground height (outdoors only) at this map position." },
	{ "String", "getShape", SqMission::_getShape, 0, 0, "Get the name of a shape at this position." },
	{ "int", "getDungeonDepth", SqMission::_getDungeonDepth, 0, 0, "Get the current depth." },
	{ "void", "descendDungeon", SqMission::_descendDungeon, 0, 0, "Travel one dungeon level lower." },
	{ "void", "ascendDungeon", SqMission::_ascendDungeon, 0, 0, "Travel one dungeon level higher." },
	{ "bool", "areQuakesEnabled", SqMission::_areQuakesEnabled, 0, 0, "Are earthquakes enabled on this level?" },
	{ "void", "setQuakesEnabled", SqMission::_setQuakesEnabled, 0, 0, "Set to true if quakes are enabled on this level. (False by default.)" },
	{ "void", "setDoorLocked", SqMission::_setDoorLocked, 0, 0, "Set the door located at x,y,z to locked value (true=locked, false=unlocked)" },
	{ "bool", "isDoorLocked", SqMission::_isDoorLocked, 0, 0, "Is the door at location x,y,z locked?" },
	{ "bool", "isStoryLineMission", SqMission::_isStoryLineMission, 0, 0, "Is the current mission a storyline mission?" },
	{ "bool", "isReplayMap", SqMission::_isReplayMap, 0, 0, "Is the current mission a replayed storyline map?" },
	{ "void", "setMapConfig", SqMission::_setMapConfig, 0, 0, "Load conversation and npc info from this file and apply it to the current map." },	
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

int SqMission::_replaceCreature( HSQUIRRELVM vm ) {
	GET_STRING( newCreatureType, 200 )
	GET_OBJECT( Creature* )
	Creature *replacement = SqBinding::sessionRef->replaceCreature( object, newCreatureType );
	sq_pushobject( vm, *(SqBinding::binding->creatureMap[ replacement ]) );
	return 1;
}

int SqMission::_addCreature( HSQUIRRELVM vm ) {
	GET_STRING( creatureType, 200 )
	GET_INT( z )
	GET_INT( y )
	GET_INT( x )
	Creature *c = SqBinding::sessionRef->addCreatureFromScript( creatureType, x, y );
	sq_pushobject( vm, *(SqBinding::binding->creatureMap[ c ]) );
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

int SqMission::_setMapPosition( HSQUIRRELVM vm ) {
	GET_STRING( shapeName, 255 )
	GET_INT( z )
	GET_INT( y )
	GET_INT( x )
	GLShape *shape = SqBinding::sessionRef->getShapePalette()->findShapeByName( shapeName );
	SqBinding::sessionRef->getMap()->setPosition( x, y, z, shape );
	return 0;
}

int SqMission::_getHeightMap( HSQUIRRELVM vm ) {
	GET_INT( y )
	GET_INT( x )
	if( SqBinding::sessionRef->getMap()->isHeightMapEnabled() ) {
		sq_pushfloat( vm, SqBinding::sessionRef->getMap()->getGroundHeight( x / OUTDOORS_STEP, y / OUTDOORS_STEP ) );
	} else {
		sq_pushfloat( vm, 0 );
	}
	return 1;
}

int SqMission::_setHeightMap( HSQUIRRELVM vm ) {
	GET_FLOAT( h )
	GET_INT( y )
	GET_INT( x )
	if( SqBinding::sessionRef->getMap()->isHeightMapEnabled() ) {
		SqBinding::sessionRef->getMap()->setGroundHeight( x / OUTDOORS_STEP, y / OUTDOORS_STEP, h );
	}
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

int SqMission::_setQuakesEnabled( HSQUIRRELVM vm ) {
	GET_BOOL( b )
	SqBinding::sessionRef->getMap()->setQuakesEnabled( b );
	return 0;
}

int SqMission::_areQuakesEnabled( HSQUIRRELVM vm ) {
	SQBool b = SqBinding::sessionRef->getMap()->areQuakesEnabled();
	sq_pushbool( vm, b );
	return 1;
}

int SqMission::_setDoorLocked( HSQUIRRELVM vm ) {
	GET_BOOL( locked )
	GET_INT( z )
	GET_INT( y )
	GET_INT( x )
	/*
	cerr << "isDoor=" << SqBinding::sessionRef->getMap()->isDoor( x, y ) << endl;
	Location *pos = SqBinding::sessionRef->getMap()->getLocation( x, y, z );
	if( pos ) {
		cerr << "\tpos=" << pos->shape->getName() << " at: " << pos->x << "," << pos->y << "," << pos->z << endl;
	} else {
		cerr << "\tpos=NULL" << endl;
	}
	cerr << "BEFORE locked=" << SqBinding::sessionRef->getMap()->isLocked( x, y, z ) << endl;
	cerr << "Setting door at " << x << "," << y << "," << z << " to locked: " << locked << endl;
	*/
	SqBinding::sessionRef->getMap()->setLocked( x, y, z, locked );
	//cerr << "AFTER locked=" << SqBinding::sessionRef->getMap()->isLocked( x, y, z ) << endl;
	return 0;
}

int SqMission::_isDoorLocked( HSQUIRRELVM vm ) {
	GET_INT( z )
	GET_INT( y )
	GET_INT( x )
	SQBool b = SqBinding::sessionRef->getMap()->isLocked( x, y, z );
	sq_pushbool( vm, b );
	return 1;
}

int SqMission::_isStoryLineMission( HSQUIRRELVM vm ) {
	Mission *m = SqBinding::sessionRef->getCurrentMission();
	SQBool b = ( m && m->isStoryLine() );
	sq_pushbool( vm, b );
	return 1;
}

int SqMission::_isReplayMap( HSQUIRRELVM vm ) {
	Mission *m = SqBinding::sessionRef->getCurrentMission();
	SQBool b = ( m && m->isReplay() );
	sq_pushbool( vm, b );
	return 1;
}

int SqMission::_setMapConfig( HSQUIRRELVM vm ) {
	GET_STRING( mapName, 255 )
	Mission::loadMapConfig( SqBinding::sessionRef->getGameAdapter(), mapName );
	return 0;
}

