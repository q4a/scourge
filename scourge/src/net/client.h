/***************************************************************************
                          client.h  -  description
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

#ifndef CLIENT_H
#define CLIENT_H

#include "../constants.h"

#ifdef HAVE_SDL_NET
  #include <SDL_net.h>
  #include <SDL_thread.h>
  #include "udputil.h"

/**
 *@author Gabor Torok
 */
class Client {
private:
  char *clientServerName;
  char *clientUserName;
  int clientPort;
  UDPsocket clientSocket;
  UDPpacket *clientOut, *clientIn;
  IPaddress clientServerIP;
  Uint32 clientId;

public:
  Client(char *server, int port, char *name);
  ~Client(); 
  Uint32 login();
  void logout();
  void sendChat(char *message);

  inline Uint32 getUserId() { return clientId; }
};              

#endif
#endif

