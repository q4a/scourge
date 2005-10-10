/***************************************************************************
                          sqbinding.h  -  description
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

#ifndef SQBINDING_H
#define SQBINDING_H

#include <iostream>
#include <string>
#include <vector>
#include "../constants.h"
#include "../squirrel/squirrel.h"
#include "../squirrel/sqstdio.h"
#include "../squirrel/sqstdaux.h"
#include "consoleprinter.h"

using namespace std;

/**
 * @author Gabor Torok
 */

class Session;
class SqGame;

/**
 * A squirrel member function declaration.
 */
struct ScriptClassMemberDecl  {
  const SQChar *name;
  SQFUNCTION func;
  int params;
  const SQChar *typemask;
};

/**
 * A squirrel class declaration.
 */
struct SquirrelClassDecl  {
  const SQChar *name;
  const SQChar *base;
  const ScriptClassMemberDecl *members;
};  

/**
 * A squirrel constant declaration.
 */
struct ScriptConstantDecl  {
  const SQChar *name;
  SQObjectType type;
  union value {
    value(float v){ f = v; }
    value(int v){ i = v; }
    value(const SQChar *v){ s = v; }
    float f;
    int i;
    const SQChar *s;
  } val;
};

/**
 * A squirrel namespace declaration.
 */
struct ScriptNamespaceDecl  {
  const SQChar *name;
  const ScriptClassMemberDecl *members;
  const ScriptConstantDecl *constants;
  const ScriptClassMemberDecl *delegate;
};

/**
   Scourge object bindings to squirrel.
 */
class SqBinding {
private:
  Session *session;
  HSQUIRRELVM vm;
  
  // Squirrel object references
  HSQOBJECT refGame;

  // Native objects backing squirrel objects
  SqGame *game;

public:
  SqBinding( Session *session, ConsolePrinter *consolePrinter = NULL );
  ~SqBinding();

  static ConsolePrinter *consolePrinterRef;

  // events
  void startGame();
  void endGame();

  void loadMapScript( char *name );
  bool startLevel();
  bool endLevel();

  void compileBuffer( const char *s );

protected:
  bool compile( const char *filename );
  bool createClass( SquirrelClassDecl *cd );
  bool instantiateClass( const SQChar *classname, 
                         HSQOBJECT *obj 
                         //,SQRELEASEHOOK hook 
                         );

};

#endif

