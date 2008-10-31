#include "../common/constants.h"
#ifdef HAVE_SDL_NET
#include "clientinfo.h"

// ###### MS Visual C++ specific ###### 
#if defined(_MSC_VER) && defined(_DEBUG)
# define new DEBUG_NEW
# undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif 

using namespace std;

#define DEBUG_CLIENT_INFO 0

ClientInfo::ClientInfo( Server *server, TCPsocket socket, int id, char const* username ) {
	this->server = server;
	this->socket = socket;
	this->dead = false;
	this->id = id;
	this->username = username;
	this->commands = new Commands( this );
	this->characterInfo = NULL;

	// init the lag
	aveLag = 0.0f;
	totalLag = 0;
	lastLagCheck = 0;

	// start the client thread
	threadRunning = true;
	mutex = SDL_CreateMutex();
	thread = SDL_CreateThread( clientInfoLoop, this );
}

ClientInfo::~ClientInfo() {
	threadRunning = false;

	// tell the client not to reconnect
	TCPUtil::send( socket, "CLOSING" );

	// empty message queue
	if ( SDL_mutexP( mutex ) == -1 ) {
		cerr << "Couldn't lock mutex." << endl;
		exit( 7 );
	}
	while ( messageQueue.size() ) {
		Message *message = messageQueue.front();
		messageQueue.pop();
		delete message;
	}
	if ( SDL_mutexV( mutex ) == -1 ) {
		cerr << "Couldn't lock mutex." << endl;
		exit( 7 );
	}

	// stop the thread
	cerr << "* Stopping client thread: " << describe() << endl;
	int status;
	SDL_WaitThread( thread, &status );
	// kill the mutex
	SDL_DestroyMutex( mutex );
	cerr << "* Closing client socket: " << describe() << endl;
	// close the socket
	SDLNet_TCP_Close( socket );
	// misc. other stuff
	delete commands;
	delete characterInfo;
	cerr << "* Client stopped: " << describe() << endl;
}

char *ClientInfo::describe() {
	snprintf( desc, DESC_SIZE, "id=%d name=%s", id, username.c_str() );
	return desc;
}

void ClientInfo::chat( char *message ) {
	char s[1024];
	Commands::buildChat( s, 1024, getUsername(), message );
	server->sendToAllTCP( s );
}

void ClientInfo::logout() {
	cerr << "\t* logout: " << describe() << endl;
	dead = true;
}

void ClientInfo::ping( int frame ) {
	// keep track of client's lag
	updateLag( frame );
}

void ClientInfo::handleUnknownMessage() {
	// bad request; kill client
	dead = true;
}

void ClientInfo::processGameState( int frame, char *p ) {
	// do nothing
}

void ClientInfo::serverClosing() {
	// do nothing
}

void ClientInfo::character( char *bytes, int length ) {
	if ( length != sizeof( CreatureInfo ) ) {
		cerr << "* Server: Received bad size for character info! length=" <<
		length << " size=" << sizeof( CreatureInfo ) << endl;
		return;
	}
	characterInfo =  new CreatureInfo;
	memcpy( characterInfo, bytes, length );

	// FIXME: do some byte ordering for PPC, etc. here.

	cerr << "* Server: received character info for: " << describe() << endl;

	// send an addplayer to everyone but us
	int size = sizeof( CreatureInfo );
	std::vector<char> message( size + 20 );
	int messageSize;
	Commands::buildBytesAddPlayer( &message[0], size, bytes, getId(), &messageSize );
	server->sendToAllTCP( &message[0], messageSize, this );
}

void ClientInfo::addPlayer( Uint32 id, char *bytes, int length ) {
	// do nothing
}

void ClientInfo::sendMessageAsync( char *message, int length ) {
	// wake up the thread
	if ( message && !dead ) {

		// lock the mutex
		if ( SDL_mutexP( mutex ) == -1 ) {
			cerr << "Couldn't lock mutex." << endl;
			exit( 7 );
		}

		// put the message on the queue
		Message *m;
		if ( !length ) {
			m = new Message( strlen( message ) + 1 );
			strcpy( &( *m )[0], message );
		} else {
			m = new Message( length );
			memcpy( &( *m )[0], message, length );
		}
		messageQueue.push( m );

		// unlock the mutex
		if ( SDL_mutexV( mutex ) == -1 ) {
			cerr << "Couldn't unlock mutex." << endl;
			exit( 7 );
		}
	}
}

void ClientInfo::receiveTCP() {
	// read from the socket
	char *text = NULL;
	int length;
	if ( TCPUtil::receive( socket, &text, &length ) ) {
		commands->interpret( text, length );
	}
	delete text;
}

void ClientInfo::sendTCP( char *message, int length ) {
	if ( message ) {
		if ( !TCPUtil::send( socket, message, length ) ) {
			cerr << "* Can't send TCP to client: " << describe() << endl;
			dead = true;
		}
	}
}

int clientInfoLoop( void *data ) {
	bool runAgain = false;
	ClientInfo *clientInfo = ( ClientInfo* )data;

	SDLNet_SocketSet set = SDLNet_AllocSocketSet( 1 );
	if ( !set ) {
		cerr << "*** error: SDLNet_AllocSocketSet: " << SDLNet_GetError() << endl;
		exit( 1 ); //most of the time this is a major error, but do what you want.
	}

	// add the socket to the set, so we can call SDLNet_CheckSockets
	if ( SDLNet_TCP_AddSocket( set, clientInfo->getSocket() ) == -1 ) {
		cerr << "* SDLNet_TCP_AddSocket: " << SDLNet_GetError() << endl;
		SDLNet_FreeSocketSet( set );
		clientInfo->dead = true;
		return 0;
	}

	// copy the message  into this, so Message can be deleted in mutex
	char *messageStr = NULL;
	int messageLength = 0;

	while ( clientInfo->isThreadRunning() ) {

		cerr << "&";

		if ( !runAgain ) {
			SDL_Delay( 500 );
		}
		if ( !clientInfo->isThreadRunning() ) break;

		// lock the mutex
		if ( SDL_mutexP( clientInfo->getMutex() ) == -1 ) {
			cerr << "Couldn't lock mutex." << endl;
			exit( 7 );
		}
		// then wait for a signal to start again
#if DEBUG_CLIENT_INFO
		cerr << "Net: Server: clientInfo: " << clientInfo->describe() << " waiting..." << endl;
#endif

		runAgain = false;

		// remove a message from the queue
		Message *message = NULL;
		if ( clientInfo->getMessageQueue()->size() ) {

#if DEBUG_CLIENT_INFO
			cerr << "Net: Server: clientInfo: " << clientInfo->describe() << " handling outgoing messages." << endl;
#endif

			// do I need to copy the message?
			message = clientInfo->getMessageQueue()->front();
			messageLength = message->size();
			messageStr = new char[messageLength];
			memcpy( messageStr, &( *message )[0], messageLength );

			clientInfo->getMessageQueue()->pop();
			delete message;
			runAgain = true;
		}
		// unlock the mutex
		if ( SDL_mutexV( clientInfo->getMutex() ) == -1 ) {
			cerr << "Couldn't unlock mutex." << endl;
			exit( 7 );
		}
		if ( !clientInfo->isThreadRunning() ) break;

		// send message if any
		if ( messageStr ) {
			clientInfo->sendTCP( messageStr, messageLength );
			delete [] messageStr;
			messageStr = NULL;
		}

		if ( !clientInfo->isThreadRunning() ) break;
		// handle the request if any
		int numready = SDLNet_CheckSockets( set, ( Uint32 )1000 );
		if ( numready == -1 ) {
			cerr << "SDLNet_CheckSockets: " << SDLNet_GetError() << endl;
			clientInfo->dead = true;
			break;
		} else if ( numready && SDLNet_SocketReady( clientInfo->getSocket() ) ) {
#if DEBUG_CLIENT_INFO
			cerr << "Net: Server: clientInfo: " << clientInfo->describe() << " handling incoming socket messages." << endl;
#endif
			if ( !clientInfo->isThreadRunning() ) break;
			clientInfo->receiveTCP();
			runAgain = true;
		}
	}

	delete messageStr;

	SDLNet_FreeSocketSet( set );
	return 0;
}

void ClientInfo::setLagTimer( int frame, Uint32 n ) {
	if ( static_cast<int>( lagMap.size() ) > MAX_SCHEDULED_LAG_MESSAGES ) {
		dead = true;
	} else {
		lagMap[frame] = n;
	}
}

Uint32 ClientInfo::updateLag( int frame ) {
	if ( lagMap.find( frame ) != lagMap.end() ) {
		Uint32 n = lagMap[frame];

		// FIXME: do this more efficiently
		// remove older frames
		for ( map<int, Uint32>::iterator i = lagMap.begin(); i != lagMap.end(); ) {
			int f = i->first;

			if ( f <= frame )
				lagMap.erase( i++ );
			else ++i;
		}

		// compute the lag
		Uint32 t = SDL_GetTicks();
		Uint32 lag = t - n;
		totalLag += lag;
		if ( t - lastLagCheck > 5000 ) {
			aveLag = static_cast<float>( totalLag ) / static_cast<float>( t - lastLagCheck );
			totalLag = 0;
			lastLagCheck = t;
			cerr << "Avg lag for " << describe() << " is " << ( static_cast<float>( aveLag ) / 1000.0f ) << " sec." << endl;
		}

		return n;
	}
	return ( Uint32 )0;
}

/*
Message::Message(char *message, int length) {
  this->message = message;
  this->length = length;
}

Message::~Message() {
  free(message);
}*/
#endif
