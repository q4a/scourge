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
  shapeNames = (char**)malloc( ( scourge->getShapePalette()->getShapeCount() - 2 ) * 
                               sizeof(char*) );
  for(int i = 0; i < scourge->getShapePalette()->getShapeCount() - 2; i++) {
    shapeNames[i] = (char*)malloc( 120 * sizeof(char) );
    // shapes are 1-based!
    strcpy( shapeNames[ i ], 
            scourge->getShapePalette()->getShape( i + 1 )->getName() );
  }
  shapeList->setLines( scourge->getShapePalette()->getShapeCount() - 2, 
                       (const char**)shapeNames );
}                                                                         

MapEditor::~MapEditor() {
  for(int i = 0; i < scourge->getShapePalette()->getShapeCount() - 2; i++) {
    free( shapeNames[ i ] );
  }
  free( shapeNames );
  delete mainWin;
}

void MapEditor::drawView() {
}

void MapEditor::drawAfter() {
}

bool MapEditor::handleEvent(SDL_Event *event) {
  switch(event->type) {
  case SDL_KEYDOWN:
    switch(event->key.keysym.sym) {
    case SDLK_ESCAPE: hide(); return true;
    default: break;
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


