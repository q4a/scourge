/***************************************************************************
                          main.cpp  -  description
                             -------------------
    begin                : Sat May  3 19:39:34 EDT 2003
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

#include <stdlib.h>

#include "userconfiguration.h"
#include "gameadapter.h"
#include "session.h"
#include "scourge.h"

using namespace std;

GameAdapter *createGameAdapter(UserConfiguration *config) {
  GameAdapter *adapter;
  if(config->getStandAloneMode() == UserConfiguration::SERVER) {
    adapter = new ServerAdapter(config);
  } else if(config->getStandAloneMode() == UserConfiguration::CLIENT) {
    adapter = new ClientAdapter(config);
  } else if( config->getStandAloneMode() == UserConfiguration::TEST ) {
    adapter = new GameAdapter( config );
  } else {
    adapter = new Scourge(config);
  }
  return adapter;
}

int main(int argc, char *argv[]) {
  UserConfiguration *userConfiguration = new UserConfiguration();  
  userConfiguration->loadConfiguration();    
  userConfiguration->parseCommandLine(argc, argv); 

  return Session::runGame( createGameAdapter( userConfiguration ), argc, argv );
}

