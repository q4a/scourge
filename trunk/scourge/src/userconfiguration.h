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
#include "sdl.h"

using namespace std;

// Max number of event binding for each engine action
#define MAX_BINDING 3
#define CONFIG_FILE_NAME "saved/scourge.cfg"

// set to non-zero for debugging
#define DEBUG_USER_CONFIG 0   

#define ENGINE_ACTION_UP_COUNT 12   // the number of engine up actions


class UserConfiguration{

private:
  static const char * ENGINE_ACTION_NAMES[];     
  static const char * ENGINE_ACTION_UP_NAMES[];
  ifstream *configFile;  
  
  // Associate keys and mouse buttons to an engineAction  
  map<string, string> keyDownBindings;
  map<string, string> keyUpBindings;
  map<Uint8, string> mouseDownBindings;
  map<Uint8, string> mouseUpBindings;    
  map<string, int> engineActionUpNames; 
       
  // Associate the SDL event to an engine action  
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
  inline int getBpp()        { return bpp;        }
  inline int getW()          { return w;          }
  inline int getH()          { return h;          }
  inline int getShadows()    { return shadows;    }
     
  // reads the configuration file where keys are binded, returns -1 if error
  void loadConfiguration(); 
    
  void bind(string s1, string s2, int lineNumber);
  void set (string s1, string s2, int lineNumber);
  
  // returns the action to do for this event
  string getEngineAction(SDL_Event *event);   
  
  
};

#endif

