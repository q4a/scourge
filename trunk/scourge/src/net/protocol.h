/***************************************************************************
                          protocol.h  -  description
                             -------------------
    begin                : Sun Sep 28 2003
    copyright            : (C) 2003 by Gabor Torok
    email                : cctorok@yahoo.com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef PROTOCOL_H
#define PROTOCOL_H

#include "../constants.h"
#include "../scourge.h"

#ifdef HAVE_SDL_NET
#include <SDL_net.h>
#include <SDL_thread.h>

/**
   Main server loop
   @param data a pointer to a Protocol object.
 */
int serverLoop(void *data);
int udpsend(UDPsocket sock, int channel, UDPpacket *out, UDPpacket *in, Uint32 delay, Uint8 expect, int timeout);
int udprecv(UDPsocket sock, UDPpacket *in, Uint32 delay, Uint8 expect, int timeout);

#endif

/**
 *@author Gabor Torok
 */
class Protocol {
private:
  Scourge *scourge;

#ifdef HAVE_SDL_NET

  // server code
  static const int DEFAULT_SERVER_PORT = 6543;
  int serverPort;
  UDPsocket serverSocket;
  UDPpacket *serverOut, *serverIn;
  SDL_Thread *serverThread;
  bool stopServerThread;


  // client code
  char clientServerName[40];
  char clientUserName[40];
  int clientPort;
  UDPsocket clientSocket;
  UDPpacket *clientOut, *clientIn;
  IPaddress clientServerIP;
  Uint32 clientId;
#endif

 public:
   static const char *localhost;
   static const char *adminUserName;

  Protocol(Scourge *scourge);
  ~Protocol(); 
#ifdef HAVE_SDL_NET
  int startServer(int port=DEFAULT_SERVER_PORT);
  void stopServer();
  Uint32 login(char *server, int port, char *name);
  void logout();
  void sendChat(char *message);

  inline int getServerPort() { return serverPort; }
  inline UDPpacket *getServerOutPacket() { return serverOut; }
  inline UDPpacket *getServerInPacket() { return serverIn; }
  inline UDPsocket getServerSocket() { return serverSocket; }
  inline bool getStopServerThread() { return stopServerThread; }
#endif
};

#endif
