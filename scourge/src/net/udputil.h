
/***************************************************************************
                          udputil.h  -  description
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

#ifndef UDP_UTIL_H
#define UDP_UTIL_H

#include "../constants.h"

#ifdef HAVE_SDL_NET
#include <SDL_net.h>
#include <SDL_thread.h>

#define ERROR (0xff)
#define TIMEOUT (5000) //five seconds
#define PACKET_LENGTH 65535

class UDPUtil {

  public:

static int udpsend(UDPsocket sock, int channel, 
                   UDPpacket *out, UDPpacket *in, 
                   Uint32 delay, Uint8 expect, 
                   int timeout);
static int udprecv(UDPsocket sock, UDPpacket *in, 
                   Uint32 delay, Uint8 expect, 
                   int timeout);
};

#endif

#endif

