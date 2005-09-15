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
#include "creature.h"
#include "rpg/rpglib.h"
#include "gui/window.h"
#include "gui/button.h"
#include "gui/textfield.h"
#include "gui/scrollinglabel.h"

#define POINTS_AVAILABLE "Points Available:"

TrainDialog::TrainDialog( Scourge *scourge ) {
  this->scourge = scourge;
  this->creature = NULL;
  int w = 400;
  int h = 200;
  win = 
    scourge->createWindow( 50, 50, 
                           w, h, 
                           Constants::getMessage( Constants::TRAIN_DIALOG_TITLE ) );
  creatureLabel = win->createLabel( 10, 15, "" );
  pointsLabel = win->createLabel( 10, 30, POINTS_AVAILABLE );
  applyButton = win->createButton( 320, 45, 390, 70, "Train!" );
  
  result = new ScrollingLabel( 10, 75, w - 20, 65, "" );
  win->addWidget( result );

  h = 20;
  int y = win->getHeight() - Window::BOTTOM_HEIGHT - Window::TOP_HEIGHT - h - 10;
  closeButton = win->createButton( w - 80, y, w - 10, y + h, "Close" );
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
  char s[255];
  sprintf( s, "%s (level %d)", 
           creature->getName(), 
           creature->getNpcInfo()->level );
  creatureLabel->setText( s );
  sprintf( s, "%s %d", 
           POINTS_AVAILABLE, 
           scourge->getParty()->getPlayer()->getAvailableSkillPoints() );
  pointsLabel->setText( s );

  result->setText( "" );
}

void TrainDialog::handleEvent( Widget *widget, SDL_Event *event ) {
  if( widget == closeButton || widget == win->closeButton ) {
    win->setVisible( false );
  } else if( widget == applyButton ) {
    train();
  }
}
  
void TrainDialog::train() {
  if( scourge->getParty()->getPlayer()->getAvailableSkillPoints() <= 0 ) {
    scourge->showMessageDialog( "Select a player who has skill points." );
    return;
  }

  // take a point
  scourge->getParty()->getPlayer()->setAvailableSkillPoints( 
    scourge->getParty()->getPlayer()->getAvailableSkillPoints() - 1 );
  // update the coin label
  char s[255];
  sprintf( s, "%s %d", 
           POINTS_AVAILABLE, 
           scourge->getParty()->getPlayer()->getAvailableSkillPoints() );
  pointsLabel->setText( s );

  // do something...
  int points = 0;
  char *skillName = Constants::SKILL_NAMES[ 0 ];
  cerr << "FIXME: add some to selected skill as a function of the trainer's level." << endl;

  sprintf( s, "After several hours of training you improved %d points in %s", points, skillName );
  result->setText( s );
}

