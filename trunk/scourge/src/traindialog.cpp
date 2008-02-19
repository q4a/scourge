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

	list = new ScrollingList( 10, 75, w - 30, 120, scourge->getShapePalette()->getHighlightTexture() );
  win->addWidget( list );

	description = new ScrollingLabel( 10, 205, w - 30, 120, "" );
  win->addWidget( description );

  h = 20;
  int y = win->getHeight() - h - 30;
  closeButton = win->createButton( w - 80, y, w - 10, y + h, _( "Close" ) );
  applyButton = win->createButton( w - 160, y, w - 90, y + h, _( "Train!" ) );

}

TrainDialog::~TrainDialog() {
  delete win;	
}

void TrainDialog::setCreature( Creature *creature ) {
  this->creature = creature;
  updateUI();
  win->setVisible( true );
}

void TrainDialog::updateUI() {
	enum { S_SIZE = 255 };
	char s[ S_SIZE ];
  
  // level-based mark-up
	Creature *player = scourge->getParty()->getPlayer();
  int base = 150;
  int price = base + 
    (int)Util::getRandomSum( (float)(base / 2), creature->getNpcInfo()->level );
  // 25% variance based on leadership skill.
  float skill = (float)( player->getSkill( Skill::LEADERSHIP ) );
  int percentage = (int)( (float)price * ( 100.0f - skill ) / 100.0f * 0.25f );
  cost = price + percentage;

  snprintf( s, S_SIZE, "%s (%s %d) %s: %d", 
           _( creature->getName() ), 
					 _( "level" ),
           creature->getNpcInfo()->level,
					 _( "Cost" ),
           cost );
  creatureLabel->setText( s );

	// is the trainer high enough level?	
	list->setLines( 0, text );
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
		snprintf( s, S_SIZE, _( "%s, I cannot teach you. " ), player->getName() );
		errorLabel->setText( s );
		snprintf( s, S_SIZE, _( "You must seek out one who can train a %s." ), rc->getDisplayName() );
		errorLabel2->setText( s );
	} else if( creature->getNpcInfo()->level < player->getLevel() ) {
		errorLabel->setColor( 0, 1, 1 );
		errorLabel2->setColor( 0, 1, 1 );
		snprintf( s, S_SIZE, _( "%s, I can teach you no more. " ), player->getName() );
		errorLabel->setText( s );
		errorLabel2->setText( _( "You must seek a higher level trainer." ) );
	} else if( player->getCharacter()->getChildCount() == 0 ) {
		errorLabel->setColor( 1, 0, 1 );
		errorLabel2->setColor( 1, 0, 1 );
		snprintf( s, S_SIZE, _( "%s, I can teach you no more. " ), player->getName() );
		errorLabel->setText( s );
		errorLabel2->setText( _( "You must learn by yourself from now on." ) );
	} else if( player->getCharacter()->getChild( 0 )->getMinLevelReq() > 
						 player->getLevel() ) {
		errorLabel->setColor( 1, 1, 0 );
		errorLabel2->setColor( 1, 1, 0 );
		snprintf( s, S_SIZE, _( "%s, you are not yet ready." ), player->getName() );
		errorLabel->setText( s );
		snprintf( s, S_SIZE, _( "Come back when you've reached level %d." ), 
						 player->getCharacter()->getChild( 0 )->getMinLevelReq() );
		errorLabel2->setText( s );
	} else {
		errorLabel->setColor( 0, 1, 0 );
		errorLabel2->setColor( 0, 1, 0 );
		snprintf( s, S_SIZE, _( "%s, you are ready to learn." ), player->getName() );
		errorLabel->setText( s );
		errorLabel2->setText( _( "Select your next profession from the list below." ) );

		for( int i = 0; i < player->getCharacter()->getChildCount(); i++ ) {
			text[i] = player->getCharacter()->getChild(i)->getDisplayName();
			text[i] += " (";
			text[i] += _( "min level" );
			char str[20];
			snprintf( str, 20, " %d)", player->getCharacter()->getChild(i)->getMinLevelReq() );  
			text[i] += str;
		}
		list->setLines( player->getCharacter()->getChildCount(), text );
		description->setText( player->getCharacter()->getChildCount() > 0 ?
													player->getCharacter()->getChild( 0 )->getDescription() :
													"" );
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
		scourge->showMessageDialog( _( "You cannot afford the training!" ) );
		return;
	}

	player->setMoney( player->getMoney() - cost );
	player->changeProfession( newProfession );

	updateUI();

	char tmp[120];
	snprintf( tmp, 120, _( "Congratulation %1$s, you are now a %2$s." ),
					 player->getName(),
					 player->getCharacter()->getDisplayName() );
	scourge->showMessageDialog( tmp );
}

