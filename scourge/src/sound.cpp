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
#include "battle.h"
#include "render/renderlib.h"
#include "rpg/rpglib.h"
#include "item.h"
#include "creature.h"

using namespace std;

Sound::Sound(Preferences *preferences) {
  haveSound = false;

  if(preferences->isSoundEnabled()) {
#ifdef HAVE_SDL_MIXER
    if(Mix_OpenAudio(MIX_DEFAULT_FREQUENCY, MIX_DEFAULT_FORMAT, 2, 1024)) {
      cerr << "*** Error opening audio: " << Mix_GetError() << endl;
      cerr << "\tDisabling sound." << endl;
    } else {
      haveSound = true;
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

      setMusicVolume(preferences->getMusicVolume());
    }
#endif
  }
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
    if(soundMap.size()) soundMap.erase(soundMap.begin(), soundMap.end());
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
      // force stop music
      Mix_HaltMusic();      
    } else {
      /*
      while(Mix_PlayingMusic()) {
        // wait for any fades to complete
        SDL_Delay(100);
      }
      cerr << "*** Music stopped." << endl;
      */
    }
  }
}
#endif

void Sound::loadSounds(Preferences *preferences) {
  if(!haveSound) return;

//  cerr << "Loading UI sounds..." << endl;
  storeSound(0, Window::ROLL_OVER_SOUND);
  storeSound(0, Window::ACTION_SOUND);
  storeSound(0, Window::DROP_SUCCESS);
  storeSound(0, Window::DROP_FAILED);

//  cerr << "Loading battle sounds..." << endl;
  for(int i = 0; i < Battle::getSoundCount(); i++) { 
    storeSound(0, Battle::getSound(i));
  }

//  cerr << "Loading character sounds..." << endl;
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

//  cerr << "Loading item sounds..." << endl;
  for(map<int, vector<string>*>::iterator i = Item::soundMap.begin(); 
       i != Item::soundMap.end(); ++i) {
    //Creature *creature = i->first;
    vector<string> *v = i->second;
    for(int r = 0; r < (int)v->size(); r++) {
      string file = (*v)[r];
      storeSound(0, file.c_str());
    }
  }

//  cerr << "Loading spell sounds..." << endl;
  for(int i = 0; i < (int)MagicSchool::getMagicSchoolCount(); i++) {
    for(int t = 0; t < (int)MagicSchool::getMagicSchool(i)->getSpellCount(); t++) {
      storeSound(0, MagicSchool::getMagicSchool(i)->getSpell(t)->getSound());
    }
  }

  /*
  cerr << "Loading monster sounds..." << endl;
  for(map<string, map<int, vector<string>*>*>::iterator i = Monster::soundMap.begin(); 
      i != Monster::soundMap.end(); ++i) {
    map<int, vector<string>*> *m = i->second;
    for(map<int, vector<string>*>::iterator i2 = m->begin(); 
        i2 != m->end(); ++i2) {
      vector<string> *v = i2->second;
      for(int i = 0; i < (int)v->size(); i++) {
        string file = (*v)[i];
        storeSound(0, file.c_str());
      }
    }
  }
  */  

  setEffectsVolume(preferences->getEffectsVolume());
}

void Sound::loadMonsterSounds( char *monsterType, map<int, vector<string>*> *m, 
							   Preferences *preferences ) {
//  cerr << "Loading monster sounds for " << monsterType << "..." << endl;
  if( m ) {
	for(map<int, vector<string>*>::iterator i2 = m->begin(); i2 != m->end(); ++i2) {
	  vector<string> *v = i2->second;
	  for(int i = 0; i < (int)v->size(); i++) {
		string file = (*v)[i];
		storeSound(0, file.c_str());
	  }
	}
  }
  setEffectsVolume(preferences->getEffectsVolume());
}

void Sound::unloadMonsterSounds( char *monsterType, map<int, vector<string>*> *m ) {  
//  cerr << "Unloading monster sounds for " << monsterType << "..." << endl;
  if( m ) {
	for(map<int, vector<string>*>::iterator i2 = m->begin(); i2 != m->end(); ++i2) {
	  vector<string> *v = i2->second;
	  for(int i = 0; i < (int)v->size(); i++) {
		string file = (*v)[i];
		unloadSound(0, file.c_str());
	  }
	}
  }
}

void Sound::storeSound(int type, const char *file) {
#ifdef HAVE_SDL_MIXER
  if(haveSound) {
    string fileStr = file;
    if(soundMap.find(fileStr) == soundMap.end()) {
      char fn[300];
	  if( file[0] == '/' || file[0] == '\\' ) {
		sprintf(fn, "%s%s", rootDir, file);
	  } else {
		sprintf(fn, "%s/%s", rootDir, file);
	  }
//      cerr << "*** Loading sound file: " << fn << endl;
      Mix_Chunk *sample = Mix_LoadWAV(fn);
      if(!sample) {
        cerr << "*** Error loading WAV file: " << Mix_GetError() << endl;
      } else {
        soundMap[fileStr] = sample;
      }
    }
  }
#endif
}

void Sound::unloadSound( int type, const char *file ) {
#ifdef HAVE_SDL_MIXER
  if(haveSound) {
    string fileStr = file;
    if(soundMap.find(fileStr) != soundMap.end()) {
//	  cerr << "*** Freeing sound: " << fileStr << endl;
	  Mix_Chunk *sample = soundMap[ fileStr ];
	  Mix_FreeChunk( sample );
	  soundMap.erase( fileStr );
	}
  }
#endif 
}

void Sound::playSound(const char *file) {
#ifdef HAVE_SDL_MIXER
  if(haveSound && file) {
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

void Sound::setMusicVolume(int volume) {
#ifdef HAVE_SDL_MIXER
  if(haveSound) Mix_VolumeMusic(volume);
#endif
}

void Sound::setEffectsVolume(int volume) {
#ifdef HAVE_SDL_MIXER
  if(haveSound) {
    for(map<string, Mix_Chunk*>::iterator i = soundMap.begin(); i != soundMap.end(); ++i) {
      Mix_Chunk *sample = i->second;
      Mix_VolumeChunk(sample, volume);
    }
  }
#endif
}
