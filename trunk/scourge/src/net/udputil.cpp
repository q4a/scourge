
/***************************************************************************
                          udputil.cpp  -  description
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
#include "udputil.h"

int UDPUtil::udpsend(UDPsocket sock, int channel, 
                     UDPpacket *out, UDPpacket *in, 
                     Uint32 delay, Uint8 expect, 
                     int timeout) {
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
  } while(!err || (expect && !(in->data[0] == expect || in->data[0] == ERROR)));
  if(in->data[0]==ERROR)
    cerr << "udpsend: received error code=" << (int)(in->data[0]) << endl;
  return(in->data[0]==ERROR?-1:1);
}           
              
int UDPUtil::udprecv(UDPsocket sock, UDPpacket *in, 
                     Uint32 delay, Uint8 expect, 
                     int timeout) {
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
    cerr << "udprecv: received error code=" << (int)(in->data[0]) << endl;
  return(in->data[0]==ERROR?-1:1);
}       
  
#endif

