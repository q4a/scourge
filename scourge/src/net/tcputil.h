#ifdef HAVE_SDL_NET

#ifndef TCP_UTIL_H
#define TCP_UTIL_H

#include "../constants.h"

class TCPUtil {

 public:
  // receive a buffer from a TCP socket with error checking
  // this function handles the memory, so it can't use any [] arrays
  // returns 0 on any errors, or a valid char* on success
  //
  // if length==NULL, then no length is returned. (Assumes buf is a 0-terminated string)
  static char *receive(TCPsocket sock, char **buf, int *length=NULL);

  // send a string buffer over a TCP socket with error checking
  // returns 0 on any errors, length sent on success
  //
  // if length==0, buf is assumed to be a 0 terminated string
  static int send(TCPsocket sock, char *buf, int length=0);
};

#endif

#endif
