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
#include <map>

using namespace std;
	
/**
  *@author Gabor Torok
  */
	
typedef	struct _Mission {
	char name[80]; // name of mission
	int level; // what level dungeon
	int dungeonStoryCount; // how many stories
	char story[2000]; // the background story of the level
} Mission;

class Board	{								
private:
	Scourge *scourge;
	vector<const Mission* > missions;
	static const Mission prototypes[];
	map<int, vector<const Mission*>* > prototypesPerLevel;
		
public:

	static const int MAX_AVAILABLE_MISSION_COUNT = 20;

	enum {
		MISSING_WAND = 0,
		STRANGE_CREATURES,
		DIAMONDS,
		
		// must be the last one
		MISSION_COUNT
	};

public:

	Board(Scourge *scourge);
	virtual ~Board();

	void initMissions();

	inline int getMissionCount() { return missions.size(); }
	inline const Mission *getMission(int index) { return missions[index]; }
	
};

#endif

