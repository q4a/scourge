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
#include "gui/scrollinglist.h"

using namespace std;

#define POINTS_AVAILABLE "Points Available:"
#define COINS_AVAILABLE "Coins Available:"

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
  pointsLabel = win->createLabel( 10, 35, POINTS_AVAILABLE );
  coinsLabel = win->createLabel( w / 2, 35, COINS_AVAILABLE );

  skillList = new ScrollingList( 10, 40, w - 20, 100, 
                                 scourge->getHighlightTexture() );
  win->addWidget( skillList );

  win->createLabel( 10, 160, "Skill Description:" );
  description = new ScrollingLabel( 10, 165, w - 20, 50, "" );
  win->addWidget( description );
  
  win->createLabel( 10, 235, "Training Result:" );
  result = new ScrollingLabel( 10, 240, w - 20, 50, "" );
  win->addWidget( result );

  h = 20;
  int y = win->getHeight() - Window::BOTTOM_HEIGHT - Window::TOP_HEIGHT - h - 10;
  closeButton = win->createButton( w - 80, y, w - 10, y + h, "Close" );
  applyButton = win->createButton( w - 160, y, w - 90, y + h, "Train!" );

  skillText = (char**)malloc( Constants::SKILL_COUNT * sizeof(char*) );
  for( int i = 0; i < Constants::SKILL_COUNT; i++ ) {
    skillText[i] = (char*)malloc( 120 * sizeof(char) );
  }
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
  
  // level-based mark-up
  int base = 150;
  int price = base + 
    (int)Util::getRandomSum( (float)(base / 2), creature->getNpcInfo()->level );
  // 25% variance based on leadership skill.
  float skill = (float)( scourge->getParty()->getPlayer()->getSkill( Constants::LEADERSHIP ) );
  int percentage = (int)( (float)price * ( 100.0f - skill ) / 100.0f * 0.25f );
  cost = price + percentage;

  sprintf( s, "%s (level %d) Cost: %d", 
           creature->getName(), 
           creature->getNpcInfo()->level,
           cost );
  creatureLabel->setText( s );
//  sprintf( s, "%s %d", 
//           POINTS_AVAILABLE, 
//           scourge->getParty()->getPlayer()->getAvailableSkillPoints() );
  strcpy( s, "FIXME" );
  pointsLabel->setText( s );
  sprintf( s, "%s %d", 
           COINS_AVAILABLE, 
           scourge->getParty()->getPlayer()->getMoney() );
  coinsLabel->setText( s );

  result->setText( "" );

  skills.clear();
  for( int i = 0; i < Constants::SKILL_COUNT; i++ ) {
    if( creature->getNpcInfo()->isSubtype( i ) ) {
      strcpy( skillText[ skills.size() ], Constants::SKILL_NAMES[ i ] );
      skills.push_back( i );
    }
  }
  skillList->setLines( (int)skills.size(),
                       (const char**)skillText );
}

void TrainDialog::handleEvent( Widget *widget, SDL_Event *event ) {
  if( widget == closeButton || widget == win->closeButton ) {
    win->setVisible( false );
  } else if( widget == applyButton ) {
    int line = skillList->getSelectedLine();
    if( line > -1 ) {
      train( skills[ line ] );
    }
  } else if( widget == skillList ) {
    int line = skillList->getSelectedLine();
    if( line > -1 ) {
      description->setText( Constants::SKILL_DESCRIPTION[ skills[ line ] ] );
    }
  }
}
  
void TrainDialog::train( int skill ) {
    scourge->showMessageDialog( "FIXME:");
    /*
  if( scourge->getParty()->getPlayer()->getAvailableSkillPoints() <= 0 ) {
    scourge->showMessageDialog( "Select a player who has skill points." );
    return;
  }
  if( scourge->getParty()->getPlayer()->getMoney() < cost ) {
    scourge->showMessageDialog( "Select a player who has enough coins." );
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

  // take the $$$
  scourge->getParty()->getPlayer()->setMoney( 
    scourge->getParty()->getPlayer()->getMoney() - cost );
  // update the coin label
  sprintf( s, "%s %d", 
           COINS_AVAILABLE, 
           scourge->getParty()->getPlayer()->getMoney() );
  coinsLabel->setText( s );

  // add some skill points
  int points = 
    3 +
    creature->getNpcInfo()->level + 
    (int)( 5.0f * rand() / RAND_MAX );
  char *skillName = Constants::SKILL_NAMES[ skill ];
  
  scourge->getParty()->getPlayer()->
    setSkill( skill, 
              scourge->getParty()->getPlayer()->getSkill( skill ) + 
              points );

  sprintf( s, "After several hours of assisted training you improved %d points in %s.", points, skillName );
  result->setText( s );
  */
}

