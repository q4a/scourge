/***************************************************************************
                          sqcreature.h  -  description
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

#ifndef SQCREATURE_H
#define SQCREATURE_H

#include "sqbinding.h"
#include "sqobject.h"

/**
 * The topmost squirrel object in the Scourge object hierarchy.
 */
class SqCreature : public SqObject {
private:
  static const char *className;
  static SquirrelClassDecl classDecl;
  static ScriptClassMemberDecl members[];

public:
  SqCreature();
  ~SqCreature();

  inline const char *getInstanceName() { return "Creature"; }
  inline const char *getClassName() { return SqCreature::className; }
  inline SquirrelClassDecl *getClassDeclaration() { return &SqCreature::classDecl; }

  // ===========================================================================
  // Static callback methods to Creature squirrel object member functions.
  static int _squirrel_typeof( HSQUIRRELVM vm );
  static int _constructor( HSQUIRRELVM vm );

  // general
  static int _getName( HSQUIRRELVM vm );
//  inline Character *getCharacter() { return character; }  
//  inline Monster *getMonster() { return monster; }  
  static int _getLevel( HSQUIRRELVM vm );
  static int _getExpOfNextLevel( HSQUIRRELVM vm );
  static int _getExp( HSQUIRRELVM vm );
  static int _getMoney( HSQUIRRELVM vm );
  static int _getHp( HSQUIRRELVM vm );
  static int _getStartingHp( HSQUIRRELVM vm );
  static int _getMaxHp( HSQUIRRELVM vm );
  static int _getMp( HSQUIRRELVM vm );
  static int _getStartingMp( HSQUIRRELVM vm );
  static int _getMaxMp( HSQUIRRELVM vm );
  static int _getThirst( HSQUIRRELVM vm );
  static int _getHunger( HSQUIRRELVM vm );
  static int _getSkill( HSQUIRRELVM vm );
  static int _getSkillByName( HSQUIRRELVM vm );
  static int _getStateMod( HSQUIRRELVM vm );
  static int _getProtectedStateMod( HSQUIRRELVM vm );
  static int _getArmor( HSQUIRRELVM vm );
  static int _getSkillModifiedArmor( HSQUIRRELVM vm );

};

#endif

