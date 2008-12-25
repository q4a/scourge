/***************************************************************************
               userconfiguration.h  -  Manages game preferences
                             -------------------
    begin                : Sat Feb 14 2004
    copyright            : (C) 2004 by Daroth-U
    email                : daroth-u@ifrance.com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef USER_CONFIGURATION_H
#define USER_CONFIGURATION_H
#pragma once

#include <fstream>
#include <iostream>
#include <string>
#include <map>
#include "preferences.h"

// set to non-zero for debugging
#define DEBUG_USER_CONFIG 0

/// Interface to the saved game options.
class UserConfiguration : public Preferences {

private:
	static const char * ENGINE_ACTION_NAMES[];
	static const char * ENGINE_ACTION_UP_NAMES[];
	static const char * ENGINE_ACTION_DESCRIPTION[];
	static const char default_key[][40];

	// becomes true every time loadConfiguration is called
	// and false every time getConfigurationChanged is called
	bool configurationChanged;

	// mappings to speed-up search processing
	std::map<std::string, int> keyDownBindings;      // string keyName -> int ea
	std::map<std::string, int> keyUpBindings;        // string keyName -> int ea
	std::map<Uint8, int> mouseDownBindings;     // uint8 mouseButton -> int ea
	std::map<Uint8, int> mouseUpBindings;       // uint8 mouseButton -> int ea
	std::map<std::string, int> engineActionUpNumber; // string ea -> int ea
	std::map<std::string, int> engineActionNumber;   // string ea -> int ea
	std::map<int, std::string> keyForEngineAction;   // int ea -> string keyName
	std::map<int, std::string> engineActionName;     // int ea -> string ea

	// return next word from string or empty string
	std::string getNextWord( const std::string theInput, int fromPos, int &endWord );

	// replace " " by "_" in a string
	void replaceSpaces( std::string& s );

	void writeFile( std::ofstream *fileOut, char *text );

	// engine variables (video settings)
	bool fullscreen;
	bool doublebuf;
	bool hwpal;
	bool resizeable;
	bool force_hwsurf;
	bool force_swsurf;
	bool hwaccel;
	bool test;
	bool multitexturing;
	bool stencilbuf;
	int bpp;
	int w;
	int h;
	int shadows;
	bool lights;
	bool alwaysShowPath;
	bool tooltipEnabled;
	int tooltipInterval;
	bool enableScreenshots;
	bool aniso_filter;

	// game settings
	int gamespeed;
	bool centermap;
	bool keepMapSize;
	bool frameOnFullScreen;
	bool turnBasedBattle;
	bool ovalCutoutShown;
	bool outlineInteractiveItems;
	int combatInfoDetail;
	bool hideInventoriesOnMove;
	int logLevel;
	int pathFindingQuality;
	long maxPathNodes;

	// audio settings
	bool soundEnabled;
	int soundFreq;
	int musicVolume;
	int effectsVolume;

	int standAloneMode;
	std::string host;
	std::string userName;
	int port;
	int monsterToughness;
	bool debugTheme;
	bool flaky;

public:

	UserConfiguration();
	~UserConfiguration();

	const char * getEngineActionDescription( int i );
	const char * getEngineActionKeyName( int i );

	// engine variables
	inline bool getFlaky()     {
		return flaky;
	}
	inline bool getFullscreen() {
		return fullscreen;
	}
	inline bool getDoublebuf() {
		return doublebuf;
	}
	inline bool getHwpal()     {
		return hwpal;
	}
	inline bool getResizeable() {
		return resizeable;
	}
	inline bool getForce_hwsurf() {
		return force_hwsurf;
	}
	inline bool getForce_swsurf() {
		return force_swsurf;
	}
	inline bool getHwaccel()   {
		return hwaccel;
	}
	inline bool getTest()      {
		return test;
	}
	inline bool getStencilbuf() {
		return stencilbuf;
	}
	inline bool getMultitexturing() {
		return Constants::multitexture;
	}
	inline int getBpp()        {
		return bpp;
	}
	inline int getW()          {
		return w;
	}
	inline int getH()          {
		return h;
	}
	inline int getShadows()    {
		return shadows;
	}
	inline bool getLights()    {
		return lights;
	}	
	inline bool getAnisoFilter() {
		return aniso_filter;
	}
	inline int getGameSpeedLevel()  {
		return gamespeed;
	} // [0, 1, 2, 3, 4]
	inline bool getAlwaysCenterMap() {
		return centermap;
	}
	inline bool getKeepMapSize() {
		return keepMapSize;
	}
	inline bool getFrameOnFullScreen() {
		return frameOnFullScreen;
	}
	inline bool isBattleTurnBased() {
		return turnBasedBattle;
	}
	inline bool isOvalCutoutShown() {
		return ovalCutoutShown;
	}
	inline bool isOutlineInteractiveItems() {
		return outlineInteractiveItems;
	}
	inline bool isHideInventoriesOnMove() {
		return hideInventoriesOnMove;
	}
	inline int getCombatInfoDetail() {
		return combatInfoDetail;
	}
	inline int getSoundFreq() {
		return soundFreq;
	}
	inline bool isSoundEnabled() {
		return soundEnabled;
	}
	inline int getMusicVolume() {
		return musicVolume;
	}
	inline int getEffectsVolume() {
		return effectsVolume;
	}
	inline bool getAlwaysShowPath() {
		return alwaysShowPath;
	}
	inline bool getTooltipEnabled() {
		return tooltipEnabled;
	}
	inline bool getEnableScreenshots() {
		return enableScreenshots;
	}
	inline int getTooltipInterval() {
		return tooltipInterval;
	}
	inline int getLogLevel() {
		return logLevel;
	}
	inline int getPathFindingQuality() {
		return pathFindingQuality;
	}
	inline long getMaxPathNodes() {
		return maxPathNodes;
	}
	inline int getMonsterToughness() {
		return monsterToughness;
	}

	inline void setFullscreen( bool t ) {
		fullscreen = t;
	}
	inline void setDoublebuf( bool t ) {
		doublebuf = t;
	}
	inline void setHwpal( bool t )     {
		hwpal = t;
	}
	inline void setResizeable( bool t ) {
		resizeable = t;
	}
	inline void setForce_hwsurf( bool t ) {
		force_hwsurf = t;
	}
	inline void setForce_swsurf( bool t ) {
		force_swsurf = t;
	}
	inline void setHwaccel( bool t )   {
		hwaccel = t;
	}
	inline void setStencilbuf( bool t ) {
		stencilbuf = t;
	}
	inline void setMultitexturing( bool t ) {
		Constants::multitexture = t;
	}
	inline void setBpp( int t )         {
		bpp = t;
	}
	inline void setW( int t )           {
		w = t;
	}
	inline void setH( int t )           {
		h = t;
	}
	inline void setShadows( int t )     {
		shadows = t;
	}
	inline void setLights( bool t )     {
		lights = t;
	}	
	inline void setAnisoFilter( bool t ) {
		aniso_filter = t;
	}
	inline void setGameSpeedLevel( int t )   {
		if ( t >= 0 && t <= 4 ) gamespeed = t;
	} // [0, 1, 2, 3, 4]
	inline void setAlwaysCenterMap( bool t ) {
		centermap = t;
	}
	inline void setKeepMapSize( bool t ) {
		keepMapSize = t;
	}
	inline void setFrameOnFullScreen( bool t ) {
		frameOnFullScreen = t;
	}
	inline void setBattleTurnBased( bool b ) {
		turnBasedBattle = b;
	}
	inline void setOvalCutoutShown( bool b ) {
		ovalCutoutShown = b;
	}
	inline void setOutlineInteractiveItems( bool b ) {
		outlineInteractiveItems = b;
	}
	inline void setCombatInfoDetail( int n ) {
		combatInfoDetail = n;
	}
	inline void setSoundFreq( int n ) {
		soundFreq = n;
	}
	inline void setSoundEnabled( bool b ) {
		soundEnabled = b;
	}
	inline void setMusicVolume( int n ) {
		musicVolume = n;
	}
	inline void setEffectsVolume( int n ) {
		effectsVolume = n;
	}
	inline void setAlwaysShowPath( bool b ) {
		alwaysShowPath = b;
	}
	inline void setTooltipEnabled( bool b ) {
		tooltipEnabled = b;
	}
	inline void setEnableScreenshots( bool b ) {
		enableScreenshots = b;
	}
	inline void setTooltipInterval( int n ) {
		tooltipInterval = n;
	}
	inline void setLogLevel( int t )   {
		if ( t >= 0 && t <= 3 ) logLevel = t;
	} // [0, 1, 2, 3]
	inline void setPathFindingQuality( int t ) {
		if ( t >= 0 && t <= 2 ) {
			pathFindingQuality = t; maxPathNodes =  pow( 10., t + 1. ) * 50;
		}
	}
	inline void setMonsterToughness( int n ) {
		monsterToughness = n;
	}

	// return gameSpeed in ticks
	int getGameSpeedTicks();


	// reads the configuration file where keys are binded
	void loadConfiguration();

	// save configuration into file
	void saveConfiguration();

	// Associate SDL events to an engine action
	void bind( std::string s1, std::string s2, int lineNumber );

	// Read in engine variables from file
	void set( std::string s1, std::string s2, int lineNumber );

	// returns the action to do for this event
	int getEngineAction( SDL_Event *event );

	void parseCommandLine( int argc, char *argv[] );
	void setKeyForEngineAction( std::string keyName, int ea );

	inline int getStandAloneMode() {
		return standAloneMode;
	}
	inline char const* getHost() {
		return host.c_str();
	}
	inline char const* getUserName() {
		return userName.c_str();
	}
	inline int getPort() {
		return port;
	}
	inline bool isDebugTheme() {
		return debugTheme;
	}

	void createConfigDir();

protected:
	void createDefaultConfigFile();
};

#endif

