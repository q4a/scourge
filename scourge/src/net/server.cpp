#ifdef HAVE_SDL_NET

#include "server.h"

Server::Server(int port) {
  this->port = port;
  this->clientCount = 0;
  this->gsh = NULL;
  this->currentFrame = 1;

  initTCPSocket();
  broadcast = new Broadcast(port);
  
  // create a socket set
  set = SDLNet_AllocSocketSet(5);
  if(!set) {
    cerr << "*** error: SDLNet_AllocSocketSet: " << SDLNet_GetError() << endl;
    exit(1); //most of the time this is a major error, but do what you want.
  }  

  // add server sockets
  SDLNet_TCP_AddSocket(set, tcpSocket);

  stopThread = false;
  thread = SDL_CreateThread( serverLoop, this );
}

void Server::initTCPSocket() {
  // Resolve the argument into an IPaddress type
  if(SDLNet_ResolveHost(&ip, NULL, port) == -1) {
    cerr << "*** error: SDLNet_ResolveHost: " << SDLNet_GetError() << endl;
    exit(3);
  }

  // perform a byte endianess correction for the next printf
  // FIXME: will this work on PPC?
  Uint32 ipaddr = SDL_SwapBE32( ip.host );

  // output the IP address nicely
  fprintf(stderr, "IP Address : %d.%d.%d.%d\n",
          ipaddr>>24, (ipaddr>>16)&0xff, (ipaddr>>8)&0xff, ipaddr&0xff);

  // resolve the hostname for the IPaddress
  const char *host = SDLNet_ResolveIP(&ip);

  // print out the hostname we got
  if(host) cerr << "Hostname   : " << host << endl;
  else cerr << "Hostname   : N/A" << endl;

  // output the port number
  cerr << "Port   : " << port << endl;

  // create a tcp serversocket
  tcpSocket = SDLNet_TCP_Open( &ip );
  if(!tcpSocket) {
    cerr << "*** error: SDLNet_TCP_Open: " << SDLNet_GetError() << endl;
    exit(5);
  }
}

Server::~Server() {
  SDLNet_FreeSocketSet(set);
  // close the socket
  SDLNet_TCP_Close(tcpSocket);
  delete broadcast;
}

void Server::initTCPConnection(TCPsocket socket) {
  char *text = NULL;
  if(TCPUtil::receive(socket, &text)) {
    //    cerr << "Server received: " << text << endl;

    // FIXME: if there are more commands like LOGIN, handle them thru another interface
    // via the Commands class.

    // add a new client if space is available
    if(strstr(text, "LOGIN,") == text && clientCount < MAX_CLIENT_COUNT) {
      char *username = strdup(text + 6);
      int id = clientCount;
      clients[clientCount] = new ClientInfo(this, socket, id, username);
      SDLNet_TCP_AddSocket(set, socket);
      cerr << "* New user logged in: id=" << clients[clientCount]->describe() << endl;
      clientCount++;
    } else {
      // not login command or out of client space
      SDLNet_TCP_Close(socket);
      cerr << "Server closed client conn: did not send login, or out of client slots." << endl;
    }
  } else {
    // no incoming data
    SDLNet_TCP_Close(socket);
    cerr << "Server closed client conn: no incoming data." << endl;
  }
}

void Server::removeDeadClients() {
  for(int i = 0; i < clientCount; i++) {
    if(clients[i]->dead) {
      SDLNet_TCP_DelSocket(set, clients[i]->socket);
      cerr << "* Removing dead client: id=" << clients[i]->describe() << endl;
      for(int t = i; t < clientCount - 1; t++) {
        clients[t] = clients[t + 1];
        clients[t]->setId(clients[t]->getId() - 1);
      }
      clientCount--;
      i--;
    }
  }
}

void Server::sendToAllTCP(char *message) {
  //  cerr << "* Sending message: " << message << endl;
  for(int i = 0; i < clientCount; i++) {
    //    cerr << "\tto: " << clients[i]->describe() << endl;
    if(!clients[i]->dead)
      clients[i]->sendMessageAsync(message);
  }
}

void Server::sendGameState() {
  if(!gsh) return;
  // reset clients' lag timers
  Uint32 now = SDL_GetTicks();
  for(int i = 0; i < clientCount; i++) {
    if(!clients[i]->dead)
      clients[i]->setLagTimer(currentFrame, now);
  }
  char message[1024];
  sprintf(message, "STATE,%d,%s", currentFrame, gsh->getGameState());
  sendToAllTCP(message);
  currentFrame++;
}

int serverLoop(void *data) {
  Server *server = (Server*)data;
  Uint32 lastTick = 0;
  while(!server->getStopThread()) {

    // wait for a real long time, if there's no activity
    int numReady = SDLNet_CheckSockets(server->getSocketSet(), Server::SERVER_LOOP_DELAY);
    if(numReady == -1) {
      cerr << "*** error: SDLNet_CheckSockets: " << SDLNet_GetError() << endl;
      break;
    }
    // 49+ days passed and nothing to do.
    if(numReady) {

      // activity on the tcp server socket
      if(numReady > 0 && SDLNet_SocketReady(server->getTCPSocket())) {
        numReady--;
        
        // accept a client connection
        TCPsocket socket = SDLNet_TCP_Accept(server->getTCPSocket());
        if(socket) {
          server->initTCPConnection(socket);
        }
      }
      
      //    // activity on the udp server socket
      //    // UDP socket is part of "set"
      //    if(numReady > 0 && SDLNet_SocketReady(server->getUDPSocket())) {
      //      numReady--;
      //      server->handleUDPConnection();
      //    }
      
      // check client tcp sockets
      for(int i = 0; numReady > 0 && i < server->getClientCount(); i++) {
        if(!server->getClient(i)->dead) {
          server->getClient(i)->handleRequestAsync();
        }
      }
    }

    // send game state
    Uint32 t = SDL_GetTicks();
    if(t - lastTick >= Server::SERVER_LOOP_DELAY) {
      lastTick = t;
      server->sendGameState();
    }

    // remove dead clients
    server->removeDeadClients();

    // broadcast UDP signal
    server->getBroadcast()->broadcast();
  }

  return 0;
}

#ifdef SERVER_MAIN
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

  int port;
  if(argc < 2) port = 6543;
  else port = atoi(argv[1]);

  Server *server = new Server(port);
  TestGameStateHandler *tgsh = new TestGameStateHandler();
  server->setGameStateHandler(tgsh);
  int status;
  SDL_WaitThread(server->getThread(), &status);

  delete tgsh;
  delete server;

  // shutdown SDL_net
  SDLNet_Quit();
  
  // shutdown SDL
  SDL_Quit();
  
  return(0);  
}
#endif
#endif
