/***************************************************************************
                          userconfiguration.cpp  -  description
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
 
#include "userconfiguration.h"

using namespace std;

#ifndef WIN32
#include <sys/stat.h>
#include <unistd.h>
#endif

#include "gui/guitheme.h"
 

// TODO : - warn if there is an unknown parameter in the config file ?
//        - manage doubled keynames -> must be impossible in optionsmenu
//        - make an array for variables too (they are all hard coded for now)
//        - default config if config file not there ??

const char UserConfiguration::default_key[][20] = {

  /*
  SET_MOVE_DOWN = 0,
  SET_MOVE_RIGHT,
  SET_MOVE_UP,
  SET_MOVE_LEFT,
  */


  "down",
  "right",
  "up",
  "left",

/*    SET_PLAYER_0,
    SET_PLAYER_1,
    SET_PLAYER_2,
    SET_PLAYER_3,
    SET_PLAYER_ONLY,
*/

  "1",
  "2",
  "3",
  "4",
  "0",

/*    SHOW_INVENTORY, 
    SHOW_OPTIONS_MENU,
    SET_NEXT_FORMATION,
*/

  "i",
  "o",
  "f",

/*
    TOGGLE_MINIMAP,
*/

  "[+]",

/*
    SET_ZOOM_IN,
    SET_ZOOM_OUT,
*/

  "8",
  "9",

/*
    TOGGLE_MAP_CENTER, 
    INCREASE_GAME_SPEED, 
    DECREASE_GAME_SPEED, 
*/

  "m",
  "k",
  "l",

//    START_ROUND, 

  "space",

  /*
    LAYOUT_1,
    LAYOUT_2,
    LAYOUT_4,
*/

  "g",
  "h",
  "j",

/*
    SWITCH_COMBAT,
*/

  "c",

/*
  NEXT_WEAPON,
*/
  "]",

/*
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
    QUICK_SPELL_12
*/

  "f1",
  "f2",
  "f3",
  "f4",
  "f5",
  "f6",
  "f7",
  "f8",
  "f9",
  "f10",
  "f11",
  "f12"


};

// Must be exact copy of enums defined in userconfiguration.h
// (except for ENGINE_ACTION_COUNT)
const char * UserConfiguration::ENGINE_ACTION_NAMES[]={

  "SET_MOVE_DOWN",
  "SET_MOVE_RIGHT",
  "SET_MOVE_UP",
  "SET_MOVE_LEFT",

  "SET_PLAYER_0",
  "SET_PLAYER_1",
  "SET_PLAYER_2",
  "SET_PLAYER_3",
  "SET_PLAYER_ONLY",

  "SHOW_INVENTORY", 
  "SHOW_OPTIONS_MENU",
  "SET_NEXT_FORMATION",

  "TOGGLE_MINIMAP",

  "SET_ZOOM_IN",
  "SET_ZOOM_OUT",

  "TOGGLE_MAP_CENTER",
  "INCREASE_GAME_SPEED", 
  "DECREASE_GAME_SPEED", 

  "START_ROUND",

  "LAYOUT_1",
  "LAYOUT_2",
  "LAYOUT_4",

  "SWITCH_COMBAT",

  "NEXT_WEAPON",

  "QUICK_SPELL_1",
  "QUICK_SPELL_2",
  "QUICK_SPELL_3",
  "QUICK_SPELL_4",
  "QUICK_SPELL_5",
  "QUICK_SPELL_6",
  "QUICK_SPELL_7",
  "QUICK_SPELL_8",
  "QUICK_SPELL_9",
  "QUICK_SPELL_10",
  "QUICK_SPELL_11",
  "QUICK_SPELL_12"
};


// Must be exact copy of enums defined in userconfiguration.h 
// (without the "_STOP" and except for ENGINE_ACTION_UP_COUNT)
const char * UserConfiguration :: ENGINE_ACTION_UP_NAMES[]={

  "SET_MOVE_DOWN",
  "SET_MOVE_RIGHT",
  "SET_MOVE_UP",
  "SET_MOVE_LEFT",
  "SET_Y_ROT_PLUS",
  "SET_Y_ROT_MINUS",
  "SET_Z_ROT_PLUS",
  "SET_Z_ROT_MINUS",
  "SET_ZOOM_IN",
  "SET_ZOOM_OUT",
  "SET_NEXT_FORMATION"
  
};



const char * UserConfiguration::ENGINE_ACTION_DESCRIPTION[]={

  _( "Scroll map south" ),
  _( "Scroll map north" ),
  _( "Scroll map east" ),
  _( "Scroll map west" ),

  _( "Select player 0" ),
  _( "Select player 1" ),
  _( "Select player 2" ),
  _( "Select player 3" ),
  _( "Toggle group mode" ),

  _( "Show inventory" ),
  _( "Show options" ),
  _( "Choose next formation" ),

  _( "Toggle minimap" ),

  _( "Zoom in map" ),
  _( "Zoom out map" ),

  _( "Always center map" ),
  _( "Increase game speed" ), 
  _( "Decrease game speed" ),

  _( "Start next round" ),

  _( "Floating UI layout" ),
  _( "Bottom UI layout" ),
  _( "Inventory UI layout" ),

  _( "Switch combat mode" ),

  _( "Switch to next weapon" ),

  _( "Quick Spell 1" ),
  _( "Quick Spell 2" ),
  _( "Quick Spell 3" ),
  _( "Quick Spell 4" ),
  _( "Quick Spell 5" ),
  _( "Quick Spell 6" ),
  _( "Quick Spell 7" ),
  _( "Quick Spell 8" ),
  _( "Quick Spell 9" ),
  _( "Quick Spell 10" ),
  _( "Quick Spell 11" ),
  _( "Quick Spell 12" )

};


UserConfiguration::UserConfiguration(){
    unsigned int i, j;
    string temp;

    standAloneMode = NONE;
		debugTheme = false;

		stencilBufInitialized = false;
    configurationChanged = false;

    // default settings for video mode (are overridden by command line)
    fullscreen = true;
    doublebuf = true;
    hwpal = true;
    resizeable = true;
    force_hwsurf = false;
    force_swsurf = false;
    hwaccel = true;
    test = false; 
    multitexturing = true; 
    stencilbuf = true;     
    bpp = -1;
    w = 800;
    h = 600;
    shadows = 0;
    alwaysShowPath = true;
    tooltipEnabled = true;
		enableScreenshots = true;
    tooltipInterval = 50;

    // game settings
    gamespeed = 1;  // fast speed
    centermap = true;
    keepMapSize = true;
    frameOnFullScreen = true;
    turnBasedBattle = true;
    ovalCutoutShown = true;
    outlineInteractiveItems = true;
    combatInfoDetail = 0;
    monsterToughness = 0;
    hideInventoriesOnMove = true;
    logLevel = 3;


    // audio settings
    soundEnabled = true;
    soundFreq = 5;
    musicVolume = 64;
    effectsVolume = 128;

    // Build (string engineAction -> int engineAction ) lookup table
    // and   (int ea -> string ea) lookup table    
    for (i = 0; i < ENGINE_ACTION_COUNT ; i++){
        temp = ENGINE_ACTION_NAMES[i];
        for(j = 0; j < temp.length(); j++){
            temp[j] = tolower(temp[j]);
        } 
        engineActionNumber[temp] = i;
        engineActionName[i] = temp;
    }

    // Build (string engineActionUp -> int engineActionUp ) lookup table
    for (i = SET_MOVE_DOWN_STOP; i < ENGINE_ACTION_UP_COUNT ; i++){
        temp = ENGINE_ACTION_UP_NAMES[i - SET_MOVE_DOWN_STOP];
        for(j = 0; j < temp.length(); j++){
            temp[j] = tolower(temp[j]);
        } 
        engineActionUpNumber[temp] = i;
    }

    if(DEBUG_USER_CONFIG){
        map<string, int>::iterator p;

        p = engineActionUpNumber.begin();
        cout << "Engine Action Up list : " << endl;
        while(p != engineActionUpNumber.end()){
            cout << " '" << p->first << "' associated to  '" << p->second << "'" << endl;
            p++;
        }

        cout << endl << endl;
        p = engineActionNumber.begin();
        cout << "Engine Action list : " << endl;
        while(p != engineActionNumber.end()){
            cout << " '" << p->first << "' associated to  '" << p->second << "'" << endl;
            p++;
        }
    }
}

void UserConfiguration::loadConfiguration(){
  ifstream *configFile;  
  string sLine;
  string sInstruction, sFirstParam, sSecondParam;    
  char textLine[255];
  int pos, firstChar, endWord, foo;
  int lineNumber;
  unsigned int i;
  bool isCurrentVersion = false;

  //    strcpy(path, rootDir);
  //    strcat(path, CONFIG_FILE_NAME);
  string path = get_config_file_name();
  configFile = new ifstream(path.c_str());
  if (!configFile->is_open()) {
    cerr << "Can't open configuration file: " << path << endl;
    cerr << "Will create a default config file at the above location." << endl; 
    createDefaultConfigFile();

    // try to open it again
    delete configFile;
    configFile = new ifstream(path.c_str());
    if (!configFile->is_open()) {
      cout << "Error: Can't open configuration file: " << path << endl;   
      exit(1);
    }
  }

  // loop through the whole configuration file
  lineNumber = 0;
  while (!configFile->eof()) {
    configFile -> getline(textLine, 255);
    sLine = textLine;  
	if ( !sInstruction.empty() ) sInstruction.clear();
    if ( !sFirstParam.empty() ) sFirstParam.clear();
    if ( !sSecondParam.empty() ) sSecondParam.clear();

    for (i = 0; i < sLine.length(); i++) {
      sLine[i] = tolower(sLine[i]);
    }

    // search for keywords, ignore lines not begining with keyword or spaces
    endWord = -1;
    foo = -1;
    firstChar = sLine.find_first_not_of(' ', 0);
    pos = sLine.find("bind", 0);
    if (pos < 0) {
      pos = sLine.find("set", 0);
      if ( pos < 0 ) {
        pos = sLine.find("version", 0);
        if ( pos >= 0 && pos <= firstChar ) {
          sInstruction = "version";
          sFirstParam = getNextWord(sLine, pos + 7, endWord);
          // hack
          foo = 2;
        }
      } else if ( pos <= firstChar ) {
        sInstruction = "set";
        sFirstParam = getNextWord(sLine, pos + 3, endWord);
        sSecondParam = getNextWord(sLine, endWord, foo);
      }
    } else if (pos <= firstChar) {
      sInstruction = "bind";
      sFirstParam = getNextWord(sLine, pos + 4, endWord);
      sSecondParam = getNextWord(sLine, endWord, foo);
    }

    if (sFirstParam.empty() || (sInstruction != "version" && sSecondParam.empty()) || foo == endWord) {
      if ( pos >= 0 && sInstruction.length()!=0) {
        cerr  << "Warning : in file " << path 
          << " missing parameter at line " << lineNumber 
          << ", ignoring line." << endl;
      }
    } else {
      if (sInstruction == "bind") {
        bind(sFirstParam, sSecondParam, lineNumber);
      } else if( sInstruction == "version" ) {
        isCurrentVersion = ( sFirstParam == SCOURGE_VERSION );
      } else {
        set(sFirstParam, sSecondParam, lineNumber);
      }
    } 

    lineNumber++;
  }
  configurationChanged = true;

  delete configFile;

  // set default bindings for unbound actions
  if( !isCurrentVersion ) {
    keyForEngineAction.clear();
    keyDownBindings.clear();
    keyUpBindings.clear();
  }
  for( int i = 0; i < ENGINE_ACTION_COUNT; i++ ) {
    // if no bindings
    if( keyForEngineAction.find( i ) == keyForEngineAction.end() ) {
      string keyName = default_key[ i ];
      bind( keyName, engineActionName[ i ], 1 );
    }
  }

  // merge old and new settings
  if( !isCurrentVersion ) {
    saveConfiguration();
  }
}

void UserConfiguration::saveConfiguration(){
    ofstream *configFile;  
    string sLine;    
    int i;

	string path = get_config_file_name();
    configFile = new ofstream(path.c_str());
    if(!configFile->is_open()){
        cout << "Error while saving " << path << endl;
        return;
    }
	enum { TXT_SIZE = 255 };
    char textLine[ TXT_SIZE ];
    snprintf(textLine, TXT_SIZE, "Generated by Scourge version %s\n", SCOURGE_VERSION);
    writeFile(configFile, textLine);
    writeFile(configFile, "Modify at your own risks.\n");
    writeFile(configFile, "-------------------------------------------------\n"); 
    writeFile(configFile, "- A line not beginning with BIND or SET is ignored (spaces excepted)\n");
    writeFile(configFile, "- Only one instruction per line will be processed\n");
    writeFile(configFile, "- No upper/lower case distinction\n");
    writeFile(configFile, "- A space is a parameter separator so replace spaces by '_' if needed in your parameters.\n"); 
    writeFile(configFile, "    Example : for 'left bracket' put 'left_bracket'\n");
    writeFile(configFile, "- No specific order needed between BIND/SET commands\n\n");
    writeFile(configFile, "Syntax : \n");
    writeFile(configFile, "- BIND  sdl_key_name  engineAction\n");
    writeFile(configFile, "- SET   variable 	 value\n");
    writeFile(configFile, "without the '-' at the beginning\n");
    writeFile(configFile, "sdl_key_names are defined in SDL.h\n");
    writeFile(configFile, "engineActions and variables are defined in userconfiguration.h\n");    
    writeFile(configFile, "-------------------------------------------------\n\n"); 

    // print the version
    snprintf(textLine, TXT_SIZE, "version %s\n", SCOURGE_VERSION);
    writeFile(configFile, textLine);

    writeFile(configFile, "// Bindings\n");

    // save bindings
    for (i = 0; i < ENGINE_ACTION_COUNT ; i++){
        if(keyForEngineAction.find(i)!=keyForEngineAction.end()){
            sLine = "bind " + keyForEngineAction[i];
            if(engineActionName.find(i)!=engineActionName.end()){
                sLine = sLine + " " + engineActionName[i] + "\n";
                writeFile(configFile, (char *) sLine.c_str());
            }
        }
    }

    // save video variables
    writeFile(configFile, "\n// Video settings\n");
		snprintf(textLine, TXT_SIZE, "set fullscreen %s\n", fullscreen ? "true":"false");
    writeFile(configFile, textLine);
		snprintf(textLine, TXT_SIZE, "set doublebuf %s\n", doublebuf ? "true":"false");
    writeFile(configFile, textLine);
		snprintf(textLine, TXT_SIZE, "set stencilbuf %s\n", stencilbuf ? "true":"false");
    writeFile(configFile, textLine);
		snprintf(textLine, TXT_SIZE, "set hwpal %s\n", hwpal ? "true":"false");
    writeFile(configFile, textLine);
		snprintf(textLine, TXT_SIZE, "set resizeable %s\n", resizeable ? "true":"false");
    writeFile(configFile, textLine);
		snprintf(textLine, TXT_SIZE, "set force_hwsurf %s\n", force_hwsurf ? "true":"false");
    writeFile(configFile, textLine);
		snprintf(textLine, TXT_SIZE, "set force_swsurf %s\n", force_swsurf ? "true":"false");
    writeFile(configFile, textLine);
		snprintf(textLine, TXT_SIZE, "set multitexturing %s\n", Constants::multitexture ? "true":"false");
    writeFile(configFile, textLine);
		snprintf(textLine, TXT_SIZE, "set hwaccel %s\n", hwaccel ? "true":"false");
    writeFile(configFile, textLine);
		snprintf(textLine, TXT_SIZE, "set shadows %d  // 0 : no shadows, 2 : best shadows\n", shadows);
    writeFile(configFile, textLine);
		snprintf(textLine, TXT_SIZE, "set w %d\n", w);
    writeFile(configFile, textLine);
		snprintf(textLine, TXT_SIZE, "set h %d\n", h);
    writeFile(configFile, textLine);
		snprintf(textLine, TXT_SIZE, "set bpp %d\n", bpp);
    writeFile(configFile, textLine);
		snprintf(textLine, TXT_SIZE, "\n// Game settings\n");
    writeFile(configFile, textLine);
		snprintf(textLine, TXT_SIZE, "set gamespeed %d  // 0 : fastest, 4 : slowest\n", gamespeed);
    writeFile(configFile, textLine);
		snprintf(textLine, TXT_SIZE, "set monstertoughness %d  // 0 : easy, 2 : toughest\n", monsterToughness);
    writeFile(configFile, textLine);
		snprintf(textLine, TXT_SIZE, "set centermap %s\n", centermap ? "true":"false");
    writeFile(configFile, textLine);
		snprintf(textLine, TXT_SIZE, "set keepmapsize %s\n", keepMapSize ? "true":"false");
    writeFile(configFile, textLine);
		snprintf(textLine, TXT_SIZE, "set frameonfullscreen %s\n", frameOnFullScreen ? "true":"false");
    writeFile(configFile, textLine);
		snprintf(textLine, TXT_SIZE, "set ovalcutoutshown %s\n", ovalCutoutShown ? "true":"false");
    writeFile(configFile, textLine);
		snprintf(textLine, TXT_SIZE, "set outlineinteractiveitems %s\n", outlineInteractiveItems ? "true":"false");
    writeFile(configFile, textLine);
		snprintf(textLine, TXT_SIZE, "set hideinventoriesonmove %s\n", hideInventoriesOnMove ? "true":"false");
    writeFile(configFile, textLine);
		snprintf(textLine, TXT_SIZE, "set combatinfodetail %d\n", combatInfoDetail );
    writeFile(configFile, textLine);
		snprintf(textLine, TXT_SIZE, "set turnbasedbattle %s\n", turnBasedBattle ? "true":"false");
    writeFile(configFile, textLine);
		snprintf(textLine, TXT_SIZE, "set alwaysshowpath %s\n", alwaysShowPath ? "true":"false");
    writeFile(configFile, textLine);
		snprintf(textLine, TXT_SIZE, "set tooltipenabled %s\n", tooltipEnabled ? "true":"false");
    writeFile(configFile, textLine);
		snprintf(textLine, TXT_SIZE, "set enablescreenshots %s\n", enableScreenshots ? "true":"false");
    writeFile(configFile, textLine);
		snprintf(textLine, TXT_SIZE, "set tooltipinterval %d\n", tooltipInterval );
    writeFile(configFile, textLine);
		snprintf(textLine, TXT_SIZE, "set loglevel %d  // 0 : minimal, 3 : full\n", logLevel);
    writeFile(configFile, textLine);
		snprintf(textLine, TXT_SIZE, "\n// Audio settings\n");
    writeFile(configFile, textLine);
		snprintf(textLine, TXT_SIZE, "set soundenabled %s\n", soundEnabled ? "true" : "false");
    writeFile(configFile, textLine);
		snprintf(textLine, TXT_SIZE, "set soundfreq %d\n", soundFreq);
    writeFile(configFile, textLine);
		snprintf(textLine, TXT_SIZE, "set musicvolume %d\n", musicVolume);
    writeFile(configFile, textLine);
		snprintf(textLine, TXT_SIZE, "set effectsvolume %d\n", effectsVolume);
    writeFile(configFile, textLine);

    delete configFile;
}

void UserConfiguration::setKeyForEngineAction(string keyName, int ea){

    string oldKeyName;
    string eas; 

    if(keyForEngineAction.find(ea)!=keyForEngineAction.end()){
        oldKeyName = keyForEngineAction[ea];
        if(keyDownBindings.find(oldKeyName)!=keyDownBindings.end()){
            keyDownBindings.erase(oldKeyName);
            if(keyUpBindings.find(oldKeyName)!=keyUpBindings.end()){
                keyUpBindings.erase(oldKeyName);
            }
        }
        keyForEngineAction[ea] = keyName;
        keyDownBindings[keyName] = ea;

        if(engineActionName.find(ea) != engineActionName.end()){
            eas = engineActionName[ea];
            if(engineActionUpNumber.find(eas) != engineActionUpNumber.end()){
                keyUpBindings[eas] = engineActionUpNumber[eas];
            }
        }
    } 
}
 
//    Bind   sdl_name_of_key    engineAction
// OR Bind   sdl_mouse_button   engineAction
void UserConfiguration::bind(string s1, string s2, int lineNumber){
    //int i;

    if(DEBUG_USER_CONFIG){
        cout << "line : " << lineNumber << " ";
        cout << "bind '" << s1 << "' '" << s2 << "'" << endl;
    }

    // for now, we trust what is written in configuration file
    if(engineActionNumber.find(s2) != engineActionNumber.end()){
        // Ignore debug ea 
      keyDownBindings[s1] = engineActionNumber[s2];
      keyForEngineAction[engineActionNumber[s2]] = s1;
      if(engineActionUpNumber.find(s2) != engineActionUpNumber.end()){
        keyUpBindings[s1] = engineActionUpNumber[s2];
      }
    }
}

void UserConfiguration::set(string s1, string s2, int lineNumber){
    bool paramValue = false;

    if(DEBUG_USER_CONFIG){
        cout << "line : " << lineNumber << " ";
        cout << "set '" << s1 << "' '" << s2 << "'" << endl; 
    }

    // Check if s1 is a valid variable to set (engineVariable?)
    // Check if s2 is a valid value

    if(s1 == "fullscreen"||s1 == "doublebuf" || s1 == "hwpal" || s1 == "resizeable" ||
       s1 == "force_hwsurf" || s1 == "force_swsurf" || s1 == "hwaccel" || 
       s1 == "multitexturing" || s1 == "stencilbuf" || s1 == "centermap" ||
       s1 == "keepmapsize" || s1 == "frameonfullscreen" || s1 == "turnbasedbattle" ||
       s1 == "ovalcutoutshown" || s1 == "outlineinteractiveitems" ||
       s1 == "hideinventoriesonmove" ||
       s1 == "soundenabled" || s1 == "alwaysshowpath" || s1 == "tooltipenabled" || 
			 s1 == "enablescreenshots" ) {
        if(s2 == "true") {
            paramValue = true;
        } else if(s2 == "false") {
            paramValue = false;
        } else {
		  cerr << "Warning : in file " << CONFIG_FILE // _NAME 
             << " invalid parameter at line " << lineNumber 
             << ", valid parameter are 'true' or 'false'. Ignoring line" << endl; 
             return;
        }
    }

    if(s1 == "fullscreen") {
        fullscreen = paramValue;
    } else if(s1 == "doublebuf") {
        doublebuf = paramValue;
    } else if(s1 == "hwpal") {
        hwpal = paramValue;
    } else if(s1 == "resizeable") {
        resizeable = paramValue;
    } else if(s1 == "force_hwsurf") {
        force_hwsurf = paramValue;
    } else if(s1 == "force_swsurf") {
        force_swsurf = paramValue;
    } else if(s1 == "hwaccel") {
        hwaccel = paramValue;
    } else if(s1 == "test") {
        test = paramValue;
    } else if(s1 == "bpp") {
        bpp = atoi(s2.c_str());
        if(!(bpp ==8 || bpp == 15 || bpp == 16 || bpp == 24 || bpp == 32)) {
		  cerr << "Warning : in file " << CONFIG_FILE //_NAME 
             << " invalid bpp value at line " << lineNumber 
             << ", valid values are 8, 15, 16, 24 or 32 . Ignoring line" << endl;    
             bpp = -1; // To autodetect best bpp value
        }
    } else if(s1 == "soundfreq") {
      soundFreq = atoi(s2.c_str());
    } else if(s1 == "musicvolume") {
      musicVolume = atoi(s2.c_str());
    } else if(s1 == "effectsvolume") {
      effectsVolume = atoi(s2.c_str());
    } else if(s1 == "tooltipinterval") {
      tooltipInterval = atoi(s2.c_str());
    } else if(s1 == "w") {
        w = atoi(s2.c_str());
    } else if(s1 == "h") {
        h = atoi(s2.c_str());
    } else if(s1 == "combatinfodetail") {
      combatInfoDetail = atoi(s2.c_str());
    } else if(s1 == "shadows") {
        shadows = atoi(s2.c_str());
        if(!(shadows == 0 || 
           shadows == 1 || 
           shadows == 2)) {
		  cerr << "Warning : in file " << CONFIG_FILE //_NAME 
             << " invalid shadow mode at line " << lineNumber 
             << ", valid modes 0, 1, 2 . Ignoring line" << endl;
             shadows = 2; // Default value
        }
    } else if(s1 == "stencilbuf") {
        stencilbuf = paramValue;
    } else if(s1 == "multitexturing") {
        Constants::multitexture = paramValue;
    } else if(s1 == "centermap") {
        centermap = paramValue;
    } else if(s1 == "keepmapsize") {
      keepMapSize = paramValue;
    } else if(s1 == "frameonfullscreen") {
      frameOnFullScreen = paramValue;
    } else if(s1 == "turnbasedbattle") {
      turnBasedBattle = paramValue;
    } else if(s1 == "ovalcutoutshown") {
      ovalCutoutShown = paramValue;
    } else if( s1 == "outlineinteractiveitems" ) {
      outlineInteractiveItems = paramValue;
    } else if(s1 == "hideinventoriesonmove" ) {
      hideInventoriesOnMove = paramValue;
    } else if(s1 == "soundenabled") {
      soundEnabled = paramValue;
    } else if(s1 == "alwaysshowpath" ) {
      alwaysShowPath = paramValue;
    } else if(s1 == "tooltipenabled" ) {
      tooltipEnabled = paramValue;
    } else if(s1 == "loglevel") {
        logLevel = atoi(s2.c_str());
        if(logLevel < 0 || logLevel > 3) {
		  cerr << "Warning : in file " << CONFIG_FILE //_NAME 
             << " invalid loglevel at line " << lineNumber 
             << ", valid values are 0, 1, 2 and 3 . Ignoring line" << endl;
             logLevel = 3; // Default value
        }
    } else if( s1 == "enablescreenshots" ) {
			enableScreenshots = paramValue;
    } else if(s1 == "monstertoughness" ) {
      monsterToughness = atoi(s2.c_str());
      if( monsterToughness < 0 ) monsterToughness = 0;
      if( monsterToughness > 2 ) monsterToughness = 2;
    } else if(s1 == "gamespeed") {
        gamespeed = atoi(s2.c_str());
        if(gamespeed < 0 || gamespeed > 4) {
		  cerr << "Warning : in file " << CONFIG_FILE //_NAME 
             << " invalid gamespeed level at line " << lineNumber 
             << ", valid values are 0, 1, 2, 3 and 4 . Ignoring line" << endl;
             gamespeed = 1; // Default value
        }
    }
}


int UserConfiguration::getGameSpeedTicks(){
		return gamespeed * 50;
}


void UserConfiguration::parseCommandLine(int argc, char *argv[]){
  bool printusage;

  printusage = false;


  // interpret command line args
  for (int i = 1; i < argc; i++) {
    if (strstr(argv[i], "--bpp") == argv[i]) {
      bpp = atoi(argv[i] + 5);
      if (!(bpp ==8 || bpp == 15 || bpp == 16 || bpp == 24 || bpp == 32)) {
        printf("Error: bad bpp=%d\n", bpp);
        printusage = true;
      }
    } else if (strstr(argv[i], "--width") == argv[i]) {
      w = atoi(argv[i] + 7);
      if (!w) {
        printf("Error: bad width=%s\n", argv[i] + 7);
        printusage = true;
      }
    } else if (strstr(argv[i], "--height") == argv[i]) {
      h = atoi(argv[i] + 8);
      if (!h) {
        printf("Error: bad height=%s\n", argv[i] + 8);
        printusage = true;
      }
    } else if (strstr(argv[i], "--shadow") == argv[i]) {
      shadows = atoi(argv[i] + 8);    
      if (!(shadows == 0 || shadows == 1 || shadows == 2)) {
        printf("Error: bad shadow mode: %d\n", shadows);
        printusage = true;
      }
		} else if (strstr(argv[i], "--theme") == argv[i]) {
			strcpy( GuiTheme::DEFAULT_THEME, argv[i] + 7 );
			cerr << "Using gui theme: " << GuiTheme::DEFAULT_THEME << endl;
    } else if (!strcmp(argv[i], "--version")) {
      printf("Scourge, version %s\n", SCOURGE_VERSION);
      exit(0);
#ifdef HAVE_SDL_NET
    } else if (!strncmp(argv[i], "--server", 8)) {
      standAloneMode = SERVER;
      port = atoi(argv[i] + 8);
    } else if (!strncmp(argv[i], "--client", 8)) {
      char *p = strdup(argv[i] + 8);
      host = strdup(strtok(p, ":"));
      port = atoi(strtok(NULL, ","));
      userName = strdup(strtok(NULL, ","));
      standAloneMode = CLIENT;
      //free(host);
      //free(userName);
      free(p);
#endif
    } else if (!strcmp(argv[i], "--test")) {
      test = true;
    } else if (!strcmp(argv[i], "--nosound")) {
      soundEnabled = false;
    } else if (argv[i][0] == '-' && argv[i][1] != '-') {
      for (int t = 1; t < static_cast<int>(strlen(argv[i])); t++) {
        switch (argv[i][t]) {
        case 'h':
        case '?': printusage = true; break;
        case 'f': fullscreen = false; break;
        case 'd': doublebuf = false; break;
        case 'p': hwpal = false; break;
        case 'r': resizeable = false; break;
        case 'H': force_hwsurf = true; break;
        case 'S': force_swsurf = true; break;
        case 'a': hwaccel = false; break;
        case 's': stencilbuf = false; break;
        case 'm': Constants::multitexture = false; break;
        }
      }
    } else if( !strcmp(argv[i], "--run-tests") ) {
      // this is ok
      standAloneMode = TEST;
		} else if( !strcmp( argv[ i ], "--debugtheme" ) ) {
			debugTheme = true;
    } else {
      printusage = true;
    }
  }

  if (printusage) {
    printf( _( "S.C.O.U.R.G.E.: Heroes of Lesser Renown\n" ) );
    printf( _( "A 3D, roguelike game of not quite epic proportions.\n\n" ) );
    printf( _( "Usage:\n" ) );
    printf( _( "scourge [-fdprHSa?hsm] [--test] [--bppXX] [--help] [--version] [--shadowX]\n" ) );
    printf( _( "version: %s\n" ), SCOURGE_VERSION);
#ifdef HAVE_SDL_NET
    printf( _( "[Multiplayer support]\n" ) );
#endif
#ifdef HAVE_SDL_MIXER
    printf( _( "[Sound support]\n" ) );
#endif
    printf( _( "\nOptions:\n" ) );
    printf( _( "\tf - disable fullscreen mode\n" ) );
    printf( _( "\td - disable double buffering\n" ) );
    printf( _( "\tp - disable hardware palette\n" ) );
    printf( _( "\tr - disable resizable window\n" ) );
    printf( _( "\tH - force hardware surface\n" ) );
    printf( _( "\tS - force software surface\n" ) );
    printf( _( "\ta - disable hardware acceleration\n" ) );
    printf( _( "\th,?,--help - show this info\n" ) );
    printf( _( "\ts - disable stencil buffer\n" ) );
    printf( _( "\tm - disable multitexturing\n" ) );
    printf( _( "\t--test - list card's supported video modes\n" ) );
    printf( _( "\t--test-config - print out the file configuration and exit\n" ) );
    printf( _( "\t--run-tests [path] - run internal tests of the rpg engine and save the result in html in the $path dir\n" ) );
    printf( _( "\t--version - print the build version\n" ) );
    printf( _( "\t--bppXX - use XX bits per pixel (8,15,16,24,32)\n" ) );
    printf( _( "\t--widthXX - use XX pixels for the screen width\n" ) );
    printf( _( "\t--heightXX - use XX pixels for the screen height\n" ) );
    printf( _( "\t--shadowX - shadow's cast by: 0-nothing, 1-objects and creatures, 2-everything\n" ) );
		printf( _( "\t--themeXYZ - use gui theme XYZ to draw the ui.\n" ) );
		printf( _( "\t--debugtheme - use the debug theme only to for wall textures.\n" ) );
    printf( _( "\nBy default (with no options):\n\tbpp is the highest possible value\n\tfullscreen mode is on\n\tdouble buffering is on\n\thwpal is used if available\n\tresizeable is on (no effect in fullscreen mode)\n\thardware surface is used if available\n\thardware acceleration is used if available\n\tstencil buffer is used if available\n\tmultitexturing is used if available\n\tshadows are cast by everything.\n\n" ) );
#ifdef HAVE_SDL_NET
    printf( _( "Multiplayer options:\n" ) );
    printf( _( "\t--serverPORT - run a standalone server w/o a ui on PORT\n" ) );
    printf( _( "\t--clientHOST:PORT,USERNAME - run a standalone admin client w/o a ui. Connect to server HOST:PORT as USERNAME.\n" ) );
#endif
#ifdef HAVE_SDL_MIXER
    printf( _( "Sound options:\n" ) );
    printf( _( "\t--nosound - run without sound\n" ) );
#endif
    exit(0);
  }
} 


void UserConfiguration::writeFile(ofstream *fileOut, char *text){
    string s;
    s = text;
    fileOut->write(s.c_str(), s.length());
}

// returns the action to do for this event
int UserConfiguration::getEngineAction(SDL_Event *event){
    string s;
    int res;

	if( !s.empty() ) s.clear();
    res = -1;
    if(event->type == SDL_KEYDOWN){
        s = SDL_GetKeyName(event->key.keysym.sym);
        replaceSpaces(s);
        if(keyDownBindings.find(s) != keyDownBindings.end()){
            res = keyDownBindings[s];
				}
    }
    else if(event->type == SDL_KEYUP){
        s = SDL_GetKeyName(event->key.keysym.sym);
        replaceSpaces(s);
        if(keyUpBindings.find(s) != keyUpBindings.end()){ 
            res = keyUpBindings[s]; 
				}
    }
    else if(event->type == SDL_MOUSEBUTTONDOWN){
        if(mouseDownBindings.find(event->button.button) != mouseDownBindings.end()){
            res = mouseDownBindings[event->button.button];
        }
    }
    else if(event->type == SDL_MOUSEBUTTONUP){
        if(mouseUpBindings.find(event->button.button) != mouseUpBindings.end()){
            res = mouseUpBindings[event->button.button];
        }
    }

    if(DEBUG_USER_CONFIG){
        cout << "engine action returned : " << res << endl;
    }
    return res;


 /* case SDL_MOUSEBUTTONUP:
    if(event->button.button) {
        int region = gui->testActiveRegions(event->button.x, event->button.y);
        if(region == Constants::SHOW_INVENTORY) {
            inventory->show();
        } else if(region == Constants::SHOW_OPTIONS) {
            // do something
        } else if(region == Constants::ESCAPE) {
            return true;
		} else if(region >= Constants::DIAMOND_FORMATION && region <= Constants::CROSS_FORMATION) {
		  setFormation(region - Constants::DIAMOND_FORMATION);
		} else if(region >= Constants::PLAYER_1 && region <= Constants::PLAYER_4) {
		  setPlayer(region - Constants::PLAYER_1);
		} else if(region == Constants::PLAYER_ONLY) {
		  player_only = (player_only ? false : true);
        } else {        
            processGameMouseClick(event->button.x, event->button.y, event->button.button);
        }
    }
    break;*/   

}


const char * UserConfiguration::getEngineActionDescription(int i){
    if(! (i < 0 || i > ENGINE_ACTION_COUNT)){
        return _(ENGINE_ACTION_DESCRIPTION[i]);
    }
    else{
        return "";
    }
}


const char * UserConfiguration::getEngineActionKeyName(int i){ 
    if(keyForEngineAction.find(i) != keyForEngineAction.end()){
        return (keyForEngineAction[i].c_str());
    }
    else{
        return "";
    }
}

// Returns next word from the given position. If there is not a space at the given
// position, the function suppose it is the first letter of the word wanted. 
string UserConfiguration::getNextWord(const string theInput, int fromPos, int &endWord){
    int firstChar, lastStringChar;
    string sub;
	if( !sub.empty() ) sub.clear();

    if (theInput.empty() || fromPos==-1) {return sub;}

    lastStringChar = theInput.find_last_not_of(' ');
    if(theInput[fromPos] == ' '){
        firstChar = theInput.find_first_not_of(' ', fromPos);
    }
    else{
        firstChar = fromPos;
    }
    endWord = theInput.find_first_of(' ', firstChar);
   // cout << "line :" << theInput << endl;
   // cout << "\t\tpos = " << fromPos << " firstChar = " << firstChar << " endWord = " << endWord << " lastStringChar =" << lastStringChar << endl; 
    if(endWord == -1){
        if( (lastStringChar >= firstChar)&&(firstChar!=-1)){
            sub = theInput.substr(firstChar, lastStringChar - firstChar + 1);
        }
    } 
    else{
        sub = theInput.substr(firstChar, endWord - firstChar);
    }
    return sub;

}

void UserConfiguration::replaceSpaces(string& s) {
	replace(s.begin(), s.end(), ' ', '_');
}

UserConfiguration::~UserConfiguration(){

}

void UserConfiguration::createConfigDir() {
#ifndef WIN32     
  // first create the directory
  // no need on windows, this file is saved in the current dir
  string path = get_config_dir_name();
  FILE *fp = fopen( path.c_str(), "r" );
  if( !fp ) {
    int err = mkdir( path.c_str(), S_IRWXU|S_IRGRP|S_IXGRP );
    if(err) {
      cerr << "Error creating config directory: " << path << endl;
      cerr << "Error: " << err << endl;
      perror("UserConfiguration::createDefaultConfigFile: ");
      exit(1);
    }
  } else {
    fclose( fp );
  }
#endif
}

void UserConfiguration::createDefaultConfigFile() {
  createConfigDir();

  // now create the file
  string path = get_config_file_name();
  ofstream configFile (path.c_str());
  if(!configFile.is_open()){
	cerr << "Can't open configuration file: " << path << endl;
	exit(1);
  }

  configFile << "Generated by Scourge version " << SCOURGE_VERSION << endl;
  configFile << "Modify at your own risks." << endl;
  configFile << "-------------------------------------------------" << endl;
  configFile << "- A line not beginning with BIND or SET is ignored (spaces excepted)" << endl;
  configFile << "- Only one instruction per line will be processed" << endl;
  configFile << "- No upper/lower case distinction" << endl;
  configFile << "- A space is a parameter separator so replace spaces by '_' if needed in your parameters." << endl;
  configFile << "    Example : for 'left bracket' put 'left_bracket'" << endl;
  configFile << "- No specific order needed between BIND/SET commands" << endl;
  configFile << "" << endl;
  configFile << "Syntax : " << endl;
  configFile << "- BIND  sdl_key_name  engineAction" << endl;
  configFile << "- SET   variable 	 value" << endl;
  configFile << "without the '-' at the beginning" << endl;
  configFile << "sdl_key_names are defined in SDL.h" << endl;
  configFile << "engineActions and variables are defined in userconfiguration.h" << endl;
  configFile << "-------------------------------------------------" << endl;
  configFile << "" << endl;
  configFile << "version " << SCOURGE_VERSION << endl;
  configFile << "" << endl;
  configFile << "// Bindings" << endl;
  configFile << "bind down set_move_down" << endl;
  configFile << "bind right set_move_right" << endl;
  configFile << "bind up set_move_up" << endl;
  configFile << "bind left set_move_left" << endl;
  configFile << "bind 1 set_player_0" << endl;
  configFile << "bind 2 set_player_1" << endl;
  configFile << "bind 3 set_player_2" << endl;
  configFile << "bind 4 set_player_3" << endl;
  configFile << "bind 0 set_player_only" << endl;
  configFile << "bind i show_inventory" << endl;
  configFile << "bind o show_options_menu" << endl;
  configFile << "bind u use_item" << endl;
  configFile << "bind f set_next_formation" << endl;
  configFile << "bind [+] toggle_minimap" << endl;
  configFile << "bind 8 set_zoom_in" << endl;
  configFile << "bind 9 set_zoom_out" << endl;
  configFile << "bind m toggle_map_center" << endl;
  configFile << "bind k increase_game_speed" << endl;
  configFile << "bind l decrease_game_speed" << endl;
  configFile << "bind space start_round" << endl;
  configFile << "bind g layout_1" << endl;
  configFile << "bind h layout_2" << endl;
  configFile << "bind j layout_4" << endl;
  configFile << "bind c switch_combat" << endl;
  configFile << "bind ] next_weapon" << endl;
  configFile << "" << endl;
  configFile << "// Video settings" << endl;
  configFile << "set fullscreen true" << endl;
  configFile << "set doublebuf true" << endl;
  configFile << "set stencilbuf true" << endl;
  configFile << "set hwpal true" << endl;
  configFile << "set resizeable false" << endl;
  configFile << "set force_hwsurf true" << endl;
  configFile << "set force_swsurf false" << endl;
  configFile << "set multitexturing true" << endl;
  configFile << "set hwaccel true" << endl;
  configFile << "set shadows 2  // 0 : no shadows, 2 : best shadows" << endl;
  configFile << "set w 1024" << endl;
  configFile << "set h 768" << endl;
  configFile << "set bpp 32" << endl;
	configFile << "set enablescreenshots true" << endl;
  configFile << "" << endl;
  configFile << "// Game settings" << endl;
  configFile << "set gamespeed 2  // 0 : fastest, 4 : slowest" << endl;
  configFile << "set monstertoughness 0  // 0 : easy, 2 : toughest" << endl;
  configFile << "set centermap false" << endl;
  configFile << "set keepmapsize true" << endl;
  configFile << "set frameonfullscreen true" << endl;
  configFile << "set turnbasedbattle true" << endl;
  configFile << "set ovalcutoutshown true" << endl;
  configFile << "set outlineinteractiveitems true" << endl;
  configFile << "set hideinventoriesonmove true" << endl;
  configFile << "set combatinfodetail 0" << endl;
  configFile << "set alwaysshowpath false" << endl;
  configFile << "set tooltipenabled true" << endl;
  configFile << "set tooltipinterval 50" << endl;
  configFile << "set loglevel 3" << endl;
  configFile << "" << endl;
  configFile << "// Audio settings" << endl;
  configFile << "set soundenabled true" << endl;
  configFile << "set musicvolume 64 // 0-silent, 128-loudest" << endl;
  configFile << "set effectsvolume 128 // 0-silent, 128-loudest" << endl;
  configFile << "set soundfreq 5  // 0: most frequent, 10: least frequent" << endl;
  configFile << "" << endl;

  configFile.close();
}

