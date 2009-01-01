/***************************************************************************
                 conversationgui.h  -  The conversation window
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

#ifndef CONVERSATION_GUI_H
#define CONVERSATION_GUI_H
#pragma once

#include <iostream>
#include <string>
#include <map>
#include "scourge.h"
#include "gui/window.h"
#include "gui/widget.h"
#include "gui/button.h"
#include "gui/canvas.h"
#include "gui/widgetview.h"
#include "gui/scrollinglabel.h"
#include "gui/scrollinglist.h"
#include "gui/eventhandler.h"

class Creature;
class CardContainer;

/// The conversation window.

class ConversationGui : public WordClickedHandler, EventHandler {

private:
	Scourge *scourge;
	Creature *creature;
	Window *win;
	bool useCreature;
	Label *label;
	ScrollingLabel *answer;

	ScrollingList *list;
	std::vector<std::string> words;
	TextField *entry;
	Canvas *canvas;

	Button *closeButton, *tradeButton, *trainButton;
	Button *identifyButton, *uncurseItemButton, *rechargeButton;
	Button *healButton, *donateButton;
	CardContainer *cards;

public:
	ConversationGui( Scourge *scourge );
	~ConversationGui();

	bool handleEvent( Widget *widget, SDL_Event *event );

	inline Creature *getCreature() {
		return creature;
	}
	void start( Creature *creature );
	void start( Creature *creature, char const* message, bool useCreature );
	inline Window *getWindow() {
		return win;
	}
	void hide();

	void wordClicked( std::string const& word );
	void showingWord( char *word );

	/// Widget::Draw handler
	bool onDraw( Widget* w );
};

#endif

