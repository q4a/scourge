/***************************************************************************
                          conversationgui.cpp  -  description
                             -------------------
    begin                : Sat May 3 2003
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

#include "conversationgui.h"

ConversationGui::ConversationGui(Scourge *scourge) {
  this->scourge = scourge;

  int width = 500;
  int height = 200;

  int x = (scourge->getSDLHandler()->getScreen()->w - width) / 2;
  int y = (scourge->getSDLHandler()->getScreen()->h - height) / 2;

  win = scourge->createWindow( x, y, width, height, Constants::getMessage(Constants::CONVERSATION_GUI_TITLE) );

  label = win->createLabel( 10, 13, "Talking to " );
  answer = new ScrollingLabel( 10, 20, width - 20, 100, "" );
  answer->setWordClickedHandler( this );
  Color color;
  color.r = 1;
  color.g = 1;
  color.b = 0;
  color.a = 1;
  answer->addColoring( '$', color );
  win->addWidget( answer );

  x = 10;
  y = 140;
  closeButton = win->createButton( x, y, x + 100, y + 20, "Close" );

  win->setVisible( false );
}

ConversationGui::~ConversationGui() {
  delete win;
}

bool ConversationGui::handleEvent(Widget *widget, SDL_Event *event) {
  if( widget == win->closeButton || 
      widget == closeButton ) {
    win->setVisible(false);
  }
  return false;
}

void ConversationGui::start(Creature *creature) {
  // pause the game
  scourge->getParty()->toggleRound( true );
  this->creature = creature;
  char tmp[ 80 ];
  sprintf( tmp, "Talking to %s", creature->getName() );
  label->setText( tmp );
  answer->setText( Mission::getIntro() );
  win->setVisible( true );
}

void ConversationGui::wordClicked( char *word ) {
  cerr << "Clicked: " << word << endl;
  answer->setText( Mission::getAnswer( word ) );
}

