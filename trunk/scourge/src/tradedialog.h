/***************************************************************************
tradedialog.h  -  The trade dialog
-------------------
    begin                : 8/26/2005
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

#ifndef TRADE_DIALOG_H
#define TRADE_DIALOG_H

#include "common/constants.h"
#include "itemlist.h"
#include "pcui.h"
#include <map>

class Scourge;
class Creature;
class Window;
class Label;
class Widget;
class Item;
class PcUi;

class TradeDialog : public ItemRenderer {
private:
  Scourge *scourge;
  Creature *creature;
  Window *win;
  int totalAmount;
  std::map<Item*, int> prices;
  
  Label *playerName, *creatureName;
  Label *playerTotal, *creatureTotal;
  ItemList *playerList, *creatureList;
  Button *closeButton, *tradeButton, *infoButtonA, *infoButtonB, *stealButton;
  Label *tradeInfo;
  
public:
  TradeDialog( Scourge *scourge );
  ~TradeDialog();
  void setCreature( Creature *creature );
  void updateUI();
  inline Window *getWindow() { return win; }
  void handleEvent( Widget *widget, SDL_Event *event );
  
  void render( const Widget *widget, const Item *item, std::string& buffer );
  
protected:
    void updateLabels();
  void trade();
  void steal();
  int getSelectedTotal( ItemList *list );
  bool validateInventory();
};

#endif
