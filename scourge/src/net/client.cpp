#ifdef HAVE_SDL_NET

#include "client.h"

// 0 or less is retry forever
#define RETRY_COUNT 10

Client::Client(char *host, int port, char *username, CommandInterpreter *ci) {
  this->host = host;
  this->port = port;
  this->username = username;
  this->connected = false;
  this->readError = false;
  this->gsh = NULL;
  this->broadcast = new Broadcast();
  this->commands = new Commands(ci);

  if(host && port) {
    // Resolve the argument into an IPaddress type
    if(SDLNet_ResolveHost(&ip, host, port) == -1) {
      cerr << "*** error: SDLNet_ResolveHost: " << SDLNet_GetError() << endl;
      exit(3);
    }
  }

  // start message receiving thread
  threadRunning = true;
  thread = SDL_CreateThread(clientLoop, this);
  if(!thread) {
    cerr << "* Couldn't create client thread." << endl;
    exit(1);
  }
}

Client::~Client() {
  cerr << "* Stopping client thread..." << endl;
  threadRunning = false;
  int status;
  SDL_WaitThread(thread, &status);
  cerr << "* Closing connection." << endl;
  closeConnection();
  delete broadcast;
}

bool Client::findServer() {
  IPaddress ip;
  if(broadcast->listen(&ip, FIND_SERVER_TIMEOUT)) {
    this->ip.host = ip.host;
    this->ip.port = ip.port;
    return true;
  } 
  return false;
}

int clientLoop(void *data) {
  Client *client = (Client*)data;

  // wait until connected
  while(!client->isConnected() && client->isThreadRunning()) SDL_Delay(200);

  SDLNet_SocketSet set;
  int numready;
  char *str = NULL;
  bool sendPing = false;

  set=SDLNet_AllocSocketSet(1);
  if(client->isThreadRunning() && !set) {
    cerr << "* SDLNet_AllocSocketSet: " << SDLNet_GetError() << endl;
    SDLNet_Quit();
    SDL_Quit();
    client->setThreadRunning(false);
  }

  // process incoming data
  while(client->isThreadRunning()) {

    sendPing = false;

    if(!client->isConnected()) {
      SDL_Delay(2000);
      continue;
    }

    // add the socket to the set, so we can call SDLNet_CheckSockets
    if(SDLNet_TCP_AddSocket(set, client->getTCPSocket()) == -1) {
      cerr << "* SDLNet_TCP_AddSocket: " << SDLNet_GetError() << endl;
      SDLNet_Quit();
      SDL_Quit();
      client->setThreadRunning(false);
      break;
    }

    // wait for a real long time for activity
    numready=SDLNet_CheckSockets(set, (Uint32)1000);
    if(numready==-1) {
      cerr << "SDLNet_CheckSockets: " << SDLNet_GetError() << endl;
      client->setThreadRunning(false);
      break;
    }
    // check to see if the server sent us data
    if(numready && SDLNet_SocketReady(client->getTCPSocket())) {

      if(!TCPUtil::receive(client->getTCPSocket(), &str)) {
        char *error = SDLNet_GetError();
        cerr << "* Error in get message: " << (strlen(error) ? error : "Server disconnected?") << endl;
        client->setReadError(true);
        // next ping will reconnect
        sendPing = true;
      } else {
        
        client->getCommands()->interpret(str);

        // respond to this with a ping
        if(!strncmp(str, "STATE,", 6)) {
          sendPing = true;
        }

        /*
        if(!strncmp(str, "STATE,", 6)) {
          client->processGameState(str);        
          sendPing = true;
        } else {
          // FIXME: call a client interface here
          cout << str << endl;
        }
        */
      }
    }

    // remove socket from set in case there was a failure in the read,
    // or in case there is one in sending the PING below.
    // (in either case a new socket will be created)
    if(SDLNet_TCP_DelSocket(set, client->getTCPSocket()) == -1) {
      cerr << "* SDLNet_TCP_DelSocket: " << SDLNet_GetError() << endl;
      SDLNet_Quit();
      SDL_Quit();
      client->setThreadRunning(false);
      break;
    }
    
    // send a ping
    if(sendPing && !client->sendPing()) {
      cerr << "* Ping failed; quitting." << endl;
      break;
    }
  }

  // free stuff
  SDLNet_FreeSocketSet(set);
  if(str) free(str);

  return 0;
}

int Client::openConnection() {
  if(!initTCPSocket()) return 0;
  connected = true;
  return 1;
}

void Client::closeConnection() {
  if(!connected) return;
  // close the socket
  if(tcpSocket) {
    SDLNet_TCP_Close(tcpSocket);
    tcpSocket = NULL;
  }
  connected = false;
  cerr << "* Disconnected." << endl;
}

int Client::connect() {
  if(connected && !readError) return 1;
  readError = false;
  cerr << "* Connecting to server..." << endl;
  for(int i = 0; RETRY_COUNT <= 0 || i < RETRY_COUNT; i++) {
    closeConnection();
    if(openConnection()) {
      cerr << "\tSuccess." << endl;
      return 1;
    }
    cerr << "\tFailed to connect to server. Retrying..." << endl;
    SDL_Delay(2000);
  }
  cerr << "* Giving up on server." << endl;
  return 0;
}

int Client::initTCPSocket() {
  // open the server socket
  tcpSocket = SDLNet_TCP_Open(&ip);
  if(!tcpSocket) {
    cerr << "*** error: SDLNet_TCP_Open: " << SDLNet_GetError() << endl;
    //    exit(5);
    return 0;
  }
  return 1;
}

int Client::login() {
  char msg[80];
  sprintf(msg, "LOGIN,%s", username);
  return sendRawTCP(msg);
}

int Client::sendChatTCP(char *message) {
  char msg[80];
  sprintf(msg, "CHAT,%s", message);
  return sendRawTCP(msg);
}

int Client::sendPing() {
  char message[80];
  sprintf(message, "PING,%d", commands->getLastGameFrameReceived());
  return sendRawTCP(message);
}

int Client::sendRawTCP(char *s) {
  // try to connect the first time
  if(!connected && !connect()) return 0;

  // send message with retry
  for(int i = 0; RETRY_COUNT <= 0 || i < RETRY_COUNT; i++) {
    if(!TCPUtil::send(tcpSocket, s)) {
      cerr << "* Connection lost to server. Re-logging in..." << endl;
      if(!connect()) {
        cerr << "* Reconnect failed. Giving up on server." << endl;
        return 0;
      }
      login();
    } else {
      return 1;
    }
  }
  cerr << "* Giving up on server." << endl;
  return 0;
}

#ifdef CLIENT_MAIN
int main(int argc, char **argv) {
  // initialize SDL
  if(SDL_Init(0)==-1) {
    cerr << "*** error: SDL_Init: " << SDL_GetError() << endl;
    exit(1);
  }

  // initialize SDL_net
  if(SDLNet_Init()==-1) {
    cerr << "*** error: SDLNet_Init: " << SDL_GetError() << endl;
    exit(2);
  }
  
  char *host, *username;
  int port;
  if(argc < 4) {
    cerr << "Usage: client host port username" << endl;
    //    exit(1);
    username = "gabor";
    host = NULL;
    port = 0;
  } else {
    host = argv[1];
    port = atoi(argv[2]);
    username = argv[3];
  }
  
  Client *client = new Client(host, port, username, new TestCommandInterpreter());

  // find a server
  if(port == 0) {
    if(!client->findServer()) {
      cerr << "* Timeout while trying to find server." << endl;
      delete client;
      exit(1);
    }
  }

  TestGameStateHandler *tgsh = new TestGameStateHandler();
  client->setGameStateHandler(tgsh);
  if(!client->login()) {
    cerr << "* Couldn't connect to server." << endl;
    delete client;
    exit(1);
  }

  char message[80];
  while(true) {
    cout << "> ";
    int c;
    int n = 0;
    while(n < 79 && (c = getchar()) != '\n') message[n++] = c;
    message[n] = 0;
    //    client->sendChatTCP(message);
    client->sendRawTCP(message);
  }

  delete tgsh;
  delete client;

  // shutdown SDL_net
  SDLNet_Quit();
  
  // shutdown SDL
  SDL_Quit();
  
  return(0);  
}
#endif
#endif
