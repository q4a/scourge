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
                        250, 250, 270, 250, 
                        strdup("Multiplayer Setup"), 
                        scourge->getShapePalette()->getGuiTexture(),
												true );
  mainWin->setModal( true );
  startServer = mainWin->createButton( 20, 20, 120, 40, strdup("Host a game"), false );
  joinServer = mainWin->createButton( 20, 60, 120, 80, strdup("Join a game"), false );
}

MultiplayerDialog::~MultiplayerDialog() {
  delete startServer;
  delete joinServer;
  delete mainWin;
}

bool MultiplayerDialog::handleEvent(SDL_Event *event) {
  return false;
}

bool MultiplayerDialog::handleEvent(Widget *widget, SDL_Event *event) {
  if(widget == mainWin->closeButton) mainWin->setVisible(false);
  return false;
}

