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
#include "session.h"
#include "userconfiguration.h"
#include "shapepalette.h"

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
#ifdef HAVE_SDL_NET
  session->runServer(userConfiguration->getPort());
#endif
}



ClientAdapter::ClientAdapter(UserConfiguration *config) : GameAdapter(config) {
}

ClientAdapter::~ClientAdapter() {
}

void ClientAdapter::start() {
#ifdef HAVE_SDL_NET
  session->runClient(userConfiguration->getHost(), 
                     userConfiguration->getPort(), 
                     userConfiguration->getUserName());
#endif
}                               

