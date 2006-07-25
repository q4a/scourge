/***************************************************************************
  traindialog.cpp  -  description
-------------------
    begin                : 9/9/2005
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

#include "traindialog.h"
#include "scourge.h"
#include "shapepalette.h"
#include "creature.h"
#include "rpg/rpglib.h"
#include "gui/window.h"
#include "gui/button.h"
#include "gui/textfield.h"
#include "gui/scrollinglabel.h"
#include "gui/scrollinglist.h"

using namespace std;

#define MAX_TEXT_COUNT 100

TrainDialog::TrainDialog( Scourge *scourge ) {
  this->scourge = scourge;
  this->creature = NULL;
  int w = 400;
  int h = 400;
  win = 
    scourge->createWindow( 50, 50, 
                           w, h, 
                           Constants::getMessage( Constants::TRAIN_DIALOG_TITLE ) );
  creatureLabel = win->createLabel( 10, 15, "" );
	errorLabel = win->createLabel( 10, 45, "" );
	errorLabel->setSpecialColor();
	errorLabel2 = win->createLabel( 10, 60, "" );
	errorLabel2->setSpecialColor();

	text = (char**)malloc( MAX_TEXT_COUNT * sizeof(char*) );
  for(int i = 0; i < MAX_TEXT_COUNT; i++) {
    text[i] = (char*)malloc( 120 * sizeof( char ) );
  }
	list = new ScrollingList( 10, 75, w - 30, 120, scourge->getShapePalette()->getHighlightTexture() );
  win->addWidget( list );

	description = new ScrollingLabel( 10, 205, w - 30, 120, "" );
  win->addWidget( description );

  h = 20;
  int y = win->getHeight() - h - 10;
  closeButton = win->createButton( w - 80, y, w - 10, y + h, "Close" );
  applyButton = win->createButton( w - 160, y, w - 90, y + h, "Train!" );

}

TrainDialog::~TrainDialog() {
  delete win;	
  for(int i = 0; i < MAX_TEXT_COUNT; i++) {
    free( text[i] );
  }
	free( text );
}

void TrainDialog::setCreature( Creature *creature ) {
  this->creature = creature;
  updateUI();
  win->setVisible( true );
}

void TrainDialog::updateUI() {
  char s[255];
  
  // level-based mark-up
	Creature *player = scourge->getParty()->getPlayer();
  int base = 150;
  int price = base + 
    (int)Util::getRandomSum( (float)(base / 2), creature->getNpcInfo()->level );
  // 25% variance based on leadership skill.
  float skill = (float)( player->getSkill( Skill::LEADERSHIP ) );
  int percentage = (int)( (float)price * ( 100.0f - skill ) / 100.0f * 0.25f );
  cost = price + percentage;

  sprintf( s, "%s (level %d) Cost: %d", 
           creature->getName(), 
           creature->getNpcInfo()->level,
           cost );
  creatureLabel->setText( s );

	// is the trainer high enough level?	
	list->setLines( 0, (const char**)text );
	description->setText( "" );
	
	// does this trainer teach your profession?
	Character *rc = player->getCharacter();
	while( rc->getParent() ) {
		rc = rc->getParent();
	}
	int index = Character::getRootCharacterIndexByName( rc->getName() );
	if( creature->getNpcInfo()->getSubtype()->find( index ) == 
			creature->getNpcInfo()->getSubtype()->end() ) {
		errorLabel->setColor( 1, 0, 0 );
		errorLabel2->setColor( 1, 0, 0 );
		sprintf( s, "%s, I cannot teach you. ", player->getName() );
		errorLabel->setText( s );
		sprintf( s, "You must seek out one who can train a %s.", rc->getName() );
		errorLabel2->setText( s );
	} else if( creature->getNpcInfo()->level < player->getLevel() ) {
		errorLabel->setColor( 0, 1, 1 );
		errorLabel2->setColor( 0, 1, 1 );
		sprintf( s, "%s, I can teach you no more. ", player->getName() );
		errorLabel->setText( s );
		errorLabel2->setText( "You must seek a higher level trainer." );
	} else if( player->getCharacter()->getChildCount() == 0 ) {
		errorLabel->setColor( 1, 0, 1 );
		errorLabel2->setColor( 1, 0, 1 );
		sprintf( s, "%s, I can teach you no more. ", player->getName() );
		errorLabel->setText( s );
		errorLabel2->setText( "You must learn by yourself from now on." );
	} else if( player->getCharacter()->getChild( 0 )->getMinLevelReq() > 
						 player->getLevel() ) {
		errorLabel->setColor( 1, 1, 0 );
		errorLabel2->setColor( 1, 1, 0 );
		sprintf( s, "%s, you are not yet ready.", player->getName() );
		errorLabel->setText( s );
		sprintf( s, "Come back when you've reached level %d.", 
						 player->getCharacter()->getChild( 0 )->getMinLevelReq() );
		errorLabel2->setText( s );
	} else {
		errorLabel->setColor( 0, 1, 0 );
		errorLabel2->setColor( 0, 1, 0 );
		sprintf( s, "%s, you are ready to learn.", player->getName() );
		errorLabel->setText( s );
		errorLabel2->setText( "Select your next profession from the list below." );

		for( int i = 0; i < player->getCharacter()->getChildCount(); i++ ) {
			sprintf( text[i], "%s (min level %d)", 
							 player->getCharacter()->getChild(i)->getName(),
							 player->getCharacter()->getChild(i)->getMinLevelReq() );
		}
		list->setLines( player->getCharacter()->getChildCount(), (const char**)text );
		description->setText( player->getCharacter()->getChildCount() > 0 ?
													player->getCharacter()->getChild( 0 )->getDescription() :
													(char*)"" );
	}
}

void TrainDialog::handleEvent( Widget *widget, SDL_Event *event ) {
  if( widget == closeButton || widget == win->closeButton ) {
    win->setVisible( false );
	} else if( widget == list ) {
		int n = list->getSelectedLine();
		if( n >= 0 ) {
			description->setText( scourge->getParty()->getPlayer()->getCharacter()->getChild( n )->getDescription() );
		}
  } else if( widget == applyButton ) {
		int n = list->getSelectedLine();
		if( n >= 0 ) {
			train( scourge->getParty()->getPlayer()->getCharacter()->getChild( n ) );
		}
  }
}
  
void TrainDialog::train( Character *newProfession ) {
	Creature *player = scourge->getParty()->getPlayer();
	if( player->getMoney() < cost ) {
		scourge->showMessageDialog( "You cannot afford the training!" );
		return;
	}

	player->setMoney( player->getMoney() - cost );
	player->changeProfession( newProfession );

	updateUI();
	scourge->getInventory()->refresh();

	char tmp[120];
	sprintf( tmp, "Congratulation %s, you are now a %s.",
					 player->getName(),
					 player->getCharacter()->getName() );
	scourge->showMessageDialog( tmp );
}

