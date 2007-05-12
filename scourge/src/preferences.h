/***************************************************************************
                          preferences.h  -  description
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

#ifndef PREFERENCES_H
#define PREFERENCES_H


#include "common/constants.h"

// All engine action that can be binded
// If you change this, ALSO change ENGINE_ACTION_NAMES in userconfiguration.cpp
// AND update ENGINE_ACTION_DEBUG_IND (first indice of invisible engine actions)
// AND add a corresponding bind line in config file (to have a default value)
enum engine_action_int{
    
  SET_MOVE_DOWN = 0,
  SET_MOVE_RIGHT,
  SET_MOVE_UP,
  SET_MOVE_LEFT,
  
  SET_PLAYER_0,
  SET_PLAYER_1,
  SET_PLAYER_2,
  SET_PLAYER_3,
  SET_PLAYER_ONLY,      
  
  SHOW_INVENTORY, 
  SHOW_OPTIONS_MENU,
  SET_NEXT_FORMATION,
  
  TOGGLE_MINIMAP,
  
  SET_ZOOM_IN,     
  SET_ZOOM_OUT,
  
  TOGGLE_MAP_CENTER, 
  INCREASE_GAME_SPEED, 
  DECREASE_GAME_SPEED, 
  
  START_ROUND, 
  
  LAYOUT_1,
  LAYOUT_2,
  LAYOUT_4,
  
  SWITCH_COMBAT,
  
  NEXT_WEAPON,
  
  QUICK_SPELL_1,
  QUICK_SPELL_2,
  QUICK_SPELL_3,      
  QUICK_SPELL_4,      
  QUICK_SPELL_5,      
  QUICK_SPELL_6,      
  QUICK_SPELL_7,      
  QUICK_SPELL_8,      
  QUICK_SPELL_9,      
  QUICK_SPELL_10,      
  QUICK_SPELL_11,      
  QUICK_SPELL_12,      
  
  // must be last
  ENGINE_ACTION_COUNT    
};

// All engine actions that have a corresponding keyup action
// If you change this, ALSO change ENGINE_ACTION_UP_NAMES in userconfiguration.cpp
enum engine_action_up_int{

  SET_MOVE_DOWN_STOP = 500,   // Must be first
  SET_MOVE_RIGHT_STOP, 
  SET_MOVE_UP_STOP,
  SET_MOVE_LEFT_STOP,
  SET_Y_ROT_PLUS_STOP,
  SET_Y_ROT_MINUS_STOP,    
  SET_Z_ROT_PLUS_STOP,        
  SET_Z_ROT_MINUS_STOP,
  SET_ZOOM_IN_STOP,     
  SET_ZOOM_OUT_STOP,
  SET_NEXT_FORMATION_STOP,    
    
  // must be the last one
  ENGINE_ACTION_UP_COUNT
};

class Preferences {

protected:
  // was the stencil buf. initialized at start?
  bool stencilBufInitialized;
   
public:
   
  enum {
    NONE = 0,
    SERVER,
    CLIENT,
    TEST
  };
 
  Preferences() {
    stencilBufInitialized = false;
  }
  virtual ~Preferences() {}
  
  // engine variables
  virtual bool getFullscreen() = 0;
  virtual bool getDoublebuf() = 0;
  virtual bool getHwpal() = 0;
  virtual bool getResizeable() = 0;
  virtual bool getForce_hwsurf() = 0;
  virtual bool getForce_swsurf() = 0;
  virtual bool getHwaccel() = 0;
  virtual bool getTest() = 0;
  virtual bool getStencilbuf() = 0;
  virtual bool getMultitexturing() = 0;
  virtual int getBpp() = 0;
  virtual int getW() = 0;
  virtual int getH() = 0;
  virtual int getShadows() = 0;
  virtual int getGameSpeedLevel() = 0; // [0, 1, 2, 3, 4] 
  virtual bool getAlwaysCenterMap() = 0;
  virtual bool getKeepMapSize() = 0;
  virtual bool getFrameOnFullScreen() = 0;
  virtual bool isBattleTurnBased() = 0;
  virtual bool isOvalCutoutShown() = 0;
  virtual bool isOutlineInteractiveItems() = 0;
  virtual int getCombatInfoDetail() = 0;
  virtual int getSoundFreq() = 0;
  virtual bool isSoundEnabled() = 0;
  virtual int getMusicVolume() = 0;
  virtual int getEffectsVolume() = 0;
  virtual bool getAlwaysShowPath() = 0;
  virtual bool getTooltipEnabled() = 0;
  virtual int getTooltipInterval() = 0;
  virtual int getGameSpeedTicks() = 0;
  virtual int getStandAloneMode() = 0;
  virtual char *getHost() = 0;
  virtual char *getUserName() = 0;
  virtual int getPort() = 0;
  virtual int getMonsterToughness() = 0;
	virtual bool getEnableScreenshots() = 0;

  virtual void setBpp(int t) = 0;

  // returns the action to do for this event
  virtual int getEngineAction(SDL_Event *event) = 0;
  
  inline void setStencilBufInitialized(bool b) { stencilBufInitialized = b; }
  inline bool getStencilBufInitialized() { return stencilBufInitialized; }

  virtual void createConfigDir() = 0;

	virtual bool isDebugTheme() = 0;
};

#endif

