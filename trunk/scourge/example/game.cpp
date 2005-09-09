/***************************************************************************
                          game.cpp  -  description
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
#include "game.h"

Game::Game( Preferences *preferences, MapAdapter *adapter ) {
  // Create the shapes object. This is how we access the shapes.txt properties file.
  shapes = new Shapes( false );

  // Actually load some shapes
  shapes->initialize();
  cerr << "Shapes loaded: " << shapes->getShapeCount() << endl;

  // Use a random wall theme.
  shapes->loadRandomTheme();

  // Create a map.
  levelMap = new Map( adapter, preferences, shapes );
  
  // For now use the editor settings because we have no player.
  //MapSettings *mapSettings = new GameMapSettings();
  MapSettings *mapSettings = new EditorMapSettings();
  levelMap->setMapSettings( mapSettings );

  // Fill map with random shapes.
  createMap();
}

Game::~Game() {
  delete levelMap;
  delete shapes;
}

void Game::drawView() {
  // draw the map
  levelMap->draw();
}

void Game::handleEvent( SDL_Event *event ) {
  levelMap->handleEvent( event );
}

// Create a random map
void Game::createMap() {  

  // clean the map
  levelMap->reset();

  for( int x = MAP_OFFSET; x < MAP_WIDTH - MAP_OFFSET; x += MAP_UNIT ) {
    for( int y = MAP_OFFSET; y < MAP_DEPTH - MAP_OFFSET; y += MAP_UNIT ) {
      levelMap->
        setFloorPosition( x, y, 
                          shapes->findShapeByName( ( (int)( 2.0f * rand()/RAND_MAX ) == 0 ? "FLOOR_TILE" : "ROOM_FLOOR_TILE" ), true ) );
    }
  }

  // put down some random shapes
  for( int i = 0; i < 2500; i++ ) {
    int x = (int)( (float)( MAP_WIDTH - ( 2 * MAP_OFFSET ) ) * 
                   rand() / RAND_MAX ) + MAP_OFFSET;
    int y = (int)( (float)( MAP_DEPTH - ( 2 * MAP_OFFSET ) ) * 
                   rand() / RAND_MAX ) + MAP_OFFSET;
    int z = 0;
    int index = (int)( (float)( shapes->getShapeCount() - 2 ) * 
                       rand() / RAND_MAX ) + 1;
    Shape *shape = shapes->getShape( index );
    if( !levelMap->isBlocked( x, y, z, -1, -1, -1, shape ) ) {
      levelMap->setPosition( x, y, z, shape );
    }
  }

  // center someplace reasonable
  levelMap->center( MAP_WIDTH / 2, MAP_DEPTH / 2, true );
}

