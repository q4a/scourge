#ifdef HAVE_SDL_NET

#ifndef CLIENT_H
#define CLIENT_H

#include "../constants.h"
#include "tcputil.h"
#include "gamestatehandler.h"
#include "testgamestatehandler.h"
#include "broadcast.h"
#include "commands.h"

int clientLoop(void *data);

class Client {
 private:
  char *host;
  int port;
  char *username;
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
  Client(char *host, int port, char *username, CommandInterpreter *ci);
  virtual ~Client();
  int connect();
  bool findServer();
  int login();
  int sendChatTCP(char *message);
  int sendPing();
  int sendRawTCP(char *s);

  inline void setGameStateHandler(GameStateHandler *gsh) { this->gsh = gsh; }
  inline GameStateHandler *getGameStateHandler() { return gsh; }
  inline Commands *getCommands() { return commands; }
  inline void setTryToReconnect(bool b) { tryToReconnect = b; }
  inline bool getTryToReconnect() { return tryToReconnect; }

  inline bool isConnected() { return connected; }
  inline bool isThreadRunning() { return threadRunning; }
  inline void setThreadRunning(bool b) { threadRunning = b; }
  inline TCPsocket getTCPSocket() { return tcpSocket; }
  inline void setReadError(bool b) { readError = b; }
  inline Broadcast *getBroadcast() { return broadcast; }

 protected:
  int openConnection();
  void closeConnection();
  int initTCPSocket();
};

#endif

#endif
