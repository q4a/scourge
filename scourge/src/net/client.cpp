/***************************************************************************
                          client.cpp  -  description
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
 
#ifdef HAVE_SDL_NET 
#include "client.h" 
 
Client::Client(char *server, int port, char *name) {
  clientServerName = server;
  clientPort = port;
  clientUserName = name;
  clientSocket = NULL;
  clientOut = NULL;
  clientIn = NULL;
  clientId = 0;
}

Client::~Client() {
  logout();
}
 
Uint32 Client::login() {
  if(SDLNet_ResolveHost(&clientServerIP, clientServerName, clientPort)==-1) {
    printf("SDLNet_ResolveHost: %s\n", SDLNet_GetError());
    exit(4);
  }
  
  // open udp client socket
  if(!(clientSocket = SDLNet_UDP_Open(0))) {
    printf("SDLNet_UDP_Open: %s\n", SDLNet_GetError());
    exit(5);
  }

	// allocate max packet
	if(!(clientOut = SDLNet_AllocPacket(65535))) {
    printf("SDLNet_AllocPacket: %s\n", SDLNet_GetError());
    exit(6);
	}
	if(!(clientIn = SDLNet_AllocPacket(65535))) {
    printf("SDLNet_AllocPacket: %s\n",SDLNet_GetError());
    exit(6);
  }
  
	// bind server address to channel 0
  if(SDLNet_UDP_Bind(clientSocket, 0, &clientServerIP)==-1) {
		printf("SDLNet_UDP_Bind: %s\n",SDLNet_GetError());
    exit(7);
  }
  cerr << "Client is connected to " << clientServerName << ":" << clientPort << endl;

  // send login request
	cerr << "Client is logging in as " << clientUserName << endl;
	
  // FIXME: encapsulate in command factory
  clientOut->data[0] = 1 << 4;
  sprintf((char*)(clientOut->data + 1), "LOGIN,%s", clientUserName);
	clientOut->len = 1 + 6 + strlen(clientUserName) + 1;

	if(UDPUtil::udpsend(clientSocket, 0, clientOut, clientIn, 200, 1, TIMEOUT) < 1) exit(9);
	
	clientId = SDLNet_Read32(clientIn->data + 1);
	cerr << "Server sent userid: " << clientId << endl;
  
  return clientId;
}

void Client::logout() {
  if(clientId == 0) return;

	// close the socket
	SDLNet_UDP_Close(clientSocket);
	
	// free packets
	SDLNet_FreePacket(clientOut);
	SDLNet_FreePacket(clientIn);

  clientId = 0;
}

void Client::sendChat(char *message) {
}

#endif
