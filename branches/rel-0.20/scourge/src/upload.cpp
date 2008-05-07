/*
	Adapted from Battle for Wesnoth:

   Copyright (C) 2005 by Rusty Russell <rusty@xxxxxxxxxxxxxxx>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/
#include "common/constants.h"
#include "upload.h"

#ifdef HAVE_SDL_NET
#include "SDL_net.h"
#endif

#include <iostream>
#include <vector>
#include <string>
#include <sstream>

using namespace std;

#define TARGET_HOST "scourge.sourceforge.net"
#define TARGET_URL "/highscore/score2.php"
#define TARGET_PORT 80
#define SUCCESS_STR "add-success: "

struct error
{
	error(const std::string& msg="");
	std::string message;
};

error::error(const std::string& msg) : message(msg)
{
}

template<typename To, typename From>
To lexical_cast(From a)
{
	To res;
	std::stringstream str;

	if(!(str << a && str >> res)) {
		throw error( "Bad lexical cast." );
	} else {
		return res;
	}
}

#ifdef HAVE_SDL_NET
static void send_string(TCPsocket sock, const std::string &str) {	
	cerr << "SENDING: " << str << endl;
	if( SDLNet_TCP_Send( sock, (void *)str.c_str(), str.length() ) != static_cast<int>(str.length()) ) {
		throw error( "Bad write." );
	}	
}
#endif

int Upload::uploadScoreToWeb( char *score, RESULT& result ) {
	int errNum = 0;
	strcpy( result, "" );

#ifdef HAVE_SDL_NET
	TCPsocket sock = NULL;

	const char *header =
		"POST " TARGET_URL " HTTP/1.1\n"
		"Host: " TARGET_HOST "\n"
		"User-Agent: Scourge " SCOURGE_VERSION "\n"
		"Content-Type: application/x-www-form-urlencoded\n";
		//"Content-Type: text/plain\n";
	
	try {
		IPaddress ip;
		cerr << "Resolving address..." << endl;
		if( SDLNet_ResolveHost( &ip, TARGET_HOST, TARGET_PORT ) == 0 ) {

			string contents = score;
			cerr << "Connecting..." << endl;
			sock = SDLNet_TCP_Open(&ip);
			if( !sock ) throw error( "Can't connect." );

			cerr << "Sending data..." << endl;
			send_string( sock, header );
			send_string( sock, "Content-length: " );
			send_string( sock, lexical_cast<std::string>(contents.length()) );
			send_string( sock, "\n\n" );
			send_string( sock, contents.c_str() );

			cerr << "Waiting for ack..." << endl;
			
			// print out the entire string
			char res[5000];
			int count = SDLNet_TCP_Recv( sock, res, sizeof( res ) );
			cerr << ">>>";
			for( int i = 0; i < count; i++ ) {
				cerr << res[i];
			}
			cerr << "<<<" << endl;
			cerr << "Got " << count << " bytes." << endl;

			if( count < 10 ) {
				strcpy( result, "Too few bytes received." );
				errNum = 1;
			} else if( memcmp( res, "HTTP/1.", strlen( "HTTP/1." ) ) != 0 ) {
				// Must be version 1.x, must start with 2 (eg. 200) for success
				cerr << "Wrong http version." << endl;
				strcpy( result, "Wrong http version." );
				errNum = 1;
			} else if( memcmp( res + 8, " 2", strlen( " 2" ) ) != 0 ) {
				cerr << "Post did not succeed." << endl;
				strcpy( result, "Post did not succeed." );
				errNum = 1;
			} else {
				cerr << "Post succeeded." << endl;
				res[count] = '\0';
				char *p = strstr( res, SUCCESS_STR );
				if( p ) {
					char *q = strpbrk( p, "\n\r" );
					if( q ) {
						*q = 0;
						strcpy( result, p + strlen( SUCCESS_STR ) );
					} else {
						snprintf( result, RESULT_SIZE, "Can't parse result: %s", res );
						errNum = 1;
					}
				} else {
					snprintf( result, RESULT_SIZE,  "Bad result: %s", res );
					errNum = 1;
				}
			}
			
			cerr << "Closing socket..." << endl;
			SDLNet_TCP_Close(sock);
			sock = NULL;
		} else {
      cerr << "*** Error: unable to resolve address: " << TARGET_HOST << ":" << TARGET_PORT << endl;
      cerr << "\tError: " << SDLNet_GetError() << endl;
    } 
	} catch( error *err ) { 
		cerr << "Exception caught." << err->message << endl;
		strcpy( result, err->message.c_str() );
		errNum = 1;
	}

	if( sock )
		SDLNet_TCP_Close( sock );
	
	cerr << "Done." << endl;
#endif
	return errNum;
}

/*
int main( int argc, char *argv[] ) {
	if( SDLNet_Init() == -1 ) {
		cerr << "Error: " << SDL_GetError() << endl;
	}

	int res = uploadScore( "mode=add&user=Tipsy McStagger&score=5000&desc=OMG, I can't believe this works." );

	SDLNet_Quit();

	return res;
}
*/
