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

const Mission Board::prototypes[] = {
	{ "Quest for the missing wand", 0, 2,
		"One of our returning clients, Miffy the Merciless \
		(the name has been changed to protect the client's identity) \
		has managed to loose his favorite wand of Slow Strangling, \
		somewhere on level 2 within the Dungeon of Distaste. Your \
		job is to fetch this object and return it to him. \
		Don't even think about trying to keep or use the wand, as \
		it is magically encoded to only work for its owner."		 
	},
	{ "Strange creatures seen in Hopelessness", 0, 3, 
		"The local authorities contracted us to clean out the  \
		hillside dungeon of Fatal Hopelessness. They have reportedly seen strange \
		monsters within its dismal halls. Please behave yourselves \
		on this mission, as it is to our advantage to co-operate \
		with the local police force."
	}
};
	
Board::Board(Scourge *scourge) {
	this->scourge = scourge;
	// organize missons per level
	for(int i = 0; i < MISSION_COUNT; i++) {
		vector<const Mission*> *list = NULL;
		if(prototypesPerLevel.find(prototypes[i].level) != prototypesPerLevel.end()){
			list = prototypesPerLevel[(const int)(prototypes[i].level)];
		} else {
			list = new vector<const Mission*>();
			prototypesPerLevel[prototypes[i].level] = list;
		}
		list->push_back(&(prototypes[i]));
	}
}

Board::~Board() {
}

void Board::initMissions() {
	missions.clear();
	int sum = 0;
	for(int i = 0; i < 4; i++) sum += scourge->getParty(i)->getLevel();
	for(int i = 0; i < MAX_AVAILABLE_MISSION_COUNT; i++) {
		// find the average level for this mission
		int level = (int)(((float)(sum) / 4.0f) + (4.0f * rand() / RAND_MAX) - 2.0f);
		// pick a mission of this level
		int count = 0;
		if(prototypesPerLevel.find(level) != prototypesPerLevel.end()){
			count = prototypesPerLevel[level]->size();
		}		
		const Mission *p;
		if(!count) {
//			cerr << "i=" << i << "no missions for level: " << level << ", reusing first mission." << endl;
			p = &(prototypes[0]);
		} else {
			int index = (int)((float)count * rand() / RAND_MAX);
			//cerr << "i=" << i << "level=" << level << " index=" << index << endl;
			vector<const Mission*> list = *prototypesPerLevel[level];
			p = list[index];
		}

		// see if this mission has been chosen yet
		// FIXME: use a vector-set combo instead for mission; something that supports [] and uniqueness
		bool found = false;
		for(int i = 0; i < missions.size(); i++) {
			if(missions[i] == p) {
				found = true;
				break;
			}
		}
		if(!found) missions.push_back(p);

	}
}


