#ifdef HAVE_SDL_NET
#ifndef BROADCAST_H
#define BROADCAST_H

#include "../constants.h"

// since broadcasting doesn't appear to work in SDL_net, this is disabled
#define ENABLE_BROADCASTING 0

class Broadcast {
 private:

  // UDP broadcast
  static const Uint16 udpPortServer = 9999;
  static const Uint16 udpPortClient = 9998;
  IPaddress broadcastIP;
  UDPsocket udpSocket;
  UDPpacket *udpPacket;
  Uint32 lastBroadcastTime;
  static const Uint32 BROADCAST_DELAY = 1000;
  static const Uint32 FIND_SERVER_DELAY = 200;
  int serverPort;

 public:
  Broadcast(int serverPort=-1);  
  ~Broadcast();
  void broadcast();
  bool listen(IPaddress *ip, Uint32 timeout);

 protected:
  void initBroadcastSocket();
};

#endif
#endif
