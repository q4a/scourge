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

const char *Protocol::localhost = "localhost";
const char *Protocol::adminUserName = "admin";

#ifdef HAVE_SDL_NET

/**
 *@author Gabor Torok
 */
Protocol::Protocol(Scourge *scourge) {
  this->scourge = scourge;
  this->server = NULL;
  this->client = NULL;
 
  // initialize SDL_net
  if(SDLNet_Init()==-1) {
    printf("SDLNet_Init: %s\n",SDLNet_GetError());
    exit(2);
  }  
}

Protocol::~Protocol() {
  if(server) delete server;
  if(client) delete client;
  // shutdown SDL_net
  SDLNet_Quit();
}

int Protocol::startServer(int port) {
  if(server) {
    cerr << "*** Warning: attempting to start server when it's already running." << endl;
    return port;
  }
  server = new Server(port);
  SDL_Delay(2000);
  return port;
}

void Protocol::stopServer() {
  if(!server) return;
  delete server;
}

Uint32 Protocol::login(char *server, int port, char *name) {
  if(client) {
    cerr << "*** Warning: trying to log in while still connected." << endl;
    return client->getUserId();
  }

  client = new Client(server, port, name);
  return client->login();
}

void Protocol::logout() {
  if(!client) return;
  delete client;
}

void Protocol::sendChat(char *message) {
  if(!client) {
    cerr << "*** Warning: you must log in first." << endl;
  }
  client->sendChat(message);
}

#else

Protocol::Protocol(Scourge *scourge) {
  this->scourge = scourge;
}

Protocol::~Protocol() {
}

#endif
