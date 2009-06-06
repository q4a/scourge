/***************************************************************************
              sqcreature.h  -  Squirrel binding - Creature class
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
#pragma once

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

	inline const char *getInstanceName() {
		return "Creature";
	}
	inline const char *getClassName() {
		return SqCreature::className;
	}
	inline SquirrelClassDecl *getClassDeclaration() {
		return &SqCreature::classDecl;
	}

	// ===========================================================================
	// Static callback methods to Creature squirrel object member functions.
	static int _squirrel_typeof( HSQUIRRELVM vm );
	static int _constructor( HSQUIRRELVM vm );

	// getters
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
	static int _getSex( HSQUIRRELVM vm );
	static int _hasCapability( HSQUIRRELVM vm );
	static int _isCharacter( HSQUIRRELVM vm );
	static int _isMonster( HSQUIRRELVM vm );
	static int _isNpc( HSQUIRRELVM vm );
	static int _setNpc( HSQUIRRELVM vm );
	static int _setNpcInfo( HSQUIRRELVM vm );
	static int _setConversation( HSQUIRRELVM vm );
	static int _getMonsterType( HSQUIRRELVM vm );

	// other
	static int _isOfClass( HSQUIRRELVM vm );
	static int _isOfRootClass( HSQUIRRELVM vm );
	static int _getDeity( HSQUIRRELVM vm );
	static int _startConversation( HSQUIRRELVM vm );
	static int _startConversationAbout( HSQUIRRELVM vm );
	static int _getTargetCreature( HSQUIRRELVM vm );
	static int _getEquippedItem( HSQUIRRELVM vm );
	static int _getBackpackItem( HSQUIRRELVM vm );
	static int _getBackpackContentsCount( HSQUIRRELVM vm );

	// setters
	static int _setLevel( HSQUIRRELVM vm );
	static int _setExp( HSQUIRRELVM vm );
	static int _setMoney( HSQUIRRELVM vm );
	static int _setHp( HSQUIRRELVM vm );
	static int _takeDamage( HSQUIRRELVM vm );
	static int _setMp( HSQUIRRELVM vm );
	static int _setThirst( HSQUIRRELVM vm );
	static int _setHunger( HSQUIRRELVM vm );
	static int _setSkill( HSQUIRRELVM vm );
	static int _setSkillByName( HSQUIRRELVM vm );
	static int _setStateMod( HSQUIRRELVM vm );
	static int _setProtectedStateMod( HSQUIRRELVM vm );
	static int _setSex( HSQUIRRELVM vm );

	// npc
	static int _setIntro( HSQUIRRELVM vm );

	// other
	static int _addToBackpackByName( HSQUIRRELVM vm );

	// movement and movie mode
	static int _moveTo( HSQUIRRELVM vm );
	static int _getX( HSQUIRRELVM vm );
	static int _getY( HSQUIRRELVM vm );
	static int _setScripted( HSQUIRRELVM vm );
	static int _isScripted( HSQUIRRELVM vm );
	static int _setAnimation( HSQUIRRELVM vm );
	static int _setVisible( HSQUIRRELVM vm );
	static int _isVisible( HSQUIRRELVM vm );
	static int _say( HSQUIRRELVM vm );
	static int _clearSpeech( HSQUIRRELVM vm );
	static int _setOffset( HSQUIRRELVM vm );
	
	// combat
	static int _castSpell( HSQUIRRELVM vm );
	static int _getSummoner( HSQUIRRELVM vm );
	static int _summon( HSQUIRRELVM vm );
	static int _remove( HSQUIRRELVM vm );
	
protected:	
	static void push_creature( HSQUIRRELVM vm, Creature *summoner );

};
#endif

