/***************************************************************************
                          sound.cpp  -  description
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

#include "sound.h"

Sound::Sound() {
#ifdef HAVE_SDL_MIXER
  haveSound = true;
  if(Mix_OpenAudio(MIX_DEFAULT_FREQUENCY, MIX_DEFAULT_FORMAT, 2, 1024)) {
    cerr << "*** Error opening audio: " << Mix_GetError() << endl;
    cerr << "\tDisabling sound." << endl;
    haveSound = false;
  }

  if(haveSound) {
    char fn[300];
    sprintf(fn, "%s/sound/menu.ogg", rootDir);
    menuMusic = Mix_LoadMUS(fn);
    if(!menuMusic) {
      cerr << "*** Error: couldn't load music: " << fn << endl;
      cerr << "\t" << Mix_GetError() << endl;
    }

    sprintf(fn, "%s/sound/dungeon.ogg", rootDir);
    dungeonMusic = Mix_LoadMUS(fn);
    if(!dungeonMusic) {
      cerr << "*** Error: couldn't load music: " << fn << endl;
      cerr << "\t" << Mix_GetError() << endl;
    }

    // FIXME: this should come from userconfig: set the music volume
    Mix_VolumeMusic(MIX_MAX_VOLUME / 2);
  }

#endif
}

Sound::~Sound() {
#ifdef HAVE_SDL_MIXER
  if(haveSound) {
    // delete music
    if(menuMusic) {
      Mix_FreeMusic(menuMusic);
      menuMusic = NULL;
    }
    // delete sounds
    for(map<string, Mix_Chunk*>::iterator i=soundMap.begin(); i != soundMap.end(); ++i) {
      Mix_Chunk *sample = i->second;
      Mix_FreeChunk(sample);
      sample = NULL;
    }
    soundMap.clear();
    // stop audio system
    Mix_CloseAudio();
  }
#endif
}

#ifdef HAVE_SDL_MIXER
void Sound::playMusic(Mix_Music *music) {
  if(haveSound && music) {
    if(Mix_FadeInMusic(music, -1, 2000)) {
      cerr << "*** Error playing music: " << Mix_GetError() << endl;
    }
  }
}

void Sound::stopMusic(Mix_Music *music) {
  if(haveSound && music) {
    if(!Mix_FadeOutMusic(3000)) {
      cerr << "*** Error stopping music: " << Mix_GetError() << endl;
    } else {
      while(Mix_PlayingMusic()) {
        // wait for any fades to complete
        SDL_Delay(100);
      }
      cerr << "*** Music stopped." << endl;
    }
  }
}
#endif

void Sound::loadSounds() {
  cerr << "Loading UI sounds..." << endl;
  storeSound(0, Window::ROLL_OVER_SOUND);
  storeSound(0, Window::ACTION_SOUND);

  cerr << "Loading character sounds..." << endl;
  for(map<string, Character*>::iterator i = Character::character_class.begin(); 
       i != Character::character_class.end(); ++i) {
    //Creature *creature = i->first;
    Character *c = i->second;
    for(map<int, vector<string>*>::iterator t = c->soundMap.begin(); t != c->soundMap.end(); ++t) {
      int type = t->first;
      vector<string>* v = t->second;
      for(int r = 0; r < (int)v->size(); r++) {
        string file = (*v)[r];
        storeSound(type, file.c_str());
      }
    }
  }

  cerr << "Loading item sounds..." << endl;
  for(map<int, vector<string>*>::iterator i = Item::soundMap.begin(); 
       i != Item::soundMap.end(); ++i) {
    //Creature *creature = i->first;
    vector<string> *v = i->second;
    for(int r = 0; r < (int)v->size(); r++) {
      string file = (*v)[r];
      storeSound(0, file.c_str());
    }
  }
}

void Sound::storeSound(int type, const char *file) {
#ifdef HAVE_SDL_MIXER
  char fn[300];
  sprintf(fn, "%s/%s", rootDir, file);
  cerr << "*** Loading sound file: " << fn << endl;
  Mix_Chunk *sample = Mix_LoadWAV(fn);
  if(!sample) {
    cerr << "*** Error loading WAV file: " << Mix_GetError() << endl;
  } else {
    string fileStr = file;
    soundMap[fileStr] = sample;
    // FIXME: this should come from userconfig: set the volume
    Mix_VolumeChunk(sample, MIX_MAX_VOLUME);
  }
#endif
}

void Sound::playSound(const char *file) {
#ifdef HAVE_SDL_MIXER
  if(file) {
    //cerr << "*** Playing WAV: " << file << endl;
    string fileStr = file;
    if(soundMap.find(fileStr) != soundMap.end()) {
      if(Mix_PlayChannel(-1, soundMap[fileStr], 0) == -1) {
        cerr << "*** Error playing WAV file: " << fileStr << endl;
        cerr << "\t" << Mix_GetError() << endl;
      }
    }
  }
#endif
}
