/***************************************************************************
                          board.h  -  description
                             -------------------
    begin                : Sat May 3 2003
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

#ifndef BOARD_H
#define BOARD_H

#include "constants.h"
#include "scourge.h"
#include "gui/window.h"
#include "gui/button.h"
#include "gui/scrollinglist.h"
#include "gui/label.h"
#include <map>

using namespace std;
	
/**
  *@author Gabor Torok
  */
	
typedef	struct _Mission {
  char name[80]; // name of mission
  int level; // what level dungeon
  int dungeonStoryCount; // how many stories
  bool completed;
  char story[2000]; // the background story of the level
} Mission;

#define BOARD_GUI_WIDTH 400
#define BOARD_GUI_HEIGHT 400

class Board	{								
 private:
  Scourge *scourge;
  vector<const Mission*> availableMissions;
  map<int, vector<Mission*>* > missions;

  // gui
  ScrollingList *missionList;
  Label *missionDescriptionLabel;
  Button *playMission;
  char **missionText;
 
 public:

  Window *boardWin;
  static const int EVENT_HANDLED = 0;
  static const int EVENT_PLAY_MISSION = 1;

  Board(Scourge *scourge);
  virtual ~Board();
  
  void initMissions();
  
  inline int getMissionCount() { return availableMissions.size(); }
  inline const Mission *getMission(int index) { return availableMissions[index]; }

  int handleEvent(Widget *widget, SDL_Event *event);
  inline int getSelectedLine() { return missionList->getSelectedLine(); }
  
};

#endif

