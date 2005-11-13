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
  vector<Item*> items;
  
  // -----------------------------------------------------------
  // The attacker
  Creature *attacker = createCharacter( session, "RA", "Attacker", 1 );
  Item *weapon = equipItem( session, attacker, "Dirk", 1 );
  items.push_back( weapon );
  
  // The defender
  Creature *defender = createCharacter( session, "RA", "Defender", 1 );
  items.push_back( equipItem( session, defender, "Horned helmet", 1 ) );
  items.push_back( equipItem( session, defender, "Leather Armor", 1 ) );

  // fight!
  bool ret = true;
  char filename[80];
  for( int i = 0; i < RpgItem::itemCount; i++ ) {
    if( !RpgItem::getItem( i )->isWeapon() ) continue;
    attacker->removeInventory( 0 );
    Item *weapon = equipItem( session, attacker, RpgItem::getItem( i )->getName(), 1 );
    items.push_back( weapon );
    sprintf( filename, "%s.html", RpgItem::getItem( i )->getName() );
    if( !fight( path, filename, session, attacker, weapon, defender ) ) break;
  }

  // cleanup
  for( int i = 0; i < (int)items.size(); i++ ) {
    Item *item = items[ i ];
    delete item;
  }
  delete attacker;
  delete defender;

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

  int sum=0, low=0, high=0;
  double ave;
  for( int attackLevel = 1; attackLevel < SHOW_MAX_LEVEL; attackLevel++ ) {
    attacker->setLevel( attackLevel );
    fprintf( fp, "<tr><td>%d</td>\n", attackLevel );

    for( int defendLevel = 1; defendLevel < SHOW_MAX_LEVEL; defendLevel++ ) {
      defender->setLevel( defendLevel );
      fprintf( fp, "<td bgcolor=%s>", ( defendLevel == attackLevel ? "gray" : "white" ) );

      sum = low = high = 0;
      for( int i = 0; i < count; i++ ) {
        computeHighLow( attacker->getToHit( weapon ),
                        &sum, &low, &high );
      }
      ave = ((double)sum) / ((double)count);
      fprintf( fp, "ATK:<span style='background: %s'>%d/%d/%.2f</span><br>\
               AC :%d/<b>%d</b><br>\n",
               ( ave > (double)(defender->getSkillModifiedArmor()) ? "green" : "red" ),
               low, high, ave,
               defender->getArmor(), defender->getSkillModifiedArmor() );

      sum = low = high = 0;
      for( int i = 0; i < count; i++ ) {
        computeHighLow( attacker->getDamage( weapon ),
                        &sum, &low, &high );
      }
      ave = ((double)sum) / ((double)count);
      fprintf( fp, "DAM:%d/%d/%.2f\n",
               low, high, ave );

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

void CombatTest::computeHighLow( int value, int *sum, int *low, int *high ) {
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
    c->setSkill( t, ( t < 8 ? 10 : 50 ) );
  }
  
  /*
    // add a weapon anyone can wield
    int n = (int)(4.0f * rand()/RAND_MAX);
    switch(n) {
    case 0: c->addInventory(scourge->getSession()->newItem(RpgItem::getItemByName("Smallbow"))); break;
    case 1: c->addInventory(scourge->getSession()->newItem(RpgItem::getItemByName("Short sword"))); break;
    case 2: c->addInventory(scourge->getSession()->newItem(RpgItem::getItemByName("Dagger"))); break;
    case 3: c->addInventory(scourge->getSession()->newItem(RpgItem::getItemByName("Wooden club"))); break;
    }
    c->equipInventory(0);
    */
    
    /*
    // add some armor
    if(0 == (int)(4.0f * rand()/RAND_MAX)) {
      c->addInventory(scourge->getSession()->newItem(RpgItem::getItemByName("Horned helmet")));
      c->equipInventory(1);
    }
    
    // some potions
    if(0 == (int)(4.0f * rand()/RAND_MAX))
      c->addInventory(scourge->getSession()->newItem(RpgItem::getItemByName("Health potion")));  
    if(0 == (int)(4.0f * rand()/RAND_MAX))
      c->addInventory(scourge->getSession()->newItem(RpgItem::getItemByName("Magic potion")));  
    if(0 == (int)(4.0f * rand()/RAND_MAX))
      c->addInventory(scourge->getSession()->newItem(RpgItem::getItemByName("Liquid armor")));  
    
    // some food
    for(int t = 0; t < (int)(6.0f * rand()/RAND_MAX); t++) {
      if(0 == (int)(4.0f * rand()/RAND_MAX))
        c->addInventory(scourge->getSession()->newItem(RpgItem::getItemByName("Apple")));
      if(0 == (int)(4.0f * rand()/RAND_MAX))
        c->addInventory(scourge->getSession()->newItem(RpgItem::getItemByName("Bread")));
      if(0 == (int)(4.0f * rand()/RAND_MAX))
      c->addInventory(scourge->getSession()->newItem(RpgItem::getItemByName("Mushroom")));
      if(0 == (int)(4.0f * rand()/RAND_MAX))
        c->addInventory(scourge->getSession()->newItem(RpgItem::getItemByName("Big egg")));
      if(0 == (int)(4.0f * rand()/RAND_MAX))
        c->addInventory(scourge->getSession()->newItem(RpgItem::getItemByName("Mutton meat")));
    }
    
    // some spells
    if(c->getMaxMp() > 0) {
      // useful spells
      c->addSpell(Spell::getSpellByName("Flame of Azun"));
      c->addSpell(Spell::getSpellByName("Ole Taffy's purty colors"));
      // attack spell
      if(0 == (int)(2.0f * rand()/RAND_MAX))
        c->addSpell(Spell::getSpellByName("Silent knives"));
      else
        c->addSpell(Spell::getSpellByName("Stinging light"));
      // defensive spell
      if(0 == (int)(2.0f * rand()/RAND_MAX))
        c->addSpell(Spell::getSpellByName("Lesser healing touch"));
      else
        c->addSpell(Spell::getSpellByName("Body of stone"));


      // testing
      if( LEVEL > 1 ) {
        c->addSpell(Spell::getSpellByName("Ring of Harm"));
        c->addSpell(Spell::getSpellByName("Malice Storm"));
        c->addSpell(Spell::getSpellByName("Unholy Decimator"));
        c->addSpell(Spell::getSpellByName("Remove curse"));
        c->addSpell(Spell::getSpellByName("Teleportation"));
        c->setMp( 5000 );
      }
    }
    */
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

