#ifdef HAVE_SDL_NET
#include "tcputil.h"

using namespace std;

// receive a buffer from a TCP socket with error checking
// this function handles the memory, so it can't use any [] arrays
// returns 0 on any errors, or a valid char* on success
char *TCPUtil::receive(TCPsocket sock, char **buf, int *length) {
  Uint32 len,result;
  
  // free the old buffer
  if(*buf) free(*buf);
  *buf=NULL;

  // receive the length of the string message
  result = SDLNet_TCP_Recv(sock, &len, sizeof(len));
  if(result < sizeof(len)) {
    if(SDLNet_GetError() && strlen(SDLNet_GetError())) // sometimes blank!
      printf("SDLNet_TCP_Recv: %s\n", SDLNet_GetError());
    return(NULL);
  }
	
  // swap byte order to our local order
  len = SDL_SwapBE32(len);
  if(length) *length = len;
	
  // check if anything is strange, like a zero length buffer
  if(!len) return(NULL);

  // allocate the buffer memory
  *buf = (char*)malloc(len);
  if(!(*buf)) return(NULL);

  // get the string buffer over the socket
  result = SDLNet_TCP_Recv(sock,*buf,len);
  if(result < len) {
    if(SDLNet_GetError() && strlen(SDLNet_GetError())) // sometimes blank!
      printf("SDLNet_TCP_Recv: %s\n", SDLNet_GetError());
    free(*buf);
    buf=NULL;
  }
  // return the new buffer
  return(*buf);
}

// send a string buffer over a TCP socket with error checking
// returns 0 on any errors, length sent on success
//
// if length==0, buf is assumed to be a 0 terminated string
int TCPUtil::send(TCPsocket sock, char *buf, int length) {
  Uint32 len,result;
  
  if(!buf || !(length || strlen(buf))) return(1);


  // determine the length of the string
  if(!length) {
    len = strlen(buf) + 1; // add one for the terminating NULL
  } else {
    len = length;
  }
  
  // change endianness to network order
  len = SDL_SwapBE32(len);
  
  // send the length of the string
  result = SDLNet_TCP_Send(sock,&len,sizeof(len));
  if(result < sizeof(len)) {
    if(SDLNet_GetError() && strlen(SDLNet_GetError())) // sometimes blank!
      printf("SDLNet_TCP_Send: %s\n", SDLNet_GetError());
    return(0);
  }
	
  // revert to our local byte order
  len = SDL_SwapBE32(len);
	
  // send the buffer, with the NULL as well
  result = SDLNet_TCP_Send(sock, buf, len);
  if(result < len) {
    if(SDLNet_GetError() && strlen(SDLNet_GetError())) // sometimes blank!
      printf("SDLNet_TCP_Send: %s\n", SDLNet_GetError());
    return(0);
  }

  // return the length sent
  return(result);
}

#endif
