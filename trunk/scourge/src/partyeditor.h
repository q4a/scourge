/***************************************************************************
                          partyeditor.h  -  description
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

#ifndef PARTY_EDITOR_H
#define PARTY_EDITOR_H

#include <string.h>
#include "constants.h"
#include "sdlhandler.h"
#include "sdleventhandler.h"
#include "sdlscreenview.h"
#include "scourge.h"
#include "userconfiguration.h"
#include "util.h"
#include "gui/window.h"
#include "gui/button.h"
#include "gui/scrollinglist.h"
#include "gui/cardcontainer.h"
#include "gui/multiplelabel.h"
#include "gui/checkbox.h"
#include "gui/slider.h"

/**
  *@author Gabor Torok
  */

class Scourge;
class UserConfiguration;

class PartyEditor {
private:

  enum {
    INTRO_TEXT = 0,
    SELECT_DIETY,
    SELECT_NAME,
    SELECT_CLASS,
    SELECT_MODEL_AND_PORTRAIT,
    ADJUST_SKILLS,
    OUTRO_TEXT
  };

  Scourge *scourge;
  Window *mainWin;
  CardContainer *cards;
  Label *intro;
  Button *next, *back, *cancel, *done;

  int step;
  
public:
  PartyEditor(Scourge *scourge);
  ~PartyEditor();

  inline void setVisible( bool b ) { mainWin->setVisible( b ); }
  inline Widget *getStartGameButton() { return done; }

};

#endif
