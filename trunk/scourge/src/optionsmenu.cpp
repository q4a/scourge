/***************************************************************************
                          optionsmenu.cpp  -  description
                             -------------------
    begin                : Tue Aug 12 2003
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

#include "optionsmenu.h"

void OptionsMenu::createButton(int x1, int y1, int x2, int y2, char *label, bool toggle, Button * &theButton){
    theButton = new Button(x1, y1, x2, y2, strdup(label));
	theButton->setToggle(toggle);	
	mainWin->addWidget((Widget*)theButton);            	
}

OptionsMenu::OptionsMenu(Scourge *scourge){    
        
    this->scourge = scourge;
    this->uc = scourge->getUserConfiguration();
    controlsLoaded = false;
    controlLines = NULL;
    nbControlLines = 0;
    waitingForNewKey = false;
     
    mainWin = new Window( scourge->getSDLHandler(),
						100, 50, 525, 505, 
						strdup("Options"), 
						scourge->getShapePalette()->getGuiTexture() );
	
    createButton (105, 0, 210, 30, "Game settings", true, gameSettingsButton);  					
	createButton (210, 0, 315, 30, "Video", true, videoButton);
 	createButton (315, 0, 420, 30, "Audio", true, audioButton);
    createButton (420, 0, 525, 30, "Controls", true, controlsButton);
    //createButton (420, 0, 525, 30, "Close", false, closeButton);   
    createButton (350, 440, 455, 470, "Close", false, closeButton);   
            
    cards = new CardContainer(mainWin);
    
    Label *label = new Label(220, 50, strdup("Key bindings"));
	label->setColor( 0.8f, 0.2f, 0, 1 );
	cards->addWidget(label, CONTROLS);	    
    controlBindingsList = new ScrollingList(30, 100, 450, 300);
    cards->addWidget(controlBindingsList, CONTROLS);
    createButton (65, 440, 170, 470, "Change", false, changeControlButton);
    changeControlButton->setLabelPosition(Button::CENTER);
    cards->addWidget(changeControlButton, CONTROLS);    
    createButton (205, 440, 310, 470, "Save changes", false, saveControlButton);    		          
    saveControlButton->setVisible(false);
    cards->addWidget(saveControlButton, CONTROLS);		      
    
    waitingLabel = new Label(35, 80, strdup(" ")); 	
    waitingLabel->setColor( 0.0f, 0.3f, 0.9f, 1 );  
    cards->addWidget(waitingLabel, CONTROLS);          

    selectedMode = CONTROLS;
    				
}

void OptionsMenu::loadControls(){
    string line, s1, s2, s3;
    
    int i, j;
    char spaces[MAX_CONTROLS_LINE_SIZE];
    for(i = 0; i < MAX_CONTROLS_LINE_SIZE ; i++){
        spaces[i] = ' ';
    }
    spaces[i] = '\0';
    
    if(controlLines) {  
        for(int i = 0; i < uc->getEngineActionCount() ; i++){
            if( controlLines[i] ){
                free(controlLines[i]);
                controlLines[i] = NULL;
            }
        }
        free(controlLines);
        controlLines = NULL;
    }
    
    nbControlLines = uc -> getEngineActionCount();            
    controlLines = (char **) malloc (nbControlLines *sizeof(char *));    
    for (i = 0; i < nbControlLines; i++){
        //line = spaces;
        s1 = uc->getEngineActionDescription(i);
        s2 = uc->getEngineActionKeyName(i);              
        /*for(j=0 ; j < 50 - s1.length(); j++){
            s1.insert(s1.length(), " ");            
        } */                       
        
        line = s1 + "           " + s2;         
        /*line.replace(0, s1.length(), s1);
        line.replace (70 - s2.length(), s2.length(), s2);*/
        controlLines[i] = (char *)malloc(MAX_CONTROLS_LINE_SIZE * sizeof(char));        
        strcpy(controlLines[i], line.c_str());
    }            
    
    controlBindingsList->setLines(nbControlLines, (const char**) controlLines);       
                   
}

void OptionsMenu::setSelectedMode(){

    videoButton->setSelected(selectedMode == VIDEO);
    audioButton->setSelected(selectedMode == AUDIO);
    controlsButton->setSelected(selectedMode == CONTROLS);
    gameSettingsButton->setSelected(selectedMode == GAME_SETTINGS);        
    
    switch(selectedMode){
        case VIDEO : 
            break;
        case AUDIO : 
            break;
        case CONTROLS :
            if((!controlsLoaded)||(uc->getConfigurationChanged())){                
                loadControls();
                if(!controlsLoaded){
                    controlsLoaded = true;
                }
            }            
            break;
        case GAME_SETTINGS :
            break;
        default : 
            break;       
    }
    
    cards->setActiveCard(selectedMode);  
}


bool OptionsMenu::handleEvent(Widget *widget, SDL_Event *event) {    
    if(widget == closeButton) mainWin->setVisible(false);
    else if(widget == gameSettingsButton) {
        selectedMode = GAME_SETTINGS;        
    }  
    else if(widget == videoButton) {
        selectedMode = VIDEO;
    }   
    else if(widget == audioButton) {
        selectedMode = AUDIO;
    }
    else if(widget == controlsButton) {
        selectedMode = CONTROLS;        
    }
    else if(widget == changeControlButton) { // && selectedMode== Controls?       
        waitingLabel->setText(strdup("Waiting for new key ... Press ESCAPE to cancel"));        
        waitingForNewKey = true;        
    }
    else if(widget == saveControlButton){
        uc->saveConfiguration(controlLines);
    }
               
    setSelectedMode(); 
     
    return false;
}

bool OptionsMenu::handleEvent(SDL_Event *event) {
  switch(event->type) {
  case SDL_MOUSEBUTTONUP:
    break;
  case SDL_KEYDOWN:
    if(waitingForNewKey){
        if(event->key.keysym.sym != SDLK_ESCAPE){
            int ind;
            ind = controlBindingsList->getSelectedLine();                     
            string s1 = uc->getEngineActionDescription(ind);                      
            string s2 = SDL_GetKeyName(event->key.keysym.sym);  
            for (int i = 0; i < s2.length(); i++){
                if(s2[i] == ' '){
                    s2[i] = '_';
                }            
            }
            uc->setKeyForEngineAction(s2, ind); // update userConfig too
            s1 = s1 + "           " + s2;            
            strcpy(controlLines[ind], s1.c_str());
            controlBindingsList->setLines(nbControlLines, (const char**) controlLines);
            controlBindingsList->setSelectedLine(ind);                                                          
        }          
        waitingLabel->setText(strdup(" "));
        waitingForNewKey = false;
    }
    else{
        switch(event->key.keysym.sym) {
    	case SDLK_ESCAPE:
    	  hide();
        case SDLK_RETURN:
            if(selectedMode == CONTROLS){
                waitingLabel->setText(strdup("Waiting for new key ... Press ESCAPE to cancel"));        
                waitingForNewKey = true;  
            }
        /*case SDLK_DOWN:
            if(selectedMode == CONTROLS){
                int ind = controlBindingsList->getSelectedLine();
                ind ++;
                controlBindingsList->setSelectedLine(ind);                
            }*/
            
    	  return true;
        default: break;
        }  
    }
  default: break;  
  }  
  return false;
}

OptionsMenu::~OptionsMenu(){

    if(controlsButton) delete controlsButton;
    if(videoButton) delete videoButton;
    if(audioButton) delete audioButton;
    if(closeButton) delete closeButton;
    if(gameSettingsButton) delete gameSettingsButton;
    if(changeControlButton) delete changeControlButton;
    
}
