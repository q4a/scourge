/***************************************************************************
                          sqgame.h  -  description
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

#ifndef SQGAME_H
#define SQGAME_H

#include "sqbinding.h"
#include "sqobject.h"

using namespace std;

/**
 * The topmost squirrel object in the Scourge object hierarchy.
 */
class SqGame : public SqObject {
private:
  static const char *className;
  static SquirrelClassDecl classDecl;
  static ScriptClassMemberDecl members[];

public:
  SqGame();
  ~SqGame();

  inline const char *getClassName() { return SqGame::className; }
  inline SquirrelClassDecl *getClassDeclaration() { return &SqGame::classDecl; }

  // ===========================================================================
  // Static callback methods to ScourgeGame squirrel object member functions.
  static int _game_typeof( HSQUIRRELVM vm );
  static int _constructor( HSQUIRRELVM vm );
  static int _doSomething( HSQUIRRELVM v);
};

#endif

