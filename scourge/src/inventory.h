
/***************************************************************************
                          inventory.h  -  description
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

#ifndef INVENTORY_H
#define INVENTORY_H

#include <iostream>
#include "constants.h"
#include "sdlhandler.h"
#include "sdleventhandler.h"
#include "sdlscreenview.h"
#include "gui.h"
#include "scourge.h"
#include "rpg/pc.h"

/**
  *@author Gabor Torok
  */

class Inventory : public SDLEventHandler,SDLScreenView {
private:
    Scourge *scourge;
    int win;
    int selected; // which player is selected?
    int selectedMode; // which mode is selected?
    char *modeName[4];
    enum mode {
        CHARACTER = 0, INVENTORY, SPELL, LOG
    };
    int skillList, itemList;
    char **invText, **pcInvText;

protected:

public:
    Inventory(Scourge *scourge);
	~Inventory();
    void show();
    void drawView(SDL_Surface *screen);
    bool handleEvent(SDL_Event *event);
    void drawInventory();

protected:
    void createGui();
    void drawParty();
    void drawModeButtons();
    bool processMouseClick(int x, int y, int button);
    void drawCharacterInfo();
    void drawInventoryInfo();
    void drawSpellInfo();
    void drawLogInfo();
};

#endif
