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

/**
 * @author Gabor Torok
 */

class Session;
class SqGame;
class SqCreature;
class SqMission;
class SqItem;
class Creature;
class Item;

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

#define DEBUG_SQUIRREL 1
#define SCOURGE_ID_TOKEN "scourge_id"

#define GET_OBJECT(x)   SQUserPointer up;\
  if( !SqBinding::getObjectValue( vm, SCOURGE_ID_TOKEN, &up ) ) {\
    return sq_throwerror( vm, _SC( "Can't find userpointer." ) );\
  }\
  x object = (x)up;

/**
   Scourge object bindings to squirrel.
 */
class SqBinding {
private:
  Session *session;
  HSQUIRRELVM vm;
  
  // Native objects backing squirrel objects (squirrel class definitions).
  SqGame *game;
  SqCreature *creature;
  SqMission *mission;
  SqItem *item;

public:
  SqBinding( Session *session );
  ~SqBinding();

  static Session *sessionRef;
  static SqBinding *binding;

  // Squirrel object references
  HSQOBJECT refGame;
  HSQOBJECT refParty[MAX_PARTY_SIZE];
  HSQOBJECT refMission;
  std::vector<HSQOBJECT*> refCreature;
  std::map<Creature*,HSQOBJECT*> creatureMap; 
  std::map<Creature*,HSQOBJECT*> partyMap; 
  std::vector<HSQOBJECT*> refItem;
  std::map<Item*,HSQOBJECT*> itemMap;

  // events
  void startGame();
  void endGame();

  void loadMapScript( char *name );
  bool startLevel();
  bool endLevel();

  void compileBuffer( const char *s );

  static bool getObjectValue( HSQUIRRELVM vm, const char *key, void **ptr );

  static void printArgs( HSQUIRRELVM v );

  bool compile( const char *filename );

  bool callBoolMethod( const char *name, HSQOBJECT *param, bool *result );
  HSQOBJECT *getCreatureRef( Creature *creature );
  HSQOBJECT *getItemRef( Item *item );


protected:
  bool callNoArgMethod( const char *name );
  bool createClass( SquirrelClassDecl *cd, const char *key = NULL );
  bool instantiateClass( const SQChar *classname, 
                         HSQOBJECT *obj 
                         //,SQRELEASEHOOK hook 
                         );
  bool createClassMember( const char *classname, 
                          const char *key, 
                          int value );
  bool setObjectValue( HSQOBJECT object, 
                       const char *key, 
                       void *ptr );

};

#endif

