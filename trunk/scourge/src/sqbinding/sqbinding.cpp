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
#include "../item.h"
#include "../rpg/rpglib.h"
#include "sqgame.h"
#include "sqcreature.h"
#include "sqmission.h"
#include "sqitem.h"
#include "sqspell.h"
#include "../squirrel/squirrel.h"
#include "../squirrel/sqstdio.h"
#include "../squirrel/sqstdaux.h"
#include "../squirrel/sqstdmath.h"
#include "../squirrel/sqstdstring.h"
#include "../io/file.h"
#include "../render/map.h"
#include "../debug.h"
#include <set>

using namespace std;

#ifdef SQUNICODE
#define scvprintf vswprintf
#else
#define scvprintf vsnprintf
#endif

Session *SqBinding::sessionRef = NULL;
SqBinding *SqBinding::binding = NULL;
Item *SqBinding::currentWeapon = NULL;

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
  if( SqBinding::sessionRef ) SqBinding::sessionRef->getGameAdapter()->printToConsole( (const char*)buff );  
  else cerr << "CONSOLE:" << buff << endl;
}

SqBinding::SqBinding( Session *session ) {
  SqBinding::sessionRef = this->session = session;
  if( !binding ) SqBinding::binding = this;

	partySize = 0;

  if( DEBUG_SQUIRREL ) cerr << "Initializing squirrel vm" << endl;
  vm = sq_open(1024); //creates a VM with initial stack size 1024

  sqstd_seterrorhandlers( vm );

  //sets the print function
  sq_setprintfunc( vm, printfunc );

  // push the root table(were the globals of the script will be stored)
  sq_pushroottable( vm );

  // init the math lib
  sqstd_register_mathlib( vm );
	sqstd_register_stringlib( vm );

  // Define some squirrel classes:

  // the root class 
  game = new SqGame();  
  createClass( game->getClassDeclaration() );

  // the creature class
  creature = new SqCreature();
  createClass( creature->getClassDeclaration(), SCOURGE_ID_TOKEN );

  mission = new SqMission();
  createClass( mission->getClassDeclaration() );

  item = new SqItem();
  createClass( item->getClassDeclaration(), SCOURGE_ID_TOKEN );

  spell = new SqSpell();
  createClass( spell->getClassDeclaration(), SCOURGE_ID_TOKEN );

  // compile some static scripts:
  // Special skills
  char s[200];
  sprintf(s, "%s/script/skills.nut", rootDir);
  if( !compile( s ) ) {
    cerr << "Error: *** Unable to compile special skills code: " << s << endl;
  }
  registerScript( s );

  // map interaction
  sprintf(s, "%s/script/map.nut", rootDir);
  if( !compile( s ) ) {
    cerr << "Error: *** Unable to compile map interaction code: " << s << endl;
  }
  registerScript( s );
}

SqBinding::~SqBinding() {
  sq_pop( vm, 1 ); //pops the root table
  sq_close( vm );
}


void SqBinding::partyChanged() {
	if( DEBUG_SQUIRREL ) {
		cerr << "*** Party changed!" << endl;
		cerr << "BEFORE: creatures=" << refCreature.size() << endl;
	}

	// remove from creatures if party member used to be a wandering hero
	HSQOBJECT *obj;
	for( int i = 0; i < session->getParty()->getPartySize(); i++ ) {
		if( creatureMap.find( session->getParty()->getParty(i) ) != creatureMap.end() ) {
			obj = creatureMap[ session->getParty()->getParty(i) ];
			for( vector<HSQOBJECT*>::iterator e = refCreature.begin(); e != refCreature.end(); ++e ) {
				HSQOBJECT *ref = *e;
				if( ref == obj ) {
					refCreature.erase( e );
					creatureMap.erase( session->getParty()->getParty(i) );
					delete obj;
					if( DEBUG_SQUIRREL ) cerr << "&&& Removed prior creature ref for " << 
						session->getParty()->getParty(i)->getName() << endl;
					break;
				}
			}
			break;
		}
	}

	// add a creature ref to any new creatures (dismissed heroes)
  for( int i = 0; i < session->getCreatureCount(); i++ ) {
		if( creatureMap.find( session->getCreature( i ) ) == creatureMap.end() ) {
			if( DEBUG_SQUIRREL ) cerr << "&&& Adding creature ref for dismissed hero: " << 
						session->getCreature( i )->getName() << endl;
			obj = (HSQOBJECT*)malloc( sizeof( HSQOBJECT ) );
			if( SQ_SUCCEEDED( instantiateClass( _SC( creature->getClassName() ), obj ) ) ) {
				// Set a token in the class so we can resolve the squirrel instance to a native creature.
				// The value is the address of the native creature object.
				setObjectValue( *obj, SCOURGE_ID_TOKEN, session->getCreature(i) );
				refCreature.push_back( obj );
				creatureMap[ session->getCreature(i) ] = obj;
			} else {
				cerr << "Unable instantiate class: " << creature->getClassName() << endl;
			}
		}
  }

  // release party references (using the original party size)
  for( int i = 0; i < partySize; i++ ) {
    sq_release( vm, &(refParty[ i ]) );  
  }
  partyMap.clear();
	partySize = session->getParty()->getPartySize();

	// create the party  
  for( int i = 0; i < session->getParty()->getPartySize(); i++ ) {
    if( SQ_SUCCEEDED( instantiateClass( _SC( creature->getClassName() ), &(refParty[i]) ) ) ) {
      // Set a token in the class so we can resolve the squirrel instance to a native creature.
      // The value is the address of the native creature object.
      setObjectValue( refParty[i], SCOURGE_ID_TOKEN, session->getParty()->getParty(i) );
      partyMap[ session->getParty()->getParty(i) ] = &(refParty[i]);
    }
  }

	if( DEBUG_SQUIRREL ) cerr << "AFTER: creatures=" << refCreature.size() << endl;
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
	partySize = session->getParty()->getPartySize();
  for( int i = 0; i < session->getParty()->getPartySize(); i++ ) {
    if( SQ_SUCCEEDED( instantiateClass( _SC( creature->getClassName() ), &(refParty[i]) ) ) ) {
      // Set a token in the class so we can resolve the squirrel instance to a native creature.
      // The value is the address of the native creature object.
      setObjectValue( refParty[i], SCOURGE_ID_TOKEN, session->getParty()->getParty(i) );
      partyMap[ session->getParty()->getParty(i) ] = &(refParty[i]);
    }
  }

  // create spells
  if( DEBUG_SQUIRREL ) cerr << "Creating spells:" << endl;  
  HSQOBJECT *obj;
  for( int i = 0; i < MagicSchool::getMagicSchoolCount(); i++ ) {
    MagicSchool *ms = MagicSchool::getMagicSchool( i );
    for( int t = 0; t < ms->getSpellCount(); t++ ) {
      obj = (HSQOBJECT*)malloc( sizeof( HSQOBJECT ) );
      if( SQ_SUCCEEDED( instantiateClass( _SC( spell->getClassName() ), obj ) ) ) {
        // Set a token in the class so we can resolve the squirrel instance to a native creature.
        // The value is the address of the native creature object.
        setObjectValue( *obj, SCOURGE_ID_TOKEN, ms->getSpell( t ) );
        refSpell.push_back( obj );
        spellMap[ ms->getSpell( t ) ] = obj;
      } else {
        cerr << "Unable instantiate class: " << spell->getClassName() << endl;
      }
    }
  }


}

void SqBinding::endGame() {
  // release party references
  for( int i = 0; i < session->getParty()->getPartySize(); i++ ) {
    sq_release( vm, &(refParty[ i ]) );  
  }
  partyMap.clear();
	partySize = 0;

  // destroy the spell references
  for( int i = 0; i < (int)refSpell.size(); i++ ) {
    sq_release( vm, refSpell[ i ] );
    free( refSpell[ i ] );
    refSpell[ i ] = NULL;
  }
  refSpell.clear();
  spellMap.clear();

  // release game ref.
  sq_release( vm, &refGame );  
  // remove the global scourgeGame variable
  sq_pushstring( vm, _SC( game->getInstanceName() ), -1 );
  sq_deleteslot( vm, -1, (SQBool)false );
}












void SqBinding::registerCreature( Creature *ptr ) {
	HSQOBJECT *obj = (HSQOBJECT*)malloc( sizeof( HSQOBJECT ) );
	if( SQ_SUCCEEDED( instantiateClass( _SC( creature->getClassName() ), obj ) ) ) {
		// Set a token in the class so we can resolve the squirrel instance to a native creature.
		// The value is the address of the native creature object.
		setObjectValue( *obj, SCOURGE_ID_TOKEN, ptr );
		refCreature.push_back( obj );
		creatureMap[ ptr ] = obj;
	} else {
		cerr << "Unable instantiate class: " << creature->getClassName() << endl;
	}
}

void SqBinding::registerItem( Item *ptr ) {
	HSQOBJECT *obj = (HSQOBJECT*)malloc( sizeof( HSQOBJECT ) );
	if( SQ_SUCCEEDED( instantiateClass( _SC( item->getClassName() ), obj ) ) ) {
		// Set a token in the class so we can resolve the squirrel instance to a native creature.
		// The value is the address of the native creature object.
		setObjectValue( *(obj), SCOURGE_ID_TOKEN, ptr );
		refItem.push_back( obj );
		itemMap[ ptr ] = obj;
	} else {
		cerr << "Unable instantiate class: " << creature->getClassName() << endl;
	}
}

void SqBinding::initLevelObjects() {
	// create the Mission
  if( DEBUG_SQUIRREL ) cerr << "Creating mission:" << endl;
  if( SQ_FAILED( instantiateClass( _SC( mission->getClassName() ), &refMission ) ) ) {
    cerr << "Failed to instantiate mission object." << endl;
  }

  // create the creatures of the level
  if( DEBUG_SQUIRREL ) cerr << "Creating level's creatures:" << endl;  
  for( int i = 0; i < session->getCreatureCount(); i++ ) {
		registerCreature( session->getCreature(i) );
  }

  // create the items of the level
  if( DEBUG_SQUIRREL ) cerr << "Creating level's items:" << endl;  
  for( int i = 0; i < session->getItemCount(); i++ ) {
		registerItem( session->getItem( i ) );
  }
}

bool SqBinding::startLevel() {
  bool ret = callMapMethod( "enterMap", session->getMap()->getName() );
  return ret;
}     

bool SqBinding::endLevel() {
  
  bool ret = callMapMethod( "exitMap", session->getMap()->getName() );

  // destroy the creatures of the level
  for( int i = 0; i < (int)refCreature.size(); i++ ) {
    sq_release( vm, refCreature[ i ] );
    free( refCreature[ i ] );
    refCreature[ i ] = NULL;
  }
  refCreature.clear();
  creatureMap.clear();

  // destroy the items of the level
  for( int i = 0; i < (int)refItem.size(); i++ ) {
    sq_release( vm, refItem[ i ] );
    free( refItem[ i ] );
    refItem[ i ] = NULL;
  }
  refItem.clear();
  itemMap.clear();

  // destroy the mission
  sq_release( vm, &refMission );

  return ret;  
}



HSQOBJECT *SqBinding::getCreatureRef( Creature *creature ) {
  if( creatureMap.find( creature ) != creatureMap.end() ) {
    return creatureMap[ creature ];
  } else if( partyMap.find( creature ) != partyMap.end() ) {
    return partyMap[ creature ];
  } else {
    // this could be b/c the objects aren't created yet
    //if( DEBUG_SQUIRREL ) cerr << "*** Warning: can't find squirrel object for creature: " << creature->getName() << endl;
    return NULL;
  }
}

HSQOBJECT *SqBinding::getItemRef( Item *item ) {
  if( itemMap.find( item ) != itemMap.end() ) {
    return itemMap[ item ];
  } else {
    // this could be b/c the objects aren't created yet
    //if( DEBUG_SQUIRREL ) cerr << "*** Warning: can't find squirrel object for item: " << item->getRpgItem()->getName() << endl;
    return NULL;
  }
}

HSQOBJECT *SqBinding::getSpellRef( Spell *spell ) {
  if( spellMap.find( spell ) != spellMap.end() ) {
    return spellMap[ spell ];
  } else {
    // this could be b/c the objects aren't created yet
    //if( DEBUG_SQUIRREL ) cerr << "*** Warning: can't find squirrel object for item: " << item->getRpgItem()->getName() << endl;
    return NULL;
  }
}

bool SqBinding::callBoolMethod( const char *name, 
                                HSQOBJECT *param, 
                                bool *result ) {
  bool ret;
  int top = sq_gettop( vm ); //saves the stack size before the call
  sq_pushroottable( vm ); //pushes the global table
  sq_pushstring( vm, _SC( name ), -1 );
  if( SQ_SUCCEEDED( sq_get( vm, -2 ) ) ) { //gets the field 'foo' from the global table
    sq_pushroottable( vm ); //push the 'this' (in this case is the global table)
    sq_pushobject( vm, *param );
    sq_call( vm, 2, 1 ); //calls the function
    SQBool sqres;
    sq_getbool( vm, -1, &sqres );
    *result = (bool)sqres;
    ret = true;
  } else {
    cerr << "Can't find function " << name << endl;
    ret = false;
  }
  sq_settop( vm, top ); //restores the original stack size
  return ret;
}

bool SqBinding::callConversationMethod( const char *name, 
                                        Creature *creature, 
                                        const char *word, 
                                        char *answer ) {

  strcpy( answer, "" );

  HSQOBJECT *creatureParam = getCreatureRef( creature );
  if( !creatureParam ) {
    cerr << "Can't find creature ref in callConversationMethod." << endl;
    return false;
  }

  bool ret;
  int top = sq_gettop( vm ); //saves the stack size before the call
  sq_pushroottable( vm ); //pushes the global table
  sq_pushstring( vm, _SC( name ), -1 );
  if( SQ_SUCCEEDED( sq_get( vm, -2 ) ) ) { //gets the field 'foo' from the global table
    sq_pushroottable( vm ); //push the 'this' (in this case is the global table)
    sq_pushobject( vm, *creatureParam );
    sq_pushstring( vm, _SC( word ), -1 );
    sq_call( vm, 3, 1 ); //calls the function    
    const SQChar *sqres = NULL;
    sq_getstring( vm, -1, &sqres );
    if( sqres ) {
      strcpy( answer, (char*)sqres );
    }
    ret = true;    
  } else {
    cerr << "Can't find function " << name << endl;
    ret = false;
  }
  sq_settop( vm, top ); //restores the original stack size
  return ret;
}

bool SqBinding::callItemEvent( Item *item, 
                               const char *function ) {
  HSQOBJECT *itemParam = getItemRef( item );
  if( itemParam ) {
    return callOneArgMethod( function, itemParam );
  }
  return false;
}

bool SqBinding::callItemEvent( Creature *creature, 
                               Item *item, 
                               const char *function ) {
  HSQOBJECT *creatureParam = getCreatureRef( creature );
  HSQOBJECT *itemParam = getItemRef( item );
  if( creatureParam && itemParam ) {
    return callTwoArgMethod( function, 
                             creatureParam, 
                             itemParam );
  }
  return false;
}

bool SqBinding::callSpellEvent( Creature *creature, 
                                Spell *spell, 
                                const char *function ) {
  HSQOBJECT *creatureParam = getCreatureRef( creature );
  HSQOBJECT *spellParam = getSpellRef( spell );
  if( creatureParam && spellParam ) {
    return callTwoArgMethod( function, 
                             creatureParam, 
                             spellParam );
  }
  return false;
}

bool SqBinding::callSkillEvent( Creature *creature, 
																const char *skillName, 
																const char *function ) {
  HSQOBJECT *creatureParam = getCreatureRef( creature );
  if( creatureParam ) {
    return callTwoArgMethod( function, 
                             creatureParam,
														 NULL,
                             skillName );
  }
  return false;
}

bool SqBinding::callTwoArgMethod( const char *name,
                                  HSQOBJECT *param1,
                                  HSQOBJECT *param2,
																	const char *s ) {
  bool ret;
  int top = sq_gettop( vm ); //saves the stack size before the call
  sq_pushroottable( vm ); //pushes the global table
  sq_pushstring( vm, _SC( name ), -1 );
  if( SQ_SUCCEEDED( sq_get( vm, -2 ) ) ) { //gets the field 'foo' from the global table
    sq_pushroottable( vm ); //push the 'this' (in this case is the global table)
    sq_pushobject( vm, *param1 );
		if( s ) {
			sq_pushstring( vm, _SC( s ), -1 );
		} else {
			sq_pushobject( vm, *param2 );
		}
    sq_call( vm, 3, 0 ); //calls the function
    ret = true;
  } else {
    cerr << "Can't find function " << name << endl;
    ret = false;
  }
  sq_settop( vm, top ); //restores the original stack size
  return ret;
}

bool SqBinding::callOneArgMethod( const char *name, HSQOBJECT *param1 ) {
  bool ret;
  int top = sq_gettop( vm ); //saves the stack size before the call
  sq_pushroottable( vm ); //pushes the global table
  sq_pushstring( vm, _SC( name ), -1 );
  if( SQ_SUCCEEDED( sq_get( vm, -2 ) ) ) { //gets the field 'foo' from the global table
    sq_pushroottable( vm ); //push the 'this' (in this case is the global table)
    sq_pushobject( vm, *param1 );
    sq_call( vm, 2, 0 ); //calls the function
    ret = true;
  } else {
    cerr << "Can't find function " << name << endl;
    ret = false;
  }
  sq_settop( vm, top ); //restores the original stack size
  return ret;
}

bool SqBinding::callMapPosMethod( const char *name, int x, int y, int z ) {
  bool ret;
  int top = sq_gettop( vm ); //saves the stack size before the call
  sq_pushroottable( vm ); //pushes the global table
  sq_pushstring( vm, _SC( name ), -1 );
  if( SQ_SUCCEEDED( sq_get( vm, -2 ) ) ) { //gets the field 'foo' from the global table
    sq_pushroottable( vm ); //push the 'this' (in this case is the global table)
    sq_pushinteger( vm, x );
    sq_pushinteger( vm, y );
    sq_pushinteger( vm, z );
		sq_call( vm, 4, 1 ); //calls the function
    SQBool sqres;
    sq_getbool( vm, -1, &sqres );
    ret = (bool)sqres;
  } else {
    cerr << "Can't find function " << name << endl;
    ret = false;
  }
  sq_settop( vm, top ); //restores the original stack size
  return ret;
}

bool SqBinding::callNoArgMethod( const char *name, HSQOBJECT *param ) {
  //int ret = -1;
  bool ret;
  int top = sq_gettop( vm ); //saves the stack size before the call
  sq_pushroottable( vm ); //pushes the global table
  sq_pushstring( vm, _SC( name ), -1 );
  if( SQ_SUCCEEDED( sq_get( vm, -2 ) ) ) { //gets the field 'foo' from the global table
    sq_pushroottable( vm ); //push the 'this' (in this case is the global table)
    if( param ) {
      sq_pushobject( vm, *param );
      sq_call( vm, 2, 0 ); //calls the function
    } else {
      sq_call( vm, 1, 0 ); //calls the function
    }
    ret = true;
  } else {
    cerr << "Can't find function " << name << endl;
    ret = false;
  }
  sq_settop( vm, top ); //restores the original stack size
  return ret;
}

bool SqBinding::callMapMethod( const char *name, const char *mapName ) {
  //int ret = -1;
  bool ret;
  int top = sq_gettop( vm ); //saves the stack size before the call
  sq_pushroottable( vm ); //pushes the global table
  sq_pushstring( vm, _SC( name ), -1 );
  if( SQ_SUCCEEDED( sq_get( vm, -2 ) ) ) { //gets the field 'foo' from the global table
    sq_pushroottable( vm ); //push the 'this' (in this case is the global table)
    sq_pushstring( vm, mapName, -1 );
    sq_call( vm, 2, 0 ); //calls the function
    ret = true;
  } else {
    cerr << "Can't find function " << name << endl;
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

bool SqBinding::createClass( SquirrelClassDecl *cd, const char *key ) {
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

  // create a slot to hold the Creature* pointer.
  if( key ) {
    createClassMember( cd->name, key, -2 );
  }

  return true;
}

bool SqBinding::instantiateClass( const SQChar *classname, 
                                  HSQOBJECT *obj 
                                  //,SQRELEASEHOOK hook 
                                  ) {
  SQUserPointer ud = NULL;
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

bool SqBinding::setObjectValue( HSQOBJECT object, const char *key, void *ptr ) {
  bool ret = true;
  int top = sq_gettop( vm );
  sq_pushobject( vm, object );
  sq_pushstring( vm, _SC( key ), -1 );
  sq_pushuserpointer( vm, (SQUserPointer)ptr );
  //sq_pushinteger( vm, value );
  if( SQ_FAILED( sq_set( vm, -3 ) ) ) {
    cerr << "Unable to set object member:" << key << " to " << ptr << endl;
    ret = false;
  }
  sq_settop( vm, top );
  return ret;
}

bool SqBinding::getObjectValue( HSQUIRRELVM vm, const char *key, void **ptr ) {
  bool ret = false;
  int top = sq_gettop( vm );
  sq_pushstring( vm, _SC( key ), -1 );
  if( SQ_SUCCEEDED( sq_get( vm, -2 ) ) ) {
//    SQUserPointer creature;
//    sq_getuserpointer( vm, -1, &creature );
    sq_getuserpointer( vm, -1, (SQUserPointer*)ptr );
    ret = true;
  }
  sq_settop( vm, top );
  return ret;
}

void SqBinding::printArgs( HSQUIRRELVM v ) {
  SQInteger nargs = sq_gettop(v); //number of arguments
  for(SQInteger n=1;n<=nargs;n++) {
    printf("arg %d is ",n);
    switch(sq_gettype(v,n)) {
    case OT_NULL:
    printf("null");        
    break;
    case OT_INTEGER:
    printf("integer");
    break;
    case OT_FLOAT:
    printf("float");
    break;
    case OT_STRING:
    printf("string");
    break;    
    case OT_TABLE:
    printf("table");
    break;
    case OT_ARRAY:
    printf("array");
    break;
    case OT_USERDATA:
    printf("userdata");
    break;
    case OT_CLOSURE:        
    printf("closure(function)");    
    break;
    case OT_NATIVECLOSURE:
    printf("native closure(C function)");
    break;
    case OT_GENERATOR:
    printf("generator");
    break;
    case OT_USERPOINTER:
    printf("userpointer");
    break;
    case OT_INSTANCE:
    printf( "object instance" );
    break;
    default:
    cerr << "invalid param" << endl; //throws an exception
    }
  }
  printf("\n");
}

void SqBinding::registerScript( char *file ) { 
  std::string fileString = file;    
  loadedScripts[ fileString ] = getLastModTime( file );
}

void SqBinding::unregisterScript( char *file ) {
  std::string fileString = file;    
  if( loadedScripts.find( fileString ) != loadedScripts.end() ) {
    loadedScripts.erase( fileString );
  }
}

void SqBinding::reloadScripts() {
  if( DEBUG_SQUIRREL ) cerr << "Checking loaded scripts" << endl;
  for( map<string,time_t>::iterator i = loadedScripts.begin(); 
       i != loadedScripts.end(); ++i ) {
    string file = i->first;
    time_t lastMod = i->second;
    if( DEBUG_SQUIRREL ) cerr << "Checking file: " << file << endl;
    time_t newLastMod = getLastModTime( (char*)file.c_str() );
    if( lastMod != newLastMod ) {
      if( DEBUG_SQUIRREL ) cerr << "\tReloading!" << endl;
      // not sure why I need to push the root table here... 
      sq_pushroottable( vm );
      if( compile( file.c_str() ) ) {
        loadedScripts[ file ] = newLastMod;
      } else {
        cerr << "Error: *** Unable to compile special skills code: " << file << endl;
      }
      sq_pop( vm, 1 ); //pops the root table
    }
  }
  if( DEBUG_SQUIRREL ) cerr << "----------------------" << endl;
}

time_t SqBinding::getLastModTime( char *file ) {
#ifdef WIN32
  return 0;
#else  
  struct stat buf;
  int err = stat( file, &buf );
  if( err ) {
    cerr << "Error while looking  at file " << file << ": " << strerror( errno ) << " (" << errno << ")" << endl;
    return 0;
  }
  return buf.st_mtime;
#endif  
}

void SqBinding::saveValues( File *file ) {
  //std::cerr << "SqBinding::setValue size=" << values.size() << std::endl; 
  Uint32 n;
  for( std::map<std::string,std::string>::iterator i = values.begin(); i != values.end(); ++i ) {
    std::string k = i->first;
    std::string v = i->second;

    n = (Uint32)k.size() + 1;
    file->write( &n );
    file->write( (Uint8*)( k.c_str() ), n );

    n = (Uint32)v.size() + 1;
    file->write( &n );
    file->write( (Uint8*)( v.c_str() ), n );
  }
}

void SqBinding::loadValues( File *file ) {
  char k[255], v[255];
  Uint32 size;
  values.clear();
  while( true ) {
    if( file->read( &size ) < 1 ) break;
    file->read( (Uint8*)k, size );
    file->read( &size );
    file->read( (Uint8*)v, size );
    setValue( k, v );
  }
}

void SqBinding::setGlobalVariable( char *name, float value ) {
  int oldtop = sq_gettop( vm );
  sq_pushroottable( vm );
  sq_pushstring( vm, _SC( name ), -1 );
  sq_pushfloat( vm, value );
  bool success = true;
  if( SQ_FAILED( sq_createslot( vm, -3 ) ) ) {
    success = false;
  }
//  sq_pop( vm, -1 );
  sq_settop( vm, oldtop );
  if( !success ) {
    // maybe it exists already... try to set it
    sq_pushroottable( vm );
    sq_pushstring( vm, _SC( name ), -1 );
    sq_pushfloat( vm, value );
    if( SQ_FAILED( sq_set( vm, -3 ) ) ) {
      cerr << "Unable to create global var slot or set value for:" << name << endl;
    }
  }
}

float SqBinding::getGlobalVariable( char *name ) {
  float value;
  int oldtop = sq_gettop( vm );
  sq_pushroottable( vm );
  sq_pushstring( vm, _SC( name ), -1 );
  if( SQ_FAILED( sq_get( vm, -2 ) ) ) {
    cerr << "Unable to get global var slot:" << name << endl;
  }
  sq_getfloat( vm, -1, &value );
//  sq_pop( vm, -1 );
  sq_settop( vm, oldtop );
  return value;
}

void SqBinding::documentSOM( char *path ) {
  set<SqObject*> objects;

  objects.insert( new SqGame() );
  objects.insert( new SqMission() );
  objects.insert( new SqCreature() );
  objects.insert( new SqItem() );
	objects.insert( new SqSpell() );

  set<string> names;
  for( set<SqObject*>::iterator e = objects.begin();
       e != objects.end(); ++e ) {
    SqObject *object = *e;
    string s = object->getClassName();
    names.insert( s );
  }

  for( set<SqObject*>::iterator e = objects.begin();
       e != objects.end(); ++e ) {
    SqObject *object = *e;
    object->documentSOM( path, &names );
  }
}

