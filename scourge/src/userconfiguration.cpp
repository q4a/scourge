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
 

// TODO : - warn if there is an unknown parameter in the config file ?
//        - manage doubled keynames -> must be impossible in optionsmenu
//        - make an array for variables too (they are all hard coded for now)
//        - default config if config file not there ??


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
    "USE_ITEM",
    "SET_NEXT_FORMATION",
         
    "SET_Y_ROT_PLUS",
    "SET_Y_ROT_MINUS",    
    "SET_Z_ROT_PLUS",        
    "SET_Z_ROT_MINUS",         
    
    "MINIMAP_ZOOM_IN",
    "MINIMAP_ZOOM_OUT",
    "MINIMAP_TOGGLE",
    
    "SET_ZOOM_IN",     
    "SET_ZOOM_OUT",
    
    "TOGGLE_MAP_CENTER",    
    "INCREASE_GAME_SPEED", 
    "DECREASE_GAME_SPEED", 
    
    "START_ROUND",
    
    "BLEND_A",
    "BLEND_B",
    "SET_X_ROT_PLUS",   
    "SET_X_ROT_MINUS",
    "ADD_X_POS_PLUS",
    "ADD_X_POS_MINUS",
    "ADD_Y_POS_PLUS",
    "ADD_Y_POS_MINUS",
    "ADD_Z_POS_PLUS",
    "ADD_Z_POS_MINUS"     
    
};


// Must be exact copy of enums defined in userconfiguration.h 
// (without the "_STOP" and except for ENGINE_ACTION_UP_COUNT)
const char * UserConfiguration :: ENGINE_ACTION_UP_NAMES[]={

    "SET_MOVE_DOWN",     
    "SET_MOVE_RIGHT",
    "SET_MOVE_UP",
    "SET_MOVE_LEFT",
    "SET_X_ROT_PLUS",   
    "SET_X_ROT_MINUS",    
    "SET_Y_ROT_PLUS",
    "SET_Y_ROT_MINUS",    
    "SET_Z_ROT_PLUS",        
    "SET_Z_ROT_MINUS",
    "SET_ZOOM_IN",     
    "SET_ZOOM_OUT",
    "USE_ITEM", 
    "SET_NEXT_FORMATION"
    
};



const char * UserConfiguration::ENGINE_ACTION_DESCRIPTION[]={
    
    "Move player south",
    "Move player north",
    "Move player east",
    "Move player west",
            
    "Select player 0",
    "Select player 1",
    "Select player 2",
    "Select player 3",
    "Move only selected player",    
    
    "Show inventory", 
    "Show options",
    "Use item",
    "Choose next formation",
            
    "Rotate map up",
    "Rotate map down",    
    "Rotate map right",        
    "Rotate map left",       
    
    "Zoom in minimap",
    "Zoom out minimap",
    "Show/hide minimap",
    
    "Zoom in map",     
    "Zoom out map",
    
    "Always center map",
    "Increase game speed", 
    "Decrease game speed",
    
    "Start next round",
    
    // Not visible to the user
    "BLEND_A",    
    "BLEND_B",  
    "SET_X_ROT_PLUS",   
    "SET_X_ROT_MINUS",
    "ADD_X_POS_PLUS",       
    "ADD_X_POS_MINUS",
    "ADD_Y_POS_PLUS",
    "ADD_Y_POS_MINUS",
    "ADD_Z_POS_PLUS",
    "ADD_Z_POS_MINUS"
};


UserConfiguration::UserConfiguration(){    
    int i, j;
    string temp;
    
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
    bpp = -1;
    w = 800;
    h = 600;
    shadows = 0;               
    
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
        
        cout << "debug ea (ie invisible): " << endl;
        for (i = ENGINE_ACTION_DEBUG_IND; i < ENGINE_ACTION_COUNT ; i++){                
            if (engineActionName.find(i) != engineActionName.end()){
                cout << engineActionName[i] << endl; 
            }                
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
    int i;
        
    configFile = new ifstream(CONFIG_FILE_NAME);
    if(!configFile->is_open()){
        cout << "Error while opening " << CONFIG_FILE_NAME << endl;        
        exit(1);
    }    
    
    // loop through the whole configuration file
    lineNumber = 0;
    while(!configFile->eof()){
        configFile -> getline(textLine, 255);
        sLine = textLine;  
        sInstruction.clear();      
        sFirstParam.clear();
        sSecondParam.clear();
                      
        for(i = 0; i < sLine.length(); i++){
            sLine[i] = tolower(sLine[i]);                                 
        }         
        
        // search for keywords, ignore lines not begining with keyword or spaces
        endWord = -1;
        foo = -1;
        firstChar = sLine.find_first_not_of(' ', 0);
        pos = sLine.find("bind", 0);       
        if(pos < 0){            
            pos = sLine.find("set", 0);
            if ((pos >= 0)&&(pos <= firstChar)){
                sInstruction = "set";                
                sFirstParam = getNextWord(sLine, pos + 3, endWord);
                sSecondParam = getNextWord(sLine, endWord, foo);                             
            }           
        }
        else if(pos <= firstChar){
            sInstruction = "bind";                                 
            sFirstParam = getNextWord(sLine, pos + 4, endWord);
            sSecondParam = getNextWord(sLine, endWord, foo);                        
        }
        
        if (sFirstParam.empty() || sSecondParam.empty() || foo == endWord){            
            if( pos >= 0 && sInstruction.length()!=0){                
                cerr  << "Warning : in file " << CONFIG_FILE_NAME 
                << " missing parameter at line " << lineNumber 
                << ", ignoring line." << endl;                
            }            
        }                    
        else{              
            if(sInstruction == "bind"){
                bind(sFirstParam, sSecondParam, lineNumber);
            }
            else{
                set(sFirstParam, sSecondParam, lineNumber);
            }
        } 
                
        lineNumber++;
    }
    configurationChanged = true;
    
    delete configFile;    

}

void UserConfiguration::saveConfiguration(char **controlLine){
    ofstream *configFile;  
    string sLine;    
    char textLine[255];    
    int i;
    
    configFile = new ofstream(CONFIG_FILE_NAME);
    if(!configFile->is_open()){
        cout << "Error while saving " << CONFIG_FILE_NAME << endl;        
        return;
    }    
    
    sprintf(textLine, "Generated by Scourge version %.2f\n", SCOURGE_VERSION);
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
    writeFile(configFile, "// Bindings\n");
    
    // save bindings
    for (i = 0; i < ENGINE_ACTION_DEBUG_IND ; i++){        
        if(keyForEngineAction.find(i)!=keyForEngineAction.end()){            
            sLine = "bind " + keyForEngineAction[i];
            if(engineActionName.find(i)!=engineActionName.end()){
                sLine = sLine + " " + engineActionName[i] + "\n";
                writeFile(configFile, (char *) sLine.c_str());
            }
        }            
    }       
    
    // save variables
    writeFile(configFile, "\n// Video settings\n");
    sprintf(textLine, "set fullscreen %s\n", fullscreen ? "true":"false");
    writeFile(configFile, textLine);
    sprintf(textLine, "set doublebuf %s\n", doublebuf ? "true":"false");
    writeFile(configFile, textLine);
    sprintf(textLine, "set hwpal %s\n", hwpal ? "true":"false");
    writeFile(configFile, textLine);
    sprintf(textLine, "set resizeable %s\n", resizeable ? "true":"false");
    writeFile(configFile, textLine);
    sprintf(textLine, "set force_hwsurf %s\n", force_hwsurf ? "true":"false");
    writeFile(configFile, textLine);
    sprintf(textLine, "set force_swsurf %s\n", force_swsurf ? "true":"false");
    writeFile(configFile, textLine);
    sprintf(textLine, "set hwaccel %s\n", hwaccel ? "true":"false");
    writeFile(configFile, textLine);
    sprintf(textLine, "set shadows %d  // 0 : no shadows, 2 : best shadows\n", shadows);
    writeFile(configFile, textLine);
    sprintf(textLine, "set w %d\n", w);
    writeFile(configFile, textLine);
    sprintf(textLine, "set h %d\n", h);
    writeFile(configFile, textLine);
    sprintf(textLine, "set bpp %d\n", bpp);
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
    int i;  
    
    if(DEBUG_USER_CONFIG){
        cout << "line : " << lineNumber << " ";        
        cout << "bind '" << s1 << "' '" << s2 << "'" << endl;   
    }
          
    // for now, we trust what is written in configuration file        
    if(engineActionNumber.find(s2) != engineActionNumber.end()){                   
        // Ignore debug ea 
        if(!isDebugEa(engineActionNumber[s2])){                        
            keyDownBindings[s1] = engineActionNumber[s2];
            keyForEngineAction[engineActionNumber[s2]] = s1;
            if(engineActionUpNumber.find(s2) != engineActionUpNumber.end()){                                      
                keyUpBindings[s1] = engineActionUpNumber[s2];
            }
        }
    }        
}  
  
void UserConfiguration::set(string s1, string s2, int lineNumber){
    bool paramValue;
    
    if(DEBUG_USER_CONFIG){
        cout << "line : " << lineNumber << " ";              
        cout << "set '" << s1 << "' '" << s2 << "'" << endl; 
    }
    
    // Check if s1 is a valid variable to set (engineVariable?)
    // Check if s2 is a valid value
    
    if(s1 == "fullscreen"||s1 == "doublebuf" || s1 == "hwpal" || s1 == "resizeable" ||
       s1 == "force_hwsurf" || s1 == "force_swsurf" || s1 == "hwaccel"){
        if(s2 == "true"){
            paramValue = true;
        }
        else if(s2 == "false"){
            paramValue = false;
        }
        else{          
            cerr << "Warning : in file " << CONFIG_FILE_NAME 
             << " invalid parameter at line " << lineNumber 
             << ", valid parameter are 'true' or 'false'. Ignoring line" << endl; 
             return;                  
        }        
    }
    
    if(s1 == "fullscreen"){
        fullscreen = paramValue;        
    }
    else if(s1 == "doublebuf"){
        doublebuf = paramValue;
    }
    else if(s1 == "hwpal"){
        hwpal = paramValue;
    }
    else if(s1 == "resizeable"){
        resizeable = paramValue;
    }
    else if(s1 == "force_hwsurf"){
        force_hwsurf = paramValue;
    }
    else if(s1 == "force_swsurf"){
        force_swsurf = paramValue;
    }
    else if(s1 == "hwaccel"){
        hwaccel = paramValue;
    }     
    else if(s1 == "test"){
        test = paramValue;
    }  
    
    else if(s1 == "bpp"){        
        bpp = atoi(s2.c_str());
        if(!(bpp ==8 || bpp == 15 || bpp == 16 || bpp == 24 || bpp == 32)) {
		 cerr << "Warning : in file " << CONFIG_FILE_NAME 
             << " invalid bpp value at line " << lineNumber 
             << ", valid values are 8, 15, 16, 24 or 32 . Ignoring line" << endl;    
             bpp = -1; // To autodetect best bpp value
        }                               
    }
    else if(s1 == "w"){
        w = atoi(s2.c_str());
    }
    else if(s1 == "h"){
        h = atoi(s2.c_str());
    }
    else if(s1 == "shadows"){        
        shadows = atoi(s2.c_str());
        if(!(shadows == 0 || 
           shadows == 1 || 
           shadows == 2)) {
            cerr << "Warning : in file " << CONFIG_FILE_NAME 
             << " invalid shadow mode at line " << lineNumber 
             << ", valid modes 0, 1, 2 . Ignoring line" << endl;    
             shadows = 2; // Default value
        }      
          
    }  
}

void UserConfiguration::parseCommandLine(int argc, char *argv[]){
  bool printusage;
  
  printusage = false;
  
  
  // interpret command line args
  for(int i = 1; i < argc; i++) {
	if(strstr(argv[i], "--bpp") == argv[i]) {	  
	  bpp = atoi(argv[i] + 5);
	  if(!(bpp ==8 || bpp == 15 || bpp == 16 || bpp == 24 || bpp == 32)) {
		printf("Error: bad bpp=%d\n", bpp);
		printusage = true;
	  }      
	} else if(strstr(argv[i], "--width") == argv[i]) {	  
	  w = atoi(argv[i] + 7);
	  if(!w) {
		printf("Error: bad width=%s\n", argv[i] + 7);
		printusage = true;
	  }     	  
	} else if(strstr(argv[i], "--height") == argv[i]) {	  
	  h = atoi(argv[i] + 8);
	  if(!h) {
		printf("Error: bad height=%s\n", argv[i] + 8);
		printusage = true;
	  }	   
	} else if(strstr(argv[i], "--shadow") == argv[i]) {	  
	  Constants::shadowMode = atoi(argv[i] + 8);	  
      if(!(Constants::shadowMode == 0 || 
           Constants::shadowMode == 1 || 
           Constants::shadowMode == 2)) {
          printf("Error: bad shadow mode: %d\n", Constants::shadowMode);
          printusage = true;
      }       
	} else if(!strcmp(argv[i], "--version")) {
	  printf("Scourge, version %.2f\n", SCOURGE_VERSION);
	  exit(0);
	} else if(!strcmp(argv[i], "--test")) {
	  test = true;
	} else if(argv[i][0] == '-' && argv[i][1] != '-') {
	  for(int t = 1; t < (int)strlen(argv[i]); t++) {
		switch(argv[i][t]) {
		case 'h': case '?': printusage = true; break;
		case 'f': fullscreen = false; break;
		case 'd': doublebuf = false; break;
		case 'p': hwpal = false; break;
		case 'r': resizeable = false; break;
		case 'H': force_hwsurf = true; break;
		case 'S': force_swsurf = true; break;
		case 'a': hwaccel = false; break;
		case 's': Constants::stencilbuffer = false; break;
		case 'm': Constants::multitexture = false; break;
		}
	  }
	} else {
	  printusage = true;
	}
  }

  if(printusage) {
	printf("S.C.O.U.R.G.E.: Heroes of Lesser Renown\n");
	printf("A 3D, roguelike game of not quite epic proportions.\n\n");
	printf("Usage:\n");
	printf("scourge [-fdprHSa?hsm] [--test] [--bppXX] [--help] [--version] [--shadowX]\n");
	printf("version: %.2f\n", SCOURGE_VERSION);
	printf("\nOptions:\n");
	printf("\tf - disable fullscreen mode\n");
	printf("\td - disable double buffering\n");
	printf("\tp - disable hardware palette\n");
	printf("\tr - disable resizable window\n");
	printf("\tH - force hardware surface\n");
	printf("\tS - force software surface\n");
	printf("\ta - disable hardware acceleration\n");
	printf("\th,?,--help - show this info\n");
	printf("\ts - disable stencil buffer\n");
	printf("\tm - disable multitexturing\n");
	printf("\t--test - list card's supported video modes\n");
	printf("\t--version - print the build version\n");
	printf("\t--bppXX - use XX bits per pixel (8,15,16,24,32)\n");
	printf("\t--widthXX - use XX pixels for the screen width\n");
	printf("\t--heightXX - use XX pixels for the screen height\n");
    printf("\t--shadowX - shadow's cast by: 0-nothing, 1-objects and creatures, 2-everything\n");
	printf("\nBy default (with no options):\n\tbpp is the highest possible value\n\tfullscreen mode is on\n\tdouble buffering is on\n\thwpal is used if available\n\tresizeable is on (no effect in fullscreen mode)\n\thardware surface is used if available\n\thardware acceleration is used if available\n\tstencil buffer is used if available\n\tmultitexturing is used if available\n\tshadows are cast by everything.\n\n");
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
    int i, j, res;         
    
    s.clear();
    res = -1;
    if(event->type == SDL_KEYDOWN){                
        s = SDL_GetKeyName(event->key.keysym.sym);        
        s = replaceSpaces(s);         
        if(keyDownBindings.find(s) != keyDownBindings.end()){             
            res = keyDownBindings[s];               
				}
    }
    else if(event->type == SDL_KEYUP){
        s = SDL_GetKeyName(event->key.keysym.sym);
        s = replaceSpaces(s);   
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
        return ENGINE_ACTION_DESCRIPTION[i];
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

bool UserConfiguration::isDebugEa(int j){
    if (j >= ENGINE_ACTION_DEBUG_IND && j <= ENGINE_ACTION_COUNT){        
            return true;        
    }
    else{
        return false;
    }
}

/*bool UserConfiguration::getConfigurationChanged(){ 
    bool temp;
    temp = configurationChanged; 
    configurationChanged = false; 
    return temp;
}*/
   

string UserConfiguration::getNextWord(const string theInput, int fromPos, int &endWord){
    int firstChar, lastStringChar;
    string sub;
    sub.clear();
    
    if (theInput.empty() || fromPos==-1) {return sub;}

    lastStringChar = theInput.find_last_not_of(' ');
    firstChar = theInput.find_first_not_of(' ', fromPos);
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

string UserConfiguration::replaceSpaces(string s){
    int i;
    for (i = 0; i < s.length(); i++){
        if(s[i] == ' '){
            s[i] = '_';
        }            
    }
    return s;
} 


UserConfiguration::~UserConfiguration(){

}
