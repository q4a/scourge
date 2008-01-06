/***************************************************************************
                          main.cpp  -  description
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

#include <stdlib.h>
#ifdef _MSC_VER
#	include <io.h>
#	include <fcntl.h>
#endif
#include "userconfiguration.h"
#include "gameadapter.h"
#include "session.h"
#include "scourge.h"
#include "common/binreloc.h"

using namespace std;

GameAdapter *createGameAdapter(UserConfiguration *config) {
  GameAdapter *adapter;
  if(config->getStandAloneMode() == UserConfiguration::SERVER) {
    adapter = new ServerAdapter(config);
  } else if(config->getStandAloneMode() == UserConfiguration::CLIENT) {
    adapter = new ClientAdapter(config);
  } else if( config->getStandAloneMode() == UserConfiguration::TEST ) {
    adapter = new GameAdapter( config );
  } else {
    adapter = new Scourge(config);
  }
  return adapter;
}

// -=K=-: MSVC does lose all console i/o to void; i noticed scourge 
// uses console i/o; so lets create a console and redirect i/o there 
// RedirectIOToConsole should be called once/app so inline is OK 
inline void RedirectIOToConsole(void)
{
//define WANT_CONSOLE if you want it to work 
#if defined(_MSC_VER) && defined(WANT_CONSOLE) 
	int const MAX_CONSOLE_LINES(500);
	int hConHandle;

	CONSOLE_SCREEN_BUFFER_INFO coninfo;
	FILE *fp;

	// allocate a console for this app
	AllocConsole();

	// set the screen buffer to be big enough to let us scroll text
	HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
	GetConsoleScreenBufferInfo(hOut, &coninfo);
	coninfo.dwSize.Y = MAX_CONSOLE_LINES;
	SetConsoleScreenBufferSize(hOut, coninfo.dwSize);

	// redirect unbuffered STDOUT to the console
	hConHandle = _open_osfhandle((intptr_t)hOut, _O_TEXT);
	fp = _fdopen( hConHandle, "w" );
	*stdout = *fp;
	setvbuf( stdout, NULL, _IONBF, 0 );

	// redirect unbuffered STDIN to the console
	hConHandle = _open_osfhandle((intptr_t)GetStdHandle(STD_INPUT_HANDLE), _O_TEXT);
	fp = _fdopen( hConHandle, "r" );
	*stdin = *fp;
	setvbuf( stdin, NULL, _IONBF, 0 );

	// redirect unbuffered STDERR to the console
	hConHandle = _open_osfhandle((intptr_t)GetStdHandle(STD_ERROR_HANDLE), _O_TEXT);
	fp = _fdopen( hConHandle, "w" );
	*stderr = *fp;
	setvbuf( stderr, NULL, _IONBF, 0 );
#endif //defined(_MSC_VER) && defined(WANT_CONSOLE) 
}

int main(int argc, char *argv[]) {
  BrInitError error; 
  if( br_init( &error ) == 0 && error != BR_INIT_ERROR_DISABLED ) { 
      printf( "Warning: BinReloc failed to initialize (error code %d)\n", error ); 
      printf( "Will fallback to hardcoded default path.\n" ); 
  }

  UserConfiguration *userConfiguration = new UserConfiguration();  
  userConfiguration->loadConfiguration();    
  userConfiguration->parseCommandLine(argc, argv); 
  RedirectIOToConsole();
  return Session::runGame( createGameAdapter( userConfiguration ), argc, argv );
}

