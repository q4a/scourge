/***************************************************************************
                          label.h  -  description
                             -------------------
    begin                : Thu Aug 28 2003
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

#ifndef LABEL_H
#define LABEL_H

#include "../constants.h"
#include "../sdlhandler.h"
#include "widget.h"

/**
  *@author Gabor Torok
  */

class SDLHandler;

class Label : public Widget {
 private:
  char *text;
 public: 
  Label(int x, int y, char *text);
  ~Label();
  inline char *getText() { return text; }
  void drawWidget(SDLHandler *sdlHandler);
};

#endif

