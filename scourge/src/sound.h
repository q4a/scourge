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
#include "rpg/character.h"
#include "gui/window.h"
#include "item.h"

using namespace std;

class Sound {
private:
  bool haveSound;
#ifdef HAVE_SDL_MIXER
  Mix_Music *menuMusic;
  map<string, Mix_Chunk*> soundMap;
#endif

public:
  Sound();
  virtual ~Sound();
  
  inline void playMusicMenu() {
#ifdef HAVE_SDL_MIXER
    playMusic(menuMusic);
#endif
  }

  inline void stopMusicMenu() {
#ifdef HAVE_SDL_MIXER
    stopMusic(menuMusic);
#endif
  }

  void loadSounds();
  void storeSound(int type, const char *file);
  void playSound(const char *file);

protected:
#ifdef HAVE_SDL_MIXER
  void playMusic(Mix_Music *music);
  void stopMusic(Mix_Music *music);
#endif
};

#endif
