/***************************************************************************
                          multiplayer.cpp  -  description
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

#include "multiplayer.h"

/**
  *@author Gabor Torok
  */
MultiplayerDialog::MultiplayerDialog(Scourge *scourge) {
  this->scourge = scourge;
  mainWin = new Window( scourge->getSDLHandler(),
                        250, 250, 440, 250, 
                        strdup("Multiplayer Setup"), 
                        scourge->getShapePalette()->getGuiTexture(),
												true );
  mainWin->setModal( true );
  startServer = mainWin->createButton( 10, 10, 160, 30, strdup("Host a game"), true );
  startServer->setSelected(true);

  mainWin->createLabel( 90, 55, strdup("--- OR ---") );

  mainWin->createLabel( 10, 80, strdup("Server address:") );
  serverName = mainWin->createTextField( 10, 85, 30);
  mainWin->createLabel( 10, 120, strdup("Server port:") );
  serverPort = mainWin->createTextField( 10, 125, 10 );
  mainWin->createLabel( 10, 160, strdup("Username:") );
  userName = mainWin->createTextField( 10, 165, 30 );
  
  joinServer = mainWin->createButton( 10, 190, 160, 210, strdup("Join a game"), true );

  
  
  mainWin->createLabel( 230, 20, strdup("Select a character:") );
  characterList = new ScrollingList( 230, 30, 200, 130, 
                                     scourge->getShapePalette()->getHighlightTexture() );
  mainWin->addWidget( characterList );


  // allocate strings for list
  // FIXME: use a character set not the party here
  charStr = (char**)malloc(Party::pcCount * sizeof(char*));
  for(int i = 0; i < Party::pcCount; i++) {
    charStr[i] = (char*)malloc(255 * sizeof(char));
    sprintf(charStr[i], "%s, %s level: %d", Party::pc[i]->getName(),
            Party::pc[i]->getCharacter()->getName(),
            Party::pc[i]->getLevel());
  }
  characterList->setLines(Party::pcCount, (const char**)charStr);

  okButton = mainWin->createButton( 330, 180, 430, 210, strdup("Start Game") );
}

MultiplayerDialog::~MultiplayerDialog() {
  delete startServer;
  delete joinServer;
  delete userName;
  delete serverName;
  delete serverPort;
  delete mainWin;

  // FIXME: delete charStr
}

bool MultiplayerDialog::handleEvent(SDL_Event *event) {
  return false;
}

bool MultiplayerDialog::handleEvent(Widget *widget, SDL_Event *event) {
  if(widget == mainWin->closeButton) mainWin->setVisible(false);
  if(widget == startServer) {
    joinServer->setSelected(false);
  } else if(widget == joinServer) {
    startServer->setSelected(false);
  } else if(widget == okButton) {
    if(startServer->isSelected()) {
      value = START_SERVER;
    } else if(joinServer->isSelected()) {
      value = JOIN_SERVER;
    }
    hide();
  }
  return false;
}

