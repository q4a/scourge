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
class Mission;
class Battle;
class Shape;
class Location;
class Item;

class GameAdapter {
protected:
  UserConfiguration *userConfiguration;
  Session *session;

public:
  GameAdapter(UserConfiguration *config);
  virtual ~GameAdapter();

  inline UserConfiguration *getUserConfiguration() { return userConfiguration; }

  inline void setSession(Session *session) { this->session = session; }
  inline Session *getSession() { return session; }

  virtual inline void initVideo(ShapePalette *shapePal) {}
  virtual inline void initUI() {}
  virtual inline void start() {}
  virtual inline int getScreenWidth() { return 0; }
  virtual inline int getScreenHeight() { return 0; }
  virtual inline void fightProjectileHitTurn(Projectile *proj, Creature *creature) {}
  virtual inline void fightProjectileHitTurn(Projectile *proj, int x, int y) {}
  virtual inline void resetPartyUI() {}
  virtual inline void refreshInventoryUI(int playerIndex) {}
  virtual inline void refreshInventoryUI() {}
  virtual inline void toggleRoundUI(bool startRound) {}
  virtual inline void setFormationUI(int formation, bool playerOnly) {}
  virtual inline void togglePlayerOnlyUI(bool playerOnly) {}
  virtual inline void setPlayerUI(int index) {}
  virtual inline void updateBoardUI(int count, const char **missionText, Color *missionColor) {}
  virtual inline void setMissionDescriptionUI(char *s) {}
  virtual inline void missionCompleted() {}
  virtual inline void cancelBattle(Creature *creature) {}
  virtual inline void moveMonster(Creature *monster) {}
  virtual inline void removeBattle(Battle *battle) {}
  virtual inline void colorMiniMapPoint(int x, int y, Shape *shape, Location *pos=NULL) {}
  virtual inline void eraseMiniMapPoint(int x, int y) {}
  virtual inline void playSound(const char *sound) {}
  virtual inline void loadMonsterSounds( char *type, map<int, vector<string>*> *soundMap ) {}
  virtual inline void unloadMonsterSounds( char *type, map<int, vector<string>*> *soundMap ) {}
  virtual inline void setDebugStr(char *s) {}
  virtual inline void showItemInfoUI(Item *item, int level) {}
  virtual inline double getFps() { return 0.0f; }
  virtual inline void createParty( Creature **pc, int *partySize ) {}
  virtual inline void setBlendFunc() {}

  // initialization events
  virtual inline void initStart(int statusCount, char *message) { cerr << message << endl; }
  virtual inline void initUpdate(char *message) { cerr << message << endl; }
  virtual inline void initEnd() { }

  virtual inline bool isHeadless() { return true; }
};

class ServerAdapter : public GameAdapter {
public:
  ServerAdapter(UserConfiguration *config);
  virtual ~ServerAdapter();
  void start();
};

class ClientAdapter : public GameAdapter {
public:
  ClientAdapter(UserConfiguration *config);
  virtual ~ClientAdapter();
  void start();
};

#endif

