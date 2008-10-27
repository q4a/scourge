/***************************************************************************
                partyeditor.cpp  - The "create character" window
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

#include "common/constants.h"
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

/// Unused.
struct Preset {
	char name[80];
	int deity;
	int charClass;
	int portrait;
	int model;
};

Preset presets[] = {
	{ "Lezidor",     4, 4, 7, 0 }, // rogue
	{ "Kaz-Mokh",    1, 0, 8, 1 }, // fighter
	{ "Arcoraxe",    2, 1, 6, 15 }, // mage
	{ "Deiligiliam", 0, 2, 10, 7 }, // healer
};

PartyEditor::PartyEditor( Scourge *scourge ) {
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

	if ( pcEditor->getWindow()->isVisible() ) pcEditor->handleEvent( widget, event );
}

void PartyEditor::createParty( Creature **pc, int *partySize, bool addRandomBackpack ) {
	for ( int i = 0; i < STARTING_PARTY_SIZE; i++ )
		pc[i] = pcEditor->createPartyMember();
	if ( addRandomBackpack ) addStartingBackpack( pc, STARTING_PARTY_SIZE );
	if ( partySize ) *partySize = STARTING_PARTY_SIZE;
}

RenderedCreature *PartyEditor::createWanderingHero( int level ) {
	int sex = Util::dice( 2 ) ? Constants::SEX_MALE : Constants::SEX_FEMALE;
	Creature *pc = scourge->getSession()->
	               newCreature( Character::getRandomCharacter( level ),
				                Rpg::createName().c_str(),
	                            sex,
	                            Util::dice( scourge->getShapePalette()->getCharacterModelInfoCount( sex ) ) );
	pc->setLevel( LEVEL );
	pc->setExp( 0 );
	pc->setHp();
	pc->setMp();
	pc->setHunger( Util::pickOne( 5, 9 ) );
	pc->setThirst( Util::pickOne( 5, 9 ) );

	// deity
	pc->setDeityIndex( MagicSchool::getRandomSchoolIndex() );

	// assign portraits
	pc->setPortraitTextureIndex( Util::dice( scourge->getShapePalette()->getPortraitCount( sex ) ) );

	// compute starting skill levels
	pcEditor->rollSkillsForCreature( pc );

	addStartingBackpack( pc );

	pc->setMotion( Constants::MOTION_LOITER );

	return pc;
}

void PartyEditor::addStartingBackpack( Creature **pc, int partySize ) {
	for ( int i = 0; i < partySize; i++ ) {
		addStartingBackpack( pc[i] );
		if ( LEVEL > 1 && i == 0 ) {
			// add all special items
			for ( int t = 0; t < RpgItem::getSpecialCount(); t++ ) {
				pc[i]->addToBackpack( scourge->getSession()->newItem( RpgItem::getSpecial( t ) ), true );
			}
			// add some spell-containing items
			for ( int t = 0; t < 5; t++ ) {
				pc[i]->addToBackpack(
				  scourge->getSession()->newItem(
				    RpgItem::getItemByName( "Dwarven steel ring" ),
				    1,
				    MagicSchool::getRandomSpell( 1 ) ), true );
			}
		}
	}
}

void PartyEditor::addStartingBackpack( Creature *pc ) {
	// add a weapon anyone can wield
	int n = Util::dice( 5 );
	switch ( n ) {
	case 0: pc->addToBackpack( scourge->getSession()->newItem( RpgItem::getItemByName( "Smallbow" ), LEVEL, NULL, true ), true ); break;
	case 1: pc->addToBackpack( scourge->getSession()->newItem( RpgItem::getItemByName( "Short sword" ), LEVEL, NULL, true ), true ); break;
	case 2: pc->addToBackpack( scourge->getSession()->newItem( RpgItem::getItemByName( "Dagger" ), LEVEL, NULL, true ), true ); break;
	case 3: pc->addToBackpack( scourge->getSession()->newItem( RpgItem::getItemByName( "Wooden club" ), LEVEL, NULL, true ), true ); break;
	case 4: pc->addToBackpack( scourge->getSession()->newItem( RpgItem::getItemByName( "Quarter Staff" ), LEVEL, NULL, true ), true ); break;
	}
	int invIndex = 0;
	pc->equipFromBackpack( invIndex++ );

	// add some armor
	if ( 0 == Util::dice( 4 ) ) {
		pc->addToBackpack( scourge->getSession()->newItem( RpgItem::getItemByName( "Horned helmet" ), LEVEL, NULL, true ), true );
		pc->equipFromBackpack( invIndex++ );
	}
	if ( 0 == Util::dice( 3 ) ) {
		pc->addToBackpack( scourge->getSession()->newItem( RpgItem::getItemByName( "Buckler" ), LEVEL, NULL, true ), true );
		pc->equipFromBackpack( invIndex++ );
	}

	// some potions
	if ( 0 == Util::dice( 4 ) )
		pc->addToBackpack( scourge->getSession()->newItem( RpgItem::getItemByName( "Health potion" ), LEVEL ), true );
	if ( 0 == Util::dice( 4 ) )
		pc->addToBackpack( scourge->getSession()->newItem( RpgItem::getItemByName( "Magic potion" ), LEVEL ), true );
	if ( 0 == Util::dice( 4 ) )
		pc->addToBackpack( scourge->getSession()->newItem( RpgItem::getItemByName( "Liquid armor" ), LEVEL ), true );

	// some food
	for ( int t = 0; t < Util::dice( 6 ); t++ ) {
		if ( 0 == Util::dice( 4 ) )
			pc->addToBackpack( scourge->getSession()->newItem( RpgItem::getItemByName( "Apple" ) ), true );
		if ( 0 == Util::dice( 4 ) )
			pc->addToBackpack( scourge->getSession()->newItem( RpgItem::getItemByName( "Bread" ) ), true );
		if ( 0 == Util::dice( 4 ) )
			pc->addToBackpack( scourge->getSession()->newItem( RpgItem::getItemByName( "Mushroom" ) ), true );
		if ( 0 == Util::dice( 4 ) )
			pc->addToBackpack( scourge->getSession()->newItem( RpgItem::getItemByName( "Big egg" ) ), true );
		if ( 0 == Util::dice( 4 ) )
			pc->addToBackpack( scourge->getSession()->newItem( RpgItem::getItemByName( "Mutton meat" ) ), true );
	}

	// some spells
	if ( pc->getMaxMp() > 0 ) {
		// useful spells
		pc->addSpell( Spell::getSpellByName( "Flame of Azun" ) );
		pc->addSpell( Spell::getSpellByName( "Ole Taffy's purty colors" ) );
		// attack spell
		if ( 0 == Util::dice( 2 ) )
			pc->addSpell( Spell::getSpellByName( "Silent knives" ) );
		else
			pc->addSpell( Spell::getSpellByName( "Stinging light" ) );
		// defensive spell
		if ( 0 == Util::dice( 2 ) )
			pc->addSpell( Spell::getSpellByName( "Lesser healing touch" ) );
		else
			pc->addSpell( Spell::getSpellByName( "Body of stone" ) );


		// testing
		if ( LEVEL > 1 ) {
			pc->addSpell( Spell::getSpellByName( "Ring of Harm" ) );
			pc->addSpell( Spell::getSpellByName( "Malice Storm" ) );
			pc->addSpell( Spell::getSpellByName( "Unholy Decimator" ) );
			pc->addSpell( Spell::getSpellByName( "Remove curse" ) );
			pc->addSpell( Spell::getSpellByName( "Teleportation" ) );
			pc->addSpell( Spell::getSpellByName( "Recall to life" ) );
			pc->addSpell( Spell::getSpellByName( "Blast of Fury" ) );
			pc->addSpell( Spell::getSpellByName( "Dori's Tumblers" ) );
			pc->addSpell( Spell::getSpellByName( "Gust of wind" ) );
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
	return NULL; // never filled: ( maxSkills.find( skill ) == maxSkills.end() ? NULL : maxSkills[ skill ] );
}

Button *PartyEditor::getStartGameButton() {
	return pcEditor->getOkButton();
}

Button *PartyEditor::getCancelButton() {
	return pcEditor->getCancelButton();
}

