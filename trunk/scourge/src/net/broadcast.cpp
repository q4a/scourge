#include "broadcast.h"

#define SIGNAL_PREFIX "SCOURGE,"

Broadcast::Broadcast(int serverPort) {
  this->serverPort = serverPort;
  lastBroadcastTime = 0;
  initBroadcastSocket();
}

Broadcast::~Broadcast() {
  // free the broadcast stuff
  SDLNet_UDP_Close(udpSocket);
  SDLNet_FreePacket(udpPacket);
}

void Broadcast::initBroadcastSocket() {
  if(serverPort > -1) {
    // server mode
    broadcastIP.host = INADDR_BROADCAST;
    broadcastIP.port = SDL_SwapBE16(udpPortServer);

    if(!(udpSocket = SDLNet_UDP_Open(udpPortServer))) {
      cerr << "*** error: Failed to open UDP socket: " << SDLNet_GetError() << endl;
      exit(6);
    }

  } else {
    // client mode
    broadcastIP.host = INADDR_ANY;
    broadcastIP.port = SDL_SwapBE16(udpPortServer);

    if(!(udpSocket = SDLNet_UDP_Open(0))) {
      cerr << "*** error: Failed to open UDP socket: " << SDLNet_GetError() << endl;
      exit(6);
    }

    if(SDLNet_UDP_Bind(udpSocket, -1, &broadcastIP) < 0) {
      cerr << "*** error: Failed to bind UDP socket: " << SDLNet_GetError() << endl;
      exit(6);
    }

  }

  if(!(udpPacket = SDLNet_AllocPacket(1024))) {
    cerr << "*** error: Failed to allocate UDP packet: " << SDLNet_GetError() << endl;
    exit(6);
  }

  lastBroadcastTime = 0;
}

// FIXME: use Commands to create message?
void Broadcast::broadcast() {
#if ENABLE_BROADCASTING
  Uint32 t = SDL_GetTicks();
  if(t - lastBroadcastTime >= BROADCAST_DELAY) {
    sprintf((char*)(udpPacket->data), "%s%d", SIGNAL_PREFIX, serverPort);
    udpPacket->len = strlen((char*)(udpPacket->data)) + 1;
    cerr << "Sending broadcast signal: " << (char*)(udpPacket->data) << endl;

    // don't use channels, send to this address:
    udpPacket->address.host = INADDR_BROADCAST;
    udpPacket->address.port = SDL_SwapBE16(udpPortServer);
    if(SDLNet_UDP_Send(udpSocket, -1, udpPacket) <= 0) {
      cerr << "*** error: couldn't send broadcast packet!" << SDLNet_GetError() << endl;
    }
    lastBroadcastTime = t;
  }
#endif
}

/**
   @return true if data was read.
 */
bool Broadcast::listen(IPaddress *ip, Uint32 timeout) {
  bool ret = false;
#if ENABLE_BROADCASTING
  Uint32 startTime = SDL_GetTicks();
  Uint32 now;
  SDLNet_SocketSet set;
  if(!(set = SDLNet_AllocSocketSet(1))) {
    cerr << "* Error allocating socket set: " << SDLNet_GetError() << endl;
    exit(1);
  }
  if(SDLNet_UDP_AddSocket(set, udpSocket) == -1) {
    cerr << "* Error adding socket to set: " << SDLNet_GetError() << endl;
    exit(1);
  }
  do {    
    cerr << "Looking for server..." << endl;
    int numReady = SDLNet_CheckSockets(set, FIND_SERVER_DELAY);
    if(numReady == -1) {
      cerr << "* Error checking sockets: " << SDLNet_GetError() << endl;
      break;
    } else if(numReady) {
      cerr << "\tFound something..." << endl;
      if(SDLNet_UDP_Recv(udpSocket, udpPacket) > 0) {
        int n = strlen(SIGNAL_PREFIX);
        if(udpPacket->len > n && 
           !strncmp((char*)udpPacket->data, SIGNAL_PREFIX, n)) {
          ip->host = udpPacket->address.host;
          ip->port = atoi((char*)udpPacket->data + n);
          cerr << "\tFound it. host=" << ip->host << " port=" << ip->port << endl;
          ret = true;
          break;
        }
      } 
    }
    now = SDL_GetTicks();
  } while(now - startTime < timeout);
  SDLNet_FreeSocketSet(set);
#endif
  return ret;  
}
