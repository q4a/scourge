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

