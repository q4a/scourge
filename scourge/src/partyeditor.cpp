/***************************************************************************
                          partyeditor.cpp  -  description
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

#include "partyeditor.h"

/**
  *@author Gabor Torok
  */

PartyEditor::PartyEditor(Scourge *scourge) {
  this->scourge = scourge;
  int x = Window::SCREEN_GUTTER;
  int y = ( scourge->getSDLHandler()->getScreen()->h - 600 ) / 2;
  int w = scourge->getSDLHandler()->getScreen()->w - Window::SCREEN_GUTTER * 2;
  int h = 600;
  mainWin = new Window( scourge->getSDLHandler(),
                        x, y, w, h,
                        "Create your party of brave souls...", false, Window::BASIC_WINDOW, "default" );

  mainWin->setVisible( false );
  mainWin->setModal( true );  
  mainWin->setLocked( true );  

  
  step = INTRO_TEXT;

  cards = new CardContainer( mainWin );
  cards->setActiveCard( INTRO_TEXT );

  intro = cards->createLabel( 100, 100, "Welcome adventurer!", INTRO_TEXT );
  

  cancel = mainWin->createButton( 10, h - Window::TOP_HEIGHT - Window::BOTTOM_HEIGHT - 30, 
                                  110, h - Window::TOP_HEIGHT - Window::BOTTOM_HEIGHT - 10, 
                                  "Return to Menu" );
  back = mainWin->createButton( 120, h - Window::TOP_HEIGHT - Window::BOTTOM_HEIGHT - 30, 
                                220, h - Window::TOP_HEIGHT - Window::BOTTOM_HEIGHT - 10, 
                                "Step Back" );
  next = mainWin->createButton( 230, h - Window::TOP_HEIGHT - Window::BOTTOM_HEIGHT - 30, 
                                330, h - Window::TOP_HEIGHT - Window::BOTTOM_HEIGHT - 10, 
                                "Next Step" );
  done = mainWin->createButton( 340, h - Window::TOP_HEIGHT - Window::BOTTOM_HEIGHT - 30, 
                                440, h - Window::TOP_HEIGHT - Window::BOTTOM_HEIGHT - 10, 
                                "Start Game" );
  //  done->setVisible( false );

}

PartyEditor::~PartyEditor() {
}
