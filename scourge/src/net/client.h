#ifndef CLIENT_H
#define CLIENT_H

#include <SDL.h>
#include <SDL_net.h>
#include <SDL_thread.h>
#include <iostream.h>
#include <string>
#include "tcputil.h"
#include "gamestatehandler.h"
#include "testgamestatehandler.h"
#include "broadcast.h"

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

  static const Uint32 FIND_SERVER_TIMEOUT = 10000;

 public:
  Client(char *host, int port, char *username);
  virtual ~Client();
  int connect();
  bool findServer();
  int login();
  int sendChatTCP(char *message);
  int sendPing();
  int sendRawTCP(char *s);

  inline void setGameStateHandler(GameStateHandler *gsh) { this->gsh = gsh; }
  inline GameStateHandler *getGameStateHandler() { return gsh; }

  inline bool isConnected() { return connected; }
  inline bool isThreadRunning() { return threadRunning; }
  inline void setThreadRunning(bool b) { threadRunning = b; }
  inline TCPsocket getTCPSocket() { return tcpSocket; }
  inline void setReadError(bool b) { readError = b; }
  inline Broadcast *getBroadcast() { return broadcast; }

  void processGameState(char *str);

 protected:
  int openConnection();
  void closeConnection();
  int initTCPSocket();
};

#endif
