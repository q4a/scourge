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
#include "persist.h"
#include "sdlhandler.h"
#include "render/mapadapter.h"

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
class Preferences;
class RenderedItem;
class RenderedCreature;

class GameAdapter : public MapAdapter {
protected:
  Preferences *preferences;
  Session *session;

public:
  GameAdapter( Preferences *config );
  virtual ~GameAdapter();

  inline Preferences *getPreferences() { return preferences; }

  inline void setSession(Session *session) { this->session = session; }
  inline Session *getSession() { return session; }

  // general UI
  virtual inline void initVideo() {}
  virtual inline void initUI() {}
  virtual inline void start() {}
  virtual inline int getScreenWidth() { return 0; }
  virtual inline int getScreenHeight() { return 0; }
  virtual inline void playSound(const char *sound) {}
  virtual inline bool isMouseIsMovingOverMap() { return false; }
  virtual inline Uint16 getMouseX() { return 0; }
  virtual inline Uint16 getMouseY() { return 0; }

  // debug
  virtual inline void setDebugStr(char *s) {}
  virtual inline double getFps() { return 0.0f; }
  virtual inline void setBlendFunc() {}

  // game events
  virtual inline void fightProjectileHitTurn(Projectile *proj, RenderedCreature *creature) {}
  virtual inline void fightProjectileHitTurn(Projectile *proj, int x, int y) {}
  virtual inline void missionCompleted() {}
  virtual inline void cancelBattle(Creature *creature) {}
  virtual inline void moveMonster(Creature *monster) {}
  virtual inline void removeBattle(Battle *battle) {}
  virtual inline void colorMiniMapPoint(int x, int y, Shape *shape, Location *pos=NULL) {}
  virtual inline void eraseMiniMapPoint(int x, int y) {}
  virtual inline void loadMonsterSounds( char *type, std::map<int, std::vector<std::string>*> *soundMap ) {}
  virtual inline void unloadMonsterSounds( char *type, std::map<int, std::vector<std::string>*> *soundMap ) {}
  virtual inline void createParty( Creature **pc, int *partySize ) {}
  virtual inline void teleport( bool toHQ=true ) {}
  virtual inline int getCurrentDepth() { return 0; }

  // initialization status events
  virtual inline void initStart(int statusCount, char *message) { std::cerr << message << std::endl; }
  virtual inline void initUpdate(char *message) { std::cerr << message << std::endl; }
  virtual inline void initEnd() { }
  
  virtual inline bool isHeadless() { return true; }
  
  // UI methods. Only call these if isHeadless() is false.
  virtual inline void resetPartyUI() {}
  virtual inline void refreshInventoryUI(int playerIndex) {}
  virtual inline void refreshInventoryUI() {}
  virtual inline void toggleRoundUI(bool startRound) {}
  virtual inline void setFormationUI(int formation, bool playerOnly) {}
  virtual inline void togglePlayerOnlyUI(bool playerOnly) {}
  virtual inline void setPlayerUI(int index) {}
  virtual inline void updateBoardUI(int count, const char **missionText, Color *missionColor) {}
  virtual inline void setMissionDescriptionUI(char *s, int mapx, int mapy) {}
  virtual inline void showItemInfoUI(Item *item, int level) {}
  virtual inline GLuint getCursorTexture( int cursorMode ) { return 0; }
  virtual inline GLuint getHighlightTexture() { return 0; }
  virtual inline GLuint loadSystemTexture( char *line ) { return 0; }

  virtual bool isMissionCreature( RenderedCreature *creature );
  virtual bool hasParty();
  virtual int getPartySize();
  virtual void loadMapData( const char *name );
  virtual void saveMapData( const char *name );
  virtual inline Color *getOutlineColor( Location *pos ) { return NULL; }

  /**
   * Set up the opengl view.
   */
  virtual void setView() {}

  virtual bool isLevelShaded() { return false; }

  // project-specific castings
  virtual RenderedItem *load( ItemInfo *info );
  virtual RenderedCreature *load( CreatureInfo *info );
  virtual RenderedCreature *getPlayer();
  virtual RenderedCreature *getParty( int index );

  virtual inline bool startTextEffect( char *message ) { return false; }

  // squirrel console
  virtual void printToConsole( const char *s ) { std::cerr << s << std::endl; }
};

class SDLOpenGLAdapter : public GameAdapter {
protected:
  SDLHandler *sdlHandler;
  //int lastMapX, lastMapY, lastMapZ, lastX, lastY;

public:

  SDLOpenGLAdapter( Preferences *config );
  virtual ~SDLOpenGLAdapter();

  virtual void initVideo();
  virtual inline SDLHandler *getSDLHandler() { return sdlHandler; }

  virtual inline int getScreenWidth() { return getSDLHandler()->getScreen()->w; }
  virtual inline int getScreenHeight() { return getSDLHandler()->getScreen()->h; }
  virtual void playSound(const char *sound);

  virtual inline bool isMouseIsMovingOverMap() { return getSDLHandler()->mouseIsMovingOverMap; }
  virtual inline Uint16 getMouseX() { return getSDLHandler()->mouseX; }
  virtual inline Uint16 getMouseY() { return getSDLHandler()->mouseY; }

  inline double getFps() { return getSDLHandler()->getFPS(); }
  inline void setDebugStr(char *s) { sdlHandler->setDebugStr(s); }

  virtual inline bool isHeadless() { return false; }

  /**
   * Set up the opengl view.
   */
  virtual void setView() { getSDLHandler()->setOrthoView(); }

protected:
  void decodeName(int name, Uint16* mapx, Uint16* mapy, Uint16* mapz);
};

class ServerAdapter : public GameAdapter {
public:
  ServerAdapter( Preferences *config );
  virtual ~ServerAdapter();
  void start();
};

class ClientAdapter : public GameAdapter {
public:
  ClientAdapter( Preferences *config );
  virtual ~ClientAdapter();
  void start();
};

#endif

