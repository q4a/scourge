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

#ifdef HAVE_SDL_NET

#include "../constants.h"
#include "../scourge.h"
#include <SDL_net.h>
#include <SDL_thread.h>

/**
   Main server loop
   @param data a pointer to a Protocol object.
 */
int serverLoop(void *data);

/**
 *@author Gabor Torok
 */
class Protocol {
 private:
  Scourge *scourge;
  static const int DEFAULT_SERVER_PORT = 6543;
  int serverPort;
  UDPsocket serverSocket;
  UDPpacket *serverOut, *serverIn;
  SDL_Thread *serverThread;
  bool stopServerThread;

 public:
  Protocol(Scourge *scourge);
  ~Protocol(); 

  void startServer(int port=DEFAULT_SERVER_PORT);
  void stopServer();
  int login(char *server, int port, char *name);
  void sendChat(char *message);

  inline int getServerPort() { return serverPort; }
  inline UDPpacket *getServerOutPacket() { return serverOut; }
  inline UDPpacket *getServerInPacket() { return serverIn; }
  inline UDPsocket getServerSocket() { return serverSocket; }
  inline bool getStopServerThread() { return stopServerThread; }
};

#endif

#endif
