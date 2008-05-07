/***************************************************************************
                          sqitem.h  -  description
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

#ifndef SQITEM_H
#define SQITEM_H

#include "sqbinding.h"
#include "sqobject.h"

class SqItem : public SqObject {
private:
  static const char *className;
  static SquirrelClassDecl classDecl;
  static ScriptClassMemberDecl members[];

public:
  SqItem();
  ~SqItem();

  inline const char *getInstanceName() { return "Item"; }
  inline const char *getClassName() { return SqItem::className; }
  inline SquirrelClassDecl *getClassDeclaration() { return &SqItem::classDecl; }

  // ===========================================================================
  // Static callback methods to Creature squirrel object member functions.
  static int _squirrel_typeof( HSQUIRRELVM vm );
  static int _constructor( HSQUIRRELVM vm );

  // general
  static int _getName( HSQUIRRELVM vm );
	static int _getDisplayName( HSQUIRRELVM vm );
  static int _getLevel( HSQUIRRELVM vm );
  static int _getWeight( HSQUIRRELVM vm );
  static int _getPrice( HSQUIRRELVM vm );
  static int _getQuality( HSQUIRRELVM vm );
  static int _isMagicItem( HSQUIRRELVM vm );
  static int _getSkillBonus( HSQUIRRELVM vm );
  static int _getMagicLevel( HSQUIRRELVM vm );
  static int _getBonus( HSQUIRRELVM vm );
  static int _getDamageMultiplier( HSQUIRRELVM vm );
  static int _getMonsterType( HSQUIRRELVM vm );
  static int _getSchool( HSQUIRRELVM vm );
  static int _getMagicResistance( HSQUIRRELVM vm );
  static int _describeMagicDamage( HSQUIRRELVM vm );
  static int _isCursed( HSQUIRRELVM vm );
  static int _isStateModSet( HSQUIRRELVM vm );
  static int _isStateModProtected( HSQUIRRELVM vm );
  static int _isRanged( HSQUIRRELVM vm );
	static int _getRange( HSQUIRRELVM vm );

	// from rpgitem

	// weapon
  static int _getDamage( HSQUIRRELVM vm );
  static int _getMaxCharges( HSQUIRRELVM vm );	
	static int _getDamageSkill( HSQUIRRELVM vm );
	static int _getDamageType( HSQUIRRELVM vm );
  static int _getParry( HSQUIRRELVM vm );
	static int _getAP( HSQUIRRELVM vm );
  static int _getTwoHanded( HSQUIRRELVM vm );
  static int _hasTag( HSQUIRRELVM vm );

	// armor
	static int _getDefense( HSQUIRRELVM vm );
	static int _getDefenseSkill( HSQUIRRELVM vm );
	static int _getDodgePenalty( HSQUIRRELVM vm );

	// potion
	static int _getPotionPower( HSQUIRRELVM vm );
	static int _getPotionSkill( HSQUIRRELVM vm );
	static int _getPotionTime( HSQUIRRELVM vm );

	// spell
	static int _getSpellLevel( HSQUIRRELVM vm );
};

#endif

