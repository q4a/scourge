/***************************************************************************
                          sqspell.h  -  description
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

#ifndef SQSPELL_H
#define SQSPELL_H

#include "sqbinding.h"
#include "sqobject.h"
#include <map>

/**
 * The topmost squirrel object in the Scourge object hierarchy.
 */
class SqSpell : public SqObject {
private:
  static const char *className;
  static SquirrelClassDecl classDecl;
  static ScriptClassMemberDecl members[];

  

public:
  SqSpell();
  ~SqSpell();

  inline const char *getInstanceName() { return "spell"; }
  inline const char *getClassName() { return SqSpell::className; }
  inline SquirrelClassDecl *getClassDeclaration() { return &SqSpell::classDecl; }

  // ===========================================================================
  // Static callback methods to ScourgeGame squirrel object member functions.
  static int _squirrel_typeof( HSQUIRRELVM vm );
  static int _constructor( HSQUIRRELVM vm );

  // spell related
  static int _getName( HSQUIRRELVM vm );
  static int _getDisplayName( HSQUIRRELVM vm );
  static int _getLevel( HSQUIRRELVM vm );
  static int _getMp( HSQUIRRELVM vm );
  static int _getExp( HSQUIRRELVM vm );
  static int _getAction( HSQUIRRELVM vm );
  static int _getFailureRate( HSQUIRRELVM vm );
  static int _getDistance( HSQUIRRELVM vm );
  static int _getTargetType( HSQUIRRELVM vm );
  static int _getNotes( HSQUIRRELVM vm );
  static int _getSpeed( HSQUIRRELVM vm );
  static int _getEffect( HSQUIRRELVM vm );  
  static int _isFriendly( HSQUIRRELVM vm );
  static int _hasStateModPrereq( HSQUIRRELVM vm );
  static int _getStateModPrereq( HSQUIRRELVM vm );
  static int _isCreatureTargetAllowed( HSQUIRRELVM vm );
  static int _isLocationTargetAllowed( HSQUIRRELVM vm );
  static int _isItemTargetAllowed( HSQUIRRELVM vm );
  static int _isPartyTargetAllowed( HSQUIRRELVM vm );
	static int _isDoorTargetAllowed( HSQUIRRELVM vm );

  // magic school
  static int _getSchoolName( HSQUIRRELVM vm );
  static int _getSchoolDisplayName( HSQUIRRELVM vm );
  static int _getSchoolShortName( HSQUIRRELVM vm );
  static int _getDeity( HSQUIRRELVM vm );
  static int _getSkill( HSQUIRRELVM vm );
  static int _getResistSkill( HSQUIRRELVM vm );

};

#endif

