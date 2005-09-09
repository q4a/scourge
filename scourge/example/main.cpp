/***************************************************************************
                          main.cpp  -  description
                             -------------------
    begin                : Sat May 3 2003
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
#include <constants.h>
#include "examplepreferences.h"
#include "graphics.h"
#include "game.h"

int main( int argc, char *argv[] ) {

  // where is our data dir?
  if( argc < 3 ) {
    cerr << "Usage: ./main <true|false to use stencil buffer> <path to data dir>" << endl;
    exit( 1 );
  }
  bool useStencilBuffer = ( !strcmp( argv[1], "true" ) );
  rootDir = strdup( argv[ 2 ] );

  // Set up the graphics and preferences
  Graphics *graphics = new Graphics();
  Preferences *pref = new ExamplePreferences( useStencilBuffer );
  graphics->setVideoMode( pref );

  // Run the game
  Game *game = new Game( pref, graphics );
  graphics->mainLoop( game );
}

