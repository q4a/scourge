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
#include <iomanip>
#include <string>
#include "sound.h"
#include "battle.h"
#include "render/renderlib.h"
#include "rpg/rpglib.h"
#include "item.h"
#include "creature.h"

using namespace std;

#define MISSION_MUSIC_COUNT 6.0f
#define FIGHT_MUSIC_COUNT 1.0f

char *Sound::TELEPORT = "sound/equip/teleport.wav";
char *Sound::OPEN_DOOR = "sound/equip/push-heavy-door.wav";
char *Sound::OPEN_BOX = "sound/equip/open-box.wav";

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

		int reserved_count;
		reserved_count=Mix_ReserveChannels(8);
		if(reserved_count!=8) {
		  cerr << "reserved " << reserved_count << " channels from default mixing." << endl;
		  cerr << "8 channels were not reserved!" << endl;
		  // this might be a critical error...
		}

		lastChapter = -1;
		missionMusicIndex = -1;
		currentMusic = currentLevelMusic = menuMusic = hqMusic = missionMusic = fightMusic = chapterMusic = NULL;
		musicStartTime = 0;
		musicPosition = 0;
		if(haveSound) {
			selectMusic( preferences );
		}
#endif
	}
}

Sound::~Sound() {
#ifdef HAVE_SDL_MIXER
	if(haveSound) {
		// delete music
		if(menuMusic) {
			Mix_FreeMusic( menuMusic );
			menuMusic = NULL;
		}
		if(hqMusic) {
			Mix_FreeMusic(hqMusic);
			hqMusic = NULL;
		}
		if( missionMusic ) {
			Mix_FreeMusic( missionMusic );
			missionMusic = NULL;
		}
		if( fightMusic ) {
			Mix_FreeMusic( fightMusic );
			fightMusic = NULL;
		}
		if( !chapterMusic ) {
			Mix_FreeMusic( chapterMusic );
			chapterMusic = NULL;
		}
		// delete sounds
		for(map<string, Mix_Chunk*>::iterator i=soundMap.begin(); i != soundMap.end(); ++i) {
			Mix_Chunk *sample = i->second;
			Mix_FreeChunk(sample);
			sample = NULL;
		}
		if( !soundMap.empty() ) soundMap.clear();
		// stop audio system
		Mix_CloseAudio();
	}
#endif
}

// randomly select mission music, load others if needed
void Sound::selectMusic( Preferences *preferences, Mission * mission ) {
#ifdef HAVE_SDL_MIXER
	if( haveSound ) {
   // load fixed musics if needed
		if(!menuMusic) {
			string fn = rootDir + "/sound/music/menu.ogg";
			menuMusic = Mix_LoadMUS(fn.c_str());
			if( !menuMusic ) {
				cerr << "*** Error: couldn't load music: " << fn << endl;
				cerr << "\t" << Mix_GetError() << endl;
			}
		}
		if(!hqMusic) {
			string fn = rootDir + "/sound/music/headquarter.ogg";
			hqMusic = Mix_LoadMUS(fn.c_str());
			if( !hqMusic ) {
				cerr << "*** Error: couldn't load music: " << fn << endl;
				cerr << "\t" << Mix_GetError() << endl;
			}
		}
		if( mission && mission->isStoryLine() ) {
			if( lastChapter != mission->getChapter() ) {
				if( !chapterMusic ) {
					Mix_FreeMusic( chapterMusic );
					chapterMusic = NULL;
				}
				lastChapter = mission->getChapter();
				stringstream filename;
				filename << rootDir << "/sound/music/chapters/chapter" << lastChapter << ".ogg";
				string fn = filename.str();
				chapterMusic = Mix_LoadMUS( fn.c_str() );
				if( !chapterMusic ) {
					cerr << "*** Error: couldn't load music: " << fn << endl;
					cerr << "\t" << Mix_GetError() << endl;
					cerr << "\tfalling back to chapter 1 music." << endl;
					fn = rootDir + "/sound/music/chapters/chapter1.ogg";
					chapterMusic = Mix_LoadMUS( fn.c_str() );
					if( !chapterMusic ) {
						cerr << "*** Error: couldn't load music: " << fn << endl;
						cerr << "\t" << Mix_GetError() << endl;
					}
				}
			}
		}


		//select mission music
		// unload the current one
		if(missionMusic) {
			Mix_FreeMusic(missionMusic);
			missionMusic = NULL;
		}

		stringstream filename;
		if(mission && mission->getMusicTrack()) {
			//selects mission specific track 
			filename << rootDir << "/sound/music/" << mission->getMusicTrack() << ".ogg";
		}
		else {
			//selects random one
			missionMusicIndex = Util::pickOne( 1, MISSION_MUSIC_COUNT );
			filename << rootDir << "/sound/music/track" << setw(2) << setfill('0') << missionMusicIndex << ".ogg";
		}
		string fn = filename.str();

  // load the new one
		missionMusic = Mix_LoadMUS( fn.c_str() );
		if( !missionMusic ) {
			cerr << "*** Error: couldn't load music: " << fn << endl;
			cerr << "\t" << Mix_GetError() << endl;
		}

		// selects fight music
		int n = Util::pickOne( 1, FIGHT_MUSIC_COUNT );
		if( fightMusicIndex != n ) {
			fightMusicIndex = n;

			// unload the current one
			if( fightMusic ) {
				Mix_FreeMusic( fightMusic );
				fightMusic = NULL;
			}

			// load the new one
			filename.clear();
			filename << rootDir << "/sound/music/fight" << setw(2) << setfill('0') << fightMusicIndex << ".ogg";
			string fn = filename.str();
			fightMusic = Mix_LoadMUS( fn.c_str() );
			if( !fightMusic ) {
				cerr << "*** Error: couldn't load music: " << fn << endl;
				cerr << "\t" << Mix_GetError() << endl;
			}
		}

		setMusicVolume( preferences->getMusicVolume() );
	}
#endif
}


void Sound::playMusic( Mix_Music *music, int ms, int loopCount ) {
#ifdef HAVE_SDL_MIXER
	if( haveSound && music ) {
		currentMusic = music;
		if( currentMusic != fightMusic ) {
			if( currentLevelMusic != currentMusic ) {
				currentLevelMusic = currentMusic;
				musicStartTime = SDL_GetTicks();
				musicPosition = 0;
			}
			if( Mix_FadeInMusicPos( music, loopCount, ms, musicPosition ) ) {
				cerr << "*** Error playing music: " << Mix_GetError() << endl;
			}
		} else {
			if( Mix_FadeInMusic( music, loopCount, ms ) ) {
				cerr << "*** Error playing music: " << Mix_GetError() << endl;
			}
		}
	}
#endif
}

void Sound::stopMusic( int ms ) {
#ifdef HAVE_SDL_MIXER
	if(haveSound) {

		// remember where the level music is stopped
		if( currentMusic != fightMusic ) {
			musicPosition = (double)( SDL_GetTicks() - musicStartTime );
		}

		if( !Mix_FadeOutMusic( ms ) ) {
		//  cerr << "*** Error stopping music: " << Mix_GetError() << endl;
			// force stop music
			Mix_HaltMusic();
		}
	}
#endif
}


void Sound::checkMusic( bool inCombat ) {
#ifdef HAVE_SDL_MIXER
	if( haveSound ) {
		Mix_Music *should = ( inCombat ? fightMusic : currentLevelMusic );
		if( should != currentMusic ) {
			if( currentMusic )
				stopMusic( 500 );
			playMusic( should, 500 );
		}
	}
#endif
}

void Sound::loadUISounds(Preferences *preferences) {
	if(!haveSound) return;

//  cerr << "Loading UI sounds..." << endl;
	storeSound(0, Window::ROLL_OVER_SOUND);
	storeSound(0, Window::ACTION_SOUND);
	storeSound(0, Window::DROP_SUCCESS);
	storeSound(0, Window::DROP_FAILED);

	storeSound( 0, TELEPORT );
	storeSound( 0, OPEN_DOOR );
	storeSound( 0, OPEN_BOX );

	setEffectsVolume(preferences->getEffectsVolume());

}

void Sound::loadSounds(Preferences *preferences) {
	if(!haveSound)
		return;

	//cerr << "Loading battle sounds..." << endl;
	for(int i = 0; i < Battle::getSoundCount(); i++) {
		storeSound(0, Battle::getSound(i));
	}

	//cerr << "Loading item sounds..." << endl;
	for(map<int, vector<string>*>::iterator i = Item::soundMap.begin();
				i != Item::soundMap.end(); ++i) {
		//Creature *creature = i->first;
		vector<string> *v = i->second;
		for(int r = 0; r < (int)v->size(); r++) {
			string file = (*v)[r];
			storeSound(0, file.c_str());
		}
	}

	//cerr << "Loading spell sounds..." << endl;
	for(int i = 0; i < (int)MagicSchool::getMagicSchoolCount(); i++) {
		for(int t = 0; t < (int)MagicSchool::getMagicSchool(i)->getSpellCount(); t++) {
			storeSound(0, MagicSchool::getMagicSchool(i)->getSpell(t)->getSound());
		}
	}
	
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

void Sound::loadCharacterSounds( char *type ) {
#ifdef HAVE_SDL_MIXER
	if( haveSound ) {
		//std::map<std::string, std::map<int, std::vector<Mix_Chunk*>* >* > characterSounds;
		string typeStr = type;
		map<int,vector<Mix_Chunk*>*>* charSoundMap;
		if( characterSounds.find( typeStr ) == characterSounds.end() ) {
			charSoundMap = new map<int,vector<Mix_Chunk*>*>();
			characterSounds[ typeStr ] = charSoundMap;
		} else {
			charSoundMap = characterSounds[ typeStr ];
		}

		// load the sounds
		storeCharacterSounds( charSoundMap, type, GameAdapter::COMMAND_SOUND, "command" );
		storeCharacterSounds( charSoundMap, type, GameAdapter::HIT_SOUND, "hit" );
		storeCharacterSounds( charSoundMap, type, GameAdapter::SELECT_SOUND, "select" );    
	}
#endif
}

#ifdef HAVE_SDL_MIXER
void Sound::storeCharacterSounds( map<int,vector<Mix_Chunk*>*> *charSoundMap, 
                                  char *type, int soundType, char *filePrefix ) {  
	stringstream filename;
	vector<Mix_Chunk*> *v;
	if( charSoundMap->find( soundType ) == charSoundMap->end() ) {
		v = new vector<Mix_Chunk*>();
		(*charSoundMap)[ soundType ] = v;
	} else {
		v = (*charSoundMap)[ soundType ];
	}
	for( int i = 0; i < 100; i++ ) {
		filename << rootDir << type <<"/sound/" << filePrefix << (i + 1) << ".wav";
		//cerr << "Looking for character sound: " << filename << endl;
		Mix_Chunk *sample = Mix_LoadWAV( filename.str().c_str() );
		if( !sample ) break;
		//cerr << "Loaded character sound: " << filename << endl;
		v->push_back( sample );
	}
}
#endif

void Sound::playCharacterSound( char *type, int soundType ) {
#ifdef HAVE_SDL_MIXER
	if( haveSound ) {
		string typeStr = type;
		if( characterSounds.find( typeStr ) != characterSounds.end() ) {
			map<int,vector<Mix_Chunk*>*> *charSoundMap = characterSounds[ typeStr ];
			if( charSoundMap->find( soundType ) != charSoundMap->end() ) {
				vector<Mix_Chunk*> *v = (*charSoundMap)[ soundType ];
				if( v->size() > 0 ) {
					int index = Util::dice( v->size() );
					if( Mix_PlayChannel( -1, (*v)[ index ], 0 ) == -1 ) {
						// commented out; happens too often
						//cerr << "*** Error playing WAV file: " << fileStr << endl;
						//cerr << "\t" << Mix_GetError() << endl;
					}
				}
			}
		}
	}
#endif
}

void Sound::unloadCharacterSounds( char *type ) {
#ifdef HAVE_SDL_MIXER
	if( haveSound ) {
		string typeStr = type;
		if( characterSounds.find( typeStr ) != characterSounds.end() ) {
			map<int,vector<Mix_Chunk*>*>* charSoundMap = characterSounds[ typeStr ];
			for( map<int, vector<Mix_Chunk*>*>::iterator i = charSoundMap->begin(); 
						i != charSoundMap->end(); ++i) {
				vector<Mix_Chunk*> *v = i->second;
				for( vector<Mix_Chunk*>::iterator i2 = v->begin(); i2 != v->end(); ++i2 ) {
					Mix_Chunk *sample = *i2;
					Mix_FreeChunk( sample );
				}
				v->clear();
			}
		}
	}
#endif
}

void Sound::storeSound(int type, const string& file) {
#ifdef HAVE_SDL_MIXER
	if(haveSound) {
		string fileStr = file;
		if(soundMap.find(fileStr) == soundMap.end()) {
			string fn;
			if( file[0] == '/' || file[0] == '\\' ) {
				fn = rootDir + file;
			} else {
				fn = rootDir + ("/" + file);
			}
//      cerr << "*** Loading sound file: " << fn << endl;
			Mix_Chunk *sample = Mix_LoadWAV(fn.c_str());
			if(!sample) {
        // commented out; happens too often to be meaningful
			  // cerr << "*** Error loading WAV file (" << file << "): " << Mix_GetError() << endl;
			} else {
				soundMap[fileStr] = sample;
			}
		}
	}
#endif
}

void Sound::unloadSound( int type, const string& file ) {
#ifdef HAVE_SDL_MIXER
	if(haveSound) {
		if(soundMap.find(file) != soundMap.end()) {
//	  cerr << "*** Freeing sound: " << fileStr << endl;
			Mix_Chunk *sample = soundMap[ file ];
			Mix_FreeChunk( sample );
			soundMap.erase( file );
		}
  }
#endif
}

// ######################################
void Sound::playSound(const string& file) {
#ifdef HAVE_SDL_MIXER
	if(haveSound) {
		//cerr << "*** Playing WAV: " << file << endl;
		if(soundMap.find(file) != soundMap.end()) {
			for( int t = 0; t < 5; t++ ) {
				for( int i = 0; i < 3; i++ ) {
					if(Mix_PlayChannel(i, soundMap[file], 0) == i) return;
				}
			}
		}
  }
#endif
}

void Sound::startFootsteps( std::string& name, int depth ) {
#ifdef HAVE_SDL_MIXER
	AmbientSound *as = getAmbientSound( name, depth );
	if( as ) as->playFootsteps();
#endif
}

void Sound::stopFootsteps() {
#ifdef HAVE_SDL_MIXER
  Mix_HaltChannel( 7 );
#endif
}

void Sound::addAmbientSound( std::string& name, std::string& ambient, std::string& footsteps, std::string& afterFirstLevel ) {
	#ifdef HAVE_SDL_MIXER
	ambients[name] = new AmbientSound( name, ambient, footsteps, afterFirstLevel );
	#endif
}
void Sound::startAmbientSound( std::string& name, int depth ) {
	#ifdef HAVE_SDL_MIXER
		AmbientSound *as = getAmbientSound( name, depth );
		if( as ) as->playRandomAmbientSample();
	#endif
}

void Sound::stopAmbientSound() {
#ifdef HAVE_SDL_MIXER
	Mix_HaltChannel( 6 );
#endif
}

AmbientSound *Sound::getAmbientSound( std::string& name, int depth ) {
	AmbientSound *as = NULL;
	if( ambients.find( name ) != ambients.end() ) {
		as = ambients[ name ];
		if( depth > 0 ) {
			if( ambients.find( as->getAfterFirstLevel() ) != ambients.end() ) {
				as = ambients[ as->getAfterFirstLevel() ];
			} else {
				cerr << "2 Ambient not found: " << as->getAfterFirstLevel() << endl;
				as = NULL;
			}
		}
	} else {
		cerr << "1 Ambient not found: " << name << endl;
	}
	return as;
}

// ######################################

AmbientSound::AmbientSound( std::string& name, std::string& ambient, std::string& footsteps, std::string& afterFirstLevel ) {
	this->name = name;
	this->afterFirstLevel = afterFirstLevel;
#ifdef HAVE_SDL_MIXER
	size_t start = 0;
	size_t found = ambient.find( "," );

	//cerr << "Loading ambients: " << name << endl;
	//cerr << "ambients=" << ambient << endl;
	while( found != string::npos ) {
		string s = ambient.substr( start, found - start );
		//cerr << "\tstart=" << start << " found=" << found << " s=" << s << endl;
		start = found + 1;
		found = ambient.find( ",", start );

		stringstream filename;
		filename << rootDir <<"/sound/ambient/" << s;
		//cerr << "\t" << filename.str() << endl;
		Mix_Chunk *sample = Mix_LoadWAV( filename.str().c_str() );
		if( !sample ) {
			cerr << "*** Error cannot load sound sample: " << filename.str() << " reason=" << Mix_GetError() << endl;
		} else {
			ambients.push_back( sample );
		}
	}
	stringstream filename;
	filename << rootDir <<"/sound/footsteps/" << footsteps;
	//cerr << "\t" << filename.str() << endl;
	this->footsteps = Mix_LoadWAV( filename.str().c_str() );
	if( !this->footsteps ) {
		cerr << "*** Error cannot load sound sample: " << filename.str() << " reason=" << Mix_GetError() << endl;
	}
#endif
}

AmbientSound::~AmbientSound() {
#ifdef HAVE_SDL_MIXER
	// halt playback on all channels
	Mix_HaltChannel( -1 );
	Mix_FreeChunk( this->footsteps );
	for( unsigned n = 0; n < ambients.size(); n++ ) {
		Mix_FreeChunk( this->ambients[ n ] );
	}
#endif
}

int AmbientSound::playRandomAmbientSample() {
#ifdef HAVE_SDL_MIXER
	int n = Util::dice( ambients.size() );
	for( int t = 0; t < 5; t++ ) {
		if( Mix_PlayChannel( 6, ambients[ n ], 0 ) ) return 1;
	}
#endif
	return -1;
}

int AmbientSound::playFootsteps() {
#ifdef HAVE_SDL_MIXER
	for( int i = 0; i < 5; i++ ) {
		if(Mix_PlayChannel(7, footsteps, 0) != -1) return 1;
  }
#endif
	return -1;
}

// ######################################

void Sound::setMusicVolume(int volume) {
#ifdef HAVE_SDL_MIXER
	if(haveSound)
		Mix_VolumeMusic(volume);
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

