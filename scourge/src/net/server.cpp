/***************************************************************************
                          server.cpp -  description
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
#include "server.h"

Server::Server(int port) {
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
  stopServerThread = false;
  serverThread = SDL_CreateThread(serverLoop, this);
}

Server::~Server() {
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


/*
 * FIXME: implement this as a slot server:
 * When logging in, client is bound to a channel. 
 * (On login look thru channel for empty slot.)
 * Each connected user has a chanel, so server doesn't have to bind/unbind 
 * Reuse same in/out packets among channels?
 * Note: SDLNet_UDP_Bind can bind multiple addresses to a channel.
*/
int serverLoop(void *data) {

  Server *server = (Server*)data;
  char text[PACKET_LENGTH];

  cerr << "Server listening on port " << server->getServerPort() << endl;
  while(!server->getStopServerThread()) {

    // unbind so requests can come from any address
    SDLNet_UDP_Unbind(server->getServerSocket(), 0);
    server->getServerInPacket()->data[0] = 0;
    cerr << "Waiting..." << endl;

    // wait until we get a request
    while(!SDLNet_UDP_Recv(server->getServerSocket(), 
                           server->getServerInPacket())) SDL_Delay(100); //1/10th of a second

    // check that first byte is for us
    if(server->getServerInPacket()->data[0] != 1<<4) {
      server->getServerInPacket()->data[0] = ERROR;
      server->getServerInPacket()->len = 1;
      SDLNet_UDP_Send(server->getServerSocket(), -1, server->getServerInPacket());
      continue; // not a request...
    }

    IPaddress ip;
    memcpy(&ip, &(server->getServerInPacket()->address), sizeof(IPaddress));
    const char *host = SDLNet_ResolveIP(&ip);
    Uint32 ipnum = SDL_SwapBE32(ip.host);
    Uint16 port=SDL_SwapBE16(ip.port);
    if(host)
      cerr << "request from host=" << host << " port=" << port << endl;
    else
      cerr << "request from host=" << (ipnum>>24) << "." << ((ipnum>>16)&0xff) << "." << 
        ((ipnum>>8)&0xff) << "." << (ipnum&0xff) << " port=" << port << endl;

    // bind client to channel (cbannel used by SDLNet_UDP_Send)
    if(SDLNet_UDP_Bind(server->getServerSocket(), 0, &ip)==-1) {
      cerr << "SDLNet_UDP_Bind: " << SDLNet_GetError() << endl;
      exit(7);
    }
    
    // get text
    strcpy(text, (const char*)(server->getServerInPacket()->data + 1));
    cerr << "text=" << text << endl;

    // FIXME: encapsulate command processing in command factory (shared with client)
		
    // send some bogus data for now
    server->getServerOutPacket()->data[0] = 1; // response code checked in UDPUtil
    SDLNet_Write32(12, server->getServerOutPacket()->data + 1); // actual data
    server->getServerOutPacket()->len = 5;

    // send response to all addresses on channel 0
    // FIXME: this will need to change... login should only respond to new user 
    // (maybe not, it's ok to broadcast new user id.)
    if(UDPUtil::udpsend(server->getServerSocket(), 0, 
                        server->getServerOutPacket(), 
                        server->getServerInPacket(), 10, 
                        0, // 0=no response wanted
                        TIMEOUT) < 1)
      continue;
  }

  return 0;
}

#endif


