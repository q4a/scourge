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

  intro = new Label( 150, 100, 
                     "You have arrived... as we knew you would. The sand swirls gently in the hourglass of time and reveals all. Sooner or later even the proudest realize that there is no more adventure to be had in the city of Horghh. But fear not! The S.C.O.U.R.G.E. Vermin Extermination Services Company takes good care of its employees. You will be payed in gold, fed nourishing gruel on most days and have access to the company training grounds and shops. Should you sustain injuries or a debilitating predicament (including but not limited to: poison, curses, possession or death ) our clerics will provide healing at a reduced cost. Positions fill up fast, but there are always some available (...) so sign up with a cheerful heart and a song in your step. Your past glories cannot possible compare to the wonder and excitement that lies ahead in the ...uh.. sewers of your new vocation!", 94, SDLHandler::LARGE_FONT, 24 );
  cards->addWidget( intro, INTRO_TEXT );
  

  cancel = cards->createButton( w / 2 - 160, h - Window::TOP_HEIGHT - Window::BOTTOM_HEIGHT - 130, 
                                w / 2 - 10, h - Window::TOP_HEIGHT - Window::BOTTOM_HEIGHT - 100, 
                                "I will not join", INTRO_TEXT );
  toChar0 = cards->createButton( w / 2 + 10, h - Window::TOP_HEIGHT - Window::BOTTOM_HEIGHT - 130, 
                                 w / 2 + 160, h - Window::TOP_HEIGHT - Window::BOTTOM_HEIGHT - 100, 
                                 "Ready to exterminate", INTRO_TEXT );

  for( int i = 0; i < MAX_PARTY_SIZE; i++ ) {
    createCharUI( 1 + i, &( info[ i ] ) );
  }

  toLastChar = cards->createButton( w / 2 - 160, h - Window::TOP_HEIGHT - Window::BOTTOM_HEIGHT - 130, 
                                    w / 2 - 10, h - Window::TOP_HEIGHT - Window::BOTTOM_HEIGHT - 100, 
                                    "Back", OUTRO_TEXT );
  done = cards->createButton( w / 2 + 10, h - Window::TOP_HEIGHT - Window::BOTTOM_HEIGHT - 130, 
                              w / 2 + 160, h - Window::TOP_HEIGHT - Window::BOTTOM_HEIGHT - 100, 
                              "Enter Head Quarters", OUTRO_TEXT );

}

PartyEditor::~PartyEditor() {
}

void PartyEditor::reset() { 
  step = INTRO_TEXT; 
  cards->setActiveCard( step ); 
}

void PartyEditor::handleEvent( Widget *widget ) {

  //
  // cancel and done are handled in mainmenu.cpp
  //

  int oldStep = step;
  if( widget == toChar0 ) {
    step = CREATE_CHAR_0;
  } else if( widget == toLastChar ) {
    step = CREATE_CHAR_3;
  } else {
    for( int i = 0; i < MAX_PARTY_SIZE; i++ ) {
      if( widget == info[i].back ) {
        if( i == 0 ) step = INTRO_TEXT;
        else step = 1 + ( i - 1 );
      } else if( widget == info[i].next ) {
        if( i == MAX_PARTY_SIZE - 1 ) step = OUTRO_TEXT;
        else step = 1 + ( i + 1 );
      }
    }
  }
  if( oldStep != step ) cards->setActiveCard( step );

}

void PartyEditor::createCharUI( int n, CharacterInfo *info ) {
  char msg[80];
  sprintf( msg, "Create character %d out of %d", n - INTRO_TEXT, MAX_PARTY_SIZE );
  cards->createLabel( 30, 10, msg, n );
  cards->createLabel( 30, 30, "Name:", n );
  info->name = new TextField( 100, 30, 40 );
  cards->addWidget( info->name, n );
  
  cards->createLabel( 30, 30, "Character Type:", n );
  info->charType = new ScrollingList( 30, 50, 150, 100, scourge->getShapePalette()->getHighlightTexture() );
  info->charTypeStr = (char**)malloc( Character::character_list.size() * sizeof(char*));
  for(int i = 0; i < (int)Character::character_list.size(); i++) {
    info->charTypeStr[i] = (char*)malloc(120 * sizeof(char));
    strcpy( info->charTypeStr[i], Character::character_list[i]->getName() );
  }
  info->charType->setLines( (int)Character::character_list.size(), (const char**)info->charTypeStr );
  info->charTypeDescription = new Label( 30, 160, "", 50 );

  cards->createLabel( 30, 180, "Chosen Deity:", n );
  info->deityType = new ScrollingList( 30, 200, 150, 100, scourge->getShapePalette()->getHighlightTexture() );
  info->deityTypeStr = (char**)malloc( 1 * sizeof(char*));
  for(int i = 0; i < 1; i++) {
    info->deityTypeStr[i] = (char*)malloc(120 * sizeof(char));
    strcpy( info->deityTypeStr[i], "FIXME: deity list" );
  }
  info->deityType->setLines( 1, (const char**)info->deityTypeStr );
  info->deityTypeDescription = new Label( 30, 160, "FIXME: deity description", 50 );

  // FIXME: copy-paste from constructor
  int w = scourge->getSDLHandler()->getScreen()->w - Window::SCREEN_GUTTER * 2;
  int h = 600;
  info->back = cards->createButton( w / 2 - 160, h - Window::TOP_HEIGHT - Window::BOTTOM_HEIGHT - 130, 
                                    w / 2 - 10, h - Window::TOP_HEIGHT - Window::BOTTOM_HEIGHT - 100, 
                                    "Back", n );
  info->next = cards->createButton( w / 2 + 10, h - Window::TOP_HEIGHT - Window::BOTTOM_HEIGHT - 130, 
                                    w / 2 + 160, h - Window::TOP_HEIGHT - Window::BOTTOM_HEIGHT - 100, 
                                    "Next", n );

  
  /*
  TextField *name;
  ScrollingList *charType;
  Label *charTypeDescription;
  ScrollingList *deityType;
  Label *deityTypeDescription;
  
  Canvas *portrait;
  Button *nextPortrait;
  Button *prevPortrait;
  int portraitIndex;
  
  Canvas *model;
  Button *nextModel;
  Button *prevModel;
  int modelIndex;
  */
}
