#ifndef TCP_UTIL_H
#define TCP_UTIL_H

#include "../constants.h"

class TCPUtil {
 private:
  static char *buffer;

 public:
  // receive a buffer from a TCP socket with error checking
  // this function handles the memory, so it can't use any [] arrays
  // returns 0 on any errors, or a valid char* on success
  static char *receive(TCPsocket sock, char **buf);

  // send a string buffer over a TCP socket with error checking
  // returns 0 on any errors, length sent on success
  static int send(TCPsocket sock, char *buf);
};

#endif
