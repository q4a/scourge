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

char Board::objectiveName[OBJECTIVE_COUNT][80] = {
  "FIND_OBJECT",
  "KILL_MONSTER"
};

Board::Board(Scourge *scourge) {
  this->scourge = scourge;
  
  char errMessage[500];
  char s[200];
  sprintf(s, "%s/world/missions.txt", rootDir);
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
	  cerr << "Creating mission: " << name << endl;

	  // read the level and story count
	  n = Constants::readLine(line, fp);
	  int level = atoi(strtok(line + 1, ","));
	  int stories = atoi(strtok(NULL, ","));

	  // read the objective line
	  //	  cerr << "Creating mission objective..." << endl;
	  n = Constants::readLine(line, fp);
	  MissionObjective *obj = new MissionObjective();
	  obj->index = -1;
	  strcpy(obj->name, "");
	  obj->paramCount = 0;
	  obj->itemCount = 0;
	  obj->monsterCount = 0;
	  char *p = strtok(line + 1, ",");
	  for(int i = 0; i < OBJECTIVE_COUNT; i++) {
		if(!strcmp(p, objectiveName[i])) {
		  obj->index = i;
		  strcpy(obj->name, objectiveName[i]);
		  while(obj->paramCount < MAX_OBJECTIVE_PARAM_COUNT) {
			char *s = strtok(NULL, ",");
			if(!s) break;
			strcpy(obj->param[obj->paramCount++], s);
		  }
		  break;
		}
	  }

	  vector<Mission *> *v;
	  if(missions.find(level) == missions.end()) {
		v = new vector<Mission *>();
		missions[level] = v;
	  } else v = missions[level];
	  //	  cerr << "Creating mission..." << endl;
	  current_mission = new Mission(name, level, stories);
	  current_mission->setObjective(obj);
	  v->push_back(current_mission);
	} else if(n == 'S' && current_mission) {
	  //	  cerr << "\tadding to story..." << endl;
	  // skip ':'
	  fgetc(fp);
	  n = Constants::readLine(line, fp);
	  current_mission->addToStory(line);
	} else if(n == 'I' && current_mission) {
	  //	  cerr << "\tadding to items..." << endl;
	  fgetc(fp);
	  n = Constants::readLine(line, fp);
	  if(current_mission->getObjective()->itemCount < MAX_OBJECTIVE_PARAM_COUNT) {
		current_mission->getObjective()->item[current_mission->getObjective()->itemCount] = RpgItem::getItemByName(line);
		current_mission->getObjective()->itemHandled[current_mission->getObjective()->itemCount] = false;
		current_mission->getObjective()->itemCount++;
	  }
	} else if(n == 'M' && current_mission) {
	  //	  cerr << "\tadding to monsters..." << endl;
	  fgetc(fp);
	  n = Constants::readLine(line, fp);
	  if(current_mission->getObjective()->monsterCount < MAX_OBJECTIVE_PARAM_COUNT) {
		current_mission->getObjective()->monster[current_mission->getObjective()->monsterCount] = Monster::getMonsterByName(line);
		current_mission->getObjective()->monsterHandled[current_mission->getObjective()->monsterCount] = false;
		current_mission->getObjective()->monsterCount++;
	  }
	} else {
	  n = Constants::readLine(line, fp);
	}
  }
  fclose(fp);

  // init gui
  boardWin = scourge->createWoodWindow((scourge->getSDLHandler()->getScreen()->w - BOARD_GUI_WIDTH) / 2, 
									   (scourge->getSDLHandler()->getScreen()->h - BOARD_GUI_HEIGHT) / 2, 
									   BOARD_GUI_WIDTH, BOARD_GUI_HEIGHT, 
									   strdup("Available Missions"));
  missionList = new ScrollingList(5, 40, BOARD_GUI_WIDTH - 10, 150);
  boardWin->addWidget(missionList);
  missionDescriptionLabel = new Label(5, 210, strdup(""), 67);
  boardWin->addWidget(missionDescriptionLabel);
  playMission = new Button(5, 5, 105, 35, Constants::getMessage(Constants::PLAY_MISSION_LABEL));
  boardWin->addWidget(playMission);
}

Board::~Board() {
  freeListText();
}

void Board::freeListText() {
  // free ui
  if(availableMissions.size()) {
	for(int i = 0; i < (int)availableMissions.size(); i++) {
	  free(missionText[i]);
	}
	free(missionText);
	free(missionColor);
  }
}

void Board::reset() {
  cerr << "Resetting missions" << endl;
  for(map<int, vector<Mission*>* >::iterator i=missions.begin(); i!=missions.end(); ++i) {
	vector<Mission*> *p = i->second;
	for(vector<Mission*>::iterator e=p->begin(); e!=p->end(); ++e) {
	  Mission *m = *e;
	  m->reset();
	}
  }
}

void Board::initMissions() {
  // free ui
  freeListText();

  // find the highest and lowest levels in the party
  int highest = 0;
  int lowest = -1;
  int sum = 0;
  for(int i = 0; i < 4; i++) {
	if(highest < scourge->getParty()->getParty(i)->getLevel()) {
	  highest = scourge->getParty()->getParty(i)->getLevel();
	} else if(lowest == -1 || lowest > scourge->getParty()->getParty(i)->getLevel()) {
	  lowest = scourge->getParty()->getParty(i)->getLevel();
	}
	sum += scourge->getParty()->getParty(i)->getLevel();
  }
  int ave = (int)((float)sum / (float)scourge->getParty()->getPartySize());

  // find missions
  availableMissions.clear();  
  for(int level = 0; level <= highest; level++) {
	if(missions.find(level) == missions.end()) continue;
	vector<Mission*> *v = missions[level];
	for(int i = 0; i < (int)v->size(); i++) {
	  availableMissions.push_back((*v)[i]);
	}
  }

  // init ui
  if(availableMissions.size()) {
	missionText = (char**)malloc(availableMissions.size() * sizeof(char*));
	missionColor = (Color*)malloc(availableMissions.size() * sizeof(Color));
	for(int i = 0; i < (int)availableMissions.size(); i++) {
	  missionText[i] = (char*)malloc(120 * sizeof(char));
	  sprintf(missionText[i], "L:%d, S:%d, %s%s", 
			  availableMissions[i]->getLevel(), 
			  availableMissions[i]->getDungeonStoryCount(), 
			  availableMissions[i]->getName(),
			  (availableMissions[i]->isCompleted() ? "(completed)" : ""));
	  missionColor[i].r = 1.0f;
	  missionColor[i].g = 1.0f;
	  missionColor[i].b = 0.0f;
	  if(availableMissions[i]->isCompleted()) {
		missionColor[i].r = 0.5f;
		missionColor[i].g = 0.5f;
		missionColor[i].b = 0.5f;
	  } else if(availableMissions[i]->getLevel() < ave) {
		missionColor[i].r = 1.0f;
		missionColor[i].g = 1.0f;
		missionColor[i].b = 1.0f;
	  } else if(availableMissions[i]->getLevel() > ave) {
		missionColor[i].r = 1.0f;
		missionColor[i].g = 0.0f;
		missionColor[i].b = 0.0f;
	  }
	  if(i == 0) {
		missionDescriptionLabel->setText((char*)availableMissions[i]->getStory());
	  }
	}
	missionList->setLines(availableMissions.size(), (const char**)missionText, missionColor);
  }
}

int Board::handleEvent(Widget *widget, SDL_Event *event) {
  if(widget == boardWin->closeButton) {
	boardWin->setVisible(false);
	return EVENT_HANDLED;
  } else if(widget == missionList) {
	int selected = missionList->getSelectedLine();
	if(selected != -1 && selected < getMissionCount()) {
	  missionDescriptionLabel->setText((char*)(getMission(selected)->getStory()));
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


// ------------------------------
// Mission stuff
//
Mission::Mission(char *name, int level, int dungeonStoryCount) {
  strcpy(this->name, name);
  this->level = level;
  this->dungeonStoryCount = dungeonStoryCount;

  strcpy(this->story, "");
  this->completed = false;
  this->objective = NULL;
}

Mission::~Mission() {
  delete objective;
}

bool Mission::itemFound(RpgItem *item) {
  if(!completed && 
	 objective &&
	 item) {
	for(int i = 0; i < objective->itemCount; i++) {
	  if(objective->item[i] == item && !objective->itemHandled[i]) {
		objective->itemHandled[i] = true;
		checkMissionCompleted();
		return isCompleted();
	  }
	}
  }
  return false;
}

bool Mission::monsterSlain(Monster *monster) {
  if(!completed &&
	 objective &&
	 monster) {
	for(int i = 0; i < objective->monsterCount; i++) {
	  if(objective->monster[i] == monster &&
		 !objective->monsterHandled[i]) {
		objective->monsterHandled[i] = true;
		checkMissionCompleted();
		return isCompleted();
	  }
	}
  }
  return false;
}

void Mission::checkMissionCompleted() {
  completed = true;
  if(objective) {
	for(int i = 0; i < objective->itemCount; i++) {
	  if(!objective->itemHandled[i]) {
		completed = false;
		return;
	  }
	}
	for(int i = 0; i < objective->monsterCount; i++) {
	  if(!objective->monsterHandled[i]) {
		completed = false;
		return;
	  }
	}
  }
}

void Mission::reset() {
  completed = false;
  if(objective) {
	for(int i = 0; i < objective->itemCount; i++) {
	  objective->itemHandled[i] = false;
	}
	for(int i = 0; i < objective->monsterCount; i++) {
	  objective->monsterHandled[i] = false;
	}
  }
}
