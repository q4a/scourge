/***************************************************************************
                    netplay.h  -  Handler for online play
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

#ifndef NET_PLAY_H
#define NET_PLAY_H
#pragma once

#include <iostream>
#include <string>
#include "scourge.h"
#include "net/gamestatehandler.h"
#include "net/commands.h"
#include "gui/scrollinglist.h"


/**
  @author Gabor Torok
*/

/// Handles online play.
class NetPlay : public GameStateHandler, CommandInterpreter {
private:
	Scourge *scourge;
	Window *mainWin;
	ScrollingList *messageList;
	TextField *chatText;
	int chatStrCount;
	std::string* chatStr;
	static const int MAX_CHAT_SIZE = 100;
	static const int CHAT_STR_LENGTH = 120;

public:
	NetPlay( Scourge *scourge );
	virtual ~NetPlay();

	char *getGameState();

	void chat( char *message );
	void logout();
	void ping( int frame );
	void processGameState( int frame, char *p );
	void handleUnknownMessage();
	void serverClosing();
	void character( char *bytes, int length );
	void addPlayer( Uint32 id, char *bytes, int length );

	inline Window *getWindow() {
		return mainWin;
	}

	bool handleEvent( Widget *widget, SDL_Event *event );
};

#endif

