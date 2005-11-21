/***************************************************************************
                          characterinfo.h  -  description
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

#ifndef CHARACTER_INFO_H
#define CHARACTER_INFO_H

#include <iostream>
#include <string>
#include <vector>
#include "constants.h"
#include "session.h"
#include "gui/widgetview.h"

/**
  *@author Gabor Torok
  */

class Session;
class Creature;
class Window;

class CharacterInfoUI : public WidgetView {
private:
  Scourge *scourge;
  Creature *creature;
  Window *win;

public:
  CharacterInfoUI( Scourge *scourge );
  ~CharacterInfoUI();

  inline void setCreature( Window *win, 
                           Creature *creature ) { 
    this->win = win;
    this->creature = creature; 
  }
  inline Creature *getCreature() { return creature; }
  inline Window *getWindow() { return win; }

  /** Draw the widget. */
  virtual void drawWidgetContents( Widget *w );

};

#endif

