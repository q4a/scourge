/***************************************************************************
                          gameadapter.h  -  description
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

#ifndef GAME_ADAPTER_H
#define GAME_ADAPTER_H

#include "constants.h"
#include "userconfiguration.h"

class GameAdapter {
protected:
  UserConfiguration *userConfiguration;

public:
  GameAdapter(UserConfiguration *config);
  virtual ~GameAdapter();

  inline UserConfiguration *getUserConfiguration() { return userConfiguration; }
};

class ServerAdapter : public GameAdapter {
public:
  ServerAdapter(UserConfiguration *config);
  virtual ~ServerAdapter();
};

class ClientAdapter : public GameAdapter {
public:
  ClientAdapter(UserConfiguration *config);
  virtual ~ClientAdapter();
};

#endif

