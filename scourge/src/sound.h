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
#include "board.h"

class AmbientSound {
	std::string name;
#ifdef HAVE_SDL_MIXER
  Mix_Chunk *footsteps;
	std::vector<Mix_Chunk*> ambients;
#endif
	std::string afterFirstLevel;
public:
	AmbientSound( std::string& name, std::string& ambient, std::string& footsteps, std::string& afterFirstLevel );
	~AmbientSound();
	int playRandomAmbientSample();
	int playFootsteps( int panning );
	inline std::string &getAfterFirstLevel() { return afterFirstLevel; }
};

class Sound {
private:
  int missionMusicIndex;
  int fightMusicIndex;
  bool haveSound;  
	std::map<std::string, AmbientSound*> ambients;
#ifdef HAVE_SDL_MIXER
  Mix_Music *menuMusic;
  Mix_Music *hqMusic;
  Mix_Music *missionMusic;
  Mix_Music *fightMusic;
  Mix_Music *currentMusic;
  Mix_Music *currentLevelMusic;
	Mix_Music *chapterMusic;
	int lastChapter;
  Uint32 musicStartTime;
  double musicPosition;
  std::map<std::string, Mix_Chunk*> soundMap;  
	std::map<std::string, std::string> soundNameMap;  
	std::map<std::string, Mix_Chunk*> ambient_objects;
  std::map<std::string, std::map<int, std::vector<Mix_Chunk*>* >* > characterSounds;
#endif

public:

	static char *TELEPORT;
	static char *OPEN_DOOR;
	static char *OPEN_BOX;
	static char *FOOTSTEP_INDOORS;
	static char *FOOTSTEP_OUTDOORS;

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

  inline void playMusicChapter() {
#ifdef HAVE_SDL_MIXER
    playMusic( chapterMusic, 2000, 1 );
#endif
  }

	void stopMusic( int ms=3000 );

  void loadSounds(Preferences *preferences);
	void loadUISounds(Preferences *preferences);
  void loadMonsterSounds( char *monsterType, std::map<int, std::vector<std::string>*> *m,
													Preferences *preferences );
  void unloadMonsterSounds( char *monsterType, std::map<int, std::vector<std::string>*> *m );

  void loadCharacterSounds( char *type );
  void unloadCharacterSounds( char *type );
  void playCharacterSound( char *type, int soundType, int panning );

	void storeSound(const std::string& name, const std::string& file);
  void storeSound(int type, const std::string& file);
  void unloadSound( int type, const std::string& file );
  void playSound( const std::string& file, int panning );

  void setMusicVolume(int volume);
  void setEffectsVolume(int volume);

  void selectMusic( Preferences *preferences, Mission *mission=NULL );

  void checkMusic( bool inCombat );

  void startFootsteps( std::string& name, int depth, int panning );
  void stopFootsteps();

	void addAmbientSound( std::string& name, std::string& ambient, std::string& footsteps, std::string& afterFirstLevel );
	void startAmbientSound( std::string& name, int depth );
	void stopAmbientSound();

	void playObjectSound( std::string& name, int percent, int panning );
	void storeAmbientObjectSound( std::string const& sound );

	void startRain();
	void stopRain();

protected:
	AmbientSound *getAmbientSound( std::string& name, int depth );
#ifdef HAVE_SDL_MIXER
  void playMusic( Mix_Music *music, int ms=2000, int loopCount=-1 );
  void storeCharacterSounds( std::map<int,std::vector<Mix_Chunk*>*> *charSoundMap, 
                             char *type, int soundType, char *filePrefix );
#endif
};

#endif

