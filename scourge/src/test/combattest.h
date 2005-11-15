/***************************************************************************
                          combattest.h  -  description
                             -------------------
    begin                : Sat Nov 12 2005
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

#ifndef COMBAT_TEST_H
#define COMBAT_TEST_H

#include "../constants.h"

/**
  *@author Gabor Torok
  */
  
class Session;
class Creature;
class Item;
	
class CombatTest {

public:
  CombatTest();
  ~CombatTest();
  static bool executeTests( Session *session, char *path );

protected:
  static bool fight( char *path,
                     char *filename,
                     Session *session, 
                     Creature *attacker, 
                     Item *weapon,
                     Creature *defender, 
                     int count=100 );
  static void printInventory( FILE *fp, Creature *creature );
  static Creature *createCharacter( Session *session, 
                                    char *characterShortName,
                                    char *name,
                                    int level );
  static Item *equipItem( Session *session, 
                          Creature *c, 
                          char *itemName, 
                          int itemLevel );
  static void computeHighLow( float value, float *sum, float *low, float *high );
  static void setMinSkills( Creature *c );
};

#endif

