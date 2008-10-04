#pragma once
#ifdef HAVE_SDL_NET

#ifndef CLIENT_H
#define CLIENT_H

#include "../persist.h"
#include "tcputil.h"
#include "gamestatehandler.h"
#include "testgamestatehandler.h"
#include "broadcast.h"
#include "commands.h"

int clientLoop( void *data );

class Client {
private:
	char const* host;
	int port;
	char const* username;
	bool connected;
	bool readError;
	bool threadRunning;
	SDL_Thread *thread;
	GameStateHandler *gsh;
	int lastGameFrameReceived;
	TCPsocket tcpSocket;
	Broadcast *broadcast;
	IPaddress ip;
	Commands *commands;
	bool tryToReconnect;

	static const Uint32 FIND_SERVER_TIMEOUT = 10000;

public:
	Client( char const* host, int port, char const* username, CommandInterpreter *ci );
	virtual ~Client();
	int connect();
	bool findServer();
	int login();
	int sendChatTCP( char *message );
	int sendPing();
	// if length==0, s is assumed to be a 0-terminated string
	int sendRawTCP( char *s, int length = 0 );
	int sendCharacter( CreatureInfo *info );

	inline void setGameStateHandler( GameStateHandler *gsh ) {
		this->gsh = gsh;
	}
	inline GameStateHandler *getGameStateHandler() {
		return gsh;
	}
	inline Commands *getCommands() {
		return commands;
	}
	inline void setTryToReconnect( bool b ) {
		tryToReconnect = b;
	}
	inline bool getTryToReconnect() {
		return tryToReconnect;
	}

	inline bool isConnected() {
		return connected;
	}
	inline bool isThreadRunning() {
		return threadRunning;
	}
	inline void setThreadRunning( bool b ) {
		threadRunning = b;
	}
	inline TCPsocket getTCPSocket() {
		return tcpSocket;
	}
	inline void setReadError( bool b ) {
		readError = b;
	}
	inline Broadcast *getBroadcast() {
		return broadcast;
	}

protected:
	int openConnection();
	void closeConnection();
	int initTCPSocket();
};

#endif

#endif
