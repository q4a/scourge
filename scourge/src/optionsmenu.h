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
#include "gui/window.h"
#include "gui/button.h"
#include "gui/scrollinglist.h"
#include "gui/cardcontainer.h"
#include "gui/multiplelabel.h"

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
  int nbControlLines;
  bool waitingForNewKey;
  bool ignoreKeyUp;
    
  enum modeOptions {
    CONTROLS = 0, VIDEO, AUDIO, GAME_SETTINGS
  };
  int selectedMode; 
  
  Window *mainWin;  
  Button *controlsButton, *videoButton, *audioButton, *gameSettingsButton, *closeButton;  
  Button *changeControlButton, *saveControlButton;
  Label * waitingLabel;  
  
  MultipleLabel * gameSpeedML;  

  
  CardContainer *cards;
  ScrollingList *controlBindingsList;
  
  void createButton(int x1, int y1, int x2, int y2, char *label, bool toggle, Button * &theButton);
  void setSelectedMode();
  void loadControls();      
  
public:
  OptionsMenu(Scourge *scourge);
  ~OptionsMenu();

  bool handleEvent(SDL_Event *event);
  bool handleEvent(Widget *widget, SDL_Event *event);
  inline void show() { mainWin->setVisible(true); }
  inline void hide() { mainWin->setVisible(false); }
  inline bool isVisible() { return mainWin->isVisible(); }  

 protected:
};

#endif
