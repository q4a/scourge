/***************************************************************************
                          examplepreferences.h  -  description
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

#ifndef EXAMPLE_PREFERENCES_H
#define EXAMPLE_PREFERENCES_H


#include <constants.h>
#include <preferences.h>

using namespace std;

class ExamplePreferences : public Preferences {

private:
  int bpp;
   
public:
   
  ExamplePreferences::ExamplePreferences() {
    bpp = -1;
  }
  virtual ExamplePreferences::~ExamplePreferences() {
  }
  
  // engine variables
  virtual inline bool getFullscreen() { return false; }
  virtual inline bool getDoublebuf() { return false; }
  virtual inline bool getHwpal() { return false; }
  virtual inline bool getResizeable() { return false; }
  virtual inline bool getForce_hwsurf() { return false; }
  virtual inline bool getForce_swsurf() { return false; }
  virtual inline bool getHwaccel() { return true; }
  virtual inline bool getTest() { return false; }
  virtual inline bool getStencilbuf() { return false; }
  virtual inline bool getMultitexturing() { return true; }
  virtual inline int getBpp() { return this->bpp; } // sdl will guess it
  virtual inline int getW() { return 640; }
  virtual inline int getH() { return 480; }
  virtual inline int getShadows() { return 0; }
  virtual inline int getGameSpeedLevel() { return 0; }
  virtual inline bool getAlwaysCenterMap() { return false; }
  virtual inline bool getKeepMapSize() { return false; }
  virtual inline bool getFrameOnFullScreen() { return true; }
  virtual inline bool isBattleTurnBased() { return true; }
  virtual inline bool isOvalCutoutShown() { return true; }
  virtual inline bool isOutlineInteractiveItems() { return true; }
  virtual inline int getSoundFreq() { return 0; }
  virtual inline bool isSoundEnabled() { return false; }
  virtual inline int getMusicVolume() { return 0; }
  virtual inline int getEffectsVolume() { return 0; }
  virtual inline bool getAlwaysShowPath() { return false; }
  virtual inline bool getTooltipEnabled() { return true; }
  virtual inline int getTooltipInterval() { return 200; }
  virtual inline int getGameSpeedTicks() { return 100; }
  virtual inline int getStandAloneMode() { return 0; }
  virtual inline char *getHost() { return ""; }
  virtual inline char *getUserName() { return ""; }
  virtual inline int getPort() { return 0; }

  virtual inline void setBpp(int t) { 
    this->bpp = t;
  }

  /**
   * Convert the sdl event to an engine action (see preferences.h for the enum).
   * This conversion is needed so you can map keys to different actions.
   * Here I only listed the values used by the map.
   */
  virtual inline int getEngineAction( SDL_Event *event ) { 
    switch( event->type ) {
    case SDL_KEYDOWN:
      switch( event->key.keysym.sym ) {
      case SDLK_DOWN: return SET_MOVE_DOWN;
      case SDLK_UP: return SET_MOVE_UP;
      case SDLK_LEFT: return SET_MOVE_LEFT;
      case SDLK_RIGHT: return SET_MOVE_RIGHT;
      case SDLK_z: return SET_ZOOM_IN;
      case SDLK_x: return SET_ZOOM_OUT;
      default: break;
      }
      break;
    case SDL_KEYUP:
      switch( event->key.keysym.sym ) {
      case SDLK_DOWN: return SET_MOVE_DOWN_STOP;
      case SDLK_UP: return SET_MOVE_UP_STOP;
      case SDLK_LEFT: return SET_MOVE_LEFT_STOP;
      case SDLK_RIGHT: return SET_MOVE_RIGHT_STOP;
      case SDLK_z: return SET_ZOOM_IN_STOP;
      case SDLK_x: return SET_ZOOM_OUT_STOP;
      default: break;
      }
      break;
    default: break;
    }
    return 0;
  }
  
  virtual inline void createConfigDir() { 
    //cerr << "Implement me: createConfigDir()." << endl; 
  }
};

#endif

