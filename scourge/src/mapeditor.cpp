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

#define MOUSE_ROT_DELTA 2

/**
  *@author Gabor Torok
  */

MapEditor::MapEditor( Scourge *scourge ) {
  this->scourge = scourge;

  mapSettings = new EditorMapSettings();
  mouseMoveScreen = true;
  mouseZoom = mouseRot = false;

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

  // cancel mouse-based map movement (middle button)
  if(mouseRot) {
    scourge->getMap()->setXRot(0);
    scourge->getMap()->setYRot(0);
    scourge->getMap()->setZRot(0);
  }
  if(mouseZoom) {
    mouseZoom = false;
    scourge->getMap()->setZoomIn(false);
    scourge->getMap()->setZoomOut(false);
  }

  if(move) scourge->getMap()->move( move );
}

void MapEditor::drawAfter() {
}

bool MapEditor::handleEvent(SDL_Event *event) {
  int ea;
  int mx, my;
  switch(event->type) {
  case SDL_MOUSEMOTION:
    if(mouseRot) {
      scourge->getMap()->setZRot(-event->motion.xrel * MOUSE_ROT_DELTA);
      scourge->getMap()->setYRot(-event->motion.yrel * MOUSE_ROT_DELTA);
    } else {
      //sdlHandler->applyMouseOffset(event->motion.x, event->motion.y, &mx, &my);
      mx = event->motion.x;
      my = event->motion.y;
      if(mx < 10) {
        mouseMoveScreen = true;
        setMove(Constants::MOVE_LEFT);
      } else if(mx >= scourge->getSDLHandler()->getScreen()->w - 10) {
        mouseMoveScreen = true;
        setMove(Constants::MOVE_RIGHT);
      } else if(my < 10) {
        mouseMoveScreen = true;
        setMove(Constants::MOVE_UP);
      } else if(my >= scourge->getSDLHandler()->getScreen()->h - 10) {
        mouseMoveScreen = true;
        setMove(Constants::MOVE_DOWN);
      } else {
        if(mouseMoveScreen) {
          mouseMoveScreen = false;
          removeMove(Constants::MOVE_LEFT | Constants::MOVE_RIGHT);
          removeMove(Constants::MOVE_UP | Constants::MOVE_DOWN);
          scourge->getMap()->setYRot(0.0f);
          scourge->getMap()->setZRot(0.0f);
        }
      }
    }
    break;
  case SDL_MOUSEBUTTONDOWN:
  if( event->button.button ) {
    if( event->button.button == SDL_BUTTON_MIDDLE ) {
      mouseRot = true;
    } if( event->button.button == SDL_BUTTON_WHEELUP ) {
      mouseZoom = true;
      scourge->getMap()->setZoomIn(false);
      scourge->getMap()->setZoomOut(true);
    } if( event->button.button == SDL_BUTTON_WHEELDOWN ) {
      mouseZoom = true;
      scourge->getMap()->setZoomIn(true);
      scourge->getMap()->setZoomOut(false);
    }
  }
  break;  
  case SDL_MOUSEBUTTONUP:
  if( event->button.button ) {
    if( event->button.button == SDL_BUTTON_MIDDLE ) {
      mouseRot = false;
      scourge->getMap()->setXRot(0);
      scourge->getMap()->setYRot(0);
      scourge->getMap()->setZRot(0);
    }
  } 
  break;
  case SDL_KEYDOWN:
  case SDL_KEYUP:
    if(event->type == SDL_KEYUP && event->key.keysym.sym == SDLK_ESCAPE) {
      hide();
      return true;
    }
    // xxx_yyy_stop means : "do xxx_yyy action when the corresponding key is up"
    ea = scourge->getUserConfiguration()->getEngineAction(event);    
    if(ea == SET_MOVE_DOWN){        
      setMove(Constants::MOVE_DOWN);
    } else if(ea == SET_MOVE_UP){
      setMove(Constants::MOVE_UP);
    } else if(ea == SET_MOVE_RIGHT){
      setMove(Constants::MOVE_RIGHT);
    } else if(ea == SET_MOVE_LEFT){
      setMove(Constants::MOVE_LEFT);
    } else if(ea == SET_MOVE_DOWN_STOP){        
      scourge->getMap()->setYRot(0.0f);
      scourge->getMap()->setYRot(0);
      removeMove(Constants::MOVE_DOWN);
    } else if(ea == SET_MOVE_UP_STOP){
      scourge->getMap()->setYRot(0.0f);
      scourge->getMap()->setYRot(0);
      removeMove(Constants::MOVE_UP);
    } else if(ea == SET_MOVE_RIGHT_STOP){
      scourge->getMap()->setYRot(0.0f);
      scourge->getMap()->setZRot(0);
      removeMove(Constants::MOVE_RIGHT);
    } else if(ea == SET_MOVE_LEFT_STOP){
      scourge->getMap()->setYRot(0.0f);
      scourge->getMap()->setZRot(0);
      removeMove(Constants::MOVE_LEFT);
    } else if(ea == SET_ZOOM_IN){
      scourge->getMap()->setZoomIn(true);
    } else if(ea == SET_ZOOM_OUT){
      scourge->getMap()->setZoomOut(true);
    } else if(ea == SET_ZOOM_IN_STOP){
      scourge->getMap()->setZoomIn(false);
    } else if(ea == SET_ZOOM_OUT_STOP){
      scourge->getMap()->setZoomOut(false);
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
  mainWin->setVisible( true ); 
}

void MapEditor::hide() { 
  mainWin->setVisible( false ); 
  scourge->getMap()->setMapSettings( scourge->getMapSettings() );
}


