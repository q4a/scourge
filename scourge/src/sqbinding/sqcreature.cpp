/***************************************************************************
                          sqcreature.cpp  -  description
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
#include "sqcreature.h"
#include "../session.h"
#include "../creature.h"
#include "../rpg/rpglib.h"

using namespace std;

const char *SqCreature::className = "Creature";
ScriptClassMemberDecl SqCreature::members[] = {
  { "_typeof", SqCreature::_squirrel_typeof, 1, 0 },
  { "constructor", SqCreature::_constructor, 0, 0 },
  
  { "getName", SqCreature::_getName, 0, 0 },
  { "getLevel", SqCreature::_getLevel, 0, 0 },
  { "getExpOfNextLevel", SqCreature::_getExpOfNextLevel, 0, 0 },
  { "getExp", SqCreature::_getExp, 0, 0 },
  { "getMoney", SqCreature::_getMoney, 0, 0 },
  { "getHp", SqCreature::_getHp, 0, 0 },
  { "getStartingHp", SqCreature::_getStartingHp, 0, 0 },
  { "getMaxHp", SqCreature::_getMaxHp, 0, 0 },
  { "getMp", SqCreature::_getMp, 0, 0 },
  { "getStartingMp", SqCreature::_getStartingMp, 0, 0 },
  { "getMaxMp", SqCreature::_getMaxMp, 0, 0 },
  { "getThirst", SqCreature::_getThirst, 0, 0 },
  { "getHunger", SqCreature::_getHunger, 0, 0 },
  { "getSkill", SqCreature::_getSkill, SQ_MATCHTYPEMASKSTRING, "xn" },
  { "getSkillByName", SqCreature::_getSkillByName, SQ_MATCHTYPEMASKSTRING, "xs" },
  { "getStateMod", SqCreature::_getStateMod, SQ_MATCHTYPEMASKSTRING, "xn" },
  { "getProtectedStateMod", SqCreature::_getProtectedStateMod, SQ_MATCHTYPEMASKSTRING, "xn" },
  { "getArmor", SqCreature::_getArmor, 0, 0 },
  { "getSkillModifiedArmor", SqCreature::_getSkillModifiedArmor, 0, 0 },

  { "setLevel", SqCreature::_setLevel, SQ_MATCHTYPEMASKSTRING, "xn" },
  { "setExp", SqCreature::_setExp, SQ_MATCHTYPEMASKSTRING, "xn" },
  { "setMoney", SqCreature::_setMoney, SQ_MATCHTYPEMASKSTRING, "xn" },
  { "setHp", SqCreature::_setHp, SQ_MATCHTYPEMASKSTRING, "xn" },
  { "setMp", SqCreature::_setMp, SQ_MATCHTYPEMASKSTRING, "xn" },
  { "setThirst", SqCreature::_setThirst, SQ_MATCHTYPEMASKSTRING, "xn" },
  { "setHunger", SqCreature::_setHunger, SQ_MATCHTYPEMASKSTRING, "xn" },
  { "setSkill", SqCreature::_setSkill, SQ_MATCHTYPEMASKSTRING, "xnn" },
  { "setSkillByName", SqCreature::_setSkillByName, SQ_MATCHTYPEMASKSTRING, "xsn" },
  { "setStateMod", SqCreature::_setStateMod, SQ_MATCHTYPEMASKSTRING, "xnb" },
  { "setProtectedStateMod", SqCreature::_setProtectedStateMod, SQ_MATCHTYPEMASKSTRING, "xnb" },

  { "isOfClass", SqCreature::_isOfClass, SQ_MATCHTYPEMASKSTRING, "xs" },  

  { 0,0,0,0 } // terminator
};
SquirrelClassDecl SqCreature::classDecl = { SqCreature::className, 0, members };

SqCreature::SqCreature() {
}

SqCreature::~SqCreature() {
}

// ===========================================================================
// Static callback methods to ScourgeGame squirrel object member functions.
int SqCreature::_squirrel_typeof( HSQUIRRELVM vm ) {
  sq_pushstring( vm, SqCreature::className, -1 );
  return 1; // 1 value is returned
}

int SqCreature::_constructor( HSQUIRRELVM vm ) {
  cerr << "in " << SqCreature::className << " constructor." << endl;
  return 0; // no values returned
}

// ===========================================================================
// Member methods
int SqCreature::_getName( HSQUIRRELVM vm ) {
  GET_OBJECT(Creature*)
  sq_pushstring( vm, _SC( object->getName() ), -1 );
  return 1;
}

int SqCreature::_getLevel( HSQUIRRELVM vm ) {
  GET_OBJECT(Creature*)
  sq_pushinteger( vm, _SC( object->getLevel() ) );
  return 1;
}

int SqCreature::_getExpOfNextLevel( HSQUIRRELVM vm ) {
  GET_OBJECT(Creature*)
  sq_pushinteger( vm, _SC( object->getExpOfNextLevel() ) );
  return 1;
}

int SqCreature::_getExp( HSQUIRRELVM vm ) {
  GET_OBJECT(Creature*)
  sq_pushinteger( vm, _SC( object->getExp() ) );
  return 1;
}

int SqCreature::_getMoney( HSQUIRRELVM vm ) {
  GET_OBJECT(Creature*)
  sq_pushinteger( vm, _SC( object->getMoney() ) );
  return 1;
}

int SqCreature::_getHp( HSQUIRRELVM vm ) {
  GET_OBJECT(Creature*)
  sq_pushinteger( vm, _SC( object->getHp() ) );
  return 1;
}

int SqCreature::_getStartingHp( HSQUIRRELVM vm ) {
  GET_OBJECT(Creature*)
  sq_pushinteger( vm, _SC( object->getStartingHp() ) );
  return 1;
}

int SqCreature::_getMaxHp( HSQUIRRELVM vm ) {
  GET_OBJECT(Creature*)
  sq_pushinteger( vm, _SC( object->getMaxHp() ) );
  return 1;
}

int SqCreature::_getMp( HSQUIRRELVM vm ) {
  GET_OBJECT(Creature*)
  sq_pushinteger( vm, _SC( object->getMp() ) );
  return 1;
}

int SqCreature::_getStartingMp( HSQUIRRELVM vm ) {
  GET_OBJECT(Creature*)
  sq_pushinteger( vm, _SC( object->getStartingMp() ) );
  return 1;
}

int SqCreature::_getMaxMp( HSQUIRRELVM vm ) {
  GET_OBJECT(Creature*)
  sq_pushinteger( vm, _SC( object->getMaxMp() ) );
  return 1;
}

int SqCreature::_getThirst( HSQUIRRELVM vm ) {
  GET_OBJECT(Creature*)
  sq_pushinteger( vm, _SC( object->getThirst() ) );
  return 1;
}

int SqCreature::_getHunger( HSQUIRRELVM vm ) {
  GET_OBJECT(Creature*)
  sq_pushinteger( vm, _SC( object->getHunger() ) );
  return 1;
}

int SqCreature::_getSkill( HSQUIRRELVM vm ) {
//  SqBinding::printArgs( vm );
  int index;
  if( SQ_FAILED( sq_getinteger( vm, -1, &index ) ) ) {
    return sq_throwerror( vm, _SC( "Can't get index from stack in _getSkill." ) );
  }
  sq_poptop( vm );
  GET_OBJECT(Creature*)
  sq_pushinteger( vm, _SC( object->getSkill( index ) ) );
  return 1;
}

int SqCreature::_getSkillByName( HSQUIRRELVM vm ) {
//  SqBinding::printArgs( vm );
  const char *tmp;
  if( SQ_FAILED( sq_getstring( vm, -1, &tmp ) ) ) {
    return sq_throwerror( vm, _SC( "Can't get name from stack in _getSkillByName." ) );
  }
  char name[80];
  strcpy( name, tmp );
  sq_poptop( vm );
  GET_OBJECT(Creature*)
  sq_pushinteger( vm, _SC( object->getSkill( Constants::getSkillByName( (char*)name ) ) ) );
  return 1;
}

int SqCreature::_getStateMod( HSQUIRRELVM vm ) {
  int index;
  if( SQ_FAILED( sq_getinteger( vm, -1, &index ) ) ) {
    return sq_throwerror( vm, _SC( "Can't get index from stack in _getStateMod." ) );
  }
  sq_poptop( vm );
  GET_OBJECT(Creature*)
  sq_pushinteger( vm, _SC( object->getStateMod( index ) ) );
  return 1;
}


int SqCreature::_getProtectedStateMod( HSQUIRRELVM vm ) {
  int index;
  if( SQ_FAILED( sq_getinteger( vm, -1, &index ) ) ) {
    return sq_throwerror( vm, _SC( "Can't get index from stack in _getProtectedStateMod." ) );
  }
  sq_poptop( vm );
  GET_OBJECT(Creature*)
    cerr << "FIXME: getProtectedStateMod() need index." << endl;
  sq_pushinteger( vm, _SC( object->getProtectedStateMod( index ) ) );
  return 1;
}


int SqCreature::_getArmor( HSQUIRRELVM vm ) {
  GET_OBJECT(Creature*)
  sq_pushinteger( vm, _SC( object->getStateMod( 0 ) ) );
  return 1;
}

int SqCreature::_getSkillModifiedArmor( HSQUIRRELVM vm ) {
  GET_OBJECT(Creature*)
  sq_pushinteger( vm, _SC( object->getSkillModifiedArmor() ) );
  return 1;
}

int SqCreature::_isOfClass( HSQUIRRELVM vm ) {
  GET_STRING(name)
  GET_OBJECT(Creature*)
  SQBool b = ( object->getCharacter() && 
               !strcmp( object->getCharacter()->getName(), name ) );
  sq_pushbool( vm, b );
  free( name );
  return 1;
}

int SqCreature::_setLevel( HSQUIRRELVM vm ) {
  return 0;
}

int SqCreature::_setExp( HSQUIRRELVM vm ) {
  return 0;
}

int SqCreature::_setMoney( HSQUIRRELVM vm ) {
  return 0;
}

int SqCreature::_setHp( HSQUIRRELVM vm ) {
  return 0;
}

int SqCreature::_setMp( HSQUIRRELVM vm ) {
  return 0;
}

int SqCreature::_setThirst( HSQUIRRELVM vm ) {
  return 0;
}

int SqCreature::_setHunger( HSQUIRRELVM vm ) {
  return 0;
}

int SqCreature::_setSkill( HSQUIRRELVM vm ) {
  return 0;
}

int SqCreature::_setSkillByName( HSQUIRRELVM vm ) {
  return 0;
}

int SqCreature::_setStateMod( HSQUIRRELVM vm ) {
  return 0;
}

int SqCreature::_setProtectedStateMod( HSQUIRRELVM vm ) {
  return 0;
}

