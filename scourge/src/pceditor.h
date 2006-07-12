/***************************************************************************
                          pceditor.h  -  description
                             -------------------
    begin                : Tue Jul 10 2006
    copyright            : (C) 2006 by Gabor Torok
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

#ifndef PC_EDITOR_H
#define PC_EDITOR_H

#include "common/constants.h"

class Window;
class Scourge;
class Creature;
class CardContainer;
class Button;
class Widget;
class TextField;
class ScrollingList;
class ScrollingLabel;

class PcEditor {
private:
	Window *win;
	Scourge *scourge;
	Creature *creature;
	CardContainer *cards;
	Button *nameButton, *profButton, *statsButton, *deityButton;
	Button *imageButton, *okButton, *cancelButton;
	TextField *nameField;
	ScrollingList *charType;
	ScrollingLabel *charTypeDescription;
	char **charTypeStr;

	enum {
		NAME_TAB=0,
		CLASS_TAB,
		STAT_TAB,
		DEITY_TAB,
		IMAGE_TAB
	};

public:
	PcEditor( Scourge *scourge );
	~PcEditor();

	void setCreature( Creature *creature );
	inline Window *getWindow() { return win; }

	void handleEvent( Widget *widget, SDL_Event *event );
};

#endif

