/***************************************************************************
                          progress.h  -  description
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

#ifndef PROGRESS_H
#define PROGRESS_H

#include "../constants.h"
#include "../scourge.h"

using namespace std;
	
/**
  *@author Gabor Torok
  */
class Progress	{								
 private:
  Scourge *scourge;
  int maxStatus;
  int status;
  bool clearScreen;
  bool center;
  bool opaque;
 
 public:
  Progress(Scourge *scourge, int maxStatus, bool clearScreen=false, bool center=false, bool opaque=true);
  virtual ~Progress();  
  void updateStatus(const char *message, bool updateScreen=true, int status=-1, int maxStatus=-1, int altStatus=-1);
};

#endif

