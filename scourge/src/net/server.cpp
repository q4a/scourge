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

  this->port = port;

  // open udp server socket
  if(!(socket = SDLNet_UDP_Open( port ))) {
    cerr << "SDLNet_UDP_Open: " << SDLNet_GetError() << endl;
    exit(4);
  }

  // allocate max packet
  if(!(out = SDLNet_AllocPacket(PACKET_LENGTH))) {
    cerr << "SDLNet_AllocPacket: (out) %s\n" << SDLNet_GetError() << endl;
    exit(5);
  }
  if(!(in = SDLNet_AllocPacket(PACKET_LENGTH))) {
    cerr << "SDLNet_AllocPacket: (in) %s\n" << SDLNet_GetError() << endl;
    exit(5);
  }
  
  // start server thread
  stopServerThread = false;
  thread = SDL_CreateThread(serverLoop, this);
}

Server::~Server() {
  cerr << "Stopping server..." << endl;
  
  // close the socket
  SDLNet_UDP_Close(socket);
  
  // free packet
  SDLNet_FreePacket(out);
  SDLNet_FreePacket(in);

  // kill the server thread
  if(thread) {
    cerr << "Waiting for server thread to stop..." << endl;
    int status;
    stopServerThread = true; // should this be synchronized?
    SDL_WaitThread(thread, &status);
    thread = NULL;
    stopServerThread = false;
    cerr << "Server thread to stopped." << endl;
  }

  // mark server stopped
  port = -1;
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

  cerr << "Server listening on port " << server->getPort() << endl;
  while(!server->getStopServerThread()) {

    // unbind so requests can come from any address
    SDLNet_UDP_Unbind(server->getSocket(), 0);
    server->getInPacket()->data[0] = 0;
    cerr << "Waiting..." << endl;

    // wait until we get a request
    while(!SDLNet_UDP_Recv(server->getSocket(), 
                           server->getInPacket())) SDL_Delay(100); //1/10th of a second

    // check that first byte is for us
    if(server->getInPacket()->data[0] != 1<<4) {
      server->getInPacket()->data[0] = ERROR;
      server->getInPacket()->len = 1;
      SDLNet_UDP_Send(server->getSocket(), -1, server->getInPacket());
      continue; // not a request...
    }

    // create a thread to deal with this request
    IPaddress ip;
    memcpy(&ip, &(server->getInPacket()->address), sizeof(IPaddress));
    const char *host = SDLNet_ResolveIP(&ip);
    Uint32 ipnum = SDL_SwapBE32(ip.host);
    Uint16 port=SDL_SwapBE16(ip.port);
    if(host)
      cerr << "request from host=" << host << " port=" << port << endl;
    else
      cerr << "request from host=" << (ipnum>>24) << "." << ((ipnum>>16)&0xff) << "." << 
        ((ipnum>>8)&0xff) << "." << (ipnum&0xff) << " port=" << port << endl;

    // bind client to channel (cbannel used by SDLNet_UDP_Send)
    if(SDLNet_UDP_Bind(server->getSocket(), 0, &ip)==-1) {
      cerr << "SDLNet_UDP_Bind: " << SDLNet_GetError() << endl;
      exit(7);
    }
    
    // get text
    strcpy(text, (const char*)(server->getInPacket()->data + 1));
    cerr << "text=" << text << endl;

    // FIXME: encapsulate command processing in command factory (shared with client)
		
    // send some bogus data for now
    server->getOutPacket()->data[0] = 1; // response code checked in UDPUtil
    SDLNet_Write32(12, server->getOutPacket()->data + 1); // actual data
    server->getOutPacket()->len = 5;

    // send response to all addresses on channel 0
    // FIXME: this will need to change... login should only respond to new user 
    // (maybe not, it's ok to broadcast new user id.)
    if(UDPUtil::udpsend(server->getSocket(), 0, 
                        server->getOutPacket(), 
                        server->getInPacket(), 10, 
                        0, // 0=no response wanted
                        TIMEOUT) < 1)
      continue;
  }

  return 0;
}

#endif


