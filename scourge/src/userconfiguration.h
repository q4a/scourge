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
#define CONFIG_FILE_NAME "saved/scourge.cfg"

// Indice for the first debug engine action defined in engine_action_int
#define ENGINE_ACTION_DEBUG_IND 26

// set to non-zero for debugging
#define DEBUG_USER_CONFIG 0  

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
    USE_ITEM,
    SET_NEXT_FORMATION,
        
    SET_Y_ROT_PLUS,
    SET_Y_ROT_MINUS,    
    SET_Z_ROT_PLUS,        
    SET_Z_ROT_MINUS,         
    
    MINIMAP_ZOOM_IN,
    MINIMAP_ZOOM_OUT,
    MINIMAP_TOGGLE,
    
    SET_ZOOM_IN,     
    SET_ZOOM_OUT,
    
    TOGGLE_MAP_CENTER, 
    INCREASE_GAME_SPEED, 
    DECREASE_GAME_SPEED, 
    
    START_ROUND, 
        
    // Debug engine actions invisible for user (not saved or loaded)
    BLEND_A,        
    BLEND_B,  
    SET_X_ROT_PLUS,   
    SET_X_ROT_MINUS,
    ADD_X_POS_PLUS,
    ADD_X_POS_MINUS,
    ADD_Y_POS_PLUS,
    ADD_Y_POS_MINUS,
    ADD_Z_POS_PLUS,
    ADD_Z_POS_MINUS,                 
    
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
    
    // must be the last one
    ENGINE_ACTION_UP_COUNT
};


class UserConfiguration{

private:
  static const char * ENGINE_ACTION_NAMES[];     
  static const char * ENGINE_ACTION_UP_NAMES[];
  static const char * ENGINE_ACTION_DESCRIPTION[];   
  
  // becomes true every time loadConfiguration is called  
  // and false every time getConfigurationChanged is called
  bool configurationChanged; 
  
  // mappings to speed-up search processing
  map<string, int> keyDownBindings;      // string keyName -> int ea
  map<string, int> keyUpBindings;        // string keyName -> int ea
  map<Uint8, int> mouseDownBindings;     // uint8 mouseButton -> int ea
  map<Uint8, int> mouseUpBindings;       // uint8 mouseButton -> int ea 
  map<string, int> engineActionUpNumber; // string ea -> int ea
  map<string, int> engineActionNumber;   // string ea -> int ea
  map<int, string> keyForEngineAction;   // int ea -> string keyName
  map<int, string> engineActionName;     // int ea -> string ea  
       
  // return next word from string or empty string
  string getNextWord(const string theInput, int fromPos, int &endWord);
  
  // replace " " by "_" in a string
  string replaceSpaces(string s);
  
  void writeFile(ofstream *fileOut, char *text);  
  
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
  inline int getEngineActionCount() { return ENGINE_ACTION_DEBUG_IND; }
  const char * getEngineActionDescription(int i);  
  const char * getEngineActionKeyName(int i);
  //bool getConfigurationChanged();
  
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
  
  bool isDebugEa(int j);
   
     
  // reads the configuration file where keys are binded
  void loadConfiguration();
  
  // save configuration into file
  void saveConfiguration(char **controlLine); 
  
  // Associate SDL events to an engine action    
  void bind(string s1, string s2, int lineNumber);
  
  // Read in engine variables from file
  void set (string s1, string s2, int lineNumber); 
  
  // returns the action to do for this event
  int getEngineAction(SDL_Event *event); 
  
  void parseCommandLine(int argc, char *argv[]);  
  void setKeyForEngineAction(string keyName, int ea);
  
  
};

#endif

