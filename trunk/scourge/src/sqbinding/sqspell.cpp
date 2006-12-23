/***************************************************************************
                          sqspell.cpp  -  description
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
#include "sqspell.h"
#include "../session.h"
#include "../rpg/rpglib.h"

using namespace std;

const char *SqSpell::className = "Spell";
ScriptClassMemberDecl SqSpell::members[] = {
  { "void", "_typeof", SqSpell::_squirrel_typeof, 1, 0, "" },
  { "void", "constructor", SqSpell::_constructor, 0, 0, "" },
  { "string", "getName", SqSpell::_getName, 0, 0, "Get the name of the spell." },
  { "int", "getLevel", SqSpell::_getLevel, 0, 0, "Get the spell's level (min level of caster)." },
  { "int", "getMp", SqSpell::_getMp, 0, 0, "Get the MP used up when casting this spell." },
  { "int", "getExp", SqSpell::_getMp, 0, 0, "Get the experience gained when casting this spell." },
  { "int", "getAction", SqSpell::_getAction, 0, 0, "Get the damage (or other action value) this spell causes." },
  { "int", "getFailureRate", SqSpell::_getFailureRate, 0, 0, "Get the failure rate (percentage) when casting this spell." },
  { "int", "getDistance", SqSpell::_getDistance, 0, 0, "Get the range of this spell." },
  { "int", "getTargetType", SqSpell::_getTargetType, 0, 0, "" },
  { "string", "getNotes", SqSpell::_getNotes, 0, 0, "Describe this spell." },
  { "int", "getSpeed", SqSpell::_getSpeed, 0, 0, "Get the speed of this spell." },
  { "int", "getEffect", SqSpell::_getEffect, 0, 0, "Get the effect type displayed when the spell is cast." },
  { "bool", "isFriendly", SqSpell::_isFriendly, 0, 0, "Is this spell considered to have good effects (towards party when caster is player, other monster when cast by monsters.)" },
  { "bool", "hasStateModPrereq", SqSpell::_hasStateModPrereq, 0, 0, "When considering to cast this spell, is there a prerequisite (monsters)." },
  { "int", "getStateModPrereq", SqSpell::_getStateModPrereq, 0, 0, "What is the prereq to a monster casting a friendly spell? (e.g.: low hp)" },
  { "bool", "isCreatureTargetAllowed", SqSpell::_isCreatureTargetAllowed, 0, 0, "Can the spell target another creature?" },
  { "bool", "isItemTargetAllowed", SqSpell::_isItemTargetAllowed, 0, 0, "Can the spell target an item?" },
  { "bool", "isLocationTargetAllowed", SqSpell::_isLocationTargetAllowed, 0, 0, "Can the spell target a location?" },
  { "bool", "isPartyTargetAllowed", SqSpell::_isPartyTargetAllowed, 0, 0, "Can the spell target the party?" },
	{ "bool", "isDoorTargetAllowed", SqSpell::_isDoorTargetAllowed, 0, 0, "Can the spell target a door?" },
  { "string", "getSchoolName", SqSpell::_getSchoolName, 0, 0, "Get the spell's magic school's name." },
  { "string", "getSchoolShortName", SqSpell::_getSchoolShortName, 0, 0, "Get the spell's magic school's short name." },
  { "string", "getDeity", SqSpell::_getDeity, 0, 0, "Get the spell's magic school's patron deity's name." },
  { "int", "getSkill", SqSpell::_getSkill, 0, 0, "Get the skill checked when using this spell." },
  { "int", "getResistSkill", SqSpell::_getResistSkill, 0, 0, "Get the skill checked when resisting this spell." },
  { 0,0,0,0,0 } // terminator
};
SquirrelClassDecl SqSpell::classDecl = { SqSpell::className, 0, members,
  "A scourge magic spell and its magic school." };

SqSpell::SqSpell() {
}

SqSpell::~SqSpell() {
}

// ===========================================================================
// Static callback methods to ScourgeGame squirrel object member functions.
int SqSpell::_squirrel_typeof( HSQUIRRELVM vm ) {
  sq_pushstring( vm, SqSpell::className, -1 );
  return 1; // 1 value is returned
}

int SqSpell::_constructor( HSQUIRRELVM vm ) {
  return 0; // no values returned
}

int SqSpell::_getName( HSQUIRRELVM vm ) {
  GET_OBJECT(Spell*)
  sq_pushstring( vm, _SC( object->getName() ), -1 );
  return 1;
}

int SqSpell::_getLevel( HSQUIRRELVM vm ) {
  GET_OBJECT(Spell*)
  sq_pushinteger( vm, object->getLevel() );
  return 1;
}

int SqSpell::_getMp( HSQUIRRELVM vm ) {
  GET_OBJECT(Spell*)
  sq_pushinteger( vm, object->getMp() );
  return 1;
}

int SqSpell::_getExp( HSQUIRRELVM vm ) {
  GET_OBJECT(Spell*)
  sq_pushinteger( vm, object->getExp() );
  return 1;
}

int SqSpell::_getAction( HSQUIRRELVM vm ) {
  GET_OBJECT(Spell*)
  sq_pushinteger( vm, object->getAction() );
  return 1;
}

int SqSpell::_getFailureRate( HSQUIRRELVM vm ) {
  GET_OBJECT(Spell*)
  sq_pushinteger( vm, object->getFailureRate() );
  return 1;
}

int SqSpell::_getDistance( HSQUIRRELVM vm ) {
  GET_OBJECT(Spell*)
  sq_pushinteger( vm, object->getDistance() );
  return 1;
}

int SqSpell::_getTargetType( HSQUIRRELVM vm ) {
  GET_OBJECT(Spell*)
  sq_pushinteger( vm, object->getTargetType() );
  return 1;
}

int SqSpell::_getNotes( HSQUIRRELVM vm ) {
  GET_OBJECT(Spell*)
  sq_pushstring( vm, _SC( object->getNotes() ), -1 );
  return 1;
}

int SqSpell::_getSpeed( HSQUIRRELVM vm ) {
  GET_OBJECT(Spell*)
  sq_pushinteger( vm, object->getSpeed() );
  return 1;
}

int SqSpell::_getEffect( HSQUIRRELVM vm ) {
  GET_OBJECT(Spell*)
  sq_pushinteger( vm, object->getEffect() );
  return 1;
}

int SqSpell::_isFriendly( HSQUIRRELVM vm ) {
  GET_OBJECT(Spell*)
  sq_pushbool( vm, object->isFriendly() );
  return 1;
}

int SqSpell::_getStateModPrereq( HSQUIRRELVM vm ) {
  GET_OBJECT(Spell*)
  sq_pushinteger( vm, object->getStateModPrereq() );
  return 1;
}

int SqSpell::_hasStateModPrereq( HSQUIRRELVM vm ) {
  GET_OBJECT(Spell*)
  sq_pushbool( vm, object->hasStateModPrereq() );
  return 1;
}

int SqSpell::_isCreatureTargetAllowed( HSQUIRRELVM vm ) {
  GET_OBJECT(Spell*)
  sq_pushbool( vm, object->isCreatureTargetAllowed() );
  return 1;
}

int SqSpell::_isLocationTargetAllowed( HSQUIRRELVM vm ) {
  GET_OBJECT(Spell*)
  sq_pushbool( vm, object->isLocationTargetAllowed() );
  return 1;
}

int SqSpell::_isItemTargetAllowed( HSQUIRRELVM vm ) {
  GET_OBJECT(Spell*)
  sq_pushbool( vm, object->isItemTargetAllowed() );
  return 1;
}

int SqSpell::_isPartyTargetAllowed( HSQUIRRELVM vm ) {
  GET_OBJECT(Spell*)
  sq_pushbool( vm, object->isPartyTargetAllowed() );
  return 1;
}

int SqSpell::_isDoorTargetAllowed( HSQUIRRELVM vm ) {
  GET_OBJECT(Spell*)
  sq_pushbool( vm, object->isDoorTargetAllowed() );
  return 1;
}

  // magic school
int SqSpell::_getSchoolName( HSQUIRRELVM vm ) {
  GET_OBJECT(Spell*)
  sq_pushstring( vm, _SC( object->getSchool()->getName() ), -1 );
  return 1;
}

int SqSpell::_getSchoolShortName( HSQUIRRELVM vm ) {
  GET_OBJECT(Spell*)
  sq_pushstring( vm, _SC( object->getSchool()->getShortName() ), -1 );
  return 1;
}

int SqSpell::_getDeity( HSQUIRRELVM vm ) {
  GET_OBJECT(Spell*)
  sq_pushstring( vm, _SC( object->getSchool()->getDeity() ), -1 );
  return 1;
}

int SqSpell::_getSkill( HSQUIRRELVM vm ) {
  GET_OBJECT(Spell*)
  sq_pushinteger( vm, object->getSchool()->getSkill() );
  return 1;
}

int SqSpell::_getResistSkill( HSQUIRRELVM vm ) {
  GET_OBJECT(Spell*)
  sq_pushinteger( vm, object->getSchool()->getResistSkill() );
  return 1;
}

