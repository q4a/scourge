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

#include "protocol.h"

#define ERROR (0xff)
#define TIMEOUT (5000) //five seconds

const char *Protocol::localhost = "localhost";
const char *Protocol::adminUserName = "admin";

#ifdef HAVE_SDL_NET

#define PACKET_LENGTH 65535

/**
 *@author Gabor Torok
 */
Protocol::Protocol(Scourge *scourge) {
  this->scourge = scourge;
  this->serverPort = -1;
  this->serverThread = NULL;
  this->stopServerThread = false;

  strcpy(clientServerName, "");
  strcpy(clientUserName, "");
  clientPort = -1;
  clientId = 0;

  // initialize SDL_net
  if(SDLNet_Init()==-1) {
    printf("SDLNet_Init: %s\n",SDLNet_GetError());
    exit(2);
  }  
}

Protocol::~Protocol() {
  stopServer();
  logout();
  // shutdown SDL_net
  SDLNet_Quit();
}

int Protocol::startServer(int port) {
  if(serverPort > -1) {
    cerr << "*** Warning: attempting to start server when it's already running." << endl;
    return serverPort;
  }
  cerr << "Starting server..." << endl;

  serverPort = port;

  // open udp server socket
  if(!(serverSocket = SDLNet_UDP_Open( serverPort ))) {
    cerr << "SDLNet_UDP_Open: " << SDLNet_GetError() << endl;
    exit(4);
  }

  // allocate max packet
  if(!(serverOut = SDLNet_AllocPacket(PACKET_LENGTH))) {
    cerr << "SDLNet_AllocPacket: (out) %s\n" << SDLNet_GetError() << endl;
    exit(5);
  }
  if(!(serverIn = SDLNet_AllocPacket(PACKET_LENGTH))) {
    cerr << "SDLNet_AllocPacket: (in) %s\n" << SDLNet_GetError() << endl;
    exit(5);
  }
  
  // start server thread
  serverThread = SDL_CreateThread(serverLoop, this);

  SDL_Delay(2000);

  return serverPort;
}

void Protocol::stopServer() {
  if(serverPort == -1) return;
  cerr << "Stopping server..." << endl;
  
  // close the socket
  SDLNet_UDP_Close(serverSocket);
  
  // free packet
  SDLNet_FreePacket(serverOut);
  SDLNet_FreePacket(serverIn);

  // kill the server thread
  if(serverThread) {
    cerr << "Waiting for server thread to stop..." << endl;
    int status;
    stopServerThread = true; // should this be synchronized?
    SDL_WaitThread(serverThread, &status);
    serverThread = NULL;
    stopServerThread = false;
    cerr << "Server thread to stopped." << endl;
  }

  // mark server stopped
  serverPort = -1;
  cerr << "Server stopped." << endl;
}

Uint32 Protocol::login(char *server, int port, char *name) {
  strcpy(clientServerName, server);
  strcpy(clientUserName, name);
  clientPort = port;

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
	clientOut->data[0] = 1<<4;
	//strcpy(clientOut->data + 1, clientUserName);
  sprintf((char*)(clientOut->data + 1), "LOGIN,%s", clientUserName);
	clientOut->len = 1 + 6 + strlen(clientUserName) + 1;
	if(udpsend(clientSocket, 0, clientOut, clientIn, 200, 1, TIMEOUT) < 1) exit(9);
	
	clientId = SDLNet_Read32(clientIn->data + 1);
	cerr << "Server sent userid: " << clientId << endl;
  
  return clientId;
}

void Protocol::logout() {
  if(clientId == 0) return;

	// close the socket
	SDLNet_UDP_Close(clientSocket);
	
	// free packets
	SDLNet_FreePacket(clientOut);
	SDLNet_FreePacket(clientIn);

  clientId = 0;
}

void Protocol::sendChat(char *message) {
}



















int udpsend(UDPsocket sock, int channel, UDPpacket *out, UDPpacket *in, Uint32 delay, Uint8 expect, int timeout) {
  Uint32 t,t2;
  int err;
  
  in->data[0]=0;
  t=SDL_GetTicks();
  do {
    t2=SDL_GetTicks();
    if(t2-t>(Uint32)timeout) {
      printf("timed out\n");
      return(0);
    }
    if(!SDLNet_UDP_Send(sock, channel, out)) {
      printf("SDLNet_UDP_Send: %s\n",SDLNet_GetError());
      exit(1);
    }
    err=SDLNet_UDP_Recv(sock, in);
    if(!err)
      SDL_Delay(delay);
  } while(!err || (in->data[0]!=expect && in->data[0]!=ERROR));
  if(in->data[0]==ERROR)
    printf("received error code\n");
  return(in->data[0]==ERROR?-1:1);
}

int udprecv(UDPsocket sock, UDPpacket *in, Uint32 delay, Uint8 expect, int timeout) {
  Uint32 t,t2;
  int err;
  
  in->data[0]=0;
  t=SDL_GetTicks();
  do {
    t2=SDL_GetTicks();
    if(t2-t>(Uint32)timeout) {
      printf("timed out\n");
      return(0);
    }
    err=SDLNet_UDP_Recv(sock, in);
    if(!err) SDL_Delay(delay);
  } while(!err || (in->data[0]!=expect && in->data[0]!=ERROR));
  if(in->data[0]==ERROR)
    printf("received error code\n");
  return(in->data[0]==ERROR?-1:1);
}

int serverLoop(void *data) {

  Protocol *protocol = (Protocol*)data;
  char text[PACKET_LENGTH];

  cerr << "Server listening on port " << protocol->getServerPort() << endl;
  while(!protocol->getStopServerThread()) {

    // close previous connection?
    SDLNet_UDP_Unbind(protocol->getServerSocket(), 0);
    protocol->getServerInPacket()->data[0] = 0;
    cerr << "Waiting..." << endl;

    while(!SDLNet_UDP_Recv(protocol->getServerSocket(), 
                           protocol->getServerInPacket())) SDL_Delay(100); //1/10th of a second

    if(protocol->getServerInPacket()->data[0] != 1<<4) {
      protocol->getServerInPacket()->data[0] = ERROR;
      protocol->getServerInPacket()->len = 1;
      SDLNet_UDP_Send(protocol->getServerSocket(), -1, protocol->getServerInPacket());
      continue; // not a request...
    }

    IPaddress ip;
    memcpy(&ip, &(protocol->getServerInPacket()->address), sizeof(IPaddress));
    const char *host = SDLNet_ResolveIP(&ip);
    Uint32 ipnum = SDL_SwapBE32(ip.host);
    Uint16 port=SDL_SwapBE16(ip.port);
    if(host)
      cerr << "request from host=" << host << " port=" << port << endl;
    else
      cerr << "request from host=" << (ipnum>>24) << "." << ((ipnum>>16)&0xff) << "." << 
        ((ipnum>>8)&0xff) << "." << (ipnum&0xff) << " port=" << port << endl;

    // bind client to channel
    if(SDLNet_UDP_Bind(protocol->getServerSocket(), 0, &ip)==-1) {
      cerr << "SDLNet_UDP_Bind: " << SDLNet_GetError() << endl;
      exit(7);
    }
    
    // get text
    strcpy(text, (const char*)(protocol->getServerInPacket()->data + 1));
    cerr << "text=" << text << endl;
		
    // send some bogus data for now
    protocol->getServerOutPacket()->data[0] = 1;
    SDLNet_Write32(12, protocol->getServerOutPacket()->data+1);
    protocol->getServerOutPacket()->len=5;
    if(udpsend(protocol->getServerSocket(), 0, 
               protocol->getServerOutPacket(), 
               protocol->getServerInPacket(), 10, 2<<4, TIMEOUT) < 1)
      continue;
    
  }

  return 0;
}

#else

Protocol::Protocol(Scourge *scourge) {
  this->scourge = scourge;
}

Protocol::~Protocol() {
}

#endif
