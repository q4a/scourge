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
#include "shapepalette.h"
//#include "map.h"

class Party;
class Map;
class Projectile;
class Creature;
class Session;

class GameAdapter {
protected:
  UserConfiguration *userConfiguration;

public:
  GameAdapter(UserConfiguration *config);
  virtual ~GameAdapter();

  inline UserConfiguration *getUserConfiguration() { return userConfiguration; }

  virtual inline void initVideo(ShapePalette *shapePal) {}
  virtual inline void initUI(Session *session) {}
  virtual inline void start() {}
  virtual inline int getScreenWidth() { return 0; }
  virtual inline int getScreenHeight() { return 0; }
  virtual inline void fightProjectileHitTurn(Projectile *proj, Creature *creature) {}
  virtual inline void resetPartyUI() {}
  virtual inline void refreshInventoryUI(int playerIndex) {}
  virtual inline void toggleRoundUI(bool startRound) {}
  virtual inline void setFormationUI(int formation, bool playerOnly) {}
  virtual inline void togglePlayerOnlyUI(bool playerOnly) {}
  virtual inline void setPlayerUI(int index) {}
  virtual inline void updateBoardUI(int count, const char **missionText, Color *missionColor) {}
  virtual inline void setMissionDescriptionUI(char *s) {}

  // initialization events
  virtual inline void initStart(int statusCount, char *message) { cerr << message << endl; }
  virtual inline void initUpdate(char *message) { cerr << message << endl; }
  virtual inline void initEnd() { }
};

class ServerAdapter : public GameAdapter {
public:
  ServerAdapter(UserConfiguration *config);
  virtual ~ServerAdapter();
  virtual void start();
};

class ClientAdapter : public GameAdapter {
public:
  ClientAdapter(UserConfiguration *config);
  virtual ~ClientAdapter();
  virtual void start();
};

#endif

