/***************************************************************************
                          server.h  -  description
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

#ifndef SERVER_H
#define SERVER_H

#include "../constants.h"

#ifdef HAVE_SDL_NET
  #include <SDL_net.h>
  #include <SDL_thread.h>
  #include "udputil.h"

/**
   Main server loop
   @param data a pointer to a Protocol object.
 */
int serverLoop(void *data);

class Server {
private:
  int port;
  UDPsocket socket;
  UDPpacket *out, *in;
  SDL_Thread *thread;
  bool stopServerThread;

public:
  Server(int port);
  ~Server();

  inline int getPort() { return port;}
  inline UDPpacket *getOutPacket() { return out;}
  inline UDPpacket *getInPacket() { return in;}
  inline UDPsocket getSocket() { return socket;}
  inline bool getStopServerThread() { return stopServerThread;}

};

#endif

#endif

