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
  
NetPlay::NetPlay() {
}

NetPlay::~NetPlay() {
}

void NetPlay::setScourge(Scourge *scourge) {
  this->scourge = scourge;
}

char *NetPlay::getGameState() {
  return "abc";
}
  
void NetPlay::chat(char *message) {
  cout << message << endl;
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


