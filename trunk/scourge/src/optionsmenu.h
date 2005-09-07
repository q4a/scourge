/***************************************************************************
                          options.h  -  description
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

#ifndef OPTIONSMENU_H
#define OPTIONSMENU_H

#include <string.h>
#include "constants.h"
#include "sdlhandler.h"
#include "sdleventhandler.h"
#include "sdlscreenview.h"
#include "scourge.h"
#include "userconfiguration.h"
#include "util.h"
#include "gui/window.h"
#include "gui/button.h"
#include "gui/scrollinglist.h"
#include "gui/cardcontainer.h"
#include "gui/multiplelabel.h"
#include "gui/checkbox.h"
#include "gui/slider.h"

/**
  *@author Gabor Torok
  */

#define MAX_CONTROLS_LINE_SIZE 80

class Scourge;
class UserConfiguration;

class OptionsMenu {
private:

  char ** controlLines; // move it to function loadControls() ?
  Scourge *scourge;
  UserConfiguration * uc;
  bool showDebug;
  bool controlsLoaded;
  bool videoLoaded; 
  bool gameSettingsLoaded;
  int nbControlLines;
  bool waitingForNewKey;
  bool ignoreKeyUp;
    
  enum modeOptions {
    CONTROLS = 0, VIDEO, AUDIO, GAME_SETTINGS
  };
  int selectedMode; 
  
  Window *mainWin;  
  Button *controlsButton, *videoButton, *audioButton, *gameSettingsButton;
  Button *changeControlButton, *saveButton, *closeButton;
  Label * keyBindingsLabel; 
  Label * waitingLabel;  
  Label * changeTakeEffectLabel;
  
  MultipleLabel * gameSpeedML; 
  Checkbox * alwaysCenterMapCheckbox;
  Checkbox * keepMapSize;
  Checkbox * frameOnFullScreen;
  Checkbox * turnBasedBattle;
  Checkbox *ovalCutoutShown;
  Checkbox *outlineInteractiveItems;
  Checkbox * alwaysShowPath;
  Checkbox *tooltipEnabled;
  Slider *tooltipInterval;
  
  MultipleLabel * videoResolutionML;  
  Checkbox * fullscreenCheckbox;
  Checkbox * doublebufCheckbox;   
  Checkbox * hwpalCheckbox;
  Checkbox * resizeableCheckbox;
  Checkbox * forceHwsurfCheckbox;
  Checkbox * forceSwsurfCheckbox;
  Checkbox * hwaccelCheckbox;
  Checkbox * stencilbufCheckbox;
  Checkbox * multitexturingCheckbox;
  MultipleLabel * shadowsML;
     
  CardContainer *cards;
  ScrollingList *controlBindingsList;

  Slider *musicVolume;
  Slider *effectsVolume;

  void setSelectedMode();
  void loadControls(); 
  void loadVideo();
  void loadGameSettings();
  
public:
  OptionsMenu(Scourge *scourge);
  ~OptionsMenu();

  bool handleEvent(SDL_Event *event);
  bool handleEvent(Widget *widget, SDL_Event *event);
  //inline void show() { mainWin->setVisible(true); }
  void show();
  void hide();
  inline bool isVisible() { return mainWin->isVisible(); }    

 protected:
};

#endif
