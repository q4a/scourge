#pragma once
#ifndef GAME_STATE_HANDLER_H
#define GAME_STATE_HANDLER_H


class GameStateHandler {
public:
	GameStateHandler();
	virtual ~GameStateHandler();

	// the producer
	virtual char *getGameState() = 0;

};

#endif
