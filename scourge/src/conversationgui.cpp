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
  this->keyPhraseCount = 0;

  int width = 500;
  int height = 400;

  int x = (scourge->getSDLHandler()->getScreen()->w - width) / 2;
  int y = (scourge->getSDLHandler()->getScreen()->h - height) / 2;

  win = scourge->createWindow( x, y, width, height, Constants::getMessage(Constants::CONVERSATION_GUI_TITLE) );

  label = win->createLabel( 10, 13, "Talking to " );
  answer = new ScrollingLabel( 10, 20, width - 20, height / 2, "" );
  Color color;
  color.r = 1;
  color.g = 1;
  color.b = 0;
  color.a = 1;
  answer->addColoring( '$', color );
  win->addWidget( answer );

  y = 20 + height/2 + 10;

  keyPhrases = (char**)malloc(100 * sizeof(char*));
  for(int i = 0; i < 100; i++) {
    keyPhrases[i] = (char*)malloc(120 * sizeof(char));
  }
  keyPhraseList = new ScrollingList(10, y, 
                                    width / 2, 
                                    (height - 40) - y, 
                                    scourge->getShapePalette()->getHighlightTexture() );
  win->addWidget( keyPhraseList );

  x = 20 + width / 2;
  talkButton = win->createButton( x, y, 
                                  width - 10, y + 20, 
                                  "Talk About" );
  closeButton = win->createButton( x, y + 20 + 10, 
                                   width - 10, y + 20 + 30, 
                                   "Close" );

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
  keyPhraseCount = 0;

  keyPhraseList->setLines( keyPhraseCount, (const char**)keyPhrases );

  win->setVisible( true );
}


