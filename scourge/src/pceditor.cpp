/***************************************************************************
                          pceditor.cpp  -  description
                             -------------------
    begin                : Tue Jul 10 2006
    copyright            : (C) 2006 by Gabor Torok
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
#include "pceditor.h"
#include "gui/window.h"
#include "gui/cardcontainer.h"
#include "gui/button.h"
#include "gui/textfield.h"
#include "gui/scrollinglabel.h"
#include "gui/scrollinglist.h"
#include "gui/canvas.h"
#include "scourge.h"
#include "shapepalette.h"
#include "characterinfo.h"
#include "rpg/character.h"

using namespace std;

PcEditor::PcEditor( Scourge *scourge ) {
	this->scourge = scourge;

	int x = 50;
	int y = 30;
	int w = 500;
	int h = 420;

	win = new Window( scourge->getSDLHandler(),
										x, y, w, h,
										"Character Editor", 
										false, 
										Window::BASIC_WINDOW, 
										"default" );
  win->setVisible( false );
  win->setModal( true );  

	x = 10;	
	int buttonHeight = 17;
	int buttonSpace = 5;
	y = buttonSpace;
	int firstColWidth = 115;
	int secondColStart = firstColWidth + 20;
	int secondColWidth = w - secondColStart - 20;

	nameButton = win->createButton( x, y, firstColWidth, y + buttonHeight, "Name", true );
	y += buttonHeight + buttonSpace;
	profButton = win->createButton( x, y, firstColWidth, y + buttonHeight, "Profession", true );
	y += buttonHeight + buttonSpace;
	statsButton = win->createButton( x, y, firstColWidth, y + buttonHeight, "Stats", true );
	y += buttonHeight + buttonSpace;
	deityButton = win->createButton( x, y, firstColWidth, y + buttonHeight, "Deity", true );
	y += buttonHeight + buttonSpace;
	imageButton = win->createButton( x, y, firstColWidth, y + buttonHeight, "Image", true );
	y += buttonHeight + buttonSpace;
	okButton = win->createButton( x, 
																h - x - Window::TOP_HEIGHT - Window::BOTTOM_HEIGHT - buttonHeight, 
																firstColWidth, 
																h - x - Window::TOP_HEIGHT - Window::BOTTOM_HEIGHT, 
																"Ok" );
	cancelButton = win->createButton( x + firstColWidth + buttonSpace, 
																		h - x - Window::TOP_HEIGHT - Window::BOTTOM_HEIGHT - buttonHeight, 
																		firstColWidth + buttonSpace + firstColWidth, 
																		h - x - Window::TOP_HEIGHT - Window::BOTTOM_HEIGHT, 
																		"Cancel" );

	cards = new CardContainer( win );  


	// ----------------------------------------------
	// name
	Label *p = cards->createLabel( secondColStart, 30, "Name:", NAME_TAB );
	p->setFontType( Constants::SCOURGE_LARGE_FONT );
	nameField = new TextField( secondColStart, 50, 32 );
	cards->addWidget( nameField, NAME_TAB );
	cards->addWidget( new ScrollingLabel( secondColStart, 80, 
																				secondColWidth, 50, 
																				"Enter a name for this character." ), 
										NAME_TAB );


	// ----------------------------------------------
	// class
	p = cards->createLabel( secondColStart, 30, "Profession:", CLASS_TAB );
	p->setFontType( Constants::SCOURGE_LARGE_FONT );


  charType = new ScrollingList( secondColStart, 50, 
																secondColWidth, 80, 
																scourge->getShapePalette()->getHighlightTexture() );
  cards->addWidget( charType, CLASS_TAB );
  charTypeStr = (char**)malloc( Character::rootCharacters.size() * sizeof(char*));
  for(int i = 0; i < (int)Character::rootCharacters.size(); i++) {
    charTypeStr[i] = (char*)malloc( 120 * sizeof(char) );
    strcpy( charTypeStr[i], Character::rootCharacters[i]->getName() );
  }
  charType->setLines( (int)Character::rootCharacters.size(), (const char**)charTypeStr );
  int charIndex = (int)( (float)( Character::rootCharacters.size() ) * rand()/RAND_MAX );
  charType->setSelectedLine( charIndex );
  charTypeDescription = new ScrollingLabel( secondColStart, 140, 
																						secondColWidth, 150, 
																						Character::rootCharacters[charIndex]->getDescription() );
	cards->addWidget( charTypeDescription, CLASS_TAB );


	// ----------------------------------------------
	// stats
	p = cards->createLabel( secondColStart, 30, "Statistics:", STAT_TAB );
	p->setFontType( Constants::SCOURGE_LARGE_FONT );

	int n = 0;
	for( int i = 0; n < 10 && i < (int)Skill::skills.size(); i++ ) {
		Skill *skill = Skill::skills[i];
		if( skill->getGroup()->isStat() ) {
			y = 60 + n * buttonHeight;
			cards->createLabel( secondColStart, y, skill->getName(), STAT_TAB );
			skillValue[n] = cards->createLabel( secondColStart + 120, y, 
																					"0", 
																					STAT_TAB );
			skillPlus[n] = cards->createButton( secondColStart + 150, y - 10,
																					secondColStart + 150 + 25, y - 10 + buttonHeight,
																					"+", 
																					STAT_TAB );
			skillPlus[n]->setFontType( Constants::SCOURGE_DEFAULT_FONT );
			skillMinus[n] = cards->createButton( secondColStart + 180, y - 10,
																					 secondColStart + 180 + 25, y - 10 + buttonHeight,
																					 "-", 
																					 STAT_TAB );
			skillMinus[n]->setFontType( Constants::SCOURGE_DEFAULT_FONT );

			n++;
		}
	}
	cards->createLabel( secondColStart + 180 + 25 + 15, 60, "Points Remaining:", STAT_TAB );
	remainingLabel = cards->createLabel( secondColStart + 180 + 25 + 15, 80, "0", STAT_TAB );

	int detailsHeight = 145;
  detailsInfo = new CharacterInfoUI( scourge );
  detailsCanvas = new Canvas( secondColStart, 200, 
															secondColStart + secondColWidth, 200 + detailsHeight, 
															detailsInfo );
  cards->addWidget( detailsCanvas, STAT_TAB );



	// ----------------------------------------------
	// deity
	p = cards->createLabel( secondColStart, 30, "Patron Deity:", DEITY_TAB );
	p->setFontType( Constants::SCOURGE_LARGE_FONT );


	// ----------------------------------------------
	// appearence
	p = cards->createLabel( secondColStart, 30, "Appearance:", IMAGE_TAB );
	p->setFontType( Constants::SCOURGE_LARGE_FONT );
}

PcEditor::~PcEditor() {
	delete win;
}

void PcEditor::setCreature( Creature *creature ) {
	this->creature = creature;
	cards->setActiveCard( NAME_TAB );
	nameButton->setSelected( true );
	profButton->setSelected( false );
	statsButton->setSelected( false );
	deityButton->setSelected( false );
	imageButton->setSelected( false );
	win->setVisible( true );
}

void PcEditor::handleEvent( Widget *widget, SDL_Event *event ) {
	if( widget == cancelButton ) {
		win->setVisible( false );
	} else if( widget == okButton ) {
		win->setVisible( false );
	} else if( widget == nameButton ) {
		cards->setActiveCard( NAME_TAB );
		nameButton->setSelected( true );
		profButton->setSelected( false );
		statsButton->setSelected( false );
		deityButton->setSelected( false );
		imageButton->setSelected( false );
	} else if( widget == profButton ) {
		cards->setActiveCard( CLASS_TAB );
		nameButton->setSelected( false );
		profButton->setSelected( true );
		statsButton->setSelected( false );
		deityButton->setSelected( false );
		imageButton->setSelected( false );
	} else if( widget == statsButton ) {
		cards->setActiveCard( STAT_TAB );
		nameButton->setSelected( false );
		profButton->setSelected( false );
		statsButton->setSelected( true );
		deityButton->setSelected( false );
		imageButton->setSelected( false );
	} else if( widget == deityButton ) {
		cards->setActiveCard( DEITY_TAB );
		nameButton->setSelected( false );
		profButton->setSelected( false );
		statsButton->setSelected( false );
		deityButton->setSelected( true );
		imageButton->setSelected( false );
	} else if( widget == imageButton ) {
		cards->setActiveCard( IMAGE_TAB );
		nameButton->setSelected( false );
		profButton->setSelected( false );
		statsButton->setSelected( false );
		deityButton->setSelected( false );
		imageButton->setSelected( true );
	}
}
