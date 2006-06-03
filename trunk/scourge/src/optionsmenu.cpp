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
#include "sound.h" 
#include "shapepalette.h"

using namespace std;

#define XPOS 10
#define SPACING 18
#define MINOR_SPACING 4
#define YPOS ( SPACING + MINOR_SPACING )
#define WIN_WIDTH 340
#define X_SIZE WIN_WIDTH - 30
#define BUTTON_WIDTH WIN_WIDTH / 4

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
                        100, 170, WIN_WIDTH, 320, 
                        "Options", 
                        scourge->getShapePalette()->getGuiTexture(),
                        true, Window::BASIC_WINDOW,
                        scourge->getShapePalette()->getGuiTexture2());
    
  int x = 0;
  gameSettingsButton = mainWin->createButton(x, 0, x + BUTTON_WIDTH, SPACING, "Gameplay", true);
  x += BUTTON_WIDTH;
  videoButton = mainWin->createButton (x, 0, x + BUTTON_WIDTH, SPACING, "Video", true);
  x += BUTTON_WIDTH;
  audioButton = mainWin->createButton (x, 0, x + BUTTON_WIDTH, SPACING, "Audio", true);
  x += BUTTON_WIDTH;
  controlsButton = mainWin->createButton (x, 0, x + BUTTON_WIDTH, SPACING, "Controls", true);           

  x = 10;
  saveButton = mainWin->createButton(x, 272, x + BUTTON_WIDTH, 272 + SPACING, "Save", false);      
  x += BUTTON_WIDTH + MINOR_SPACING;
  closeButton = mainWin->createButton(x, 272, x + BUTTON_WIDTH, 272 + SPACING, "Close", false);      
  
  cards = new CardContainer(mainWin);
  
  // Controls tab
  int y = YPOS;
  keyBindingsLabel = cards->createLabel(XPOS, y + 10, "Key bindings", CONTROLS, Constants::RED_COLOR);
  y += SPACING + MINOR_SPACING;
  controlBindingsList = new ScrollingList(XPOS, y, X_SIZE, 200, scourge->getShapePalette()->getHighlightTexture());
  cards->addWidget(controlBindingsList, CONTROLS);
  y += 200 + MINOR_SPACING;
  changeControlButton = cards->createButton(XPOS, y, XPOS + X_SIZE, y + SPACING, 
											Constants::getMessage( Constants::CHANGE_KEY ), 
											CONTROLS, false);
  y += SPACING + MINOR_SPACING;
  waitingLabel = cards->createLabel(35, 80, " ", CONTROLS, Constants::BLUE_COLOR);         

  // Game settings tab
  y = YPOS;
  gameSpeedML = new MultipleLabel(XPOS, y, XPOS + X_SIZE, y + SPACING, "Game speed", 100);
  gameSpeedML -> addText(strdup("Very slow"));
  gameSpeedML -> addText(strdup("Slow"));
  gameSpeedML -> addText(strdup("Normal"));
  gameSpeedML -> addText(strdup("Fast"));
  gameSpeedML -> addText(strdup("Fastest"));    
  cards->addWidget(gameSpeedML, GAME_SETTINGS);
  y += SPACING + MINOR_SPACING;
  alwaysCenterMapCheckbox = cards->createCheckbox(XPOS, y, XPOS + X_SIZE, y + SPACING, "Always center map", GAME_SETTINGS);
  y += SPACING + MINOR_SPACING;
  keepMapSize = cards->createCheckbox(XPOS, y, XPOS + X_SIZE, y + SPACING, "Keep zoom when switching layouts", GAME_SETTINGS);
  y += SPACING + MINOR_SPACING;
  frameOnFullScreen = cards->createCheckbox(XPOS, y, XPOS + X_SIZE, y + SPACING, "Frame map in fullscreen mode", GAME_SETTINGS);
  y += SPACING + MINOR_SPACING;
  turnBasedBattle = cards->createCheckbox(XPOS, y, XPOS + X_SIZE, y + SPACING, "Is battle turn-based?", GAME_SETTINGS);
  y += SPACING + MINOR_SPACING;
  ovalCutoutShown = cards->createCheckbox(XPOS, y, XPOS + X_SIZE, y + SPACING, "Shadow overlay?", GAME_SETTINGS);
  y += SPACING + MINOR_SPACING;
  outlineInteractiveItems = cards->createCheckbox(XPOS, y, XPOS + X_SIZE, y + SPACING, "Outline items?", GAME_SETTINGS);
  y += SPACING + MINOR_SPACING;
  //alwaysShowPath = cards->createCheckbox(XPOS, y, XPOS + X_SIZE, y + SPACING, "Show path in TB battle?", GAME_SETTINGS);
  //y += SPACING + MINOR_SPACING;
  tooltipEnabled = cards->createCheckbox(XPOS, y, XPOS + X_SIZE, y + SPACING, "Show tooltips", GAME_SETTINGS);
  y += SPACING + MINOR_SPACING;
  tooltipInterval = new Slider(XPOS, y, XPOS + X_SIZE, scourge->getShapePalette()->getHighlightTexture(), 0, 200, "Tooltip Delay:");
  cards->addWidget(tooltipInterval, GAME_SETTINGS);
   
  // Video settings tabs        
  y = YPOS;
  videoResolutionML = new MultipleLabel(XPOS, y, XPOS + X_SIZE, y + SPACING, "Screen resolution", 100);
  int nbModes, i;
  char **modes;    
  modes = scourge->getSDLHandler()->getVideoModes(nbModes);    
  for(i = 0; i < nbModes; i++) {
    videoResolutionML -> addText(modes[i]);        
  }    
  cards->addWidget(videoResolutionML, VIDEO);    
  
  y += SPACING + MINOR_SPACING;
  fullscreenCheckbox = cards->createCheckbox(XPOS, y, XPOS + X_SIZE, y + SPACING, "Fullscreen", VIDEO);    
  y += SPACING + MINOR_SPACING;
  resizeableCheckbox = cards->createCheckbox(XPOS, y, XPOS + X_SIZE, y + SPACING, "Window resizeable", VIDEO);
  y += SPACING + MINOR_SPACING;
  doublebufCheckbox  = cards->createCheckbox(XPOS, y, XPOS + X_SIZE, y + SPACING, "Use double buffering", VIDEO);    
  y += SPACING + MINOR_SPACING;
  stencilbufCheckbox = cards->createCheckbox(XPOS, y, XPOS + X_SIZE, y + SPACING, "Use stencil buffer", VIDEO );    
  y += SPACING + MINOR_SPACING;
  forceHwsurfCheckbox = cards->createCheckbox(XPOS, y, XPOS + X_SIZE, y + SPACING, "Force hardware surfaces", VIDEO);
  y += SPACING + MINOR_SPACING;
  forceSwsurfCheckbox = cards->createCheckbox(XPOS, y, XPOS + X_SIZE, y + SPACING, "Force software surfaces", VIDEO);
  y += SPACING + MINOR_SPACING;
  multitexturingCheckbox = cards->createCheckbox(XPOS, y, XPOS + X_SIZE, y + SPACING, "Use multitexturing", VIDEO);    
  y += SPACING + MINOR_SPACING;
  hwpalCheckbox   = cards->createCheckbox(XPOS, y, XPOS + X_SIZE, y + SPACING, "Use hardware palette", VIDEO);
  y += SPACING + MINOR_SPACING;
  hwaccelCheckbox = cards->createCheckbox(XPOS, y, XPOS + X_SIZE, y + SPACING, "Use hardware acceleration", VIDEO); 
  y += SPACING + MINOR_SPACING;
  shadowsML = new MultipleLabel(XPOS, y, XPOS + X_SIZE, y + SPACING, "Shadows", 100);
  shadowsML -> addText("None");  
  shadowsML -> addText("Some");
  shadowsML -> addText("All");       
  cards->addWidget(shadowsML, VIDEO);       
  y += SPACING + MINOR_SPACING;
  changeTakeEffectLabel = cards->createLabel(XPOS, y, " ", VIDEO, Constants::BLUE_COLOR);
  
  // audio
  y = YPOS;
  musicVolume = new Slider(XPOS, y, XPOS + X_SIZE, scourge->getShapePalette()->getHighlightTexture(), 0, 128, "Music Volume:");
  cards->addWidget(musicVolume, AUDIO);
  y += SPACING + MINOR_SPACING + 15;
  effectsVolume = new Slider(XPOS, y, XPOS + X_SIZE, scourge->getShapePalette()->getHighlightTexture(), 0, 128, "Effects Volume:");
  cards->addWidget(effectsVolume, AUDIO);
  
  selectedMode = GAME_SETTINGS;
  cards->setActiveCard(GAME_SETTINGS);
}   

void OptionsMenu::loadGameSettings(){
        
    gameSpeedML->setText(gameSpeedML->getNbText() - uc->getGameSpeedLevel() - 1);
    alwaysCenterMapCheckbox->setCheck(uc->getAlwaysCenterMap());
    keepMapSize->setCheck(uc->getKeepMapSize());
    frameOnFullScreen->setCheck(uc->getFrameOnFullScreen());
    turnBasedBattle->setCheck(uc->isBattleTurnBased());
    ovalCutoutShown->setCheck( uc->isOvalCutoutShown() );
    outlineInteractiveItems->setCheck( uc->isOutlineInteractiveItems() );
//    alwaysShowPath->setCheck(uc->getAlwaysShowPath());
    musicVolume->setValue(scourge->getUserConfiguration()->getMusicVolume());
    effectsVolume->setValue(scourge->getUserConfiguration()->getEffectsVolume());
    tooltipEnabled->setCheck(uc->getTooltipEnabled());
    tooltipInterval->setValue(scourge->getUserConfiguration()->getTooltipInterval());
}

// line i must correspond to engine action i if we want this scrolling list to work
void OptionsMenu::loadControls(){
    int i;    
    if(controlLines) {  
        for(int i = 0; i < ENGINE_ACTION_COUNT ; i++){
            if( controlLines[i] ){
                free(controlLines[i]);
                controlLines[i] = NULL;
            }
        }
        free(controlLines);
        controlLines = NULL;
    }
    
    nbControlLines = ENGINE_ACTION_COUNT;
    controlLines = (char **) malloc (nbControlLines *sizeof(char *));    
    for (i = 0; i < nbControlLines; i++){
        controlLines[i] = (char *)malloc(MAX_CONTROLS_LINE_SIZE * sizeof(char));        
        sprintf( controlLines[i], "%s           %s", 
                 uc->getEngineActionDescription(i),
                 uc->getEngineActionKeyName(i) );
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
    if(widget == mainWin->closeButton) {
      scourge->toggleOptionsWindow();
      return true;
    } else if(widget == gameSettingsButton) {
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
	  //	  waitingLabel->setText("Waiting for new key ... Press ESCAPE to cancel");        
	  changeControlButton->setLabel( Constants::getMessage( Constants::WAITING_FOR_KEY ) );
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
    else if(widget == ovalCutoutShown){
        uc ->setOvalCutoutShown(ovalCutoutShown->isChecked());
    }
    else if(widget == outlineInteractiveItems){
        uc ->setOutlineInteractiveItems(outlineInteractiveItems->isChecked());
    }
    else if(widget == alwaysShowPath){
        uc ->setAlwaysShowPath(alwaysShowPath->isChecked());
        //scourge->setShowPath(alwaysShowPath->isChecked());
    } else if(widget == tooltipEnabled){
        uc ->setTooltipEnabled(tooltipEnabled->isChecked());
        if( !( uc ->getTooltipEnabled() ) ) scourge->resetInfos();
    } else if(widget == tooltipInterval){
        uc->setTooltipInterval(tooltipInterval->getValue());
    } else if(widget == videoResolutionML){
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
	else if(widget == closeButton) {
		scourge->toggleOptionsWindow();
		return true;
	}
    else if(widget == saveButton){
        uc->saveConfiguration();
        if(selectedMode == VIDEO){
            changeTakeEffectLabel -> setText("Some changes will only take effect upon restart");            
        }       
    } else if(widget == musicVolume) {
      scourge->getSDLHandler()->getSound()->setMusicVolume(musicVolume->getValue());
      uc->setMusicVolume(musicVolume->getValue());
    } else if(widget == effectsVolume) {
      scourge->getSDLHandler()->getSound()->setEffectsVolume(effectsVolume->getValue());
      uc->setEffectsVolume(effectsVolume->getValue());
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
		scourge->toggleOptionsWindow();
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
	  //waitingLabel->setText(" ");
	  changeControlButton->setLabel( Constants::getMessage( Constants::CHANGE_KEY ) );
	  waitingForNewKey = false;
    } else{
	  switch(event->key.keysym.sym) {
	  case SDLK_RETURN:
		if(selectedMode == CONTROLS){
		  //waitingLabel->setText("Waiting for new key ... Press ESCAPE to cancel");        
		  changeControlButton->setLabel( Constants::getMessage( Constants::WAITING_FOR_KEY) );
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

void OptionsMenu::hide() {
  mainWin->setVisible(false);
}

