#ifdef HAVE_SDL_NET

#ifndef CLIENT_INFO_H
#define CLIENT_INFO_H

#include "../constants.h"
#include "server.h"
#include "tcputil.h"
#include "commands.h"

using namespace std;

int clientInfoLoop(void *data);

class Server;

class Message {
 public:
  char *message;
  Message(char *message);
  ~Message();
};

class ClientInfo : public CommandInterpreter {
 public:
  enum {
    RECEIVE=0,
    SEND
  };

  Server *server;
  bool dead;
  TCPsocket socket;
  int id;
  char *username;
  char desc[80];
  bool threadRunning;
  SDL_Thread *thread;
  SDL_mutex *mutex;
  queue<Message*> messageQueue;
  map<int,Uint32> lagMap;
  Commands *commands;
  
  Uint32 totalLag, lastLagCheck;
  float aveLag;

  // if this many messages are not acknowledged, the client is disconnected
  static const int MAX_SCHEDULED_LAG_MESSAGES = 25;

  ClientInfo(Server *server, TCPsocket socket, int id, char *username);
  virtual ~ClientInfo();
  inline int getId() { return id; }
  inline void setId(int id) { this->id = id; }
  inline char *getUsername() { return username; }
  char *describe();
  void sendMessageAsync(char *s);
  void setLagTimer(int frame, Uint32 n);
  Uint32 updateLag(int frame);

  // commandInterpreter
  void chat(char *message);
  void logout();
  void ping(int frame);
  void handleUnknownMessage();
  void processGameState(int frame, char *p);

  // protected
  void receiveTCP();
  void sendTCP(char *message);
  inline SDL_mutex *getMutex() { return mutex; }
  inline TCPsocket getSocket() { return socket; }
  inline bool isThreadRunning() { return threadRunning; }
  inline queue<Message*> *getMessageQueue() { return &messageQueue; }
  inline Commands *getCommands() { return commands; }
};

#endif
#endif
