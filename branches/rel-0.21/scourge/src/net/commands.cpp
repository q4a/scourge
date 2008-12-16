#include "../common/constants.h"
#include "commands.h"

using namespace std;

Commands::Commands( CommandInterpreter *ci ) {
	this->ci = ci;
	this->lastGameFrameReceived = 0;
	this->lastCommand = -1;
}

Commands::~Commands() {
}

void Commands::interpret( char *rawMessage, int length ) {
	if ( length > 2 && !strncmp( rawMessage, "C:", 2 ) ) {
		lastCommand = CHARACTER;
		ci->character( rawMessage + 2, length - 2 );
	} else if ( length > 2 && !strncmp( rawMessage, "P:", 2 ) ) {
		lastCommand = ADD_PLAYER;
		Uint32 id = ( Uint32 ) * ( rawMessage + 2 );
		ci->addPlayer( id, rawMessage + 2 + sizeof( Uint32 ), length - ( 2 + sizeof( Uint32 ) ) );
	} else if ( !strncmp( rawMessage, "CHAT,", 5 ) ) {
		lastCommand = CHAT;
		ci->chat( rawMessage + 5 );
	} else if ( !strncmp( rawMessage, "LOGOUT,", 7 ) ) {
		lastCommand = LOGOUT;
		ci->logout();
	} else if ( !strncmp( rawMessage, "PING,", 5 ) ) {
		lastCommand = PING;
		ci->ping( atoi( rawMessage + 5 ) );
	} else if ( !strncmp( rawMessage, "STATE,", 6 ) ) {
		lastCommand = STATE;
		char *p = strchr( rawMessage + 6, ',' );
		lastGameFrameReceived = atoi( rawMessage + 6 );
		ci->processGameState( lastGameFrameReceived, p );
	} else if ( !strncmp( rawMessage, "CLOSING", 7 ) ) {
		lastCommand = CLOSING;
		ci->serverClosing();
	} else {
		lastCommand = -1;
		cerr << "* Bad request." << endl;
		ci->handleUnknownMessage();
	}
}

void Commands::buildGameState( char *buff, size_t buffSize, int frame, char *state ) {
	snprintf( buff, buffSize, "STATE,%d,%s", frame, state );
}

void Commands::buildChat( char *buff, size_t buffSize, char const* username, char *message ) {
	snprintf( buff, buffSize, "CHAT,%s> %s", username, message );
}

void Commands::buildLogin( char *buff, size_t buffSize, char const* username ) {
	snprintf( buff, buffSize, "LOGIN,%s", username );
}

void Commands::buildPing( char *buff, size_t buffSize ) {
	snprintf( buff, buffSize, "PING,%d", getLastGameFrameReceived() );
}

void Commands::buildBytesCharacter( char *buff, int size, char *info, int *messageSize ) {
	strcpy( buff, "C:" );
	memcpy( buff + 2, info, size );
	*messageSize = size + 2;
}

void Commands::buildBytesAddPlayer( char *buff, int size, char *info, int id, int *messageSize ) {
	strcpy( buff, "P:" );
	Uint32 i = ( Uint32 )id;
	memcpy( buff + 2, &i, sizeof( Uint32 ) );
	memcpy( buff + 2 + sizeof( Uint32 ), info, size );
	*messageSize = size + 2 + sizeof( Uint32 );
}

TestCommandInterpreter::TestCommandInterpreter() {
}

TestCommandInterpreter::~TestCommandInterpreter() {
}

void TestCommandInterpreter::chat( char *message ) {
	cout << message << endl;
}

void TestCommandInterpreter::logout() {
	cout << "Logout." << endl;
}

void TestCommandInterpreter::ping( int frame ) {
	cout << "Ping." << endl;
}

void TestCommandInterpreter::processGameState( int frame, char *p ) {
	cout << "Game state: frame=" << frame << " state=" << p << endl;
}

void TestCommandInterpreter::handleUnknownMessage() {
	cout << "Unknown message received." << endl;
}

void TestCommandInterpreter::serverClosing() {
	cout << "Server closing." << endl;
}

void TestCommandInterpreter::character( char *bytes, int length ) {
	cout << "Received character: length=" << length << endl;
}

void TestCommandInterpreter::addPlayer( Uint32 id, char *bytes, int length ) {
	cerr << "* Received character data for player. Server id=" <<
	id << " data length=" << length << endl;
	if ( length != sizeof( CreatureInfo ) ) {
		cerr << "* Bad length for addPlayer!. length=" <<
		length << " size=" << sizeof( CreatureInfo ) << endl;
		return;
	}
}
