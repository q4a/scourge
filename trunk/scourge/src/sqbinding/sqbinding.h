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
#include "../common/constants.h"
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
class SqSpell;
class Creature;
class Item;
class Spell;
class File;

/**
 * A squirrel member function declaration.
 */
struct ScriptClassMemberDecl  {
  const char *returnType;
  const SQChar *name;
  SQFUNCTION func;
  int params;
  const SQChar *typemask;
  const char *description;
};

/**
 * A squirrel class declaration.
 */
struct SquirrelClassDecl  {
  const SQChar *name;
  const SQChar *base;
  const ScriptClassMemberDecl *members;
  const char *description;
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

#define SCOURGE_ID_TOKEN "scourge_id"

#define GET_OBJECT(x) x object;\
  {\
    SQUserPointer up;\
    if( !SqBinding::getObjectValue( vm, SCOURGE_ID_TOKEN, &up ) ) {\
      return sq_throwerror( vm, _SC( "Can't find userpointer." ) );\
    }\
    object = (x)up;\
  }
  
#define GET_STRING(__str_,__len_) char __str_[__len_];\
  {\
    const char *__tmp_;\
    if( SQ_FAILED( sq_getstring( vm, -1, &__tmp_ ) ) ) {\
      return sq_throwerror( vm, _SC( "Can't get string from stack." ) );\
    }\
    strncpy( __str_, __tmp_, __len_ );\
    __str_[__len_ - 1] = '\0';\
  }\
  sq_poptop( vm );
  
#define GET_INT(__n_) int __n_;\
  if( SQ_FAILED( sq_getinteger( vm, -1, &__n_ ) ) ) {\
    return sq_throwerror( vm, _SC( "Can't get int from stack." ) );\
  }\
  sq_poptop( vm );
  
#define GET_FLOAT(__n_) float __n_;\
  if( SQ_FAILED( sq_getfloat( vm, -1, &__n_ ) ) ) {\
    return sq_throwerror( vm, _SC( "Can't get float from stack." ) );\
  }\
  sq_poptop( vm );

#define GET_BOOL(__n_) bool __n_;\
  {\
    SQBool __tmp_;\
    if( SQ_FAILED( sq_getbool( vm, -1, &__tmp_ ) ) ) {\
      return sq_throwerror( vm, _SC( "Can't get bool from stack." ) );\
    }\
    __n_ = ( __tmp_ > 0 ? true : false );\
    sq_poptop( vm );\
  }
  
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
  SqSpell *spell;
  char lastMapScriptFileName[3000];

  static Item *currentWeapon;

public:
  SqBinding( Session *session );
  ~SqBinding();

  static Session *sessionRef;
  static SqBinding *binding;

  // Squirrel object references
  HSQOBJECT refGame;
	int partySize;
  HSQOBJECT refParty[MAX_PARTY_SIZE];
  HSQOBJECT refMission;
  std::vector<HSQOBJECT*> refCreature;
  std::map<Creature*,HSQOBJECT*> creatureMap; 
  std::map<Creature*,HSQOBJECT*> partyMap; 
  std::vector<HSQOBJECT*> refItem;
  std::map<Item*,HSQOBJECT*> itemMap;
  std::vector<HSQOBJECT*> refSpell;
  std::map<Spell*,HSQOBJECT*> spellMap;
  std::map<std::string, std::string> values;
  std::map<std::string,time_t> loadedScripts;

  inline void setValue( char *key, char *value ) {
    std::string skey = key;
    std::string svalue = value;
    values[ skey ] = svalue;

    /*
    // debug
    std::cerr << "SqBinding::setValue size=" << values.size() << std::endl; 
    for( std::map<std::string,std::string>::iterator i = values.begin(); i != values.end(); ++i ) {
      std::string k = i->first;
      std::string v = i->second;
      std::cerr << "\t" << k << "=" << v << std::endl;
    }
    std::cerr << "============================================" << std::endl;
    // end of debug
    */
  }

  inline char *getValue( char *key ) {
    std::string skey = key;
    if( values.find( skey ) == values.end() ) return NULL;
    return (char*)( values[ skey ].c_str() );
  }

  inline void eraseValue( char *key ) {
    std::string skey = key;
    if( values.find( skey ) != values.end() ) values.erase( skey );
  }
  void saveValues( File *file );
  void loadValues( File *file );

  // events
  void startGame();
  void endGame();

	// called when adding/removing party members
	void partyChanged();

	void initLevelObjects();
  bool startLevel();	
  bool endLevel();

	void registerCreature( Creature *ptr );
	void registerItem( Item *ptr );

  void compileBuffer( const char *s );

  static bool getObjectValue( HSQUIRRELVM vm, const char *key, void **ptr );

  static void printArgs( HSQUIRRELVM v );

  bool compile( const std::string& filename );

  bool callBoolMethod( const char *name, HSQOBJECT *param, bool *result );
  bool callNoArgMethod( const char *name, HSQOBJECT *param=NULL );
	bool callOneArgMethod( const char *name, HSQOBJECT *param1 );
	// if s is not NULL it is pushed on the stack instead of param2.
  bool callTwoArgMethod( const char *name, HSQOBJECT *param1, HSQOBJECT *param2, const char *s=NULL );
  bool callItemEvent( Creature *creature, Item *item, const char *function );
	bool callItemEvent( Item *item, const char *function );
  bool callSpellEvent( Creature *creature, Spell *spell, const char *function );
	bool callSkillEvent( Creature *creature, const char *skillName, const char *function );
  bool callMapPosMethod( const char *name, int x, int y, int z );
  bool callMapMethod( const char *name, const char *mapName );
  bool callConversationMethod( const char *name, Creature *creature, const char *word, char *answer );
  HSQOBJECT *getCreatureRef( Creature *creature );
  HSQOBJECT *getItemRef( Item *item );
  HSQOBJECT *getSpellRef( Spell *spell );
  void setGlobalVariable( char *name, float value );
  float getGlobalVariable( char *name );

  void reloadScripts();
  void documentSOM( char *path );
  
  inline static void setCurrentWeapon( Item *item ) { currentWeapon = item; }
  inline static Item *getCurrentWeapon() { return currentWeapon; }

protected:
  void registerScript( const std::string& file );
  void unregisterScript( const std::string& file );
  time_t getLastModTime( const std::string& file );

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

