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
#include <map>
#include "constants.h"
#include "gui/widgetview.h"
#include "party.h"
#include "rpg/rpg.h"
                         
/**
  *@author Gabor Torok
  */

class Creature;
class Scourge;
class UserConfiguration;
class CharacterInfoUI;
class Window;
class Button;
class Canvas;
class ScrollingList;
class ScrollingLabel;
class CardContainer;
class TextField;
class Label;
class SkillsView;

typedef struct _CharacterInfo {
  TextField *name;

  ScrollingList *charType;
  ScrollingLabel *charTypeDescription;
  char **charTypeStr;

  ScrollingList *deityType;
  ScrollingLabel *deityTypeDescription;
  char **deityTypeStr;

  CharacterInfoUI *detailsInfo;
  Canvas *detailsCanvas;
  
  Canvas *portrait;
  Button *nextPortrait;
  Button *prevPortrait;
  int portraitIndex;
  
  Canvas *model;
  Button *nextModel;
  Button *prevModel;
  int modelIndex;

  Label *skillLabel;
  SkillsView *skills;
  Button *skillAddButton, *skillRerollButton, *skillSubButton;
  int availableSkillMod;
  int skill[ Skill::SKILL_COUNT ], skillMod[ Skill::SKILL_COUNT ];
  ScrollingLabel *skillDescription;

  Button *back, *next;
  
} CharacterInfo;

class PartyEditor : public WidgetView, CreatureGroupInfo {
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
  Uint32 lastTick;
  float zrot;  
  Creature *tmp[4];
  std::map<int,Creature*> maxSkills;
  
public:
  PartyEditor(Scourge *scourge);
  ~PartyEditor();

  void drawWidgetContents(Widget *w);
  void drawAfter();

  Creature *getHighestSkillPC( int skill );
  void recomputeMaxSkills();

  bool isVisible();
  void setVisible( bool b );
  inline Button *getStartGameButton() { return done; }
  inline Button *getCancelButton() { return cancel; }
  void reset();
  void handleEvent( Widget *widget, SDL_Event *event );
  void createParty( Creature **pc, int *partySize=NULL, bool addRandomInventory=true );

 protected:
  void createCharUI( int step, CharacterInfo *info );
  void deleteLoadedShapes();
  void rollSkills( CharacterInfo *info );
  void updateUI( CharacterInfo *info, int index );
  void saveUI( Creature **pc );
  void addStartingInventory( Creature **pc, int partySize );
  void setCharType( int pcIndex, int charIndex );
  void setDeityType( int pcIndex, int deityIndex );

};

#endif
