#ifdef HAVE_SDL_NET

#ifndef SERVER_H
#define SERVER_H

#include "../constants.h"
#include "tcputil.h"
#include "clientinfo.h"
#include "gamestatehandler.h"
#include "testgamestatehandler.h"
#include "broadcast.h"

using namespace std;

int serverLoop(void *data);

class ClientInfo;

class Server {
 private:
  IPaddress ip;
  int port;
  static const int MAX_CLIENT_COUNT = 4;
  SDLNet_SocketSet set;
  bool stopThread;
  TCPsocket tcpSocket;
  SDL_Thread *thread;
  ClientInfo *clients[4];
  int clientCount;
  GameStateHandler *gsh;
  int currentFrame;
  Broadcast *broadcast;

 public:
  static const Uint32 SERVER_LOOP_DELAY = 1000;

  Server(int port);
  virtual ~Server();

  inline void setGameStateHandler(GameStateHandler *gsh) { this->gsh = gsh; }
  inline GameStateHandler *getGameStateHandler() { return gsh; }
  void sendGameState();

  // server params (called by serverLoop)
  inline bool getStopThread() { return stopThread; }
  inline SDL_Thread *getThread() { return thread; }
  inline TCPsocket getTCPSocket() { return tcpSocket; }
  inline SDLNet_SocketSet getSocketSet() { return set; }
  inline int getClientCount() { return clientCount; }
  inline ClientInfo *getClient(int index) { return clients[index]; }

  // handle clients (called by serverLoop)
  void initTCPConnection(TCPsocket socket);
  void handleTCPConnection(int clientIndex);
  void removeDeadClients();

  void sendToAllTCP(char *message);
  inline Broadcast *getBroadcast() { return broadcast; }

 protected:
  void initTCPSocket();
};

#endif

#endif
