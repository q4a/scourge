/***************************************************************************
                          mapwidget.cpp  -  description
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

#include "mapwidget.h"
#include "scourge.h"
#include "gui/canvas.h"

/**
  *@author Gabor Torok
  */
  
#define MAP_GRID_HEIGHT ( Constants::MAP_GRID_TILE_HEIGHT * Constants::MAP_GRID_TILE_PIXEL_HEIGHT )
#define MAP_GRID_WIDTH ( Constants::MAP_GRID_TILE_WIDTH * Constants::MAP_GRID_TILE_PIXEL_WIDTH )

MapWidget::MapWidget( Scourge *scourge, int x, int y, int x2, int y2 ) {
  this->scourge = scourge;
  this->canvas = new Canvas( x, y, x2, y2, this, this );
  oldSelX = oldSelY = selX = selY = 0;
  oldx = oldy = 0;
  dragging = false;
  calculateValues();
}

MapWidget::~MapWidget() {
  delete canvas;
}

void MapWidget::setPosition( int x, int y ) {
  //cerr << "drag: " << x << "," << y << endl;
  selX = oldSelX - ( x - oldx );
  selY = oldSelY - ( y - oldy );

  if( selX < 0 ) selX = 0;
  if( selX >= MAP_GRID_WIDTH ) selX = MAP_GRID_WIDTH - 1;
  
  if( selY < 0 ) selY = 0;
  if( selY >= MAP_GRID_HEIGHT ) selY = MAP_GRID_HEIGHT - 1;
  
  calculateValues();
}

void MapWidget::receive(Widget *widget) {
  dragging = false;
}

bool MapWidget::startDrag(Widget *widget, int x, int y) {
  if( !dragging ) {
    oldx = x;
    oldy = y;
    oldSelX = selX;
    oldSelY = selY;
    dragging = true;
  }
  return true;
}

void MapWidget::calculateValues() { 
  // figure out what needs to show
  int sx = selX;
  int sy = selY;
  int ex = selX + canvas->getWidth();
  int ey = selY + canvas->getHeight();

  if( sx < 0 ) {
    ex = canvas->getWidth();
    sx = 0;
  }
  if( sy < 0 ) {
    sy = 0;
    ey = canvas->getHeight();
  }
  if( ex > MAP_GRID_WIDTH ) {
    sx = MAP_GRID_WIDTH - canvas->getWidth();
    ex = MAP_GRID_WIDTH;
  }
  if( ey > MAP_GRID_HEIGHT ) {
    sy = MAP_GRID_HEIGHT - canvas->getHeight();
    ey = MAP_GRID_HEIGHT;
  }

  gx = sx / Constants::MAP_GRID_TILE_PIXEL_WIDTH;
  gy = sy / Constants::MAP_GRID_TILE_PIXEL_HEIGHT;
  tx = sx % Constants::MAP_GRID_TILE_PIXEL_WIDTH;
  ty = sy % Constants::MAP_GRID_TILE_PIXEL_HEIGHT;

  /*
  cerr << "pixel=" << sx << "-" << ex << " , " << sy << "-" << ey <<
    " out of " << MAP_GRID_WIDTH << "," << MAP_GRID_HEIGHT <<
    " starting tile=" << gx << "," << gy <<
    " starting offset=-1 * (" << tx << "," << ty << ")" << endl;
  */
}

void MapWidget::drawWidget(Widget *w) {
  glEnable( GL_TEXTURE_2D );
  for( int xx = 0; xx < canvas->getWidth() / Constants::MAP_GRID_TILE_PIXEL_WIDTH + 2; xx++ ) {
    if( gx + xx >= Constants::MAP_GRID_TILE_WIDTH ) continue;
    for( int yy = 0; yy < canvas->getWidth() / Constants::MAP_GRID_TILE_PIXEL_HEIGHT + 2; yy++ ) {
      if( gy + yy >= Constants::MAP_GRID_TILE_HEIGHT ) continue;
      glPushMatrix();
      int xp = xx * Constants::MAP_GRID_TILE_PIXEL_WIDTH - tx;
      int yp = yy * Constants::MAP_GRID_TILE_PIXEL_HEIGHT - ty;
      glTranslatef( xp, yp, 0 );
//      cerr << "\txx=" << xx << " yy=" << yy << 
//        " gird=" << ( gx + xx ) << "," << ( gy + yy ) << endl;
      glBindTexture( GL_TEXTURE_2D, 
                     scourge->getShapePalette()->
                     getMapGridTile( gx + xx, gy + yy ) );
      glColor4f( 1, 1, 1, 1 );
      glBegin( GL_QUADS );
      glNormal3f( 0, 0, 1 );
      glTexCoord2f( 0, 0 );
      glVertex2f( 0, 0 );
      glTexCoord2f( 0, 1 );
      glVertex2f( 0, Constants::MAP_GRID_TILE_PIXEL_HEIGHT );
      glTexCoord2f( 1, 1 );
      glVertex2d( Constants::MAP_GRID_TILE_PIXEL_WIDTH, Constants::MAP_GRID_TILE_PIXEL_HEIGHT );
      glTexCoord2f( 1, 0 );
      glVertex2d( Constants::MAP_GRID_TILE_PIXEL_WIDTH, 0 );
      glEnd();
      glPopMatrix();
    }
  }  
}

