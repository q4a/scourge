/***************************************************************************
                          combattest.cpp  -  description
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
#include "combattest.h"
#include "../rpg/rpglib.h"
#include "../creature.h"
#include "../item.h"
#include "../session.h"
#include <vector>

using namespace std;

/**
  *@author Gabor Torok
  */

#define SHOW_MAX_LEVEL 20
#define HTML_PREFIX "<html><head><title>%s</title><style>\
BODY { \
  background: white;\
  font-size: 9pt; \
  FONT-FAMILY: Arial, Helvetica, sans-serif; \
  margin: 15px;\
  color: black;\
}\
TD { \
  font-size: 8pt; \
  FONT-FAMILY: Monospace, Courier, sans-serif; \
}\
TH { \
  font-size: 9pt; \
  FONT-FAMILY: Arial, Helvetica, sans-serif; \
  font-weight:bold;\
}\
</style></head><body bgcolor=white>"
#define HTML_POSTFIX "</body></html>"
	
CombatTest::CombatTest() {
}

CombatTest::~CombatTest() {
}

bool CombatTest::executeTests( Session *session, char *path ) {
  vector<Creature*> creatures;
  vector<Item*> items;
  
  // -----------------------------------------------------------
  // The attacker
  Creature *attacker = createCharacter( session, "RA", "Attacker", 1 );
  creatures.push_back( attacker );
  Item *weapon = equipItem( session, attacker, "Dirk", 1 );
  items.push_back( weapon );
  
  // The defender
  Creature *defender = createCharacter( session, "RA", "Defender", 1 );
  creatures.push_back( defender );
  items.push_back( equipItem( session, defender, "Horned helmet", 1 ) );
  items.push_back( equipItem( session, defender, "Leather Armor", 1 ) );

  // Test all weapons vs. leather armor
  bool ret = true;
  char filename[80];
  for( int i = 0; i < RpgItem::itemCount; i++ ) {
    if( !RpgItem::getItem( i )->isWeapon() ) continue;
    attacker->removeInventory( 0 );
    weapon = equipItem( session, attacker, RpgItem::getItem( i )->getName(), 1 );
    items.push_back( weapon );
    sprintf( filename, "weapon_%s.html", RpgItem::getItem( i )->getName() );
    if( !fight( path, filename, session, attacker, weapon, defender ) ) break;
  }

  // Test all character classes with long sword vs. leather armor
  weapon = equipItem( session, attacker, "Long sword", 1 );
  items.push_back( weapon );
  for( int i = 0; i < (int)Character::character_list.size(); i++ ) {
    Creature *c = createCharacter( session, Character::character_list[i]->getShortName(), "Defender", 1 );
    creatures.push_back( c );
    items.push_back( equipItem( session, c, "Horned helmet", 1 ) );
    items.push_back( equipItem( session, c, "Leather Armor", 1 ) );
    sprintf( filename, "character_%s.html", Character::character_list[i]->getName() );
    if( !fight( path, filename, session, attacker, weapon, c ) ) break;
  }

  // cleanup
  for( int i = 0; i < (int)items.size(); i++ ) {
    Item *item = items[ i ];
    delete item;
  }
  for( int i = 0; i < (int)creatures.size(); i++ ) {
    Creature *creature = creatures[ i ];
    delete creature;
  }
  

  return ret;
}





bool CombatTest::fight( char *path,
                        char *filename,
                        Session *session, 
                        Creature *attacker, 
                        Item *weapon,
                        Creature *defender, 
                        int count ) {

  char file[300];
  sprintf( file, "%s/%s", path, filename );
  FILE *fp = fopen( file, "w" );
  if( !fp ) {
    cerr << "Unable to open file: " << file << endl;
    return false;
  }
  cout << "...creating file: " << file << endl;

  attacker->setTargetCreature( defender );

  fprintf( fp, HTML_PREFIX, "Combat Test" );

  fprintf( fp, "<h1>Combat test with %d iterations</h1>", count );
  fprintf( fp, "<b>%s</b> with %s(%d) vs. ",
           attacker->getCharacter()->getName(),
           weapon->getRpgItem()->getName(),
           weapon->getLevel() );
  fprintf( fp, "<b>%s</b><br>\n",
           defender->getCharacter()->getName() );
  fprintf( fp, "<a href=\"#details\">Character details...</a><br><br>");

  fprintf( fp, "<b>Attacker</b> vert., <b>Defender</b> horiz.<br>\n" );
  fprintf( fp, "<table border=1><tr><td>&nbsp;</td>\n" );
  for( int defendLevel = 1; defendLevel < SHOW_MAX_LEVEL; defendLevel++ ) {
    fprintf( fp, "<td>%d</td>", defendLevel );
  }
  fprintf( fp, "</tr>\n" );

  float sum=0, low=0, high=0, ave, ATKave;
  for( int attackLevel = 1; attackLevel < SHOW_MAX_LEVEL; attackLevel++ ) {
    attacker->setLevel( attackLevel );
    fprintf( fp, "<tr><td>%d<br>\n", attackLevel );

    // -------------------------------------------------
    // Attack roll
    sum = low = high = 0;
    for( int i = 0; i < count; i++ ) {
      computeHighLow( attacker->getAttackRoll( weapon ),
                      &sum, &low, &high );
    }
    ATKave = sum / ((double)count);
    fprintf( fp, "ATK:%.2f/%.2f/%.2f</td>\n", low, high, ATKave );
    // -------------------------------------------------

    for( int defendLevel = 1; defendLevel < SHOW_MAX_LEVEL; defendLevel++ ) {
      defender->setLevel( defendLevel );
      fprintf( fp, "<td bgcolor=%s>", ( defendLevel == attackLevel ? "gray" : "white" ) );

      // -------------------------------------------------
      // Armor class
      float armor, shield, skillBonus, armorPenalty, shieldPenalty;
      float ac = defender->getAC( &armor, &shield, &skillBonus, &armorPenalty, &shieldPenalty );
      fprintf( fp, "AC :<span style='background: %s'><b>%.2f</b>(ARM:%.2f,SLD:%.2f,BON:%.2f,AP:%.2f,SP:%.2f)</span><br>\n",
               ( ATKave < ac ? "green" : "red" ), ac,
               armor, shield, skillBonus, armorPenalty, shieldPenalty );

      // -------------------------------------------------
      // Damage roll
      sum = low = high = 0;
      for( int i = 0; i < count; i++ ) {
        computeHighLow( attacker->getDamage( weapon ),
                        &sum, &low, &high );
      }
      ave = sum / ((double)count);
      fprintf( fp, "DAM:%.2f/%.2f/%.2f\n",
               low, high, ave );
      // -------------------------------------------------

      fprintf( fp, "</td>" );
    }
    fprintf( fp, "</tr>\n" );
  }
  fprintf( fp, "</table><br>\n" );
  
  fprintf( fp, "<br><a name=\"details\"></a><b>Character details:</b>" );
  fprintf( fp, "<table><tr><td valign=top>" );
  fprintf( fp, "Attacker: <b>%s</b> with %s(%d)<br>\n",
           attacker->getCharacter()->getName(),
           weapon->getRpgItem()->getName(),
           weapon->getLevel() );
  printInventory( fp, attacker );
  
  fprintf( fp, "</td><td valign=top>" );
  fprintf( fp, "Defender: <b>%s</b><br>\n",
           defender->getCharacter()->getName() );
  printInventory( fp, defender );
  fprintf( fp, "</td></tr></table>" );

  fclose( fp );
  return true;
}

void CombatTest::computeHighLow( float value, float *sum, float *low, float *high ) {
  if( !(*high) ) {
    *low = *high = value;
  } else if( value > *high ) {
    *high = value;
  } else if( value < *low ) {
    *low = value;
  }
  *sum += value;
}

void CombatTest::printInventory( FILE *fp, Creature *creature ) {  
  fprintf( fp, "<b>Skills:</b><table border=1>\n" );
  for( int i = 0; i < Constants::SKILL_COUNT; i++ ) {
    fprintf( fp, "<tr><td><b>%s</b></td><td>%d</td></tr>\n", 
             Constants::SKILL_NAMES[ i ],
             creature->getSkill( i ) );
  }
  fprintf( fp, "</table>\n" );

  fprintf( fp, "<b>Inventory:</b><ul>\n" );
  for( int i = 0; i < creature->getInventoryCount(); i++ ) {
    fprintf( fp, "<li>%s(%d) %s<br>\n", 
             creature->getInventory( i )->getRpgItem()->getName(),
             creature->getInventory( i )->getLevel(),
             ( creature->isEquipped( i ) ? "<i>Equipped</i>" : "" ) );
  }
  fprintf( fp, "</ul>\n" );
}

Creature *CombatTest::createCharacter( Session *session, 
                                       char *characterShortName,
                                       char *name,
                                       int level ) {
  Character *character = 
    Character::
    getCharacterByShortName( characterShortName );
  Creature *c = 
    new Creature( session, 
                  character, 
                  strdup( name ), 
                  0, 
                  false );
  //c->setDeityIndex( info[i].deityType->getSelectedLine() );
  c->setLevel( level ); 
  c->setExp( 0 );
  c->setHp();
  c->setMp();
  c->setHunger(10);
  c->setThirst(10); 
  
  // assign portraits
  //c->setPortraitTextureIndex( info[i].portraitIndex );
  
  // set skill levels to 50%
  for(int t = 0; t < Constants::SKILL_COUNT; t++) {
    c->setSkill( t, MAX_SKILL / 2 );
  }
  
  return c;
}

Item *CombatTest::equipItem( Session *session, 
                             Creature *c, 
                             char *itemName, 
                             int itemLevel ) {
  Item *item = new Item( session, RpgItem::getItemByName( itemName ), itemLevel );
  c->addInventory( item, true );
  c->equipInventory( c->getInventoryCount() - 1 );
  return item;
}

