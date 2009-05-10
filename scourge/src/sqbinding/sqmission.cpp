/***************************************************************************
              sqmission.cpp  -  Squirrel binding - Mission class
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
#include "../common/constants.h"
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
	{ "Creature", "addCreature", SqMission::_addCreature, 0, 0, "Create a new creature from the given template exactly at the specified location. Return a reference to the monster or npc created. It is valid while on this map." },
	{ "Creature", "addCreatureAround", SqMission::_addCreatureAround, 0, 0, "Create a new creature from the given template around the specified location. Return a reference to the monster or npc created. It is valid while on this map." },
	{ "int", "getItemCount", SqMission::_getItemCount, 0, 0, "Get the number of items on this level." },
	{ "Item", "getItem", SqMission::_getItem, 0, 0, "Returns a reference to an item on this level. These references are only valid while on this map." },
	{ "Item", "getCurrentWeapon", SqMission::_getCurrentWeapon, 0, 0, "Get the item currently used to attack the player. (or null if by hands or spell.)" },
	{ "int", "getChapter", SqMission::_getChapter, 0, 0, "Get the current storyline chapter." },
	{ "void", "removeMapPosition", SqMission::_removeMapPosition, 0, 0, "Remove the shape at this map position." },
	{ "void", "isFree", SqMission::_isFree, 0, 0, "Will this place fit at this location? This method returns false if the position is occupied or if it would block a door." },
	{ "void", "isFreeOutdoors", SqMission::_isFreeOutdoors, 0, 0, "Same as isFree plus it discounts lakes, high elevations and where a floor shape is set." },
	{ "void", "setMapPosition", SqMission::_setMapPosition, 0, 0, "Set a shape at this map position. Shape is given by its name." },
	{ "void", "setMapFloorPosition", SqMission::_setMapFloorPosition, 0, 0, "Set a floor shape at this map position. Shape is given by its name." },
	{ "void", "flattenChunk", SqMission::_flattenChunk, 0, 0, "Flatten a chunk on an outdoors map." },
	{ "void", "flattenLandChunk", SqMission::_flattenLandChunk, 0, 0, "Flatten a chunk on an outdoors map, leave water intact." },
	{ "void", "setMapEffect", SqMission::_setMapEffect, 0, 0, "Set an effect at this map position. The effect is identified by its name." },
	{ "void", "removeMapEffect", SqMission::_removeMapEffect, 0, 0, "Remove an effect at this map position." },
	{ "float", "getHeightMap", SqMission::_getHeightMap, 0, 0, "Get the ground height (outdoors only) at this map position." },
	{ "void", "setHeightMap", SqMission::_setHeightMap, 0, 0, "Set the ground height (outdoors only) at this map position." },
	{ "String", "getShape", SqMission::_getShape, 0, 0, "Get the name of a shape at this position." },
	{ "int", "getDungeonDepth", SqMission::_getDungeonDepth, 0, 0, "Get the current depth." },
	{ "void", "descendDungeon", SqMission::_descendDungeon, 0, 0, "Travel one dungeon level lower." },
	{ "void", "ascendDungeon", SqMission::_ascendDungeon, 0, 0, "Travel one dungeon level higher." },
	{ "bool", "areQuakesEnabled", SqMission::_areQuakesEnabled, 0, 0, "Are earthquakes enabled on this level?" },
	{ "void", "setQuakesEnabled", SqMission::_setQuakesEnabled, 0, 0, "Set to true if quakes are enabled on this level. (False by default.)" },
	{ "void", "quake", SqMission::_quake, 0, 0, "Start an earthquake now." },
	{ "void", "thunder", SqMission::_thunder, 0, 0, "Start a thunder now." },
	{ "void", "setDoorLocked", SqMission::_setDoorLocked, 0, 0, "Set the door located at x,y,z to locked value (true=locked, false=unlocked)" },
	{ "bool", "isDoorLocked", SqMission::_isDoorLocked, 0, 0, "Is the door at location x,y,z locked?" },
	{ "bool", "isStoryLineMission", SqMission::_isStoryLineMission, 0, 0, "Is the current mission a storyline mission?" },
	{ "bool", "isReplayMap", SqMission::_isReplayMap, 0, 0, "Is the current mission a replayed storyline map?" },
	{ "void", "setMapConfig", SqMission::_setMapConfig, 0, 0, "Load conversation and npc info from this file and apply it to the current map." },
	{ "Item", "addItem", SqMission::_addItem, 0, 0, "Create a new item on the map at this location." },
	{ "void", "setOffset", SqMission::_setOffset, 0, 0, "Set a location's offset on the map." },
	{ "void", "startHouse", SqMission::_startHouse, 0, 0, "Start defining a house." },
	{ "void", "endHouse", SqMission::_endHouse, 0, 0, "End defining a house." },
	{ "void", "clearHouses", SqMission::_clearHouses, 0, 0, "Clear house definitions." },
	{ "void", "setRug", SqMission::_setRug, 0, 0, "Put a rug on the map." },
	{ "void", "removeRug", SqMission::_removeRug, 0, 0, "Remove a rug from the map." },
	{ "void", "addOutdoorTexture", SqMission::_addOutdoorTexture, 0, 0, "Set an outdoor texture (like a road)." },
	{ "float", "mt_rand", SqMission::_mt_rand, 0, 0, "Return a random float between 0 and 1." },
	{ 0, 0, 0, 0, 0 } // terminator
};
SquirrelClassDecl SqMission::classDecl = { SqMission::className, 0, members,
    "Information about the currently used map."
                                         };

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
	if ( SQ_FAILED( sq_getinteger( vm, 2, &index ) ) ) {
		return sq_throwerror( vm, _SC( "Can't get creature index in _getCreature." ) );
	}
	if ( index < 0 || index > SqBinding::sessionRef->getCreatureCount() ) {
		return sq_throwerror( vm, _SC( "Creature index is out of range." ) );
	}

	sq_pushobject( vm, *( SqBinding::binding->creatureMap[ SqBinding::sessionRef->getCreature( index ) ] ) );
	//sq_pushobject( vm, *( SqBinding::binding->refCreature[index] ) );
	return 1;
}

int SqMission::_replaceCreature( HSQUIRRELVM vm ) {
	GET_STRING( newCreatureType, 200 )
	GET_OBJECT( Creature* )
	Creature *replacement = SqBinding::sessionRef->replaceCreature( object, newCreatureType );
	sq_pushobject( vm, *( SqBinding::binding->creatureMap[ replacement ] ) );
	return 1;
}

int SqMission::_addCreature( HSQUIRRELVM vm ) {
	GET_STRING( creatureType, 200 )
	GET_INT( z )
	GET_INT( y )
	GET_INT( x )
	Creature *c = SqBinding::sessionRef->addCreatureFromScript( creatureType, x, y );
	sq_pushobject( vm, *( SqBinding::binding->creatureMap[ c ] ) );
	return 1;
}

int SqMission::_addCreatureAround( HSQUIRRELVM vm ) {
	GET_STRING( creatureType, 200 )
	GET_INT( z )
	GET_INT( y )
	GET_INT( x )
	int fx, fy;
	Creature *c = SqBinding::sessionRef->addCreatureFromScript( creatureType, x, y, &fx, &fy );
	sq_pushobject( vm, *( SqBinding::binding->creatureMap[ c ] ) );
	return 1;
}


int SqMission::_addItem( HSQUIRRELVM vm ) {
	GET_BOOL( isContainer )
	GET_INT( z )
	GET_INT( y )
	GET_INT( x )
	GET_STRING( itemType, 200 )
	Item *item = SqBinding::sessionRef->addItemFromScript( itemType, x, y, z, isContainer, 1, 1 );
	sq_pushobject( vm, *( SqBinding::binding->itemMap[ item ] ) );
	return 1;
}

int SqMission::_startHouse( HSQUIRRELVM vm ) {
	SqBinding::sessionRef->getMap()->startHouse();
	return 0;
}

int SqMission::_endHouse( HSQUIRRELVM vm ) {
	SqBinding::sessionRef->getMap()->endHouse();
	return 0;
}

int SqMission::_clearHouses( HSQUIRRELVM vm ) {
	SqBinding::sessionRef->getMap()->clearHouses();
	return 0;
}

int SqMission::_getItemCount( HSQUIRRELVM vm ) {
	sq_pushinteger( vm, SqBinding::sessionRef->getItemCount() );
	return 1;
}

int SqMission::_getItem( HSQUIRRELVM vm ) {
	int index;
	if ( SQ_FAILED( sq_getinteger( vm, 2, &index ) ) ) {
		return sq_throwerror( vm, _SC( "Can't get item index in _getItem." ) );
	}
	if ( index < 0 || index > SqBinding::sessionRef->getItemCount() ) {
		return sq_throwerror( vm, _SC( "Item index is out of range." ) );
	}

	sq_pushobject( vm, *( SqBinding::binding->refItem[ index ] ) );
	return 1;
}

int SqMission::_getCurrentWeapon( HSQUIRRELVM vm ) {
	if ( SqBinding::getCurrentWeapon() ) {
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
	if( shape->getIgnoreHeightMap() && !shape->getNotFlatten() ) SqBinding::sessionRef->getMap()->flattenChunk( x, y - MAP_UNIT ); 
	SqBinding::sessionRef->getMap()->setPosition( x, y, z, shape );
	return 0;
}

int SqMission::_setRug( HSQUIRRELVM vm ) {
	GET_INT( y )
	GET_INT( x )
	Rug rug;
	rug.isHorizontal = Util::dice( 2 ) == 0 ? true : false;
	rug.texture = SqBinding::sessionRef->getShapePalette()->getRandomRug();
	rug.angle = Util::roll( -15.0f, 15.0f );
	SqBinding::sessionRef->getMap()->setRugPosition( x, y, &rug );
	return 0;
}

int SqMission::_removeRug( HSQUIRRELVM vm ) {
	GET_INT( y )
	GET_INT( x )
	SqBinding::sessionRef->getMap()->removeRugPosition( x, y );
	return 0;
}

int SqMission::_setMapFloorPosition( HSQUIRRELVM vm ) {
	GET_STRING( shapeName, 255 )
	GET_INT( y )
	GET_INT( x )
	GLShape *shape = SqBinding::sessionRef->getShapePalette()->findShapeByName( shapeName );
	SqBinding::sessionRef->getMap()->flattenChunk( x, y - MAP_UNIT );
	SqBinding::sessionRef->getMap()->setFloorPosition( x, y, shape );
	return 0;
}

int SqMission::_flattenChunk( HSQUIRRELVM vm ) {
	GET_INT( y )
	GET_INT( x )
	SqBinding::sessionRef->getMap()->flattenChunk( x, y );
	return 0;
}

int SqMission::_flattenLandChunk( HSQUIRRELVM vm ) {
	GET_INT( y )
	GET_INT( x )
	SqBinding::sessionRef->getMap()->flattenChunk( x, y, 0.0f, true );
	return 0;
}

int SqMission::_removeMapEffect( HSQUIRRELVM vm ) {
	GET_INT( z )
	GET_INT( y )
	GET_INT( x )
	SqBinding::sessionRef->getMap()->stopEffect( x, y, z );
	return 0;
}

int SqMission::_setMapEffect( HSQUIRRELVM vm ) {
	GET_FLOAT( b )
	GET_FLOAT( g )
	GET_FLOAT( r )
	GET_FLOAT( oz )
	GET_FLOAT( oy )
	GET_FLOAT( ox )
	GET_BOOL( forever )
	GET_INT( delay )
	GET_INT( h )
	GET_INT( w )
	GET_STRING( effectName, 255 )
	GET_INT( z )
	GET_INT( y )
	GET_INT( x )
	int effect_type = 0;
	for ( int i = 0; i < Constants::EFFECT_COUNT; i++ ) {
		if ( !strcmp( effectName, Constants::EFFECT_NAMES[i] ) ) {
			effect_type = i;
			break;
		}
	}
	DisplayInfo di;
	di.red = r;
	di.green = g;
	di.blue = b;
	di.offset_x = ox;
	di.offset_y = oy;
	di.offset_z = oz;
	SqBinding::sessionRef->getMap()->startEffect( x, y, z, effect_type, ( Constants::DAMAGE_DURATION * 4 ), w, h, delay, forever, &di );
	return 0;
}

int SqMission::_isFree( HSQUIRRELVM vm ) {
	GET_STRING( shapeName, 255 )
	GET_INT( z )
	GET_INT( y )
	GET_INT( x )
	GLShape *shape = SqBinding::sessionRef->getShapePalette()->findShapeByName( shapeName );
	SQBool b = ( SqBinding::sessionRef->getMap()->shapeFits( shape, x, y, z ) &&
	             !SqBinding::sessionRef->getMap()->coversDoor( shape, x, y ) );
	sq_pushbool( vm, b );
	return 1;
}

int SqMission::_isFreeOutdoors( HSQUIRRELVM vm ) {
	GET_STRING( shapeName, 255 )
	GET_INT( z )
	GET_INT( y )
	GET_INT( x )
	GLShape *shape = SqBinding::sessionRef->getShapePalette()->findShapeByName( shapeName );
	SQBool b = SqBinding::sessionRef->getMap()->shapeFitsOutdoors( shape, x, y, z );
	sq_pushbool( vm, b );
	return 1;
}

int SqMission::_getHeightMap( HSQUIRRELVM vm ) {
	GET_INT( y )
	GET_INT( x )
	if ( SqBinding::sessionRef->getMap()->isHeightMapEnabled() ) {
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
	if ( SqBinding::sessionRef->getMap()->isHeightMapEnabled() ) {
		SqBinding::sessionRef->getMap()->setGroundHeight( x / OUTDOORS_STEP, y / OUTDOORS_STEP, h );
	}
	return 0;
}

int SqMission::_getShape( HSQUIRRELVM vm ) {
	GET_INT( z )
	GET_INT( y )
	GET_INT( x )
	Location *pos = SqBinding::sessionRef->getMap()->getLocation( x, y, z );
	if ( pos && pos->shape ) {
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

int SqMission::_quake( HSQUIRRELVM vm ) {
	SqBinding::sessionRef->getMap()->quake();
	return 0;
}

int SqMission::_thunder( HSQUIRRELVM vm ) {
	SqBinding::sessionRef->getWeather()->thunder();
	return 0;
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

int SqMission::_setOffset( HSQUIRRELVM vm ) {
	GET_FLOAT( oz )
	GET_FLOAT( oy )
	GET_FLOAT( ox )
	GET_INT( z )
	GET_INT( y )
	GET_INT( x )
	Location *pos = SqBinding::sessionRef->getMap()->getLocation( x, y, z );
	if ( pos ) {
		pos->moveX = ox * MUL;
		pos->moveY = oy * MUL;
		pos->moveZ = oz * MUL;
	}
	return 0;
}

int SqMission::_addOutdoorTexture( HSQUIRRELVM vm ) {
	GET_BOOL( vert )
	GET_BOOL( horiz )
	GET_FLOAT( angle )
	GET_INT( ref )
	GET_INT( mapy )
	GET_INT( mapx )
	SqBinding::sessionRef->getMap()->addOutdoorTexture( mapx, mapy, ref, angle, horiz, vert );	
	return 0;
}

int SqMission::_mt_rand( HSQUIRRELVM vm ) {
	sq_pushfloat( vm, Util::mt_rand() );
	return 1;
}
