/***************************************************************************
                          sound.h  -  description
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

#ifndef SOUND_H
#define SOUND_H

#include <string>
#include <map>
#include <vector>
#include "common/constants.h"
#include "gui/window.h"
#include "userconfiguration.h"

class Sound {
private:
  int missionMusicIndex;
  int fightMusicIndex;
  bool haveSound;  
#ifdef HAVE_SDL_MIXER
  Mix_Music *menuMusic;
  Mix_Music *hqMusic;
  Mix_Music *missionMusic;
  Mix_Music *fightMusic;
  Mix_Music *currentMusic;
  Mix_Music *currentLevelMusic;
  Uint32 musicStartTime;
  double musicPosition;
  std::map<std::string, Mix_Chunk*> soundMap;  
  std::map<std::string, std::map<int, std::vector<Mix_Chunk*>* >* > characterSounds;
#endif

public:

	static char *TELEPORT;
	static char *OPEN_DOOR;
	static char *OPEN_BOX;

  Sound(Preferences *preferences);
  virtual ~Sound();

  inline void playMusicMenu() {
#ifdef HAVE_SDL_MIXER
    playMusic(menuMusic);
#endif
  }

  inline void playMusicHQ() {
#ifdef HAVE_SDL_MIXER
    playMusic(hqMusic);
#endif
  }

  inline void playMusicMission() {
#ifdef HAVE_SDL_MIXER
    playMusic( missionMusic );
#endif
  }

	void stopMusic( int ms=3000 );

  void loadSounds(Preferences *preferences);
  void loadMonsterSounds( char *monsterType, std::map<int, std::vector<std::string>*> *m,
						  Preferences *preferences );
  void unloadMonsterSounds( char *monsterType, std::map<int, std::vector<std::string>*> *m );
  
  void loadCharacterSounds( char *type );
  void unloadCharacterSounds( char *type );
  void playCharacterSound( char *type, int soundType );

  void storeSound(int type, const char *file);
  void unloadSound( int type, const char *file );
  void playSound(const char *file);

  void setMusicVolume(int volume);
  void setEffectsVolume(int volume);

  void selectMusic( Preferences *preferences );

  void checkMusic( bool inCombat );

protected:
#ifdef HAVE_SDL_MIXER
  void playMusic( Mix_Music *music, int ms=2000 );
  void storeCharacterSounds( std::map<int,std::vector<Mix_Chunk*>*> *charSoundMap, 
                             char *type, int soundType, char *filePrefix );
#endif
};

#endif

