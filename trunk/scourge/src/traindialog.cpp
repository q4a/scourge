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

  h = 20;
  int y = win->getHeight() - Window::BOTTOM_HEIGHT - Window::TOP_HEIGHT - h - 10;
  closeButton = win->createButton( w - 80, y, w - 10, y + h, "Close" );
  applyButton = win->createButton( w - 160, y, w - 90, y + h, "Train!" );

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
  int cost = price + percentage;

  sprintf( s, "%s (level %d) Cost: %d", 
           creature->getName(), 
           creature->getNpcInfo()->level,
           cost );
  creatureLabel->setText( s );
}

void TrainDialog::handleEvent( Widget *widget, SDL_Event *event ) {
  if( widget == closeButton || widget == win->closeButton ) {
    win->setVisible( false );
  } else if( widget == applyButton ) {
		train();
  }
}
  
void TrainDialog::train() {
    scourge->showMessageDialog( "FIXME:");
}

