/***************************************************************************
                          partyeditor.cpp  -  description
                             -------------------
    begin                : Tue Aug 12 2003
    copyright            : (C) 2003 by Gabor Torok
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

#include "partyeditor.h"
#include "render/renderlib.h"
#include "rpg/rpglib.h"
#include "item.h"
#include "creature.h"
#include "characterinfo.h"
#include "sdlhandler.h"
#include "sdleventhandler.h"
#include "sdlscreenview.h"
#include "scourge.h"
#include "userconfiguration.h"
#include "util.h"
#include "gui/window.h"
#include "gui/button.h"
#include "gui/canvas.h"
#include "gui/scrollinglist.h"
#include "gui/cardcontainer.h"
#include "gui/scrollinglabel.h"
#include "shapepalette.h"
#include "debug.h"
#include "skillsview.h"
#include "pceditor.h"

using namespace std;

/**
  *@author Gabor Torok
  */

#define PORTRAIT_SIZE 150
#define MODEL_SIZE 210
#define STARTING_PARTY_SIZE 1
#define LEVEL STARTING_PARTY_LEVEL

typedef struct _Preset {
  char name[80];
  int deity;
  int charClass;
  int portrait;
  int model;
} Preset;

Preset presets[] = {
  { "Lezidor",     4, 4, 7, 0 }, // rogue
  { "Kaz-Mokh",    1, 0, 8, 1 }, // fighter
  { "Arcoraxe",    2, 1, 6, 15 }, // mage
  { "Deiligiliam", 0, 2, 10, 7 }, // healer
};

PartyEditor::PartyEditor(Scourge *scourge) {
  this->scourge = scourge;
	pcEditor = new PcEditor( scourge );
  pcEditor->setCreature(); // use a temp. creature
}

PartyEditor::~PartyEditor() {
}

void PartyEditor::handleEvent( Widget *widget, SDL_Event *event ) {
  
  //
  // cancel and done are handled in mainmenu.cpp
  //

	if( pcEditor->getWindow()->isVisible() ) pcEditor->handleEvent( widget,event );
}

void PartyEditor::createParty( Creature **pc, int *partySize, bool addRandomInventory ) {
	for( int i = 0; i < STARTING_PARTY_SIZE; i++ )
		pc[i] = pcEditor->createPartyMember();
	if( addRandomInventory ) addStartingInventory( pc, STARTING_PARTY_SIZE );
	if( partySize ) *partySize = STARTING_PARTY_SIZE;
}

RenderedCreature *PartyEditor::createWanderingHero( int level ) {
	int sex = 4.0f * rand() / RAND_MAX >= 2.0f ? Constants::SEX_MALE : Constants::SEX_FEMALE;
	Creature *pc = scourge->getSession()->
		newCreature( Character::getRandomCharacter( level ),
								 Rpg::createName(), 
								 sex,
								 (int)( (float)(scourge->getShapePalette()->getCharacterModelInfoCount( sex )) * rand() / RAND_MAX ) );
	pc->setLevel( LEVEL ); 
	pc->setExp(0);
	pc->setHp();
	pc->setMp();
	pc->setHunger((int)(5.0f * rand()/RAND_MAX) + 5);
	pc->setThirst((int)(5.0f * rand()/RAND_MAX) + 5);

	// deity
	pc->setDeityIndex( MagicSchool::getRandomSchoolIndex() );
	
	// assign portraits
	pc->setPortraitTextureIndex( (int)( (float)(scourge->getShapePalette()->getPortraitCount( sex )) * rand() / RAND_MAX ) );
	
	// compute starting skill levels
	pcEditor->rollSkillsForCreature( pc );
	
	addStartingInventory( pc );

	pc->setMotion( Constants::MOTION_LOITER );

	return pc;
}

void PartyEditor::addStartingInventory( Creature **pc, int partySize ) {
  for( int i = 0; i < partySize; i++ ) {
		addStartingInventory( pc[i] );
		if( LEVEL > 1 && i == 0 ) {
			// add all special items
			for( int t = 0; t < RpgItem::getSpecialCount(); t++ ) {
				pc[i]->addInventory( scourge->getSession()->newItem( RpgItem::getSpecial( t ) ), true );
			}
			// add some spell-containing items
			for( int t = 0; t < 5; t++ ) {
				pc[i]->addInventory( 
					scourge->getSession()->newItem( 
						RpgItem::getItemByName( "Dwarven steel ring" ),
						1, 
						MagicSchool::getRandomSpell( 1 ) ), true );
			}
		}
  }
}

void PartyEditor::addStartingInventory( Creature *pc ) {
	// add a weapon anyone can wield
	int n = (int)(5.0f * rand()/RAND_MAX);
	switch(n) {
	case 0: pc->addInventory(scourge->getSession()->newItem( RpgItem::getItemByName("Smallbow"), LEVEL, NULL, true ), true); break;
	case 1: pc->addInventory(scourge->getSession()->newItem(RpgItem::getItemByName("Short sword"), LEVEL, NULL, true ), true); break;
	case 2: pc->addInventory(scourge->getSession()->newItem(RpgItem::getItemByName("Dagger"), LEVEL, NULL, true ), true ); break;
	case 3: pc->addInventory(scourge->getSession()->newItem(RpgItem::getItemByName("Wooden club"), LEVEL, NULL, true ), true ); break;
	case 4: pc->addInventory(scourge->getSession()->newItem(RpgItem::getItemByName("Quarter Staff"), LEVEL, NULL, true ), true ); break;
	}
	int invIndex = 0;
	pc->equipInventory( invIndex++ );

	// add some armor
	if(0 == (int)(4.0f * rand()/RAND_MAX)) {
		pc->addInventory(scourge->getSession()->newItem(RpgItem::getItemByName("Horned helmet"), LEVEL, NULL, true ), true);
		pc->equipInventory( invIndex++ );
	}
	if(0 == (int)(3.0f * rand()/RAND_MAX)) {
		pc->addInventory(scourge->getSession()->newItem(RpgItem::getItemByName("Buckler"), LEVEL, NULL, true ), true);
		pc->equipInventory( invIndex++ );
	}

	// some potions
	if(0 == (int)(4.0f * rand()/RAND_MAX))
		pc->addInventory(scourge->getSession()->newItem(RpgItem::getItemByName("Health potion"), LEVEL ), true);  
	if(0 == (int)(4.0f * rand()/RAND_MAX))
		pc->addInventory(scourge->getSession()->newItem(RpgItem::getItemByName("Magic potion"), LEVEL ), true);  
	if(0 == (int)(4.0f * rand()/RAND_MAX))
		pc->addInventory(scourge->getSession()->newItem(RpgItem::getItemByName("Liquid armor"), LEVEL ), true);  

	// some food
	for(int t = 0; t < (int)(6.0f * rand()/RAND_MAX); t++) {
		if(0 == (int)(4.0f * rand()/RAND_MAX))
			pc->addInventory(scourge->getSession()->newItem(RpgItem::getItemByName("Apple")), true);
		if(0 == (int)(4.0f * rand()/RAND_MAX))
			pc->addInventory(scourge->getSession()->newItem(RpgItem::getItemByName("Bread")), true);
		if(0 == (int)(4.0f * rand()/RAND_MAX))
		pc->addInventory(scourge->getSession()->newItem(RpgItem::getItemByName("Mushroom")), true);
		if(0 == (int)(4.0f * rand()/RAND_MAX))
			pc->addInventory(scourge->getSession()->newItem(RpgItem::getItemByName("Big egg")), true);
		if(0 == (int)(4.0f * rand()/RAND_MAX))
			pc->addInventory(scourge->getSession()->newItem(RpgItem::getItemByName("Mutton meat")), true);
	}

	// some spells
	if(pc->getMaxMp() > 0) {
		// useful spells
		pc->addSpell(Spell::getSpellByName("Flame of Azun"));
		pc->addSpell(Spell::getSpellByName("Ole Taffy's purty colors"));
		// attack spell
		if(0 == (int)(2.0f * rand()/RAND_MAX))
			pc->addSpell(Spell::getSpellByName("Silent knives"));
		else
			pc->addSpell(Spell::getSpellByName("Stinging light"));
		// defensive spell
		if(0 == (int)(2.0f * rand()/RAND_MAX))
			pc->addSpell(Spell::getSpellByName("Lesser healing touch"));
		else
			pc->addSpell(Spell::getSpellByName("Body of stone"));


		// testing
		if( LEVEL > 1 ) {
			pc->addSpell(Spell::getSpellByName("Ring of Harm"));
			pc->addSpell(Spell::getSpellByName("Malice Storm"));
			pc->addSpell(Spell::getSpellByName("Unholy Decimator"));
			pc->addSpell(Spell::getSpellByName("Remove curse"));
			pc->addSpell(Spell::getSpellByName("Teleportation"));
			pc->addSpell(Spell::getSpellByName("Recall to life"));
			pc->addSpell(Spell::getSpellByName("Blast of Fury"));        
			pc->addSpell(Spell::getSpellByName("Dori's Tumblers"));        
			pc->addSpell(Spell::getSpellByName("Gust of wind"));        
			pc->setMp( 5000 );
			pc->setMoney( 10000 );
		}
	}
}

bool PartyEditor::isVisible() { 
  return pcEditor->getWindow()->isVisible(); 
}

void PartyEditor::setVisible( bool b ) { 
  //mainWin->setVisible( b ); 
	pcEditor->getWindow()->setVisible( b );
}

Creature *PartyEditor::getHighestSkillPC( int skill ) {
  return( maxSkills.find( skill ) == maxSkills.end() ? NULL : maxSkills[ skill ] );
}

Button *PartyEditor::getStartGameButton() {
	return pcEditor->getOkButton();
}

Button *PartyEditor::getCancelButton() {
	return pcEditor->getCancelButton();
}

