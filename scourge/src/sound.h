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
#include "constants.h"
#include "gui/window.h"
#include "userconfiguration.h"

class Sound {
private:
  int missionMusicIndex;
  bool haveSound;
#ifdef HAVE_SDL_MIXER
  Mix_Music *menuMusic;
  Mix_Music *hqMusic;
  Mix_Music *missionMusic;
  std::map<std::string, Mix_Chunk*> soundMap;
#endif

public:
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
    playMusic(missionMusic);
#endif
  }

#ifdef HAVE_SDL_MIXER
  void stopMusic();
#endif

  void loadSounds(Preferences *preferences);
  void loadMonsterSounds( char *monsterType, std::map<int, std::vector<std::string>*> *m,
						  Preferences *preferences );
  void unloadMonsterSounds( char *monsterType, std::map<int, std::vector<std::string>*> *m );

  void storeSound(int type, const char *file);
  void unloadSound( int type, const char *file );
  void playSound(const char *file);

  void setMusicVolume(int volume);
  void setEffectsVolume(int volume);

  void selectMusic( Preferences *preferences );

protected:
#ifdef HAVE_SDL_MIXER
  void playMusic(Mix_Music *music);
#endif
};

#endif

