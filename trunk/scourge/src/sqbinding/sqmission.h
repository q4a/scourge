/***************************************************************************
               sqmission.h  -  Squirrel binding - Mission class
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

#ifndef SQMISSION_H
#define SQMISSION_H
#pragma once

#include "sqbinding.h"
#include "sqobject.h"
#include <map>

/**
 * The topmost squirrel object in the Scourge object hierarchy.
 */
class SqMission : public SqObject {
private:
	static const char *className;
	static SquirrelClassDecl classDecl;
	static ScriptClassMemberDecl members[];



public:
	SqMission();
	~SqMission();

	inline const char *getInstanceName() {
		return "mission";
	}
	inline const char *getClassName() {
		return SqMission::className;
	}
	inline SquirrelClassDecl *getClassDeclaration() {
		return &SqMission::classDecl;
	}

	// ===========================================================================
	// Static callback methods to ScourgeGame squirrel object member functions.
	static int _squirrel_typeof( HSQUIRRELVM vm );
	static int _constructor( HSQUIRRELVM vm );

	// general
	static int _getCreatureCount( HSQUIRRELVM vm );
	static int _getCreature( HSQUIRRELVM vm );
	static int _replaceCreature( HSQUIRRELVM vm );
	static int _addCreature( HSQUIRRELVM vm );
	static int _addCreatureAround( HSQUIRRELVM vm );
	static int _addCreatureBounded( HSQUIRRELVM vm );
	static int _addWanderingHero( HSQUIRRELVM vm );

	static int _getItemCount( HSQUIRRELVM vm );
	static int _getItem( HSQUIRRELVM vm );
	static int _addItem( HSQUIRRELVM vm );

	// battle-related (maybe move this to another class?)
	static int _getCurrentWeapon( HSQUIRRELVM vm );

	static int _isCompleted( HSQUIRRELVM vm );
	static int _setCompleted( HSQUIRRELVM vm );

	static int _getChapter( HSQUIRRELVM vm );

	static int _isFree( HSQUIRRELVM vm );
	static int _isFreeOutdoors( HSQUIRRELVM vm );
	static int _setMapPosition( HSQUIRRELVM vm );
	static int _setMapFloorPosition( HSQUIRRELVM vm );
	static int _setMapEffect( HSQUIRRELVM vm );
	static int _removeMapEffect( HSQUIRRELVM vm );
	static int _removeMapPosition( HSQUIRRELVM vm );
	static int _flattenChunk( HSQUIRRELVM vm );
	static int _flattenChunkWalkable( HSQUIRRELVM vm );
	static int _getShape( HSQUIRRELVM vm );

	static int _getHeightMap( HSQUIRRELVM vm );
	static int _setHeightMap( HSQUIRRELVM vm );

	static int _getDungeonDepth( HSQUIRRELVM vm );
	static int _descendDungeon( HSQUIRRELVM vm );
	static int _ascendDungeon( HSQUIRRELVM vm );

	static int _setQuakesEnabled( HSQUIRRELVM vm );
	static int _areQuakesEnabled( HSQUIRRELVM vm );
	static int _quake( HSQUIRRELVM vm );
	static int _thunder( HSQUIRRELVM vm );

	static int _setDoorLocked( HSQUIRRELVM vm );
	static int _isDoorLocked( HSQUIRRELVM vm );

	static int _isStoryLineMission( HSQUIRRELVM vm );
	static int _isReplayMap( HSQUIRRELVM vm );

	static int _setMapConfig( HSQUIRRELVM vm );

	static int _setOffset( HSQUIRRELVM vm );
	
	static int _startHouse( HSQUIRRELVM vm );
	static int _endHouse( HSQUIRRELVM vm );
	static int _clearHouses( HSQUIRRELVM vm ); 
	
	static int _setRug( HSQUIRRELVM vm );
	static int _removeRug( HSQUIRRELVM vm ); 	
	
	static int _addOutdoorTexture( HSQUIRRELVM vm );

	static int _mt_rand( HSQUIRRELVM vm );
	
	static int _getCityName( HSQUIRRELVM vm );
};

#endif

