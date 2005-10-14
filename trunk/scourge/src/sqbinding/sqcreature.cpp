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

using namespace std;

#define GET_CREATURE   SQUserPointer up;\
  if( !SqBinding::getObjectValue( vm, CREATURE_ID_TOKEN, &up ) ) {\
    return sq_throwerror( vm, _SC( "Can't find userpointer." ) );\
  }\
  Creature *creature = (Creature*)up;

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
  { "getSkill", SqCreature::_getSkill, 0, 0 },
  { "getStateMod", SqCreature::_getStateMod, 0, 0 },
  { "getProtectedStateMod", SqCreature::_getProtectedStateMod, 0, 0 },
  { "getArmor", SqCreature::_getArmor, 0, 0 },
  { "getSkillModifiedArmor", SqCreature::_getSkillModifiedArmor, 0, 0 },
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
  GET_CREATURE
  sq_pushstring( vm, _SC( ((Creature*)creature)->getName() ), -1 );
  return 1;
}

int SqCreature::_getLevel( HSQUIRRELVM vm ) {
  GET_CREATURE
  sq_pushinteger( vm, _SC( ((Creature*)creature)->getLevel() ) );
  return 1;
}

int SqCreature::_getExpOfNextLevel( HSQUIRRELVM vm ) {
  GET_CREATURE
  sq_pushinteger( vm, _SC( ((Creature*)creature)->getExpOfNextLevel() ) );
  return 1;
}

int SqCreature::_getExp( HSQUIRRELVM vm ) {
  GET_CREATURE
  sq_pushinteger( vm, _SC( ((Creature*)creature)->getExp() ) );
  return 1;
}

int SqCreature::_getMoney( HSQUIRRELVM vm ) {
  GET_CREATURE
  sq_pushinteger( vm, _SC( ((Creature*)creature)->getMoney() ) );
  return 1;
}

int SqCreature::_getHp( HSQUIRRELVM vm ) {
  GET_CREATURE
  sq_pushinteger( vm, _SC( ((Creature*)creature)->getHp() ) );
  return 1;
}

int SqCreature::_getStartingHp( HSQUIRRELVM vm ) {
  GET_CREATURE
  sq_pushinteger( vm, _SC( ((Creature*)creature)->getStartingHp() ) );
  return 1;
}

int SqCreature::_getMaxHp( HSQUIRRELVM vm ) {
  GET_CREATURE
  sq_pushinteger( vm, _SC( ((Creature*)creature)->getMaxHp() ) );
  return 1;
}

int SqCreature::_getMp( HSQUIRRELVM vm ) {
  GET_CREATURE
  sq_pushinteger( vm, _SC( ((Creature*)creature)->getMp() ) );
  return 1;
}

int SqCreature::_getStartingMp( HSQUIRRELVM vm ) {
  GET_CREATURE
  sq_pushinteger( vm, _SC( ((Creature*)creature)->getStartingMp() ) );
  return 1;
}

int SqCreature::_getMaxMp( HSQUIRRELVM vm ) {
  GET_CREATURE
  sq_pushinteger( vm, _SC( ((Creature*)creature)->getMaxMp() ) );
  return 1;
}

int SqCreature::_getThirst( HSQUIRRELVM vm ) {
  GET_CREATURE
  sq_pushinteger( vm, _SC( ((Creature*)creature)->getThirst() ) );
  return 1;
}

int SqCreature::_getHunger( HSQUIRRELVM vm ) {
  GET_CREATURE
  sq_pushinteger( vm, _SC( ((Creature*)creature)->getHunger() ) );
  return 1;
}

int SqCreature::_getSkill( HSQUIRRELVM vm ) {
  GET_CREATURE
    cerr << "FIXME: getSkill() need index." << endl;
  sq_pushinteger( vm, _SC( ((Creature*)creature)->getSkill( 0 ) ) );
  return 1;
}

int SqCreature::_getStateMod( HSQUIRRELVM vm ) {
  GET_CREATURE
    cerr << "FIXME: getStateMod() need index." << endl;
  sq_pushinteger( vm, _SC( ((Creature*)creature)->getStateMod( 0 ) ) );
  return 1;
}


int SqCreature::_getProtectedStateMod( HSQUIRRELVM vm ) {
  GET_CREATURE
    cerr << "FIXME: getProtectedStateMod() need index." << endl;
  sq_pushinteger( vm, _SC( ((Creature*)creature)->getProtectedStateMod( 0 ) ) );
  return 1;
}


int SqCreature::_getArmor( HSQUIRRELVM vm ) {
  GET_CREATURE
  sq_pushinteger( vm, _SC( ((Creature*)creature)->getStateMod( 0 ) ) );
  return 1;
}

int SqCreature::_getSkillModifiedArmor( HSQUIRRELVM vm ) {
  GET_CREATURE
  sq_pushinteger( vm, _SC( ((Creature*)creature)->getSkillModifiedArmor() ) );
  return 1;
}
