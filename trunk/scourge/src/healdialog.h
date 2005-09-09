/***************************************************************************
  healdialog.h  -  description
-------------------
    begin                : 9/9/2005
    copyright            : (C) 2005 by Gabor Torok
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

#ifndef HEAL_DIALOG_H
#define HEAL_DIALOG_H

#include "constants.h"
#include <map>

class Scourge;
class Creature;
class Window;
class Label;
class ScrollingList;
class ScrollingLabel;
class Widget;
class Button;
class Spell;

using namespace std;

class HealDialog {
private:
  Scourge *scourge;
  Creature *creature;
  Window *win;
  vector<Spell*> spells;
  map<Spell*, int> prices;
  
  Label *creatureLabel, *coinLabel;
  ScrollingList *spellList;
  ScrollingLabel *spellDescription;
  Button *closeButton, *applyButton;

	char **spellText;
	GLuint *spellIcons;

public:
  HealDialog( Scourge *scourge );
  ~HealDialog();
  void setCreature( Creature *creature );
  void updateUI();
  inline Window *getWindow() { return win; }
  void handleEvent( Widget *widget, SDL_Event *event );
  
protected:
  void heal( Spell *spell, int price );
  void showSpellDescription( Spell *spell );
};

#endif

