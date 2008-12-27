/***************************************************************************
             main.cpp  -  The code that starts everything else
                             -------------------
    begin                : Sat May  3 19:39:34 EDT 2003
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
#include "common/constants.h"

#include <stdlib.h>
#ifdef _MSC_VER
# include <io.h>
# include <fcntl.h>
#endif
#include "userconfiguration.h"
#include "gameadapter.h"
#include "session.h"
#include "scourge.h"
#include "common/binreloc.h"

using namespace std;

// ###### MS Visual C++ specific ###### 
// When running with _CRTDBG_LEAK_CHECK_DF flag set you get tons of 
// reports about memory leaks. Usually you want to know where the leaked
// memory block was allocated. Take following block and paste it to every
// .cpp file that uses new. It makes the reports to contain the info.
// It will attach the data to each and every memory block allocated 
// so go buy more memory too. ;) 
#if defined(_MSC_VER)
# ifdef _DEBUG
#   define new DEBUG_NEW
#   undef THIS_FILE
    static char THIS_FILE[] = __FILE__;
# endif
#endif 

GameAdapter *createGameAdapter( UserConfiguration *config ) {
	GameAdapter *adapter;
	if ( config->getStandAloneMode() == UserConfiguration::SERVER ) {
		adapter = new ServerAdapter( config );
	} else if ( config->getStandAloneMode() == UserConfiguration::CLIENT ) {
		adapter = new ClientAdapter( config );
	} else if ( config->getStandAloneMode() == UserConfiguration::TEST ) {
		adapter = new GameAdapter( config );
	} else {
		adapter = new Scourge( config );
	}
	return adapter;
}

// This works against MSVC from losing all console i/o to void; 
//	it creates a console and redirects all standard i/o there
void RedirectIOToConsole( void ) {
//define WANT_CONSOLE if you want it to work
#if defined(_MSC_VER) && defined(WANT_CONSOLE)
	int const MAX_CONSOLE_LINES( 500 );
	int hConHandle;

	CONSOLE_SCREEN_BUFFER_INFO coninfo;
	FILE *fp;

	// allocate a console for this app
	AllocConsole();

	// set the screen buffer to be big enough to let us scroll text
	HANDLE hOut = GetStdHandle( STD_OUTPUT_HANDLE );
	GetConsoleScreenBufferInfo( hOut, &coninfo );
	coninfo.dwSize.Y = MAX_CONSOLE_LINES;
	SetConsoleScreenBufferSize( hOut, coninfo.dwSize );

	// redirect unbuffered STDOUT to the console
	hConHandle = _open_osfhandle( ( intptr_t )hOut, _O_TEXT );
	fp = _fdopen( hConHandle, "w" );
	*stdout = *fp;
	setvbuf( stdout, NULL, _IONBF, 0 );

	// redirect unbuffered STDIN to the console
	hConHandle = _open_osfhandle( ( intptr_t )GetStdHandle( STD_INPUT_HANDLE ), _O_TEXT );
	fp = _fdopen( hConHandle, "r" );
	*stdin = *fp;
	setvbuf( stdin, NULL, _IONBF, 0 );

	// redirect unbuffered STDERR to the console
	hConHandle = _open_osfhandle( ( intptr_t )GetStdHandle( STD_ERROR_HANDLE ), _O_TEXT );
	fp = _fdopen( hConHandle, "w" );
	*stderr = *fp;
	setvbuf( stderr, NULL, _IONBF, 0 );
#endif //defined(_MSC_VER) && defined(WANT_CONSOLE)
#if defined(_MSC_VER) && defined(_DEBUG)
	// More MS Visual C++ specific debug crap  
	// Send all reports to STDOUT
	_CrtSetReportMode( _CRT_WARN, _CRTDBG_MODE_FILE | _CRTDBG_MODE_DEBUG );
	_CrtSetReportFile( _CRT_WARN, _CRTDBG_FILE_STDERR );
	_CrtSetReportMode( _CRT_ERROR, _CRTDBG_MODE_FILE | _CRTDBG_MODE_WNDW );
	_CrtSetReportFile( _CRT_ERROR, _CRTDBG_FILE_STDERR );
	_CrtSetReportMode( _CRT_ASSERT, _CRTDBG_MODE_FILE | _CRTDBG_MODE_WNDW );
	_CrtSetReportFile( _CRT_ASSERT, _CRTDBG_FILE_STDERR );
	// Following things can be used all around code to hunt down bugs.
	// Here they set default behaviour.
	// Uncomment if you want to check corrupts once for every 1024 mem ops. 
	// Values like EVERY_128, EVERY_16 or ALWAYS make your software real SLOW.
	SET_CRT_DEBUG_FIELD( _CRTDBG_CHECK_EVERY_1024_DF );

	// Uncomment to make above _CRTDBG_CHECK_sometimes_DFs reports readable.
	// It does not really free freed memory so better dont use here!!! 
	// SET_CRT_DEBUG_FIELD( _CRTDBG_DELAY_FREE_MEM_DF );

	// Uncomment if you also want to check memory used by crt internally.
	// Usually its not needed but never say never, MS staff are also human beings.
	// SET_CRT_DEBUG_FIELD( _CRTDBG_CHECK_CRT_DF );

	// Uncomment if you want to see leaks at application end (there are lots.)
	// SET_CRT_DEBUG_FIELD( _CRTDBG_LEAK_CHECK_DF );
#endif
}

/// Everything begins here.
int main( int argc, char *argv[] ) {
	BrInitError error;
	if ( br_init( &error ) == 0 && error != BR_INIT_ERROR_DISABLED ) {
		printf( "Warning: BinReloc failed to initialize (error code %d)\n", error );
		printf( "Will fallback to hardcoded default path.\n" );
	}

	// Session::runGame() seemingly never returns (so local UserConfiguration leaks). 
	// static is possibly most safe (no destruction order dependencies).  
	static UserConfiguration userConfiguration;
	userConfiguration.loadConfiguration();
	userConfiguration.parseCommandLine( argc, argv );
	RedirectIOToConsole();
	return Session::runGame( createGameAdapter( &userConfiguration ), argc, argv );
}

