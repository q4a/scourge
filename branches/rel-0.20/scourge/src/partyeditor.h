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
#include "common/constants.h"
#include "gui/widgetview.h"
#include "party.h"
#include "rpg/rpg.h"
                         
/**
  *@author Gabor Torok
  */

class RenderedCreature;
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

class PcEditor;

class PartyEditor : public CreatureGroupInfo {
private:

  Scourge *scourge;
  std::map<int,Creature*> maxSkills;
	PcEditor *pcEditor;
  
public:
  PartyEditor(Scourge *scourge);
  ~PartyEditor();

  Creature *getHighestSkillPC( int skill );
  void recomputeMaxSkills();

  bool isVisible();
  void setVisible( bool b );
  Button *getStartGameButton();
  Button *getCancelButton();
  void handleEvent( Widget *widget, SDL_Event *event );
  void createParty( Creature **pc, int *partySize=NULL, bool addRandomInventory=true );
	RenderedCreature *createWanderingHero( int level );

 protected:
  void saveUI( Creature **pc );
  void addStartingInventory( Creature **pc, int partySize );
	void addStartingInventory( Creature *pc );
  //void setCharType( int pcIndex, int charIndex );
  //void setDeityType( int pcIndex, int deityIndex );

};

#endif
