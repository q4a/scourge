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
 
 
// FIXME : does not seem to exit if the configuration file is not found
// (although there is the code to do so..!!)-> due to ifstream implementation ?
// TODO : - warn if there is an unknown parameter in the file ?


const char * UserConfiguration::ENGINE_ACTION_NAMES[]={

    "SET_MOVE_STOP",
    "SET_MOVE_DOWN",
    "SET_MOVE_RIGHT",
    "SET_MOVE_UP",
    "SET_MOVE_LEFT",
            
    "SET_PLAYER_0",
    "SET_PLAYER_1",
    "SET_PLAYER_2",
    "SET_PLAYER_3",
    "SET_PLAYER_ONLY",
    "BLEND_A",
    "BLEND_B",    
    
    "SHOW_INVENTORY", 
    "SHOW_OPTIONS_MENU",
    "USE_ITEM",
    "SET_NEXT_FORMATION",
    
    "SET_X_ROT_PLUS",   
    "SET_X_ROT_MINUS",    
    "SET_Y_ROT_PLUS",
    "SET_Y_ROT_MINUS",    
    "SET_Z_ROT_PLUS",        
    "SET_Z_ROT_MINUS",
     
    "ADD_X_POS_PLUS",
    "ADD_X_POS_MINUS",
    "ADD_Y_POS_PLUS",
    "ADD_Y_POS_MINUS",
    "ADD_Z_POS_PLUS",
    "ADD_Z_POS_MINUS",
    
    "MINIMAP_ZOOM_IN",
    "MINIMAP_ZOOM_OUT",
    "MINIMAP_TOGGLE",
    
    "SET_ZOOM_IN",     
    "SET_ZOOM_OUT"               
};


// All engine actions that have a corresponding keyup action 
// (which will be formed as follows : engineActionName_stop)
// Do not forget to update ENGINE_ACTION_UP_COUNT if the
// variable below is modified
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
    "SET_ZOOM_OUT"            
};


UserConfiguration::UserConfiguration(){
    int i, j;
    string temp;
    
    // engine variables
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
    
    // Build (string engineAction <-> int engineAction ) lookup table
    /*for (i = 0; i < ENGINE_ACTION_COUNT ; i++){
        engineActionNames.insert(
            pair<string, int>(ENGINE_ACTION_NAMES[i], i)
        );
    }*/
    
    // Build (string engineActionUp <-> int engineActionUp ) lookup table
    for (i = 0; i < ENGINE_ACTION_UP_COUNT ; i++){
        temp = ENGINE_ACTION_UP_NAMES[i];
        for(j = 0; j < temp.length(); j++){
            temp[j] = tolower(temp[j]);                                 
        } 
        engineActionUpNames.insert(pair<string, int>(temp, i));
    }
    if(DEBUG_USER_CONFIG){
        map<string, int>::iterator p;
       /* p = engineActionNames.begin();
        cout << "Engine Action list : " << endl;
        while(p != engineActionNames.end()){
            cout << " '" << p->first << "' associated to  '" << p->second << "'" << endl;
            p++;     
        }*/
                
        p = engineActionUpNames.begin();
        cout << "Engine Action Up list : " << endl;
        while(p != engineActionUpNames.end()){
            cout << " '" << p->first << "' associated to  '" << p->second << "'" << endl;
            p++;     
        }
    }

}

void UserConfiguration::loadConfiguration(){
    string sLine;
    string sInstruction, sFirstParam, sSecondParam;    
    char textLine[255];
    int pos, firstChar, endWord, foo;
    int lineNumber;
    int i;
    
    configFile = new ifstream(CONFIG_FILE_NAME);
    if(!configFile){
        cout << "Error while opening" << CONFIG_FILE_NAME << endl;
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
        //cout << "line : " << lineNumber << " '" << sLine << "'" << endl;
        
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

    
    delete configFile;    

}
 
//    Bind   sdl_name_of_key    engineAction
// OR Bind   sdl_mouse_button   engineAction
void UserConfiguration::bind(string s1, string s2, int lineNumber){        
    int i;    
    SDLKey keyCode;
    cout << "line : " << lineNumber << " ";          
    cout << "bind '" << s1 << "' '" << s2 << "'" << endl; 
            
    // for now, we trust what is written in configuration file
    // so it should be generated later    
    keyDownBindings.insert(pair<string, string>(s1, s2));
    cout << "keyDownBindings[" << s1 << "]=" << keyDownBindings[s1]<<endl;

    if(engineActionUpNames.find(s2) != engineActionUpNames.end()){        
        s2.insert(s2.length(), "_stop");
        cout<< "s2 == " << s2 << endl;                                   
        keyUpBindings.insert(pair<string, string>(s1, s2));
    }            
    
    /*map<string, int>::iterator c;
    c = engineActionNames.find("SET_X_ROT_PLUS");
    if(c == engineActionNames.end())
        cout <<"qsdhsqdfhdsf";
    else
        cout <<"YES!!!!!!";*/                   
}  
  
void UserConfiguration::set(string s1, string s2, int lineNumber){
    bool paramValue;
    
    cout << "line : " << lineNumber << " ";              
    cout << "set '" << s1 << "' '" << s2 << "'" << endl; 
    
    // Check if s1 is a valid variable to set (engineVariable?)
    // Check if s2 is a valid value
    
    if((s1!="bpp")&&(s1!="h")&&(s1!="w")){
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
    // TODO : int values like in sdlhandler :: setvideomode
    else if(s1 == "bpp"){
        //bpp = 
    }
    else if(s1 == "w"){
        //w = int(s2.c_str());
    }
    else if(s1 == "h"){
        //hwpal = paramValue;
    }
    
    
    
}

// returns the action to do for this event
string UserConfiguration::getEngineAction(SDL_Event *event){    
    string s;    
    int i, j;
    
            
    s.clear();
    if(event->type == SDL_KEYDOWN){                
        s = SDL_GetKeyName(event->key.keysym.sym);        
        s = replaceSpaces(s);         
        if(keyDownBindings.find(s) != keyDownBindings.end()){             
            return keyDownBindings[s];               
        }
    }
    else if(event->type == SDL_KEYUP){
        s = SDL_GetKeyName(event->key.keysym.sym);
        s = replaceSpaces(s);        
        if(keyUpBindings.find(s) != keyDownBindings.end()){ 
            return keyUpBindings[s]; 
        }
    }
    else if(event->type == SDL_MOUSEBUTTONDOWN){
        if(mouseDownBindings.find(event->button.button) != mouseDownBindings.end()){
            return mouseDownBindings[event->button.button];            
        }        
    }
    else if(event->type == SDL_MOUSEBUTTONUP){
        if(mouseUpBindings.find(event->button.button) != mouseUpBindings.end()){
            return mouseUpBindings[event->button.button];            
        }        
    }
    
    // Should never reach this code
    return s;
    
    
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



/*
SDLKey getKeyCode(string s1){

    if(s1 == "SDLK_ESCAPE")	{return SDLK_ESCAPE;}
    else if(s1 == "SDLK_1")	{return SDLK_1;}
    else if(s1 == "SDLK_2")	{return SDLK_2;}
	else if(s1 == "SDLK_3")	{return SDLK_3;}
	else if(s1 == "SDLK_4")	{return SDLK_4;}
	else if(s1 == "SDLK_5")	{return SDLK_5;}
	else if(s1 == "SDLK_6")	{return SDLK_6;}
	else if(s1 == "SDLK_7")	{return SDLK_7;}
	else if(s1 == "SDLK_8")	{return SDLK_8;}
	else if(s1 == "SDLK_9")	{return SDLK_9;}
	else if(s1 == "SDLK_0")	{return SDLK_0;}
	
    else if(s1 == "SDLK_COLON")	{return SDLK_COLON;}
    else if(s1 == "SDLK_SEMICOLON")	{return SDLK_SEMICOLON;}
	else if(s1 == "SDLK_EQUALS")	{return SDLK_EQUALS;}
	else if(s1 == "SDLK_LESS")	{return SDLK_LESS;}
	else if(s1 == "SDLK_GREATER")	{return SDLK_GREATER;}
	else if(s1 == "SDLK_QUESTION")	{return SDLK_QUESTION;}
	else if(s1 == "SDLK_LEFTBRACKET")	{return SDLK_LEFTBRACKET;}
	else if(s1 == "SDLK_RIGHTBRACKET")	{return SDLK_RIGHTBRACKET;}
	else if(s1 == "SDLK_BACKSLASH")	{return SDLK_BACKSLASH;}
	else if(s1 == "SDLK_CARET")	{return SDLK_CARET;}
	else if(s1 == "SDLK_UNDERSCORE")	{return SDLK_UNDERSCORE;}
	else if(s1 == "SDLK_BACKQUOTE")	{return SDLK_BACKQUOTE;}

	
	else if(s1 == "SDLKP_1")	{return SDLKP_1;}
    else if(s1 == "SDLKP_2")	{return SDLKP_2;}
	else if(s1 == "SDLKP_3")	{return SDLKP_3;}
	else if(s1 == "SDLKP_4")	{return SDLKP_4;}
	else if(s1 == "SDLKP_5")	{return SDLKP_5;}
	else if(s1 == "SDLKP_6")	{return SDLKP_6;}
	else if(s1 == "SDLKP_7")	{return SDLKP_7;}
	else if(s1 == "SDLKP_8")	{return SDLKP_8;}
	else if(s1 == "SDLKP_9")	{return SDLKP_9;}
	else if(s1 == "SDLKP_0")	{return SDLKP_0;}	
	else if(s1 == "SDLK_KP_PERIOD")	{return SDLK_KP_PERIOD;}
    else if(s1 == "SDLK_KP_DIVIDE")	{return SDLK_KP_DIVIDE;}
	else if(s1 == "SDLK_KP_MULTIPLY")	{return SDLK_KP_MULTIPLY;}
	else if(s1 == "SDLK_KP_MINUS")	{return SDLK_KP_MINUS;}
	else if(s1 == "SDLK_KP_PLUS")	{return SDLK_KP_PLUS;}
	else if(s1 == "SDLK_KP_ENTER")	{return SDLK_KP_ENTER;}
	else if(s1 == "SDLK_KP_EQUALS")	{return SDLK_KP_EQUALS;}
	else if(s1 == "SDLK_UP")	{return SDLK_UP;}
	else if(s1 == "SDLK_DOWN")	{return SDLK_DOWN;}
	else if(s1 == "SDLK_RIGHT")	{return SDLK_RIGHT;}
	else if(s1 == "SDLK_LEFT")	{return SDLK_LEFT;}
	
	else if(s1 == "SDLK_INSERT")	{return SDLK_INSERT;}
	else if(s1 == "SDLK_HOME")	{return SDLK_HOME;}
	else if(s1 == "SDLK_END")	{return SDLK_END;}
	else if(s1 == "SDLK_PAGEUP")	{return SDLK_PAGEUP;}
	else if(s1 == "SDLK_PAGEDOWN")	{return SDLK_PAGEDOWN;}
	else if(s1 == "SDLK_NUMLOCK")	{return SDLK_NUMLOCK;}		
	else if(s1 == "SDLK_CAPSLOCK")	{return SDLK_CAPSLOCK;}		
	else if(s1 == "SDLK_SCROLLOCK")	{return SDLK_SCROLLOCK;}		
	else if(s1 == "SDLK_LSHIFT")	{return SDLK_LSHIFT;}		
	else if(s1 == "SDLK_LCTRL")	{return SDLK_LCTRL;}		
	else if(s1 == "SDLK_LALT")	{return SDLK_LALT;}
    else if(s1 == "SDLK_RSHIFT")	{return SDLK_RSHIFT;}		
	else if(s1 == "SDLK_RCTRL")	{return SDLK_RCTRL;}		
	else if(s1 == "SDLK_RALT")	{return SDLK_RALT;}	
	
	else if(s1 == "SDLK_BACKSPACE")	{return SDLK_BACKSPACE;}
	else if(s1 == "SDLK_TAB")	{return SDLK_TAB;}
	else if(s1 == "SDLK_RETURN"){return SDLK_RETURN;}
	else if(s1 == "SDLK_SPACE")	{return SDLK_SPACE;}
	
	else if(s1 == "SDLK_a")	{return SDLK_a;}
    else if(s1 == "SDLK_b")	{return SDLK_b;}
    else if(s1 == "SDLK_c")	{return SDLK_c;}
	else if(s1 == "SDLK_d")	{return SDLK_d;}
	else if(s1 == "SDLK_e")	{return SDLK_e;}
	else if(s1 == "SDLK_f")	{return SDLK_f;}
	else if(s1 == "SDLK_g")	{return SDLK_g;}
	else if(s1 == "SDLK_h")	{return SDLK_h;}
	else if(s1 == "SDLK_i")	{return SDLK_i;}
	else if(s1 == "SDLK_j")	{return SDLK_j;}
	else if(s1 == "SDLK_k")	{return SDLK_k;}
	else if(s1 == "SDLK_l")	{return SDLK_l;}	
	else if(s1 == "SDLK_m")	{return SDLK_m;}
	else if(s1 == "SDLK_n")	{return SDLK_n;}	
	else if(s1 == "SDLK_o")	{return SDLK_o;}
	else if(s1 == "SDLK_p")	{return SDLK_p;}
	else if(s1 == "SDLK_q")	{return SDLK_q;}
	else if(s1 == "SDLK_r")	{return SDLK_r;}
	else if(s1 == "SDLK_s")	{return SDLK_s;}
	else if(s1 == "SDLK_t")	{return SDLK_t;}	
	else if(s1 == "SDLK_u")	{return SDLK_u;}
	else if(s1 == "SDLK_v")	{return SDLK_v;}
	else if(s1 == "SDLK_w")	{return SDLK_w;}
	else if(s1 == "SDLK_x")	{return SDLK_x;}
	else if(s1 == "SDLK_y")	{return SDLK_y;}
	else if(s1 == "SDLK_z")	{return SDLK_z;}	    
 	
	else if(s1 == "SDLK_F1")	{return SDLK_F1;}		
	else if(s1 == "SDLK_F2")	{return SDLK_F2;}		
	else if(s1 == "SDLK_F3")	{return SDLK_F3;}		
	else if(s1 == "SDLK_F4")	{return SDLK_F4;}		
	else if(s1 == "SDLK_F5")	{return SDLK_F5;}		
	else if(s1 == "SDLK_F6")	{return SDLK_F6;}		
	else if(s1 == "SDLK_F7")	{return SDLK_F7;}		
	else if(s1 == "SDLK_F8")	{return SDLK_F8;}		
	else if(s1 == "SDLK_F9")	{return SDLK_F9;}		
	else if(s1 == "SDLK_F10")	{return SDLK_F10;}		
    
}*/
  

