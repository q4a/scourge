/***************************************************************************
                          mapeditor.cpp  -  description
                             -------------------
    begin                : Tue Jun 18 2005
    copyright            : (C) 2005 by Gabor Torok
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

#include "mapeditor.h"

/**
  *@author Gabor Torok
  */

MapEditor::MapEditor( Scourge *scourge ) {
  this->scourge = scourge;

  mapSettings = new EditorMapSettings();

  int w = 200;
  mainWin = new Window( scourge->getSDLHandler(),
                        scourge->getScreenWidth() - w, 0,
                        w, scourge->getScreenHeight(),
                        "Map Editor", false, Window::BASIC_WINDOW,
                        GuiTheme::DEFAULT_THEME );
  mainWin->setVisible( false );
  
  doneButton = mainWin->createButton( 5, 5, w - 10, 20, "Done" );

  wallButton = mainWin->createButton( 5, 25, w - 10, 45, "Wall", true );
  toggleButtonList.push_back( wallButton );
  wallButton->setSelected( true );
  doorButton = mainWin->createButton( 5, 50, w - 10, 70, "Door", true );
  toggleButtonList.push_back( doorButton );
  eraseButton = mainWin->createButton( 5, 75, w - 10, 95, "Erase", true );
  toggleButtonList.push_back( eraseButton );

  mainWin->createLabel( 5, 110, "Shapes:" );
  shapeList = new ScrollingList( 5, 120, w - 10, 150, 
                                 scourge->getShapePalette()->getHighlightTexture() );
  mainWin->addWidget( shapeList );
  map< string, GLShape* > *shapeMap = scourge->getShapePalette()->getShapeMap();
  shapeNames = (char**)malloc( shapeMap->size() * sizeof(char*) );
  int count = 0;
  for (map<string, GLShape*>::iterator i = shapeMap->begin(); i != shapeMap->end(); ++i ) {
    string name = i->first;
    char *p = (char*)name.c_str();
    shapeNames[ count ] = (char*)malloc( 120 * sizeof(char) );
    strcpy( shapeNames[ count ], p );
    count++;
  }
  shapeList->setLines( shapeMap->size(), (const char**)shapeNames );
}                                                                         

MapEditor::~MapEditor() {
  map< string, GLShape* > *shapeMap = scourge->getShapePalette()->getShapeMap();
  for(int i = 0; i < (int)shapeMap->size(); i++) {
    free( shapeNames[ i ] );
  }
  free( shapeNames );
  delete mainWin;
}

void MapEditor::drawView() {
  scourge->getMap()->draw();

  glDisable( GL_CULL_FACE );
  glDisable( GL_SCISSOR_TEST );

  glDisable( GL_CULL_FACE );
  glDisable( GL_DEPTH_TEST );
  glDisable( GL_TEXTURE_2D );
  glPushMatrix();
  glColor3f( 1, 0, 0 );
  
  glLoadIdentity();

  scourge->getSDLHandler()->texPrint( 50, 120, "F:%d,%d C:%d,%d", 
                                      scourge->getMap()->getCursorFlatMapX(), 
                                      scourge->getMap()->getCursorFlatMapY(), 
                                      scourge->getMap()->getCursorChunkX(), 
                                      scourge->getMap()->getCursorChunkY() );
  
  glTranslatef( 50, 50, 0 );
  glRotatef( scourge->getMap()->getZRot(), 0, 0, 1 );
  
  int n = 30;
  glBegin( GL_LINES );
  glVertex2f( 0, 0 );
  glVertex2f( 0, n );

  glVertex2f( 0, 0 );
  glVertex2f( -n/2, n/2 );

  glVertex2f( 0, 0 );
  glVertex2f( n/2, n/2 );
  glEnd();

  glPopMatrix();


}

void MapEditor::drawAfter() {
}

bool MapEditor::handleEvent(SDL_Event *event) {

  scourge->getMap()->handleEvent( event );

  switch(event->type) {
  case SDL_MOUSEMOTION:
  processMouseMotion( event->motion.state );
  break;
  case SDL_MOUSEBUTTONDOWN:
  processMouseMotion( event->button.button );
  break;
  case SDL_KEYUP:
  if( event->key.keysym.sym == SDLK_ESCAPE ) {
    hide();
    return true;
  }
  break;
  default: break;
  }
  return false;
}

bool MapEditor::handleEvent(Widget *widget, SDL_Event *event) {
  if( widget == doneButton ) {
    hide();
    return true;
  }

  int found = -1;
  for( int i = 0; i < (int)toggleButtonList.size(); i++ ) {
    if( toggleButtonList[ i ] == widget ) {
      found = i;
      break;
    }
  }
  if( found > -1 ) {
    for( int i = 0; i < (int)toggleButtonList.size(); i++ ) {
      toggleButtonList[ i ]->setSelected( i == found );
    }
  }

  return false;
}

void MapEditor::show() { 
  scourge->getMap()->setMapSettings( mapSettings );
  scourge->getMap()->reset();
  scourge->getMap()->center( MAP_WIDTH / 2, MAP_DEPTH / 2, true );
  scourge->getShapePalette()->loadTheme( "egypt" );
  mainWin->setVisible( true ); 
}

void MapEditor::hide() { 
  mainWin->setVisible( false ); 
  scourge->getMap()->setMapSettings( scourge->getMapSettings() );
}

void MapEditor::processMouseMotion( Uint8 button ) {
  if( button == SDL_BUTTON_LEFT || 
      button == SDL_BUTTON_RIGHT ) {
    
    // draw the correct walls in this chunk
    int innerX = scourge->getMap()->getCursorFlatMapX() - 
      ( scourge->getMap()->getCursorChunkX() * MAP_UNIT + MAP_OFFSET );
    int innerY = scourge->getMap()->getCursorFlatMapY() - 
      ( scourge->getMap()->getCursorChunkY() * MAP_UNIT + MAP_OFFSET );

    int mapx = scourge->getMap()->getCursorChunkX() * MAP_UNIT + MAP_OFFSET;
    int mapy = scourge->getMap()->getCursorChunkY() * MAP_UNIT + MAP_OFFSET;

//    cerr << "pos: " << mapx << "," << mapy << 
//      " map:" << scourge->getMap()->getCursorFlatMapX() << "," << 
//      scourge->getMap()->getCursorFlatMapX() << endl;
    
    // find the region in the chunk
    if( innerX < MAP_UNIT_OFFSET ) { 
      // west
      if( button == SDL_BUTTON_RIGHT || eraseButton->isSelected() ) {
        removeEWWall( mapx, mapy + 1, 1 );
      } else if( wallButton->isSelected() ) {
        addEWWall( mapx, mapy + 1, 1 );
      } else if( doorButton->isSelected() ) {
      }
    } else if( innerY < MAP_UNIT_OFFSET ) { 
      // north
      if( button == SDL_BUTTON_RIGHT || eraseButton->isSelected() ) {
        removeNSWall( mapx, mapy + 1, 1 ); 
      } else if( wallButton->isSelected() ) {
        addNSWall( mapx, mapy + 1, 1 ); 
      } else if( doorButton->isSelected() ) {
      }
    } else if( innerX >= MAP_UNIT - MAP_UNIT_OFFSET ) {
      // east
      if( button == SDL_BUTTON_RIGHT || eraseButton->isSelected() ) {
        removeEWWall( mapx + MAP_UNIT - MAP_UNIT_OFFSET, mapy + 1, -1 );
      } else if( wallButton->isSelected() ) {
        addEWWall( mapx + MAP_UNIT - MAP_UNIT_OFFSET, mapy + 1, -1 );
      } else if( doorButton->isSelected() ) {
      }
    } else if( innerY >= MAP_UNIT - MAP_UNIT_OFFSET ) {
      // south
      if( button == SDL_BUTTON_RIGHT || eraseButton->isSelected() ) {
        removeNSWall( mapx, mapy + MAP_UNIT - MAP_UNIT_OFFSET + 1, -1 ); 
      } else if( wallButton->isSelected() ) {
        addNSWall( mapx, mapy + MAP_UNIT - MAP_UNIT_OFFSET + 1, -1 ); 
      } else if( doorButton->isSelected() ) {
      }
    } else {
      addFloor( mapx, mapy + 1 );
    }
  }
}

void MapEditor::addEWWall( Sint16 mapx, Sint16 mapy, int dir ) {
  // short wall
  bool north = false;
  bool south = false;
  if( !scourge->getMap()->getLocation( mapx, mapy + MAP_UNIT / 2, 0 ) ) {

    // see if EW_WALL to the north and south
    north = ( mapy - 1 >= 0 && 
              scourge->getMap()->getLocation( mapx, mapy - 1, 0 ) &&
              !scourge->getMap()->getLocation( mapx + dir * ( MAP_UNIT / 2 ), mapy + 1, 0 ) );
    south = ( mapy + MAP_UNIT + 1 < MAP_DEPTH && 
              scourge->getMap()->getLocation( mapx, mapy + MAP_UNIT + 1, 0 ) &&
              !scourge->getMap()->getLocation( mapx + dir * ( MAP_UNIT / 2 ), mapy + MAP_UNIT - 1, 0 ) );

    // corner
    if( south ) {
      scourge->getMap()->
        removePosition( mapx, mapy + MAP_UNIT, 0 );
    } else if( !scourge->getMap()->getLocation( mapx, mapy + MAP_UNIT, 0 ) ) {
      scourge->getMap()->
        setPosition( mapx, mapy + MAP_UNIT, 0, 
                     scourge->getShapePalette()->
                     findShapeByName( "CORNER", true ) );
    }

    // corner
    if( north ) {
      scourge->getMap()->
        removePosition( mapx, mapy + MAP_UNIT_OFFSET, 0 ); 
    } else if( !scourge->getMap()->getLocation( mapx, mapy + MAP_UNIT_OFFSET, 0 ) ) {
      scourge->getMap()->
        setPosition( mapx, mapy + MAP_UNIT_OFFSET, 0, 
                     scourge->getShapePalette()->
                     findShapeByName( "CORNER", true ) );  
    }

    if( north && south ) {
      scourge->getMap()->
        setPosition( mapx, mapy + MAP_UNIT, 0, 
                     scourge->getShapePalette()->
                     findShapeByName( "EW_WALL_TWO_EXTRAS", true ) );
    } else if( south ) {
      scourge->getMap()->
        setPosition( mapx, mapy + MAP_UNIT, 0, 
                     scourge->getShapePalette()->
                     findShapeByName( "EW_WALL_EXTRA", true ) );
    } else if( north ) {
      scourge->getMap()->
        setPosition( mapx, mapy + MAP_UNIT - MAP_UNIT_OFFSET, 0, 
                     scourge->getShapePalette()->
                     findShapeByName( "EW_WALL_EXTRA", true ) );
    } else {
      scourge->getMap()->
        setPosition( mapx, mapy + MAP_UNIT - MAP_UNIT_OFFSET, 0, 
                     scourge->getShapePalette()->
                     findShapeByName( "EW_WALL", true ) );
    }

    // change north chunk
    //cerr << "Looking north of EW_WALL map=" << mapx << "," << mapy << endl;
    if( isShape( mapx, mapy - MAP_UNIT_OFFSET, 0, "EW_WALL" ) &&
        isShape( mapx, mapy, 0, "CORNER" ) &&
        !scourge->getMap()->getLocation( mapx + dir * ( MAP_UNIT / 2 ), mapy - 1, 0 ) ) {
      //cerr << "Success!" << endl;
      scourge->getMap()->removePosition( mapx, mapy - MAP_UNIT_OFFSET, 0 );
      scourge->getMap()->removePosition( mapx, mapy, 0 );
      scourge->getMap()->
        setPosition( mapx, mapy, 0, scourge->getShapePalette()->
                     findShapeByName( "EW_WALL_EXTRA", true ) );
    }

    //cerr << "Looking north of EW_WALL_EXTRA map=" << mapx << "," << mapy << endl;
    if( isShape( mapx, mapy - MAP_UNIT_OFFSET, 0, "EW_WALL_EXTRA" ) &&
        isShape( mapx, mapy, 0, "CORNER" ) &&
        !scourge->getMap()->getLocation( mapx + dir * ( MAP_UNIT / 2 ), mapy - 1, 0 ) ) {
      //cerr << "Success!" << endl;
      scourge->getMap()->removePosition( mapx, mapy - MAP_UNIT_OFFSET, 0 );
      scourge->getMap()->removePosition( mapx, mapy, 0 );
      scourge->getMap()->
        setPosition( mapx, mapy, 0, scourge->getShapePalette()->
                     findShapeByName( "EW_WALL_TWO_EXTRAS", true ) );
    }

    // change the south chunk
    //cerr << "Looking south of EW_WALL map=" << mapx << "," << mapy << endl;
    if( isShape( mapx, mapy + MAP_UNIT + MAP_UNIT - MAP_UNIT_OFFSET, 0, "EW_WALL" ) &&
        isShape( mapx, mapy + MAP_UNIT + MAP_UNIT_OFFSET, 0, "CORNER" ) &&
        !scourge->getMap()->getLocation( mapx + dir * ( MAP_UNIT / 2 ), mapy + MAP_UNIT + 1, 0 ) ) {
      //cerr << "Success!" << endl;
      scourge->getMap()->removePosition( mapx, mapy + MAP_UNIT + MAP_UNIT - MAP_UNIT_OFFSET, 0 );
      scourge->getMap()->removePosition( mapx, mapy + MAP_UNIT + MAP_UNIT_OFFSET, 0 );
      scourge->getMap()->
        setPosition( mapx, mapy + MAP_UNIT + MAP_UNIT - MAP_UNIT_OFFSET, 0,
                     scourge->getShapePalette()->
                     findShapeByName( "EW_WALL_EXTRA", true ) );
    }
    //cerr << "Looking south of EW_WALL_EXTRA map=" << mapx << "," << mapy << endl;
    if( isShape( mapx, mapy + MAP_UNIT + MAP_UNIT, 0, "EW_WALL_EXTRA" ) &&
        isShape( mapx, mapy + MAP_UNIT + MAP_UNIT_OFFSET, 0, "CORNER" ) &&
        !scourge->getMap()->getLocation( mapx + dir * ( MAP_UNIT / 2 ), mapy + MAP_UNIT + 1, 0 ) ) {
      //cerr << "Success!" << endl;
      scourge->getMap()->removePosition( mapx, mapy + MAP_UNIT + MAP_UNIT, 0 );
      scourge->getMap()->removePosition( mapx, mapy + MAP_UNIT + MAP_UNIT_OFFSET, 0 );
      scourge->getMap()->
        setPosition( mapx, mapy + MAP_UNIT + MAP_UNIT, 0,
                     scourge->getShapePalette()->
                     findShapeByName( "EW_WALL_TWO_EXTRAS", true ) );
    }
    //cerr << "----------------------------------------------------" << endl;
  }
}

void MapEditor::addNSWall( Sint16 mapx, Sint16 mapy, int dir ) {

  bool east = false;
  bool west = false;

  // short wall
  if( !scourge->getMap()->getLocation( mapx + MAP_UNIT / 2, 
                                       mapy + MAP_UNIT_OFFSET, 
                                       0 ) ) {

    // see if NS_WALL to the east and west
    east = ( mapx + MAP_UNIT < MAP_WIDTH && 
             scourge->getMap()->getLocation( mapx + MAP_UNIT, mapy + MAP_UNIT_OFFSET, 0 ) &&
             !scourge->getMap()->getLocation( mapx + MAP_UNIT - 1, mapy + dir * ( MAP_UNIT / 2 ), 0 ) );
    west = ( mapx - 1 >= 0 && 
             scourge->getMap()->getLocation( mapx - 1, mapy + MAP_UNIT_OFFSET, 0 ) &&
             !scourge->getMap()->getLocation( mapx, mapy + dir * ( MAP_UNIT / 2 ), 0 ) );

    // corner
    if( west ) {
      scourge->getMap()->
        removePosition( mapx, mapy + MAP_UNIT_OFFSET, 0 );
    } else if( !scourge->getMap()->getLocation( mapx, 
                                                mapy + MAP_UNIT_OFFSET,
                                                0 ) ) {
      scourge->getMap()->
        setPosition( mapx, mapy + MAP_UNIT_OFFSET, 0,
                     scourge->getShapePalette()->
                     findShapeByName( "CORNER", true ) );
    }
    
    // corner
    if( east ) {
      scourge->getMap()->
        removePosition( mapx + MAP_UNIT - MAP_UNIT_OFFSET, mapy + MAP_UNIT_OFFSET, 0 );
    } else if( !scourge->getMap()->getLocation( mapx + MAP_UNIT - MAP_UNIT_OFFSET, 
                                                mapy + MAP_UNIT_OFFSET, 
                                                0 ) ) {
      scourge->getMap()->
        setPosition( mapx + MAP_UNIT - MAP_UNIT_OFFSET, mapy + MAP_UNIT_OFFSET, 0,
                     scourge->getShapePalette()->
                     findShapeByName( "CORNER", true ) );
    }


    if( east && west ) {
      scourge->getMap()->setPosition( mapx, mapy + MAP_UNIT_OFFSET, 0,
                                      scourge->getShapePalette()->findShapeByName( "NS_WALL_TWO_EXTRAS", true ) );
    } else if( west ) {
      scourge->getMap()->setPosition( mapx, mapy + MAP_UNIT_OFFSET, 0,
                                      scourge->getShapePalette()->findShapeByName( "NS_WALL_EXTRA", true ) );
    } else if( east ) {
      scourge->getMap()->setPosition( mapx + MAP_UNIT_OFFSET, mapy + MAP_UNIT_OFFSET, 0,
                                      scourge->getShapePalette()->findShapeByName( "NS_WALL_EXTRA", true ) );
    } else {
      scourge->getMap()->setPosition( mapx + MAP_UNIT_OFFSET, mapy + MAP_UNIT_OFFSET, 0,
                                      scourge->getShapePalette()->findShapeByName( "NS_WALL", true ) );
    }


    // change west chunk
    if( isShape( mapx - MAP_UNIT + MAP_UNIT_OFFSET, mapy + MAP_UNIT_OFFSET, 0, "NS_WALL" ) &&
        isShape( mapx - MAP_UNIT_OFFSET, mapy + MAP_UNIT_OFFSET, 0, "CORNER" ) &&
        !scourge->getMap()->getLocation( mapx - MAP_UNIT_OFFSET, mapy + dir * ( MAP_UNIT / 2 ), 0 ) ) {
      scourge->getMap()->removePosition( mapx - MAP_UNIT_OFFSET, mapy + MAP_UNIT_OFFSET, 0 );
      scourge->getMap()->removePosition( mapx - MAP_UNIT + MAP_UNIT_OFFSET, mapy + MAP_UNIT_OFFSET, 0 );
      scourge->getMap()->
        setPosition( mapx - MAP_UNIT + MAP_UNIT_OFFSET, mapy + MAP_UNIT_OFFSET, 0, 
                     scourge->getShapePalette()->findShapeByName( "NS_WALL_EXTRA", true ) );
    }
    if( isShape( mapx - MAP_UNIT, mapy + MAP_UNIT_OFFSET, 0, "NS_WALL_EXTRA" ) &&
        isShape( mapx - MAP_UNIT_OFFSET, mapy + MAP_UNIT_OFFSET, 0, "CORNER" ) &&
        !scourge->getMap()->getLocation( mapx - MAP_UNIT_OFFSET, mapy + dir * ( MAP_UNIT / 2 ), 0 ) ) {
      scourge->getMap()->removePosition( mapx - MAP_UNIT_OFFSET, mapy + MAP_UNIT_OFFSET, 0 );
      scourge->getMap()->removePosition( mapx - MAP_UNIT, mapy + MAP_UNIT_OFFSET, 0 );
      scourge->getMap()->
        setPosition( mapx - MAP_UNIT, mapy + MAP_UNIT_OFFSET, 0, 
                     scourge->getShapePalette()->findShapeByName( "NS_WALL_TWO_EXTRAS", true ) );
    }

    // change the east chunk
    if( isShape( mapx + MAP_UNIT + MAP_UNIT_OFFSET, mapy + MAP_UNIT_OFFSET, 0, "NS_WALL" ) &&
        isShape( mapx + MAP_UNIT, mapy + MAP_UNIT_OFFSET, 0, "CORNER" ) &&
        !scourge->getMap()->getLocation( mapx + MAP_UNIT, mapy + dir * ( MAP_UNIT / 2 ), 0 ) ) {
      scourge->getMap()->removePosition( mapx + MAP_UNIT + MAP_UNIT_OFFSET, mapy + MAP_UNIT_OFFSET, 0 );
      scourge->getMap()->removePosition( mapx + MAP_UNIT, mapy + MAP_UNIT_OFFSET, 0 );
      scourge->getMap()->
        setPosition( mapx + MAP_UNIT, mapy + MAP_UNIT_OFFSET, 0,
                     scourge->getShapePalette()->
                     findShapeByName( "NS_WALL_EXTRA", true ) );
    }
    if( isShape( mapx + MAP_UNIT + MAP_UNIT_OFFSET, mapy + MAP_UNIT_OFFSET, 0, "NS_WALL_EXTRA" ) &&
        isShape( mapx + MAP_UNIT, mapy + MAP_UNIT_OFFSET, 0, "CORNER" ) &&
        !scourge->getMap()->getLocation( mapx + MAP_UNIT, mapy + dir * ( MAP_UNIT / 2 ), 0 ) ) {
      scourge->getMap()->removePosition( mapx + MAP_UNIT + MAP_UNIT_OFFSET, mapy + MAP_UNIT_OFFSET, 0 );
      scourge->getMap()->removePosition( mapx + MAP_UNIT, mapy + MAP_UNIT_OFFSET, 0 );
      scourge->getMap()->
        setPosition( mapx + MAP_UNIT, mapy + MAP_UNIT_OFFSET, 0,
                     scourge->getShapePalette()->
                     findShapeByName( "NS_WALL_TWO_EXTRAS", true ) );
    }
  }
}

bool MapEditor::isShape( Sint16 mapx, Sint16 mapy, Sint16 mapz, const char *name ) {
  //cerr << "\ttesting map=" << mapx << "," << mapy << " looking for " << name;
  if( mapx >= 0 && mapx < MAP_WIDTH &&
      mapy >= 0 && mapy < MAP_DEPTH &&
      mapz >= 0 && mapz < MAP_VIEW_HEIGHT ) {
    Location *pos = pos = scourge->getMap()->getPosition( mapx, mapy, mapz );
    //cerr << " found=" << ( !pos ? "NULL" : pos->shape->getName() ) << endl;
    return( pos && !strcmp( pos->shape->getName(), name ) );
  } else {
    //cerr << " found nothing." << endl;
    return false;
  }
}

void MapEditor::addFloor( Sint16 mapx, Sint16 mapy ) {
  if( scourge->getMap()->getFloorPosition( mapx, mapy + MAP_UNIT ) ) return;
  scourge->getMap()->setFloorPosition( mapx, mapy + MAP_UNIT, 
                                       scourge->getShapePalette()->findShapeByName( "FLOOR_TILE", true ) );
}

void MapEditor::removeEWWall( Sint16 mapx, Sint16 mapy, int dir ) {
  for( int y = 1; y <= MAP_UNIT; y++ ) {
    scourge->getMap()->removePosition( mapx, mapy + y, 0 );
  }
}

void MapEditor::removeNSWall( Sint16 mapx, Sint16 mapy, int dir ) {
  for( int x = 0; x < MAP_UNIT; x++ ) {
    scourge->getMap()->removePosition( mapx + x, mapy + MAP_UNIT_OFFSET, 0 );
  }
}

