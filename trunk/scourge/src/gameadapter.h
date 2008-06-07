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

#include "common/constants.h"
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
	bool ambientPaused;

public:

  enum { COMMAND_SOUND=0, HIT_SOUND, SELECT_SOUND };

  GameAdapter( Preferences *config );
  virtual ~GameAdapter();

  inline Preferences *getPreferences() { return preferences; }

  inline void setSession(Session *session) { this->session = session; }
  inline Session *getSession() { return session; }

	inline bool getAmbientPaused() { return this->ambientPaused; }
	inline void setAmbientPaused( bool b ) { this->ambientPaused = b; }

  // general UI
  virtual inline void initVideo() {}
	virtual inline void repaintScreen() {}
	virtual inline void setUpdate( char *message, int n=-1, int total=-1 ) {}
  virtual inline void initUI() {}
  virtual inline void start() {}
	virtual inline void setCursorVisible( bool b ) {}
  virtual inline int getScreenWidth() { return 0; }
  virtual inline int getScreenHeight() { return 0; }

  virtual inline bool isMouseIsMovingOverMap() { return false; }
  virtual inline Uint16 getMouseX() { return 0; }
  virtual inline Uint16 getMouseY() { return 0; }
  virtual inline void setCursorMode( int cursor, bool useTimer=false ) {}

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
  virtual inline void loadMonsterSounds( char *type, std::map<int, std::vector<std::string>*> *soundMap ) {}
  virtual inline void unloadMonsterSounds( char *type, std::map<int, std::vector<std::string>*> *soundMap ) {}
  virtual inline void loadCharacterSounds( char *type ) {}
  virtual inline void playCharacterSound( char *type, int soundType, int panning ) {}
  virtual inline void unloadCharacterSounds( char *type ) {}
  virtual inline void createParty( Creature **pc, int *partySize ) {}
  virtual inline void teleport( bool toHQ=true ) {}
  virtual inline int getCurrentDepth() { return 0; }
	virtual inline void descendDungeon( Location *pos ) {}
	virtual inline void ascendDungeon( Location *pos ) {}

  // initialization status events
  virtual inline void initStart(int statusCount, char *message) { std::cerr << message << std::endl; }
  virtual inline void initUpdate(char *message) { std::cerr << message << std::endl; }
  virtual inline void initEnd() { }
  
  virtual inline bool isHeadless() { return true; }

  virtual inline char *getDeityLocation( Location *pos ) { return NULL; }
  
  // UI methods. Only call these if isHeadless() is false.
  virtual inline void resetPartyUI() {}
  virtual inline void refreshInventoryUI(int playerIndex) {}
  virtual inline void refreshInventoryUI() {}
  virtual inline void toggleRoundUI(bool startRound) {}
  virtual inline void setFormationUI(int formation, bool playerOnly) {}
  virtual inline void togglePlayerOnlyUI(bool playerOnly) {}
  virtual inline void setPlayerUI(int index) {}
  virtual inline void updateBoardUI(int count, std::string const missionText[], Color *missionColor) {}
  virtual inline void setMissionDescriptionUI(char *s, int mapx, int mapy) {}
  virtual inline void showItemInfoUI(Item *item, int level) {}
  virtual inline GLuint getCursorTexture( int cursorMode ) { return 0; }
	virtual inline int getCursorWidth() { return 0; }
	virtual inline int getCursorHeight() { return 0; }
  virtual inline GLuint getHighlightTexture() { return 0; }
	virtual inline GLuint getGuiTexture() { return 0; }
	virtual inline GLuint getGuiTexture2() { return 0; }
  virtual inline GLuint loadSystemTexture( char *line ) { return 0; }
  virtual inline void showTextMessage( char *message ) {}
	virtual void showMessageDialog(char *message) {}
	virtual inline void askToUploadScore() {}

  virtual bool isMissionCreature( RenderedCreature *creature );
  virtual bool hasParty();
  virtual int getPartySize();
  virtual void loadMapData( const std::string& name );
  virtual void saveMapData( const std::string& name );
  virtual inline Color *getOutlineColor( Location *pos ) { return NULL; }
  virtual inline bool inTurnBasedCombat() { return false; }
	virtual char *getMagicSchoolIndexForLocation( Location *pos ) { return NULL; }
	virtual void setMagicSchoolIndexForLocation( Location *pos, char *magicSchoolName ) {}

  virtual inline void completeCurrentMission() {}

  /**
   * Set up the opengl view.
   */
  virtual void setView() {}

  virtual bool isLevelShaded() { return false; }

  // project-specific castings
  virtual RenderedItem *createItem( char *item_name, int level, int depth );
	virtual void fillContainer( Item *container, int level, int depth );
	virtual Item *createRandomItem( int level, int depth );
  virtual RenderedCreature *createMonster( char *monster_name );
  virtual RenderedCreature *getPlayer();
  virtual RenderedCreature *getParty( int index );
	virtual RenderedItem *createItem( ItemInfo *info );
  virtual RenderedCreature *createMonster( CreatureInfo *info );

  virtual inline bool startTextEffect( char *message ) { return false; }

  // squirrel console
  virtual void printToConsole( const char *s ) { std::cerr << s << std::endl; }

  virtual void startConversation( RenderedCreature *creature, char *message=NULL ) {}
	virtual void endConversation() {}

  virtual bool intersects( int x, int y, int w, int h,
                           int x2, int y2, int w2, int h2 ) {
    return false;							 
  }

	virtual RenderedCreature *createWanderingHero( int level ) { return NULL; }
	virtual bool useDoor( Location *pos, bool openLocked=false ) { return false; }

	virtual inline void addDescription(char const* description, float r=1.0f, float g=1.0f, float b=0.4f, int logLevel=Constants::LOGLEVEL_FULL ) {}
	virtual inline void writeLogMessage( char const* message, int messageType = Constants::MSGTYPE_NORMAL, int logLevel=Constants::LOGLEVEL_FULL ) {}

	virtual GLuint getNamedTexture( char *name ) { return 0; }

	virtual void startMovieMode() {}
	virtual void endMovieMode() {}
	virtual void setContinueAt( char *func, int timeout ) {}
	virtual void setDepthLimits( float minLimit, float maxLimit ) {}
};

class SDLOpenGLAdapter : public GameAdapter {
protected:
  SDLHandler *sdlHandler;
  //int lastMapX, lastMapY, lastMapZ, lastX, lastY;

public:

  SDLOpenGLAdapter( Preferences *config );
  virtual ~SDLOpenGLAdapter();

  virtual void initVideo();
	virtual void repaintScreen() { getSDLHandler()->processEventsAndRepaint(); }
	virtual inline void setUpdate( char *message, int n=-1, int total=-1 ) {
		getSDLHandler()->setUpdate( message, n, total );
	}
  virtual inline SDLHandler *getSDLHandler() { return sdlHandler; }
  virtual inline void setCursorMode( int cursor, bool useTimer=false ) { sdlHandler->setCursorMode( cursor, useTimer ); }

	virtual void setCursorVisible( bool b ) { getSDLHandler()->setCursorVisible( b ); }
  virtual inline int getScreenWidth() { return getSDLHandler()->getScreen()->w; }
  virtual inline int getScreenHeight() { return getSDLHandler()->getScreen()->h; }

  virtual inline bool isMouseIsMovingOverMap() { return getSDLHandler()->mouseIsMovingOverMap; }
  virtual inline Uint16 getMouseX() { return getSDLHandler()->mouseX; }
  virtual inline Uint16 getMouseY() { return getSDLHandler()->mouseY; }

  inline double getFps() { return getSDLHandler()->getFPS(); }
  inline void setDebugStr(char *s) { sdlHandler->setDebugStr(s); }

  virtual inline bool isHeadless() { return false; }

	virtual void setDepthLimits( float minLimit, float maxLimit ) { getSDLHandler()->setDepthLimits( minLimit, maxLimit ); }
  
  /**
   * Set up the opengl view.
   */
  virtual void setView() { getSDLHandler()->setOrthoView(); }

  bool intersects( int x, int y, int w, int h,
                   int x2, int y2, int w2, int h2 );

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

