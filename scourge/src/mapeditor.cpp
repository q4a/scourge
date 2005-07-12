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
  
  
/*
  TODO:
  - separate lists: items, creatures, interactive things, shapes
  - before putting a shape down, check that it fits
  - putting shapes down should use Map::isBlocked() to find Z
  - when using "removePosition" make sure to call with shape's hotspot
  - how to edit items/shapes?
  - need to call ShapePalette::decrementSkinRefCount() when closing map?
*/  
  
bool contains( vector<Shape*> *seen, Shape *shape ) {
  for( int i = 0; i < (int)seen->size(); i++ ) {
    if( (*seen)[i] == shape ) return true;
  }
  return false;
}

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


  // Lists
  vector<Shape*> seen;

  // items
  itemButton = mainWin->createButton( 5, 110, w - 10, 130, "Item", true );
  toggleButtonList.push_back( itemButton );
  itemList = new ScrollingList( 5, 140, w - 10, 140, 
                                scourge->getShapePalette()->getHighlightTexture() );
  mainWin->addWidget( itemList );
  map<string, const RpgItem *> *itemMap = RpgItem::getItemMap();
  itemNames = (char**)malloc( itemMap->size() * sizeof(char*) );
  int count = 0;
  for (map<string, const RpgItem*>::iterator i = itemMap->begin(); 
        i != itemMap->end(); ++i ) {
    string name = i->first;
    RpgItem *item = (RpgItem*)( i->second );
    Shape *shape = scourge->getShapePalette()->getShape( item->getShapeIndex() );
    seen.push_back( shape );
    char *p = (char*)name.c_str();
    itemNames[ count ] = (char*)malloc( 120 * sizeof(char) );
    strcpy( itemNames[ count ], p );
    count++;
  }
  itemList->setLines( itemMap->size(), (const char**)itemNames );

  
  // creatures
  creatureButton = mainWin->createButton( 5, 260, w - 10, 280, "Creature", true );
  toggleButtonList.push_back( creatureButton );
  creatureList = new ScrollingList( 5, 290, w - 10, 140, 
                                    scourge->getShapePalette()->getHighlightTexture() );
  mainWin->addWidget( creatureList );
  map<string, Monster*> *creatureMap = &(Monster::monstersByName);
  creatureNames = (char**)malloc( creatureMap->size() * sizeof(char*) );
  count = 0;
  for (map<string, Monster*>::iterator i = creatureMap->begin(); 
        i != creatureMap->end(); ++i ) {
    string name = i->first;
    Monster *monster = (Monster*)( i->second );
    GLShape *shape = scourge->getSession()->getShapePalette()->
      getCreatureShape(monster->getModelName(), 
                       monster->getSkinName(), 
                       monster->getScale(),
                       monster);
    seen.push_back( shape );
    char *p = (char*)name.c_str();
    creatureNames[ count ] = (char*)malloc( 120 * sizeof(char) );
    strcpy( creatureNames[ count ], p );
    count++;
  }
  creatureList->setLines( creatureMap->size(), (const char**)creatureNames );

  // shapes
  shapeButton = mainWin->createButton( 5, 450, w - 10, 470, "Shape", true );
  toggleButtonList.push_back( shapeButton );
  shapeList = new ScrollingList( 5, 480, w - 10, 140, 
                                 scourge->getShapePalette()->getHighlightTexture() );
  mainWin->addWidget( shapeList );
  map< string, GLShape* > *shapeMap = scourge->getShapePalette()->getShapeMap();
  shapeNames = (char**)malloc( shapeMap->size() * sizeof(char*) );
  count = 0;
  for (map<string, GLShape*>::iterator i = shapeMap->begin(); i != shapeMap->end(); ++i ) {
    string name = i->first;
    GLShape *shape = i->second;
    if( !contains( &seen, shape ) ) {
      char *p = (char*)name.c_str();
      shapeNames[ count ] = (char*)malloc( 120 * sizeof(char) );
      strcpy( shapeNames[ count ], p );
      count++;
    }
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

  Location *pos = scourge->getMap()->getLocation( scourge->getMap()->getCursorFlatMapX(), 
                                                  scourge->getMap()->getCursorFlatMapY(),
                                                  0 );
  scourge->getSDLHandler()->texPrint( 50, 120, "F:%d,%d C:%d,%d Shape=%s", 
                                      scourge->getMap()->getCursorFlatMapX(), 
                                      scourge->getMap()->getCursorFlatMapY(), 
                                      scourge->getMap()->getCursorChunkX(), 
                                      scourge->getMap()->getCursorChunkY(),
                                      ( pos ? pos->shape->getName() : "NULL" ) );
  
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
    int xx = scourge->getMap()->getCursorFlatMapX();
    int yy = scourge->getMap()->getCursorFlatMapY();

    int mapx = scourge->getMap()->getCursorChunkX() * MAP_UNIT + MAP_OFFSET;
    int mapy = scourge->getMap()->getCursorChunkY() * MAP_UNIT + MAP_OFFSET;

    int innerX = xx - mapx;
    int innerY = yy - mapy;

//    cerr << "pos: " << mapx << "," << mapy << 
//      " map:" << scourge->getMap()->getCursorFlatMapX() << "," << 
//      scourge->getMap()->getCursorFlatMapX() << endl;
    
    // find the region in the chunk
    int mx = -1;
    int my = -1;
    int dir = -1;
    if( innerX < MAP_UNIT_OFFSET ) { 
      mx = mapx;
      my = mapy;
      dir = Constants::WEST;
    } else if( innerY < MAP_UNIT_OFFSET ) { 
      mx = mapx;
      my = mapy;
      dir = Constants::NORTH;
    } else if( innerX >= MAP_UNIT - MAP_UNIT_OFFSET ) {
      mx = mapx + MAP_UNIT - MAP_UNIT_OFFSET;
      my = mapy;
      dir = Constants::EAST;
    } else if( innerY >= MAP_UNIT - MAP_UNIT_OFFSET ) {
      mx = mapx;
      my = mapy + MAP_UNIT - MAP_UNIT_OFFSET;
      dir = Constants::SOUTH;
    }

    if( dir != -1 ) {
      if( button == SDL_BUTTON_RIGHT ) {
        removeWall( mx, my, dir ); 
      } else if( wallButton->isSelected() ) {
        addWall( mx, my, dir ); 
      } else if( doorButton->isSelected() ) {
        addDoor( mx, my, dir );
      }

      // blend the corners
      for( int x = -1; x <= 1; x++ ) {
        for( int y = -1; y <= 1; y++ ) {
          blendCorners( mapx + ( x * MAP_UNIT ), 
                        mapy + ( y * MAP_UNIT ) );
        }
      }
    } else if( creatureButton->isSelected() ) {
      if( button == SDL_BUTTON_LEFT && 
          creatureList->getSelectedLine() > -1 ) {
        Monster *monster = Monster::getMonsterByName( creatureNames[ creatureList->getSelectedLine() ] );
        GLShape *shape = scourge->getSession()->getShapePalette()->
          getCreatureShape(monster->getModelName(), 
                           monster->getSkinName(), 
                           monster->getScale(),
                           monster);
        scourge->getMap()->setPosition( xx, yy, 0, shape );
      } else if( button == SDL_BUTTON_RIGHT ) {
        scourge->getMap()->removePosition( xx, yy, 0 );
      }
    } else if( itemButton->isSelected() ) {
      if( button == SDL_BUTTON_LEFT && 
          itemList->getSelectedLine() > -1 ) {
        RpgItem *rpgItem = 
          RpgItem::getItemByName( itemNames[ itemList->getSelectedLine() ] );
        Shape *shape = scourge->getShapePalette()->getShape( rpgItem->getShapeIndex() );
        scourge->getMap()->setPosition( xx, yy, 0, shape );
      } else if( button == SDL_BUTTON_RIGHT ) {
        scourge->getMap()->removePosition( xx, yy, 0 );
      }
    } else if( shapeButton->isSelected() ) {
      if( button == SDL_BUTTON_LEFT && 
          shapeList->getSelectedLine() > -1 ) {
        Shape *shape = scourge->getShapePalette()->
          findShapeByName( shapeNames[ shapeList->getSelectedLine() ] );
        scourge->getMap()->setPosition( xx, yy, 0, shape );
      } else if( button == SDL_BUTTON_RIGHT ) {
        scourge->getMap()->removePosition( xx, yy, 0 );
      }
    } else {
      if( button == SDL_BUTTON_RIGHT ) {
        removeFloor( mapx, mapy );
      } else {
        addFloor( mapx, mapy );
      }
    }
  }
}

void MapEditor::addWall( Sint16 mapx, Sint16 mapy, int dir ) {
  switch( dir ) {
  case Constants::NORTH: addNSWall( mapx, mapy, 1 ); break;
  case Constants::SOUTH: addNSWall( mapx, mapy, -1 ); break;
  case Constants::WEST: addEWWall( mapx, mapy, 1 ); break;
  case Constants::EAST: addEWWall( mapx, mapy, -1 ); break;
  default: cerr << "*** addWall, Unknown dir=" << dir << endl;
  }
}

void MapEditor::addDoor( Sint16 mapx, Sint16 mapy, int dir ) {
  switch( dir ) {
  case Constants::NORTH: addNSDoor( mapx, mapy, 1 ); break;
  case Constants::SOUTH: addNSDoor( mapx, mapy, -1 ); break;
  case Constants::WEST: addEWDoor( mapx, mapy, 1 ); break;
  case Constants::EAST: addEWDoor( mapx, mapy, -1 ); break;
  default: cerr << "*** addDoor, Unknown dir=" << dir << endl;
  }
}

void MapEditor::removeWall( Sint16 mapx, Sint16 mapy, int dir ) {
  switch( dir ) {
  case Constants::NORTH: removeNSWall( mapx, mapy, 1 ); break;
  case Constants::SOUTH: removeNSWall( mapx, mapy, -1 ); break;
  case Constants::WEST: removeEWWall( mapx, mapy, 1 ); break;
  case Constants::EAST: removeEWWall( mapx, mapy, -1 ); break;
  default: cerr << "*** removeWall, Unknown dir=" << dir << endl;
  }
}

void MapEditor::addEWDoor( Sint16 mapx, Sint16 mapy, int dir ) {
  ShapePalette *shapePal = scourge->getShapePalette();
  if( !scourge->getMap()->getLocation( mapx, mapy + MAP_UNIT / 2, 0 ) ) {
    scourge->getMap()->setPosition(mapx, mapy + MAP_UNIT - MAP_UNIT_OFFSET, 
                                   MAP_WALL_HEIGHT - 2, shapePal->findShapeByName("EW_DOOR_TOP"));
    scourge->getMap()->setPosition(mapx, mapy + MAP_UNIT_OFFSET +  2, 
                                   0, shapePal->findShapeByName("DOOR_SIDE"));
    scourge->getMap()->setPosition(mapx, mapy + MAP_UNIT_OFFSET * 2 +  2, 
                                   0, shapePal->findShapeByName("DOOR_SIDE"));
    scourge->getMap()->setPosition(mapx + 1, mapy + MAP_UNIT - MAP_UNIT_OFFSET - 2, 
                                   0, shapePal->findShapeByName("EW_DOOR"));
    scourge->getMap()->setPosition(mapx, mapy + MAP_UNIT - MAP_UNIT_OFFSET, 
                                   0, shapePal->findShapeByName("DOOR_SIDE"));

    // corners
    if( !scourge->getMap()->getLocation( mapx, mapy + MAP_UNIT_OFFSET, 0 ) ) {
      scourge->getMap()->setPosition( mapx, mapy + MAP_UNIT_OFFSET, 0, 
                                      shapePal->findShapeByName("CORNER"));
    }
    if( !scourge->getMap()->getLocation( mapx, mapy + MAP_UNIT, 0 ) ) {
      scourge->getMap()->setPosition( mapx, mapy + MAP_UNIT, 0, 
                                      shapePal->findShapeByName("CORNER"));
    }
  }
}

void MapEditor::addNSDoor( Sint16 mapx, Sint16 mapy, int dir ) {
  ShapePalette *shapePal = scourge->getShapePalette();
  if( !scourge->getMap()->getLocation( mapx + MAP_UNIT / 2, 
                                       mapy + MAP_UNIT_OFFSET, 
                                       0 ) ) {
    scourge->getMap()->setPosition(mapx + MAP_UNIT_OFFSET, mapy + MAP_UNIT_OFFSET, 
                                   MAP_WALL_HEIGHT - 2, shapePal->findShapeByName("NS_DOOR_TOP"));
    scourge->getMap()->setPosition(mapx + MAP_UNIT_OFFSET, mapy + MAP_UNIT_OFFSET, 
                                   0, shapePal->findShapeByName("DOOR_SIDE"));
    scourge->getMap()->setPosition(mapx + MAP_UNIT_OFFSET * 2, mapy + MAP_UNIT_OFFSET - 1, 
                                   0, shapePal->findShapeByName("NS_DOOR"));
    scourge->getMap()->setPosition(mapx + MAP_UNIT - MAP_UNIT_OFFSET * 2, mapy + MAP_UNIT_OFFSET, 
                                   0, shapePal->findShapeByName("DOOR_SIDE"));
    scourge->getMap()->setPosition(mapx + MAP_UNIT - MAP_UNIT_OFFSET * 3, mapy + MAP_UNIT_OFFSET, 
                                   0, shapePal->findShapeByName("DOOR_SIDE"));

    // corners
    if( !scourge->getMap()->getLocation( mapx, mapy + MAP_UNIT_OFFSET, 0 ) ) {
      scourge->getMap()->setPosition( mapx, mapy + MAP_UNIT_OFFSET, 0, 
                                      shapePal->findShapeByName("CORNER"));
    }
    if( !scourge->getMap()->getLocation( mapx + MAP_UNIT - MAP_UNIT_OFFSET, mapy + MAP_UNIT_OFFSET, 0 ) ) {
      scourge->getMap()->setPosition( mapx + MAP_UNIT - MAP_UNIT_OFFSET, mapy + MAP_UNIT_OFFSET, 0, 
                                      shapePal->findShapeByName("CORNER"));
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

void MapEditor::addFloor( Sint16 mapx, Sint16 mapy ) {
  if( scourge->getMap()->getFloorPosition( mapx, mapy + MAP_UNIT ) ) return;
  scourge->getMap()->setFloorPosition( mapx, mapy + MAP_UNIT, 
                                       scourge->getShapePalette()->findShapeByName( "FLOOR_TILE", true ) );
}

void MapEditor::removeFloor( Sint16 mapx, Sint16 mapy ) {
  scourge->getMap()->removeFloorPosition( mapx, mapy + MAP_UNIT );
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

void MapEditor::blendCorners( Sint16 mapx, Sint16 mapy ) {

  if( !( mapx >= 0 && mapx < MAP_WIDTH &&
         mapy >= 0 && mapy < MAP_DEPTH ) ) 
    return;


  Map *levelMap = scourge->getMap();
  ShapePalette *pal = scourge->getShapePalette();

  // check NW corner
  if( isShape( mapx - 1, mapy + 1, 0, "CORNER" ) &&
      isShape( mapx, mapy - 1, 0, "CORNER" ) &&
      !levelMap->getLocation( mapx, mapy + MAP_UNIT_OFFSET, 0 ) ) {

    bool nsWall = ( isShape( mapx - MAP_UNIT + MAP_UNIT_OFFSET, mapy + MAP_UNIT_OFFSET, 0, "NS_WALL" ) ? true : false );
    bool nsWallExtra = ( isShape( mapx - MAP_UNIT, mapy + MAP_UNIT_OFFSET, 0, "NS_WALL_EXTRA" ) ? true : false );
    bool ewWall = ( isShape( mapx, mapy - MAP_UNIT_OFFSET, 0, "EW_WALL" ) ? true : false );
    bool ewWallExtra = ( isShape( mapx, mapy - MAP_UNIT_OFFSET, 0, "EW_WALL_EXTRA" ) ? true : false );

    if( ( nsWall || nsWallExtra ) && ( ewWall || ewWallExtra ) ) {

      levelMap->setPosition( mapx, mapy + MAP_UNIT_OFFSET, 0, 
                             pal->findShapeByName( "CORNER", true ) );
      levelMap->removePosition( mapx - 1, mapy + 1, 0 );
      levelMap->removePosition( mapx, mapy - 1, 0 );
    
      // change west chunk
      if( nsWall ) {
        levelMap->removePosition( mapx - MAP_UNIT + MAP_UNIT_OFFSET, mapy + MAP_UNIT_OFFSET, 0 );
        levelMap->setPosition( mapx - MAP_UNIT + MAP_UNIT_OFFSET, mapy + MAP_UNIT_OFFSET, 0,
                               pal->findShapeByName( "NS_WALL_EXTRA", true ) );
      } else if( nsWallExtra ) {
        levelMap->removePosition( mapx - MAP_UNIT, mapy + MAP_UNIT_OFFSET, 0 );
        levelMap->setPosition( mapx - MAP_UNIT, mapy + MAP_UNIT_OFFSET, 0,
                               pal->findShapeByName( "NS_WALL_TWO_EXTRAS", true ) );
      }
      
      // change north chunk
      if( ewWall ) {
        levelMap->removePosition( mapx, mapy - MAP_UNIT_OFFSET, 0 );
        levelMap->setPosition( mapx, mapy, 0,
                               pal->findShapeByName( "EW_WALL_EXTRA", true ) );
      } else if( ewWallExtra ) {
        levelMap->removePosition( mapx, mapy - MAP_UNIT_OFFSET, 0 );
        levelMap->setPosition( mapx, mapy, 0,
                               pal->findShapeByName( "EW_WALL_TWO_EXTRAS", true ) );
      }      
    }
  }

  // check NE corner
  if( isShape( mapx + MAP_UNIT, mapy + 1, 0, "CORNER" ) &&
      isShape( mapx + MAP_UNIT - MAP_UNIT_OFFSET, mapy - 1, 0, "CORNER" ) &&
      !levelMap->getLocation( mapx + MAP_UNIT - MAP_UNIT_OFFSET, mapy + MAP_UNIT_OFFSET, 0 ) ) {

    bool nsWall = ( isShape( mapx + MAP_UNIT + MAP_UNIT_OFFSET, mapy + MAP_UNIT_OFFSET, 0, "NS_WALL" ) ? true : false );
    bool nsWallExtra = ( isShape( mapx + MAP_UNIT + MAP_UNIT_OFFSET, mapy + MAP_UNIT_OFFSET, 0, "NS_WALL_EXTRA" ) ? true : false );
    bool ewWall = ( isShape( mapx + MAP_UNIT - MAP_UNIT_OFFSET, mapy - MAP_UNIT_OFFSET, 0, "EW_WALL" ) ? true : false );
    bool ewWallExtra = ( isShape( mapx + MAP_UNIT - MAP_UNIT_OFFSET, mapy - MAP_UNIT_OFFSET, 0, "EW_WALL_EXTRA" ) ? true : false );

    if( ( nsWall || nsWallExtra ) && ( ewWall || ewWallExtra ) ) {

      levelMap->setPosition( mapx + MAP_UNIT - MAP_UNIT_OFFSET, mapy + MAP_UNIT_OFFSET, 0, 
                             pal->findShapeByName( "CORNER", true ) );
      levelMap->removePosition( mapx + MAP_UNIT, mapy + 1, 0 );
      levelMap->removePosition( mapx + MAP_UNIT - MAP_UNIT_OFFSET, mapy - 1, 0 );
    
      // change west chunk
      if( nsWall ) {
        levelMap->removePosition( mapx + MAP_UNIT + MAP_UNIT_OFFSET, mapy + MAP_UNIT_OFFSET, 0 );
        levelMap->setPosition( mapx + MAP_UNIT, mapy + MAP_UNIT_OFFSET, 0,
                               pal->findShapeByName( "NS_WALL_EXTRA", true ) );
      } else if( nsWallExtra ) {
        levelMap->removePosition( mapx + MAP_UNIT + MAP_UNIT_OFFSET, mapy + MAP_UNIT_OFFSET, 0 );
        levelMap->setPosition( mapx + MAP_UNIT, mapy + MAP_UNIT_OFFSET, 0,
                               pal->findShapeByName( "NS_WALL_TWO_EXTRAS", true ) );
      }
      
      // change north chunk
      if( ewWall ) {
        levelMap->removePosition( mapx + MAP_UNIT - MAP_UNIT_OFFSET, mapy - MAP_UNIT_OFFSET, 0 );
        levelMap->setPosition( mapx + MAP_UNIT - MAP_UNIT_OFFSET, mapy, 0,
                               pal->findShapeByName( "EW_WALL_EXTRA", true ) );
      } else if( ewWallExtra ) {
        levelMap->removePosition( mapx + MAP_UNIT - MAP_UNIT_OFFSET, mapy - MAP_UNIT_OFFSET, 0 );
        levelMap->setPosition( mapx + MAP_UNIT - MAP_UNIT_OFFSET, mapy, 0,
                               pal->findShapeByName( "EW_WALL_TWO_EXTRAS", true ) );
      }      
    }
  }

  // check SE corner
  if( isShape( mapx + MAP_UNIT, mapy + MAP_UNIT, 0, "CORNER" ) &&
      isShape( mapx + MAP_UNIT - MAP_UNIT_OFFSET, mapy + MAP_UNIT + MAP_UNIT_OFFSET, 0, "CORNER" ) &&
      !levelMap->getLocation( mapx + MAP_UNIT - MAP_UNIT_OFFSET, mapy + MAP_UNIT, 0 ) ) {

    bool nsWall = ( isShape( mapx + MAP_UNIT + MAP_UNIT_OFFSET, mapy + MAP_UNIT, 0, "NS_WALL" ) ? true : false );
    bool nsWallExtra = ( isShape( mapx + MAP_UNIT + MAP_UNIT_OFFSET, mapy + MAP_UNIT, 0, "NS_WALL_EXTRA" ) ? true : false );
    bool ewWall = ( isShape( mapx + MAP_UNIT - MAP_UNIT_OFFSET, mapy + MAP_UNIT + MAP_UNIT - MAP_UNIT_OFFSET, 0, "EW_WALL" ) ? true : false );
    bool ewWallExtra = ( isShape( mapx + MAP_UNIT - MAP_UNIT_OFFSET, mapy + MAP_UNIT + MAP_UNIT, 0, "EW_WALL_EXTRA" ) ? true : false );

    if( ( nsWall || nsWallExtra ) && ( ewWall || ewWallExtra ) ) {

      levelMap->setPosition( mapx + MAP_UNIT - MAP_UNIT_OFFSET, mapy + MAP_UNIT, 0, 
                             pal->findShapeByName( "CORNER", true ) );
      levelMap->removePosition( mapx + MAP_UNIT, mapy + MAP_UNIT_OFFSET, 0 );
      levelMap->removePosition( mapx + MAP_UNIT - MAP_UNIT_OFFSET, mapy + MAP_UNIT + MAP_UNIT_OFFSET, 0 );
    
      // change west chunk
      if( nsWall ) {
        levelMap->removePosition( mapx + MAP_UNIT + MAP_UNIT_OFFSET, mapy + MAP_UNIT, 0 );
        levelMap->setPosition( mapx + MAP_UNIT, mapy + MAP_UNIT, 0,
                               pal->findShapeByName( "NS_WALL_EXTRA", true ) );
      } else if( nsWallExtra ) {
        levelMap->removePosition( mapx + MAP_UNIT + MAP_UNIT_OFFSET, mapy + MAP_UNIT, 0 );
        levelMap->setPosition( mapx + MAP_UNIT, mapy + MAP_UNIT, 0,
                               pal->findShapeByName( "NS_WALL_TWO_EXTRAS", true ) );
      }
      
      // change north chunk
      if( ewWall ) {
        levelMap->removePosition( mapx + MAP_UNIT - MAP_UNIT_OFFSET, mapy + MAP_UNIT + MAP_UNIT - MAP_UNIT_OFFSET, 0 );
        levelMap->setPosition( mapx + MAP_UNIT - MAP_UNIT_OFFSET, mapy + MAP_UNIT + MAP_UNIT - MAP_UNIT_OFFSET, 0,
                               pal->findShapeByName( "EW_WALL_EXTRA", true ) );
      } else if( ewWallExtra ) {
        levelMap->removePosition( mapx + MAP_UNIT - MAP_UNIT_OFFSET, mapy + MAP_UNIT + MAP_UNIT, 0 );
        levelMap->setPosition( mapx + MAP_UNIT - MAP_UNIT_OFFSET, mapy + MAP_UNIT + MAP_UNIT, 0,
                               pal->findShapeByName( "EW_WALL_TWO_EXTRAS", true ) );
      }      
    }
  }

  // check SW corner
  if( isShape( mapx - MAP_UNIT_OFFSET, mapy + MAP_UNIT, 0, "CORNER" ) &&
      isShape( mapx, mapy + MAP_UNIT + MAP_UNIT_OFFSET, 0, "CORNER" ) &&
      !levelMap->getLocation( mapx, mapy + MAP_UNIT, 0 ) ) {

    bool nsWall = ( isShape( mapx - MAP_UNIT + MAP_UNIT_OFFSET, mapy + MAP_UNIT, 0, "NS_WALL" ) ? true : false );
    bool nsWallExtra = ( isShape( mapx - MAP_UNIT, mapy + MAP_UNIT, 0, "NS_WALL_EXTRA" ) ? true : false );
    bool ewWall = ( isShape( mapx, mapy + MAP_UNIT + MAP_UNIT - MAP_UNIT_OFFSET, 0, "EW_WALL" ) ? true : false );
    bool ewWallExtra = ( isShape( mapx, mapy + MAP_UNIT + MAP_UNIT, 0, "EW_WALL_EXTRA" ) ? true : false );

    if( ( nsWall || nsWallExtra ) && ( ewWall || ewWallExtra ) ) {

      levelMap->setPosition( mapx, mapy + MAP_UNIT, 0, 
                             pal->findShapeByName( "CORNER", true ) );
      levelMap->removePosition( mapx - MAP_UNIT_OFFSET, mapy + MAP_UNIT, 0 );
      levelMap->removePosition( mapx, mapy + MAP_UNIT + MAP_UNIT_OFFSET, 0 );
    
      // change west chunk
      if( nsWall ) {
        levelMap->removePosition( mapx - MAP_UNIT + MAP_UNIT_OFFSET, mapy + MAP_UNIT, 0 );
        levelMap->setPosition( mapx - MAP_UNIT + MAP_UNIT_OFFSET, mapy + MAP_UNIT, 0,
                               pal->findShapeByName( "NS_WALL_EXTRA", true ) );
      } else if( nsWallExtra ) {
        levelMap->removePosition( mapx - MAP_UNIT, mapy + MAP_UNIT, 0 );
        levelMap->setPosition( mapx - MAP_UNIT, mapy + MAP_UNIT, 0,
                               pal->findShapeByName( "NS_WALL_TWO_EXTRAS", true ) );
      }
      
      // change north chunk
      if( ewWall ) {
        levelMap->removePosition( mapx, mapy + MAP_UNIT + MAP_UNIT - MAP_UNIT_OFFSET, 0 );
        levelMap->setPosition( mapx, mapy + MAP_UNIT + MAP_UNIT - MAP_UNIT_OFFSET, 0,
                               pal->findShapeByName( "EW_WALL_EXTRA", true ) );
      } else if( ewWallExtra ) {
        levelMap->removePosition( mapx, mapy + MAP_UNIT + MAP_UNIT, 0 );
        levelMap->setPosition( mapx, mapy + MAP_UNIT + MAP_UNIT, 0,
                               pal->findShapeByName( "EW_WALL_TWO_EXTRAS", true ) );
      }      
    }
  }
}

bool MapEditor::isShape( Sint16 mapx, Sint16 mapy, Sint16 mapz, const char *name ) {
//  cerr << "\ttesting map=" << mapx << "," << mapy << " looking for " << name;
  if( mapx >= 0 && mapx < MAP_WIDTH &&
      mapy >= 0 && mapy < MAP_DEPTH &&
      mapz >= 0 && mapz < MAP_VIEW_HEIGHT ) {
    Location *pos = pos = scourge->getMap()->getLocation( mapx, mapy, mapz );
//    cerr << " found=" << ( !pos ? "NULL" : pos->shape->getName() ) << endl;
    return( pos && !strcmp( pos->shape->getName(), name ) );
  } else {
//    cerr << " found nothing." << endl;
    return false;
  }
}
