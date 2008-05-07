/***************************************************************************
  donatedialog.h  -  description
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

#ifndef DONATE_DIALOG_H
#define DONATE_DIALOG_H

#include "common/constants.h"
#include <map>

class Scourge;
class Creature;
class Window;
class Label;
class ScrollingLabel;
class Widget;
class Button;
class TextField;

class DonateDialog {
private:
  Scourge *scourge;
  Creature *creature;
  Window *win;
  
  Label *creatureLabel, *coinLabel;
  ScrollingLabel *result;
  Button *closeButton, *applyButton;
  TextField *amount;

public:
  DonateDialog( Scourge *scourge );
  ~DonateDialog();
  void setCreature( Creature *creature );
  void updateUI();
  inline Window *getWindow() { return win; }
  void handleEvent( Widget *widget, SDL_Event *event );
  
protected:
  void donate( int amount );
};

#endif

