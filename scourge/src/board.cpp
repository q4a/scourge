/***************************************************************************
                          board.cpp  -  description
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

#include "board.h"
	
/**
  *@author Gabor Torok
  */
	
Board::Board(Scourge *scourge) {
  this->scourge = scourge;
  
  char errMessage[500];
  char s[200];
  sprintf(s, "data/world/missions.txt");
  FILE *fp = fopen(s, "r");
  if(!fp) {        
	sprintf(errMessage, "Unable to find the file: %s!", s);
	cerr << errMessage << endl;
	exit(1);
  }

  Mission *current_mission = NULL;
  char name[255], line[255];
  int n = fgetc(fp);
  while(n != EOF) {
	if(n == 'T') {
	  // skip ':'
	  fgetc(fp);
	  // read the name
	  n = Constants::readLine(name, fp);

	  // read the level and story count
	  n = Constants::readLine(line, fp);
	  int level = atoi(strtok(line + 1, ","));
	  int stories = atoi(strtok(NULL, ","));

	  // skip the objective line for now...
	  n = Constants::readLine(line, fp);

	  vector<Mission *> *v;
	  if(missions.find(level) == missions.end()) {
		v = new vector<Mission *>();
		missions[level] = v;
	  } else v = missions[level];
	  current_mission = new Mission();
	  strcpy(current_mission->name, name);
	  current_mission->level = level;
	  current_mission->dungeonStoryCount = stories;
	  current_mission->completed = false;
	  strcpy(current_mission->story, "");
	  v->push_back(current_mission);
	} else if(n == 'M' && current_mission) {
	  // skip ':'
	  fgetc(fp);
	  n = Constants::readLine(line, fp);
	  strcat(current_mission->story, line);
	} else {
	  n = Constants::readLine(line, fp);
	}
  }
  fclose(fp);

  // init gui
  boardWin = new Window( scourge->getSDLHandler(),
						 (scourge->getSDLHandler()->getScreen()->w - BOARD_GUI_WIDTH) / 2, 
						 (scourge->getSDLHandler()->getScreen()->h - BOARD_GUI_HEIGHT) / 2, 
						 BOARD_GUI_WIDTH, BOARD_GUI_HEIGHT, 
						 strdup("Available Missions"), 
						 scourge->getShapePalette()->getGuiWoodTexture(),
						 true, Window::SIMPLE_WINDOW );
  boardWin->setBackgroundTileHeight(96);
  boardWin->setBorderColor( 0.5f, 0.2f, 0.1f );
  boardWin->setColor( 0.8f, 0.8f, 0.7f, 1 );
  boardWin->setBackground( 0.65, 0.30f, 0.20f, 0.15f );
  boardWin->setSelectionColor(  0.25f, 0.35f, 0.6f );


  missionList = new ScrollingList(5, 40, BOARD_GUI_WIDTH - 10, 150);
  boardWin->addWidget(missionList);
  missionDescriptionLabel = new Label(5, 210, strdup(""), 70);
  boardWin->addWidget(missionDescriptionLabel);
  playMission = new Button(5, 5, 105, 35, Constants::getMessage(Constants::PLAY_MISSION_LABEL));
  boardWin->addWidget(playMission);
}

Board::~Board() {
  // free ui
  if(availableMissions.size()) {
	for(int i = 0; i < availableMissions.size(); i++) {
	  free(missionText[i]);
	}
	free(missionText);
  }
}

void Board::initMissions() {
  // free ui
  if(availableMissions.size()) {
	for(int i = 0; i < availableMissions.size(); i++) {
	  free(missionText[i]);
	}
	free(missionText);
  }

  // find the highest level in the party
  int highest = 0;
  for(int i = 0; i < 4; i++) 
	if(highest < scourge->getParty()->getParty(i)->getLevel())
	  highest = scourge->getParty()->getParty(i)->getLevel();

  // find missions
  availableMissions.clear();  
  for(int level = 0; level <= highest; level++) {
	if(missions.find(level) == missions.end()) continue;
	vector<Mission*> *v = missions[level];
	for(int i = 0; i < v->size(); i++) {
	  availableMissions.push_back((*v)[i]);
	}
  }

  // init ui
  if(availableMissions.size()) {
	missionText = (char**)malloc(availableMissions.size() * sizeof(char*));
	for(int i = 0; i < availableMissions.size(); i++) {
	  missionText[i] = (char*)malloc(120 * sizeof(char));
	  strcpy(missionText[i], availableMissions[i]->name);
	  if(i == 0) {
		missionDescriptionLabel->setText((char*)availableMissions[i]->story);
	  }
	}
	missionList->setLines(availableMissions.size(), (const char**)missionText);
  }
}

int Board::handleEvent(Widget *widget, SDL_Event *event) {
  if(widget == boardWin->closeButton) {
	boardWin->setVisible(false);
	return EVENT_HANDLED;
  } else if(widget == missionList) {
	int selected = missionList->getSelectedLine();
	if(selected != -1 && selected < getMissionCount()) {
	  missionDescriptionLabel->setText((char*)(getMission(selected)->story));
	}
	return EVENT_HANDLED;
  } else if(widget == playMission) {
	int selected = missionList->getSelectedLine();
	if(selected != -1 && selected < getMissionCount()) {
	  return EVENT_PLAY_MISSION;
	}
	return EVENT_HANDLED;
  }
  return -1;
}
