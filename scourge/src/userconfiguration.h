/***************************************************************************
                          userconfiguration.h  -  description
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


#include <fstream>
#include <iostream>
#include <string>
#include <map>
#include "constants.h"

using namespace std;

// Max number of event binding for each engine action
//#define MAX_BINDING 3
#define CONFIG_FILE_NAME "saved/scourge.cfg"

// set to non-zero for debugging
#define DEBUG_USER_CONFIG 0   

// All engine action that can be binded
// If you change this, ALSO change ENGINE_ACTION_NAMES in userconfiguration.cpp
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
    BLEND_A,
    BLEND_B,    
    
    SHOW_INVENTORY, 
    SHOW_OPTIONS_MENU,
    USE_ITEM,
    SET_NEXT_FORMATION,
    
    SET_X_ROT_PLUS,   
    SET_X_ROT_MINUS,    
    SET_Y_ROT_PLUS,
    SET_Y_ROT_MINUS,    
    SET_Z_ROT_PLUS,        
    SET_Z_ROT_MINUS,
     
    ADD_X_POS_PLUS,
    ADD_X_POS_MINUS,
    ADD_Y_POS_PLUS,
    ADD_Y_POS_MINUS,
    ADD_Z_POS_PLUS,
    ADD_Z_POS_MINUS,
    
    MINIMAP_ZOOM_IN,
    MINIMAP_ZOOM_OUT,
    MINIMAP_TOGGLE,
    
    SET_ZOOM_IN,     
    SET_ZOOM_OUT,
         
    START_ROUND, 
    
    // must be last
    ENGINE_ACTION_COUNT
    
};

// All engine actions that have a corresponding keyup action
// If you change this, ALSO change ENGINE_ACTION_UP_NAMES in userconfiguration.cpp
enum engine_action_up_int{

    SET_MOVE_DOWN_STOP = 500,
    SET_MOVE_RIGHT_STOP, 
    SET_MOVE_UP_STOP,
    SET_MOVE_LEFT_STOP,
    SET_X_ROT_PLUS_STOP,
    SET_X_ROT_MINUS_STOP,    
    SET_Y_ROT_PLUS_STOP,
    SET_Y_ROT_MINUS_STOP,    
    SET_Z_ROT_PLUS_STOP,        
    SET_Z_ROT_MINUS_STOP,
    SET_ZOOM_IN_STOP,     
    SET_ZOOM_OUT_STOP,
    USE_ITEM_STOP,
    SET_NEXT_FORMATION_STOP,
    
    // must be last one
    ENGINE_ACTION_UP_COUNT
};

class UserConfiguration{

private:
  static const char * ENGINE_ACTION_NAMES[];     
  static const char * ENGINE_ACTION_UP_NAMES[];
  ifstream *configFile;  
  
  // Associate keys and mouse buttons to an engineAction  
  map<string, int> keyDownBindings;     // string keyName -> int ea
  map<string, int> keyUpBindings;       // string keyName -> int ea
  map<Uint8, int> mouseDownBindings;    // uint8 mouseButton -> int ea
  map<Uint8, int> mouseUpBindings;      // uint8 mouseButton -> int ea 
  map<string, int> engineActionUpNames; // string ea -> int ea
  map<string, int> engineActionNames;   // string ea -> int ea
       
  // return next word from string or empty string
  string getNextWord(const string theInput, int fromPos, int &endWord);
  
  // replace " " by "_" in a string
  string replaceSpaces(string s);  
  
  // engine variables (video settings) 
  bool fullscreen;
  bool doublebuf;
  bool hwpal;
  bool resizeable;
  bool force_hwsurf;
  bool force_swsurf;
  bool hwaccel; 
  bool test;   
  int bpp;
  int w;
  int h;  
  int shadows;
   
 public:
  UserConfiguration::UserConfiguration();
  UserConfiguration::~UserConfiguration();
      
  // engine variables
  inline bool getFullscreen(){ return fullscreen; }
  inline bool getDoublebuf() { return doublebuf;  }
  inline bool getHwpal()     { return hwpal;      }
  inline bool getResizeable(){ return resizeable; }
  inline bool getForce_hwsurf() { return force_hwsurf; }
  inline bool getForce_swsurf() { return force_swsurf; }
  inline bool getHwaccel()   { return hwaccel;    }
  inline bool getTest()      { return test;       }   
  inline int getBpp()        { return bpp;        }
  inline int getW()          { return w;          }
  inline int getH()          { return h;          }
  inline int getShadows()    { return shadows;    }   
  
  inline bool setFullscreen(bool t){ fullscreen=t; }
  inline bool setDoublebuf(bool t) { doublebuf=t;  }
  inline bool setHwpal(bool t)     { hwpal=t;      }
  inline bool setResizeable(bool t){ resizeable=t; }
  inline bool setForce_hwsurf(bool t) { force_hwsurf=t; }
  inline bool setForce_swsurf(bool t) { force_swsurf=t; }
  inline bool setHwaccel(bool t)   { hwaccel=t;    }   
  inline int setBpp(int t)         { bpp=t;        }
  inline int setW(int t)           { w=t;          }
  inline int setH(int t)           { h=t;          }
  inline int setShadows(int t)     { shadows=t;    }
  
     
  // reads the configuration file where keys are bindeds
  void loadConfiguration(); 
  
  // Associate SDL events to an engine action    
  void bind(string s1, string s2, int lineNumber);
  
  // Read in engine variables from file
  void set (string s1, string s2, int lineNumber);
  
  // returns the action to do for this event
  int getEngineAction(SDL_Event *event); 
  
  void parseCommandLine(int argc, char *argv[]);
  
  
};

#endif

