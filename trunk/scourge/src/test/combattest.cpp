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

#define SHOW_MAX_LEVEL MAX_LEVEL
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
  padding: 5px;\
  border: 1px solid gray\
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
  attacker->removeInventory( 0 );
  weapon = equipItem( session, attacker, "Long sword", 1 );
  items.push_back( weapon );
  for( int i = 0; i < (int)Character::character_list.size(); i++ ) {
    Creature *c = createCharacter( session, Character::character_list[i]->getName(), "Defender", 1 );
    creatures.push_back( c );
    items.push_back( equipItem( session, c, "Horned helmet", 1 ) );
    items.push_back( equipItem( session, c, "Leather Armor", 1 ) );
    sprintf( filename, "character_%s.html", Character::character_list[i]->getName() );
    if( !fight( path, filename, session, attacker, weapon, c ) ) break;
  }

  // Test random skill values
  for( int i = 0; i < 20; i++ ) {
    for( int t = 0; t < 8; t++ ) {
      attacker->setSkill( t, (int)( (float)MAX_SKILL * rand() / RAND_MAX ) );
      defender->setSkill( t, (int)( (float)MAX_SKILL * rand() / RAND_MAX ) );
    }
    sprintf( filename, "rand_ability_%d.html", i );
    if( !fight( path, filename, session, attacker, weapon, defender ) ) break;
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

  fprintf( fp, "<table><tr>\n" );

  int atkOriginalLevel = attacker->getLevel();
  int defOriginalLevel = defender->getLevel();

  float skill, max, min;
  float sum=0, low=0, high=0, ATKave;
  for( int n = 0; n < SHOW_MAX_LEVEL; n++ ) {

    if( n && !( n % 3 ) ) {
      fprintf( fp, "</tr><tr>" );
    }

    int attackLevel = n + 1;
    attacker->setLevel( attackLevel );
    // adjust the skills
    setMinSkills( attacker );
    fprintf( fp, "<td><div style='background: gray; color: white;'>Attacker's Level: %d</div>\n", attackLevel );

    for( int i = 0; i < 2; i++ ) {

      defender->setLevel( !i ? 1 : attacker->getLevel() );
      setMinSkills( defender );
      fprintf( fp, "<div style='background: gray; color: white;'>Defender's Level: %d</div>\n", defender->getLevel() );

      // -------------------------------------------------
      // Attack roll
      sum = low = high = 0;
      attacker->getAttack( weapon, &max, &min, &skill );
      fprintf( fp, "ATK range:<b>%.2f - %.2f</b> (%% %.2f )<br>\n", 
               min, max, skill );
      for( int i = 0; i < count; i++ ) {
        computeHighLow( attacker->getAttack( weapon ),
                        &sum, &low, &high );
      }
      ATKave = sum / ((double)count);
      fprintf( fp, "ATK roll: <b>%.2f</b>(%.2f - %.2f)<br>\n", 
               ATKave, low, high );
      // -------------------------------------------------
      
      // -------------------------------------------------
      // Armor class at level 1
			float armor, dodgePenalty;
			defender->getArmor( &armor, &dodgePenalty, 
													weapon ? weapon->getRpgItem()->getDamageType() : 
													RpgItem::DAMAGE_TYPE_CRUSHING );
      fprintf( fp , "Armor: <span style='background: %s'><b>%.2f</b></span><br>\n",
							 ( ATKave > armor ? "red" : "green" ), armor );
			/*
      fprintf( fp , "Armor: <span style='background: %s'><b>%.2f</b></span>\
               (%.2f %% %.2f)<br>\n",
               ( ATKave > ac ? "red" : "green" ),
               armor, total, skill );
			*/
      // -------------------------------------------------
    }

    fprintf( fp, "</td>" );
    fprintf( fp, "\n" );
  }
  fprintf( fp, "</tr></table><br>\n" );

  // reset
  attacker->setLevel( atkOriginalLevel );
  defender->setLevel( defOriginalLevel );
  // adjust the skills
  setMinSkills( attacker );
  setMinSkills( defender );
  
  fprintf( fp, "<br><a name=\"details\"></a><b>Character details:</b>" );
  fprintf( fp, "<table><tr><td style='border: none;' valign=top>" );
  fprintf( fp, "Attacker: <b>%s</b> with %s(%d)<br>Level: %d<br>\n",
           attacker->getCharacter()->getName(),
           weapon->getRpgItem()->getName(),
           weapon->getLevel(),
           attacker->getLevel() );
  printInventory( fp, attacker );
  
  fprintf( fp, "</td><td style='border: none;' valign=top>" );
  fprintf( fp, "Defender: <b>%s</b><br>Level: %d<br>\n",
           defender->getCharacter()->getName(),
           defender->getLevel() );
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
  fprintf( fp, "<b>Skills:</b><table><tr><td>Name</td>\
           <td>Value</td><td>MIN-MAX</td></tr>\n" );
  char color[20];
  for( int i = 0; i < Skill::SKILL_COUNT; i++ ) {

    int n = creature->getSkill( i );
    if( n < MAX_SKILL / 4.0f ) strcpy( color, "red" );
    else if( n < MAX_SKILL / 2.0f ) strcpy( color, "orange" );
    else if( n < MAX_SKILL - MAX_SKILL / 4.0f ) strcpy( color, "white" );
    else strcpy( color, "green" );

    fprintf( fp, "<tr><td><b>%s</b></td>\
             <td bgcolor=%s>%d</td>", 
             Skill::skills[ i ]->getName(), color, n );    

    fprintf( fp, "</tr>\n" );
  }
  fprintf( fp, "</table>\n" );

  fprintf( fp, "<b>Inventory:</b><ul>\n" );
  for( int i = 0; i < creature->getInventoryCount(); i++ ) {
    fprintf( fp, "<li>%s(%d) A:%d %s<br>\n", 
             creature->getInventory( i )->getRpgItem()->getName(),
             creature->getInventory( i )->getLevel(),
             creature->getInventory( i )->getRpgItem()->getDamage(),
             ( creature->isEquipped( i ) ? "<i>Equipped</i>" : "" ) );
  }
  fprintf( fp, "</ul>\n" );
}

Creature *CombatTest::createCharacter( Session *session, 
                                       char *characterShortName,
                                       char *name,
                                       int level ) {
  Character *character = 
    Character::getRandomCharacter();
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
  
  setMinSkills( c );
  
  return c;
}

// FIXME: made to compile but is non-sensical
void CombatTest::setMinSkills( Creature *c ) {
  // starting skills
  //Character *character = c->getCharacter();
  for(int i = 0; i < Skill::SKILL_COUNT; i++) {
    int n = c->getLevel() * 10;
    if(n > 99) n = 99;
    c->setSkill( i, n );
  }
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

