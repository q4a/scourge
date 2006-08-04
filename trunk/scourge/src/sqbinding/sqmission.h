/***************************************************************************
                          sqmission.h  -  description
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

  inline const char *getInstanceName() { return "mission"; }
  inline const char *getClassName() { return SqMission::className; }
  inline SquirrelClassDecl *getClassDeclaration() { return &SqMission::classDecl; }

  // ===========================================================================
  // Static callback methods to ScourgeGame squirrel object member functions.
  static int _squirrel_typeof( HSQUIRRELVM vm );
  static int _constructor( HSQUIRRELVM vm );

  // general
  static int _getCreatureCount( HSQUIRRELVM vm );
  static int _getCreature( HSQUIRRELVM vm );

  static int _getItemCount( HSQUIRRELVM vm );
  static int _getItem( HSQUIRRELVM vm );

  // battle-related (maybe move this to another class?)
  static int _getCurrentWeapon( HSQUIRRELVM vm );

  static int _isCompleted( HSQUIRRELVM vm );
  static int _setCompleted( HSQUIRRELVM vm );

	static int _getChapter( HSQUIRRELVM vm );

	static int _removeMapPosition( HSQUIRRELVM vm );
	static int _getShape( HSQUIRRELVM vm );

	static int _getDungeonDepth( HSQUIRRELVM vm );
	static int _descendDungeon( HSQUIRRELVM vm );
	static int _ascendDungeon( HSQUIRRELVM vm );
};

#endif

