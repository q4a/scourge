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
  { "void", "_typeof", SqCreature::_squirrel_typeof, 1, 0, "" },
  { "void", "constructor", SqCreature::_constructor, 0, 0, "" },
  
  { "string", "getName", SqCreature::_getName, 0, 0, "" },
  { "int", "getLevel", SqCreature::_getLevel, 0, 0, "" },
  { "int", "getExpOfNextLevel", SqCreature::_getExpOfNextLevel, 0, 0, "How many experience points are needed for the character to gain the next level." },
  { "int", "getExp", SqCreature::_getExp, 0, 0, "" },
  { "int", "getMoney", SqCreature::_getMoney, 0, 0, "" },
  { "int", "getHp", SqCreature::_getHp, 0, 0, "" },
  { "int", "getStartingHp", SqCreature::_getStartingHp, 0, 0, "The hit-points the character gains when leveling up." },
  { "int", "getMaxHp", SqCreature::_getMaxHp, 0, 0, "The total amount of hit-points for this character. (If completely healed.)" },
  { "int", "getMp", SqCreature::_getMp, 0, 0, "" },
  { "int", "getStartingMp", SqCreature::_getStartingMp, 0, 0, "The magic-points this character gains when leveling up." },
  { "int", "getMaxMp", SqCreature::_getMaxMp, 0, 0, "The total amount of magic-points for this character. (If completely restored.)" },
  { "int", "getThirst", SqCreature::_getThirst, 0, 0, "A value between 0-10 indicating how thristy the character is. 0-most, 10-least thirsty." },
  { "int", "getHunger", SqCreature::_getHunger, 0, 0, "A value between 0-10 indicating how hungry the character is. 0-most, 10-least hungry." },
  { "int", "getSkill", SqCreature::_getSkill, SQ_MATCHTYPEMASKSTRING, "xn", "A 0-100 percentage value for the given skill index. See <a href=\"ScourgeGame.html\">ScourgeGame</a>.getSkillCount() and <a href=\"ScourgeGame.html\">ScourgeGame</a>.getSkillName()." },
  { "int", "getSkillByName", SqCreature::_getSkillByName, SQ_MATCHTYPEMASKSTRING, "xs", "Same as getSkill() but instead of an index, the skill is referenced by name. See <a href=\"ScourgeGame.html\">ScourgeGame</a>.getSkillCount() and <a href=\"ScourgeGame.html\">ScourgeGame</a>.getSkillName()." },
  { "int", "getStateMod", SqCreature::_getStateMod, SQ_MATCHTYPEMASKSTRING, "xn", "Returns a boolean value if the state-mod is in effect for this character. See <a href=\"ScourgeGame.html\">ScourgeGame</a>.getStateModCount() and <a href=\"ScourgeGame.html\">ScourgeGame</a>.getStateModName()." },
  { "int", "getProtectedStateMod", SqCreature::_getProtectedStateMod, SQ_MATCHTYPEMASKSTRING, "xn", "Returns a boolean value indicating if the character is protected from the given state mod. See <a href=\"ScourgeGame.html\">ScourgeGame</a>.getStateModCount() and <a href=\"ScourgeGame.html\">ScourgeGame</a>.getStateModName()." },
  { "float", "getArmor", SqCreature::_getArmor, 0, 0, "Return the armor value (sum of armor items worn modified by skills.)" },  

  { "void", "setLevel", SqCreature::_setLevel, SQ_MATCHTYPEMASKSTRING, "xn", "" },
  { "void", "setExp", SqCreature::_setExp, SQ_MATCHTYPEMASKSTRING, "xn", "" },
  { "void", "setMoney", SqCreature::_setMoney, SQ_MATCHTYPEMASKSTRING, "xn", "" },
  { "void", "setHp", SqCreature::_setHp, SQ_MATCHTYPEMASKSTRING, "xn", "" },
  { "void", "setMp", SqCreature::_setMp, SQ_MATCHTYPEMASKSTRING, "xn", "" },
  { "void", "setThirst", SqCreature::_setThirst, SQ_MATCHTYPEMASKSTRING, "xn", "" },
  { "void", "setHunger", SqCreature::_setHunger, SQ_MATCHTYPEMASKSTRING, "xn", "" },
  { "void", "setSkill", SqCreature::_setSkill, SQ_MATCHTYPEMASKSTRING, "xnn", "" },
  { "void", "setSkillByName", SqCreature::_setSkillByName, SQ_MATCHTYPEMASKSTRING, "xsn", "" },
  { "void", "setStateMod", SqCreature::_setStateMod, SQ_MATCHTYPEMASKSTRING, "xnb", "" },
  { "void", "setProtectedStateMod", SqCreature::_setProtectedStateMod, SQ_MATCHTYPEMASKSTRING, "xnb", "" },

  { "bool", "isOfClass", SqCreature::_isOfClass, SQ_MATCHTYPEMASKSTRING, "xs", "Returns a boolean if the character is of the character class given in the argument. This function is slow because it does a string compare on the class's name." },  

  { 0,0,0,0,0 } // terminator
};
SquirrelClassDecl SqCreature::classDecl = { SqCreature::className, 0, members, 
  "Information about a scourge creature (monster, player or npc.)" };

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
  GET_INT( index )
  GET_OBJECT( Creature* )
  sq_pushinteger( vm, _SC( object->getSkill( index ) ) );
  return 1;
}

int SqCreature::_getSkillByName( HSQUIRRELVM vm ) {
  GET_STRING( name, 80 )
  GET_OBJECT( Creature* )
  sq_pushinteger( vm, _SC( object->getSkill( Constants::getSkillByName( (char*)name ) ) ) );
  return 1;
}

int SqCreature::_getStateMod( HSQUIRRELVM vm ) {
  GET_INT( index )
  GET_OBJECT(Creature*)
  sq_pushinteger( vm, _SC( object->getStateMod( index ) ) );
  return 1;
}


int SqCreature::_getProtectedStateMod( HSQUIRRELVM vm ) {
  GET_INT( index )
  GET_OBJECT(Creature*)
    cerr << "FIXME: getProtectedStateMod() need index." << endl;
  sq_pushinteger( vm, _SC( object->getProtectedStateMod( index ) ) );
  return 1;
}


int SqCreature::_getArmor( HSQUIRRELVM vm ) {
  GET_OBJECT(Creature*)
  sq_pushfloat( vm, _SC( object->getACPercent() ) );
  return 1;
}

int SqCreature::_isOfClass( HSQUIRRELVM vm ) {
  GET_STRING( name, 80 )
  GET_OBJECT( Creature* )
  SQBool b = ( object->getCharacter() && 
               !strcmp( object->getCharacter()->getName(), name ) );
  sq_pushbool( vm, b );
  return 1;
}

int SqCreature::_setLevel( HSQUIRRELVM vm ) {
  GET_INT( n );
  GET_OBJECT( Creature* )
  object->setLevel( n );
  return 0;
}

int SqCreature::_setExp( HSQUIRRELVM vm ) {
  GET_INT( n );
  GET_OBJECT( Creature* )
  object->setExp( n );
  return 0;
}

int SqCreature::_setMoney( HSQUIRRELVM vm ) {
  GET_INT( n );
  GET_OBJECT( Creature* )
  object->setMoney( n );
  return 0;
}

int SqCreature::_setHp( HSQUIRRELVM vm ) {
  GET_INT( n );
  GET_OBJECT( Creature* )

  // show an effect if there is a change
  if( n > object->getHp() ) {
    object->startEffect( Constants::EFFECT_SWIRL, ( Constants::DAMAGE_DURATION * 4 ) );
  } else if( n < object->getHp() ) {
    object->startEffect( Constants::EFFECT_GLOW, ( Constants::DAMAGE_DURATION * 4 ) );
  }

  object->setHp( n );
  return 0;
}

int SqCreature::_setMp( HSQUIRRELVM vm ) {
  GET_INT( n );
  GET_OBJECT( Creature* )
  object->setMp( n );
  return 0;
}

int SqCreature::_setThirst( HSQUIRRELVM vm ) {
  GET_INT( n );
  GET_OBJECT( Creature* )
  object->setThirst( n );
  return 0;
}

int SqCreature::_setHunger( HSQUIRRELVM vm ) {
  GET_INT( n );
  GET_OBJECT( Creature* )
  object->setHunger( n );
  return 0;
}

int SqCreature::_setSkill( HSQUIRRELVM vm ) {
  GET_INT( index );
  GET_INT( n );
  GET_OBJECT( Creature* )
  object->setSkill( index, n );
  return 0;
}

int SqCreature::_setSkillByName( HSQUIRRELVM vm ) {
  GET_STRING( name, 80 );
  GET_INT( n );
  GET_OBJECT( Creature* )
  object->setSkill( Constants::getSkillByName( (char*)name ), n );
  return 0;
}

int SqCreature::_setStateMod( HSQUIRRELVM vm ) {
  GET_INT( index );
  GET_BOOL( b );
  GET_OBJECT( Creature* )
  object->setStateMod( index, b );
  return 0;
}

int SqCreature::_setProtectedStateMod( HSQUIRRELVM vm ) {
  GET_INT( index );
  GET_BOOL( b );
  GET_OBJECT( Creature* )
  object->setProtectedStateMod( index, b );
  return 0;
}

