/***************************************************************************
                          constants.cpp  -  description
                             -------------------
    begin                : Sun Oct 12 2003
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

#include "constants.h"

char rootDir[300];

//sprintf(s, "Welcome to Scourge version %7.2f", SCOURGE_VERSION);
char *Constants::messages[][80] = {
  { 
	"Infamy awaits in the dungeons of Scourge!", 
	"Another day, another sewer! Welcome to Scourge!", 
	"Happy hunting; welcome to Scourge!" },
  { "That item is out of your reach", 
	"You can't touch that", 
	"You have to be closer to get that", 
	"You are too far to reach it" }
};

int Constants::messageCount[] = {
  3, 4
};

Constants::Constants(){
}

Constants::~Constants(){
}

char *Constants::getMessage(int index) {
  int n = (int)((float)messageCount[index] * rand() / RAND_MAX);
  return messages[index][n];
}
