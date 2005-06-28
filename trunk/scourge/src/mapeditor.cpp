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

  mainWin->createLabel( 5, 40, "Shapes:" );
  shapeList = new ScrollingList( 5, 50, w - 10, 150, 
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
  if( button == SDL_BUTTON_LEFT ) {
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
      addEWWall( mapx, mapy + 1, 1 ); // west
    } else if( innerY < MAP_UNIT_OFFSET ) { 
      addNSWall( mapx, mapy + 1, 1 ); // north
    } else if( innerX >= MAP_UNIT - MAP_UNIT_OFFSET ) {
      addEWWall( mapx + MAP_UNIT - MAP_UNIT_OFFSET, mapy + 1, -1 ); // east
    } else if( innerY >= MAP_UNIT - MAP_UNIT_OFFSET ) {
      addNSWall( mapx, mapy + MAP_UNIT - MAP_UNIT_OFFSET + 1, -1 ); // south
    } else {
      addFloor( mapx, mapy + 1 );
    }
  }
}

void MapEditor::addEWWall( Sint16 mapx, Sint16 mapy, int dir ) {
  if( scourge->getMap()->getLocation( mapx, mapy + MAP_UNIT / 2, 0 ) ) return;

  int check = mapx + dir * ( MAP_UNIT / 2 );
  bool north = ( check >= 0 && check < MAP_WIDTH && 
                 scourge->getMap()->getLocation( check, mapy + 1, 0 ) );
  bool south = ( check >= 0 && check < MAP_WIDTH && 
                 scourge->getMap()->getLocation( check, mapy + MAP_UNIT - 1, 0 ) );
  
  if( !north && !south ) {
    scourge->getMap()->setPosition( mapx, mapy + MAP_UNIT, 0, 
                                    scourge->getShapePalette()->findShapeByName( "EW_WALL_TWO_EXTRAS", true ) );
  } else if( !south ) {
    scourge->getMap()->setPosition( mapx, mapy + MAP_UNIT, 0, 
                                    scourge->getShapePalette()->findShapeByName( "EW_WALL_EXTRA", true ) );
  } else if( !north ) {
    scourge->getMap()->setPosition( mapx, mapy + MAP_UNIT - MAP_UNIT_OFFSET, 0, 
                                    scourge->getShapePalette()->findShapeByName( "EW_WALL_EXTRA", true ) );
  } else {
    scourge->getMap()->setPosition( mapx, mapy + MAP_UNIT - MAP_UNIT_OFFSET, 0, 
                                    scourge->getShapePalette()->findShapeByName( "EW_WALL", true ) );
  }
}

void MapEditor::addNSWall( Sint16 mapx, Sint16 mapy, int dir ) {
  if( scourge->getMap()->getLocation( mapx + MAP_UNIT / 2, mapy + MAP_UNIT_OFFSET, 0 ) ) return;
  
  int check = mapy + dir * ( MAP_UNIT / 2 );
  bool west = ( check >= 0 && check < MAP_DEPTH && 
                scourge->getMap()->getLocation( mapx, check, 0 ) );
  bool east = ( check >= 0 && check < MAP_DEPTH && 
                scourge->getMap()->getLocation( mapx + MAP_UNIT - 1, check, 0 ) );

  if( !east && !west ) {
    scourge->getMap()->setPosition( mapx, mapy + MAP_UNIT_OFFSET, 0,
                                    scourge->getShapePalette()->findShapeByName( "NS_WALL_TWO_EXTRAS", true ) );
  } else if( !west ) {
    scourge->getMap()->setPosition( mapx, mapy + MAP_UNIT_OFFSET, 0,
                                    scourge->getShapePalette()->findShapeByName( "NS_WALL_EXTRA", true ) );
  } else if( !east ) {
    scourge->getMap()->setPosition( mapx + MAP_UNIT_OFFSET, mapy + MAP_UNIT_OFFSET, 0,
                                    scourge->getShapePalette()->findShapeByName( "NS_WALL_EXTRA", true ) );
  } else {
    scourge->getMap()->setPosition( mapx + MAP_UNIT_OFFSET, mapy + MAP_UNIT_OFFSET, 0,
                                    scourge->getShapePalette()->findShapeByName( "NS_WALL", true ) );
  }
}

void MapEditor::addFloor( Sint16 mapx, Sint16 mapy ) {
  if( scourge->getMap()->getFloorPosition( mapx, mapy + MAP_UNIT ) ) return;
  scourge->getMap()->setFloorPosition( mapx, mapy + MAP_UNIT, 
                                       scourge->getShapePalette()->findShapeByName( "FLOOR_TILE", true ) );
}

