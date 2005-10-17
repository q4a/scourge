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

using namespace std;
  
NetPlay::NetPlay(Scourge *scourge) {
  this->scourge = scourge;

  // allocate strings for list
  chatStrCount = 0;
  chatStr = (char**)malloc(MAX_CHAT_SIZE * sizeof(char*));
  for(int i = 0; i < MAX_CHAT_SIZE; i++) {
    this->chatStr[i] = (char*)malloc(CHAT_STR_LENGTH * sizeof(char));
  }

  int width = 
    scourge->getSDLHandler()->getScreen()->w - 
    (Scourge::PARTY_GUI_WIDTH + (Window::SCREEN_GUTTER * 2));
  mainWin = new Window( scourge->getSDLHandler(),
                        0, scourge->getSDLHandler()->getScreen()->h - Scourge::PARTY_GUI_HEIGHT, 
                        width, Scourge::PARTY_GUI_HEIGHT, 
                        "Chat", 
                        scourge->getShapePalette()->getGuiTexture(), false,
                        Window::BASIC_WINDOW,
                        scourge->getShapePalette()->getGuiTexture2() );
  mainWin->setBackground(0, 0, 0);
  messageList = new ScrollingList(10, 35, width - 20, Scourge::PARTY_GUI_HEIGHT - 65, 
                                  scourge->getShapePalette()->getHighlightTexture());
  messageList->setSelectionColor( 0.15f, 0.15f, 0.3f );
  messageList->setCanGetFocus( false );
  mainWin->addWidget(messageList);
  // this has to be after addWidget
  messageList->setBackground( 1, 0.75f, 0.45f );
  messageList->setSelectionColor( 0.25f, 0.25f, 0.25f );
  messageList->setColor( 1, 1, 1 );

  chatText = new TextField( 10, 10, 70 );
  mainWin->addWidget( chatText );
}

NetPlay::~NetPlay() {
  // deleting the window deletes its controls (messageList, etc.)
  delete mainWin;
  // fixme: free chatStr
}

bool NetPlay::handleEvent(Widget *widget, SDL_Event *event) {
  if(widget == chatText && 
     chatText->getEventType() == TextField::EVENT_ACTION) {
#ifdef HAVE_SDL_NET
    scourge->getSession()->getClient()->sendChatTCP(chatText->getText());
#endif    
    chatText->clearText();
  }
  return false;
}

char *NetPlay::getGameState() {
  return "abc";
}
  
void NetPlay::chat(char *message) {
  cerr << message << endl;
  if(chatStrCount == MAX_CHAT_SIZE) {
    for(int i = 1; i < chatStrCount - 1; i++)
      strcpy(chatStr[i - 1], chatStr[i]);
  } else {
    chatStrCount++;
  }
  cerr << "chatStrCount=" << chatStrCount << endl;
  strncpy(chatStr[chatStrCount - 1], message, CHAT_STR_LENGTH - 2);
  chatStr[chatStrCount - 1][CHAT_STR_LENGTH - 1] = '\0';
  //messageList->debug = true;
  messageList->setLines(chatStrCount, (const char **)chatStr);
  messageList->setSelectedLine(chatStrCount - 1);
}

void NetPlay::logout() {
  cerr << "Logout." << endl;
}

void NetPlay::ping(int frame) {
  //cerr << "Ping." << endl;
}

void NetPlay::processGameState(int frame, char *p) {
  //cerr << "Game state: frame=" << frame << " state=" << p << endl;
}

void NetPlay::handleUnknownMessage() {
  cerr << "Unknown message received." << endl;
}

void NetPlay::serverClosing() {
  // do nothing
}

void NetPlay::character(char *bytes, int length) {
  // do nothing
}

void NetPlay::addPlayer(Uint32 id, char *bytes, int length) {
  cerr << "* Received character data for player. Server id=" << 
    id << " data length=" << length << endl;
  if(length != sizeof(CreatureInfo)) {
    cerr << "* Bad length for addPlayer!. length=" << 
      length << " size=" << sizeof(CreatureInfo) << endl;
    return;
  }

  // do something with the data
}
