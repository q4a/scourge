/***************************************************************************
                          sqgame.h  -  description
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

#ifndef SQGAME_H
#define SQGAME_H

#include "sqbinding.h"
#include "sqobject.h"
#include <map>

class Creature;

/**
 * The topmost squirrel object in the Scourge object hierarchy.
 */
class SqGame : public SqObject {
private:
  static const char *className;
  static SquirrelClassDecl classDecl;
  static ScriptClassMemberDecl members[];

  

public:
  SqGame();
  ~SqGame();

  inline const char *getInstanceName() { return "scourgeGame"; }
  inline const char *getClassName() { return SqGame::className; }
  inline SquirrelClassDecl *getClassDeclaration() { return &SqGame::classDecl; }

  // ===========================================================================
  // Static callback methods to ScourgeGame squirrel object member functions.
  static int _squirrel_typeof( HSQUIRRELVM vm );
  static int _constructor( HSQUIRRELVM vm );

  // general
  static int _getVersion( HSQUIRRELVM vm );
  static int _getRootDir( HSQUIRRELVM vm );

  // party-related
  static int _getPartySize( HSQUIRRELVM vm );
  static int _getPartyMember( HSQUIRRELVM vm );
  static int _getPlayer( HSQUIRRELVM vm );

  static int _getDeityLocation( HSQUIRRELVM vm );

  static int _getMission( HSQUIRRELVM vm );

  static int _getSkillCount( HSQUIRRELVM vm );
  static int _getSkillName( HSQUIRRELVM vm );

  static int _getStateModCount( HSQUIRRELVM vm );
  static int _getStateModName( HSQUIRRELVM vm );
  static int _getStateModByName( HSQUIRRELVM vm );

  static int _getDateString( HSQUIRRELVM vm );
  static int _isADayLater( HSQUIRRELVM vm );

  static int _getValue( HSQUIRRELVM vm );
  static int _setValue( HSQUIRRELVM vm );
  static int _eraseValue( HSQUIRRELVM vm );

  static int _printMessage( HSQUIRRELVM vm );
  static int _reloadNuts( HSQUIRRELVM vm );
  static int _documentSOM( HSQUIRRELVM vm );
  static int _runTests( HSQUIRRELVM vm );

};

#endif

