/***************************************************************************
                          gameadapter.cpp  -  description
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

#include "gameadapter.h"

GameAdapter::GameAdapter(UserConfiguration *config) {
  this->userConfiguration = config;
}

GameAdapter::~GameAdapter() {
  delete userConfiguration;
}




ServerAdapter::ServerAdapter(UserConfiguration *config) : GameAdapter(config) {
}

ServerAdapter::~ServerAdapter() {
}

void ServerAdapter::start() {
  // will work when we can reference session (can't until ref. to scourge.h is removed from session.h)
  //session->runServer(userConfiguration->getPort());
}



ClientAdapter::ClientAdapter(UserConfiguration *config) : GameAdapter(config) {
}

ClientAdapter::~ClientAdapter() {
}

void ClientAdapter::start() {
  // will work when we can reference session (can't until ref. to scourge.h is removed from session.h)
  //session->runClient(userConfiguration->getHost(), 
                     //userConfiguration->getPort(), 
                     //userConfiguration->getUserName());
}                               

