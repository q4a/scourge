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

OptionsMenu::OptionsMenu(Scourge *scourge){    
  this->scourge = scourge;
    this->uc = scourge->getUserConfiguration();
    controlsLoaded = false;
    videoLoaded = false;
    gameSettingsLoaded = false;
    controlLines = NULL;
    nbControlLines = 0;
    waitingForNewKey = false;
	ignoreKeyUp = false;
     
    mainWin = new Window( scourge->getSDLHandler(),
						100, 50, 525, 505, 
						strdup("Options"), 
						scourge->getShapePalette()->getGuiTexture() );
	 
    gameSettingsButton = mainWin->createButton(105, 0, 210, 30, strdup("Game settings"), true);
	videoButton = mainWin->createButton (210, 0, 315, 30, strdup("Video"), true);
 	audioButton = mainWin->createButton (315, 0, 420, 30, strdup("Audio"), true);
    controlsButton = mainWin->createButton (420, 0, 525, 30, strdup("Controls"), true);           
    saveButton = mainWin->createButton(205, 440, 310, 470, strdup("Save to file"), false);      
                      
    cards = new CardContainer(mainWin);
    
    // Controls tab
    keyBindingsLabel = cards->createLabel(220, 50, strdup("Key bindings"), CONTROLS, Constants::RED_COLOR);
    controlBindingsList = new ScrollingList(30, 100, 450, 300, scourge->getShapePalette()->getHighlightTexture());
    cards->addWidget(controlBindingsList, CONTROLS);        
    changeControlButton = cards->createButton (65, 440, 170, 470, strdup("Change"), CONTROLS, false);
    waitingLabel = cards->createLabel(35, 80, strdup(" "), CONTROLS, Constants::BLUE_COLOR);         

    // Game settings tab
    gameSpeedML = new MultipleLabel(100, 80, 300, 100, "Game speed", 100);
    gameSpeedML -> addText(strdup("Very slow"));
    gameSpeedML -> addText(strdup("Slow"));
    gameSpeedML -> addText(strdup("Normal"));
    gameSpeedML -> addText(strdup("Fast"));
    gameSpeedML -> addText(strdup("Fastest"));    
    cards->addWidget(gameSpeedML, GAME_SETTINGS);
    alwaysCenterMapCheckbox = cards->createCheckbox(100, 120, 258, 140, "Always center map", GAME_SETTINGS);
    keepMapSize = cards->createCheckbox(100, 160, 258, 180, "Keep zoom when switching layouts", GAME_SETTINGS);
    frameOnFullScreen = cards->createCheckbox(100, 200, 258, 220, "Frame map in fullscreen mode", GAME_SETTINGS);
    turnBasedBattle = cards->createCheckbox(100, 240, 258, 260, "Pause after battle turn", GAME_SETTINGS);
   
    // Video settings tabs        
    videoResolutionML = new MultipleLabel(100, 40, 300, 60, "Screen resolution", 100);
    int nbModes, i;
    char **modes;    
    modes = scourge->getSDLHandler()->getVideoModes(nbModes);    
    for(i = 0; i < nbModes; i++) {
      videoResolutionML -> addText(modes[i]);        
    }    
    cards->addWidget(videoResolutionML, VIDEO);    

    fullscreenCheckbox = cards->createCheckbox(100, 75, 258, 95, "Fullscreen", VIDEO);    
    resizeableCheckbox = cards->createCheckbox(100, 110, 258, 130, "Window resizeable", VIDEO);
    doublebufCheckbox  = cards->createCheckbox(100, 145, 258, 165, "Use double buffering", VIDEO);    
    stencilbufCheckbox = cards->createCheckbox(100, 180, 258, 200, "Use stencil buffer", VIDEO );    
    forceHwsurfCheckbox = cards->createCheckbox(100, 215, 258, 235, "Force hardware surfaces", VIDEO);
    forceSwsurfCheckbox = cards->createCheckbox(100, 250, 258, 270, "Force software surfaces", VIDEO);
    multitexturingCheckbox = cards->createCheckbox(100, 285, 258, 305, "Use multitexturing", VIDEO);    
    hwpalCheckbox   = cards->createCheckbox(100, 320, 258, 340, "Use hardware palette", VIDEO);
    hwaccelCheckbox = cards->createCheckbox(100, 360, 258, 380, "Use hardware acceleration", VIDEO); 
    shadowsML = new MultipleLabel(100, 395, 300, 415, "Shadows", 100);
    shadowsML -> addText("None");  
    shadowsML -> addText("Some");
    shadowsML -> addText("All");       
    cards->addWidget(shadowsML, VIDEO);       
    changeTakeEffectLabel = cards->createLabel(113, 432, strdup(" "), VIDEO, Constants::BLUE_COLOR);
    
    selectedMode = GAME_SETTINGS;
    				
}

void OptionsMenu::loadGameSettings(){
        
    gameSpeedML->setText(gameSpeedML->getNbText() - uc->getGameSpeedLevel() - 1);
    alwaysCenterMapCheckbox->setCheck(uc->getAlwaysCenterMap());
    keepMapSize->setCheck(uc->getKeepMapSize());
    frameOnFullScreen->setCheck(uc->getFrameOnFullScreen());
    turnBasedBattle->setCheck(uc->isBattleTurnBased());
}

// line i must correspond to engine action i if we want this scrolling list to work
void OptionsMenu::loadControls(){
    string line, s1, s2, s3; 
    int i;
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

void OptionsMenu::loadVideo(){    
    char temp[50];
    string line;
    string s, s1, s2, s3, s4;
    int i;
    int end;
    
    sprintf(temp, "%d x %d", uc -> getW(), uc-> getH());          
    s = temp;
    s1 = Util::getNextWord(s, 0, end);
    s2 = Util::getNextWord(s, end, end);  // ignores the ' x '  
    s2 = Util::getNextWord(s, end, end);       
    i = 0;  
    
    // don't know why the string::find() function does not work, so...
    while (i < videoResolutionML -> getNbText()){
        line = videoResolutionML -> getText(i);
        s3 = Util::getNextWord(line, 0, end);               
        if(s1 == s3){                           
            s4 = Util::getNextWord(line, end, end);   // ignores the ' x ' 
            s4 = Util::getNextWord(line, end, end);              
            if(s2 == s4){                
                break;            
            }            
        }
        i++;        
    }
    if(i < videoResolutionML -> getNbText()){
        videoResolutionML -> setText(i);
    }
    else{
        videoResolutionML -> setText(0);
    }    
    shadowsML->setText(uc->getShadows());
    
    // Checkboxes
    fullscreenCheckbox->setCheck(uc -> getFullscreen());
    doublebufCheckbox->setCheck(uc->getDoublebuf());
    stencilbufCheckbox->setCheck(uc->getStencilbuf());
    hwpalCheckbox->setCheck(uc->getHwpal());
    resizeableCheckbox->setCheck(uc->getResizeable());
    forceHwsurfCheckbox->setCheck(uc->getForce_hwsurf());
    forceSwsurfCheckbox->setCheck(uc->getForce_swsurf());
    multitexturingCheckbox->setCheck(uc->getMultitexturing());
    hwaccelCheckbox->setCheck(uc->getHwaccel());    
    
    
}

void OptionsMenu::setSelectedMode(){

    videoButton->setSelected(selectedMode == VIDEO);
    audioButton->setSelected(selectedMode == AUDIO);
    controlsButton->setSelected(selectedMode == CONTROLS);
    gameSettingsButton->setSelected(selectedMode == GAME_SETTINGS);        
    
    switch(selectedMode){
        case VIDEO :
            if(!videoLoaded){
                loadVideo();
                videoLoaded = true;                
            } 
            break;
        case AUDIO :
            break;
        case CONTROLS :
            if(!controlsLoaded){                
                loadControls();
                controlsLoaded = true;                
            }            
            break;
        case GAME_SETTINGS :
            if(!gameSettingsLoaded){
                loadGameSettings();
                gameSettingsLoaded = true;
            }
            break;
        default : 
            break;       
    }
    
    cards->setActiveCard(selectedMode);  
}


bool OptionsMenu::handleEvent(Widget *widget, SDL_Event *event) {    
    if(widget == mainWin->closeButton) mainWin->setVisible(false);
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
    else if(widget == gameSpeedML){
        uc -> setGameSpeedLevel(gameSpeedML->getNbText() - gameSpeedML->getCurrentTextInd() - 1);
    }
    else if(widget == alwaysCenterMapCheckbox){
        uc -> setAlwaysCenterMap(alwaysCenterMapCheckbox->isChecked());
    }
    else if(widget == keepMapSize){
        uc ->setKeepMapSize(keepMapSize->isChecked());
    }
    else if(widget == frameOnFullScreen){
        uc ->setFrameOnFullScreen(frameOnFullScreen->isChecked());
    }
    else if(widget == turnBasedBattle){
        uc ->setBattleTurnBased(turnBasedBattle->isChecked());
    }
    else if(widget == videoResolutionML){
        string line, s1, s2;
        int end;
        line = videoResolutionML->getCurrentText();
        s1 = Util::getNextWord(line, 0, end);
        s2 = Util::getNextWord(line, end, end);
        s2 = Util::getNextWord(line, end, end);
        uc-> setW(atoi(s1.c_str()));
        uc-> setH(atoi(s2.c_str()));        
    }
    else if(widget == fullscreenCheckbox){
        // if fullscreen checked -> not resizeable
        if(fullscreenCheckbox->isChecked()){
            resizeableCheckbox->setCheck(false);
            uc->setResizeable(false);                
        }
        uc->setFullscreen(fullscreenCheckbox->isChecked());        
    } 
    else if(widget == resizeableCheckbox){
        uc->setResizeable(resizeableCheckbox->isChecked());
    }
    else if(widget == doublebufCheckbox){
        uc->setDoublebuf(doublebufCheckbox->isChecked());
    }
    else if(widget == hwpalCheckbox){
        uc->setHwpal(hwpalCheckbox->isChecked());
    }
    else if(widget == forceSwsurfCheckbox){
        // Hardware or software surfaces but not both
        if(forceSwsurfCheckbox->isChecked()){
            forceHwsurfCheckbox->setCheck(false);
            uc->setForce_hwsurf(false); 
        }
        uc->setForce_swsurf(forceSwsurfCheckbox->isChecked());
    }
    else if(widget == forceHwsurfCheckbox){
        // Hardware or software surfaces but not both
        if(forceHwsurfCheckbox->isChecked()){
            forceSwsurfCheckbox->setCheck(false);
            uc->setForce_swsurf(false); 
        }
        uc->setForce_hwsurf(forceHwsurfCheckbox->isChecked());
    }
    else if(widget == hwaccelCheckbox){
        uc->setHwaccel(hwaccelCheckbox->isChecked());
    }
    else if(widget == stencilbufCheckbox){
        uc->setStencilbuf(stencilbufCheckbox->isChecked());
    }
    else if(widget == multitexturingCheckbox){
        uc->setMultitexturing(multitexturingCheckbox->isChecked());
    }
    else if(widget == shadowsML){
        uc->setShadows(shadowsML->getCurrentTextInd());    
    }
    else if(widget == saveButton){
        uc->saveConfiguration();
        if(selectedMode == VIDEO){
            changeTakeEffectLabel -> setText("Some changes will only take effect upon restart");            
        }       
    }    
    setSelectedMode(); 
      
    return false;
}

bool OptionsMenu::handleEvent(SDL_Event *event) {
  switch(event->type) {
  case SDL_MOUSEBUTTONUP:
    break;
  case SDL_KEYUP:
	switch(event->key.keysym.sym) {
	case SDLK_ESCAPE:
	  if(!ignoreKeyUp){
		hide();
		return true;
	  } else ignoreKeyUp = false;
	default:
	  break;
	}
	break;
  case SDL_KEYDOWN:    
    if(waitingForNewKey){
        if(event->key.keysym.sym != SDLK_ESCAPE){
            int ind;
            ind = controlBindingsList->getSelectedLine();                     
            string s1 = uc->getEngineActionDescription(ind);                      
            string s2 = SDL_GetKeyName(event->key.keysym.sym);  
            for (unsigned int i = 0; i < s2.length(); i++){
                if(s2[i] == ' '){
                    s2[i] = '_';
                }            
            }
            uc->setKeyForEngineAction(s2, ind); // update userConfig too
            s1 = s1 + "           " + s2;            
            strcpy(controlLines[ind], s1.c_str());
            controlBindingsList->setLines(nbControlLines, (const char**) controlLines);
            controlBindingsList->setSelectedLine(ind);                                                          
        } else ignoreKeyUp = true;
        waitingLabel->setText(strdup(" "));
        waitingForNewKey = false;
    }
    else{
        switch(event->key.keysym.sym) {
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
       
    // Only delete ML, scrollingLists and cardContainers. 
    // Other widgets are deleted by window.cpp       
    if(gameSpeedML) delete gameSpeedML;
    if(videoResolutionML) delete videoResolutionML;
    if(controlBindingsList) delete controlBindingsList;
    if(cards) delete cards;
    
}

void OptionsMenu::show() {
  mainWin->setVisible(true);
}  

