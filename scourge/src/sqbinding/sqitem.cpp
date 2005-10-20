/***************************************************************************
                          sqitem.cpp  -  description
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
#include "sqitem.h"
#include "../session.h"
#include "../item.h"
#include "../rpg/rpglib.h"

using namespace std;

const char *SqItem::className = "Item";
ScriptClassMemberDecl SqItem::members[] = {
  { "_typeof", SqItem::_squirrel_typeof, 1, 0 },
  { "constructor", SqItem::_constructor, 0, 0 },
  { "getName", SqItem::_getName, 0, 0 },
  { "getLevel", SqItem::_getLevel, 0, 0 },
  { "getWeight", SqItem::_getWeight, 0, 0 },
  { "getPrice", SqItem::_getPrice, 0, 0 },
  { "getAction", SqItem::_getAction, 0, 0 },
  { "getSpeed", SqItem::_getAction, 0, 0 },
  { "getDistance", SqItem::_getDistance, 0, 0 },
  { "getMaxCharges", SqItem::_getMaxCharges, 0, 0 },
  { "getDuration", SqItem::_getDuration, 0, 0 },
  { "getQuality", SqItem::_getQuality, 0, 0 },
  { "isMagicItem", SqItem::_isMagicItem, 0, 0 },
  { "getSkillBonus", SqItem::_getSkillBonus, 0, 0 },
  { "getMagicLevel", SqItem::_getMagicLevel, 0, 0 },
  { "getBonus", SqItem::_getBonus, 0, 0 },
  { "getDamageMultiplier", SqItem::_getDamageMultiplier, 0, 0 },
  { "getMonsterType", SqItem::_getMonsterType, 0, 0 },
  { "getSchool", SqItem::_getSchool, 0, 0 },
  { "getMagicResistance", SqItem::_getMagicResistance, 0, 0 },
  { "describeMagicDamage", SqItem::_describeMagicDamage, 0, 0 },
  { "isCursed", SqItem::_isCursed, 0, 0 },
  { "isStateModSet", SqItem::_isStateModSet, 0, 0 },
  { "isStateModProtected", SqItem::_isStateModProtected, 0, 0 },
  { 0,0,0,0 } // terminator
};
SquirrelClassDecl SqItem::classDecl = { SqItem::className, 0, members };

SqItem::SqItem() {
}

SqItem::~SqItem() {
}

// ===========================================================================
// Static callback methods to ScourgeGame squirrel object member functions.
int SqItem::_squirrel_typeof( HSQUIRRELVM vm ) {
  sq_pushstring( vm, SqItem::className, -1 );
  return 1; // 1 value is returned
}

int SqItem::_constructor( HSQUIRRELVM vm ) {
  cerr << "in " << SqItem::className << " constructor." << endl;
  return 0; // no values returned
}

// ===========================================================================
// Member methods
int SqItem::_getName( HSQUIRRELVM vm ) {
  GET_OBJECT(Item*)
  sq_pushstring( vm, _SC( object->getRpgItem()->getName() ), -1 );
  return 1;
}

int SqItem::_getLevel( HSQUIRRELVM vm ) {
  GET_OBJECT(Item*)
  sq_pushinteger( vm, object->getLevel() );
  return 1;
}

int SqItem::_getWeight( HSQUIRRELVM vm ) {
  GET_OBJECT(Item*)
  sq_pushfloat( vm, object->getWeight() );
  return 1;
}

int SqItem::_getPrice( HSQUIRRELVM vm ) {
  GET_OBJECT(Item*)
  sq_pushinteger( vm, object->getPrice() );
  return 1;
}

int SqItem::_getAction( HSQUIRRELVM vm ) {
  GET_OBJECT(Item*)
  sq_pushinteger( vm, object->getAction() );
  return 1;
}

int SqItem::_getSpeed( HSQUIRRELVM vm ) {
  GET_OBJECT(Item*)
  sq_pushinteger( vm, object->getSpeed() );
  return 1;
}

int SqItem::_getDistance( HSQUIRRELVM vm ) {
  GET_OBJECT(Item*)
  sq_pushinteger( vm, object->getDistance() );
  return 1;
}

int SqItem::_getMaxCharges( HSQUIRRELVM vm ) {
  GET_OBJECT(Item*)
  sq_pushinteger( vm, object->getMaxCharges() );
  return 1;
}

int SqItem::_getDuration( HSQUIRRELVM vm ) {
  GET_OBJECT(Item*)
  sq_pushinteger( vm, object->getDuration() );
  return 1;
}

int SqItem::_getQuality( HSQUIRRELVM vm ) {
  GET_OBJECT(Item*)
  sq_pushinteger( vm, object->getQuality() );
  return 1;
}

int SqItem::_isMagicItem( HSQUIRRELVM vm ) {
  GET_OBJECT(Item*)
  sq_pushbool( vm, object->isMagicItem() );
  return 1;
}

int SqItem::_getSkillBonus( HSQUIRRELVM vm ) {
  GET_OBJECT(Item*)
  cerr << "FIXME: getSkillBonus() need index." << endl;
  sq_pushinteger( vm, object->getSkillBonus( 1 ) );
  return 1;
}

int SqItem::_getMagicLevel( HSQUIRRELVM vm ) {
  GET_OBJECT(Item*)
  sq_pushinteger( vm, object->getMagicLevel() );
  return 1;
}

int SqItem::_getBonus( HSQUIRRELVM vm ) {
  GET_OBJECT(Item*)
  sq_pushinteger( vm, object->getBonus() );
  return 1;
}

int SqItem::_getDamageMultiplier( HSQUIRRELVM vm ) {
  GET_OBJECT(Item*)
  sq_pushinteger( vm, object->getDamageMultiplier() );
  return 1;
}

int SqItem::_getMonsterType( HSQUIRRELVM vm ) {
  GET_OBJECT(Item*)
  sq_pushstring( vm, _SC( object->getMonsterType() ), -1 );
  return 1;
}

int SqItem::_getSchool( HSQUIRRELVM vm ) {
  return sq_throwerror( vm, _SC( "FIXME: implement MagicSchool class." ) );
}

int SqItem::_getMagicResistance( HSQUIRRELVM vm ) {
  GET_OBJECT(Item*)
  sq_pushinteger( vm, object->getMagicResistance() );
  return 1;
}

int SqItem::_describeMagicDamage( HSQUIRRELVM vm ) {
  GET_OBJECT(Item*)
  sq_pushstring( vm, _SC( object->describeMagicDamage() ), -1 );
  return 1;
}

int SqItem::_isCursed( HSQUIRRELVM vm ) {
  GET_OBJECT(Item*)
  sq_pushbool( vm, object->isCursed() );
  return 1;
}

int SqItem::_isStateModSet( HSQUIRRELVM vm ) {
  GET_OBJECT(Item*)
  cerr << "FIXME: isStateModSet() need index." << endl;
  sq_pushbool( vm, object->isStateModSet( 1 ) );
  return 1;
}

int SqItem::_isStateModProtected( HSQUIRRELVM vm ) {
  GET_OBJECT(Item*)
  cerr << "FIXME: isStateModProtected() need index." << endl;
  sq_pushbool( vm, object->isStateModProtected( 1 ) );
  return 1;
}

