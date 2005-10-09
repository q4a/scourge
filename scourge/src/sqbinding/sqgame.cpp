/***************************************************************************
                          sqgame.cpp  -  description
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
#include "sqgame.h"

using namespace std;

const char *SqGame::className = "ScourgeGame";
ScriptClassMemberDecl SqGame::members[] = {
  { "_typeof", SqGame::_game_typeof, 1, 0 },
  { "constructor", SqGame::_constructor, 0, 0 },
  { "doSomething", SqGame::_doSomething, 0, 0 },
  { 0,0,0,0 } // terminator
};
SquirrelClassDecl SqGame::classDecl = { SqGame::className, 0, members };

SqGame::SqGame() {
}

SqGame::~SqGame() {
}

// ===========================================================================
// Static callback methods to ScourgeGame squirrel object member functions.
int SqGame::_game_typeof( HSQUIRRELVM vm ) {
  sq_pushstring( vm, SqGame::className, -1 );
  return 1; // 1 value is returned
}

int SqGame::_constructor( HSQUIRRELVM vm ) {
  cerr << "in " << SqGame::className << " constructor." << endl;
  return 0; // no values returned
}

int SqGame::_doSomething( HSQUIRRELVM v) {
  cerr << "in " << SqGame::className << "::doSomething()" << endl;
  return 0; // no values returned
}

