/***************************************************************************
                          netplay.h  -  description
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

#ifndef NET_PLAY_H
#define NET_PLAY_H

#include <iostream>
#include <string>
#include "scourge.h"
#include "net/gamestatehandler.h"
#include "net/commands.h"

using namespace std;


/** 
  @author Gabor Torok
*/ 
class NetPlay : public GameStateHandler,CommandInterpreter {
private:
  Scourge *scourge;
  Window *mainWin;
  ScrollingList *messageList;

public:
  NetPlay(Scourge *scourge);
  virtual ~NetPlay();

  char *getGameState();
  
  void chat(char *message);
  void logout();
  void ping(int frame);
  void processGameState(int frame, char *p);
  void handleUnknownMessage();

  inline Window *getWindow() { return mainWin; }
};

#endif

