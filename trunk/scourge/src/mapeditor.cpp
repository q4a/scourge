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
#include "render/renderlib.h"
#include "rpg/rpglib.h"
#include "mapwidget.h"
#include "sdlhandler.h"
#include "scourge.h"
#include "partyeditor.h"
#include "item.h"
#include "creature.h"

using namespace std;

char *floorTypeName[80] = { 
  "FLOOR_TILE",
  "ROOM_FLOOR_TILE"
};

/**
  *@author Gabor Torok
  */
  
  
/*
  TODO:
  - ability to "name" item, shape, creature. Then text file can reference name.
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

  // default map settings
  level = 1;
  depth = 0;

  mapSettings = new EditorMapSettings();

  int w = 200;
  mainWin = new Window( scourge->getSDLHandler(),
                        scourge->getScreenWidth() - w, 0,
                        w, scourge->getScreenHeight(),
                        "Map Editor", false, Window::BASIC_WINDOW,
                        GuiTheme::DEFAULT_THEME );
  mainWin->setVisible( false );
  mainWin->setLocked( true );
  
  doneButton = mainWin->createButton( 5, 5, w - 10, 20, "Done" );

  wallButton = mainWin->createButton( 5, 25, w - 10, 45, "Wall", true );
  toggleButtonList.push_back( wallButton );
  wallButton->setSelected( true );
  doorButton = mainWin->createButton( 5, 50, w - 10, 70, "Door", true );
  toggleButtonList.push_back( doorButton );

  mainWin->createLabel( 5, 87, "Name:" );
  nameText = mainWin->createTextField( 60, 75, 10 );
  loadButton = mainWin->createButton( 5, 100, ( w - 10 ) / 2, 120, "Load" );
  saveButton = mainWin->createButton( ( w - 10 ) / 2 + 5, 100, w - 5, 120, "Save" );

  newButton = mainWin->createButton( 5, 125, w - 10, 145, "Map Properties" );
  floorType = mainWin->createButton( 5, 150, w - 10, 170, floorTypeName[ 1 ], true );

  startPosButton = mainWin->createButton( 5, 175, w - 10, 195, "Starting Position", true );
  toggleButtonList.push_back( startPosButton );

  mapButton = mainWin->createButton( 5, 200, w - 10, 220, "Map Location" );

  // new map ui
  int nw = 450;
  int nh = 400;
  newMapWin = new Window( scourge->getSDLHandler(),
                          40, 40, nw, nh,
                          "Map Properties", 
                          false, 
                          Window::BASIC_WINDOW,
                          GuiTheme::DEFAULT_THEME );
  newMapWin->setVisible( false );
  newMapWin->setModal( true );
  newMapWin->createLabel( 5, 20, "Map level (0-7):" );
  levelText = newMapWin->createTextField( 150, 10, 20 );
  newMapWin->createLabel( 5, 40, "Map depth (0-10):" );
  depthText = newMapWin->createTextField( 150, 30, 20 );
  newMapWin->createLabel( 5, 60, "Map theme:" );


  themeList = new ScrollingList( 150, 50, 220, 60, 
                                 scourge->getShapePalette()->getHighlightTexture() );
  newMapWin->addWidget( themeList );
  themeNames = (char**)malloc( scourge->getShapePalette()->getAllThemeCount() * 
                               sizeof(char*) );
  for( int i = 0; i < scourge->getShapePalette()->getAllThemeCount(); i++ ) {
    themeNames[ i ] = (char*)malloc( 120 * sizeof(char) );
    strcpy( themeNames[ i ], 
            scourge->getShapePalette()->getAllThemeName( i ) );
    if( scourge->getShapePalette()->isThemeSpecial( i ) )
      strcat( themeNames[i], "(S)" );
  }
  themeList->setLines( scourge->getShapePalette()->getAllThemeCount(), 
                       (const char**)themeNames );

  newMapWin->createLabel( 5, 130, "Select map location: (click on map, drag to move)" );
  mapWidget = new MapWidget( scourge, newMapWin, 5, 140, nw - 5, 335 );
  newMapWin->addWidget( mapWidget->getCanvas() );

  int bw = nw / 6;
  okButton = newMapWin->createButton( nw - bw * 3 - 10, 345, nw - bw * 2 - 5, 365, "New Map" );
  applyButton = newMapWin->createButton( nw - bw * 2 + 5, 345, nw - bw - 5, 365, "Apply" );
  cancelButton = newMapWin->createButton( nw - bw, 345, nw - 5, 365, "Cancel" );

  // Lists
  vector<Shape*> seen;
  int h = 240;
  int d = 150;

  // items
  itemButton = mainWin->createButton( 5, h, w - 10, h + 20, "Item", true );
  toggleButtonList.push_back( itemButton );
  itemList = new ScrollingList( 5, h + 30, w - 10, 100, 
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
  h += d;
  
  // creatures
  creatureButton = mainWin->createButton( 5, h, w - 10, h + 20, "Creature", true );
  toggleButtonList.push_back( creatureButton );
  creatureList = new ScrollingList( 5, h + 30, w - 10, 100, 
                                    scourge->getShapePalette()->getHighlightTexture() );
  mainWin->addWidget( creatureList );
  map<string, Monster*> *creatureMap = &(Monster::monstersByName);
  creatureNames = (char**)malloc( creatureMap->size() * sizeof(char*) );
  count = 0;
  for (map<string, Monster*>::iterator i = creatureMap->begin(); 
        i != creatureMap->end(); ++i ) {
    string name = i->first;
    /*
    Monster *monster = (Monster*)( i->second );
    GLShape *shape = scourge->getSession()->getShapePalette()->
      getCreatureShape(monster->getModelName(), 
                       monster->getSkinName(), 
                       monster->getScale(),
                       monster);
    seen.push_back( shape );
    */
    char *p = (char*)name.c_str();
    creatureNames[ count ] = (char*)malloc( 120 * sizeof(char) );
    strcpy( creatureNames[ count ], p );
    count++;
  }
  creatureList->setLines( creatureMap->size(), (const char**)creatureNames );
  h += d;

  // shapes
  shapeButton = mainWin->createButton( 5, h, w - 10, h + 20, "Shape", true );
  toggleButtonList.push_back( shapeButton );
  shapeList = new ScrollingList( 5, h + 30, w - 10, 100, 
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
  h += d;
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

  if( scourge->getMap()->getCursorFlatMapX() < MAP_WIDTH &&
      scourge->getMap()->getCursorFlatMapY() < MAP_DEPTH ) {
    Location *pos = scourge->getMap()->getLocation( scourge->getMap()->getCursorFlatMapX(), 
                                                    scourge->getMap()->getCursorFlatMapY(),
                                                    0 );
    scourge->getSDLHandler()->texPrint( 50, 120, "F:%d,%d C:%d,%d Shape=%s Item=%s Creature=%s", 
                                        scourge->getMap()->getCursorFlatMapX(), 
                                        scourge->getMap()->getCursorFlatMapY(), 
                                        scourge->getMap()->getCursorChunkX(), 
                                        scourge->getMap()->getCursorChunkY(),
                                        ( pos ? pos->shape->getName() : "NULL" ),
                                        ( pos && pos->item ? ((Item*)(pos->item))->getRpgItem()->getName() : "NULL" ),
                                        ( pos && pos->creature ? pos->creature->getName() : "NULL" ) );
  }
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
  scourge->getMap()->cursorWidth = scourge->getMap()->cursorDepth = 1;
  scourge->getMap()->cursorHeight = MAP_WALL_HEIGHT;
  GLShape *shape;
  if( getShape( &shape ) ) {
    scourge->getMap()->cursorWidth = shape->getWidth();
    scourge->getMap()->cursorDepth = shape->getDepth();
    scourge->getMap()->cursorHeight = shape->getHeight();
  }
  
  // find the highest point under the cursor
  int maxz = 0;
  for( int gx = 0; gx < scourge->getMap()->cursorWidth; gx++ ) {
    for( int gy = 0; gy < scourge->getMap()->cursorDepth; gy++ ) {
      for( int i = MAP_VIEW_HEIGHT - 1; i >= 0; i-- ) {
        if( scourge->getMap()->getLocation( scourge->getMap()->getCursorFlatMapX() + gx,
                                            scourge->getMap()->getCursorFlatMapY() - gy, 
                                            i ) &&
            maxz < i ) {
          maxz = i;
        }
      }
    }
  }
  scourge->getMap()->cursorZ = ( maxz >= MAP_VIEW_HEIGHT - 1 ? 0 : maxz + 1 );
  int editorZ = scourge->getMap()->cursorZ;
  if( !maxz ) editorZ = 0;
  

  scourge->getMap()->handleEvent( event );

  switch(event->type) {
  //case SDL_MOUSEMOTION:
  /*
  if( event->motion.state == SDL_BUTTON_LEFT &&
      newMapWin->isVisible() ) {
    mapWidget->setPosition( scourge->getMouseX() - newMapWin->getX() - mapWidget->getCanvas()->getX(),
                            scourge->getMouseY() - newMapWin->getY() - mapWidget->getCanvas()->getY() - Window::TOP_HEIGHT );
  }
  */
  case SDL_MOUSEBUTTONUP:
  processMouseMotion( event->button.button, editorZ );  
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

  char result[1000];
  if( widget == saveButton ) {
    scourge->getMap()->saveMap( nameText->getText(), result );
    scourge->showMessageDialog( result );
//    scourge->getParty()->toggleRound( false );
  } else if( widget == loadButton ) {
    scourge->getMap()->loadMap( nameText->getText(), result );
    scourge->showMessageDialog( result );
//    scourge->getParty()->toggleRound( false );
  } else if( widget == newButton ) {
    newMapWin->setVisible( true );
    char tmp[1000];
    sprintf( tmp, "%d", this->level );
    levelText->setText( (const char*)tmp );
    sprintf( tmp, "%d", this->depth );
    depthText->setText( (const char*)tmp );
    // FIXME: select theme!
    cerr << "FIXME: select theme!" << endl;
    mapWidget->setSelection( scourge->getMap()->mapGridX, scourge->getMap()->mapGridY );
  } else if( widget == okButton || widget == applyButton ) {
    newMapWin->setVisible( false );

    if( widget == okButton ) {
      scourge->getMap()->reset();
      int line = themeList->getSelectedLine();
      if( line > -1 ) {
        char *p = strdup( themeNames[ line ] );
        if( !strcmp( p + strlen( p ) - 3, "(S)" ) ) *( p + strlen( p ) - 3 ) = 0;
        scourge->getShapePalette()->loadTheme( p );
        free( p );
      }
    }
    this->level = atoi( levelText->getText() );
    this->depth = atoi( depthText->getText() );
    mapWidget->getSelection( &(scourge->getMap()->mapGridX), &(scourge->getMap()->mapGridY) );

  } else if( widget == cancelButton ) {
    newMapWin->setVisible( false );
  } else if( widget == floorType ) {
    floorType->setLabel( floorTypeName[ floorType->isSelected() ? 0 : 1 ] );
  } else if( widget == mapWidget->getCanvas() ) {
//    mapWidget->setPosition( scourge->getMouseX() - newMapWin->getX() - mapWidget->getCanvas()->getX(),
//                            scourge->getMouseY() - newMapWin->getY() - mapWidget->getCanvas()->getY() - Window::TOP_HEIGHT );
  }

  return false;
}

void MapEditor::show() { 
  scourge->getMap()->setMapSettings( mapSettings );
  scourge->getMap()->reset();
  scourge->getMap()->center( MAP_WIDTH / 2, MAP_DEPTH / 2, true );
  scourge->getMap()->setViewArea( 0, 0, 
                                  mainWin->getX(), 
                                  mainWin->getY() + mainWin->getHeight() );
  scourge->getShapePalette()->loadTheme( "egypt" );
  mainWin->setVisible( true ); 
}

void MapEditor::hide() { 
  mainWin->setVisible( false ); 
  scourge->getMap()->setMapSettings( scourge->getMapSettings() );
}

bool MapEditor::getShape( GLShape **shape, 
                          Item **item, 
                          Creature **creature ) {
  *shape = NULL;
  if( item ) *item = NULL;
  if( creature ) *creature = NULL;
  if( creatureButton->isSelected() && 
      creatureList->getSelectedLine() > -1 ) {
    Monster *monster = Monster::getMonsterByName( creatureNames[ creatureList->getSelectedLine() ] );
    // cache creature shapes
    if( creatureOutlines.find( monster ) == creatureOutlines.end() ) {
      creatureOutlines[ monster ] = scourge->getSession()->getShapePalette()->
        getCreatureShape(monster->getModelName(), 
                         monster->getSkinName(), 
                         monster->getScale(),
                         monster);
    }
    *shape = creatureOutlines[ monster ];
    if( creature ) {
      cerr << "new creature" << endl;
      *creature = scourge->getSession()->newCreature( monster, *shape );
    }
    return true;
  } else if( itemButton->isSelected() &&
             itemList->getSelectedLine() > -1 ) {
    RpgItem *rpgItem = 
      RpgItem::getItemByName( itemNames[ itemList->getSelectedLine() ] );
    *shape = scourge->getShapePalette()->getShape( rpgItem->getShapeIndex() );
    if( item ) {
      cerr << "new item" << endl;
      *item = scourge->getSession()->newItem( rpgItem, level );
      // fill the container with random items
      if( rpgItem->isContainer() ) {
        // some items
        int n = (int)(3.0f * rand() / RAND_MAX);
        for(int i = 0; i < n; i++) {
          RpgItem *containedItem = RpgItem::getRandomItem( depth );
          if(containedItem) 
            (*item)->addContainedItem(scourge->getSession()->
                                      newItem(containedItem, level), 
                                      true);
        }
        // some spells
        if(!((int)(25.0f * rand() / RAND_MAX))) {
          int n = (int)(2.0f * rand() / RAND_MAX) + 1;
          for(int i = 0; i < n; i++) {
            Spell *spell = MagicSchool::getRandomSpell( level );
            if( spell ) {
              Item *scroll = scourge->getSession()->
                newItem(RpgItem::getItemByName("Scroll"), level, spell);
              (*item)->addContainedItem(scroll, true);
            }
          }
        }

        // print summary
        cerr << "Container contents:" << endl;
        for( int i = 0; i < (*item)->getContainedItemCount(); i++ ) {
          cerr << "\t" << (*item)->getContainedItem( i )->getRpgItem()->getName() << endl;
        }
      }
    }
    return true;
  } else if( shapeButton->isSelected() && 
             shapeList->getSelectedLine() > -1 ) {
    *shape = scourge->getShapePalette()->
      findShapeByName( shapeNames[ shapeList->getSelectedLine() ] );
    return true;
  } else {
    return false;
  }
}

void MapEditor::processMouseMotion( Uint8 button, int editorZ ) {
  if( scourge->getSDLHandler()->mouseX < mainWin->getX() && 
      ( button == SDL_BUTTON_LEFT || 
        button == SDL_BUTTON_RIGHT ) ) {

    // draw the correct walls in this chunk
    int xx = scourge->getMap()->getCursorFlatMapX();
    int yy = scourge->getMap()->getCursorFlatMapY() - 1;

    if( startPosButton->isSelected() ) {
      scourge->getMap()->startx = xx;
      scourge->getMap()->starty = yy;
      return;
    }

    int mapx = ( ( xx - MAP_OFFSET )  / MAP_UNIT ) * MAP_UNIT + MAP_OFFSET;
    int mapy = ( ( yy - MAP_OFFSET )  / MAP_UNIT ) * MAP_UNIT + MAP_OFFSET;

    //int mapx = scourge->getMap()->getCursorChunkX() * MAP_UNIT + MAP_OFFSET;
    //int mapy = scourge->getMap()->getCursorChunkY() * MAP_UNIT + MAP_OFFSET;

    GLShape *shape;
    if( button == SDL_BUTTON_LEFT ) {
      Item *item;
      Creature *creature;
      if( getShape( &shape, &item, &creature ) ) {
        if( item ) {
          scourge->getMap()->setItem( xx, yy + 1, editorZ, item );
        } else if( creature ) {
          scourge->getMap()->setCreature( xx, yy + 1, editorZ, creature );
        } else if( shape ) {
          scourge->getMap()->setPosition( xx, yy + 1, editorZ, shape );
        }
        return;
      }
    } else if( button == SDL_BUTTON_RIGHT ) {
      if( getShape( &shape ) ) {
        for( int sx = 0; sx < shape->getWidth(); sx++ ) {
          for( int sy = 0; sy < shape->getDepth(); sy++ ) {
            for( int sz = MAP_VIEW_HEIGHT - 1; sz >= 0; sz-- ) {
              scourge->getMap()->removeLocation( xx + sx, yy - sy + 1, sz );
            }
          }
        }
        return;
      }
    }

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
        removeLocation( mapx, mapy + MAP_UNIT, 0 );
    } else if( !scourge->getMap()->getLocation( mapx, mapy + MAP_UNIT, 0 ) ) {
      scourge->getMap()->
        setPosition( mapx, mapy + MAP_UNIT, 0, 
                     scourge->getShapePalette()->
                     findShapeByName( "CORNER", true ) );
    }

    // corner
    if( north ) {
      scourge->getMap()->
        removeLocation( mapx, mapy + MAP_UNIT_OFFSET, 0 ); 
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

    // add a light
    if((int) (100.0 * rand()/RAND_MAX) <= 25) {
      if( dir == 1 ) {
        scourge->getMap()->
          setPosition( mapx + MAP_UNIT_OFFSET, mapy + MAP_UNIT - 4, 
                       6, scourge->getShapePalette()->findShapeByName( "LAMP_WEST", true ) );
        scourge->getMap()->
          setPosition( mapx + MAP_UNIT_OFFSET, mapy + MAP_UNIT - 4, 
                       4, scourge->getShapePalette()->findShapeByName( "LAMP_BASE", true ) );
      } else {
        scourge->getMap()->
          setPosition( mapx - 1, mapy + MAP_UNIT - 4, 
                       6, scourge->getShapePalette()->findShapeByName("LAMP_EAST", true));
        scourge->getMap()->
          setPosition( mapx - 1, mapy + MAP_UNIT - 4, 
                       4, scourge->getShapePalette()->findShapeByName("LAMP_BASE", true));
      }
    }


    // change north chunk
    //cerr << "Looking north of EW_WALL map=" << mapx << "," << mapy << endl;
    if( isShape( mapx, mapy - MAP_UNIT_OFFSET, 0, "EW_WALL" ) &&
        isShape( mapx, mapy, 0, "CORNER" ) &&
        !scourge->getMap()->getLocation( mapx + dir * ( MAP_UNIT / 2 ), mapy - 1, 0 ) ) {
      //cerr << "Success!" << endl;
      scourge->getMap()->removeLocation( mapx, mapy - MAP_UNIT_OFFSET, 0 );
      scourge->getMap()->removeLocation( mapx, mapy, 0 );
      scourge->getMap()->
        setPosition( mapx, mapy, 0, scourge->getShapePalette()->
                     findShapeByName( "EW_WALL_EXTRA", true ) );
    }

    //cerr << "Looking north of EW_WALL_EXTRA map=" << mapx << "," << mapy << endl;
    if( isShape( mapx, mapy - MAP_UNIT_OFFSET, 0, "EW_WALL_EXTRA" ) &&
        isShape( mapx, mapy, 0, "CORNER" ) &&
        !scourge->getMap()->getLocation( mapx + dir * ( MAP_UNIT / 2 ), mapy - 1, 0 ) ) {
      //cerr << "Success!" << endl;
      scourge->getMap()->removeLocation( mapx, mapy - MAP_UNIT_OFFSET, 0 );
      scourge->getMap()->removeLocation( mapx, mapy, 0 );
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
      scourge->getMap()->removeLocation( mapx, mapy + MAP_UNIT + MAP_UNIT - MAP_UNIT_OFFSET, 0 );
      scourge->getMap()->removeLocation( mapx, mapy + MAP_UNIT + MAP_UNIT_OFFSET, 0 );
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
      scourge->getMap()->removeLocation( mapx, mapy + MAP_UNIT + MAP_UNIT, 0 );
      scourge->getMap()->removeLocation( mapx, mapy + MAP_UNIT + MAP_UNIT_OFFSET, 0 );
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
        removeLocation( mapx, mapy + MAP_UNIT_OFFSET, 0 );
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
        removeLocation( mapx + MAP_UNIT - MAP_UNIT_OFFSET, mapy + MAP_UNIT_OFFSET, 0 );
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

    // Add a light
    if((int) (100.0 * rand()/RAND_MAX) <= 25) {
      if( dir == 1 ) {
        scourge->getMap()->
          setPosition(mapx + 4, mapy + MAP_UNIT_OFFSET + 1, 6, 
                      scourge->getShapePalette()->findShapeByName( "LAMP_NORTH", true) );
        scourge->getMap()->
          setPosition(mapx + 4, mapy + MAP_UNIT_OFFSET + 1, 4, 
                      scourge->getShapePalette()->findShapeByName("LAMP_BASE", true));
      } else {
        /*
        See gltorch.cpp on why there aren't southern torches... <sigh>
        cerr << "adding South light" << endl;
        scourge->getMap()->
          setPosition(mapx + 4, mapy, 6, 
                      scourge->getShapePalette()->findShapeByName("LAMP_SOUTH", true));
        scourge->getMap()->
          setPosition(mapx + 4, mapy, 4, 
                      scourge->getShapePalette()->findShapeByName("LAMP_BASE", true));

        */
      }
    }

    // change west chunk
    if( isShape( mapx - MAP_UNIT + MAP_UNIT_OFFSET, mapy + MAP_UNIT_OFFSET, 0, "NS_WALL" ) &&
        isShape( mapx - MAP_UNIT_OFFSET, mapy + MAP_UNIT_OFFSET, 0, "CORNER" ) &&
        !scourge->getMap()->getLocation( mapx - MAP_UNIT_OFFSET, mapy + dir * ( MAP_UNIT / 2 ), 0 ) ) {
      scourge->getMap()->removeLocation( mapx - MAP_UNIT_OFFSET, mapy + MAP_UNIT_OFFSET, 0 );
      scourge->getMap()->removeLocation( mapx - MAP_UNIT + MAP_UNIT_OFFSET, mapy + MAP_UNIT_OFFSET, 0 );
      scourge->getMap()->
        setPosition( mapx - MAP_UNIT + MAP_UNIT_OFFSET, mapy + MAP_UNIT_OFFSET, 0, 
                     scourge->getShapePalette()->findShapeByName( "NS_WALL_EXTRA", true ) );
    }
    if( isShape( mapx - MAP_UNIT, mapy + MAP_UNIT_OFFSET, 0, "NS_WALL_EXTRA" ) &&
        isShape( mapx - MAP_UNIT_OFFSET, mapy + MAP_UNIT_OFFSET, 0, "CORNER" ) &&
        !scourge->getMap()->getLocation( mapx - MAP_UNIT_OFFSET, mapy + dir * ( MAP_UNIT / 2 ), 0 ) ) {
      scourge->getMap()->removeLocation( mapx - MAP_UNIT_OFFSET, mapy + MAP_UNIT_OFFSET, 0 );
      scourge->getMap()->removeLocation( mapx - MAP_UNIT, mapy + MAP_UNIT_OFFSET, 0 );
      scourge->getMap()->
        setPosition( mapx - MAP_UNIT, mapy + MAP_UNIT_OFFSET, 0, 
                     scourge->getShapePalette()->findShapeByName( "NS_WALL_TWO_EXTRAS", true ) );
    }

    // change the east chunk
    if( isShape( mapx + MAP_UNIT + MAP_UNIT_OFFSET, mapy + MAP_UNIT_OFFSET, 0, "NS_WALL" ) &&
        isShape( mapx + MAP_UNIT, mapy + MAP_UNIT_OFFSET, 0, "CORNER" ) &&
        !scourge->getMap()->getLocation( mapx + MAP_UNIT, mapy + dir * ( MAP_UNIT / 2 ), 0 ) ) {
      scourge->getMap()->removeLocation( mapx + MAP_UNIT + MAP_UNIT_OFFSET, mapy + MAP_UNIT_OFFSET, 0 );
      scourge->getMap()->removeLocation( mapx + MAP_UNIT, mapy + MAP_UNIT_OFFSET, 0 );
      scourge->getMap()->
        setPosition( mapx + MAP_UNIT, mapy + MAP_UNIT_OFFSET, 0,
                     scourge->getShapePalette()->
                     findShapeByName( "NS_WALL_EXTRA", true ) );
    }
    if( isShape( mapx + MAP_UNIT + MAP_UNIT_OFFSET, mapy + MAP_UNIT_OFFSET, 0, "NS_WALL_EXTRA" ) &&
        isShape( mapx + MAP_UNIT, mapy + MAP_UNIT_OFFSET, 0, "CORNER" ) &&
        !scourge->getMap()->getLocation( mapx + MAP_UNIT, mapy + dir * ( MAP_UNIT / 2 ), 0 ) ) {
      scourge->getMap()->removeLocation( mapx + MAP_UNIT + MAP_UNIT_OFFSET, mapy + MAP_UNIT_OFFSET, 0 );
      scourge->getMap()->removeLocation( mapx + MAP_UNIT, mapy + MAP_UNIT_OFFSET, 0 );
      scourge->getMap()->
        setPosition( mapx + MAP_UNIT, mapy + MAP_UNIT_OFFSET, 0,
                     scourge->getShapePalette()->
                     findShapeByName( "NS_WALL_TWO_EXTRAS", true ) );
    }
  }
}

void MapEditor::addFloor( Sint16 mapx, Sint16 mapy ) {
  if( scourge->getMap()->getFloorPosition( mapx, mapy + MAP_UNIT ) ) return;
  scourge->getMap()->
    setFloorPosition( mapx, mapy + MAP_UNIT, 
                      scourge->getShapePalette()->
                      findShapeByName( floorTypeName[ floorType->isSelected() ? 0 : 1 ], 
                                       true ) );
}

void MapEditor::removeFloor( Sint16 mapx, Sint16 mapy ) {
  scourge->getMap()->removeFloorPosition( mapx, mapy + MAP_UNIT );
}

void MapEditor::removeEWWall( Sint16 mapx, Sint16 mapy, int dir ) {
  for( int y = 1; y <= MAP_UNIT; y++ ) {
    for( int z = 0; z < MAP_VIEW_HEIGHT; z++ ) {
      scourge->getMap()->removeLocation( mapx + 1, mapy + y, z );
    }
  }
}

void MapEditor::removeNSWall( Sint16 mapx, Sint16 mapy, int dir ) {
  for( int x = 0; x < MAP_UNIT; x++ ) {
    for( int z = 0; z < MAP_VIEW_HEIGHT; z++ ) {
      scourge->getMap()->removeLocation( mapx + x, mapy + MAP_UNIT_OFFSET - 1, z );
    }
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
      levelMap->removeLocation( mapx - 1, mapy + 1, 0 );
      levelMap->removeLocation( mapx, mapy - 1, 0 );
    
      // change west chunk
      if( nsWall ) {
        levelMap->removeLocation( mapx - MAP_UNIT + MAP_UNIT_OFFSET, mapy + MAP_UNIT_OFFSET, 0 );
        levelMap->setPosition( mapx - MAP_UNIT + MAP_UNIT_OFFSET, mapy + MAP_UNIT_OFFSET, 0,
                               pal->findShapeByName( "NS_WALL_EXTRA", true ) );
      } else if( nsWallExtra ) {
        levelMap->removeLocation( mapx - MAP_UNIT, mapy + MAP_UNIT_OFFSET, 0 );
        levelMap->setPosition( mapx - MAP_UNIT, mapy + MAP_UNIT_OFFSET, 0,
                               pal->findShapeByName( "NS_WALL_TWO_EXTRAS", true ) );
      }
      
      // change north chunk
      if( ewWall ) {
        levelMap->removeLocation( mapx, mapy - MAP_UNIT_OFFSET, 0 );
        levelMap->setPosition( mapx, mapy, 0,
                               pal->findShapeByName( "EW_WALL_EXTRA", true ) );
      } else if( ewWallExtra ) {
        levelMap->removeLocation( mapx, mapy - MAP_UNIT_OFFSET, 0 );
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
      levelMap->removeLocation( mapx + MAP_UNIT, mapy + 1, 0 );
      levelMap->removeLocation( mapx + MAP_UNIT - MAP_UNIT_OFFSET, mapy - 1, 0 );
    
      // change west chunk
      if( nsWall ) {
        levelMap->removeLocation( mapx + MAP_UNIT + MAP_UNIT_OFFSET, mapy + MAP_UNIT_OFFSET, 0 );
        levelMap->setPosition( mapx + MAP_UNIT, mapy + MAP_UNIT_OFFSET, 0,
                               pal->findShapeByName( "NS_WALL_EXTRA", true ) );
      } else if( nsWallExtra ) {
        levelMap->removeLocation( mapx + MAP_UNIT + MAP_UNIT_OFFSET, mapy + MAP_UNIT_OFFSET, 0 );
        levelMap->setPosition( mapx + MAP_UNIT, mapy + MAP_UNIT_OFFSET, 0,
                               pal->findShapeByName( "NS_WALL_TWO_EXTRAS", true ) );
      }
      
      // change north chunk
      if( ewWall ) {
        levelMap->removeLocation( mapx + MAP_UNIT - MAP_UNIT_OFFSET, mapy - MAP_UNIT_OFFSET, 0 );
        levelMap->setPosition( mapx + MAP_UNIT - MAP_UNIT_OFFSET, mapy, 0,
                               pal->findShapeByName( "EW_WALL_EXTRA", true ) );
      } else if( ewWallExtra ) {
        levelMap->removeLocation( mapx + MAP_UNIT - MAP_UNIT_OFFSET, mapy - MAP_UNIT_OFFSET, 0 );
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
      levelMap->removeLocation( mapx + MAP_UNIT, mapy + MAP_UNIT_OFFSET, 0 );
      levelMap->removeLocation( mapx + MAP_UNIT - MAP_UNIT_OFFSET, mapy + MAP_UNIT + MAP_UNIT_OFFSET, 0 );
    
      // change west chunk
      if( nsWall ) {
        levelMap->removeLocation( mapx + MAP_UNIT + MAP_UNIT_OFFSET, mapy + MAP_UNIT, 0 );
        levelMap->setPosition( mapx + MAP_UNIT, mapy + MAP_UNIT, 0,
                               pal->findShapeByName( "NS_WALL_EXTRA", true ) );
      } else if( nsWallExtra ) {
        levelMap->removeLocation( mapx + MAP_UNIT + MAP_UNIT_OFFSET, mapy + MAP_UNIT, 0 );
        levelMap->setPosition( mapx + MAP_UNIT, mapy + MAP_UNIT, 0,
                               pal->findShapeByName( "NS_WALL_TWO_EXTRAS", true ) );
      }
      
      // change north chunk
      if( ewWall ) {
        levelMap->removeLocation( mapx + MAP_UNIT - MAP_UNIT_OFFSET, mapy + MAP_UNIT + MAP_UNIT - MAP_UNIT_OFFSET, 0 );
        levelMap->setPosition( mapx + MAP_UNIT - MAP_UNIT_OFFSET, mapy + MAP_UNIT + MAP_UNIT - MAP_UNIT_OFFSET, 0,
                               pal->findShapeByName( "EW_WALL_EXTRA", true ) );
      } else if( ewWallExtra ) {
        levelMap->removeLocation( mapx + MAP_UNIT - MAP_UNIT_OFFSET, mapy + MAP_UNIT + MAP_UNIT, 0 );
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
      levelMap->removeLocation( mapx - MAP_UNIT_OFFSET, mapy + MAP_UNIT, 0 );
      levelMap->removeLocation( mapx, mapy + MAP_UNIT + MAP_UNIT_OFFSET, 0 );
    
      // change west chunk
      if( nsWall ) {
        levelMap->removeLocation( mapx - MAP_UNIT + MAP_UNIT_OFFSET, mapy + MAP_UNIT, 0 );
        levelMap->setPosition( mapx - MAP_UNIT + MAP_UNIT_OFFSET, mapy + MAP_UNIT, 0,
                               pal->findShapeByName( "NS_WALL_EXTRA", true ) );
      } else if( nsWallExtra ) {
        levelMap->removeLocation( mapx - MAP_UNIT, mapy + MAP_UNIT, 0 );
        levelMap->setPosition( mapx - MAP_UNIT, mapy + MAP_UNIT, 0,
                               pal->findShapeByName( "NS_WALL_TWO_EXTRAS", true ) );
      }
      
      // change north chunk
      if( ewWall ) {
        levelMap->removeLocation( mapx, mapy + MAP_UNIT + MAP_UNIT - MAP_UNIT_OFFSET, 0 );
        levelMap->setPosition( mapx, mapy + MAP_UNIT + MAP_UNIT - MAP_UNIT_OFFSET, 0,
                               pal->findShapeByName( "EW_WALL_EXTRA", true ) );
      } else if( ewWallExtra ) {
        levelMap->removeLocation( mapx, mapy + MAP_UNIT + MAP_UNIT, 0 );
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
    Location *pos = scourge->getMap()->getLocation( mapx, mapy, mapz );
//    cerr << " found=" << ( !pos ? "NULL" : pos->shape->getName() ) << endl;
    return( pos && !strcmp( pos->shape->getName(), name ) );
  } else {
//    cerr << " found nothing." << endl;
    return false;
  }
}

