/***************************************************************************
                          netplay.cpp  -  description
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

#include "netplay.h"
  
NetPlay::NetPlay(Scourge *scourge) {
  this->scourge = scourge;

  // allocate strings for list
  chatStrCount = 0;
  chatStr = (char**)malloc(MAX_CHAT_SIZE * sizeof(char*));
  for(int i = 0; i < MAX_CHAT_SIZE; i++) {
    this->chatStr[i] = (char*)malloc(120 * sizeof(char));
  }

  int width = 
    scourge->getSDLHandler()->getScreen()->w - 
    (Scourge::PARTY_GUI_WIDTH + (Window::SCREEN_GUTTER * 2));
  mainWin = new Window( scourge->getSDLHandler(),
                        0, scourge->getSDLHandler()->getScreen()->w - width, 
                        width, Scourge::PARTY_GUI_HEIGHT, 
                        strdup("Chat"), 
                        scourge->getShapePalette()->getGuiTexture(), false );
  mainWin->setBackground(0, 0, 0);
  messageList = new ScrollingList(0, 20, width, Scourge::PARTY_GUI_HEIGHT - 45, 
                                  scourge->getShapePalette()->getHighlightTexture());
  messageList->setSelectionColor( 0.15f, 0.15f, 0.3f );
  mainWin->addWidget(messageList);
  // this has to be after addWidget
  messageList->setBackground( 1, 0.75f, 0.45f );
  messageList->setSelectionColor( 0.25f, 0.25f, 0.25f );

  chatText = new TextField( 0, 0, 100 );
  mainWin->addWidget( chatText );
}

NetPlay::~NetPlay() {
  // deleting the window deletes its controls (messageList, etc.)
  delete mainWin;
  // fixme: free chatStr
}

bool NetPlay::handleEvent(Widget *widget, SDL_Event *event) {
  if(widget == chatText) {
#ifdef HAVE_SDL_NET
    scourge->getClient()->sendChatTCP(chatText->getText());
#endif    
    chatText->clearText();
  }
  return false;
}

char *NetPlay::getGameState() {
  return "abc";
}
  
void NetPlay::chat(char *message) {
  //  cout << message << endl;
  if(chatStrCount == MAX_CHAT_SIZE) {
    for(int i = 1; i < chatStrCount - 1; i++)
      strcpy(chatStr[i - 1], chatStr[i]);
  } else {
    chatStrCount++;
  }
  strcpy(chatStr[chatStrCount - 1], message);
  messageList->setLines(chatStrCount, (const char **)chatStr);
}

void NetPlay::logout() {
  cout << "Logout." << endl;
}

void NetPlay::ping(int frame) {
  cout << "Ping." << endl;
}

void NetPlay::processGameState(int frame, char *p) {
  cout << "Game state: frame=" << frame << " state=" << p << endl;
}

void NetPlay::handleUnknownMessage() {
  cout << "Unknown message received." << endl;
}


