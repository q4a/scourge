/***************************************************************************
                          sqbinding.cpp  -  description
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
#include "sqbinding.h"
#include "../session.h"
#include "../creature.h"
#include "sqgame.h"
#include "sqcreature.h"
#include "../squirrel/squirrel.h"
#include "../squirrel/sqstdio.h"
#include "../squirrel/sqstdaux.h"

#ifdef SQUNICODE
#define scvprintf vswprintf
#else
#define scvprintf vsnprintf
#endif

ConsolePrinter *SqBinding::consolePrinterRef = NULL;
Session *SqBinding::sessionRef = NULL;
SqBinding *SqBinding::binding = NULL;

/**
 * A simple print function. Later replace this by printing in the console.
 */
void printfunc(HSQUIRRELVM v, const SQChar *s, ...) {
  va_list arglist;
  va_start(arglist, s);
  char buff[1000];
  scvprintf( buff, 1000, s, arglist );  
  buff[999] = '\0';
  va_end(arglist);
  if( SqBinding::consolePrinterRef ) SqBinding::consolePrinterRef->printToConsole( (const char*)buff );  
  else cerr << "CONSOLE:" << buff << endl;
}

SqBinding::SqBinding( Session *session, ConsolePrinter *consolePrinter ) {
  SqBinding::sessionRef = this->session = session;
  if( consolePrinter ) SqBinding::consolePrinterRef = consolePrinter;
  if( !binding ) SqBinding::binding = this;

  if( DEBUG_SQUIRREL ) cerr << "Initializing squirrel vm" << endl;
  vm = sq_open(1024); //creates a VM with initial stack size 1024

  sqstd_seterrorhandlers( vm );

  //sets the print function
  sq_setprintfunc( vm, printfunc );

  // push the root table(were the globals of the script will be stored)
  sq_pushroottable( vm );

  // Define some squirrel classes:

  // the root class 
  game = new SqGame();  
  createClass( game->getClassDeclaration() );

  // the creature class
  creature = new SqCreature();
  createClass( creature->getClassDeclaration() );

  // create a slot to hold the creature id.
  createClassMember( creature->getClassName(), CREATURE_ID_TOKEN, -2 );
  
}

SqBinding::~SqBinding() {
  sq_pop( vm, 1 ); //pops the root table
  sq_close( vm );
}




void SqBinding::startGame() {
  // Create a game instance (root squirrel object)
  instantiateClass( _SC( game->getClassName() ), &refGame );
  // and bind it as a global variable
  sq_pushstring( vm, _SC( game->getInstanceName() ), -1 );
  sq_pushobject( vm, refGame );
  if( SQ_FAILED( sq_createslot( vm, -3 ) ) ) {
    cerr << "Unable to create object \"" << game->getInstanceName() << "\" slot." << endl;
  }

  // create the party
  if( DEBUG_SQUIRREL ) cerr << "Creating party:" << endl;
  for( int i = 0; i < session->getParty()->getPartySize(); i++ ) {
    if( SQ_SUCCEEDED( instantiateClass( _SC( creature->getClassName() ), &(refParty[i]) ) ) ) {

      // Set a token in the class so we can resolve the squirrel instance to a native creature.
      // Negative numbers are party members, 0 and pos. ones are monsters.
      setObjectValue( refParty[i], CREATURE_ID_TOKEN, -1 * ( i + 1 ) );

      if( DEBUG_SQUIRREL ) cerr << "\tSuccess: i=" << i << endl;
    } else {
      if( DEBUG_SQUIRREL ) cerr << "\tFailed: i=" << i << endl;
    }
  }
}

void SqBinding::endGame() {
  // release party references
  for( int i = 0; i < session->getParty()->getPartySize(); i++ ) {
    sq_release( vm, &(refParty[ i ]) );  
  }

  // release game ref.
  sq_release( vm, &refGame );  
  // remove the global scourgeGame variable
  sq_pushstring( vm, _SC( game->getInstanceName() ), -1 );
  sq_deleteslot( vm, -1, (SQBool)false );
}


void SqBinding::loadMapScript( char *name ) {
  char filename[3000];
  sprintf( filename, "%s/maps/%s.nut", rootDir, name );
  compile( (const char*)filename );
}

bool SqBinding::startLevel() {
  //int ret = -1;
  bool ret;
  int top = sq_gettop( vm ); //saves the stack size before the call
  sq_pushroottable( vm ); //pushes the global table
  sq_pushstring( vm, _SC("startLevel"), -1 );
  if( SQ_SUCCEEDED( sq_get( vm, -2 ) ) ) { //gets the field 'foo' from the global table
    sq_pushroottable( vm ); //push the 'this' (in this case is the global table)
    //sq_pushobject( vm, refGame );
    sq_call( vm, 1, 0 ); //calls the function
    //sq_getinteger( v, -1, &ret );
    ret = true;
  } else {
    cerr << "Can't find function startLevel()." << endl;
    ret = false;
  }
  sq_settop( vm, top ); //restores the original stack size
  return ret;  
}     

bool SqBinding::endLevel() {
  //int ret = -1;
  bool ret;
  int top = sq_gettop( vm ); //saves the stack size before the call
  sq_pushroottable( vm ); //pushes the global table
  sq_pushstring( vm, _SC("endLevel"), -1 );
  if( SQ_SUCCEEDED( sq_get( vm, -2 ) ) ) { //gets the field 'foo' from the global table
    sq_pushroottable( vm ); //push the 'this' (in this case is the global table)
    //sq_pushobject( vm, refGame );
    sq_call( vm, 1, 0 ); //calls the function
    //sq_getinteger( v, -1, &ret );
    ret = true;
  } else {
    cerr << "Can't find function endLevel()." << endl;
    ret = false;
  }
  sq_settop( vm, top ); //restores the original stack size
  return ret;  
}



void SqBinding::compileBuffer( const char *s ) {
  int top = sq_gettop( vm ); //saves the stack size before the call
  if( SQ_SUCCEEDED( sq_compilebuffer( vm, 
                                      _SC( s ), 
                                      strlen( s ), 
                                      _SC( "tmp" ), 
                                      (SQBool)true ) ) ) {
    // execute it
    sq_pushroottable( vm ); //push the 'this' (in this case is the global table)
    sq_call( vm, 1, 0 ); //calls the function
    // ignore the return value
  }
  sq_settop( vm, top ); //restores the original stack size
}
                        
bool SqBinding::compile( const char *filename ) {
  // compile a module
  if( DEBUG_SQUIRREL ) cerr << "Compiling file:" << filename << endl;
  if( SQ_SUCCEEDED( sqstd_dofile( vm, _SC( filename ), 0, 1 ) ) ) {
    if( DEBUG_SQUIRREL ) cerr << "\tSuccess." << endl;
    return true;
  } else {
    cerr << "Failed to compile file:" << filename << endl;
    return false;
  }
}

bool SqBinding::createClass( SquirrelClassDecl *cd ) {
  int n = 0;
  int oldtop = sq_gettop( vm );
  sq_pushroottable( vm );
  sq_pushstring( vm, cd->name, -1 );
  if( cd->base ) {
    sq_pushstring( vm, cd->base, -1 );
    if( SQ_FAILED( sq_get( vm, -3 ) ) ) {
      sq_settop( vm, oldtop );
      return false;
    }
  }
  if( SQ_FAILED( sq_newclass( vm, cd->base ? 1 : 0 ) ) ) {
    sq_settop( vm, oldtop );
    return false;
  }
  //sq_settypetag(v,-1,(unsigned int)cd);
  sq_settypetag( vm, -1, (SQUserPointer)cd );
  const ScriptClassMemberDecl *members = cd->members;
  const ScriptClassMemberDecl *m = NULL;
  while( members[ n ].name ) {
    m = &members[ n ];
    sq_pushstring( vm, m->name, -1 );
    sq_newclosure( vm, m->func, 0 );
    sq_setparamscheck( vm, m->params, m->typemask );
    sq_setnativeclosurename( vm, -1, m->name );
    sq_createslot( vm, -3 );
    n++;
  }
  sq_createslot( vm, -3 );
  sq_pop( vm, 1 );
  return true;
}

bool SqBinding::instantiateClass( const SQChar *classname, 
                                  HSQOBJECT *obj 
                                  //,SQRELEASEHOOK hook 
                                  ) {
  SQUserPointer ud;
  int oldtop = sq_gettop( vm );
  sq_pushroottable( vm );
  sq_pushstring( vm, classname, -1 );
  if( SQ_FAILED( sq_rawget( vm, -2 ) ) ) {
    sq_settop( vm, oldtop );
    return false;
  }
  //sq_pushroottable(v);
  if( SQ_FAILED( sq_createinstance( vm, -1 ) ) ) {
    sq_settop( vm, oldtop );
    return false;
  }
  sq_remove( vm, -3 ); //removes the root table
  sq_remove( vm, -2 ); //removes the the class
  if( SQ_FAILED( sq_setinstanceup( vm, -1, ud ) ) ) {
    sq_settop( vm, oldtop );
    return false;
  }
  //      sq_setreleasehook(v,-1,hook);
  //sq_settop(v,oldtop);
  
  // create a squirrel object
  sq_getstackobj( vm, -1, obj );
  // make sure it's not gc-ed
  sq_addref( vm, obj );

  // be sure to release it later!!!
  //sq_release(SquirrelVM::_VM,&_o);
  //_o = t;
  
  //              sq_pop( v, 1 );
  sq_settop( vm, oldtop );
  
  return true;
}   

bool SqBinding::createClassMember( const char *classname, const char *key, int value ) {  
  int oldtop = sq_gettop( vm );
  sq_pushroottable( vm );
  sq_pushstring( vm, classname, -1 );
  if( SQ_FAILED( sq_rawget( vm, -2 ) ) ) {
    sq_settop( vm, oldtop );
    cerr << "Error getting class " << classname << endl;
    return false;
  }
  sq_pushstring( vm, _SC( key ), -1 );
  sq_pushinteger( vm, value );
  if( SQ_FAILED( sq_createslot( vm, -3 ) ) ) {
    cerr << "Unable to create member slot:" << key << endl;
  }
//  sq_pop( vm, -1 );
  sq_settop( vm, oldtop );
  return true;
}

bool SqBinding::setObjectValue( HSQOBJECT object, const char *key, int value ) {
  bool ret = true;
  int top = sq_gettop( vm );
  sq_pushobject( vm, object );
  sq_pushstring( vm, _SC( key ), -1 );
  sq_pushinteger( vm, value );
  if( SQ_FAILED( sq_set( vm, -3 ) ) ) {
    cerr << "Unable to set object member:" << key << " to " << value << endl;
    ret = false;
  }
  sq_settop( vm, top );
  return ret;
}

