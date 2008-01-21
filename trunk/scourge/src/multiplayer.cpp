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
#include "render/renderlib.h"
#include "rpg/rpglib.h"
#include "creature.h"
#include "shapepalette.h"

using namespace std;

/**
  *@author Gabor Torok
  */
MultiplayerDialog::MultiplayerDialog(Scourge *scourge) {
  this->scourge = scourge;
  mainWin = new Window( scourge->getSDLHandler(),
                        250, 250, 440, 250, 
                        _( "Multiplayer Setup" ), 
                        scourge->getShapePalette()->getGuiTexture(),
                        true, Window::BASIC_WINDOW,
                        scourge->getShapePalette()->getGuiTexture2() );
  mainWin->setModal( true );
  startServer = mainWin->createButton( 10, 10, 160, 30, _( "Host a game" ), true );
  startServer->setSelected(true);

  mainWin->createLabel( 90, 55, _( "--- OR ---" ) );
  enum { TMP_SIZE = 300 };
	char tmp[ TMP_SIZE ];
	snprintf( tmp, TMP_SIZE, "%s:", _( "Server address" ) );
  mainWin->createLabel( 10, 80, tmp );
  serverName = mainWin->createTextField( 10, 85, 30);
	snprintf( tmp, TMP_SIZE, "%s:", _( "Server port" ) );
  mainWin->createLabel( 10, 120, tmp );
  serverPort = mainWin->createTextField( 10, 125, 10 );
	snprintf( tmp, TMP_SIZE, "%s:", _( "Username" ) );
  mainWin->createLabel( 10, 160, tmp );
  userName = mainWin->createTextField( 10, 165, 30 );
  
  joinServer = mainWin->createButton( 10, 190, 160, 210, _( "Join a game" ), true );
  okButton = mainWin->createButton( 330, 180, 430, 210, _( "Start Game" ) );
  
  
	snprintf( tmp, TMP_SIZE, "%s:", _( "Select a character" ) );
  mainWin->createLabel( 230, 20, tmp );
  characterList = new ScrollingList( 230, 30, 200, 130, 
                                     scourge->getShapePalette()->getHighlightTexture() );
  mainWin->addWidget( characterList );

  // allocate strings for list
  // FIXME: use a character set not the party here
	/*
  Party::createHardCodedParty(scourge->getSession(), pc, &pcCount);
  charStr = (char**)malloc(pcCount * sizeof(char*));
  for(int i = 0; i < pcCount; i++) {
    charStr[i] = (char*)malloc(255 * sizeof(char));
    snprintf(charStr[i], 255, "%s, %s level: %d", pc[i]->getName(),
            pc[i]->getCharacter()->getName(),
            pc[i]->getLevel());
  }
  characterList->setLines(pcCount, (const char**)charStr);
	*/
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

Creature *MultiplayerDialog::getCreature() { 
  int n = characterList->getSelectedLine();
  // The reason for this weird behavior is that party.cpp frees party members.
  // Since we don't want to delete the original set, create a new one.
  // Rewrite this when the character editor/store are done.
  Creature *pc[MAX_PARTY_SIZE];
  int pcCount;
  Party::createHardCodedParty(scourge->getSession(), pc, &pcCount);
  Creature *c = pc[n];
  // delete the ones not used
  for(int i = 0; i < pcCount; i++) {
    if(i != n) delete pc[i];
  }
  return c;
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

