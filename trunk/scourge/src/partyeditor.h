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
#include "gui/canvas.h"
#include "gui/scrollinglist.h"
#include "gui/cardcontainer.h"

/**
  *@author Gabor Torok
  */

class Scourge;
class UserConfiguration;

typedef struct _CharacterInfo {
  TextField *name;

  ScrollingList *charType;
  Label *charTypeDescription;
  char **charTypeStr;

  ScrollingList *deityType;
  Label *deityTypeDescription;
  char **deityTypeStr;
  
  Canvas *portrait;
  Button *nextPortrait;
  Button *prevPortrait;
  int portraitIndex;
  
  Canvas *model;
  Button *nextModel;
  Button *prevModel;
  int modelIndex;

  Button *back, *next;
  
} CharacterInfo;

class PartyEditor {
private:

  enum {
    INTRO_TEXT = 0,
    CREATE_CHAR_0,
    CREATE_CHAR_1,
    CREATE_CHAR_2,
    CREATE_CHAR_3,
    OUTRO_TEXT
  };

  Scourge *scourge;
  Window *mainWin;
  CardContainer *cards;
  Label *intro;
  Button *cancel, *done;
  Button *toIntro, *toChar0, *toLastChar;

  CharacterInfo info[ MAX_PARTY_SIZE ];

  int step;
  
public:
  PartyEditor(Scourge *scourge);
  ~PartyEditor();

  inline bool isVisible() { return mainWin->isVisible(); }
  inline void setVisible( bool b ) { mainWin->setVisible( b ); }
  inline Widget *getStartGameButton() { return done; }
  inline Widget *getCancelButton() { return cancel; }
  void reset();
  void handleEvent( Widget *widget );

 protected:
  void createCharUI( int step, CharacterInfo *info );
};

#endif
